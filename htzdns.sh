#!/bin/bash

# Pull functions
if [ ! -d "${0%/*}/inc" ]; then printf "Missing ./inc/\n"; exit 1; fi
for inc in `find ${0%/*}/inc -name "*.inc"`; do source "$inc" || { printf "Failed including $inc.\n"; exit 1; }; done

# Map binary dependencies
MAP_BINS "curl jq grep sed cut date find touch mkdir hostname printf wc" || _ERR "Failed mapping binaries: $B"

# Pull & validate configuration
LOAD_CONFIG 			|| _ERR "Failed loading configuration"
# Params handling, and reset params
PARAMS_PARSER "$@"; set --

VALIDATE_CONFIG 		|| _ERR "Failed validating configuration"

BUILDER				|| _ERR "Error building configuration"
# Map vendor scripts
MAP_VENDOR			|| _ERR "Vendor scripts are missing, did you really git clone --recurse-submodules this?"

# Tests (if needed)
RUN_TESTS                       && exit 0
TEST_COVERAGE                   && exit 0

# if the ip address from whatismyip matches ALL that is in the cache files, exit.
current_ip=`$whatismyip`	|| _ERR "Failed determining current IP address"
IS_VALID_IPV4 "$current_ip"	|| _ERR "IP $current_ip is not valid for A records"

# Load cache, with no error scenario: we will build in case we don't have it.
LOAD_CACHE
# Get invalid_cache=0/1 so we can act later on it.
VALIDATE_CACHE
# If this ip is the only one we have in the cache; if so, it's safe to exit (uness we want to build cache)
IS_IP_IN_CACHE "$current_ip"	&& { if [[ "$force" != "1" ]]; then exit 0; fi; }

# DO WE HAVE CACHE FOR ALL ITEMS WE HAVE CONFIG FOR?

# Otherwise, let's roll and see how we're doing at Hetzner.
API_GET_ALL_ZONES || _ERR "Failed getting zones data"
PARSE_ZONES_JSON "$rsp_body"

for z in $zones; do
    # split zone name and id
    SPLIT_ZONES_ID_NAME "$z"	   || _ERR "Failed splitting $z into name and id"
    READ_ZONE_CONFIG "$zone_id"	   || zone_config=""

    # if our zone id is not in the zone config, skip this.
    IS_ZONE_ID_IN_ZONE_CONFIG "$zone_id" || continue

    # warn about ttl if it's too high: user wants dns to update quickly using this script
    IS_TTL_LOW_ENOUGH "$zone_ttl"  || _WRN "TTL for $zone_name is too high ($zone_ttl) for DynDNS - please consider 1 hour (3600) or lower value."

    # get all the records for this zone
    API_GET_ALL_RECORDS "$zone_id" || _ERR "Failed getting records for zone $zone_name"
    PARSE_RECORDS_JSON "$rsp_body" || { _WRN "Failed parsing JSON for records for $zone_name, skipping."; continue; }
    FILTER_RECORDS_JSON "$records" || _WRN "Failed filtering exclusion list from records for $zone_name."
    oIFS=$IFS
    IFS=$($printf "\n\b")
    for r in $records; do
        SPLIT_RECORDS_ID_NAME_VALUE "$r"
	      # If record is not in zone config, skip this one.
        IS_RECORD_IN_ZONE_CONFIG "$record_id" || continue
        # if last modification is less than a TTL ago, don't touch anything.
        IS_MODIFIED_OUTSIDE_TTL "$zone_ttl" "$record_modified" || continue
	      # swtch values to match record type (use for extending beyond A record support)
	      MATCH_VALUE_RECORD_TYPE "$record_type"
        # add to cache updates in case we need to deal with this
        ADD_ITEM_TO_CACHE "$zone_id" "$record_id" "$value"
	      # does value match cached value - in which case, we can continue without pushing anything
	      DOES_VALUE_MATCH_HETZNER_VALUE "$record_value" "$value" && continue
	      # add to bulk update list
	      ADD_TO_BULK_UPDATES "$record_id" "$value" "$record_type" "$record_name" "$zone_id"
    done
    IFS=$oIFS
done

# if we have no items in the list, then we have nothing to do here.
IS_EMPTY "$bulk_construct" && { _VRB1 "Empty update, Hetzner has what we have. Updating caches."; WRITE_NEW_CACHE; exit 0; }
FINALIZE_JSON_LIST "records" "$bulk_construct"
BULK_UPDATE_RECORDS "$finalized_json" || { if [[ "$pretend" != "1" ]]; then _ERR "Failed updating records 😩"; fi; }
WRITE_NEW_CACHE || _WRN "Failed writing new data to $cache_file, we will be hitting the API every time until this is solved."

# NOTIFY WITH DETAILS

# ADD TO UPDATE JSON ONLY IN CASE OUR VALUE DIFFERS (failed to update cache scenario)
