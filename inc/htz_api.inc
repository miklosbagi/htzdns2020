### The BASH API for Hetzner
###

### Catch Error and explain
### expects error code, returns description.
function htz_error_translator {
  err_desc=""
  case $1 in
  200 ) err_desc="Successful"; return ;;
  401 ) err_desc="Unauthorized" ;;
  403 ) err_desc="Forbidden" ;;
  404 ) err_desc="Not found" ;;
  422 ) err_desc="Unprocessable entry" ;;
  esac
  shift
  return 1;
}




###
### The Records "Class" :>
###

### Get Record

### Update Record

### Delete Record is not implemented for htzdns

### Create Record is not implemented for htzdns

### Get all records

### Bulk create records is not implemented for htzdns

### Bulk update records is not implemented for htzdns

###
### The Zones "Class" :>
###

### Get Zone

### Update Zone

### Delete Zone is not implemented for htzdns

### Create Zone is not implemented for htzdns

### Get All Zones

### Import Zone file plain

### Export Zone file 

### Validate Zone file
### Validate Zone file in text/plain format
###
### HEADER PARAMETERS
### Auth API Token: string
### REQUEST BODY SCHEMA: text/plain
### Zone file to validate.
###

#htz_validate_zone

function htz_validate_zone_errors {
  echo yeah
}

###
### Error handlers "Class"
###


