#!/usr/bin/env bash

# Bash script to start EnergyPlus-OBN simulation.
# Copyright (C) by Truong X. Nghiem (2016).

# Processing arguments code copied from http://stackoverflow.com/questions/192249/how-do-i-parse-command-line-arguments-in-bash
# Use -gt 1 to consume two arguments per pass in the loop (e.g. each
# argument has a corresponding value to go with it).
# Use -gt 0 to consume one or more arguments per pass in the loop (e.g.
# some arguments don't have a corresponding value to go with it such
# as in the --default example).

WEATHER_FILE="in.epw"
IDF_FILE="in.idf"
MQTTSERVERADDRESS=""
OBN_NODENAME=""
OBN_WORKSPACE=""
OBN_QUITIFSTOP=NO
NEED_HELP=NO
OBN_DIRECTORY=""

# Show help
function show_help {
  echo Bash script to run EnergyPlus model with openBuildNet
  echo Syntax:
  echo '  obneplus -n|--node <node name> [--workspace <workspace name>]'
  echo '           [-f|--file <IDF file>] [-w|--weather <weather file>]'
  echo '           [-m|--mqtt <MQTT broker address>]'
  echo '           [--quitifstop] [-h|--help]'
  echo '           [-d|--dir <working directory>]'
}

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -w|--weather)
    WEATHER_FILE="$2"
    shift 2
    ;;
    -f|--file)
    IDF_FILE="$2"
    shift 2
    ;;
    -m|--mqtt)
    MQTTSERVERADDRESS="$2"
    shift 2
    ;;
    -n|--node)
    OBN_NODENAME="$2"
    shift 2
    ;;
    --workspace)
    OBN_WORKSPACE="$2"
    shift 2
    ;;
    -d|--dir)
    OBN_DIRECTORY="$2"
    shift 2
    ;;
    -h|--help)
    NEED_HELP=YES
    shift 1
    ;;
    --quitifstop)
    OBN_QUITIFSTOP=YES
    shift 1
    ;;
    -*) echo "unknown option: $1" >&2; exit 1;;
    *)
            # not an option
    ;;
esac
done

if [[ "${NEED_HELP}" == "YES" ]]; then
  show_help
  exit 0
fi

if [[ "${OBN_NODENAME}" == "" ]]; then
  echo ERROR: node name must be provided.
  show_help
  exit 1
fi

# Change to the working directory if requested
if [[ "${OBN_DIRECTORY}" != "" ]]; then
  echo Change to working directory "${OBN_DIRECTORY}"
  cd "${OBN_DIRECTORY}"
fi

# Create the openbuildnet.cfg file
printf "mqtt %s\n%s %s\n" "$MQTTSERVERADDRESS" "$OBN_NODENAME" "$OBN_WORKSPACE" > openbuildnet.cfg

if [[ "${OBN_QUITIFSTOP}" == "YES" ]]; then
  echo quitIfOBNstops >> openbuildnet.cfg
fi

echo Run EnergyPlus-OBN with following settings:
echo IDF file: "${IDF_FILE}"
echo Weather file: "${WEATHER_FILE}"
echo MQTT server: "${MQTTSERVERADDRESS}"
echo Node name: "${OBN_NODENAME}"
echo Workspace: "${OBN_WORKSPACE}"
echo Quit if OBN stops: "${OBN_QUITIFSTOP}"

energyplus -w "${WEATHER_FILE}" "${IDF_FILE}"
