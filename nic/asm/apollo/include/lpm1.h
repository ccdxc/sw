#include "../../../p4/apollo/include/lpm_defines.h"

%%

route_lpm_s1:
    slt      c1,         key,          keys(7)     //if key < keys[7]
    bcf      [c1],       lessthan7                 //then goto lessthan7
    slt      c1,         key,          keys(11)    //if key < keys[11]
    bcf      [c1],       lessthan11                //then goto lessthan11
    slt      c1,         key,          keys(13)    //if key < keys[13]
    bcf      [c1],       lessthan13                //then goto lessthan13
    slt      c1,         key,          keys(14)    //if key < keys[14]
    bcf      [c1],       lessthan14                //then goto lessthan14
ge14:
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B15_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 11; 0 branch delay slot waste
lessthan14:
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B14_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 12; 1 branch delay slot waste
lessthan13:
    slt      c1,         key,          keys(12)    //if key < keys[12]
    bcf      [c1],       lessthan12                //then goto lessthan12
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B13_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 12; 1 branch delay slot waste
lessthan12:
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B12_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 13; 2 branch delay slot waste
lessthan11:
    slt      c1,         key,          keys(9)     //if key < keys[9]
    bcf      [c1],       lessthan9                 //then goto lessthan9
    slt      c1,         key,          keys(10)    //if key < keys[10]
    bcf      [c1],       lessthan10                //then goto lessthan10
ge10:
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B11_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 12; 1 branch delay slot waste
lessthan10:
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B10_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 13; 2 branch delay slot waste
lessthan9:
    slt      c1,         key,          keys(8)     //if key < keys[8]
    bcf      [c1],       lessthan8                 // if c1 goto lessthan9
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B9_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 13; 2 branch delay slot waste
lessthan8:
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B8_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 14; 3 branch delay slot waste
lessthan7:
    slt      c1,         key,          keys(3)     //if key < keys[3]
    bcf      [c1],       lessthan3                 //then goto lessthan3
    slt      c1,         key,          keys(5)     //if key < keys[5]
    bcf      [c1],       lessthan5                 //then goto lessthan5
    slt      c1,         key,          keys(6)     //if key < keys[6]
    bcf      [c1],       lessthan6                 //then goto lessthan6
ge6:
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B7_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 12; 1 branch delay slot waste
lessthan6:
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B6_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 13; 2 branch delay slot waste
lessthan5:
    slt      c1,         key,          keys(4)     //if key < keys[4]
    bcf      [c1],       lessthan4                 //then goto lessthan4
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B5_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 13; 2 branch delay slot waste
lessthan4:
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B4_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 14; 3 branch delay slot waste
lessthan3:
    slt      c1,         key,          keys(1)     //if key < keys[1]
    bcf      [c1],       lessthan1                 // if c1 goto lessthan1
    slt      c1,         key,          keys(2)     //if key < keys[2]
    bcf      [c1],       lessthan2                 //then goto lessthan2
ge2:
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B3_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 13; 2 branch delay slot waste
lessthan2:
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B2_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 14; 3 branch delay slot waste
lessthan1:
    slt      c1,         key,          keys(0)     //if key < keys[0]
    bcf      [c1],       lessthan0                 //then goto lessthan0
    add      r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
    add.e    r1,         r1,           LPM_S1_B1_OFFSET
    phvwr    next_addr,  r1                        //next_addr = r1
    // Total 14; 3 branch delay slot waste
lessthan0:
    add.e    r1,         curr_addr,    s2_offset   //r1 = curr_addr + s2_base
//    add.e  r1,         r1,           LPM_S1_B0_OFFSET
    phvwr    next_addr,  r1                        // next_addr = r1
    // Total 14; 4 branch delay slot waste

