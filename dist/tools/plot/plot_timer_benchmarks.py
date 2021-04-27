import os
import argparse
import itertools
import subprocess
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
import xml.etree.ElementTree as ET

from ast import literal_eval


class FigurePlotter:
    def __init__(self, input, outdir, ci_build, board):
        self.root = ET.parse(input).getroot()
        self.outdir = outdir
        self.board = board
        if ci_build:
            self.plotlyjs = False
            self.full_html = False
        else:
            self.plotlyjs = "cdn"
            self.full_html = True

        version = self.root.find(".//property[@name='timer-version']")
        if version is None:
            raise RuntimeError("timer version not found")
        self.timer_version = version.get("value")

    def plot_accuracy(self):
        # parse
        data = {
            "function": [],
            "target_duration": [],
            "actual_duration": [],
            "timer_version": [],
            "board": [],
        }
        for prop in self.root.findall(
            "testcase[@classname='tests_{}_benchmarks.Sleep Accuracy']//property".format(
                self.timer_version
            )
        ):
            name = prop.get("name").split("-")
            if "TIMER_SLEEP" in name:
                function = "TIMER_SLEEP"
            elif "TIMER_SET" in name:
                function = "TIMER_SET"
            else:
                raise LookupError

            result_type = name[-1]
            target = literal_eval(name[-2])

            actual = [v * 1000000 for v in literal_eval(prop.get("value"))]
            if result_type == "dut":
                # dut results are in microseconds, convert to seconds to uniform with philip results
                actual = [v / 1000000 for v in actual]

            data["actual_duration"].extend(actual)
            data["target_duration"].extend([target] * len(actual))
            data["function"].extend([function] * len(actual))
            data["timer_version"].extend([self.timer_version] * len(actual))
            data["board"].extend([self.board] * len(actual))

        df = pd.DataFrame(data)
        df["diff_actual_target"] = df["actual_duration"] - df["target_duration"]
        df = (
            df.groupby(["timer_version", "board", "function", "target_duration"])
            .mean()
            .reset_index()
        )

        # plot
        fig = px.line(df, x="target_duration", y="diff_actual_target", color="function")

        fig.update_layout(
            dict(
                title="Sleep Accuracy  {:s}-{:s}".format(
                    self.board, self.timer_version
                ),
                xaxis_title="Target Sleep Duration [us]",
                yaxis_title="Delay from Target Sleep Duration [us]",
            ),
        )

        fig.write_html(
            "{}/{}.html".format(self.outdir, "accuracy"),
            full_html=self.full_html,
            include_plotlyjs=self.plotlyjs,
        )

    def plot_jitter(self):
        def get_value(property):
            return literal_eval(property.get("value"))

        def get_timer_count(property):
            return property.get("name").split("-")[1]

        def parse(root):
            data = {
                "i": [],
                "board": [],
                "timer_version": [],
                "timer_count": [],
                "timer_interval": [],
                "start_time": [],
                "wakeup_time": [],
            }

            testcase = root.find(
                "testcase[@classname='tests_{:s}_benchmarks.Sleep Jitter']".format(
                    self.timer_version
                )
            )

            timer_interval = testcase.find(".//property[@name='timer-interval']")
            if timer_interval is None:
                raise RuntimeError("timer_interval not found")

            start_times = [
                prop
                for prop in testcase.findall(".//property")
                if prop.get("name").endswith("start-time")
                and "dut" not in prop.get("name")
            ]
            wakeup_times = [
                prop
                for prop in testcase.findall(".//property")
                if prop.get("name").endswith("wakeup-time")
                and "dut" not in prop.get("name")
            ]

            for start, wakeup in zip(start_times, wakeup_times):
                timer_count = get_timer_count(start)
                s = get_value(start)
                w_values = [v * 1000000 for v in get_value(wakeup)]

                data["i"].extend(range(len(w_values)))
                data["start_time"].extend([s * 1000000] * len(w_values))
                data["wakeup_time"].extend(w_values)
                data["timer_count"].extend([timer_count] * len(w_values))
                data["timer_version"].extend([self.timer_version] * len(w_values))
                data["board"].extend([self.board] * len(w_values))
                data["timer_interval"].extend(
                    [get_value(timer_interval)] * len(w_values)
                )

            return pd.DataFrame(data)

        df = parse(self.root)

        df["calculated_target"] = (
            df["start_time"] + (df["i"] + 1) * df["timer_interval"]
        )
        df["diff_target_from_start"] = df["calculated_target"] - df["start_time"]
        df["diff_wakeup_from_start"] = df["wakeup_time"] - df["start_time"]
        df["diff_wakeup_from_target"] = df["wakeup_time"] - df["calculated_target"]

        fig = px.box(
            df,
            x="timer_count",
            y="diff_wakeup_from_target",
        )

        fig.update_layout(
            dict(
                title="Sleep Jitter  {:s}-{:s}".format(self.board, self.timer_version),
                xaxis_title="Number of Timers",
                yaxis_title="Delay from Target Wakeup Time [us]",
            ),
        )

        fig.write_html(
            "{}/{}.html".format(self.outdir, "jitter"),
            full_html=self.full_html,
            include_plotlyjs=self.plotlyjs,
        )

    def plot_set_remove_timer_from_list(self):
        def parse(root):
            data = {
                "i": [],
                "method": [],
                "duration": [],
                "timer_count": [],
                "timer_version": [],
                "board": [],
            }

            path = "testcase[@classname='tests_{:s}_benchmarks.Timer Overhead']//property".format(
                self.timer_version
            )
            properties = [
                p
                for p in root.findall(path)
                if "set" in p.get("name") or "remove" in p.get("name")
            ]

            for prop in properties:
                name = prop.get("name").split("-")

                values = [v * 1000000 for v in literal_eval(prop.get("value"))]
                data["i"].extend(range(len(values)))
                data["duration"].extend(values)
                data["timer_count"].extend([int(name[2])] * len(values))
                data["method"].extend([name[3]] * len(values))
                data["timer_version"].extend([self.timer_version] * len(values))
                data["board"].extend([self.board] * len(values))

            return pd.DataFrame(data)

        def plot_overhead_set_remove(type, df):
            if type not in ["set", "remove"]:
                raise ValueError

            df = df.loc[df["method"] == type]

            # add box plot
            fig = px.box(df, x="timer_count", y="duration")
            # add median trendline
            df_set_median = df.groupby("timer_count")["duration"].median().reset_index()
            fig.add_scatter(x=df_set_median["timer_count"], y=df_set_median["duration"])
            # set title, axis labels
            operation_title = "Setting" if type == "set" else "Removing"
            fig.update_layout(
                dict(
                    title="Overhead {:s} Timers  {:s}-{:s}".format(
                        operation_title, self.board, self.timer_version
                    ),
                    xaxis_title="Timer Count",
                    yaxis_title="Duration [us]",
                    showlegend=False,
                ),
            )
            # save file
            fig.write_html(
                "{}/{}.html".format(self.outdir, "{:s}_timer".format(type)),
                full_html=self.full_html,
                include_plotlyjs=self.plotlyjs,
            )

        df = parse(self.root)
        plot_overhead_set_remove("set", df)
        plot_overhead_set_remove("remove", df)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Produce timer benchmarks plots from xunit results"
    )

    parser.add_argument("input", help="xunit result file to parse")
    parser.add_argument("--outdir", help="output directory to write plots to")
    parser.add_argument(
        "--board",
        help="specify board",
        required=True,
    )
    parser.add_argument(
        "--for-ci",
        help="configure output for ci (this will exclude plotly.js from output files)",
        action="store_true",
    )

    args = parser.parse_args()

    args.outdir = (
        args.outdir
        if args.outdir
        else "{:s}/includes".format(args.input[: args.input.rfind("/")])
    )
    if not os.path.exists(args.outdir):
        os.makedirs(args.outdir)

    plotter = FigurePlotter(args.input, args.outdir, args.for_ci, args.board)
    plotter.plot_accuracy()
    plotter.plot_jitter()
    plotter.plot_set_remove_timer_from_list()

    # call sed to convert <br> tags to <br /> for XML
    subprocess.run(
        "find {:s} -type f ".format(args.outdir)
        + r"-execdir sed -i 's/<br>/<br \/>/g' {} \;",
        shell=True,
    )
