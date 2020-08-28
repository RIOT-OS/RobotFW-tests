#!/bin/bash
[ -z "$1" ] && {
  echo "USAGE: $0 <path/to/robot/results>"
  exit 1
}
BASEDIR=$1
BASEXML=$BASEDIR/robot.xml
METAXML=$BASEDIR/metadata.xml

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
python3 $SCRIPTPATH/env_parser.py -x -g -p -t -e --output=$METAXML

# write header to xml
echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" > $BASEXML
echo "<?xml-stylesheet type=\"text/xsl\" href=\"https://ci.riot-os.org/static/robot.xsl\"?>" >> $BASEXML
echo "<result name=\"RIOT HIL\">" >> $BASEXML
tail -n+2 $METAXML >> $BASEXML
echo "<boards>" >> $BASEXML

BOARDS=$(find ${BASEDIR} -mindepth 1 -maxdepth 1 -type d | sort)
for b in $BOARDS; do
  echo " . processing $b"
  BOARD=$(basename $b)
  echo " .. BOARD=$BOARD"
  echo "<board name=\"$BOARD\">" >> $BASEXML
  TESTSUITES=$(find $b -mindepth 1 -maxdepth 1 -type d)
  for t in $TESTSUITES; do
    SUITE=$(basename $t)
    echo " .. TESTSUITE=$SUITE"
    [ -f $t/xunit.xml ] && {
	echo " ... found xunit.xml"
	tail -n+2 $t/xunit.xml >> $BASEXML
    }
  done
  echo "</board>" >> $BASEXML
done
# write footer to xml
echo "</boards>" >> $BASEXML
echo "</result>" >> $BASEXML
