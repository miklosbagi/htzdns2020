### The BASH API for Hetzner
###

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

### Catch Error:
### 200: no error, that's a success
### 401: Unauthorized
### 403: Forbidden
### 404: Not found
### 422: Unprocessable entry

function htz_catch_error {
   echo "Catch Error: $1"
}

