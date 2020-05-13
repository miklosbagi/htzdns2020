### Request specific fucntions, anything to make our API calls fly.

# GENERAL REQUEST FUNCTION
# Parametering:
# $1: type
# $2: endpoint
# $3: headers (array)
# $4: params
# $5: body
# $6: optional debug flag (makes requests verbose)
function REQUEST {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   rsp_code=""; rsp_body=""; rsp=""
   # GET requests
   if [[ "$1" == "GET" ]]; then 
      rsp=`$curl -s -A "$_AGENT" -w "\\nHTTPCODE:%{http_code}\\n" "${headers[@]}" "$2$4"` 
   # PUT REQUESTS
   elif [[ "$1" == "PUT" ]]; then
        if [[ "$pretend" == "1" ]]; then
           _PRETEND "Would PUT the following JSON data to "$2:"\n$5\n"
	   rsp_code="200"
	   rsp_body="pretend"
 	   return 0
        else
           rsp=`$curl -s -A "$_AGENT" -X "PUT" -w "\\nHTTPCODE:%{http_code}\\n" "${headers[@]}" "$2" --data "$5"`
        fi
   # Anything else is unsupported.
   else
      _SAY "Request method '$1' is not supported."
      return 1
   fi

   # separate response from reponse code
   rsp_body=`echo "$rsp"|$sed -E 's#^HTTPCODE:[0-9]+$##' |$sed '/^$/d'` || return 1
   rsp_code=`echo "$rsp"|$grep -E -o "^HTTPCODE:[0-9]+$" |$sed 's#^HTTPCODE:##'` || return 1

   # translate response code to something meaningful
   API_ERROR_TRANSLATOR "$rsp_code" "$rsp_body" || _ERR "Response contains error: $err_desc"
   # response code should be 200 for success.
   if [[ "$rsp_code" != "200" ]]; then return 1; fi
}

# PARAMS_BUILDER
# The role of this function is to construct params key value pairs as per data provided.
# Returns full params list as $params
function PARAMS_BUILDER {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   params=""
   # iterate through params, and add meaningful to the rest of params.
   for param in "$@"; do
       _VRB2 "Param: $param"
       if [[ `echo $param |$cut -d= -f2` != "" ]]; then params="$params&$param"; fi
   done
   # remove leading &
   if [ ! -z "$params" ]; then params=`echo "?$params" |$sed 's#^&##'`; fi
}

# HEADER_BUILDER
# Building an array so we don't get lost in the impossible '" quoting with curl later on.
function HEADERS_BUILDER {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   local -n arr="headers";
   for header in "$@"; do
       arr+=('-H' "$header")
   done
}

# JSON BUILDER
# Builds JSON object out of any number of arguments (key=value) provided.
# NOTE Limitation: this is 1 level in depth only.
function JSON_BUILDER {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   # don't call without params.
   if [ -z "$1" ]; then return 1; fi
   params=""
   json_leaf="{"
   # iterate through params, and add meaningful to the rest of params.
   arg=0
   for param in "$@"; do
       (( arg=arg+1))
       param_name=`echo "$param" |cut -d= -f1`
       param_value=`echo "$param" |cut -d= -f2`
       # add this entry only in case we have value set
       if [ ! -z "$param_value" ]; then
          json_leaf="${json_leaf}${nl}   \"$param_name\": \"$param_value\","
       fi
   done
   # should we end up with no json object, return false.
   if [ -z "$json_leaf" ]; then return 1; fi
   # remove last comma
   json_leaf=`echo "$json_leaf" | $sed '$ s/.$//'`
   # close it up
   json_leaf="${json_leaf}${nl}},"
}

