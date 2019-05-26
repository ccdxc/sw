#include "../include/artemis_sacl_defines.h"

action sacl_action_p1(pad, id50,
                         id49, id48, id47, id46, id45, id44, id43, id42, id41, id40,
                         id39, id38, id37, id36, id35, id34, id33, id32, id31, id30,
                         id29, id28, id27, id26, id25, id24, id23, id22, id21, id20,
                         id19, id18, id17, id16, id15, id14, id13, id12, id11, id10,
                         id09, id08, id07, id06, id05, id04, id03, id02, id01, id00
                         )
{
    modify_field(txdma_control.p2_class_id,
                 (0 /*data510*/ >> ((txdma_control.p1_class_id % 51) * 10)));

    modify_field(scratch_metadata.field10, id00);
    modify_field(scratch_metadata.field10, id01);
    modify_field(scratch_metadata.field10, id02);
    modify_field(scratch_metadata.field10, id03);
    modify_field(scratch_metadata.field10, id04);
    modify_field(scratch_metadata.field10, id05);
    modify_field(scratch_metadata.field10, id06);
    modify_field(scratch_metadata.field10, id07);
    modify_field(scratch_metadata.field10, id08);
    modify_field(scratch_metadata.field10, id09);
    modify_field(scratch_metadata.field10, id10);
    modify_field(scratch_metadata.field10, id11);
    modify_field(scratch_metadata.field10, id12);
    modify_field(scratch_metadata.field10, id13);
    modify_field(scratch_metadata.field10, id14);
    modify_field(scratch_metadata.field10, id15);
    modify_field(scratch_metadata.field10, id16);
    modify_field(scratch_metadata.field10, id17);
    modify_field(scratch_metadata.field10, id18);
    modify_field(scratch_metadata.field10, id19);
    modify_field(scratch_metadata.field10, id20);
    modify_field(scratch_metadata.field10, id21);
    modify_field(scratch_metadata.field10, id22);
    modify_field(scratch_metadata.field10, id23);
    modify_field(scratch_metadata.field10, id24);
    modify_field(scratch_metadata.field10, id25);
    modify_field(scratch_metadata.field10, id26);
    modify_field(scratch_metadata.field10, id27);
    modify_field(scratch_metadata.field10, id28);
    modify_field(scratch_metadata.field10, id29);
    modify_field(scratch_metadata.field10, id30);
    modify_field(scratch_metadata.field10, id31);
    modify_field(scratch_metadata.field10, id32);
    modify_field(scratch_metadata.field10, id33);
    modify_field(scratch_metadata.field10, id34);
    modify_field(scratch_metadata.field10, id35);
    modify_field(scratch_metadata.field10, id36);
    modify_field(scratch_metadata.field10, id37);
    modify_field(scratch_metadata.field10, id38);
    modify_field(scratch_metadata.field10, id39);
    modify_field(scratch_metadata.field10, id40);
    modify_field(scratch_metadata.field10, id41);
    modify_field(scratch_metadata.field10, id42);
    modify_field(scratch_metadata.field10, id43);
    modify_field(scratch_metadata.field10, id44);
    modify_field(scratch_metadata.field10, id45);
    modify_field(scratch_metadata.field10, id46);
    modify_field(scratch_metadata.field10, id47);
    modify_field(scratch_metadata.field10, id48);
    modify_field(scratch_metadata.field10, id49);
    modify_field(scratch_metadata.field10, id50);
    modify_field(scratch_metadata.field2,  pad);
}

action sacl_action_p2(pad, id50,
                         id49, id48, id47, id46, id45, id44, id43, id42, id41, id40,
                         id39, id38, id37, id36, id35, id34, id33, id32, id31, id30,
                         id29, id28, id27, id26, id25, id24, id23, id22, id21, id20,
                         id19, id18, id17, id16, id15, id14, id13, id12, id11, id10,
                         id09, id08, id07, id06, id05, id04, id03, id02, id01, id00
                         )
{
    modify_field(txdma_control.p3_class_id,
                 (0 /*data510*/ >> ((txdma_control.p2_class_id % 51) * 10)));

    modify_field(scratch_metadata.field10, id00);
    modify_field(scratch_metadata.field10, id01);
    modify_field(scratch_metadata.field10, id02);
    modify_field(scratch_metadata.field10, id03);
    modify_field(scratch_metadata.field10, id04);
    modify_field(scratch_metadata.field10, id05);
    modify_field(scratch_metadata.field10, id06);
    modify_field(scratch_metadata.field10, id07);
    modify_field(scratch_metadata.field10, id08);
    modify_field(scratch_metadata.field10, id09);
    modify_field(scratch_metadata.field10, id10);
    modify_field(scratch_metadata.field10, id11);
    modify_field(scratch_metadata.field10, id12);
    modify_field(scratch_metadata.field10, id13);
    modify_field(scratch_metadata.field10, id14);
    modify_field(scratch_metadata.field10, id15);
    modify_field(scratch_metadata.field10, id16);
    modify_field(scratch_metadata.field10, id17);
    modify_field(scratch_metadata.field10, id18);
    modify_field(scratch_metadata.field10, id19);
    modify_field(scratch_metadata.field10, id20);
    modify_field(scratch_metadata.field10, id21);
    modify_field(scratch_metadata.field10, id22);
    modify_field(scratch_metadata.field10, id23);
    modify_field(scratch_metadata.field10, id24);
    modify_field(scratch_metadata.field10, id25);
    modify_field(scratch_metadata.field10, id26);
    modify_field(scratch_metadata.field10, id27);
    modify_field(scratch_metadata.field10, id28);
    modify_field(scratch_metadata.field10, id29);
    modify_field(scratch_metadata.field10, id30);
    modify_field(scratch_metadata.field10, id31);
    modify_field(scratch_metadata.field10, id32);
    modify_field(scratch_metadata.field10, id33);
    modify_field(scratch_metadata.field10, id34);
    modify_field(scratch_metadata.field10, id35);
    modify_field(scratch_metadata.field10, id36);
    modify_field(scratch_metadata.field10, id37);
    modify_field(scratch_metadata.field10, id38);
    modify_field(scratch_metadata.field10, id39);
    modify_field(scratch_metadata.field10, id40);
    modify_field(scratch_metadata.field10, id41);
    modify_field(scratch_metadata.field10, id42);
    modify_field(scratch_metadata.field10, id43);
    modify_field(scratch_metadata.field10, id44);
    modify_field(scratch_metadata.field10, id45);
    modify_field(scratch_metadata.field10, id46);
    modify_field(scratch_metadata.field10, id47);
    modify_field(scratch_metadata.field10, id48);
    modify_field(scratch_metadata.field10, id49);
    modify_field(scratch_metadata.field10, id50);
    modify_field(scratch_metadata.field2,  pad);
}

action sacl_action_p3(pad, pr45, res45, pr44, res44,
                           pr43, res43, pr42, res42, pr41, res41, pr40, res40,
                           pr39, res39, pr38, res38, pr37, res37, pr36, res36,
                           pr35, res35, pr34, res34, pr33, res33, pr32, res32,
                           pr31, res31, pr30, res30, pr29, res29, pr28, res28,
                           pr27, res27, pr26, res26, pr25, res25, pr24, res24,
                           pr23, res23, pr22, res22, pr21, res21, pr20, res20,
                           pr19, res19, pr18, res18, pr17, res17, pr16, res16,
                           pr15, res15, pr14, res14, pr13, res13, pr12, res12,
                           pr11, res11, pr10, res10, pr09, res09, pr08, res08,
                           pr07, res07, pr06, res06, pr05, res05, pr04, res04,
                           pr03, res03, pr02, res02, pr01, res01, pr00, res00
                      )
{
    modify_field(scratch_metadata.field10, pr00);
    modify_field(scratch_metadata.field1, res00);

    if (pr00 > txdma_control.rule_priority) {
        modify_field(txdma_control.rule_priority,
                 (0 /*data510*/ >> ((txdma_control.p3_class_id % 51) * 10)));
        modify_field(txdma_control.sacl_result, res00);
    }

    modify_field(scratch_metadata.field10, pr01);
    modify_field(scratch_metadata.field1, res01);
    modify_field(scratch_metadata.field10, pr02);
    modify_field(scratch_metadata.field1, res02);
    modify_field(scratch_metadata.field10, pr03);
    modify_field(scratch_metadata.field1, res03);
    modify_field(scratch_metadata.field10, pr04);
    modify_field(scratch_metadata.field1, res04);
    modify_field(scratch_metadata.field10, pr05);
    modify_field(scratch_metadata.field1, res05);
    modify_field(scratch_metadata.field10, pr06);
    modify_field(scratch_metadata.field1, res06);
    modify_field(scratch_metadata.field10, pr07);
    modify_field(scratch_metadata.field1, res07);
    modify_field(scratch_metadata.field10, pr08);
    modify_field(scratch_metadata.field1, res08);
    modify_field(scratch_metadata.field10, pr09);
    modify_field(scratch_metadata.field1, res09);
    modify_field(scratch_metadata.field10, pr10);
    modify_field(scratch_metadata.field1, res10);
    modify_field(scratch_metadata.field10, pr11);
    modify_field(scratch_metadata.field1, res11);
    modify_field(scratch_metadata.field10, pr12);
    modify_field(scratch_metadata.field1, res12);
    modify_field(scratch_metadata.field10, pr13);
    modify_field(scratch_metadata.field1, res13);
    modify_field(scratch_metadata.field10, pr14);
    modify_field(scratch_metadata.field1, res14);
    modify_field(scratch_metadata.field10, pr15);
    modify_field(scratch_metadata.field1, res15);
    modify_field(scratch_metadata.field10, pr16);
    modify_field(scratch_metadata.field1, res16);
    modify_field(scratch_metadata.field10, pr17);
    modify_field(scratch_metadata.field1, res17);
    modify_field(scratch_metadata.field10, pr18);
    modify_field(scratch_metadata.field1, res18);
    modify_field(scratch_metadata.field10, pr19);
    modify_field(scratch_metadata.field1, res19);
    modify_field(scratch_metadata.field10, pr20);
    modify_field(scratch_metadata.field1, res20);
    modify_field(scratch_metadata.field10, pr21);
    modify_field(scratch_metadata.field1, res21);
    modify_field(scratch_metadata.field10, pr22);
    modify_field(scratch_metadata.field1, res22);
    modify_field(scratch_metadata.field10, pr23);
    modify_field(scratch_metadata.field1, res23);
    modify_field(scratch_metadata.field10, pr24);
    modify_field(scratch_metadata.field1, res24);
    modify_field(scratch_metadata.field10, pr25);
    modify_field(scratch_metadata.field1, res25);
    modify_field(scratch_metadata.field10, pr26);
    modify_field(scratch_metadata.field1, res26);
    modify_field(scratch_metadata.field10, pr27);
    modify_field(scratch_metadata.field1, res27);
    modify_field(scratch_metadata.field10, pr28);
    modify_field(scratch_metadata.field1, res28);
    modify_field(scratch_metadata.field10, pr29);
    modify_field(scratch_metadata.field1, res29);
    modify_field(scratch_metadata.field10, pr30);
    modify_field(scratch_metadata.field1, res30);
    modify_field(scratch_metadata.field10, pr31);
    modify_field(scratch_metadata.field1, res31);
    modify_field(scratch_metadata.field10, pr32);
    modify_field(scratch_metadata.field1, res32);
    modify_field(scratch_metadata.field10, pr33);
    modify_field(scratch_metadata.field1, res33);
    modify_field(scratch_metadata.field10, pr34);
    modify_field(scratch_metadata.field1, res34);
    modify_field(scratch_metadata.field10, pr35);
    modify_field(scratch_metadata.field1, res35);
    modify_field(scratch_metadata.field10, pr36);
    modify_field(scratch_metadata.field1, res36);
    modify_field(scratch_metadata.field10, pr37);
    modify_field(scratch_metadata.field1, res37);
    modify_field(scratch_metadata.field10, pr38);
    modify_field(scratch_metadata.field1, res38);
    modify_field(scratch_metadata.field10, pr39);
    modify_field(scratch_metadata.field1, res39);
    modify_field(scratch_metadata.field10, pr40);
    modify_field(scratch_metadata.field1, res40);
    modify_field(scratch_metadata.field10, pr41);
    modify_field(scratch_metadata.field1, res41);
    modify_field(scratch_metadata.field10, pr42);
    modify_field(scratch_metadata.field1, res42);
    modify_field(scratch_metadata.field10, pr43);
    modify_field(scratch_metadata.field1, res43);
    modify_field(scratch_metadata.field10, pr44);
    modify_field(scratch_metadata.field1, res44);
    modify_field(scratch_metadata.field10, pr45);
    modify_field(scratch_metadata.field1, res45);
    modify_field(scratch_metadata.field2,  pad);
}

@pragma stage 2
@pragma hbm_table
@pragma raw_index_table
table sacl_p1 {
    reads {
        txdma_control.rfc_table_addr : exact;
    }
    actions {
        sacl_action_p1;
    }
}

@pragma stage 3
@pragma hbm_table
@pragma raw_index_table
table sacl_p2 {
    reads {
        txdma_control.rfc_table_addr : exact;
    }
    actions {
        sacl_action_p2;
    }
}

@pragma stage 4
@pragma hbm_table
@pragma raw_index_table
table sacl_p3 {
    reads {
        txdma_control.rfc_table_addr : exact;
    }
    actions {
        sacl_action_p3;
    }
}

@pragma stage 5
@pragma hbm_table
@pragma raw_index_table
table sacl_p1_1 {
    reads {
        txdma_control.rfc_table_addr : exact;
    }
    actions {
        sacl_action_p1;
    }
}

@pragma stage 6
@pragma hbm_table
@pragma raw_index_table
table sacl_p2_1 {
    reads {
        txdma_control.rfc_table_addr : exact;
    }
    actions {
        sacl_action_p2;
    }
}

@pragma stage 7
@pragma hbm_table
@pragma raw_index_table
table sacl_p3_1 {
    reads {
        txdma_control.rfc_table_addr : exact;
    }
    actions {
        sacl_action_p3;
    }
}

control sacl {
        apply(sacl_p1);
        apply(sacl_p2);
        apply(sacl_p3);
        apply(sacl_p1_1);
        apply(sacl_p2_1);
        apply(sacl_p3_1);
}
