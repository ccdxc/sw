Name: Create_Get
MaxObjects: 10
Enabled: True
Steps :
    - step:
        op     : Create
        status : API_STATUS_OK
    - step:
        op     : ReCreate
        status : API_STATUS_EXISTS_ALREADY
    - step:
        op     : Get
        status : API_STATUS_OK
