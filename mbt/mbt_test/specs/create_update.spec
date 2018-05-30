Name: Create_Update
MaxObjects: 2
Enabled: True
Enabled_v2: False
LoopCount : 100
Steps :
    - step:
        op     : Create
        status : API_STATUS_OK
    - step:
        op     : GetAll
        status : API_STATUS_OK
    - step:
        op     : Delete
        status : API_STATUS_OK
    - step:
        op     : Get
        status : API_STATUS_NOT_FOUND
    - step:
        op     : ReCreate
        status : API_STATUS_OK
    - step:
        op     : Update
        status : API_STATUS_OK
    - step:
        op     : Get
        status : API_STATUS_OK
