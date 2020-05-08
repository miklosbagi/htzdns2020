### Configuration Builder

function BUILDER {
   if [[ -z $rebuild && $need_builder == "0" ]]; then return; fi
   if [[ $rebuild != "1" ]]; then
      _SAY "A few items are missing from the configuration ($lt), we need to build them in order to continue."
      _SAY "This is ideally performed only once, but you can force this by setting the --rebuild flag to $0."
   else
      _SAY "Ok, let's rebuild configuration."
   fi
   _SAY "Configuration will be: $lt."
   BUILDER_APIKEY
   BUILDER_ZONES
   BUILD_CONFIG
}

function BUILDER_APIKEY {
   echo "🔑 API key set: $htz_api_key"
   if [[ "$htz_api_key" == "" ]]; then
      echo -ne "   Please go to https://dns.hetzner.com/settings/api-token, generate an API token, and provide it here:"
   else
      echo -ne "   Please provide a new API key, or hit ENTER to keep the current one [$htz_api_key]:"
   fi
   read -p " " r </dev/tty ; [ "$r" = "" ] || htz_api_key=$r;

   # keep asking while our test is failing
   while "$@"; do
     TC_API_APIKEY && break;
     echo "   ❌ The API key provided [$htz_api_key] is not accepted: (response: $rsp_code: $err_desc)."
     echo -ne "      Try again (or exit with [CTRL+C]:"
     read -p " " r </dev/tty ; [ "$r" = "" ] || htz_api_key=$r;
   done
   echo "   $r"
   echo
   #✅ Excellent, Hetzner API welcomes $htz_api_key (response: $rsp_code: $err_desc)"
}

function BUILDER_ZONES {
  echo "🔗 Checking mapping configuration..."
  API_GET_ALL_ZONES || _ERR "Failed getting zones data"
  PARSE_ZONES_JSON "$rsp_body"
  READ_ZONE_CONFIG "$zone_id" || zone_config=""
  for z in $zones; do
      # split zone name and id
      SPLIT_ZONES_ID_NAME "$z" || _ERR "Failed splitting $z into name and id"
      # get all the records for this zone
      API_GET_ALL_RECORDS "$zone_id" || _ERR "Failed getting records for zone $zone_name"
      PARSE_RECORDS_JSON "$rsp_body" || { _WRN "Failed parsing JSON for records for $zone_name, skipping."; continue; }
      FILTER_RECORDS_JSON "$records" || _WRN "Failed filtering exclusion list from records for $zone_name."
      echo
      echo "   ⚙️  $zone_name has the following configuration options:"

      # keep editing until config is considered complete
      while true; do
        n=0
        for r in $records; do
           (( n=n+1))
	   SPLIT_RECORDS_ID_NAME_VALUE "$r"
	   BUILDER_IS_RECORD_CONFIGURED "$record_id" && mark="✅" || mark=""
	   echo "    [$n] $record_name $mark"
        done
        echo
        echo "   Records marked with ✅ are currently configured."
        echo -ne "   Swith these on/off individually by typing the line number, or hit enter to continue [1-$n]:"
        read -p " " r </dev/tty ; [ "$r" = "" ] && break;
        BUILDER_SWITCH_RECORD_IN_CONFIG "$r"

	echo "Configuration updated, here's your new list:"
      done
  done
  if [ -z "$zone_config" ]; then _ERR "You have not configured any records to be updated. Exiting."; fi
}

function BUILD_CONFIG {
   echo
   echo "⌛ Building config..."
   config_template="### Automatically generated htzdns-`hostname`.conf (`date +%Y-%m-%d' '%H:%M:%S`):
   		    ### for manual modifications, take a peak in htzdns.conf, we have quite some docs there.
   	       	    htz_api_key=\"$htz_api_key\"
                    htz_update_zone_record_map=\"${nl}${zone_config}${nl}\"
   "
   config_out=`echo "$config_template" |sed -e 's#^[\t ]\+##'`

   BUILDER_CONFIG_DIFF "$config_out" && { echo -ne "ℹ️  There is no change in config, not writing anything 😊\n\n"; return; }

   echo "✅ Configuration is successfully constructed."
   echo
   echo "👁️  Please take a moment to eyeball the configuration below,"
   echo "   and confirm that ${0%/*}/htzdns-`hostname`.conf can be updated."
   echo
   echo "$config_out"
   echo
   echo -ne "Are we OK to save to ${0%/*}/htzdns-`hostname`.conf? (Y/n)"
   read -p " " r </dev/tty ; [ "$r" = "y" ] || { echo "Ok, not overwriting."; return; }
   echo "$config_out" > ${0%/*}/htzdns-`hostname`.conf || return 1
   echo
   echo "✅ Excellent, we are done here."
   echo "   Run ${0} with the -p or -v flags, and see how your records are updated 😊"
   echo
}

function BUILDER_CONFIG_DIFF {
   config_present="${0%/*}/htzdns-`hostname`.conf"
   # if we have a present config file, load it up - otherwise report difference (as missing).
   if [ -f "$config_present" ]; then config_current=`cat $config_present |grep -v "^#"`; else return 1; fi
   # and remove comments as previously done.
   config_compare=`echo "$1" |grep -v "^#"`
   if [[ "$config_current" != "$config_compare" ]]; then return 1; fi
}

### BUILDER_IS_RECORD_CONFIGURED
### identifies whether or not the current record is in the confguration
### takes a single record id as input, expects $zone_config to be available
### zone_config can be empty when this is called.
function BUILDER_IS_RECORD_CONFIGURED {
   # check if record matches
   if [[ `echo "$zone_config" | $grep "$1" | $cut -d, -f4` == "$1" ]]; then return; else return 1; fi
}

### BUILDER_SWITCH_RECORD_IN_CONFIG
### switches an individual record on/off in config
### takes line number, uses $records and $zones_config
function BUILDER_SWITCH_RECORD_IN_CONFIG {
    if [ -z $1 ]; then return 1; fi
    # grab the line number from the records list.
    m=`echo "$records" | $sed "${r}q;d"`
    # we only need the record_id for match
    m_id=`echo "$m" | $cut -d, -f2`

    # if we have this match in our config, remove it.
    if [[ `echo "$zone_config" |$grep "$m_id$"` != "" ]]; then
       zone_config=`echo "$zone_config" |$grep -v ",$m_id$"`
       # note that zone config can be allowed to be empty at this point.
    # otherwise, add it.
    else
       # if zone config is empty, no newline is needed, otherwise, yeah.
       if [ -z "$zone_config" ]; then is_nl=""; else is_nl="$nl"; fi
       m_name=`echo "$m" |$cut -d, -f3`
       # we cannot have an emtpy record name.
       if [ -z "$m_name" ]; then return 1; fi
       # construct
       zone_config="${zone_config}${is_nl}${zone_name},${zone_id},${m_name},${m_id}"
    fi
    echo "$zone_config"
}
