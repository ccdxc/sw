Name: Create_Update
MaxObjects: 10
Enabled: True
Steps :
    - step:
        op     : Create
        status : API_STATUS_OK
    - step:
        op     : Get
        status : API_STATUS_OK
    - step:
        op     : Update
        status : API_STATUS_OK
    - step:
        op     : Get
        status : API_STATUS_OK
