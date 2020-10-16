#! /usr/bin/env python3
import argparse
import xml.etree.ElementTree as ET
from datetime import datetime as DT


TIME_FORMAT = "%Y%m%d %H:%M:%S.%f"
XUNIT_OUT = "xunit.xml"

args_parser = argparse.ArgumentParser(
    description="Parse RobotFrameworks output.xml to xUnit format",
    formatter_class=argparse.ArgumentDefaultsHelpFormatter,
)
args_parser.add_argument(
    "--output",
    type=str,
    help="path to file where the output should be written to",
    default=XUNIT_OUT,
)
args_parser.add_argument(
    "input", type=str, help="path to a file in RobotFramework output format"
)
args = args_parser.parse_args()

tree = ET.parse(args.input)
root = tree.getroot()

suite = root.find("suite")
testsuite = {
    "name": suite.get("name"),
    "failures_non_critical": 0,
    "failures": 0,
    "errors": 0,
}

starttime = DT.strptime(suite.find("status").get("starttime"), TIME_FORMAT)
endtime = DT.strptime(suite.find("status").get("endtime"), TIME_FORMAT)
suitetime = str((endtime - starttime).total_seconds())
testsuite["time"] = suitetime

testcases = list()
child_suites = suite.findall("suite")
for suite in child_suites:
    for test in suite.findall("test"):
        testcase = {
            "classname": f"{testsuite['name']}.{suite.get('name')}",
            "name": test.get("name"),
            "failed": False,
            "failed_non_critical": False,
        }

        starttime = DT.strptime(test.find("status").get("starttime"), TIME_FORMAT)
        endtime = DT.strptime(test.find("status").get("endtime"), TIME_FORMAT)
        testtime = str((endtime - starttime).total_seconds())
        testcase["time"] = testtime

        # check if testcase failed
        if test.find("status").get("status") == "FAIL":
            testcase["failed"] = True
            testcase["failed_text"] = test.find("status").text
            testsuite["failures"] += 1

        # collect non-critical failures
        tags = [tag.text for tag in test.findall("tags/tag")]
        if "non-critical" in tags:
            testcase["failed_non_critical"] = True
            testcase["failed_non_critical_text"] = test.find("status").get("status")

        testcase["records"] = list()
        for record in test.findall(".//kw[@name='Record Property']"):
            r = dict()
            for e in record.iter("msg"):
                text = e.text
                if text.startswith("NAME:"):
                    r["name"] = text[len("NAME: ") :]
                elif text.startswith("VALUE:"):
                    r["value"] = text[len("VALUE: ") :]
            testcase["records"].append(r)
        testcases.append(testcase)

stats = root.find("statistics/total")
total_all = next(obj for obj in stats if obj.text == 'All Tests')
total_critical = next(obj for obj in stats if obj.text == 'Critical Tests')
testsuite["failures"] = total_critical.get("fail")
testsuite["failures_non_critical"] = str(int(total_all.get("fail")) - int(total_critical.get("fail")))
testsuite["tests"] = str(int(total_all.get("pass")) + int(total_all.get("fail")))
testsuite["errors"] = str(len(root.find("errors")))
testsuite["testcases"] = testcases
testsuite["tests"] = str(len(testsuite["testcases"]))

# Parsing done, building the xunit output
newroot = ET.Element("testsuite")
newroot.set("name", testsuite["name"])
newroot.set("tests", testsuite["tests"])
newroot.set("errors", testsuite["errors"])
newroot.set("failures", testsuite["failures"])
# FIXME: change "skipped" to really mean skipped test
newroot.set("skipped", testsuite["failures_non_critical"])
newroot.set("time", testsuite["time"])

for tc in testsuite["testcases"]:
    testcase = ET.SubElement(newroot, "testcase")
    testcase.set("classname", tc["classname"])
    testcase.set("name", tc["name"])
    testcase.set("time", tc["time"])

    if tc["failed_non_critical"]:
        # TODO: replace "skipped" with better label
        non_critical = testcase = ET.SubElement(testcase, "skipped")
        non_critical.text = tc["failed_non_critical_text"]
    elif tc["failed"]:
        failure = ET.SubElement(testcase, "failure")
        failure.text = tc["failed_text"]


    if len(tc["records"]) > 0:
        properties = ET.SubElement(testcase, "properties")
        for r in tc["records"]:
            record = ET.SubElement(properties, "property")
            record.set("name", r["name"])
            record.set("value", r["value"])

newtree = ET.ElementTree(newroot)
newtree.write(args.output, encoding="UTF-8", xml_declaration=True)
