# Deal with data

### Fetch Data
### Needs two inputs: headers and endpoint

function FETCH_DATA {
   rsp="";rsp_code="";rsp_body=""
   rsp=`$curl -sw '%{http_code}' -H "$1" "$2"` || return 1
   rsp_code=`echo "$rsp"|$grep -E -o "[ ]?[0-9]+$"` || return 1
   rsp_body=`echo "$rsp"|$sed -E 's#[ ]?[0-9]+$##' |$sed '/^$/d'` || return 1
}
