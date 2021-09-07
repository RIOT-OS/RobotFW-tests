#! /usr/bin/env python3
import sys
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
    "failures": 0,
    "skipped": 0,
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
            "skipped": False,
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

        elif test.find("status").get("status") == "SKIP":
            testcase["skipped"] = True
            testcase["skipped_text"] = test.find("status").text
            testsuite["skipped"] += 1

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
testsuite["failures"] = str(int(total_all.get("fail")))
testsuite["skipped"] = str(int(total_all.get("skip")))
testsuite["errors"] = str(len(root.find("errors")))
testsuite["testcases"] = testcases
testsuite["tests"] = str(len(testsuite["testcases"]))

# Parsing done, building the xunit output
newroot = ET.Element("testsuite")
newroot.set("name", testsuite["name"])
newroot.set("tests", testsuite["tests"])
newroot.set("errors", testsuite["errors"])
newroot.set("failures", testsuite["failures"])
newroot.set("skipped", testsuite["skipped"])
newroot.set("time", testsuite["time"])

for tc in testsuite["testcases"]:
    testcase = ET.SubElement(newroot, "testcase")
    testcase.set("classname", tc["classname"])
    testcase.set("name", tc["name"])
    testcase.set("time", tc["time"])

    if tc["skipped"]:
        skipped = testcase = ET.SubElement(testcase, "skipped")
        skipped.text = tc["skipped_text"]
    elif tc["failed"]:
        failure = ET.SubElement(testcase, "failure")
        failure.text = tc["failed_text"]

    if len(tc["records"]) > 0:
        properties = ET.SubElement(testcase, "properties")
        for r in tc["records"]:
            try:
                record = ET.SubElement(properties, "property")
                record.set("name", r["name"])
                record.set("value", r["value"])
            except KeyError:
                pass

newtree = ET.ElementTree(newroot)
newtree.write(args.output, encoding="UTF-8", xml_declaration=True)

# To catch failures in make we add failures to the exit code
sys.exit(int(total_all.get("fail")))
