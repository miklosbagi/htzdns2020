### htzdns/inc/data_validators.inc
### Everything that validates data to a specific standard.

### IS_ZONE_ID_IN_ZONE_CONFIG
### Confirms if $zone_id is present in $zone_config, returns false when that's
### not the case. (TODO: get better regex for IS_ZONE_ID_IN_ZONE_CONFIG)
### IN: $1 zone_id (string)
### OUT: bool
### Global dependencies: zone_config
function IS_ZONE_ID_IN_ZONE_CONFIG {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  res=`$printf "$zone_config" |$grep -v "^#" |$grep -q ",$1,"` || return 1
}

### IS_RECORD_IN_ZONE_CONFIG
### Confirms if record is in zone config (TODO: get better regex for IS_RECORD_IN_ZONE_CONFIG)
### IN: $1 record_id (string)
### OUT: bool
### Global dependencies: zone_config
function IS_RECORD_IN_ZONE_CONFIG {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  res=`$printf "$zone_config" |$grep -v "^#" |$grep -q ",$1$"` || return 1
}

### IS_VALID_IPV4
### Checks that an IPv4 address is in the correct format, and that it is a WAN
### address, not an internal or reserved one.
### Implements: https://en.wikipedia.org/wiki/Reserved_IP_addresses#IPv4
### (TODO: split IS_VALID_IPV4 REGEX into human interpretable chunks)
### IN: $1 current_ip (string)
### OUT: bool
### Global dependencies: none
function IS_VALID_IPV4 {
  _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  if [ -z "$1" ]; then return 1; fi
  p="(^0\.)|(^10\.)|(^100\.6[4-9]\.)|(^100\.[7-9]\d\.)|(^100\.1[0-1]\d\.)|(^100\.12[0-7]\.)|(^127\.)|(^169\.254\.)|(^172\.1[6-9]\.)|(^172\.2[0-9]\. )|(^172\.3[0-1]\.)|(^192\.0\.0\.)|(^192\.0\.2\.)|(^192\.88\.99\.)|(^192\.168\.)|(^198\.1[8-9]\.)|(^198\.51\.100\.)|(^203.0\.113\.)|(^22[4-9]\.)|(^23[0-9]\.)|(^24[0-9]\.)|(^25[0-4]\.)"
  $printf "$1" |$grep -Eq "$p" && return 1 || return 0
}

### IS_TTL_OK
### Option to warn user about TTL that's too high for a dynamic DNS setup.
### Treshold is 1 hour.
### IN: $1 zone_ttl (string)
### OUT: bool
### Global dependencies: none
function IS_TTL_LOW_ENOUGH {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   if [ "$1" -gt "3600" ]; then return 1; fi
}

### IS_MODIFIED_OUTSIDE_TTL
### If record has been updated outside of the TTL (a TTL time has passed since our last modification), return true
### (can go ahead with update)
### NOTE: expecting UTC time from the API, returning false should that not be the case.
### IN: $1 zone_ttl (string), $2 modification_time (string(date+time+timezone))
### OUT: bool, may exit to shell (UTC expected)
### Global dependencies: none
function IS_MODIFIED_OUTSIDE_TTL {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   # get timezone
   timezone=`$printf -- "$2" |$cut -d' ' -f3,4`
   # throw error when timezone is not UTC as expected.
   if [[ "$timezone" != "+0000 UTC" ]]; then _ERR "The API is not returning modif time in UTC, that's a fatal."; fi

   # get datetime, and focus ditch milliseconds
   zone_datetime=`$printf -- "$2" |$cut -d' ' -f1,2 |$sed -E 's#\.[0-9]+##'`
   # test date version
   date_os=""
   test_date=`$date -ud"$zone_datetime" > /dev/null 2>&1` || date_os="unix"
   # now get epoch so we can compare.
   if [[ "$date_os" != "unix" ]]; then
      # try linux date (note that DDWRT is not GNU date)
      zone_epoch=`$date -ud"$zone_datetime" +%s` || { _WRN "Cannot get EPOCH date for comparison for Linux date."; return 1; }
   else
      # try unix date (for MacOS)
      zone_epoch=`$date -j -u -f "%Y-%M-%d %T" "$zone_datetime" +%s` || { _WRN "Cannot get EPOCH date for comparison for Unix date."; return 1; }
   fi
   # get system UTC epoch
   curr_epoch=`TZ="UTC" $date +%s`
   _VRB1 "Epoch zone: $zone_epoch, Epoch local: $curr_epoch, TTL: $1"
   diff=$(( curr_epoch - zone_epoch ))
   if [ "$diff" -gt "$1" ]; then _VRB1 "<< OUT: yes"; return 0; else _VRB1 "<< OUT: NO"; return 1; fi
}

### DOES_VALUE_MATCH_HETZNER_VALUE
### Confirms if our value in hatzner is actually different from what update would include.
### This mitigates the scenario of pushing data to the API in case of a cache
### update (write) failure.
### IN: $1 record_value (string), $2 value (string)
### OUT: bool
### Global dependencies: none
function DOES_VALUE_MATCH_HETZNER_VALUE {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   # empty values are not accepted here
   if [ -z "$1" ] || [ -z "$2" ]; then return 1; fi
   # handle force updates here (#5)
   if [[ "$force" == "1" ]]; then
      _VRB1 "Force update is on, let's add this."
      _PRETEND "Forcing update."
      return 1
   fi
   # check for difference
   if [[ "$1" != "$2" ]]; then return 1; fi
}

### VALIDATE_JSON_DATA
### Function to avoid sending out malformed json to lower dependency on API json filtering.
### IN: none
### OUT: bool
### Global dependencies: finalized_json
function VALIDATE_JSON {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   r=`$printf -- "$finalized_json" | $jq .` 2>&1 && return 0 || return 1
}

### IS_UPDATE_EMPTY
### In case we have our locally constructed value matching exactly the one in Hetzner's data
### we have a scenario of empty updates - this function just confirms that.
### IN: @# bulk_construct (list of arguments)
### OUT: bool (our update is empty / not empty)
function IS_EMPTY {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   # if there's nothing, say it's empty.
   if [ -z "$1" ]; then return 0; fi
   # if all params are longer than 5 chars, we are not empty.
   if [[ `$printf -- "${@#}" |$wc -c` -gt "5" ]]; then return 1;
   # otherwise, we have some junk data, but we are empty.
   else _VRB1 "Empty, but that is: less than 5 chars in length: \"${@#}\""; return 0;
   fi
}
