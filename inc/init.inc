### htzdns init functions

# Version
_VERSION="0.1"
_GITURL="https://github.com/mb-0/htzdns"

# User agent
_AGENT="htzdns/${_VERSION} ${_GITURL}"

# Git submodules & vendor dependencies to be mapped:
_VENDOR_DEPENDENCIES="whatismyip/whatismyip.sh
"

# New line string
nl="
"

# binaru_locations
binloc="/bin /usr/bin /usr/local/bin /sbin /usr/sbin /opt/bin /jffs/bin /jffs/sbin /jffs/usr/sbin /jffs/usr/bin"

# simple out msg.
function _SAY { $echo -ne "$1\n"; }
# verbose messaging (needs -v turned on)
function _VRB { if [[ "$verbose" == "1" ]]; then _SAY "📢 $1"; fi; }
# communicate errors on all channels, then exit
function _ERR { _SAY "💥 ERROR: $1\n"; exit 1; }
# comminicate warnings, don't exit
function _WRN { _SAY "⚠️  WARNING: $1\n"; }
# pretend messages
function _PRETEND { _SAY "👻 PRETEND: $1\n"; }

# Find binary files in various locations (usually in PATH)
function FIND_BIN {
   _VRB ">> $BASH_SOURCE :: ${FUNCNAME} :: in: ()"
  for loc in $binloc; do
      if [ -d "$loc" ] && [ -f "$loc/$1" ]; then 
         eval "$1"="$loc/$1"; 
	 return 0
       fi
  done
  # fail in case we have not found the binary in all those directories...
  return 1
}

# map all binary dependencies
function MAP_BINS {
   _VRB ">> $BASH_SOURCE :: ${FUNCNAME} :: in: ()"
   for bin in ${1}; do 
       FIND_BIN "$bin" || _ERR "Cannot find $bin in PATH or at common locations."
   done
}

# map all vendor scripts
function MAP_VENDOR {
   _VRB ">> $BASH_SOURCE :: ${FUNCNAME} :: in: (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
  for v in $_VENDOR_DEPENDENCIES; do
      fullpath="${0%/*}/vendor/$v"
      if [ -f $fullpath ]; then
      	 # dig out script name without directories
         s="${fullpath##*/}"
	 # dig out script name without extension, so for example $whatismyip executes vendor/whatismyip/whatismyip.sh
         eval "${s%\.*}"="${fullpath}"
      else
         _ERR "Error, $v could not be mapped from $fullpath."
      fi
  done
}

# load configuration file(s)
function LOAD_CONFIG {
  _VRB ">> $BASH_SOURCE :: ${FUNCNAME} :: in: (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
  d="htzdns.conf htzdns-`$hostname`.conf"
  for cfg in $d; do 
      if [ -f "${0%/*}/$cfg" ]; then 
         source ${0%/*}/$cfg || _ERR "Failed loading config: $cfg"
	 cfok=y; 
      else
         _WRN "Could not find config: $cfg"
      fi
  done
  if [[ "$cfgok" != "" ]]; then _ERR "Failed loading configs ($d)."; fi
}

# validate key variables from config, and override with defaults should they not be present.
function VALIDATE_CONFIG {
  _VRB ">> $BASH_SOURCE :: ${FUNCNAME} :: in: (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
  need_builder=0
  if [ -z "$lt" ]; then lt="htzdns-`$hostname`"; fi
  if [ -z "$htz_api_key" ]; then _WRN "No API key was found, launching builder."; need_builder=1; fi
  if [ -z "$htz_cache_dir" ]; then _WRN "No htz_cache_dir value was found in htzdns.conf, defaulting to ${0%/*}/data"; htz_cache_dir="data"; fi
  # TODO: the record map in config could use better validation.
  if [ -z "$htz_update_zone_record_map" ]; then _WRN "No zone mapping config was found, launching builder"; need_builder=1; fi
}


