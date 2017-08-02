#add r1,d.burst_start_timestamp,r0, 0
#add r2, d.capri_intrinsic_timestamp, r0, 0
#add r3, d.burst_max_timestamp, r0, 0

slt c1, d.capri_intrinsic_timestamp, d.burst_start_timestamp
slt c2, d.burst_max_timestamp, d.capri_intrinsic_timestamp
bcf [!c1 | !c2] fill_tokens_for_microburst
nop
add r1,d.allowed_bytes,d.capri_intrinsic_frame_len, 0
slt c3, d.max_allowed_bytes, r1
//bcf [c3] burst_pkt
tbladd.c3 d.allowed_bytes, d.capri_intrinsic_frame_len
//burst_pkt:
tbladd.!c3 d.burst_exceed_bytes, d.capri_intrinsic_frame_len
tbladd.!c3 d.burst_exceed_count, r0, 1 

fill_tokens_for_microburst: 
tblwr d.allowed_bytes, d.capri_intrinsic_frame_len
tblwr d.burst_start_timestamp, d.capri_intrinsic_timestamp
add r1, d.capri_intrinsic_timestamp, d.micro_burst_cycles
tblwr d.burst_max_timestamp, r1
tblwr d.burst_exceed_count, r0
tblwr d.burst_exceed_bytes, r0

