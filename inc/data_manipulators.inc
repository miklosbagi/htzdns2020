### Manipulate data - split strings, reorder, etc.

### PARSE_ZONES_JSON
### Takes a JSON Response body, expecting zones data from Hetzner,
### picks up id and name, orders them into comma separated: zone_id,zone_name
function PARSE_ZONES_JSON {
   zones=`echo "$1" | $jq -r '.zones[] | [.id, .name, .ttl] | join(",")'` || return 1
   if [ -z "$zones" ]; then return 1; fi
}

### SPLIT_ZONES_ID_NAME
### Takes an id,name input and returns zone_id and zone_name separately
function SPLIT_ZONES_ID_NAME {
   zone_id=`echo "$1" |$cut -d, -f1`
   zone_name=`echo "$1" |$cut -d, -f2`
   zone_ttl=`echo "$1" |$cut -d, -f3`
   # we don't expect empty variables, return false in case there's any
   if [ -z "$zone_id" ] || [ -z "$zone_name" ]; then return 1; fi
}

### PARSE_RECORDS_JSON
### Takes a JSON Response body, expecting records data from Hetzner,
### filters to A records, and picks up id, name and value only, returns them comma separated
function PARSE_RECORDS_JSON {
   # NOTE: Only A records are supported, filtering & joining with that in mind:
   records=`echo "$1" | $jq -r '.records[] | select(.type == "A") | [.zone_id, .id, .name, .value, .type, .modified] | join(",")'` || return 1
   if [ -z "$records" ]; then return 1; fi
}

### SPLIT_RECORDS_ID_NAME_VALUE
### Takes an id,name,value data and splits up to individual bits
function SPLIT_RECORDS_ID_NAME_VALUE {
   record_zone_id=`echo "$1" | $cut -d, -f1`
   record_id=`echo "$1" | $cut -d, -f2`
   record_name=`echo "$1" | $cut -d, -f3`
   record_value=`echo "$1" | $cut -d, -f4`
   record_type=`echo "$1" | $cut -d, -f5`
   record_modified=`echo "$1" | $cut -d, -f6`
   # we don't expect empty variables - return false in case we have any.
   if [ -z "$record_zone_id" ] || [ -z "$record_id" ] || [ -z "$record_name" ] || [ -z "$record_value" ]; then return 1; fi
}

### FILTER_RECORDS_JSON
### Takes records and removes all names that matches the list in $htz_host_exclude
### returns $record.
function FILTER_RECORDS_JSON {
  excludes=`echo "$htz_host_exclude" | $sed 's#,##'`
  for exclude in $excludes; do
      records=`echo "$records" | $grep -v ",${exclude},"`
  done
  if [ -z "$records" ]; then return 1; fi
}

### READ_ZONE_CONFIG
### reads host specific config file and reads configuration present.
### Takes one optional param: $1 zone_id
function READ_ZONE_CONFIG {
   # read up
   if [ ! -f "${0%/*}/htzdns-$(hostname).conf" ]; then return 1; fi
   c=`$sed -n "/^htz_update_zone_record_map=\"/,/^\"$/p" "${0%/*}/htzdns-$(hostname).conf"` || return 1
   # clean junk
   zone_config=`echo "$c" |sed 's#^htz_update_zone_record_map=\"##' |$sed 's#"##' |$grep -v ^$` || return 1
   # grab specific zone if provided
   if [ ! -z "$1" ]; then zone_config=`echo "$zone_config" |$grep "$1"`; fi
   # do not return empty
   if [ -z "$zone_config" ]; then return 1; fi
}

### ADD_TO_BULK_UPDATES
### Adds one entry to the BUIL_UPDATE_SKELETON json construct.
### Reference: https://dns.hetzner.com/api-docs/#operation/BulkUpdateRecords
### EXAMPLE ADDITION TO BULK_UPDATE_SKELETON
#{
#      "id": "mnsQmZmXXmWh5MpFeT67ZZ",
#      "value": "2a01:4f8:d0a:11f5::2",
#      "type": "AAAA",
#      "name": "www",
#      "zone_id": "oH7shFebR6nLPgTnmvNjM8"
#    },
# Takes a minimum of 5 max 6 params
function ADD_TO_BULK_UPDATES {
  JSON_BUILDER "id=$1" "value=$2" "type=$3" "name=$4" "zone_id=$5" "ttl=$6"
  bulk_construct="$bulk_construct${nl}${json_leaf}"
}

# Finalizes JSON LIST
# Takes two params:
#   $1 top node name
#   $2 json object
function FINALIZE_JSON_LIST {
  # no empties, return false.
  if [ -z "$1" ] || [ -z "$2" ]; then return 1; fi
  json_data=`echo "$2" | $sed '$ s/.$//'`
  finalized_json="{${nl}\"$1\": [${json_data}${nl}]${nl}}"
  VALIDATE_JSON "$finalized_json" || return 1
}

### MATCH_VALUE_RECORD_TYPE
### Updates value to match record type expectations
### Takes one param $1 record_type
function MATCH_VALUE_RECORD_TYPE {
    value=""
    if [[ "$1" == "A" ]]; then
       value="$current_ip"
    else
       _ERR "Record type $record_type is not supported by this sctipt."
    fi
}

