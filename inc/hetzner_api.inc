### API calls & error handling for Hetzner DNS

### Get All Zones (GET request)
### Possible query params:
###   $1 name:          string, Full name of a zone. Will return an array with one or no results
###   $2 page:          string, A page parameter specifies the page to fetch. The number of the first page is 1
###   $3 per_page:      string, Number of zones to be shown per page. Returns all by default
###   $4 search_name:   string, Partial name of a zone. Will return all zones that contain the searched string
function API_GET_ALL_ZONES {
  endpoint="https://dns.hetzner.com/api/v1/zones"
  local headers
  HEADERS_BUILDER "Auth-API-Token: $htz_api_key"
  PARAMS_BUILDER "name=$1" "page=$2" "per_page=$3" "search_name=$4"
  REQUEST "GET" "$endpoint" "$headers" "$params" || return 1
}

### Get all records (GET request)
### Possible query params:
###   $1 zone_id:       string, ID of zone
###   $2 page: 		string, A page parameter specifies the page to fetch. The number of the first page is 1
###   $3 per_page: 	string, Number of records to be shown per page. Returns all by default
function API_GET_ALL_RECORDS {
  endpoint="https://dns.hetzner.com/api/v1/records"
  local headers
  HEADERS_BUILDER "Auth-API-Token: $htz_api_key"
  PARAMS_BUILDER "zone_id=$1" "page=$2" "per_page=$3"
  REQUEST "GET" "$endpoint" "$headers" "$params" || return 1
}

### Catch Error and explain
### expects error code, returns description.
function API_ERROR_TRANSLATOR {
  err_desc=""
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
  return 1;
}