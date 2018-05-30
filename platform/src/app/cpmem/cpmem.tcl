
set CPMEM_BASE         0x17fff0000
set CPMEM_RINGSZ       8
set CPMEM_BUFSZ                4096
set CPMEM_MAGIC_VAL    0x3be7dac2

set CPMEM_BASEH                [expr $CPMEM_BASE >> 32]
set CPMEM_BASEL                [expr $CPMEM_BASE & 0xffffffff]

#
# struct cpmem {
#     uint32_t magic;                                   // +0000
#     uint32_t size;                                    // +0004
#     uint32_t to_target;                               // +0008
#     uint32_t pad0[16 - 3];
#     uint32_t head;                                    // +0040
#     uint32_t pad1[16 - 1];
#     uint32_t tail;                                    // +0080
#     uint32_t pad2[1024 - 128 - 1];
#     uint32_t buf[CPMEM_RINGSZ][CPMEM_BUFSZ_WORDS];  // +1000;
# };
#

set CPMEM_MAGIC                [expr $CPMEM_BASEL + 0]
set CPMEM_SIZE         [expr $CPMEM_BASEL + 4]
set CPMEM_TO_TARGET    [expr $CPMEM_BASEL + 8]
set CPMEM_HEAD         [expr $CPMEM_BASEL + 64]
set CPMEM_TAIL         [expr $CPMEM_BASEL + 128]
set CPMEM_BUF          [expr $CPMEM_BASEL + 4096]
set CPMEM_BUFSZ_WORDS  [expr $CPMEM_BUFSZ >> 2]

proc cpmem_read_magic {}     { return [regrd $::CPMEM_BASEH $::CPMEM_MAGIC] }
proc cpmem_read_size {}      { return [regrd $::CPMEM_BASEH $::CPMEM_SIZE] }
proc cpmem_read_to_target {} { return [regrd $::CPMEM_BASEH $::CPMEM_TO_TARGET] }
proc cpmem_read_head {}      { return [regrd $::CPMEM_BASEH $::CPMEM_HEAD] }
proc cpmem_read_tail {}      { return [regrd $::CPMEM_BASEH $::CPMEM_TAIL] }

proc cpmem_write_magic { val }     { regwr $::CPMEM_BASEH $::CPMEM_MAGIC $val }
proc cpmem_write_size { val }      { regwr $::CPMEM_BASEH $::CPMEM_SIZE $val }
proc cpmem_write_to_target { val } { regwr $::CPMEM_BASEH $::CPMEM_TO_TARGET $val }
proc cpmem_write_head { val }      { regwr $::CPMEM_BASEH $::CPMEM_HEAD $val }
proc cpmem_write_tail { val }      { regwr $::CPMEM_BASEH $::CPMEM_TAIL $val }

proc cpmem_reset {} {
    cpmem_write_magic 0
    cpmem_write_head 0
    cpmem_write_tail 0
}

proc cpmem_buf { idx } {
    return [expr $::CPMEM_BUF + $idx * $::CPMEM_BUFSZ]
}

proc cpmem_wait_txready { head } {
    while { 1 } {
        set tail [cpmem_read_tail]
        if { $tail != $head } {
            return $tail
        }
    }
}

proc cpmem_wait_rxready { tail } {
    while { 1 } {
        set head [cpmem_read_head]
        if { $head != $tail } {
            return $head
        }
    }
}

proc cpmem_ring_next { val } {
    incr val
    if { $val == $::CPMEM_RINGSZ } {
        return 0
    } else {
        return $val
    }
}

proc cpmem_write_buf { idx val } {
    regwr_burst $::CPMEM_BASEH [cpmem_buf $idx] $val
}

proc cpmem_read_buf { idx nwords } {
    return [regrd_burst $::CPMEM_BASEH [cpmem_buf $idx] $nwords]
}

proc copyout { path } {
    set sz [file size $path]
    cpmem_reset
    cpmem_write_to_target 1
    cpmem_write_size $sz
    cpmem_write_magic $::CPMEM_MAGIC_VAL

    set fl [open $path]
    fconfigure $fl -translation binary
    set wtotal 0
    puts -nonewline "Sending:    "
    set head 0
    set tail 0
    while {![eof $fl]} {
        set buf [read $fl $::CPMEM_BUFSZ]
        set blen [string length $buf]
        if { $blen & 0x3 } {
            set pad [string repeat "\0" [expr 4 - ($blen & 0x3)]]
            append buf $pad
        }
        if {![binary scan $buf i* val]} {
            error "binary scan error"
        }
        set next_head [cpmem_ring_next $head]
        while { $next_head == $tail } {
            set tail [cpmem_read_tail]
        }
        if { $tail >= $::CPMEM_RINGSZ } {
            close $fl
            error "\nCancelled"
        }
        cpmem_write_buf $head $val
        set head $next_head
        cpmem_write_head $head
        incr wtotal [llength $val]
        if { ($wtotal & 0xffff) == 0 } {
            puts -nonewline [format "\b\b\b\b%3d%%" [expr $wtotal * 400 / $sz]]
        }
    }
    close $fl
    puts [format "\b\b\b\b100%% %u bytes" [expr 4 * $wtotal]]
}

proc copyin { path } {
    if { [cpmem_read_magic] != $::CPMEM_MAGIC_VAL } {
        error "No magic"
    }
    cpmem_write_magic 0
    set sz [cpmem_read_size]
    if { [cpmem_read_to_target] != 0 } {
        error "File not staged for receive"
    }
    set fl [open $path "wb"]
    set remain $sz
    set wtotal 0
    puts -nonewline "Receiving:    "
    set head [cpmem_read_head]
    set tail [cpmem_read_tail]
    while { $remain } {
        if { $remain > $::CPMEM_BUFSZ } {
            set blen $::CPMEM_BUFSZ
        } else {
            set blen $remain
        }
        set nwords [expr (($blen + 3) & -4) >> 2]
        while { $tail == $head } {
            set head [cpmem_read_head]
        }
        if { $head >= $::CPMEM_RINGSZ } {
            close $fl
            error "\nCancelled"
        }
        set val [cpmem_read_buf $tail $nwords]
        if { $blen & 0x3 } {
            set last [lindex $val end]
            set val [lrange $val 0 end-1]
        }
        puts -nonewline $fl [binary format i* $val]
        if { $blen & 0x3 } {
            for { set i 0 } { $i < [expr $blen & 0x3] } { incr i } {
                set c [expr ($last >> (8 * $i)) & 0xff]
                puts -nonewline $fl [binary format c1 $c]
            }
        }
        set tail [cpmem_ring_next $tail]
        cpmem_write_tail $tail
        incr wtotal $nwords
        if { ($wtotal & 0xfff) == 0 } {
            puts -nonewline [format "\b\b\b\b%3d%%" [expr $wtotal * 400 / $sz]]
        }
        incr remain -$blen
    }
    close $fl
    puts [format "\b\b\b\b100%% %u bytes" $sz]
}
