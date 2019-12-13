#include "../include/sacl_defines.h"
#include "sacl_ip.p4"
#include "sacl_sport.p4"
#include "sacl_proto_dport.p4"

action sacl_ip_sport_p1(pad, id50,
                         id49, id48, id47, id46, id45, id44, id43, id42, id41, id40,
                         id39, id38, id37, id36, id35, id34, id33, id32, id31, id30,
                         id29, id28, id27, id26, id25, id24, id23, id22, id21, id20,
                         id19, id18, id17, id16, id15, id14, id13, id12, id11, id10,
                         id09, id08, id07, id06, id05, id04, id03, id02, id01, id00
                         )
{
    modify_field(scratch_metadata.pad2, pad);
    modify_field(sacl_metadata.p1_class_id,
                 (0 /*data510*/ >> ((sacl_metadata.ip_sport_class_id % 51) * 10)));

    modify_field(scratch_metadata.class_id10, id00);
    modify_field(scratch_metadata.class_id10, id01);
    modify_field(scratch_metadata.class_id10, id02);
    modify_field(scratch_metadata.class_id10, id03);
    modify_field(scratch_metadata.class_id10, id04);
    modify_field(scratch_metadata.class_id10, id05);
    modify_field(scratch_metadata.class_id10, id06);
    modify_field(scratch_metadata.class_id10, id07);
    modify_field(scratch_metadata.class_id10, id08);
    modify_field(scratch_metadata.class_id10, id09);
    modify_field(scratch_metadata.class_id10, id10);
    modify_field(scratch_metadata.class_id10, id11);
    modify_field(scratch_metadata.class_id10, id12);
    modify_field(scratch_metadata.class_id10, id13);
    modify_field(scratch_metadata.class_id10, id14);
    modify_field(scratch_metadata.class_id10, id15);
    modify_field(scratch_metadata.class_id10, id16);
    modify_field(scratch_metadata.class_id10, id17);
    modify_field(scratch_metadata.class_id10, id18);
    modify_field(scratch_metadata.class_id10, id19);
    modify_field(scratch_metadata.class_id10, id20);
    modify_field(scratch_metadata.class_id10, id21);
    modify_field(scratch_metadata.class_id10, id22);
    modify_field(scratch_metadata.class_id10, id23);
    modify_field(scratch_metadata.class_id10, id24);
    modify_field(scratch_metadata.class_id10, id25);
    modify_field(scratch_metadata.class_id10, id26);
    modify_field(scratch_metadata.class_id10, id27);
    modify_field(scratch_metadata.class_id10, id28);
    modify_field(scratch_metadata.class_id10, id29);
    modify_field(scratch_metadata.class_id10, id30);
    modify_field(scratch_metadata.class_id10, id31);
    modify_field(scratch_metadata.class_id10, id32);
    modify_field(scratch_metadata.class_id10, id33);
    modify_field(scratch_metadata.class_id10, id34);
    modify_field(scratch_metadata.class_id10, id35);
    modify_field(scratch_metadata.class_id10, id36);
    modify_field(scratch_metadata.class_id10, id37);
    modify_field(scratch_metadata.class_id10, id38);
    modify_field(scratch_metadata.class_id10, id39);
    modify_field(scratch_metadata.class_id10, id40);
    modify_field(scratch_metadata.class_id10, id41);
    modify_field(scratch_metadata.class_id10, id42);
    modify_field(scratch_metadata.class_id10, id43);
    modify_field(scratch_metadata.class_id10, id44);
    modify_field(scratch_metadata.class_id10, id45);
    modify_field(scratch_metadata.class_id10, id46);
    modify_field(scratch_metadata.class_id10, id47);
    modify_field(scratch_metadata.class_id10, id48);
    modify_field(scratch_metadata.class_id10, id49);
    modify_field(scratch_metadata.class_id10, id50);
}

action sacl_p2(id255, id254, id253, id252, id251, id250,
                id249, id248, id247, id246, id245, id244, id243, id242, id241, id240,
                id239, id238, id237, id236, id235, id234, id233, id232, id231, id230,
                id229, id228, id227, id226, id225, id224, id223, id222, id221, id220,
                id219, id218, id217, id216, id215, id214, id213, id212, id211, id210,
                id209, id208, id207, id206, id205, id204, id203, id202, id201, id200,
                id199, id198, id197, id196, id195, id194, id193, id192, id191, id190,
                id189, id188, id187, id186, id185, id184, id183, id182, id181, id180,
                id179, id178, id177, id176, id175, id174, id173, id172, id171, id170,
                id169, id168, id167, id166, id165, id164, id163, id162, id161, id160,
                id159, id158, id157, id156, id155, id154, id153, id152, id151, id150,
                id149, id148, id147, id146, id145, id144, id143, id142, id141, id140,
                id139, id138, id137, id136, id135, id134, id133, id132, id131, id130,
                id129, id128, id127, id126, id125, id124, id123, id122, id121, id120,
                id119, id118, id117, id116, id115, id114, id113, id112, id111, id110,
                id109, id108, id107, id106, id105, id104, id103, id102, id101, id100,
                id099, id098, id097, id096, id095, id094, id093, id092, id091, id090,
                id089, id088, id087, id086, id085, id084, id083, id082, id081, id080,
                id079, id078, id077, id076, id075, id074, id073, id072, id071, id070,
                id069, id068, id067, id066, id065, id064, id063, id062, id061, id060,
                id059, id058, id057, id056, id055, id054, id053, id052, id051, id050,
                id049, id048, id047, id046, id045, id044, id043, id042, id041, id040,
                id039, id038, id037, id036, id035, id034, id033, id032, id031, id030,
                id029, id028, id027, id026, id025, id024, id023, id022, id021, id020,
                id019, id018, id017, id016, id015, id014, id013, id012, id011, id010,
                id009, id008, id007, id006, id005, id004, id003, id002, id001, id000
               )
{
    modify_field(p4_to_rxdma_header.sacl_result,
                 (0 /*data*/ >> (sacl_metadata.proto_dport_class_id * 2)));
    if (scratch_metadata.pad2 == 0) {
        modify_field(capri_intr.drop, TRUE);
    }

    modify_field(scratch_metadata.pad2, id000);
    modify_field(scratch_metadata.pad2, id001);
    modify_field(scratch_metadata.pad2, id002);
    modify_field(scratch_metadata.pad2, id003);
    modify_field(scratch_metadata.pad2, id004);
    modify_field(scratch_metadata.pad2, id005);
    modify_field(scratch_metadata.pad2, id006);
    modify_field(scratch_metadata.pad2, id007);
    modify_field(scratch_metadata.pad2, id008);
    modify_field(scratch_metadata.pad2, id009);
    modify_field(scratch_metadata.pad2, id010);
    modify_field(scratch_metadata.pad2, id011);
    modify_field(scratch_metadata.pad2, id012);
    modify_field(scratch_metadata.pad2, id013);
    modify_field(scratch_metadata.pad2, id014);
    modify_field(scratch_metadata.pad2, id015);
    modify_field(scratch_metadata.pad2, id016);
    modify_field(scratch_metadata.pad2, id017);
    modify_field(scratch_metadata.pad2, id018);
    modify_field(scratch_metadata.pad2, id019);
    modify_field(scratch_metadata.pad2, id020);
    modify_field(scratch_metadata.pad2, id021);
    modify_field(scratch_metadata.pad2, id022);
    modify_field(scratch_metadata.pad2, id023);
    modify_field(scratch_metadata.pad2, id024);
    modify_field(scratch_metadata.pad2, id025);
    modify_field(scratch_metadata.pad2, id026);
    modify_field(scratch_metadata.pad2, id027);
    modify_field(scratch_metadata.pad2, id028);
    modify_field(scratch_metadata.pad2, id029);
    modify_field(scratch_metadata.pad2, id030);
    modify_field(scratch_metadata.pad2, id031);
    modify_field(scratch_metadata.pad2, id032);
    modify_field(scratch_metadata.pad2, id033);
    modify_field(scratch_metadata.pad2, id034);
    modify_field(scratch_metadata.pad2, id035);
    modify_field(scratch_metadata.pad2, id036);
    modify_field(scratch_metadata.pad2, id037);
    modify_field(scratch_metadata.pad2, id038);
    modify_field(scratch_metadata.pad2, id039);
    modify_field(scratch_metadata.pad2, id040);
    modify_field(scratch_metadata.pad2, id041);
    modify_field(scratch_metadata.pad2, id042);
    modify_field(scratch_metadata.pad2, id043);
    modify_field(scratch_metadata.pad2, id044);
    modify_field(scratch_metadata.pad2, id045);
    modify_field(scratch_metadata.pad2, id046);
    modify_field(scratch_metadata.pad2, id047);
    modify_field(scratch_metadata.pad2, id048);
    modify_field(scratch_metadata.pad2, id049);
    modify_field(scratch_metadata.pad2, id050);
    modify_field(scratch_metadata.pad2, id051);
    modify_field(scratch_metadata.pad2, id052);
    modify_field(scratch_metadata.pad2, id053);
    modify_field(scratch_metadata.pad2, id054);
    modify_field(scratch_metadata.pad2, id055);
    modify_field(scratch_metadata.pad2, id056);
    modify_field(scratch_metadata.pad2, id057);
    modify_field(scratch_metadata.pad2, id058);
    modify_field(scratch_metadata.pad2, id059);
    modify_field(scratch_metadata.pad2, id060);
    modify_field(scratch_metadata.pad2, id061);
    modify_field(scratch_metadata.pad2, id062);
    modify_field(scratch_metadata.pad2, id063);
    modify_field(scratch_metadata.pad2, id064);
    modify_field(scratch_metadata.pad2, id065);
    modify_field(scratch_metadata.pad2, id066);
    modify_field(scratch_metadata.pad2, id067);
    modify_field(scratch_metadata.pad2, id068);
    modify_field(scratch_metadata.pad2, id069);
    modify_field(scratch_metadata.pad2, id070);
    modify_field(scratch_metadata.pad2, id071);
    modify_field(scratch_metadata.pad2, id072);
    modify_field(scratch_metadata.pad2, id073);
    modify_field(scratch_metadata.pad2, id074);
    modify_field(scratch_metadata.pad2, id075);
    modify_field(scratch_metadata.pad2, id076);
    modify_field(scratch_metadata.pad2, id077);
    modify_field(scratch_metadata.pad2, id078);
    modify_field(scratch_metadata.pad2, id079);
    modify_field(scratch_metadata.pad2, id080);
    modify_field(scratch_metadata.pad2, id081);
    modify_field(scratch_metadata.pad2, id082);
    modify_field(scratch_metadata.pad2, id083);
    modify_field(scratch_metadata.pad2, id084);
    modify_field(scratch_metadata.pad2, id085);
    modify_field(scratch_metadata.pad2, id086);
    modify_field(scratch_metadata.pad2, id087);
    modify_field(scratch_metadata.pad2, id088);
    modify_field(scratch_metadata.pad2, id089);
    modify_field(scratch_metadata.pad2, id090);
    modify_field(scratch_metadata.pad2, id091);
    modify_field(scratch_metadata.pad2, id092);
    modify_field(scratch_metadata.pad2, id093);
    modify_field(scratch_metadata.pad2, id094);
    modify_field(scratch_metadata.pad2, id095);
    modify_field(scratch_metadata.pad2, id096);
    modify_field(scratch_metadata.pad2, id097);
    modify_field(scratch_metadata.pad2, id098);
    modify_field(scratch_metadata.pad2, id099);
    modify_field(scratch_metadata.pad2, id100);
    modify_field(scratch_metadata.pad2, id101);
    modify_field(scratch_metadata.pad2, id102);
    modify_field(scratch_metadata.pad2, id103);
    modify_field(scratch_metadata.pad2, id104);
    modify_field(scratch_metadata.pad2, id105);
    modify_field(scratch_metadata.pad2, id106);
    modify_field(scratch_metadata.pad2, id107);
    modify_field(scratch_metadata.pad2, id108);
    modify_field(scratch_metadata.pad2, id109);
    modify_field(scratch_metadata.pad2, id110);
    modify_field(scratch_metadata.pad2, id111);
    modify_field(scratch_metadata.pad2, id112);
    modify_field(scratch_metadata.pad2, id113);
    modify_field(scratch_metadata.pad2, id114);
    modify_field(scratch_metadata.pad2, id115);
    modify_field(scratch_metadata.pad2, id116);
    modify_field(scratch_metadata.pad2, id117);
    modify_field(scratch_metadata.pad2, id118);
    modify_field(scratch_metadata.pad2, id119);
    modify_field(scratch_metadata.pad2, id120);
    modify_field(scratch_metadata.pad2, id121);
    modify_field(scratch_metadata.pad2, id122);
    modify_field(scratch_metadata.pad2, id123);
    modify_field(scratch_metadata.pad2, id124);
    modify_field(scratch_metadata.pad2, id125);
    modify_field(scratch_metadata.pad2, id126);
    modify_field(scratch_metadata.pad2, id127);
    modify_field(scratch_metadata.pad2, id128);
    modify_field(scratch_metadata.pad2, id129);
    modify_field(scratch_metadata.pad2, id130);
    modify_field(scratch_metadata.pad2, id131);
    modify_field(scratch_metadata.pad2, id132);
    modify_field(scratch_metadata.pad2, id133);
    modify_field(scratch_metadata.pad2, id134);
    modify_field(scratch_metadata.pad2, id135);
    modify_field(scratch_metadata.pad2, id136);
    modify_field(scratch_metadata.pad2, id137);
    modify_field(scratch_metadata.pad2, id138);
    modify_field(scratch_metadata.pad2, id139);
    modify_field(scratch_metadata.pad2, id140);
    modify_field(scratch_metadata.pad2, id141);
    modify_field(scratch_metadata.pad2, id142);
    modify_field(scratch_metadata.pad2, id143);
    modify_field(scratch_metadata.pad2, id144);
    modify_field(scratch_metadata.pad2, id145);
    modify_field(scratch_metadata.pad2, id146);
    modify_field(scratch_metadata.pad2, id147);
    modify_field(scratch_metadata.pad2, id148);
    modify_field(scratch_metadata.pad2, id149);
    modify_field(scratch_metadata.pad2, id150);
    modify_field(scratch_metadata.pad2, id151);
    modify_field(scratch_metadata.pad2, id152);
    modify_field(scratch_metadata.pad2, id153);
    modify_field(scratch_metadata.pad2, id154);
    modify_field(scratch_metadata.pad2, id155);
    modify_field(scratch_metadata.pad2, id156);
    modify_field(scratch_metadata.pad2, id157);
    modify_field(scratch_metadata.pad2, id158);
    modify_field(scratch_metadata.pad2, id159);
    modify_field(scratch_metadata.pad2, id160);
    modify_field(scratch_metadata.pad2, id161);
    modify_field(scratch_metadata.pad2, id162);
    modify_field(scratch_metadata.pad2, id163);
    modify_field(scratch_metadata.pad2, id164);
    modify_field(scratch_metadata.pad2, id165);
    modify_field(scratch_metadata.pad2, id166);
    modify_field(scratch_metadata.pad2, id167);
    modify_field(scratch_metadata.pad2, id168);
    modify_field(scratch_metadata.pad2, id169);
    modify_field(scratch_metadata.pad2, id170);
    modify_field(scratch_metadata.pad2, id171);
    modify_field(scratch_metadata.pad2, id172);
    modify_field(scratch_metadata.pad2, id173);
    modify_field(scratch_metadata.pad2, id174);
    modify_field(scratch_metadata.pad2, id175);
    modify_field(scratch_metadata.pad2, id176);
    modify_field(scratch_metadata.pad2, id177);
    modify_field(scratch_metadata.pad2, id178);
    modify_field(scratch_metadata.pad2, id179);
    modify_field(scratch_metadata.pad2, id180);
    modify_field(scratch_metadata.pad2, id181);
    modify_field(scratch_metadata.pad2, id182);
    modify_field(scratch_metadata.pad2, id183);
    modify_field(scratch_metadata.pad2, id184);
    modify_field(scratch_metadata.pad2, id185);
    modify_field(scratch_metadata.pad2, id186);
    modify_field(scratch_metadata.pad2, id187);
    modify_field(scratch_metadata.pad2, id188);
    modify_field(scratch_metadata.pad2, id189);
    modify_field(scratch_metadata.pad2, id190);
    modify_field(scratch_metadata.pad2, id191);
    modify_field(scratch_metadata.pad2, id192);
    modify_field(scratch_metadata.pad2, id193);
    modify_field(scratch_metadata.pad2, id194);
    modify_field(scratch_metadata.pad2, id195);
    modify_field(scratch_metadata.pad2, id196);
    modify_field(scratch_metadata.pad2, id197);
    modify_field(scratch_metadata.pad2, id198);
    modify_field(scratch_metadata.pad2, id199);
    modify_field(scratch_metadata.pad2, id200);
    modify_field(scratch_metadata.pad2, id201);
    modify_field(scratch_metadata.pad2, id202);
    modify_field(scratch_metadata.pad2, id203);
    modify_field(scratch_metadata.pad2, id204);
    modify_field(scratch_metadata.pad2, id205);
    modify_field(scratch_metadata.pad2, id206);
    modify_field(scratch_metadata.pad2, id207);
    modify_field(scratch_metadata.pad2, id208);
    modify_field(scratch_metadata.pad2, id209);
    modify_field(scratch_metadata.pad2, id210);
    modify_field(scratch_metadata.pad2, id211);
    modify_field(scratch_metadata.pad2, id212);
    modify_field(scratch_metadata.pad2, id213);
    modify_field(scratch_metadata.pad2, id214);
    modify_field(scratch_metadata.pad2, id215);
    modify_field(scratch_metadata.pad2, id216);
    modify_field(scratch_metadata.pad2, id217);
    modify_field(scratch_metadata.pad2, id218);
    modify_field(scratch_metadata.pad2, id219);
    modify_field(scratch_metadata.pad2, id220);
    modify_field(scratch_metadata.pad2, id221);
    modify_field(scratch_metadata.pad2, id222);
    modify_field(scratch_metadata.pad2, id223);
    modify_field(scratch_metadata.pad2, id224);
    modify_field(scratch_metadata.pad2, id225);
    modify_field(scratch_metadata.pad2, id226);
    modify_field(scratch_metadata.pad2, id227);
    modify_field(scratch_metadata.pad2, id228);
    modify_field(scratch_metadata.pad2, id229);
    modify_field(scratch_metadata.pad2, id230);
    modify_field(scratch_metadata.pad2, id231);
    modify_field(scratch_metadata.pad2, id232);
    modify_field(scratch_metadata.pad2, id233);
    modify_field(scratch_metadata.pad2, id234);
    modify_field(scratch_metadata.pad2, id235);
    modify_field(scratch_metadata.pad2, id236);
    modify_field(scratch_metadata.pad2, id237);
    modify_field(scratch_metadata.pad2, id238);
    modify_field(scratch_metadata.pad2, id239);
    modify_field(scratch_metadata.pad2, id240);
    modify_field(scratch_metadata.pad2, id241);
    modify_field(scratch_metadata.pad2, id242);
    modify_field(scratch_metadata.pad2, id243);
    modify_field(scratch_metadata.pad2, id244);
    modify_field(scratch_metadata.pad2, id245);
    modify_field(scratch_metadata.pad2, id246);
    modify_field(scratch_metadata.pad2, id247);
    modify_field(scratch_metadata.pad2, id248);
    modify_field(scratch_metadata.pad2, id249);
    modify_field(scratch_metadata.pad2, id250);
    modify_field(scratch_metadata.pad2, id251);
    modify_field(scratch_metadata.pad2, id252);
    modify_field(scratch_metadata.pad2, id253);
    modify_field(scratch_metadata.pad2, id254);
    modify_field(scratch_metadata.pad2, id255);
}

@pragma stage 4
@pragma hbm_table
@pragma raw_index_table
table sacl_ip_sport_p1 {
    reads {
        sacl_metadata.p1_table_addr : exact;
    }
    actions {
        sacl_ip_sport_p1;
    }
}

@pragma stage 6
@pragma hbm_table
@pragma raw_index_table
table sacl_p2 {
    reads {
        sacl_metadata.p2_table_addr : exact;
    }
    actions {
        sacl_p2;
    }
}

control sacl {
    if (p4_to_rxdma_header.sacl_bypass == FALSE) {
        /* This assumes we're supporting
         * 256 sport ranges - 512 nodes in LPM tree, outputs 7 bit class-id
         *                    32*16 entries packed in 2 level LPM
         * 512 proto+dport ranges - 2k nodes in LPM tree, outputs 8 bit class-id
         *                          16*16*8 entres packed in 3 level LPM
         * 1k ipv4 - 2k nodes in LPM tree, outputs 10 bit class-id
         *           16*16*8 entries packed in 3 level LPM
         *
         * Phase 1:
         * Combine 7 bit sport-class-id and 10 bit ipv4-class-id, output
         * 10 bit class-id
         *
         * Phase2:
         * Combine 8 bit prot+dport-class-id and 10 bit phase 1 class-id to
         * yield 2 bit result
         */
        apply(sacl_sport_lpm_s0);
        apply(sacl_sport_lpm_s1);
        apply(sacl_ip1);
        apply(sacl_ip2);
        apply(sacl_ip);
        apply(sacl_proto_dport_keys);
        apply(sacl_proto_dport_keys1);
        apply(sacl_proto_dport_data);
        apply(sacl_ip_sport_p1);
        apply(sacl_p2);
    }
}
