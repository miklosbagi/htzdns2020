### htzdns/inc/hetzner_api.inc
### A minimalistic approach to Hetzner DNS API functions implementation.

### API_GET_ALL_ZONES (GET request)
### Returns paginated zones associated with the user. Limited to 100 zones per request.
### Inputs are for request params only.
### IN:  $1 name (string) Full name of a zone. Will return an array with one or no results.
###      $2 page (string) A page parameter specifies the page to fetch. The number
###                       of the first page is 1
###      $3 per_page (string) Number of zones to be shown per page. Returns all by default
###      $4 search_name (string) Partial name of a zone. Will return all zones
###                      that contain the searched string.
### OUT: bool
### Global dependencies: none
function API_GET_ALL_ZONES {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   endpoint="https://dns.hetzner.com/api/v1/zones"
   headers=("H"="Auth-API-Token: $htz_api_key")
   PARAMS_BUILDER "name=$1" "page=$2" "per_page=$3" "search_name=$4"
   _VRB2 "REQUEST PARAMS" "GET" "$endpoint" "$params" "$headers[@]"
   REQUEST "GET" "$endpoint" "$params" "$headers" || return 1
}

### API_GET_ALL_RECORDS (GET request)
### Returns all records associated with user.
### Inputs are for request params only.
### IN: $1 zone_id  (string) ID of zone
###     $2 page     (string) A page parameter specifies the page to fetch.
###                          The number of the first page is 1
###     $3 per_page (string) Number of records to be shown per page. Returns all by default
### OUT: bool
### Global dependencies: none
function API_GET_ALL_RECORDS {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   endpoint="https://dns.hetzner.com/api/v1/records"
   headers=("H"="Auth-API-Token: $htz_api_key" "H"="Content-Type: application/json")
   PARAMS_BUILDER "zone_id=$1" "page=$2" "per_page=$3"
   _VRB2 "REQUEST PARAMS" "GET" "$endpoint" "$headers" "$params"
   REQUEST "GET" "$endpoint" "$headers" "$params" || return 1
}

### BULK_UPDATE_RECORDS (PUT request)
### Updates several records at once.
### IN: $1 finalized_json (json data)
### OUT: bool
### Global dependencies: none
function BULK_UPDATE_RECORDS {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   endpoint="https://dns.hetzner.com/api/v1/records/bulk"
   headers=("H"="Auth-API-Token: $htz_api_key" "H"="Content-Type: application/json")
   _VRB2 "REQUEST PARAMS" "PUT" "$endpoint" "$headers" "$1"
   REQUEST "PUT" "$endpoint" "$headers" "" "$1" || return 1
}

### API_ERROR_TRANSLATOR
### Tries to spit out meaningful details for error codes or JSON error messages.
### IN: $1 rsp_code (string) response code from curl call (404, 500, etc).
###     $2 rsp_body (string(long)) response body with potential error message
### OUT: est### Global dependencies: none
### Global dependencies: none
function API_ERROR_TRANSLATOR {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   err_desc=""
   # check of errors in response body first
   e=`$printf -- "$2" |$grep -q -i "json not valid"` && { err_desc="API says we sent invalid JSON"; _VRB1 "<< $err_desc"; return 1; }
   e=`$printf -- "$2" |$grep -q -i "no api key found"` && { err_desc="API Key was not included in request"; _VRB1 "<< $err_desc"; return 1; }

   # check for errors in response codes
   case $1 in
   200 ) err_desc="Successful"; return ;;
   401 ) err_desc="Unauthorized" ;;
   403 ) err_desc="Forbidden" ;;
   404 ) err_desc="Not found" ;;
   406 ) err_desc="Not acceptable" ;;
   409 ) err_desc="Conflict" ;;
   422 ) err_desc="Unprocessable entry" ;;
   esac
   shift
   _VRB1 "<< Error found: $error_desc"; return 1;
}
