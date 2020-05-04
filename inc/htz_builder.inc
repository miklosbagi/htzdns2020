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
     htz_test_apikey && break;
     echo "   ❌ The API key provided [$htz_api_key] is not accepted: (response: $rsp_code: $err_desc)."
     echo -ne "      Try again (or exit with [CTRL+C]:"
     read -p " " r </dev/tty ; [ "$r" = "" ] || htz_api_key=$r;
   done
   echo "   $r"
   #✅ Excellent, Hetzner API welcomes $htz_api_key (response: $rsp_code: $err_desc)"
}

function BUILDER_ZONES {
   echo "zone"
}

function BUILD_CONFIG {
   echo
   echo "⌛ Building config..."
   config_template="### Automatically generated htzdns-`hostname`.conf (`date +%Y-%m-%d' '%H:%M:%S`):
   		    ### for manual modifications, take a peak in htzdns.conf, we have quite some docs there.
   	       	    htz_api_key=\"$htz_api_key\"
   "
   config_out=`echo "$config_template" |sed -e 's#^[\t ]\+##'`

   CONFIG_DIFF "$config_out" && { echo -ne "ℹ️  There is no change in config, not writing anything \o/\n"; return; }

   echo "✅ The following configuration will be written into ${0%/*}/htzdns-`hostname`.conf:"
   echo "$config_out"
   echo
   echo -ne "Are we OK to proceed and (over)write ${0%/*}/htzdns-`hostname`.conf with above data? [y|n]"
   read -p " " r </dev/tty ; [ "$r" = "y" ] || { echo "Ok, not overwriting."; return; }
   echo "$config_out" > ${0%/*}/htzdns-`hostname`.conf || return 1
}

function CONFIG_DIFF {
   config_present="${0%/*}/htzdns-`hostname`.conf"
   # if we have a present config file, load it up - otherwise report difference (as missing).
   if [ -f "$config_present" ]; then config_current=`cat $config_present |grep -v "^#"`; else return 1; fi
   # and remove comments as previously done.
   config_compare=`echo "$1" |grep -v "^#"`
   if [[ "$config_current" != "$config_compare" ]]; then return 1; fi
#tnkuLZ54GwMbkU0bOEj7nQhxiKwrDUCo
}
