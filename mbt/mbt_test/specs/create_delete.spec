Name: Create_Delete_Create_Delete
MaxObjects: 1
Enabled: False
LoopCount : 1
Steps :
    - step:
        op     : Create
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
        op     : Delete
        status : API_STATUS_OK
    - step:
        op     : Get
        status : API_STATUS_NOT_FOUND
    - step:
        op     : ReCreate
        status : API_STATUS_OK
    - step:
        op     : ReCreate
        status : API_STATUS_EXISTS_ALREADY    
    - step:
        op     : Delete
        status : API_STATUS_OK
    - step:
        op     : Get
        status : API_STATUS_NOT_FOUND
