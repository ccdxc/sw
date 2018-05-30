Name: Create_Delete_Create_Delete
MaxObjects: 1
Enabled: False
Enabled_v2: True
LoopCount : 1
Steps :
    - step:
        num    : 1
        op     : Create
        status : API_STATUS_OK
    - step:
        num    : 2
        op     : Get
        status : API_STATUS_OK
    - step:
        num    : 3
        op     : Update
        status : API_STATUS_OK
    - step:
        num    : 4
        op     : Get
        status : API_STATUS_OK
    - step:
        num    : 5
        op     : Delete
        status : API_STATUS_OK
    - step:
        num    : 6
        op     : Get
        status : API_STATUS_NOT_FOUND
