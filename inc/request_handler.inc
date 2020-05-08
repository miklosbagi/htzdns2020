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
  rsp_code=""; rsp_body=""
  # GET requests
  if [[ "$1" == "GET" ]]; then 
     rsp=`$curl -s -A "$_AGENT" -w "\\nHTTPCODE:%{http_code}\\n" "${headers[@]}" "$2$4"`

 # POST REQUESTS
  elif [[ "$1" == "POST" ]]; then
    echo "POST"

 # PUT REQUESTS
 elif [[ "$1" == "PUT" ]]; then
    echo "PUT"
 # Anything else is unsupported.
 else
    _SAY "Request method '$1' is not supported."
    return 1
  fi

  # separate response from reponse code
  rsp_body=`echo "$rsp"|$sed -E 's#^HTTPCODE:[0-9]+$##' |$sed '/^$/d'` || return 1
  rsp_code=`echo "$rsp"|$grep -E -o "^HTTPCODE:[0-9]+$" |$sed 's#^HTTPCODE:##'` || return 1

  # translate response code to something meaningful
  API_ERROR_TRANSLATOR $rsp_code

 # response code should be 200 for success.
  if [[ "$rsp_code" != "200" ]]; then return 1; fi
}

# PARAMS_BUILDER
# The role of this function is to construct params key value pairs as per data provided.
# Returns full params list as $params
function PARAMS_BUILDER {
  params=""
  # iterate through params, and add meaningful to the rest of params.
  for param in "$@"; do
     if [[ `echo $param |$sed -E 's#[A-z0-9]+=##'` != "" ]]; then params="$params&$param"; fi
  done
  # remove leading &
  params=`echo "?$params" |$sed 's#^&##'`
}

# HEADER_BUILDER
# Building an array so we don't get lost in the impossible '" quoting with curl later on.
function HEADERS_BUILDER {
  local -n arr="headers";
  for header in "$@"; do
      arr+=('-H' "$header")
  done
}
