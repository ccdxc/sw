prog_name:
    slt         c1,         key,            keys(3)   //if key < keys[3]
    bcf         [c1],       lessthan3                 //then goto lessthan3
    slt         c1,         key,            keys(5)   //if key < keys[5]
    bcf         [c1],       lessthan5                 //then goto lessthan5
    slt         c1,         key,            keys(6)   //if key < keys[6]
    b           prog_name_ext
    cmov        r7,         c1,             data(5), data(6)
                                                      //then phv.result = nh5
                                                      //else phv.result = nh6
lessthan5:
    slt         c1,         key,            keys(4)   //if key < keys[4]
    b           prog_name_ext
    cmov        r7,         c1,             data(3), data(4)
                                                      //then phv.result = nh3
                                                      //else phv.result = nh4
lessthan3:
    slt         c1,         key,            keys(1)   //if key < keys[1]
    bcf         [c1],       lessthan1                 //then goto lessthan1
    slt         c1,         key,            keys(2)   //if key < keys[2]
    b           prog_name_ext
    cmov        r7,         c1,             data(1), data(2)
                                                      //then phv.result = nh1
                                                      //else phv.result = nh2
lessthan1:
    slt         c1,         key,            keys(0)   //if key < keys[0]
    b           prog_name_ext
    cmov        r7,         c1,             data(_), data(0)
                                                      //then phv.result = nh_
                                                      //else phv.result = nh0
