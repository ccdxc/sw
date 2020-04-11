#!/usr/bin/perl

use strict;
use lib "lib/$ENV{'ASIC'}";
use csrPerlAPI;

die "usage: $0 capri_path BUILD VER out_dir\n" if @ARGV != 5;
my $root = $ARGV[0];
my $build = $ARGV[1] + 0;
my $ver = $ARGV[2] + 0;
my $outdir = $ARGV[3];
my $asic = $ARGV[4];

my $asic_info = {
    'capri' => { 'prefix' => 'cap_', 'version' => 'ms_sta_ver' },
    'elba'  => { 'prefix' => 'elb_', 'version' => 'ms_soc_sta_ver' }
};

my $pref = $asic_info->{$asic}->{prefix};
my $version_reg = $asic_info->{$asic}->{version};

#
# Parse the ${pref}top_csr_defines.h and create a map of
#   address -> [ block, subblock, instance, pmfile ]
#
my $path = "$root/model/${pref}top/${pref}top_csr_defines.h";
open(DEFS, $path) || die("$path: $!\n");
my @defs = <DEFS>;
close(DEFS);
my %map;
for $_ (@defs) {
    my ($bk, $sbk, $in, $ad) = $_ =~ /BASE_([A-Z]+)_([A-Z]+[0-9]*)_([0-9]+)_OFFSET.*(0x.*)/;
    if (!defined($ad)) {
        ($bk, $sbk, $ad) = $_ =~ /BASE_([A-Z]+)_([A-Z]+[0-9]*)_OFFSET.*(0x.*)/;
        next if !defined($ad);
        $in = -1;
    }
    $ad = hex($ad);
    my $lsbk;
    ($lsbk = $sbk) =~ tr/[A-Z]/[a-z]/;
    my $pm = "$root/verif/common/csr_gen/" . $lsbk . ".pm";
    if (!-f $pm) {
        $pm = "$root/verif/common/csr_gen/${pref}" . $lsbk . ".pm";
        if (!-f $pm) {
            $pm = '';
        }
    }
    next if $pm eq '';
    $map{$ad} = [$bk, $sbk, $in, $pm];
}

#
# Extract all the register info
#
my %name_to_addr;
my %addr_to_reg;
my %addr_to_fields;
my $reg_count;
for my $ad (sort {$a <=> $b} keys(%map)) {
    my $base_addr = $ad;
    my ($block, $subblock, $instance, $perlPkgName);
    my $aref = $map{$ad};
    ($block, $subblock, $instance, $perlPkgName) = @$aref;
    $block =~ tr/[A-Z]/[a-z]/;
    $subblock =~ tr/[A-Z]/[a-z]/;
    my $perlApiObj = csrPerlAPI->new($perlPkgName);
    my $addrmapObj = $perlApiObj->getNextTopAddressmapNode();

    $addrmapObj->clearExpandArraysFlag();
    $perlApiObj->setIncludeWideRegistersFlag();
    while (my $r = $addrmapObj->getNextDesignNode()) {
        next if !$r->isRegisterNode() && !$r->isWideRegisterNode();
        my $vn = varName($block, $subblock, $instance, $r);
        my $addr = $base_addr + $r->getByteAddress();
        if (defined($name_to_addr{$vn})) {
            printf "Name $vn (from $block, $subblock, $instance, $perlPkgName) already mapped to 0x%08x.  New address is 0x%08x\n", $name_to_addr{$vn}, $addr;
            die("Dup $vn\n");
        }
        $name_to_addr{$vn} = $addr;
        my $nrows = 0;
        $nrows = $r->getArraySize() if $r->isArray();
        my $stride = 0;
        $stride = $r->getArrayElementByteSize() if $r->isArray();
        my $width = $r->getActualByteSize();
        my $secure = $r->hasSecureReadAccess();
        my $foo = {
            nrows => $nrows,
            width => $width,
            stride => $stride,
            instance => $instance,
            secure => $secure,
            name => $vn,
        };
        my @flds;
        while (my $f = $r->getNextFieldNode()) {
            my $frec = {
                hi => $f->getMsb(),
                lo => $f->getLsb(),
                name => $f->getIdentifier()
            };
            push @flds, $frec;
        }
        $foo->{fields} = \@flds;
        $addr_to_reg{$addr} = $foo;
        ++$reg_count;
    }
}
printf "Found %d registers\n", $reg_count;

#------------------------------------------------------------------------------

#
# Dump the data
#
open(OUTTXT, ">" . $outdir . "capviewdb.txt") || die("capviewdb.txt: $!\n");
for my $ad (sort {$a <=> $b} keys(%addr_to_reg)) {
    my $r = $addr_to_reg{$ad};
    printf OUTTXT "0x%08x %5u %3u %3u %d %s\n",
        $ad, $r->{nrows}, $r->{width}, $r->{stride}, $r->{secure}, $r->{name};
    my $f = $r->{fields};
    for my $i (sort {$b->{hi} <=> $a->{hi}} (@$f)) {
        printf OUTTXT " [%3d:%3d] %s\n", $i->{hi}, $i->{lo}, $i->{name};
    }
}
close(OUTTXT);

#
# Create a sorted register name table
#
my $regname_table = createStrTab(keys(%name_to_addr));
for my $ad (keys(%addr_to_reg)) {
    my $r = $addr_to_reg{$ad};
    my $name = $r->{vn};
    $r->{nameidx} = $regname_table->{offsmap}->{$r->{name}};
}
printf "Register Name Table Size: %u\n", length($regname_table->{strtab});

#
# Create a sorted field name table
#
my @field_names;
for my $ad (keys(%addr_to_reg)) {
    my $r = $addr_to_reg{$ad};
    my $f = $r->{fields};
    for my $i (@$f) {
        push @field_names, $i->{name};
    }
}
my $fieldname_table = createStrTab(@field_names);
printf "Field Name Table Size: %u\n", length($fieldname_table->{strtab});

#
# Create packed field objects
# struct field {
#   uint16_t hi;        // hi bit
#   uint16_t lo;        // lo bit
#   uint32_t name;      // name string offset
# };
# struct regfields {
#   uint32_t nfields;
#   struct field field[];
# };
#
my %fldmap = ();
my $field_array = '';
for my $ad (keys(%addr_to_reg)) {
    my $r = $addr_to_reg{$ad};
    my $f = $r->{fields};
    my $pfl = pack("L", int(@$f));
    for my $i (sort {$b->{hi} <=> $a->{hi}} (@$f)) {
        $pfl .= pack("SSL", $i->{hi}, $i->{lo}, $fieldname_table->{offsmap}->{$i->{name}});
    }
    if (!defined($fldmap{$pfl})) {
        $fldmap{$pfl} = length($field_array);
        $field_array .= $pfl;
    }
    $r->{fieldref} = $fldmap{$pfl};
}
printf "Field Array Size: %u\n", length($field_array);

#
# Create packed register objects, sorted on name
# struct {
#   uint32_t addr_s;    // address (bit[0]=1 if secure
#   uint32_t nrows;     // table row count
#   uint32_t name;      // name index from reg_nametab
#   uint32_t fldoffs;   // reg field offset
#   uint16_t width;     // register width;
#   uint16_t stride;    // table stride (<= width)
# };
#
my $reg_array = '';
my $ridx = 0;
for my $name (sort(keys(%name_to_addr))) {
    my $addr = $name_to_addr{$name};
    my $r = $addr_to_reg{$addr};
    $addr |= 0x1 if $r->{secure};
    $r->{ridx} = $ridx++;
    $reg_array .= pack("L4S2", $addr,
            $r->{nrows},
            $regname_table->{offsmap}->{$name},
            $r->{fieldref},
            $r->{width},
            $r->{stride});
}
printf "Register Array Size: %u\n", length($reg_array);

#
# Emit data
# struct header {
#   uint32_t magic;
#   uint16_t build;
#   uint16_t ver;
#   uint32_t ver_reg_idx;
#   uint32_t reg_arr_offs;
#   uint32_t reg_arr_size;
#   uint32_t field_arr_offs;
#   uint32_t field_arr_size;
#   uint32_t regname_tab_offs;
#   uint32_t regname_tab_size;
#   uint32_t fieldname_tab_offs;
#   uint32_t fieldname_tab_size;
# };
#
my $magic = 0xcaf1e1db;
open(OUT, ">" . $outdir . "capviewdb.bin") || die("capviewdb.bin: $!\n");
seek(OUT, 44, 0);
my $reg_arr_offs = tell(OUT); print OUT $reg_array;
my $field_arr_offs = tell(OUT); print OUT $field_array;
my $regname_tab_offs = tell(OUT); print OUT $regname_table->{strtab};
my $fieldname_tab_offs = tell(OUT); print OUT $fieldname_table->{strtab};
seek(OUT, 0, 0);
print OUT pack("LS2", $magic, $build, $ver);
print OUT pack("L",  $addr_to_reg{$name_to_addr{$version_reg}}->{ridx});
print OUT pack("L2", $reg_arr_offs, length($reg_array));
print OUT pack("L2", $field_arr_offs, length($field_array));
print OUT pack("L2", $regname_tab_offs, length($regname_table->{strtab}));
print OUT pack("L2", $fieldname_tab_offs, length($fieldname_table->{strtab}));
close(OUT);

sub varName {
    my ($b, $sb, $in, $r) = @_;
    my $name = $r->getHierarchicalName();
    $name =~ s/${pref}//;
    $name =~ s/_csr_/_/;
    $name =~ s/_csr_/_/;
    $name =~ s/^${sb}_//;
    my $vn = $b;
    $vn .= "_" . $sb if ($b ne $sb);
    $vn .= $in if ($in >= 0);
    $vn .= "_" . $name;
    $vn .= "__S" if $r->hasSecureReadAccess();
    return $vn;
}

sub createStrTab {
    my $n_to_offs = {};
    my $strtab = '';

    my @names = @_;
    for my $n (sort(@names)) {
        if (!defined($n_to_offs->{$n})) {
            $n_to_offs->{$n} = length($strtab);
            $strtab .= pack("Z*", $n);
        }
    }
    my $res = {
        offsmap => $n_to_offs,
        strtab => $strtab
    };
    return $res;
}
