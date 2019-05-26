#ifdef action_keys16b
/*****************************************************************************/
/*             Non Terminal Action for 16b Keys                              */
/*****************************************************************************/
.align
action_keys16b:
    slt        c1,         key,            keys16b(15) //if key < keys16b[15]
    bcf        [c1],       lessthan15_16b              //then goto lessthan15
    slt        c1,         key,            keys16b(23) //if key < keys16b[23]
    bcf        [c1],       lessthan23_16b              //then goto lessthan23
    slt        c1,         key,            keys16b(27) //if key < keys16b[27]
    bcf        [c1],       lessthan27_16b              //then goto lessthan27
    slt        c1,         key,            keys16b(29) //if key < keys16b[29]
    bcf        [c1],       lessthan29_16b              //then goto lessthan29
    slt        c1,         key,            keys16b(30) //if key < keys16b[30]
    bcf        [c1],       lessthan30_16b              //then goto lessthan30
ge30_16b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B32_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 14 instructions; 0 branch delay slot waste
lessthan30_16b:
    add        r1,         LPM_B31_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 14 instructions; 0 branch delay slot waste
lessthan29_16b:
    slt        c1,         key,            keys16b(28) //if key < keys16b[28]
    bcf        [c1],       lessthan28_16b              //then goto lessthan28
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B30_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 15 instructions; 1 branch delay slot waste
lessthan28_16b:
    add        r1,         LPM_B29_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 15 instructions; 1 branch delay slot waste
lessthan27_16b:
    slt        c1,         key,            keys16b(25) //if key < keys16b[25]
    bcf        [c1],       lessthan25_16b              //then goto lessthan25
    slt        c1,         key,            keys16b(26) //if key < keys16b[26]
    bcf        [c1],       lessthan26_16b              //then goto lessthan26
ge26_16b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B28_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 15 instructions; 1 branch delay slot waste
lessthan26_16b:
    add        r1,         LPM_B27_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 15 instructions; 1 branch delay slot waste
lessthan25_16b:
    slt        c1,         key,            keys16b(24) //if key < keys16b[24]
    bcf        [c1],       lessthan24_16b              //then goto lessthan24
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B26_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 16 instructions; 2 branch delay slot waste
lessthan24_16b:
    add        r1,         LPM_B25_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 16 instructions; 2 branch delay slot waste
lessthan23_16b:
    slt        c1,         key,            keys16b(19) //if key < keys16b[19]
    bcf        [c1],       lessthan19_16b              //then goto lessthan19
    slt        c1,         key,            keys16b(21) //if key < keys16b[21]
    bcf        [c1],       lessthan21_16b              //then goto lessthan21
    slt        c1,         key,            keys16b(22) //if key < keys16b[22]
    bcf        [c1],       lessthan22_16b              //then goto lessthan22
ge22_16b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B24_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 15 instructions; 1 branch delay slot waste
lessthan22_16b:
    add        r1,         LPM_B23_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 15 instructions; 1 branch delay slot waste
lessthan21_16b:
    slt        c1,         key,            keys16b(20) //if key < keys16b[20]
    bcf        [c1],       lessthan20_16b              //then goto lessthan20
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B22_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 16 instructions; 2 branch delay slot waste
lessthan20_16b:
    add        r1,         LPM_B21_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 16 instructions; 2 branch delay slot waste
lessthan19_16b:
    slt        c1,         key,            keys16b(17) //if key < keys16b[17]
    bcf        [c1],       lessthan17_16b              //then goto lessthan17
    slt        c1,         key,            keys16b(18) //if key < keys16b[18]
    bcf        [c1],       lessthan18_16b              //then goto lessthan18
ge18_16b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B20_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 16 instructions; 2 branch delay slot waste
lessthan18_16b:
    add        r1,         LPM_B19_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 16 instructions; 2 branch delay slot waste
lessthan17_16b:
    slt        c1,         key,            keys16b(16) //if key < keys16b[16]
    bcf        [c1],       lessthan16_16b              //then goto lessthan16
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B18_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 17 instructions; 3 branch delay slot waste
lessthan16_16b:
    add        r1,         LPM_B17_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 17 instructions; 3 branch delay slot waste
lessthan15_16b:
    slt        c1,         key,            keys16b(7)  //if key < keys16b[7]
    bcf        [c1],       lessthan7_16b               //then goto lessthan7
    slt        c1,         key,            keys16b(11) //if key < keys16b[11]
    bcf        [c1],       lessthan11_16b              //then goto lessthan11
    slt        c1,         key,            keys16b(13) //if key < keys16b[13]
    bcf        [c1],       lessthan13_16b              //then goto lessthan13
    slt        c1,         key,            keys16b(14) //if key < keys16b[14]
    bcf        [c1],       lessthan14_16b              //then goto lessthan14
ge14_16b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B16_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 15 instructions; 1 branch delay slot waste
lessthan14_16b:
    add        r1,         LPM_B15_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 15 instructions; 1 branch delay slot waste
lessthan13_16b:
    slt        c1,         key,            keys16b(12) //if key < keys16b[12]
    bcf        [c1],       lessthan12_16b              //then goto lessthan12
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B14_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 16 instructions; 2 branch delay slot waste
lessthan12_16b:
    add        r1,         LPM_B13_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 16 instructions; 2 branch delay slot waste
lessthan11_16b:
    slt        c1,         key,            keys16b(9)  //if key < keys16b[9]
    bcf        [c1],       lessthan9_16b               //then goto lessthan9
    slt        c1,         key,            keys16b(10) //if key < keys16b[10]
    bcf        [c1],       lessthan10_16b              //then goto lessthan10
ge10_16b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B12_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 16 instructions; 2 branch delay slot waste
lessthan10_16b:
    add        r1,         LPM_B11_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 16 instructions; 2 branch delay slot waste
lessthan9_16b:
    slt        c1,         key,            keys16b(8)  //if key < keys16b[8]
    bcf        [c1],       lessthan8_16b               //then goto lessthan8
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B10_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 17 instructions; 3 branch delay slot waste
lessthan8_16b:
    add        r1,         LPM_B09_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 17 instructions; 3 branch delay slot waste
lessthan7_16b:
    slt        c1,         key,            keys16b(3)  //if key < keys16b[3]
    bcf        [c1],       lessthan3_16b               //then goto lessthan3
    slt        c1,         key,            keys16b(5)  //if key < keys16b[5]
    bcf        [c1],       lessthan5_16b               //then goto lessthan5
    slt        c1,         key,            keys16b(6)  //if key < keys16b[6]
    bcf        [c1],       lessthan6_16b               //then goto lessthan6
ge6_16b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B08_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 16 instructions; 2 branch delay slot waste
lessthan6_16b:
    add        r1,         LPM_B07_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 16 instructions; 2 branch delay slot waste
lessthan5_16b:
    slt        c1,         key,            keys16b(4)  //if key < keys16b[4]
    bcf        [c1],       lessthan4_16b               //then goto lessthan4
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B06_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 17 instructions; 3 branch delay slot waste
lessthan4_16b:
    add        r1,         LPM_B05_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 17 instructions; 3 branch delay slot waste
lessthan3_16b:
    slt        c1,         key,            keys16b(1)  //if key < keys16b[1]
    bcf        [c1],       lessthan1_16b               //then goto lessthan1
    slt        c1,         key,            keys16b(2)  //if key < keys16b[2]
    bcf        [c1],       lessthan2_16b               //then goto lessthan2
ge2_16b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B04_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 17 instructions; 3 branch delay slot waste
lessthan2_16b:
    add        r1,         LPM_B03_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 17 instructions; 3 branch delay slot waste
lessthan1_16b:
    slt        c1,         key,            keys16b(0)  //if key < keys16b[0]
    bcf        [c1],       lessthan0_16b               //then goto lessthan0
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B02_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 18 instructions; 4 branch delay slot waste
lessthan0_16b:
    add        r1,         LPM_B01_OFFSET, r1,         LPM_LOG2_FANOUT16
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                          //next_addr = r1
    // Total 18 instructions; 4 branch delay slot waste
#endif
#ifdef action_keys32b
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
#endif
#ifdef action_keys64b
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
#endif
#ifdef action_keys128b
/*********************************************************************************/
/*               Non Terminal Action for 128b Keys                               */
/*********************************************************************************/
.align
action_keys128b:
    slt        c1,         keyhi,          keys128bhi(1) //if keyhi < keys128bhi[1]
    seq        c2,         keyhi,          keys128bhi(1) //if keyhi== keys128bhi[1]
    slt.c2     c2,         keylo,          keys128blo(1) //if keylo < keys128blo[1]
    bcf        [c1|c2],    lessthan1_128b                //then goto lessthan1
    slt        c1,         keyhi,          keys128bhi(2) //if keyhi < keys128bhi[2]
    seq        c2,         keyhi,          keys128bhi(2) //if keyhi== keys128bhi[2]
    slt.c2     c2,         keylo,          keys128blo(2) //if keylo < keys128blo[2]
    bcf        [c1|c2],    lessthan2_128b                //then goto lessthan2
ge2_128b:
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B04_OFFSET, r1,           LPM_LOG2_FANOUT128
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                            //next_addr = r1
    // Total 12 instructions; 0 branch delay slot waste
lessthan2_128b:
    add        r1,         LPM_B03_OFFSET, r1,           LPM_LOG2_FANOUT128
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                            //next_addr = r1
    // Total 12 instructions; 0 branch delay slot waste
lessthan1_128b:
    slt        c1,         keyhi,          keys128bhi(0) //if keyhi < keys128bhi[0]
    seq        c2,         keyhi,          keys128bhi(0) //if keyhi== keys128bhi[0]
    slt.c2     c2,         keylo,          keys128blo(0) //if keylo < keys128blo[0]
    bcf        [c1|c2],    lessthan0_128b                //then goto lessthan0
    sub        r1,         curr_addr,      base_addr
    add        r1,         LPM_B02_OFFSET, r1,           LPM_LOG2_FANOUT128
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                            //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
lessthan0_128b:
    add        r1,         LPM_B01_OFFSET, r1,          LPM_LOG2_FANOUT128
    add.e      r1,         r1,             base_addr
    phvwr      next_addr,  r1                           //next_addr = r1
    // Total 13 instructions; 1 branch delay slot waste
#endif
#ifdef action_data16b
/*****************************************************************************/
/*               Terminal Action for 16b Keys                                */
/*****************************************************************************/
.align
action_data16b:
    slt         c1,         key,            keys16b_(7) //if key < keys16b[7]
    bcf         [c1],       lessthan7_16b_              //then goto lessthan7
    slt         c1,         key,            keys16b_(11)//if key < keys16b[11]
    bcf         [c1],       lessthan11_16b_             //then goto lessthan11
    slt         c1,         key,            keys16b_(13)//if key < keys16b[13]
    bcf         [c1],       lessthan13_16b_             //then goto lessthan13
    slt         c1,         key,            keys16b_(14)//if key < keys16b[14]
    b           result_handler16b
    cmov        res_reg,    c1,             data16b(13), data16b(14)
    //then res_reg = nh13
    //else res_reg = nh14
lessthan13_16b_:
    slt         c1,         key,            keys16b_(12)//if key < keys16b[12]
    b           result_handler16b
    cmov        res_reg,    c1,             data16b(11), data16b(12)
    //then res_reg = nh11
    //else res_reg = nh12
lessthan11_16b_:
    slt         c1,         key,            keys16b_(9) //if key < keys16b[9]
    bcf         [c1],       lessthan9_16b_              //then goto lessthan9
    slt         c1,         key,            keys16b_(10)//if key < keys16b[10]
    b           result_handler16b
    cmov        res_reg,    c1,             data16b(9), data16b(10)
    //then res_reg = nh9
    //else res_reg = nh10
lessthan9_16b_:
    slt         c1,         key,            keys16b_(8) //if key < keys16b[8]
    b           result_handler16b
    cmov        res_reg,    c1,             data16b(7), data16b(8)
    //then res_reg = nh7
    //else res_reg = nh8
lessthan7_16b_:
    slt         c1,         key,            keys16b_(3) //if key < keys16b[3]
    bcf         [c1],       lessthan3_16b_              //then goto lessthan3
    slt         c1,         key,            keys16b_(5) //if key < keys16b[5]
    bcf         [c1],       lessthan5_16b_              //then goto lessthan5
    slt         c1,         key,            keys16b_(6) //if key < keys16b[6]
    b           result_handler16b
    cmov        res_reg,    c1,             data16b(5), data16b(6)
    //then res_reg = nh5
    //else res_reg = nh6
lessthan5_16b_:
    slt         c1,         key,            keys16b_(4) //if key < keys16b[4]
    b           result_handler16b
    cmov        res_reg,    c1,             data16b(3), data16b(4)
    //then res_reg = nh3
    //else res_reg = nh4
lessthan3_16b_:
    slt         c1,         key,            keys16b_(1) //if key < keys16b[1]
    bcf         [c1],       lessthan1_16b_              //then goto lessthan1
    slt         c1,         key,            keys16b_(2) //if key < keys16b[2]
    b           result_handler16b
    cmov        res_reg,    c1,             data16b(1), data16b(2)
    //then res_reg = nh1
    //else res_reg = nh2
lessthan1_16b_:
    slt         c1,         key,            keys16b_(0) //if key < keys16b[0]
    b           result_handler16b
    cmov        res_reg,    c1,             data16b(_), data16b(0)
    //then res_reg = nh_
    //else res_reg = nh0
#endif
#ifdef action_data32b
/*****************************************************************************/
/*               Terminal Action for 32b Keys                                */
/*****************************************************************************/
.align
action_data32b:
    slt         c1,         key,            keys32b_(3) //if key < keys32b[3]
    bcf         [c1],       lessthan3_32b_              //then goto lessthan3
    slt         c1,         key,            keys32b_(5) //if key < keys32b[5]
    bcf         [c1],       lessthan5_32b_              //then goto lessthan5
    slt         c1,         key,            keys32b_(6) //if key < keys32b[6]
    b           result_handler32b
    cmov        res_reg,    c1,             data32b(5), data32b(6)
    //then res_reg = nh5
    //else res_reg = nh6
lessthan5_32b_:
    slt         c1,         key,            keys32b_(4) //if key < keys32b[4]
    b           result_handler32b
    cmov        res_reg,    c1,             data32b(3), data32b(4)
    //then res_reg = nh3
    //else res_reg = nh4
lessthan3_32b_:
    slt         c1,         key,            keys32b_(1) //if key < keys32b[1]
    bcf         [c1],       lessthan1_32b_              //then goto lessthan1
    slt         c1,         key,            keys32b_(2) //if key < keys32b[2]
    b           result_handler32b
    cmov        res_reg,    c1,             data32b(1), data32b(2)
    //then res_reg = nh1
    //else res_reg = nh2
lessthan1_32b_:
    slt         c1,         key,            keys32b_(0) //if key < keys32b[0]
    b           result_handler32b
    cmov        res_reg,    c1,             data32b(_), data32b(0)
    //then res_reg = nh_
    //else res_reg = nh0
#endif
#ifdef action_data64b
/*****************************************************************************/
/*               Terminal Action for 64b Keys                                */
/*****************************************************************************/
.align
action_data64b:
    slt         c1,         key,            keys64b_(1) //if key < keys64b_[1]
    bcf         [c1],       lessthan1_64b_              //then goto lessthan1
    slt         c1,         key,            keys64b_(2) //if key < keys64b_[2]
    b           result_handler64b
    cmov        res_reg,    c1,             data64b(1), data64b(2)
    //then res_reg = nh1
    //else res_reg = nh2
lessthan1_64b_:
    slt         c1,         key,            keys64b_(0) //if key < keys64b_[0]
    b           result_handler64b
    cmov        res_reg,    c1,             data64b(_), data64b(0)
    //then res_reg = nh_
    //else res_reg = nh0
#endif
#ifdef action_data128b
/*********************************************************************************/
/*                 Terminal Action for 128b Keys                                 */
/*********************************************************************************/
.align
action_data128b:
    slt        c1,         keyhi,          keys128bhi_(0) //if keyhi<keys128bhi_[0]
    seq        c2,         keyhi,          keys128bhi_(0) //if keyhi==keys128bhi_[0]
    slt.c2     c2,         keylo,          keys128blo_(0) //if keylo<keys128blo_[0]
    setcf      c1,         [c1|c2]
    b          result_handler128b
    cmov       res_reg,    c1,             data128b(_), data128b(0)
    //then res_reg = nh_
    //else res_reg = nh0
#endif

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
error_handler(table_name):
phvwr.e         p.capri_intr_drop, 1
nop
