### htzdns/inc/cache_handler.inc
### Collection of all cache specific functions.

### LOAD_CACHE
### Sucks up cache data
### IN: none
### OUT: establishes htz_cache global variable. Returns bool.
### Global dependencies: htz_cache_dir
function LOAD_CACHE {
  _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 ":: in: (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  cache_file="${0%/*}/$htz_cache_dir/cache.data"
  VALIDATE_CACHE_FILESYSTEM
  # load lines that are formatted correctly only.
  # formatting is: zone_id,record_id,value.
  # not being too strict with value as we may decide to add text records later on.
  htz_cache=`$grep -i '[a-z0-9]\{22,22\},[a-z0-9]\{32,32\},[a-z0-9\.:]\{2,4\}' "${cache_file}"` || return 1
}

### VALIDATE_CACHE_FILESYSTEM
### Filesystem validation for cache directory and file.
### IN: none
### OUT: none (may exit to shell)
### Global dependencies: cache_file, htz_cache_dir
function VALIDATE_CACHE_FILESYSTEM {
  _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 ":: in: (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  # if we don't have a cache dir, attempt to create it
  if [ ! -d "${0%/*}/$htz_cache_dir" ]; then
     a=`$mkdir -p "${0%/*}/$htz_cache_dir"` || _ERR "Failed creating cache directory (${0%/*}/$htz_cache_dir). Please ensure write permissions      for   `whoami` user."
  fi
  if [ ! -f "${cache_file}" ]; then
     a=`$touch "${cache_file}"` || _ERR "Cannot write to cache data ${cache_file}"
  fi
}

### VALIDATE_CACHE
### Run a few tests on our cache data to figure if we can identify a rebuild
### requirement early on.
### IN: none
### OUT: bool
### Global dependencies: htz_cache
function VALIDATE_CACHE {
  _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 ":: in: (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  # invalid if empty
  if [ -z "$htz_cache" ]; then return 1; fi
  # total lines
  total_cache_lines=`$printf -- "$htz_cache\n" | $wc -l`
  # invalid if more than 50 lines
  if [[ "$total_cache_lines" -gt "50" ]]; then
     _WRN "More than 50 records to be updated? Unlikely."; return 1;
  fi
}

### IS_IP_IN_CACHE
### Attempts to read current cache and identify if we have all the entries
### having the curent IP.
### IN: $1 - current_ip, string.
### OUT: bool
### Global dependencies: htz_cache
function IS_IP_IN_CACHE {
  _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 ":: in: (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  if [ -z "$htz_cache" ]; then return 1; fi
  res=`$printf "$htz_cache" |$grep -v ",$1$"`
  if [ -z "$res" ]; then return 0; else return 1; fi
}

### ADD_ITEM_TO_CACHE
### Will add, or replace an existing item to/in cache with new value(s).
### IN: $1 zone_id (string), $2 record_id (string), $3 current_ip (string).
### OUT: updates global variable: htz_cache, returns bool.
### Global dependencies: htz_cache
function ADD_ITEM_TO_CACHE {
  _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 ":: in: (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  # no empty params
  if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]; then return 1; fi
  # check if our new cache has items, or this is a fresh build.
  if [ -z "$htz_cache_new" ]; then htz_cache_new="$htz_cache"; fi

  # check if we have this entry, and should replace the value
  if [[ `$printf "$htz_cache_new" |$grep "^${1},${2},"` != "" ]]; then
     htz_cache_new=`$printf "$htz_cache_new\n" | sed "s#^${1},${2},.*#${1},${2},${3}#"` || return 1
  else
     # or that we need a new line
     htz_cache_new=`$printf "${htz_cache_new}${nl}${1},${2},${3}\n"` || return 1
  fi
  # return false if our new cache data looks invalid
  VALIDATE_CACHE || return 1
}

### WRITE_NEW_CACHE
### Persists cache from memory to file, given pretend is not set.
### IN: none
### OUT: bool
### Global dependencies: htz_cache
function WRITE_NEW_CACHE {
  _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 ":: in: (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  # cleanup empty lines
  htz_cache_new=`$printf "$htz_cache_new" | $grep -v "^[[:space:]]*$"`
  if [[ "$pretend" != "1" ]]; then
     # write
     $printf "$htz_cache_new\n" > "$cache_file" && _VRB1 "<< Cache updated successfully" || _WRN "Error updating cache file $cache_file with data: $htz_cache_new"
  else
     _PRETEND "Would write the following data to cache file ${cache_file}:\n${htz_cache_new}\n"
  fi
}

### FLUSH_CACHE
### Function to wipe cache information when needed (e.g. config rebuild)
### IN: none
### OUT: bool (sucessful flush or error)
### Global dependencies: cache_file
function FLUSH_CACHE {
  _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 ":: in: (`for q in ${@#}; do $printf -- $q\)\(; done`)"
  if [ -f "$cache_file" ]; then $printf "" > "$cache_file" || return 1; fi
  _VRB1 "Flushed cache $cache_file"
}
