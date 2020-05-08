# Development notes

## Curl headers is a bitch
I wanted to have a single request function that deals with pretty much everything we send to Hetzner.
This would allow a traditional API implementation where functions can be matched up with methods offered by the API. For this, we need:
- a generic HEADERS_BUILDER that takes any number of header arguments, and organizes them into a -H "key1: value1" -H "key2:value2" order, and passes it onto curl.
-  generic PARAMS_BUILDER that concats all key=value pairs with ampersand, and places a ? to be the first char.

Headers is tricky though. Here's how data flows between functions with inc/hetzner_api.inc:API_GET_ALL_ZONES for example:
1. local headers is initiated before calling inc/request_handler/HEADER_BUILDER with the API key header to be built.
2. HEADER_BUILDER creates a local -n arr="headers", referencing the global.
3. HEADER_BUILDER iterates through all supplied params, and adds them to the _arr_ array.
4. API_GET_ALL_ZONES gets the $headers array back, passes onto inc/request_handler.inc:REQUEST
5. REQUEST passes this onto curl as all items in the array: ${headers[@]}

As a result, bash is a strict requirement, posix sh just won't do.
