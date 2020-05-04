### htzdns init functions

# simple out msg.
function _SAY { echo -ne "$1\n"; }
function _ERR { echo -ne "💥 ERROR: $1\n"; exit 1; }
function _WRN { echo -ne "⚠️  WARNING: $1\n"; }

function FIND_BIN {
  for loc in /bin /usr/bin /usr/local/bin /sbin /usr/sbin /opt/bin /jffs/bin /jffs/sbin /jffs/usr/sbin /jffs/usr/bin; do
      if [ -d "$loc" ]; then if [ -f "$loc/$1" ]; then eval "$1"="$loc/$1"; return; fi; fi
  done
  # fail in case we have not found the binary in all those directories...
  return 1
}

# map all binary dependencies
function MAP_BINS { for bin in ${1}; do FIND_BIN "$bin" || _ERR "Cannot find $bin in PATH or at common locations."; done; }

# load configuration file(s)
function LOAD_CONFIG {
  d="htzdns.conf htzdns-`hostname`.conf"
  for cfg in $d; do if [ -f "${0%/*}/$cfg" ]; then source ${0%/*}/$cfg || _ERR "Failed loading config: $cfg"; cfok=y; fi; done
  if [[ "$cfgok" != "" ]]; then _ERR "Failed loading configs ($d)."; fi
}

function VALIDATE_CONFIG {
  if [[ $lt == "" ]]; then lt="htzdns-`hostname`"; fi
  if [[ $htz_api_key == "" ]]; then _ERR "You wil need to provide an API key for the htz_api_key variable."; fi
}

