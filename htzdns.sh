#!/bin/bash

# Pull functions
if [ ! -d "${0%/*}/inc" ]; then echo "Missing ./inc/"; exit 1; fi
for inc in `find ${0%/*}/inc/ -name "htz_*.inc"`; do source "$inc" || { echo "Failed including $inc."; exit 1; }; done

# Map binary dependencies
MAP_BINS "curl jq grep sed"	|| _ERR "Failed mapping binaries: $B"

# Params
PARAMS_PARSER "$1"		|| _ERR "An error occured while parsing params: $1"

# Pull & validate configuration
LOAD_CONFIG 			|| _ERR "Failed loading configuration"
VALIDATE_CONFIG 		|| _ERR "Failed validating configuration"
BUILDER				|| _ERR "Error building configuration"
RUN_TESTS			|| _ERR "One or more test(s) are failing"
# Map vendor scripts
MAP_VENDOR			|| _ERR "Vendor scripts are missing, did you really git clone --recurse-submodules this?"

#FETCH_DATA https://dns.hetzner.com/api/v1/zones 
#".total_entries"

#echo $rsp_code
#echo $rsp_body | jq '.'

# get zones list: jq -r '.zones[] | [.id, .name] | join(",")'


# Check user IP
#wan_ip=`$whatismyip`


