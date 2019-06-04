#include "includes/capri_intrinsic.p4"

header_type h_t {
    fields {
        a : 8;
    }
}

header_type meta_t {
    fields {
        x : 8;
    }
}

header h_t h1;
header h_t h2;
header h_t h3;
header h_t h4;
header h_t h5;
@pragma dont_trim
metadata meta_t meta;
header cap_phv_intr_global_t capri_intrinsic;                                                       
metadata cap_phv_intr_p4_t capri_p4_intrinsic;                                                      

parser start {
    extract(capri_intrinsic);
    return select(current(0, 4)) {
        0x1 : s1;
        0x2 : s2;
        default : s3;
    }
}

parser s1 {
    extract(h1);
    return s1_1;
}

parser s1_1 {
    extract(h3);
    return s1_2;
}
parser s1_2 {
    extract(h4);
    set_metadata(meta.x, h4.a);
    return ingress;
}
parser s2 {
    extract(h1);
    return s2_1;
}

parser s2_1 {
    extract(h2);
    return ingress;
}
parser s3 {
    extract(h2);
    return s3_1;
}

parser s3_1 {
    extract(h3);
    return s3_2;
}

parser s3_2 {
    extract(h5);
    set_metadata(meta.x, h5.a);
    return ingress;
}

action nop() {}

table T1 {
    reads {
        meta.x : exact;
    }
    actions {
        nop;
    }
    size : 256;
}
control ingress {
    apply(T1);
}

control egress {
}
