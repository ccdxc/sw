action_name:
    slt         c1,         key,            keys(1)   //if key < keys[1]
    bcf         [c1],       lessthan1                 //then goto lessthan1
    slt         c1,         key,            keys(2)   //if key < keys[2]
    b           res_handler
    cmov        r7,         c1,             data(1), data(2)
    //then phv.result = nh1
    //else phv.result = nh2
lessthan1:
    slt         c1,         key,            keys(0)   //if key < keys[0]
    b           res_handler
    cmov        r7,         c1,             data(_), data(0)
    //then phv.result = nh_
    //else phv.result = nh0
