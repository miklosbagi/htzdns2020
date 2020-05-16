### Data validation

### IS_ZONE_ID_IN_ZONE_CONFIG
### Confirms if $zone_id is present in $zone_config, returns false when that's not the case.
### Takes one param: $1 zone_id
function IS_ZONE_ID_IN_ZONE_CONFIG {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  res=`$printf "$zone_config" |$grep -v "^#" |$grep -q ",$1,"` || return 1
}

### IS_RECORD_IN_ZONE_CONFIG
function IS_RECORD_IN_ZONE_CONFIG {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  res=`$printf "$zone_config" |$grep -v "^#" |$grep -q ",$1$"` || return 1
}

### IS_VALID_IPV4
### Checks an IPv4 address for formatting and returns false in case the input doesn't look like one.
### Implements: https://en.wikipedia.org/wiki/Reserved_IP_addresses#IPv4
### Takes $1 input: IP address.
function IS_VALID_IPV4 {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  if [ -z "$1" ]; then return 1; fi
  p="(^0\.)|(^10\.)|(^100\.6[4-9]\.)|(^100\.[7-9]\d\.)|(^100\.1[0-1]\d\.)|(^100\.12[0-7]\.)|(^127\.)|(^169\.254\.)|(^172\.1[6-9]\.)|(^172\.2[0-9]\. )|(^172\.3[0-1]\.)|(^192\.0\.0\.)|(^192\.0\.2\.)|(^192\.88\.99\.)|(^192\.168\.)|(^198\.1[8-9]\.)|(^198\.51\.100\.)|(^203.0\.113\.)|(^22[4-9]\.      )|(^23[0-9]\.)|(^24[0-9]\.)|(^25[0-4]\.)"
  $printf "$1" |grep -Eq "$p" && return 1 || return 0
}

### TODO: IS_VALID_IPV6
### zero compressed IPv6 addresses (section 2.2 of rfc5952)
### link-local IPv6 addresses with zone index (section 11 of rfc4007)
### IPv4-Embedded IPv6 Address (section 2 of rfc6052)
### IPv4-mapped IPv6 addresses (section 2.1 of rfc2765)
### IPv4-translated addresses (section 2.1 of rfc2765)
### based on https://gist.github.com/syzdek/6086792 - TODO: needs intense testing.
function IS_VALID_IPV6 {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   s="[0-9a-fA-F]{1,4}"
   # TEST: 1:2:3:4:5:6:7:8
   v6pattern="(${s}:){7,7}${s}|"
   # TEST: 1::                 1:2:3:4:5:6:7::
   v6pattern="${v6pattern}(${s}:){1,7}:|"
   # TEST: 1::8                1:2:3:4:5:6::8  1:2:3:4:5:6::8
   v6pattern="${v6pattern}(${s}:){1,6}:${s}|"
   # TEST: 1::7:8              1:2:3:4:5::7:8  1:2:3:4:5::8
   v6pattern="${v6pattern}(${s}:){1,5}(:${s}){1,2}|"
   # TEST: 1::6:7:8            1:2:3:4::6:7:8  1:2:3:4::8
   v6pattern="${v6pattern}(${s}:){1,4}(:${s}){1,3}|"
   # TEST: 1::5:6:7:8          1:2:3::5:6:7:8  1:2:3::8
   v6pattern="${v6pattern}(${s}:){1,3}(:${s}){1,4}|"
   # TEST: 1::4:5:6:7:8        1:2::4:5:6:7:8  1:2::8
   v6pattern="${v6pattern}(${s}:){1,2}(:${s}){1,5}|"
   # TEST: 1::3:4:5:6:7:8      1::3:4:5:6:7:8  1::8
   v6pattern="${v6pattern}${s}:((:${s}){1,6})|"
   # TEST: ::2:3:4:5:6:7:8     ::2:3:4:5:6:7:8 ::8             ::
   v6pattern="${v6pattern}:((:${s}){1,7}|:)|"
   # TEST: fe08::7:8%eth       fe08::7:8%1                                             (link-local IPv6addresses with zone index)
   v6pattern="${v6pattern}fe08:(:${s}){2,2}%[0-9a-zA-Z]{1,}|"
   # TEST: ::255.255.255.255   ::ffff:255.255.255.255  ::ffff:0:255.255.255.255        (IPv4-mappedIPv6 addresses and IPv4-translated addresses)
   v6pattern="${v6pattern}::(ffff(:0{1,4})?:)?${RE_IPV4}|"
   # TEST: 2001:db8:3:4::192.0.2.33  64:ff9b::192.0.2.33
   v6pattern="${v6pattern}(${s}:){1,4}:${RE_IPV4}"

   # let's kill grep now
   v=`$printf "$1" | grep -Eq "$v6pattern"` && return 1
}

### IS_TTL_OK
### Option to warn user about TTL that's too high for a dynamic DNS setup.
### Treshold is 1 hour.
function IS_TTL_LOW_ENOUGH {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   if [ "$1" -gt "3600" ]; then return 1; fi
}

### IS_MODIFIED_OUTSIDE_TTL
### If record has been updated outside of the TTL (a TTL time has passed since our last modification), return true
### (can go ahead with update)
### Takes modification date and TTL as params.
### NOTE: you may want to make sure your clock's ticking right for this to make sense, especially when using low values.
### NOTE: expecting UTC time from the API, returning false should that not be the case.
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
### This mitigates the scenario of pushing data to the API in case of a cache update (write) failure.
### Takes three params: $2 record_value $3 value.
function DOES_VALUE_MATCH_HETZNER_VALUE {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   # empty values are not accepted here
   if [ -z "$1" ] || [ -z "$2" ]; then return 1; fi
   if [[ "$1" != "$2" ]]; then return 1; fi
}

### VALIDATE_JSON_DATA
### Function to avoid sending out malformed json to lower dependency on API json filtering.
### Takes input: json data, returns true or false.
function VALIDATE_JSON {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   r=`$printf -- "$finalized_json" | $jq .` 2>&1 && return 0 || return 1
}

### IS_UPDATE_EMPTY
### In case we have our locally constructed value matching exactly the one in Hetzner's data
### we have a scenario of empty updates - this function just confirms that.
###
function IS_EMPTY {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   if [ -z "$1" ]; then return 0; fi
   if [[ `$printf -- "${@#}" |$wc -c` -gt "5" ]]; then
      _VRB1 "More than 5 chars in length, not empty.";
      return 1
   else
     _VRB1 "Empty, but that is: less than 5 chars in length: \"${@#}\""
   fi
}
