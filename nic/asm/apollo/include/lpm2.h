#include "../../../p4/apollo/include/lpm_defines.h"

%%

route_lpm_s2:
    slt         c1,         key,            keys(3)   //if key < keys[3]
    bcf         [c1],       lessthan3                 //then goto lessthan3
    slt         c1,         key,            keys(5)   //if key < keys[5]
    bcf         [c1],       lessthan5                 //then goto lessthan5
    slt         c1,         key,            keys(6)   //if key < keys[6]
    phvwr.c1.e  result,     data(5)                   //then phv.result = nh5
    phvwr.!c1.e result,     data(6)                   //else phv.result = nh6
    nop
    // Total 8 instructions; 0 branch delay slot waste
lessthan5:
    slt         c1,         key,            keys(4)   //if key < keys[4]
    phvwr.c1.e  result,     data(3)                   //then phv.result = nh3
    phvwr.!c1.e result,     data(4)                   //else phv.result = nh4
    nop
    // Total 9 instructions; 1 branch delay slot waste
lessthan3:
    slt         c1,         key,            keys(1)   //if key < keys[1]
    bcf         [c1],       lessthan1                 //then goto lessthan1
    slt         c1,         key,            keys(2)   //if key < keys[2]
    phvwr.c1.e  result,     data(1)                   //then phv.result = nh1
    phvwr.!c1.e result,     data(2)                   //else phv.result = nh2
    nop
    // Total 9 instructions; 1 branch delay slot waste
lessthan1:
    slt         c1,         key,            keys(0)   //if key < keys[0]
    phvwr.c1.e  result,     data(_)                   //then phv.result = nh_
    phvwr.!c1.e result,     data(0)                   //else phv.result = nh0
    nop
    // Total 10 instructions; 2 branch delay slot waste

