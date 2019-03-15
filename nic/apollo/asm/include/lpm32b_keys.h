#define LPM_LOG2_FANOUT       (4)
#define LPM_STAGE_FANOUT      (1<<LPM_LOG2_FANOUT)

%%

action_name:
    slt        c1,         key,            keys(7)   //if key < keys[7]
    bcf        [c1],       lessthan7                 //then goto lessthan7
    slt        c1,         key,            keys(11)  //if key < keys[11]
    bcf        [c1],       lessthan11                //then goto lessthan11
    slt        c1,         key,            keys(13)  //if key < keys[13]
    bcf        [c1],       lessthan13                //then goto lessthan13
    slt        c1,         key,            keys(14)  //if key < keys[14]
    bcf        [c1],       lessthan14                //then goto lessthan14
ge14:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B16_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 12 instructions; 0 branch delay slot waste
lessthan14:
    add        r1,         LPM_B15_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 12 instructions; 0 branch delay slot waste
lessthan13:
    slt        c1,         key,            keys(12)  //if key < keys[12]
    bcf        [c1],       lessthan12                //then goto lessthan12
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B14_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan12:
    add        r1,         LPM_B13_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan11:
    slt        c1,         key,            keys(9)   //if key < keys[9]
    bcf        [c1],       lessthan9                 //then goto lessthan9
    slt        c1,         key,            keys(10)  //if key < keys[10]
    bcf        [c1],       lessthan10                //then goto lessthan10
ge10:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B12_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan10:
    add        r1,         LPM_B11_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan9:
    slt        c1,         key,            keys(8)   //if key < keys[8]
    bcf        [c1],       lessthan8                 //then goto lessthan8
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B10_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 14 instructions; 2 branch delay slot waste
lessthan8:
    add        r1,         LPM_B09_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 14 instructions; 2 branch delay slot waste
lessthan7:
    slt        c1,         key,            keys(3)   //if key < keys[3]
    bcf        [c1],       lessthan3                 //then goto lessthan3
    slt        c1,         key,            keys(5)   //if key < keys[5]
    bcf        [c1],       lessthan5                 //then goto lessthan5
    slt        c1,         key,            keys(6)   //if key < keys[6]
    bcf        [c1],       lessthan6                 //then goto lessthan6
ge6:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B08_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan6:
    add        r1,         LPM_B07_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan5:
    slt        c1,         key,            keys(4)   //if key < keys[4]
    bcf        [c1],       lessthan4                 //then goto lessthan4
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B06_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 14 instructions; 2 branch delay slot waste
lessthan4:
    add        r1,         LPM_B05_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 14 instructions; 2 branch delay slot waste
lessthan3:
    slt        c1,         key,            keys(1)   //if key < keys[1]
    bcf        [c1],       lessthan1                 //then goto lessthan1
    slt        c1,         key,            keys(2)   //if key < keys[2]
    bcf        [c1],       lessthan2                 //then goto lessthan2
ge2:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B04_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 14 instructions; 2 branch delay slot waste
lessthan2:
    add        r1,         LPM_B03_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 14 instructions; 2 branch delay slot waste
lessthan1:
    slt        c1,         key,            keys(0)   //if key < keys[0]
    bcf        [c1],       lessthan0                 //then goto lessthan0
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B02_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 15 instructions; 3 branch delay slot waste
lessthan0:
    add        r1,         LPM_B01_OFFSET, r1,       LPM_LOG2_FANOUT
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                        //next_addr = r1
    // Total 15 instructions; 3 branch delay slot waste

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
error_handler(action_name):
    phvwr.e    p.capri_intr_drop, 1
    nop
