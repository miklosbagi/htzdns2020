### htzdns/inc/data_maniuplators.inc
### Everything that forges & changes some data flowing through.

### PARSE_ZONES_JSON
### Converts JSON data from API_GET_ALL_ZONES request to csv.
### IN: $1 zones_data (string (long))
### OUT: establishes zones global variable, bool.
### Global dependencies: none
function PARSE_ZONES_JSON {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   zones=`$printf '%s\n' "$1" | $jq -r '.zones[] | [.id, .name, (.ttl|tostring)] | join(",")'` || return 1
   if [ -z "$zones" ]; then return 1; fi
}

### SPLIT_ZONES_ID_NAME
### Splits up zone data to individual variables for easier processing.
### IN: $1 zones_data (string (long))
### OUT: establishes zone_id, zone_name and zone_ttl global variables, bool.
### Global dependencies: none
function SPLIT_ZONES_ID_NAME {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   zone_id=`$printf -- "$1" |$cut -d, -f1` || return 1
   zone_name=`$printf -- "$1" |$cut -d, -f2` || return 1
   zone_ttl=`$printf -- "$1" |$cut -d, -f3` || return 1
   # we don't expect empty variables, return false in case there's any
   if [ -z "$zone_id" ] || [ -z "$zone_name" ] || [ -z "$zone_ttl" ]; then return 1; fi
}

### PARSE_RECORDS_JSON
### Converts JSON data from API_GET_ALL_RECORDS request to csv.
### IN: $1 records_data (string (long))
### OUT: establishes records global variable, bool.
### Global dependencies: none
function PARSE_RECORDS_JSON {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   # NOTE: Only A records are supported, filtering & joining with that in mind:
   records=`$printf '%s\n' "$1" | $jq -r '.records[] | select(.type == "A") | [.zone_id, .id, .name, .value, .type, .modified] | join(",")'` || return 1
   if [ -z "$records" ]; then return 1; fi
}

### SPLIT_RECORDS_ID_NAME_VALUE
### Splits up records data to individual variables for easier processing.
### IN: $1 zones_data (string (long))
### OUT: establishes record_zone_id, zone_id, record_name, record_value,
###      record_type, record_modified global variables, bool.
### Global dependencies: none
function SPLIT_RECORDS_ID_NAME_VALUE {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   record_zone_id=`$printf "$1" | $cut -d, -f1`
   record_id=`$printf "$1" | $cut -d, -f2`
   record_name=`$printf "$1" | $cut -d, -f3`
   record_value=`$printf "$1" | $cut -d, -f4`
   record_type=`$printf "$1" | $cut -d, -f5`
   record_modified=`$printf "$1" | $cut -d, -f6`
   # we don't expect empty variables - return false in case we have any.
   if [ -z "$record_zone_id" ] || [ -z "$record_id" ] || [ -z "$record_name" ] || [ -z "$record_value" ]; then return 1; fi
}

### FILTER_RECORDS_JSON
### Filters records that are specified in htz_host_exclude global configuration
### to be hidden (e.g. @, localhost, anything user does not want to configure).
### If user filters all records, it returns the original list, otherwise the new one.
### IN: none
### OUT: updated records global variable
### Global dependencies: htz_host_exclude, records
function FILTER_RECORDS_JSON {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  # Return with success should we have no exclusions defined
  if [ -z "$htz_host_exclude" ]; then return 1; fi
  # split list up at commas
  excludes=`$printf "$htz_host_exclude" | $sed 's#,##'`
  # iterate through, and establish new_records
  new_records="$records"
  for exclude in $excludes; do
      _VRB1 "Excluding: $exclude"
      # remove matches
      new_records=`$printf -- "$new_records" | $grep -v ",${exclude},"`
  done
  # user should not filter out all entries (records are kept as is)
  if [ -z "$new_records" ]; then _VRB1 "We ended up with an emtpy record set."; fi
  # otherwise, let's use new_records going forward.
  records="$new_records"
}

### READ_ZONE_CONFIG
### Reads host specific config file and reads configuration present.
### IN: $1 zone_id (string)
### OUT: establishes zone_config global variable, bool
### Global dependencies: host specific configuration file.
function READ_ZONE_CONFIG {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   # read up
   if [ ! -f "${0%/*}/htzdns-$(hostname).conf" ]; then return 1; fi
   c=`$sed -n "/^htz_update_zone_record_map=\"/,/^\"$/p" "${0%/*}/htzdns-$(hostname).conf"` || return 1
   # clean junk
   zone_config=`$printf "$c" |sed 's#^htz_update_zone_record_map=\"##' |$sed 's#"##' |$grep -v ^$` || return 1
   # grab specific zone if provided
   if [ ! -z "$1" ]; then zone_config=`$printf "$zone_config" |$grep "$1"`; fi
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
### IN: $1 record_id (string), $2 record_value (string), $3 record_type (string),
###     $4 record_name (string), $5 ttl (string)
### OUT: adds to global variable bulk_construct
### Global dependencies: bulk_construct (if exists, otherwise it is created).
function ADD_TO_BULK_UPDATES {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  JSON_BUILDER "id=$1" "value=$2" "type=$3" "name=$4" "zone_id=$5" "ttl=$6"
  bulk_construct="$bulk_construct${nl}${json_leaf}"
}

### FINALIZE_JSON_LIST
### Cleans up JSON data to be submittable. Adds {[]} wrappers, and gets rid of
### empty lines.
### IN: $1 top_node_name (string) (e.g. zones or records), $2 json object
### OUT: finalized_json global variable, bool
### Global dependencies: nl
function FINALIZE_JSON_LIST {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  # no empties, return false.
  if [ -z "$1" ] || [ -z "$2" ]; then return 1; fi
  json_data=`$printf "$2" | $sed '$ s/.$//'` || return 1
  finalized_json="{${nl}\"$1\": [${json_data}${nl}]${nl}}"
  VALIDATE_JSON "$finalized_json" || return 1
}

### MATCH_VALUE_RECORD_TYPE
### Updates value to match record type expectations.
### Note that currently A records are supported only, not much matching here just yet.
### IN: $1 record_type (string)
### OUT: establishes value global variable for record.
### Global dependencies: current_ip
function MATCH_VALUE_RECORD_TYPE {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
    value=""
    if [[ "$1" == "A" ]]; then
       value="$current_ip"
    else
       _ERR "Record type $record_type is not supported by this sctipt."
    fi
}
