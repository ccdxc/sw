#define error_handler(a)      a ## _error

%%

prog_name:
    slt        c1,         key,            keys(1)   //if key < keys[1]
    bcf        [c1],       lessthan1                 //then goto lessthan1
    slt        c1,         key,            keys(2)   //if key < keys[2]
    bcf        [c1],       lessthan2                 //then goto lessthan2
ge2:
    phvwr.e    result,     data(2)                   //result = data2
    nop
    // Total 6 instructions; 0 branch delay slot waste
lessthan2:
    phvwr.e    result,     data(1)                   //result = data1
    nop
    // Total 7 instructions; 1 branch delay slot waste
lessthan1:
    slt        c1,         key,            keys(0)   //if key < keys[0]
    bcf        [c1],       lessthan0                 //then goto lessthan0
    phvwr.e    result,     data(0)                   //result = data0
    nop
    // Total 7 instructions; 1 branch delay slot waste
lessthan0:
    phvwr.e    result,     data(_)                   //result = data_
    nop
    // Total 8 instructions; 3 branch delay slot waste

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
error_handler(prog_name):
    phvwr.e    p.capri_intr_drop, 1
    nop
