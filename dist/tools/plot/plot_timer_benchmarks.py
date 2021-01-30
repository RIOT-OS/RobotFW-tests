import os
import subprocess
import argparse
from ast import literal_eval

import xml.etree.ElementTree as ET
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go


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

    def plot_accuracy(self, filename):
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
            "{}/{}.html".format(self.outdir, filename),
            full_html=self.full_html,
            include_plotlyjs=self.plotlyjs,
        )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Produce timer benchmarks plots from xunit results"
    )

    parser.add_argument("input", help="xunit result file to parse")
    parser.add_argument("--outdir", help="output directory to write plots to")
    parser.add_argument(
        "--board",
        help="specify board",
        # required=True,
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
    plotter.plot_accuracy("accuracy")

    # call sed to convert <br> tags to <br /> for XML
    subprocess.run(
        "find {:s} -type f ".format(args.outdir)
        + r"-execdir sed -i 's/<br>/<br \/>/g' {} \;",
        shell=True,
    )
