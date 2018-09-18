action erspan() {
}

@pragma stage 0
table mirror {
    reads {
        capri_intrinsic.tm_span_session : exact;
    }
    actions {
        nop;
        erspan;
    }
    size : MIRROR_SESSION_TABLE_SIZE;
}
control mirror {
    if (control_metadata.span_copy == TRUE) {
        apply(mirror);
    }
}
