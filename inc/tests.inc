###
### Functional and unit test mix
###

### RUN_TESTS
### Decides whether or not tests should be ran.
### IN: none
### OUT: bool
### Global dependencies: runtests global variable (optional)
function RUN_TESTS {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   if [[ "$runtests" != "1" ]]; then return 1; fi
   _SAY "Running all tests:\n"
   _SAY "  HTZ Api Tests:"
   TEST_API_ALL
   return 0
}

### TEST_COVERAGE
### Calculates test coverage based on unit and functional tests written.
### IN: none
### OUT: bool
### Global dependencies: coverage global variable (optional)
function TEST_COVERAGE {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   if [[ "$coverage" != "1" ]]; then return 1; fi
   _SAY "Test coverage report for HTZDNS:\n"
   _SAY "  Unit tests:"
   GET_UNIT_TESTS
   _SAY "\n---------------------------------------------------------------------------------------------------------"
   _SAY " Unit tests: $unit_tested of $unit_total total ($unit_pdone%) covered. $unit_untested needs coverage."
   _SAY "---------------------------------------------------------------------------------------------------------\n"
   _SAY "  Functional tests:"
   GET_FUNCTIONAL_TESTS
   _SAY "\n---------------------------------------------------------------------------------------------------------"
   _SAY " Functional tests: $tc_tested Test Cases implemented."
   _SAY "---------------------------------------------------------------------------------------------------------"
   total=0
   (( total = tc_tested + unit_tested ))
   _SAY "\n---------------------------------------------------------------------------------------------------------"
   _SAY " Total: $total tests"
   _SAY "   Unit tests: $unit_tested of $unit_total total ($unit_pdone%) covered. $unit_untested needs coverage."
   _SAY "   Functional tests: $tc_tested Test Cases implemented."
   _SAY "---------------------------------------------------------------------------------------------------------"

   _SAY "Note that vendor scripts (besides availability) are not tested here.\n"
   return 0
}

### GET_UNIT_TESTS
### Simple static analyzer for htzdns functions, identifies if FUNCTION has a
### coresponding TEST_FUNCTION pair where unit test is written.
### Note: having a test pair does not guarantee that it's not a return true.
### IN: none
### OUT: none
### Global dependencies: none
function GET_UNIT_TESTS {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   unit_tested=0; unit_untested=0
   units=`declare -F |sed 's#declare -f ##' |grep -v "^TEST_" |grep -v "^TC_" |grep -v "GET_.*_TESTS" |grep -v "RUN_.*_TESTS"`
   for u in $units; do
       t=`declare -f TEST_$u`
       if [ -n "$t" ]; then
          _SAY "    ✅ $u covered by TEST_$u"
          (( unit_tested = tested + 1))
       else
          _SAY "    ❌ missing TEST_$u function for coverage."
          (( unit_untested = unit_untested + 1 ))
       fi
   done
   (( unit_total = unit_tested + unit_untested ))
   (( unit_pdone  = 100/unit_total*unit_tested ))
}

### GET_FUNCTIONAL_TESTS
### Simple static analyzer for htzdns functions, identifies the number
### of test cases (which are all functional for now) declared.
### Note: having a test case declared does not guarantee that it's not a return true.
### IN: none
### OUT: bool
### Global dependencies: none
function GET_FUNCTIONAL_TESTS {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   # todo: would be nice to have some requirements management to compare TCs to.
   tc_tested=0
   tcs=`declare -F |sed 's#declare -f ##' |grep "^TC_"`
   for tc in $tcs; do
     _SAY "    ✅ Test Case: $tc"
     (( tc_tested = tc_tested +1 ))
   done
}

### TEST_INIT_ALL
### Executes all tests in INIT category
### IN: none
### OUT: none
### Global dependencies: none
function TEST_INIT_ALL {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   TEST_INIT_CONFIGS
}

### TEST_API_ALL
### Executes all tests in API category.
### IN: none
### OUT: none
### Global dependencies: none
function TEST_API_ALL {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   TC_API_APIKEY || _SAY "  ❌ API Key $htz_api_key is failing" && _SAY "  $r"
}

### TC_API_APIKEY
### Tests acceptance of API key by Hetzner DNS API.
### IN: none
### OUT: r (string), response confirmation.
### Global dependencies: htz_api_key
function TC_API_APIKEY {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   endpoint="https://dns.hetzner.com/api/v1/zones"
   headers=("H"="Auth-API-Token: $htz_api_key" "H"="Test-Header: 1")
   _VRB2 "GET" "$endpoint" "$headers"
   REQUEST "GET" "$endpoint" "${headers[@]}" || return 1
   r="✅ Excellent, Hetzner API welcomes $htz_api_key (response: $rsp_code: $err_desc)"
}

### TEST_API_GET_ALL_ZONES
### Will test this, but this is a placeholder for testing coverage report.
### IN: none
### OUT: none
### Global dependencies: none
function TEST_API_GET_ALL_ZONES {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   _SAY "1"
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
