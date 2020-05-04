### Tests

function htz_run_all_api_tests {
  htz_test_apikey || echo "  ❌ API Key $htz_api_key is failing" && echo "  $r"
}

function htz_test_apikey {
  FETCH_DATA "Auth-API-Token: $htz_api_key" "https://dns.hetzner.com/api/v1/zones" || return 1
  htz_error_translator $rsp_code || return 1
  r="✅ Excellent, Hetzner API welcomes $htz_api_key (response: $rsp_code: $err_desc)"
}


