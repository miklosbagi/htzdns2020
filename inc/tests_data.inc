### Test data for htzdns tests


###
### Static data variables
###
corrupt_htz_host_exclude0=""
corrupt_htz_host_exclude1="@;"

corrupt_endpoint0="http://google.com" # http nono, and should not follow
corrupt_endpoint1="https://localhost_does_not_exist_here-`date +%s`.com" # http nono domain doesnt exist
corrupt_endpoint2="https://expired.badssl.com" # expired ssl cert
corrupt_endpoint3="https://untrusted-root.badssl.com/" # untrusted root certificate
corrupt_endpoint4="https://revoked.badssl.com/" # revoked certificate
corrupt_endpoint5="https://sha1-intermediate.badssl.com/" # weak intermediate

corrupt_response0="" # empty
corrupt_response1="" # html
corrupt_response2="" # redirect
corrupt_response3="" # malformed_json

# Sample data from Hetzner: https://dns.hetzner.com/api-docs/#operation/GetRecords
api_get_all_records_response0="{
"records": [
{
"type": "A",
"id": "string",
"created": "2020-05-17T08:33:30Z",
"modified": "2020-05-17T08:33:30Z",
"zone_id": "string",
"name": "string",
"value": "string",
"ttl": 0
}
],
"meta": {
"pagination": {
"page": 1,
"per_page": 1,
"last_page": 1,
"total_entries": 0
}
}
}"

# Sample data from Hetzner: https://dns.hetzner.com/api-docs/#operation/BulkUpdateRecords
api_bulk_update_records_response0="{
"records": [
{
"type": "A",
"id": "string",
"created": "2020-05-17T08:33:30Z",
"modified": "2020-05-17T08:33:30Z",
"zone_id": "string",
"name": "string",
"value": "string",
"ttl": 0
}
],
"failed_records": [
{
"zone_id": "string",
"type": "A",
"name": "string",
"value": "string",
"ttl": 0
}
]
}"

bad_json_value0=""
bad_json_value1=""
bad_json_value2=""

ipv4_test_data_fail="
127.0.0.1
999.999.999.999.
108.0.0.01
0.1.2.3
00.0000.00.00
192.168.l.1
912.456.123.123
.3.3.3.0
192.168.o.0
172.16.0.9
172.16.4.1
172.17.1.1
127.0.0.2
10.0.1.5
10.0.0.1
10.155.155.155
10.255.255.254
172.16.0.4
172.16.0.1
172.17.1.1
172.31.254.254
192.168.1.2
192.168.254.0
60.123.247.255
196.168.255.255
10.255.255.255
192.168.255.255
"

ipv4_test_data_pass="
212.40.64.243
84.236.105.98
"

###
### Test data generator fucntions
###

### TEST_GET_IPV4_ADDRESS
### Generates random IPv4 address in a single address range
### to be used in test Updates
### IN: none
### OUT: ipv4_test_address global variable
### Global dependencies: rebuild
function TEST_GET_IPV4_ADDRESS {
    $printf "mock"
}
