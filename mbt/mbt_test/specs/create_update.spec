Name: Create_Update
MaxObjects: 1
Enabled: True
LoopCount : 5
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
