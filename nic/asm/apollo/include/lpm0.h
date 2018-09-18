#include "../../../p4/apollo/include/lpm_defines.h"

%%

prog_name:
    slt        c1,         key,            keys(7)   //if key < keys[7]
    bcf        [c1],       lessthan7                 //then goto lessthan7
    slt        c1,         key,            keys(11)  //if key < keys[11]
    bcf        [c1],       lessthan11                //then goto lessthan11
    slt        c1,         key,            keys(13)  //if key < keys[13]
    bcf        [c1],       lessthan13                //then goto lessthan13
    slt        c1,         key,            keys(14)  //if key < keys[14]
    bcf        [c1],       lessthan14                //then goto lessthan14
ge14:
    add        r1,         curr_addr,      LPM_S0_B15_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK15_OFFSET
    // Total 11 instructions; 0 branch delay slot waste
lessthan14:
    add        r1,         curr_addr,      LPM_S0_B14_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK14_OFFSET
    // Total 12 instructions; 1 branch delay slot waste
lessthan13:
    slt        c1,         key,            keys(12)  //if key < keys[12]
    bcf        [c1],       lessthan12                //then goto lessthan12
    add        r1,         curr_addr,      LPM_S0_B13_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK13_OFFSET
    // Total 12 instructions; 1 branch delay slot waste
lessthan12:
    add        r1,         curr_addr,      LPM_S0_B12_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK12_OFFSET
    // Total 13 instructions; 2 branch delay slot waste
lessthan11:
    slt        c1,         key,            keys(9)   //if key < keys[9]
    bcf        [c1],       lessthan9                 //then goto lessthan9
    slt        c1,         key,            keys(10)  //if key < keys[10]
    bcf        [c1],       lessthan10                //then goto lessthan10
ge10:
    add        r1,         curr_addr,      LPM_S0_B11_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK11_OFFSET
    // Total 12 instructions; 1 branch delay slot waste
lessthan10:
    add        r1,         curr_addr,      LPM_S0_B10_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK10_OFFSET
    // Total 13 instructions; 2 branch delay slot waste
lessthan9:
    slt        c1,         key,            keys(8)   //if key < keys[8]
    bcf        [c1],       lessthan8                 //then goto lessthan8
    add        r1,         curr_addr,      LPM_S0_B9_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK9_OFFSET
    // Total 13 instructions; 2 branch delay slot waste
lessthan8:
    add        r1,         curr_addr,      LPM_S0_B8_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK8_OFFSET
    // Total 14 instructions; 3 branch delay slot waste
lessthan7:
    slt        c1,         key,            keys(3)   //if key < keys[3]
    bcf        [c1],       lessthan3                 //then goto lessthan3
    slt        c1,         key,            keys(5)   //if key < keys[5]
    bcf        [c1],       lessthan5                 //then goto lessthan5
    slt        c1,         key,            keys(6)   //if key < keys[6]
    bcf        [c1],       lessthan6                 //then goto lessthan6
ge6:
    add        r1,         curr_addr,      LPM_S0_B7_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK7_OFFSET
    // Total 12 instructions; 1 branch delay slot waste
lessthan6:
    add        r1,         curr_addr,      LPM_S0_B6_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK6_OFFSET
    // Total 13 instructions; 2 branch delay slot waste
lessthan5:
    slt        c1,         key,            keys(4)   //if key < keys[4]
    bcf        [c1],       lessthan4                 //then goto lessthan4
    add        r1,         curr_addr,      LPM_S0_B5_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK5_OFFSET
    // Total 13 instructions; 2 branch delay slot waste
lessthan4:
    add        r1,         curr_addr,      LPM_S0_B4_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK4_OFFSET
    // Total 14 instructions; 3 branch delay slot waste
lessthan3:
    slt        c1,         key,            keys(1)   //if key < keys[1]
    bcf        [c1],       lessthan1                 //then goto lessthan1
    slt        c1,         key,            keys(2)   //if key < keys[2]
    bcf        [c1],       lessthan2                 //then goto lessthan2
ge2:
    add        r1,         curr_addr,      LPM_S0_B3_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK3_OFFSET
    // Total 13 instructions; 2 branch delay slot waste
lessthan2:
    add        r1,         curr_addr,      LPM_S0_B2_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK2_OFFSET
    // Total 14 instructions; 3 branch delay slot waste
lessthan1:
    slt        c1,         key,            keys(0)   //if key < keys[0]
    bcf        [c1],       lessthan0                 //then goto lessthan0
    add        r1,         curr_addr,      LPM_S0_B1_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK1_OFFSET
    // Total 14 instructions; 3 branch delay slot waste
lessthan0:
    add        r1,         curr_addr,      LPM_S0_B0_OFFSET
    phvwr.e    next_addr,  r1                        //next_addr = curr_addr+r1
    phvwr      s2_offset,  LPM_S2_BLOCK0_OFFSET
    // Total 15 instructions; 4 branch delay slot waste

