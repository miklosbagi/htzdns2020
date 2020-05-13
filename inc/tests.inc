###
### Functional and unit test mix
### 


function RUN_TESTS {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   if [[ "$runtests" != "1" ]]; then return 1; fi
   echo "Running all tests:"
   echo "  HTZ Api Tests:"
   TEST_API_ALL
   return 0
}

function TEST_COVERAGE {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   if [[ "$coverage" != "1" ]]; then return 1; fi
   echo "Test coverage report for HTZDNS:"
   echo
   echo "  Unit tests:"
   GET_UNIT_TESTS
   echo
   echo "---------------------------------------------------------------------------------------------------------"
   echo " Unit tests: $unit_tested of $unit_total total ($unit_pdone%) covered. $unit_untested needs coverage."
   echo "---------------------------------------------------------------------------------------------------------"
   echo
   echo "  Functional tests:"
   GET_FUNCTIONAL_TESTS
   echo
   echo "---------------------------------------------------------------------------------------------------------"
   echo " Functional tests: $tc_tested Test Cases implemented."
   echo "---------------------------------------------------------------------------------------------------------"
   total=0
   (( total = tc_tested + unit_tested ))
   echo
   echo "---------------------------------------------------------------------------------------------------------"
   echo " Total: $total tests"
   echo "   Unit tests: $unit_tested of $unit_total total ($unit_pdone%) covered. $unit_untested needs coverage."
   echo "   Functional tests: $tc_tested Test Cases implemented."
   echo "---------------------------------------------------------------------------------------------------------"

   echo "Note that vendor scripts (besides availability) are not tested here."
   echo
   return 0
}

function GET_UNIT_TESTS {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   unit_tested=0; unit_untested=0
   units=`declare -F |sed 's#declare -f ##' |grep -v "^TEST_" |grep -v "^TC_" |grep -v "GET_.*_TESTS" |grep -v "RUN_.*_TESTS"`
   for u in $units; do
       t=`declare -f TEST_$u`
       if [ -n "$t" ]; then
          echo "    ✅ $u covered by TEST_$u"
          (( unit_tested = tested + 1))
       else
          echo "    ❌ missing TEST_$u function for coverage."
          (( unit_untested = unit_untested + 1 ))
       fi
   done
   (( unit_total = unit_tested + unit_untested ))
   (( unit_pdone  = 100/unit_total*unit_tested ))
}

function GET_FUNCTIONAL_TESTS {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   # todo: would be nice to have some requirements management to compare TCs to.
   tc_tested=0
   tcs=`declare -F |sed 's#declare -f ##' |grep "^TC_"`
   for tc in $tcs; do
     echo "    ✅ Test Case: $tc"
     (( tc_tested = tc_tested +1 ))
   done
}

###
### Init tests
###

function TEST_INIT_ALL {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   TEST_INIT_CONFIGS 
}

###
### Hetzner API tests
###

function TEST_API_ALL {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   TC_API_APIKEY || echo "  ❌ API Key $htz_api_key is failing" && echo "  $r"
}

# functional tests
function TC_API_APIKEY {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   # given I have an API key, I can fire a to hetzner and get some results back.
   endpoint="https://dns.hetzner.com/api/v1/zones"
   local headers
   HEADERS_BUILDER "Auth-API-Token: $htz_api_key"
   _VRB2 "GET" "$endpoint" "$headers" "$params"
   REQUEST "GET" "$endpoint" "$headers" "$params" || return 1
   r="✅ Excellent, Hetzner API welcomes $htz_api_key (response: $rsp_code: $err_desc)"
}

function TEST_API_GET_ALL_ZONES {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   echo "1"
}

### TC_IPV6_WHATISMYIP
### Mocks whatismyip response to be an ipv6 address

### TC_EMPTY_DATA_FILE

### TC_CORRUPT_DATA_FILE(S)

### TC_CONFIG_SYNTAX_ERROR

### TC_HTZAPI_ERROR_ZONES
### TC_HTZAPI_ERROR_RECORDS
### TC_HTZAPI_ERROR_UPDATE

### TC_BINARY_CORRUPT

### TC_TEST_CURL
### TC_TEST_CUT
### TC_TEST_GREP
### TC_TEST_SED
### TC_TEST_JQ


# todo:
# TEST_API_ERROR_TRANSLATOR
# TEST_API_GET_ALL_RECORDS
# TEST_API_GET_ALL_ZONES
