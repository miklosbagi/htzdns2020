#!/bin/bash

# Pull functions
if [ ! -d "${0%/*}/inc" ]; then echo "Missing ./inc/"; exit 1; fi
for inc in `find ${0%/*}/inc/ -name "*.inc"`; do source "$inc" || { echo "Failed including $inc."; exit 1; }; done

# Map binary dependencies
MAP_BINS "curl jq grep sed cut"	|| _ERR "Failed mapping binaries: $B"

# Params
PARAMS_PARSER "$1"		|| _ERR "An error occured while parsing params: $1"

# Pull & validate configuration
LOAD_CONFIG 			|| _ERR "Failed loading configuration"
VALIDATE_CONFIG 		|| _ERR "Failed validating configuration"
BUILDER				|| _ERR "Error building configuration"
# Map vendor scripts
MAP_VENDOR			|| _ERR "Vendor scripts are missing, did you really git clone --recurse-submodules this?"

# Tests (if needed)
RUN_TESTS                       && exit 0
TEST_COVERAGE                   && exit 0

# Check if we need to run:
# Compare external IP with IP logged and ping (allow 5 mins).

# get all zones (get zoneIDs)
# get all records for zoneid
# update record info with changed IP
# update record

API_GET_ALL_ZONES		|| _ERR "Failed getting zones ($rsp_code: $err_desc)"

#  "id": "a9697bbd35139cc5d23647b4c040cce4",

#for z in $zones; do
#    zone_id=`echo $z |cut -d, -f1`
#    zone_name=`echo $z |cut -d, -f2`
#    API_GET_ALL_RECORDS "$zone_id"
#    for r in $records; do
#        record_id=`echo $r |cut -d, -f1`
#	rname=`echo $r |cut -d, -f2`
#	value=`echo $r |cut -d, -f3`
#	echo "$rname:$value"
#    done
#done


#FETCH_DATA https://dns.hetzner.com/api/v1/zones 
#".total_entries"

#echo $rsp_code
#echo $rsp_body | jq '.'

# get zones list: jq -r '.zones[] | [.id, .name] | join(",")'


# Check user IP
#wan_ip=`$whatismyip`


