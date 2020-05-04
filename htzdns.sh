#!/bin/bash

# Pull functions
if [ ! -d "${0%/*}/inc" ]; then echo "Missing ./inc/"; exit 1; fi
for inc in `find ${0%/*}/inc/ -name "htz_*.inc"`; do source "$inc" || { echo "Failed including $inc."; exit 1; }; done

# Map binary dependencies
B="curl jq"; MAP_BINS $B	|| _ERR "Failed mapping binaries: $B"

# Pull & validate configuration
LOAD_CONFIG 			|| _ERR "Failed loading configuration"
VALIDATE_CONFIG 		|| _ERR "Failed validating configuration"

# Check user IP
