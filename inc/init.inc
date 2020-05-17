### htzdns/inc/init.inc
### Core variables and functions to initialize htzdns

### Version
_VERSION="0.3"

### URL on GitHub
_GITURL="https://github.com/mb-0/htzdns"

# User Agent to be used for curl calls
_AGENT="htzdns/${_VERSION} ${_GITURL}"

# Git submodules & vendor dependencies to be mapped from this location
# one per line, relative to script root.
_VENDOR_DEPENDENCIES="whatismyip/whatismyip.sh
"

# New line string to avoid a lot of \ns and multiline magic.
nl="
"

# Binary location (instead of relying on PATH)
binloc="/bin /usr/bin /usr/local/bin /sbin /usr/sbin /opt/bin /jffs/bin /jffs/sbin /jffs/usr/sbin /jffs/usr/bin"

# Simple message to the user.
function _SAY { $printf -- "$1\n"; }
# Verbose/debug messaging (needs -v or -vv depending on how loud we need to be)
function _VRB1 { if [[ $verbose == "1" ]] && [[ $vrb -gt 0 ]]; then _SAY "📢 $@"; fi; }
function _VRB2 { if [[ $verbose == "1" ]] && [[ $vrb -gt 1 ]]; then _SAY "📢📢 $@;"; fi; }
# Communicate errors on all channels, then exit
function _ERR { _SAY "💥 ERROR: $1"; exit 1; }
# Comminicate warnings, don't exit
function _WRN { _SAY "⚠️  WARNING: $1"; }
# Pretend messages
function _PRETEND { if [[ $pretend == "1" ]]; then _SAY "👻 PRETEND: $1"; fi; }

### FIND_BIN
### Aims to find a list of binaries passed and map up variables to their full path.
### IN: $1 binary_name (string)
### OUT: $binary_name global variable with PATH, bool
### Global dependencies: bin_loc variable
function FIND_BIN {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"
  for loc in $binloc; do
      if [ -d "$loc" ] && [ -f "$loc/$1" ]; then
         eval "$1"="$loc/$1";
	 return 0
       fi
  done
  # fail in case we have not found the binary in all those directories...
  return 1
}

### MAP_BINS
### Takes a list of binaries and kicks FIND_BIN to fetchs location for them.
### IN: $1 list of binaries (string)
### OUT: $binary_name global variable with PATH, bool
### Global dependencies: none
function MAP_BINS {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"
   for bin in ${1}; do
       FIND_BIN "$bin" || _ERR "Cannot find $bin in PATH or at common locations."
   done
}

### MAP_VENDOR
### Maps a list of vendor scripts and creates global variables with their full paths.
### IN: none
### OUT: $vendor_name global variable with PATH, bool
### Global dependencies: _VENDOR_DEPENDENCIES
function MAP_VENDOR {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
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

### LOAD_CONFIG
### Sources in main and host_specific configuration file in this order.
### This way local (host) can override global settings.
### IN: none
### OUT: bool (+all variables from configs are loaded)
### Global dependencies: none
function LOAD_CONFIG {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   d="htzdns.conf htzdns-`$hostname`.conf"
   for cfg in $d; do
       if [ -f "${0%/*}/$cfg" ]; then
          source ${0%/*}/$cfg || _ERR "Failed loading config: $cfg"
 	  cfok=y;
       fi
   done
   if [[ "$cfgok" != "" ]]; then _ERR "Failed loading configs ($d)."; fi
}

### VALIDATE_CONFIG
### Validate key variables from config, and override with defaults
### should they not be present.
### This way local (host) can override global settings.
### IN: none
### OUT: sets global variables: htz_api_key (optionally with test api key param)
###                             htz_cache_dir default
###      amends need_builder if needed
###      bool
### Global dependencies: need_builder (optional)
function VALIDATE_CONFIG {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   if [[ "$need_builder" == "1" ]]; then return; fi
   need_builder=0
   if [ -z "$lt" ]; then lt="htzdns-`$hostname`"; fi
   # if test api key is provided, we may want to use that here
   if [ ! -z "$test_api" ]; then htz_api_key="$test_api"; fi
   if [ -z "$htz_api_key" ]; then _WRN "No API key was found, launching builder."; need_builder=1; return; fi
   if [ -z "$htz_cache_dir" ]; then _WRN "No htz_cache_dir value was found in htzdns.conf, defaulting to ${0%/*}/data"; htz_cache_dir="data"; return; fi
   # TODO: the record map in config could use better validation.
   if [ -z "$htz_update_zone_record_map" ]; then _WRN "No zone mapping config was found, launching builder"; need_builder=1; return; fi
}
