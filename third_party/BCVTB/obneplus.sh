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
EPLUS_OUTPUTFILE=""
OBN_VARIABLES=""

# Show help
function show_help {
  echo Bash script to run EnergyPlus model with openBuildNet
  echo Syntax:
  echo '  obneplus -n|--node <node name> [--workspace <workspace name>]'
  echo '           [-f|--file <IDF file>] [-w|--weather <weather file>]'
  echo '           [-v|--variables <variables file>]'
  echo '           [-m|--mqtt <MQTT broker address>]'
  echo '           [--quitifstop] [-h|--help]'
  echo '           [-d|--dir <working directory>]'
  echo '           [-o|--output <output file>]'
  echo ''
  echo 'If the working directory does not exist, it will be created.'
  echo 'Output file is the CSV file containing the values of the reported variables.'
  echo 'The output file path is relative to the working directory, and all intermediate'
  echo 'directories must already exist.'
}

# Remove old result files
function remove_old_files {
	rm -f  eplusout.end
	rm -f  eplusout.eso
	rm -f  eplusout.rdd
	rm -f  eplusout.edd
	rm -f  eplusout.mdd
	rm -f  eplusout.dbg
	rm -f  eplusout.eio
	rm -f  eplusout.err
	rm -f  eplusout.dxf
	rm -f  eplusout.csv
	rm -f  eplusout.tab
	rm -f  eplusout.txt
	rm -f  eplusmtr.csv
	rm -f  eplusmtr.tab
	rm -f  eplusmtr.txt
	rm -f  eplusout.sln
	rm -f  epluszsz.csv
	rm -f  epluszsz.tab
	rm -f  epluszsz.txt
	rm -f  eplusssz.csv
	rm -f  eplusssz.tab
	rm -f  eplusssz.txt
	rm -f  eplusout.mtr
	rm -f  eplusout.mtd
	rm -f  eplusout.cif
	rm -f  eplusout.bnd
	rm -f  eplusout.dbg
	rm -f  eplusout.sci
	rm -f  eplusout.cfp
	rm -f  eplusmap.csv
	rm -f  eplusmap.txt
	rm -f  eplusmap.tab
	rm -f  eplustbl.csv
	rm -f  eplustbl.txt
	rm -f  eplustbl.tab
	rm -f  eplustbl.htm
	rm -f  eplusout.log
	rm -f  eplusout.svg
	rm -f  eplusout.shd
	rm -f  eplusout.wrl
	rm -f  eplusoutscreen.csv
	rm -f  eplusout.delightin
	rm -f  eplusout.delightout
	rm -f  eplusout.delighteldmp
	rm -f  eplusout.delightdfdmp
	rm -f  eplusout.sparklog
	rm -f  in.imf
	rm -f  in.idf
	rm -f  out.idf
	rm -f  eplusout.inp
	rm -f  in.epw
	rm -f  eplusout.audit
	rm -f  eplusmtr.inp
	rm -f  expanded.idf
	rm -f  expandedidf.err
	rm -f  readvars.audit

	rm -f  eplusout.sql
	rm -f  sqlite.err
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
    -v|--variables)
    OBN_VARIABLES="$2"
    shift 2
    ;;
    -d|--dir)
    OBN_DIRECTORY="$2"
    shift 2
    ;;
	-o|--output)
	EPLUS_OUTPUTFILE="$2"
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
	echo Change to working directory: "${OBN_DIRECTORY}"
	if [ ! -d "$OBN_DIRECTORY" ]; then
		# Create the directory
		mkdir -p $OBN_DIRECTORY
	fi
	cd "${OBN_DIRECTORY}"
fi

# Remove old files
remove_old_files

# Copy variables.cfg file if requested
if [[ "${OBN_VARIABLES}" != "" ]]; then
	if [ -f "$OBN_VARIABLES" ]; then
		echo Copy variables list from file: $OBN_VARIABLES
		cp $OBN_VARIABLES ./variables.cfg
	fi
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

# Generate the output file if requested
if [[ "${EPLUS_OUTPUTFILE}" != "" ]]; then
	echo Generating output file: "${EPLUS_OUTPUTFILE}"
	
	echo eplusout.eso >eplusout.inp
	echo eplusout.csv >>eplusout.inp

	ReadVarsESO eplusout.inp unlimited
	if [ -f eplusout.csv ]; then
		mv -f eplusout.csv ${EPLUS_OUTPUTFILE}
	else
		echo Failed to create output CSV file!
	fi
fi

	
