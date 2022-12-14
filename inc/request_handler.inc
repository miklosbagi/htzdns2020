### htzdns/inc/request_handler.inc
### Request specific fucntions, anything to make API calls fly.

### REQUEST
### General request function to deal with GET / PUT (and maybe post in future)
### Takes a set of arguments and identifies curl arguments based on values provided.
### Then executes the GET or PUT request with the appropriate data.
### Separates out response code and response body
### TODO: Cleanup REQUEST as this is way too haeavy.
### IN: $@ multiple arguments in any order
###        headers (string): a list of -H Header: header formatted args
###        endpoint (string): https://api.*
###        request params (string): optional for GET params
###        request body (string): optional request body (e.g. JSON)
###        reqtype (string): get/post/put
### OUT: global variables: rsp_body and rsp_code, bool.
### Global dependencies: none
function REQUEST {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   # add debug option
   v=""; if [[ $vrb == "2" ]]; then v="-v"; fi
   # identify params
   # ugly, but need to do something like this for headers anyway
   # to keep Bash 3.x support (MacOS Catalina at least)
   headers=""; endpoint=""; params=""; body=""; reqtype=""
   declare -a headers
   for arg in "$@"; do
     # in the  unlikely event we have an empty arg.
     if [ -z "$arg" ]; then continue
     # detect H= headers in args
     elif [[ `$printf -- "$arg" |$grep "^H="` != "" ]]; then nh=`$printf -- "$arg" |sed 's#^H=##'`; headers+=('-H' "$nh")
     # detect arg that look like an endpoint
     elif [[ `$printf -- "$arg" |$grep "^https://"` != "" ]]; then endpoint="$arg"
     # detect arg for request params
     elif [[ `$printf -- "$arg" |$grep "^?.*=.*"` != "" ]]; then params="$arg"
     # detect JSON body arg
     elif [[ `$printf -- "$arg" | $grep "^[\{\[]"` != "" ]]; then body="$arg"
     # request Type
     elif [[ `$printf -- "$arg" | $grep -i "^get\|post\|put$"` != "" ]]; then reqtype="$arg"
     # in all other cases, raise some questions for verbose, and continue to next item.
     else _VRB1 "What is this argument? \"$arg\""; continue;
     fi
   # remove param in all positiva cases we've had a match.
   set -- "$@" "$arg"
   done

   # do not continue should critical info be missing.
   if [ -z $headers ] || [ -z $endpoint ] || [ -z $reqtype ]; then _ERR "Request error."; fi
   # debug
   _VRB2 "REQUEST ARGS:\n   Type: $reqtype\n   Endpoint: $endpoint\n   Params: $params\n   Headers: ${headers[@]}\n   Body: $body"
   # Now onto requesting...
   rsp_code=""; rsp_body=""; rsp=""
   req_base="$curl $v -s -w '\\nHTTPCODE:%{http_code}\\n' ${endpoint}${params}"
   # GET requests
   if [[ "$reqtype" == "GET" ]]; then rsp=`$req_base -A "$_AGENT" "${headers[@]}"`
   # PUT REQUESTS
   elif [[ "$1" == "PUT" ]]; then
        if [ -z "$body" ]; then _ERR "Request body is empty."; fi
        if [[ "$pretend" == "1" ]]; then _PRETEND "Would PUT the following JSON data to "$2:"\n$5\n"; rsp_code="200"; rsp_body="pretend"; return 0; fi
        rsp=`$req_base -A "$_AGENT" "${headers[@]}" -X "PUT" --data "$body"`
   # Anything else is unsupported.
   else _SAY "Request method '$1' is not supported."; return 1
   fi

   # separate response from reponse code
   # we will need to get rid of the ' and empty lines, as well as escape double quoting "\"
   # note that printf '%s\n' "$rsp" is required for portability/escaping for TXT records, e.g. "value":"\"v=DKIM1;...
   #
   rsp_body=`$printf '%s\n' "$rsp"|$sed -E 's#^HTTPCODE:[0-9]+$##' |$sed "s/^'$//"g |$sed '/^$/d'` || return 1
   rsp_code=`$printf '%s\n' "$rsp"|$grep -E -o "^HTTPCODE:[0-9]+$" |$sed 's#^HTTPCODE:##'` || return 1

   # translate response code to something meaningful
   API_ERROR_TRANSLATOR "$rsp_code" "$rsp_body" || _ERR "Response contains error: $err_desc"
   # response code should be 200 for success.
   if [[ "$rsp_code" != "200" ]]; then return 1; fi
}

### PARAMS_BUILDER
### Concatenates params key value pairs into a single list to be used in REQUEST.
### for example: a=b and c=d will become "?a=b&c=d".
### IN: $@ list of params (string: key=value)
### OUT: $params global variable
### Global dependencies: none
function PARAMS_BUILDER {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   params=""
   # iterate through params, and add meaningful to the rest of params.
   for param in "$@"; do
       _VRB2 "Param: $param"
       if [[ `$printf -- $param |$cut -d= -f2` != "" ]]; then
          if [ -z "$params" ]; then params="$param"; else params="$params&$param"; fi
       fi
   done
   # remove leading &
   if [ ! -z "$params" ]; then params=`$printf -- "?$params" |$sed 's#^&##'`; fi
}

### JSON BUILDER
### Builds JSON object out of any number of arguments (key=value) provided.
### NOTE Limitation: this is 1 level in depth only.
### IN: $@ list of params (string: key=value)
### OUT: establishes $json_leaf global variable
### Global dependencies: none
function JSON_BUILDER {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   # don't call without params.
   if [ -z "$1" ]; then return 1; fi
   params=""
   json_leaf="{"
   # iterate through params, and add meaningful to the rest of params.
   arg=0
   for param in "$@"; do
       (( arg=arg+1 ))
       param_name=`$printf "$param" |cut -d= -f1`
       param_value=`$printf -- "$param" |cut -d= -f2`
       # add this entry only in case we have value set
       if [ ! -z "$param_value" ]; then
          json_leaf="${json_leaf}${nl}   \"$param_name\": \"$param_value\","
       fi
   done
   # should we end up with no json object, return false.
   if [ -z "$json_leaf" ]; then return 1; fi
   # remove last comma
   json_leaf=`$printf -- "$json_leaf" | $sed '$ s/.$//'`
   # close it up
   json_leaf="${json_leaf}${nl}},"
}
