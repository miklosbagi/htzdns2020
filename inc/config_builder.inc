### htzdns/inc/config_builder.inc
### Everything required to interatcively build configuration files for this host.

### BUILDER
### Initializes build, kicks off all builder functions as necessary
### IN: none
### OUT: none
### Global dependencies: rebuild
function BUILDER {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   if [[ -z $rebuild && $need_builder == "0" ]]; then return; fi
   if [[ $rebuild != "1" ]]; then
      _SAY "A few items are missing from the config ($lt), we need to build them in order to continue."
      _SAY "This is ideally performed only once, but you can force this by setting the --rebuild flag to $0.\n"
   else
      _SAY "Ok, let's rebuild configuration.\n"
   fi
   _SAY "Configuration will be: $lt.conf"
   BUILDER_APIKEY
   BUILDER_ZONES
   BUILD_CONFIG
}

### BUILDER_APIKEY
### Builder for getting, testing and adding Hetzner API key to configuration.
### IN: none
### OUT: establishes htz_api_key global variable.
### Global dependencies: htz_api_key
function BUILDER_APIKEY {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   _SAY "🔑 API key set: $htz_api_key"
   if [[ "$htz_api_key" == "" ]]; then
      $printf "   Please go to https://dns.hetzner.com/settings/api-token, generate an API token, and provide it here:"
   else
      $printf "   Please provide a new API key, or hit ENTER to keep the current one [$htz_api_key]:"
   fi
   read -p " " r </dev/tty ; [ "$r" = "" ] || htz_api_key=$r;

   # keep asking while our test is failing
   while "$@"; do
     TC_API_APIKEY && break;
     _SAY "   ❌ The API key provided [$htz_api_key] is not accepted: (response: $rsp_code: $err_desc)."
     $printf "      Try again (or exit with [CTRL+C]:"
     read -p " " r </dev/tty ; [ "$r" = "" ] || htz_api_key=$r;
   done
   _SAY "   $r"
}

### BUILDER_ZONES
### Builds zone and record specific configuration.
### Fetches all zones from API with all zone specific records, and asks user
### to select & mark the ones to be updated by htzdns.
### IN: none
### OUT: establishes zone_config global variable, bool and may exit to shell.
### Global dependencies: none
function BUILDER_ZONES {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   _SAY "🔗 Checking mapping configuration..."
   API_GET_ALL_ZONES || _ERR "Failed getting zones data"
   PARSE_ZONES_JSON "$rsp_body"
   for z in $zones; do
       # split zone name and id
       SPLIT_ZONES_ID_NAME "$z" || _ERR "Failed splitting $z into name and id"
       READ_ZONE_CONFIG "$zone_id" || zone_config=""

       # get all the records for this zone
       API_GET_ALL_RECORDS "$zone_id" || _ERR "Failed getting records for zone $zone_name"
       PARSE_RECORDS_JSON "$rsp_body" || { _WRN "Failed parsing JSON for records for $zone_name, skipping."; continue; }
       FILTER_RECORDS_JSON "$records" || _WRN "Failed filtering exclusion list from records for $zone_name."
       $printf "\n   ⚙️  $zone_name has the following configuration options:\n"

       # keep editing until config is considered complete
       while true; do
         n=0
         oIFS=$IFS
      	 IFS=$($printf "\n\b")
         for r in $records; do
             (( n=n+1))
	           SPLIT_RECORDS_ID_NAME_VALUE "$r"
	           BUILDER_IS_RECORD_CONFIGURED "$record_id" && mark="✅" || mark=""
	           _SAY "    [$n] $record_name $mark"
         done
	       IFS=$oIFS
         _SAY "\n   Records marked with ✅ are currently configured.\n   Swith these on/off individually by typing the line number, or hit enter to continue [1-$n]:"
         read -p " " r </dev/tty ; [ "$r" = "" ] && break;
         BUILDER_SWITCH_RECORD_IN_CONFIG "$r"
	       _SAY "Configuration updated, here's your new list:"
       done
       ADD_TO_ALL_ZONES_CONFIG "$zone_config"
  done
  if [ -z "$all_zones_config" ]; then _ERR "You have not configured any records to be updated. Exiting."; fi
}

### BUILD_CONFIG
### Fills configuration template and compares if new configuration is different
### from the one we have saved for this host.
### Flushes cache in case new config is written, or returs w/o touching config.
### IN: none
### OUT: file (htzdns-hostname.conf), bool, may exit to shell.
### Global dependencies: none
function BUILD_CONFIG {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   _SAY "\n⌛ Building config..."
   config_template="### Automatically generated htzdns-`$hostname`.conf (`$date +%Y-%m-%d' '%H:%M:%S`):
### for manual modifications, take a peak in htzdns.conf, we have quite some docs there.
htz_api_key=\"$htz_api_key\"
htz_update_zone_record_map=\"${nl}${all_zones_config}${nl}\"
"
   config_out=`$printf "$config_template\n" |$sed -e 's#^[\t ]\+##'`

   BUILDER_CONFIG_DIFF "$config_out" && { $printf "ℹ️  There is no change in config, not writing anything 😊\n\n"; return; }

   _SAY "✅ Configuration is successfully constructed.\n"
   _SAY "👁️  Please take a moment to eyeball the configuration below,"
   _SAY "   and confirm that ${0%/*}/htzdns-`$hostname`.conf can be updated.\n"
   _SAY "$config_out\n"

   $printf "Are we OK to save to ${0%/*}/htzdns-`$hostname`.conf? (Y/n)"
   read -p " " r </dev/tty ; [ "$r" = "y" ] || { _SAY "Ok, not overwriting."; return; }
   $printf "$config_out\n" > ${0%/*}/htzdns-`$hostname`.conf || return 1

   # Need to load cache and flush it, otherwise we may not update the newly configured records (#1)
   LOAD_CACHE
   FLUSH_CACHE || _WRN "Tried flushing cache file, but failed. Please delete $cache_file manually."

   _SAY "\n✅ Excellent, configuration saved, cache flushed. We are done here."
   _SAY "   Running ${0} with pretend to see what would be done. 😊\n"
   pretend="1"
}

### BUILDER_CONFIG_DIFF
### Reads up host specific configuration from file, and compares contents to the
### variable BUILD_CONFIG function put together.
### IN: $1 config_out (string (long))
### OUT: bool
### Global dependencies: none
function BUILDER_CONFIG_DIFF {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   config_present="${0%/*}/htzdns-`hostname`.conf"
   # if we have a present config file, load it up - otherwise report difference (as missing).
   if [ -f "$config_present" ]; then config_current=`$grep -v "^#" $config_present`; else return 1; fi
   # and remove comments as previously done.
   config_compare=`$printf "$1" |grep -v "^#"`
   if [[ "$config_current" != "$config_compare" ]]; then return 1; fi
}

### BUILDER_IS_RECORD_CONFIGURED
### identifies whether or not the current record is in the confguration
### zone_config can be empty when this is called.
### IN: $1 record_id (string)
### OUT: bool
### Global dependencies: zone_config
function BUILDER_IS_RECORD_CONFIGURED {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   # check if record matches
   if [[ `$printf "$zone_config" | $grep "$1" | $cut -d, -f4` == "$1" ]]; then return
   else return 1
   fi
}

### BUILDER_SWITCH_RECORD_IN_CONFIG
### switches an individual record on/off in config
### IN: $1 (line_number (r))
### OUT: updates zone_config, bool
### Global dependencies: records (from loop), zone_id (from loop),
###                      zone_name (from loop), zone_config, nl
function BUILDER_SWITCH_RECORD_IN_CONFIG {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   if [ -z $1 ]; then return 1; fi
   # grab the line number from the records list.
   m=`$printf "$records\n" | $sed "${r}q;d"`
   # we only need the record_id for match
   m_id=`$printf "$m" | $cut -d, -f2`

   # if we have this match in our config, remove it.
   if [[ `$printf "$zone_config" |$grep "$m_id$"` != "" ]]; then
      zone_config=`$printf "$zone_config" |$grep -v ",$m_id$"`
      # note that zone config can be allowed to be empty at this point.
      # otherwise, add it.
   else
      # if zone config is empty, no newline is needed, otherwise, yeah.
      if [ -z "$zone_config" ]; then is_nl=""; else is_nl="$nl"; fi
      m_name=`$printf "$m" |$cut -d, -f3`
      # we cannot have an emtpy record name.
      if [ -z "$m_name" ]; then return 1; fi
      # construct
      zone_config="${zone_config}${is_nl}${zone_name},${zone_id},${m_name},${m_id}"
   fi
}

### ADD_TO_ALL_ZONES_CONFIG
### Creates a consolidated zones config, using all individual zone_configs.
### IN: $1 (zone_config)
### OUT: updates all_zone_config, bool
### Global dependencies: zone_config
function ADD_TO_ALL_ZONES_CONFIG {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   if [ -z "$1" ]; then _VRB1 "<< zone_config variable empty"; return 1; fi
   # if we don't have all_zones_config, create it with this entry.
   if [ -z "$all_zones_config" ]; then
      all_zones_config="$zone_config"
   else
   # otherwise, add to existing.
      all_zones_config="${all_zones_config}${nl}${zone_config}"
   fi
}
