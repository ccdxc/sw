Name: Create_Update
MaxObjects: 5
Enabled: True
Steps :
    - step:
        op     : Create
        status : API_STATUS_OK
    - step:
        op     : Delete
        status : API_STATUS_OK
    - step:
        op     : ReCreate
        status : API_STATUS_OK
    - step:
        op     : Update
        status : API_STATUS_OK
    - step:
        op     : Get
        status : API_STATUS_OK
