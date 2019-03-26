/*****************************************************************************/
/*             Non Terminal Action for 32b Keys                              */
/*****************************************************************************/
.align
action_keys32b:
    slt        c1,         key,            keys32b(7)  //if key < keys32b[7]
    bcf        [c1],       lessthan7_32b               //then goto lessthan7
    slt        c1,         key,            keys32b(11) //if key < keys32b[11]
    bcf        [c1],       lessthan11_32b              //then goto lessthan11
    slt        c1,         key,            keys32b(13) //if key < keys32b[13]
    bcf        [c1],       lessthan13_32b              //then goto lessthan13
    slt        c1,         key,            keys32b(14) //if key < keys32b[14]
    bcf        [c1],       lessthan14_32b              //then goto lessthan14
ge14_32b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B16_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 12 instructions; 0 branch delay slot waste
lessthan14_32b:
    add        r1,         LPM_B15_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 12 instructions; 0 branch delay slot waste
lessthan13_32b:
    slt        c1,         key,            keys32b(12) //if key < keys32b[12]
    bcf        [c1],       lessthan12_32b              //then goto lessthan12
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B14_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan12_32b:
    add        r1,         LPM_B13_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan11_32b:
    slt        c1,         key,            keys32b(9)  //if key < keys32b[9]
    bcf        [c1],       lessthan9_32b               //then goto lessthan9
    slt        c1,         key,            keys32b(10) //if key < keys32b[10]
    bcf        [c1],       lessthan10_32b              //then goto lessthan10
ge10_32b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B12_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan10_32b:
    add        r1,         LPM_B11_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan9_32b:
    slt        c1,         key,            keys32b(8)  //if key < keys32b[8]
    bcf        [c1],       lessthan8_32b               //then goto lessthan8
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B10_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 14 instructions; 2 branch delay slot waste
lessthan8_32b:
    add        r1,         LPM_B09_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 14 instructions; 2 branch delay slot waste
lessthan7_32b:
    slt        c1,         key,            keys32b(3)  //if key < keys32b[3]
    bcf        [c1],       lessthan3_32b               //then goto lessthan3
    slt        c1,         key,            keys32b(5)  //if key < keys32b[5]
    bcf        [c1],       lessthan5_32b               //then goto lessthan5
    slt        c1,         key,            keys32b(6)  //if key < keys32b[6]
    bcf        [c1],       lessthan6_32b               //then goto lessthan6
ge6_32b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B08_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan6_32b:
    add        r1,         LPM_B07_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan5_32b:
    slt        c1,         key,            keys32b(4)  //if key < keys32b[4]
    bcf        [c1],       lessthan4_32b               //then goto lessthan4
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B06_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 14 instructions; 2 branch delay slot waste
lessthan4_32b:
    add        r1,         LPM_B05_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 14 instructions; 2 branch delay slot waste
lessthan3_32b:
    slt        c1,         key,            keys32b(1)  //if key < keys32b[1]
    bcf        [c1],       lessthan1_32b               //then goto lessthan1
    slt        c1,         key,            keys32b(2)  //if key < keys32b[2]
    bcf        [c1],       lessthan2_32b               //then goto lessthan2
ge2_32b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B04_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 14 instructions; 2 branch delay slot waste
lessthan2_32b:
    add        r1,         LPM_B03_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 14 instructions; 2 branch delay slot waste
lessthan1_32b:
    slt        c1,         key,            keys32b(0)  //if key < keys32b[0]
    bcf        [c1],       lessthan0_32b               //then goto lessthan0
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B02_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 15 instructions; 3 branch delay slot waste
lessthan0_32b:
    add        r1,         LPM_B01_OFFSET, r1,         LPM_LOG2_FANOUT32
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 15 instructions; 3 branch delay slot waste

/*****************************************************************************/
/*               Terminal Action for 32b Keys                                */
/*****************************************************************************/
.align
action_data32b:
    phvwr       ctrl_field, FALSE                       //No more stages
    slt         c1,         key,            keys32b_(3) //if key < keys32b[3]
    bcf         [c1],       lessthan3_32b_              //then goto lessthan3
    slt         c1,         key,            keys32b_(5) //if key < keys32b[5]
    bcf         [c1],       lessthan5_32b_              //then goto lessthan5
    slt         c1,         key,            keys32b_(6) //if key < keys32b[6]
    b           res_handler
    cmov        r7,         c1,             data32b(5), data32b(6)
    //then phv.result = nh5
    //else phv.result = nh6
lessthan5_32b_:
    slt         c1,         key,            keys32b_(4) //if key < keys32b[4]
    b           res_handler
    cmov        r7,         c1,             data32b(3), data32b(4)
    //then phv.result = nh3
    //else phv.result = nh4
lessthan3_32b_:
    slt         c1,         key,            keys32b_(1) //if key < keys32b[1]
    bcf         [c1],       lessthan1_32b_              //then goto lessthan1
    slt         c1,         key,            keys32b_(2) //if key < keys32b[2]
    b           res_handler
    cmov        r7,         c1,             data32b(1), data32b(2)
    //then phv.result = nh1
    //else phv.result = nh2
lessthan1_32b_:
    slt         c1,         key,            keys32b_(0) //if key < keys32b[0]
    b           res_handler
    cmov        r7,         c1,             data32b(_), data32b(0)
    //then phv.result = nh_
    //else phv.result = nh0

/*****************************************************************************/
/*             Non Terminal Action for 64b Keys                              */
/*****************************************************************************/
.align
action_keys64b:
    slt        c1,         key,            keys64b(3)   //if key < keys64b[3]
    bcf        [c1],       lessthan3_64b                //then goto lessthan3
    slt        c1,         key,            keys64b(5)   //if key < keys64b[5]
    bcf        [c1],       lessthan5_64b                //then goto lessthan5
    slt        c1,         key,            keys64b(6)   //if key < keys64b[6]
    bcf        [c1],       lessthan6_64b                //then goto lessthan6
ge6_64b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B08_OFFSET, r1,          LPM_LOG2_FANOUT64
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                           //next_addr = r1
    // Total 10 instructions; 0 branch delay slot waste
lessthan6_64b:
    add        r1,         LPM_B07_OFFSET, r1,          LPM_LOG2_FANOUT64
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                           //next_addr = r1
    // Total 10 instructions; 0 branch delay slot waste
lessthan5_64b:
    slt        c1,         key,            keys64b(4)   //if key < keys64b[4]
    bcf        [c1],       lessthan4_64b                //then goto lessthan4
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B06_OFFSET, r1,          LPM_LOG2_FANOUT64
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                           //next_addr = r1
    // Total 11 instructions; 1 branch delay slot waste
lessthan4_64b:
    add        r1,         LPM_B05_OFFSET, r1,          LPM_LOG2_FANOUT64
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                           //next_addr = r1
    // Total 11 instructions; 1 branch delay slot waste
lessthan3_64b:
    slt        c1,         key,            keys64b(1)   //if key < keys64b[1]
    bcf        [c1],       lessthan1_64b                //then goto lessthan1
    slt        c1,         key,            keys64b(2)   //if key < keys64b[2]
    bcf        [c1],       lessthan2_64b                //then goto lessthan2
ge2_64b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B04_OFFSET, r1,          LPM_LOG2_FANOUT64
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                           //next_addr = r1
    // Total 11 instructions; 1 branch delay slot waste
lessthan2_64b:
    add        r1,         LPM_B03_OFFSET, r1,          LPM_LOG2_FANOUT64
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                           //next_addr = r1
    // Total 11 instructions; 1 branch delay slot waste
lessthan1_64b:
    slt        c1,         key,            keys64b(0)   //if key < keys64b[0]
    bcf        [c1],       lessthan0_64b                //then goto lessthan0
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B02_OFFSET, r1,          LPM_LOG2_FANOUT64
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                           //next_addr = r1
    // Total 12 instructions; 2 branch delay slot waste
lessthan0_64b:
    add        r1,         LPM_B01_OFFSET, r1,          LPM_LOG2_FANOUT64
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                           //next_addr = r1
    // Total 12 instructions; 2 branch delay slot waste

/*****************************************************************************/
/*               Terminal Action for 64b Keys                                */
/*****************************************************************************/
.align
action_data64b:
    phvwr       ctrl_field, FALSE                       //No more stages
    slt         c1,         key,            keys64b_(1) //if key < keys64b_[1]
    bcf         [c1],       lessthan1_64b_              //then goto lessthan1
    slt         c1,         key,            keys64b_(2) //if key < keys64b_[2]
    b           res_handler
    cmov        r7,         c1,             data64b(1), data64b(2)
    //then phv.result = nh1
    //else phv.result = nh2
lessthan1_64b_:
    slt         c1,         key,            keys64b_(0) //if key < keys64b_[0]
    b           res_handler
    cmov        r7,         c1,             data64b(_), data64b(0)
    //then phv.result = nh_
    //else phv.result = nh0

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
error_handler(table_name):
    phvwr.e         p.capri_intr_drop, 1
    nop
