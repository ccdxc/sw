#
# Copyright (c) SEMIFORE, INC. 2014. All rights reserved.
#
# This software and documentation constitute an unpublished work and
# contain valuable trade secrets and proprietary information belonging
# to Semifore, Inc.  None of the foregoing material may be copied,
# duplicated or disclosed without the express written permission of
# Semifore, Inc.
# SEMIFORE, INC. EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES CONCERNING
# THIS SOFTWARE AND DOCUMENTATION, INCLUDING ANY WARRANTIES OF 
# MERCHANTABILITY AND/OR FITNESS FOR ANY PARTICULAR PURPOSE, 
# AND WARRANTIES OF PERFORMANCE, AND ANY WARRANTY THAT 
# MIGHT OTHERWISE ARISE FROM COURSE OF DEALING OR USAGE OF 
# TRADE. NO WARRANTY IS EITHER EXPRESS OR IMPLIED WITH RESPECT 
# TO THE USE OF THE SOFTWARE OR DOCUMENTATION.
# Under no circumstances shall Semifore, Inc. be liable for incidental,
# special, indirect, direct or consequential damages or loss of profits,
# interruption of business, or related expenses which may arise from use
# of this software or documentation, including but not limited to those
# resulting from defects in software and/or documentation, or loss or
# inaccuracy of data of any kind.
#

#
# csrPerlAPI.pm
#
# $Revision: 1.553 $
# $Date: 2017/12/05 20:17:56 $
#
# The Perl API class definitions.
#

#!/usr/bin/perl

package csrPerlAPI;

@csrPerlAPI::ISA = qw(csrPerlAPIBase);

use Math::BigInt;
use File::Basename;
use Scalar::Util;
use strict;

my $designDBRef;
my $designPkgName;
my @visitedSubRegsList;
my @visitedAddrmapsList;
my @visitedAddrmapsListShadow;
my $skipExternalRegisters = 0;
my $skipExternalAddressmaps = 0;
my $skipDeclaredExternalRegisters = 0;
my $skipDeclaredExternalAddressmaps = 0;
my $skipContentsFlag = 0;
my $limitExternalRegArrayFlag = 0;
my $limitExternalGroupArrayFlag = 0;
my $limitExternalUnionArrayFlag = 0;
my $semiforeEnvGenFlag = 0;
my $semiforeTBGenFlag = 0;
my $expandArraysFlag = 0;
my $expandFieldArraysFlag = 1;
my $hierarchicalTBFlag = 0;
my $viewName = '';
my @referenceInfoList = ();
my $addressmapResetTiming = '';
my $updateRefArrayIndex = 0;
my $includeWideRegistersFlag = 0;
my $includeWideMemoriesFlag = 0;
my $tmpIncludeWideRegistersFlag = 0;
my $tmpIncludeWideMemoriesFlag = 0;
my $defaultResetKind = "HARD";

{
    my %apiAttrs = (_designPkgName => '',
		    _sourceFileVersion => '',
		    );
}

my $nextAddressmapIndex;
my $nextAddressmapIndexShadow;

sub new {
    my($caller, $pkgFilename, $addrmapName) = @_;

    my $addrmapHashRef;
    $nextAddressmapIndex = -1;
    my ($dirName, $suffix, $pkgName, $pkgPathname);

    ($pkgName, $pkgPathname,$suffix) = fileparse($pkgFilename, qr/\.pm/);
    my $class = ref($caller) || $caller;
    if(!defined($pkgName) || ($pkgName eq '')) {
	print "\n\t" . 'Must provide design Perl package name.' . "\n";
	exit(1);
    }
    my $self = bless {}, $class;
    my @prefixList = ();
    $designDBRef = addressmapDB->new($pkgName, $pkgPathname);
    if(!defined($addrmapName) || $addrmapName eq '') {
	# get the top level addressmap
	$addrmapHashRef = $designDBRef->{'topLevelAddressmaps'}->[0];
	$designDBRef->set_topAddressmapName($addrmapHashRef->{'name'});
	$designDBRef->set_topAddressmapHierarchicalIdentifier($addrmapHashRef->{'hierarchicalIdentifier'});
	$designDBRef->set_topAddressmapHashRefNode($addrmapHashRef);
    } else {
	$designDBRef->set_topAddressmapName($addrmapName);
	my $addrmapNode = $designDBRef->_findAddressmap($addrmapName);
	if (!defined($addrmapNode)) {
	    $self->_printError("Could not find address map '$addrmapName' in the database.");
	    exit(1);
	}
	$designDBRef->set_topAddressmapHashRefNode($addrmapNode);
    }
    my $srcFileVersion = $designDBRef->{'csrInfo'}->{'inputFileVersion'};
    my $srcFilename = $designDBRef->{'csrInfo'}->{'inputFilename'};
    $self->set_designPkgName($pkgName);
    $self->set_sourceFileVersion($srcFileVersion);
    $self->set_sourceFilename($srcFilename);
    $designPkgName = $pkgName;
    return $self;
}

sub getNextTopAddressmapNode {
    my $self = shift;

    my @hierarchicalPrefixList;
    my ($addrmapHashRef, $parentAddrmapHashRef, $relAddr);
    if($nextAddressmapIndex >= $self->getTopLevelAddressmapCount()-1) {
	$nextAddressmapIndex = -1;
	return undef;
    } else {
	$addrmapHashRef = $designDBRef->{'topLevelAddressmaps'}->[++$nextAddressmapIndex];
	$designDBRef->set_topAddressmapName($addrmapHashRef->{'name'});
	$designDBRef->set_topAddressmapHashRefNode($addrmapHashRef);
	my ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($addrmapHashRef->{'byteAddress'});
	my $bigIntAddr = Math::BigInt->new($offset);
	my @prefixList = ();
	push (@hierarchicalPrefixList, $addrmapHashRef->{'hierarchicalIdentifier'});
	($verilogFormat,
	 $relAddr) = $self->getVerilogBaseAndValue($addrmapHashRef->{'addressmapRelativeByteAddress'});
	my $relAddrBigInt = Math::BigInt->new($relAddr);
	my $arrayIndex = undef;
	my $addrmapObj = csrPerlAPIModule->new($addrmapHashRef,
					       $arrayIndex,
					       $parentAddrmapHashRef,
					       $bigIntAddr,
					       $relAddrBigInt,
					       $addrmapHashRef->{'busProtocol'},
					       $addrmapHashRef->{'inputPipelineStage'},
					       $addrmapHashRef->{'outputPipelineStage'},
					       \@prefixList,
					       \@hierarchicalPrefixList);
	return $addrmapObj;
    }
}

sub hasComponent {
    my $self = shift;
    return 1 if (defined($designDBRef->{'component'}));
    return 0;
}

sub getComponentName {
    my $self = shift;
    return $designDBRef->{'component'}->[0]->{'name'} if ($self->hasComponent());
    return '';
}

package addressmapDB;
@addressmapDB::ISA = qw(csrPerlAPIBase);

use strict;
use Cwd;

sub new {
    my($caller, $designPkgName, $pkgPathname) = @_;

    my $currDir;
    if(defined($pkgPathname) && ($pkgPathname ne '')) {
	$currDir = cwd();
	chdir($pkgPathname);
    }
    eval ("use $designPkgName");
    chdir($currDir) if(defined($pkgPathname) && ($pkgPathname ne ''));
    my $designDBRef = eval('$' . $designPkgName . '::csr_db');
    my $self = bless $designDBRef, ref($caller) || $caller;
    $self->set_nextRegHashIndex(-1);
    $self->set_nextInstHashIndex(-1);
    $self->set_nextGrpHashIndex(-1);
    return $self;
}

sub _getBusProtocol {
    my ($self, $topLevelHashRef, $parentHashRef) = @_;

    my $busProtocol = $topLevelHashRef->{'busProtocol'};
    my $inputPipelineStage = $topLevelHashRef->{'inputPipelineStage'};
    my $outputPipelineStage = $topLevelHashRef->{'outputPipelineStage'};
    $busProtocol = $parentHashRef->{'busProtocol'} if(defined($parentHashRef->{'busProtocol'}));
    if (defined($parentHashRef->{'inputPipelineStage'})) {
        $inputPipelineStage = $parentHashRef->{'inputPipelineStage'};
    }
    if (defined($parentHashRef->{'outputPipelineStage'})) {
        $outputPipelineStage = $parentHashRef->{'outputPipelineStage'}
    }
    return ($busProtocol, $inputPipelineStage, $outputPipelineStage);
}

sub _getParentHash {
    my ($self, $currHash) = @_;

    # get the parent address map or group hash ref for currHash arg.

    my $count = 0;
    my @tmpLevelNameList;
    my ($parentHashRef, $parentName);

    my $bigIntAddr = Math::BigInt->new(0);
    my @levelNameList = split('\.', $currHash->{'hierarchicalIdentifier'});
    my @hierLevelNameList = split('\.', $currHash->{'hierarchicalIdentifier'});

    return if(@levelNameList <= 1);

    pop(@levelNameList);
    pop(@hierLevelNameList);
    # start from the last level name.
    while(@levelNameList) {
        $parentName = join('.', @levelNameList);
        ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
        if(($parentHashRef->{'nodeType'} eq 'addressmap') ||
           ($parentHashRef->{'nodeType'} eq 'addressmapInstance') ||
           ($parentHashRef->{'nodeType'} eq 'memory') || # virtual groups have memory parent
           ($parentHashRef->{'nodeType'} eq 'group') ||
           ($parentHashRef->{'nodeType'} eq 'component')) {
            return $parentHashRef;
        }
        pop(@levelNameList);
        pop(@hierLevelNameList);
    }
    return;
}

sub _isExternalHashInHierarchy {
    my ($self, $currHash) = @_;

    # Is there an external hash up the hierarchy from currHash?

    my $external;
    my $parentHashRef = $self->_getParentHash($currHash);
    while (defined($parentHashRef)) {
	if (defined($external = $parentHashRef->{'external'}) &&
	    ($external eq 'true')) {
	    return 1;
	} else {
	    $parentHashRef = $self->_getParentHash($parentHashRef);
	}
    }
    return 0;
}

sub _getHashRefForName {
    my ($self, $targetName) = @_;

    my ($verilogFormat, $offset, $instHashRef, $bigIntAddr);
    my ($arrayIndex, $nextLevelHashRef, $parentHashRef);
    my ($nextLevelName, $levelIndex);

    my $found = 0;
    my $parentOffset = Math::BigInt->new('0');
    my ($tmpNextLevelHashRef, $tmpNextLevelName, $targetHashRef);
    my @levelNameList = split('\.', $targetName);
    my @targetLevelNameList = split('\.', $targetName);

    push(@levelNameList, $targetName) if(@levelNameList <= 0);

    if (defined($designDBRef->{'component'})) {
	# For PLF files with scoped address maps
	if ($targetName =~ /\:\:/) {
	    # get the top level address map hash.
	    my $nextAddrmap;
	    foreach my $nextNode (@{$designDBRef->{'topLevelAddressmaps'}}) {
		if ($nextNode->{'referenceIdentifier'} eq $levelNameList[0]) {
		    $found = 1;
		    $nextAddrmap = $nextNode;
		    last;
		}
	    }
	    if (!$found) {
		my $msg = 'Top level addressmap: ' . $targetName . ' not found!';
		$self->_printError($msg);
	    }
	    if (@levelNameList == 1) { # target has scoped address map only
		return ($nextAddrmap, $bigIntAddr);
	    } else {
		$parentHashRef = $nextAddrmap;
		$levelNameList[0] =~ s/(\w+)\:\://;
	    }
	} elsif ($targetName =~ /(\w+)\.(\w+)/) {
	    shift(@levelNameList);
	    my $found = 0;
	    foreach my $nextNode (@{$designDBRef->{'topLevelAddressmaps'}}) {
		if ($nextNode->{'name'} eq $levelNameList[0]) {
		    $found = 1;
		    $parentHashRef = $nextNode;
		    last;
		}
	    }
	    if (!$found) {
		my $msg = 'Top level addressmap: ' . $targetName . ' not found!';
		$self->_printError($msg);
	    }
	} else {
	    $parentHashRef = $designDBRef->{'component'}->[0];
	}
	shift (@targetLevelNameList);
    } else {
	# No component in database
	if ($targetName =~ /\:\:/) {
	    # But the addressmap is scoped
	    my $nextAddrmap;
	    foreach my $nextNode (@{$designDBRef->{'topLevelAddressmaps'}}) {
		if ($nextNode->{'referenceIdentifier'} eq $levelNameList[0]) {
		    $found = 1;
		    $nextAddrmap = $nextNode;
		    last;
		}
	    }
	    if (@levelNameList == 1) { # target has scoped address map only
		return ($nextAddrmap, $bigIntAddr);
	    } else {
		$parentHashRef = $nextAddrmap;
		$levelNameList[0] =~ s/(\w+)\:\://;
	    }
	} else {
	    $parentHashRef = $designDBRef->get_topAddressmapHashRefNode();
	}
    }
    if ($parentHashRef->{'hierarchicalIdentifier'} eq $targetName) {
	return ($parentHashRef, $bigIntAddr);
    }
    ($verilogFormat, $parentOffset) = $self->getVerilogBaseAndValue($parentHashRef->{'byteOffset'});
    my $totalOffset = $parentOffset;
    for ($levelIndex = 0; $levelIndex < @levelNameList; $levelIndex++) {
	$nextLevelName = $levelNameList[$levelIndex];
	$tmpNextLevelName = $nextLevelName;
	if($parentHashRef->{'nodeType'} eq 'wideRegister') {
	    if ($nextLevelName =~ /\[\d+\]$/) {
		# an array element
		$nextLevelName = $self->_getArrayName($nextLevelName);
	    }
	    ($nextLevelHashRef, $instHashRef) = $designDBRef->_findNodeByName($nextLevelName, $parentHashRef);
	    last if (!defined($nextLevelHashRef) ||
		     ($nextLevelHashRef->{'nodeType'} eq 'field'));

	    ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($nextLevelHashRef->{'byteOffset'});
	    $bigIntAddr = Math::BigInt->new($offset);
	    $bigIntAddr->badd($totalOffset);
	    if($nextLevelHashRef->{'nodeType'} eq 'register') {
		$targetHashRef = $nextLevelHashRef;
		if ($levelIndex < (@levelNameList-1)) {
		    $parentHashRef = $nextLevelHashRef;
		} else {
		    last;
		}
	    } else {
		my $msg = 'Node: ' . $nextLevelHashRef->{'name'} . ', with wideRegister parent: ' .
		    $parentHashRef->{'name'} . ' does not exit.' . "\n";
		$self->_printError($msg);
		last;
	    }
	} elsif ($nextLevelName =~ /\[\d+\]$/) {
	    # an array element
	    my $arrayElementName = $self->_getArrayName($nextLevelName);
	    ($tmpNextLevelHashRef, $instHashRef) = $designDBRef->_findNodeByName($arrayElementName,
										 $parentHashRef);
	    $nextLevelHashRef = $tmpNextLevelHashRef;
	    my $memberArrayIndexList = $self->_getArrayMemberIndexList($nextLevelName);
	    if(defined($instHashRef)) {
		# current level name is an instance name.
		($verilogFormat, $offset) = $self->getVerilogBaseAndValue($instHashRef->{'instanceAddr'});

		$bigIntAddr = $self->_getObjectAddress($instHashRef,
						       $memberArrayIndexList,
						       $offset,
						       $totalOffset);
		$totalOffset = $bigIntAddr;
	    } else {
		($verilogFormat, $offset) = $self->getVerilogBaseAndValue($nextLevelHashRef->{'byteOffset'});
		    $bigIntAddr = $self->_getObjectAddress($nextLevelHashRef,
							   $memberArrayIndexList,
							   $offset,
							   $totalOffset);
		$totalOffset = $bigIntAddr;
	    }
	    if($nextLevelHashRef->{'nodeType'} eq 'register') {
		$targetHashRef = $nextLevelHashRef;
	    }
	} else { # not an array
	    # Is this the top level addressmap? keep going!
	    if ($levelIndex == 0) {
		if ($nextLevelName eq $parentHashRef->{'name'}) {
		    next;
		} else {
		    ($nextLevelHashRef, $instHashRef) = $designDBRef->_findNodeByName($nextLevelName, $parentHashRef);
		}
	    } else {
		($nextLevelHashRef, $instHashRef) = $designDBRef->_findNodeByName($nextLevelName, $parentHashRef);
	    }
	    my $i=0;
	    if (!defined($nextLevelHashRef)) {
                my $i=0;
                while (my($pack, $file, $line, $subname, $hasargs, $wantarray) = caller($i++)) {
                    print 'In file: ' . $file . ', line \#: ' . $line . ', subroutine name: ' . $subname . "\n";
                }
                my $msg = 'Could not find node: ' . $nextLevelName . ' in parent: ' .
                          $parentHashRef->{'name'} . "\n";
                $self->_printError($msg);
                exit(1);
            }
            last if ($nextLevelHashRef->{'nodeType'} eq 'field');

            # A sub-register name?
            # If this is a sub-register at the top level, we need to also
            # get the wide regiser, since that's not part of the hierarchical name.

            if(@levelNameList == 1) {
                if($nextLevelHashRef->{'nodeType'} eq 'wideRegister') {
                    ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($nextLevelHashRef->{'byteOffset'});
                    $bigIntAddr = Math::BigInt->new($offset);
                    last;
                } else {
                    ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($nextLevelHashRef->{'byteOffset'});
                    $bigIntAddr = Math::BigInt->new($offset);
                    last;
                }
	    }
            ($nextLevelHashRef, $instHashRef) = $designDBRef->_findNodeByName($nextLevelName,
                                                                              $parentHashRef);
            if ((!defined($nextLevelHashRef) && ($parentHashRef->{'nodeType'} eq 'register')) ||
                (defined($nextLevelHashRef) && ($nextLevelHashRef->{'nodeType'} eq 'field'))) {
                # it's a field that doesn't exist in the database,
                # or a register that doesn't exist.
                my $msg = 'Node: ' . $nextLevelHashRef->{'name'} . ', with parent: ' .
                          $parentHashRef->{'name'} . ' does not exit.' . "\n";
                $self->_printError($msg);
                last;
            }
            if(defined($instHashRef)) {
                ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($instHashRef->{'instanceAddr'});
                $bigIntAddr = Math::BigInt->new($offset);
                $bigIntAddr->badd($totalOffset);
                $totalOffset = $bigIntAddr;
            } else {
                last if (!defined($nextLevelHashRef));
                ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($nextLevelHashRef->{'byteOffset'});
                $bigIntAddr = Math::BigInt->new($offset);
                $bigIntAddr->badd($totalOffset);
                $totalOffset = $bigIntAddr;
                if($nextLevelHashRef->{'nodeType'} eq 'register') {
                    $targetHashRef = $nextLevelHashRef;
                }
            }
        }
        $parentHashRef = $nextLevelHashRef;
    }
    return ($nextLevelHashRef, $bigIntAddr);
}

sub _getObjectAddress {
    my ($self,
	$objHashRef,
	$memberArrayIndexList,
	$offset,
	$totalOffset) = @_;

    my ($bigIntAddr);
    if (defined($objHashRef->{'arrayDimensionInfo'})) {
	$bigIntAddr = Math::BigInt->new($objHashRef->{'arrayElementByteSize'});
	my $addrMultiplier = $designDBRef->_getMultiplierFromArrayIndex($memberArrayIndexList,
									$objHashRef->{'arrayDimensionInfo'});
	$bigIntAddr->bmul($addrMultiplier);
	$bigIntAddr->badd($offset);
	$bigIntAddr->badd($totalOffset);
    } else {
	$bigIntAddr = Math::BigInt->new($offset);
	$bigIntAddr->badd($totalOffset);
    }
    return $bigIntAddr;
}

sub _getArrayName {
    my ($self, $memberName) = @_;

    while ($memberName =~ /\[\d+\]$/) {
	$memberName =~ s/\[\d+\]//;
    }
    return $memberName;
}

sub _getArrayMemberIndexList {
    my ($self, $memberName) = @_;

    my @arrayIndexList;
    while ($memberName =~ /\[(\d+)\]$/) {
	unshift (@arrayIndexList, $1);
	$memberName =~ s/\[\d+\]//;
    }
    return \@arrayIndexList;
}

sub _findNodeByName {
    my ($self, $nodeName, $parentHashRef) = @_;

    my ($nextRegHashRef, $regListRef, $count, $parentIsArray);
    my ($nextInstHashRef, $instListRef, $subRegHashRef);
    my ($errMsg, $memListRef, $nextMemHashRef, $subMemHashRef);

    if($parentHashRef->{'nodeType'} eq 'wideRegister') {
	if (defined($parentHashRef->{'bitfields'})) {
	    foreach my $nextFieldHashRef (@{$parentHashRef->{'bitfields'}}) {
		if($nodeName eq $nextFieldHashRef->{'name'}) {
		    return ($nextFieldHashRef, $nextInstHashRef);
		}
	    }
	}
	foreach $nextRegHashRef (@{$parentHashRef->{'subRegisters'}}) {
	    if($nodeName eq $nextRegHashRef->{'name'}) {
		return ($nextRegHashRef, $nextInstHashRef);
	    }
	}
    } elsif($parentHashRef->{'nodeType'} eq 'wideMemory') {
	foreach $nextMemHashRef (@{$parentHashRef->{'subMemories'}}) {
	    if($nodeName eq $nextMemHashRef->{'name'}) {
		return ($nextMemHashRef, $nextInstHashRef);
	    }
	}
        if(defined($parentHashRef->{'definition'})) {
            for($count = 0; $count < @{$parentHashRef->{'definition'}}; $count++) {
                if(defined($parentHashRef->{'definition'}->[$count]->{'registers'})) {
                    $regListRef = $parentHashRef->{'definition'}->[$count]->{'registers'};
                    foreach $nextRegHashRef (@{$regListRef}) {
                        if($nodeName eq $nextRegHashRef->{'name'}) {
                            return ($nextRegHashRef, $nextInstHashRef);
                        }
                    }
                } elsif(defined($parentHashRef->{'definition'}->[$count]->{'groups'})) {
                    my $grpListRef = $parentHashRef->{'definition'}->[$count]->{'groups'};
		    foreach my $nextGrpHashRef (@{$grpListRef}) {
			if($nodeName eq $nextGrpHashRef->{'name'}) {
			    return ($self->_findAddressmapForInstanceHash($nextGrpHashRef), $nextGrpHashRef);
			}
		    }
                }
            }
        }
        return;
    } else {
	if(defined($parentHashRef->{'definition'})) {
	    for($count = 0; $count < @{$parentHashRef->{'definition'}}; $count++) {
		if(defined($parentHashRef->{'definition'}->[$count]->{'registers'})) {
		    $regListRef = $parentHashRef->{'definition'}->[$count]->{'registers'};
		    foreach $nextRegHashRef (@{$regListRef}) {
			if($nodeName eq $nextRegHashRef->{'name'}) {
			    return ($nextRegHashRef, $nextInstHashRef);
			} elsif($nextRegHashRef->{'nodeType'} eq 'wideRegister') {
			    foreach $subRegHashRef (@{$nextRegHashRef->{'subRegisters'}}) {
				if (($nodeName eq $subRegHashRef->{'name'}) && 
				    defined($parentIsArray = $subRegHashRef->{'parentIsArray'}) &&
				    ($parentIsArray eq 'false')) {
				    return ($subRegHashRef, $nextInstHashRef);
				}
			    }
			}
		    }
		} elsif(defined($parentHashRef->{'definition'}->[$count]->{'topLevelAddressmaps'})) {
		    $instListRef = $parentHashRef->{'definition'}->[$count]->{'topLevelAddressmaps'};
		    foreach $nextInstHashRef (@{$instListRef}) {
			if($nodeName eq $nextInstHashRef->{'name'}) {
			    return ($self->_findAddressmap($nodeName), $nextInstHashRef);
			}
		    }
		} elsif(defined($parentHashRef->{'definition'}->[$count]->{'instances'})) {
		    $instListRef = $parentHashRef->{'definition'}->[$count]->{'instances'};
		    foreach $nextInstHashRef (@{$instListRef}) {
			if($nodeName eq $nextInstHashRef->{'name'}) {
			    return ($self->_findAddressmapForInstanceHash($nextInstHashRef), $nextInstHashRef);
			}
		    }
		} elsif(defined($parentHashRef->{'definition'}->[$count]->{'groups'})) {
		    $instListRef = $parentHashRef->{'definition'}->[$count]->{'groups'};
		    foreach $nextInstHashRef (@{$instListRef}) {
			if($nodeName eq $nextInstHashRef->{'name'}) {
			    return ($self->_findAddressmapForInstanceHash($nextInstHashRef), $nextInstHashRef);
			}
		    }
		} elsif(defined($parentHashRef->{'definition'}->[$count]->{'unions'})) {
		    $instListRef = $parentHashRef->{'definition'}->[$count]->{'unions'};
		    foreach $nextInstHashRef (@{$instListRef}) {
			if($nodeName eq $nextInstHashRef->{'name'}) {
			    return ($self->_findAddressmapForInstanceHash($nextInstHashRef), $nextInstHashRef);
			}
		    }
                } elsif (defined($parentHashRef->{'definition'}->[$count]->{'memories'})) {
		    $memListRef = $parentHashRef->{'definition'}->[$count]->{'memories'};
		    foreach $nextMemHashRef (@{$memListRef}) {
			if($nodeName eq $nextMemHashRef->{'name'}) {
			    return ($nextMemHashRef, $nextInstHashRef);
			} elsif($nextMemHashRef->{'nodeType'} eq 'wideMemory') {
			    foreach $subMemHashRef (@{$nextMemHashRef->{'subMemories'}}) {
				if (($nodeName eq $subMemHashRef->{'name'}) && 
				    defined($parentIsArray = $subMemHashRef->{'parentIsArray'}) &&
				    ($parentIsArray eq 'false')) {
				    return ($subMemHashRef, $nextInstHashRef);
				}
			    }
			}
		    }
		} else {
		    $errMsg = 'Internal Error - Could not find ' . $nodeName . ' in Perl database.';
		    $self->_printError($errMsg);
		    exit(1);
		}
	    }
	} elsif (defined($parentHashRef->{'bitfields'})) {
	    foreach my $nextFieldHashRef (@{$parentHashRef->{'bitfields'}}) {
		return ($nextFieldHashRef, $nextInstHashRef) if($nodeName eq $nextFieldHashRef->{'name'});
	    }
	}
    }
    return;
}
sub _findAddressmapNodeByName {
    my ($self, $nodeName, $parentHashRef) = @_;

    my ($count, $addrmapHashRef, $errMsg);
    my ($nextInstHashRef, $instListRef);

    if($parentHashRef->{'name'} eq $nodeName) {
	return $parentHashRef;
    } else {
	if(defined($parentHashRef->{'definition'})) {
	    for($count = 0; $count < @{$parentHashRef->{'definition'}}; $count++) {
		if(defined($parentHashRef->{'definition'}->[$count]->{'instances'})) {
		    $instListRef = $parentHashRef->{'definition'}->[$count]->{'instances'};
		    foreach $nextInstHashRef (@{$instListRef}) {
			if($nodeName eq $nextInstHashRef->{'name'}) {
			    return ($self->_findAddressmapForInstanceHash($nextInstHashRef), $nextInstHashRef);
			} else {
                            $addrmapHashRef = $self->_findAddressmapForInstanceHash($nextInstHashRef);
			    return $self->_findAddressmapNodeByName($nodeName, $addrmapHashRef);
			}
		    }
		} elsif(defined($parentHashRef->{'definition'}->[$count]->{'groups'})) {
		    $instListRef = $parentHashRef->{'definition'}->[$count]->{'groups'};
		    foreach $nextInstHashRef (@{$instListRef}) {
			if($nodeName eq $nextInstHashRef->{'name'}) {
			    return ($self->_findAddressmapForInstanceHash($nextInstHashRef), $nextInstHashRef);
			} else {
                            $addrmapHashRef = $self->_findAddressmapForInstanceHash($nextInstHashRef);
			    return $self->_findAddressmapNodeByName($nodeName, $addrmapHashRef);
			}
		    }
		} else {
                    my $i=0;
                    while (my($pack, $file, $line, $subname, $hasargs, $wantarray) = caller($i++)) {
                        print 'In file: ' . $file . ', line \#: ' . $line . ', subroutine name: ' .
                              $subname . "\n";
                    }
                    $errMsg = 'Internal Error - Could not find ' . $nodeName . ' in Perl database.';
                    $self->_printError($errMsg);
                    exit(1);
                }
            }
        }
    }
    return;
}

sub _findFieldNodeByName {
    my($self, $fieldName, $regHashRef) = @_;
    foreach my $fieldHashRef (@{$regHashRef->{'bitfields'}}) {
	if($fieldHashRef->{'name'} eq $fieldName) {
	    return $fieldHashRef;
	}
    }
    return;
}
sub _designHasFieldType {
    my ($self, $fieldType, $addrmapHashRef) = @_;

    my ($groupListRef, $nextGroupHashRef, $groupAddrmap);
    my ($nextRegHashRef, $regListRef, $count);
    my ($nextInstHashRef, $instListRef, $instAddrmap);
    my ($nextFieldHashRef);

    my $found = 0;
    my $retVal = 0;
    if(defined($addrmapHashRef->{'definition'})) {
	for($count = 0; (!$found && ($count < @{$addrmapHashRef->{'definition'}})); $count++) {
	    if(defined($addrmapHashRef->{'definition'}->[$count]->{'registers'})) {
		$regListRef = $addrmapHashRef->{'definition'}->[$count]->{'registers'};
		foreach $nextRegHashRef (@{$regListRef}) {
		    if($nextRegHashRef->{'external'} eq 'false') {
			if($fieldType eq 'reset') {
			    if($self->_regHasResetValue($nextRegHashRef)) {
				return 1;
			    }
			} else {
			    foreach $nextFieldHashRef (@{$nextRegHashRef->{'bitfields'}}) {
				if($nextFieldHashRef->{'fieldType'} eq $fieldType) {
				    $found = 1;
				    return 1;
				}
			    }
			}
		    }
		}
	    } elsif(defined($addrmapHashRef->{'definition'}->[$count]->{'instances'})) {
		$instListRef = $addrmapHashRef->{'definition'}->[$count]->{'instances'};
		foreach $nextInstHashRef (@{$instListRef}) {
		    $instAddrmap = $designDBRef->_findAddressmapForInstanceHash($nextInstHashRef);
		    $retVal = $self->_designHasFieldType($fieldType, $instAddrmap);
		    if ($retVal) {
			return ($retVal);
		    }
		}
	    } elsif(defined($addrmapHashRef->{'definition'}->[$count]->{'groups'})) {
		$groupListRef = $addrmapHashRef->{'definition'}->[$count]->{'groups'};
		foreach $nextGroupHashRef (@{$groupListRef}) {
		    $groupAddrmap = $designDBRef->_findAddressmapForInstanceHash($nextGroupHashRef);
		    $retVal = $self->_designHasFieldType($fieldType, $groupAddrmap);
		    $found = 1;
		    last if($retVal);
		}
	    }
	}
    }
    return $retVal;
}
sub _getNextFieldHash {
    my ($self, $csrHash) = @_;

    my $fieldAnonArray = $csrHash->{'bitfields'};
    my $arrayIndex=0;
    return sub {
	return if(!defined($fieldAnonArray));
	if ($arrayIndex >= @{$fieldAnonArray}) {
	    return undef;
	} else {
	    return $fieldAnonArray->[$arrayIndex++];
	}
    }
}
sub _regHasResetValue {
    my ($self, $regHash) = @_;
    my $fieldHashSub = $self->_getNextFieldHash($regHash);
    while (my $fieldHash = $fieldHashSub->()) {
	if (($fieldHash->{'external'} eq 'false') && defined($fieldHash->{'resetValueInfo'})) {
	    return 1;
	}
    }
    return 0;
}

sub _genRefNameList {
    my($self,
       $addrmapHashRef,
       $refNameHashListRef) = @_;

    my ($instModule, $nextRegHashRef, $regListRef, $count);
    my ($nextInstHashRef, $instListRef, $instAddrmap, $nextGrpHashRef);
    my ($nextFieldHashRef, $groupListRef, $groupAddrmap, $subRegHashRef);

    if(defined($addrmapHashRef->{'definition'})) {
	for($count = 0; $count < @{$addrmapHashRef->{'definition'}}; $count++) {
	    if(defined($addrmapHashRef->{'definition'}->[$count]->{'registers'})) {
		$regListRef = $addrmapHashRef->{'definition'}->[$count]->{'registers'};
		foreach $nextRegHashRef (@{$regListRef}) {
		    # the register itself may have ports with reference (write_enable, for example).
		    $self->_expandRefNames($nextRegHashRef,
					   $nextRegHashRef,
					   $refNameHashListRef,
					   $addrmapHashRef,);
		    foreach $nextFieldHashRef (@{$nextRegHashRef->{'bitfields'}}) {
			$self->_expandRefNames($nextFieldHashRef,
					       $nextRegHashRef,
					       $refNameHashListRef,
					       $addrmapHashRef);
		    }
		    if ($nextRegHashRef->{'nodeType'} eq 'wideRegister') {
			foreach $subRegHashRef (@{$nextRegHashRef->{'subRegisters'}}) {
			    $self->_expandRefNames($subRegHashRef,
						   $subRegHashRef,
						   $refNameHashListRef,
						   $addrmapHashRef);
			    foreach $nextFieldHashRef (@{$subRegHashRef->{'bitfields'}}) {
				$self->_expandRefNames($nextFieldHashRef,
						       $subRegHashRef,
						       $refNameHashListRef,
						       $addrmapHashRef);
			    }
			}
		    }			
		}
	    } elsif(defined($addrmapHashRef->{'definition'}->[$count]->{'instances'})) {
		$instListRef = $addrmapHashRef->{'definition'}->[$count]->{'instances'};
		foreach $nextInstHashRef (@{$instListRef}) {
		    $instAddrmap = $self->_findAddressmapForInstanceHash($nextInstHashRef);
		    $self->_genRefNameList($instAddrmap,
					   $refNameHashListRef);
		}
	    } elsif(defined($addrmapHashRef->{'definition'}->[$count]->{'groups'})) {
		$groupListRef = $addrmapHashRef->{'definition'}->[$count]->{'groups'};
		foreach $nextGrpHashRef (@{$groupListRef}) {
		    $groupAddrmap = $self->_findAddressmapForInstanceHash($nextGrpHashRef);
		    $self->_genRefNameList($groupAddrmap,
					   $refNameHashListRef);
		}
	    }
	}
    }
}
sub _expandRefNames {
    my($self,
       $fieldHash,
       $regHash,
       $refNameHashListRef,
       $currAddrmapHashRef) = @_;

    my (@intType, @intInfo);
    my (@portType, @portInfo, @hierLevelNameList);
    my ($nextExpandedRefNameHash, $expandedRefNameHashList);

    if(defined($fieldHash->{'ports'})) {
	for(my $portCount = 0; $portCount < @{$fieldHash->{'ports'}}; $portCount++) {
	    @portType = keys %{$fieldHash->{'ports'}->[$portCount]};
	    @portInfo = values %{$fieldHash->{'ports'}->[$portCount]};
	    if (defined($portInfo[0]->{'concatenation'})) {
		my $concatPorts = $portInfo[0]->{'concatenation'};
		for (my $concatIndex=0; $concatIndex < @$concatPorts; $concatIndex++) {
		    my $nextConcatPort = $concatPorts->[$concatIndex];
		    if(defined($nextConcatPort->{'hierarchicalRefName'})) {
			$self->_addToRefNameHashList($nextConcatPort,
						     $fieldHash,
						     $regHash,
						     $refNameHashListRef,
						     $currAddrmapHashRef);
		    }
		}
	    }
	    if(defined($portInfo[0]->{'hierarchicalRefName'})) {
		$self->_addToRefNameHashList($portInfo[0],
					     $fieldHash,
					     $regHash,
					     $refNameHashListRef,
					     $currAddrmapHashRef);
	    }
	}
    }
    if(defined($fieldHash->{'interruptInfo'})) {
	my $intInfoHashRef = $fieldHash->{'interruptInfo'};
	if(defined($intInfoHashRef->{'hierarchicalRefName'})) {
	    $self->_addToRefNameHashList($intInfoHashRef,
					 $fieldHash,
					 $regHash,
					 $refNameHashListRef,
					 $currAddrmapHashRef);
	}
    }
}
sub _addToRefNameHashList {
    my ($self,
	$referenceInfoHash,
	$fieldHash,
	$regHash,
	$refNameHashListRef,
	$currAddrmapHashRef) = @_;

    my (@hierLevelNameList, $isSubReg);
    my ($nextExpandedRefNameHash, $expandedRefNameHashList);
    my $byteEndian = $regHash->{'byteEndian'};

    @hierLevelNameList = split('\.', $referenceInfoHash->{'hierarchicalRefName'});
    ($expandedRefNameHashList) = $self->_expandRefNameMembers(\@hierLevelNameList,
							      $regHash,
							      $currAddrmapHashRef,
							      $referenceInfoHash->{'topLevelAddressmapName'},
							      $byteEndian);
    if(!$self->_isHierarchicalNameInList($referenceInfoHash->{'hierarchicalRefName'},
                                         $regHash,
					 $currAddrmapHashRef,
                                         $refNameHashListRef)) {
	$nextExpandedRefNameHash = {};
	if(defined($regHash->{'aliasOf'})) {
	    $nextExpandedRefNameHash->{'aliasedReg'} = $regHash;
	}
	if ((defined($isSubReg = $regHash->{'subRegisterFlag'}) && ($isSubReg eq 'true')) ||
	    ($regHash->{'nodeType'} eq 'wideRegister')) {
	    if ($regHash->{'name'} ne $fieldHash->{'name'}) {
		$nextExpandedRefNameHash->{'parentName'} = join('_', ($regHash->{'name'}, $fieldHash->{'name'}));
	    } else {
		$nextExpandedRefNameHash->{'parentName'} = $regHash->{'name'};
	    }
	    $nextExpandedRefNameHash->{'registerHash'} = $regHash;
	}
	$nextExpandedRefNameHash->{'hierarchicalRefName'} = $referenceInfoHash->{'hierarchicalRefName'};
	$nextExpandedRefNameHash->{'addressmapHierarchy'} = $currAddrmapHashRef->{'hierarchicalIdentifier'};
	$nextExpandedRefNameHash->{'expandedNameList'} = $expandedRefNameHashList;
	push(@{$refNameHashListRef}, $nextExpandedRefNameHash);
    }
}

sub _isHierarchicalNameInList {
    my($self,
       $hierName,
       $regHash,
       $currAddrmapHashRef,
       $refNameHashListRef) = @_;

    my ($aliasedRegHash, $tmpHierNameList, $subRegFlag);
    for(my $count=0; $count < @{$refNameHashListRef}; $count++) {
	if(defined($regHash->{'aliasOf'})) { 
	    if(defined($aliasedRegHash = $refNameHashListRef->[$count]->{'aliasedReg'})) {
                if(($aliasedRegHash->{'name'} eq $regHash->{'name'}) &&
                   ($refNameHashListRef->[$count]->{'hierarchicalRefName'} eq $hierName)) {
                    return 1;
                }
	    }
	} elsif ($refNameHashListRef->[$count]->{'hierarchicalRefName'} eq $hierName) {
	    if ($refNameHashListRef->[$count]->{'addressmapHierarchy'} eq 
		$currAddrmapHashRef->{'hierarchicalIdentifier'}) {
		if (defined($subRegFlag = $regHash->{'subRegisterFlag'}) && ($subRegFlag eq 'true')) {
		    next if (!defined($refNameHashListRef->[$count]->{'registerHash'}));
		    if ($regHash->{'name'} eq $refNameHashListRef->[$count]->{'registerHash'}->{'name'}) {
			# We've already put this reference in this sub-register in the list.
			return 1;
		    } else {
			next;
		    }
		} else {
		    return 1;
		}
	    }		
	}
    }
    return 0;
}

sub _expandRefNameMembers {
    my($self,
       $hierNameListRef,
       $regHash,
       $currAddrmapHashRef,
       $topAddrmapNameInRefName,
       $byteEndian) = @_;

    my ($csrHash, $tmpCsrHash, $instInRefHash, $grpInRefHash);
    my ($regInRefHash, $tmpMemberHash, $addrmapName, $tmpGrpHash);
    my ($tmpInstHash, @memberHashList, $parentHash);

    # pop the top addressmap name
    my @tmpHierNameList = @$hierNameListRef;
    shift(@$hierNameListRef);

    for(my $hierNameCount=0; $hierNameCount < @{$hierNameListRef}; $hierNameCount++) {
	# is hierarchical member an instance name?
	$tmpMemberHash = {};
	$tmpMemberHash->{'memberName'} = $hierNameListRef->[$hierNameCount];
	if (!defined($addrmapName)) {
	    my @partialHier;
	    for (my $tmpIndex = 0; $tmpIndex <= ($hierNameCount+1); $tmpIndex++) {
		push (@partialHier, $tmpHierNameList[$tmpIndex]);
	    }
	    $addrmapName = $self->_getAddrmapName($currAddrmapHashRef,
						  $topAddrmapNameInRefName,
						  \@partialHier);
	}
	if(defined($tmpInstHash = $self->_isHierMemberInstName($hierNameListRef->[$hierNameCount],
							       $addrmapName))) {
	    if(defined($tmpInstHash->{'arrayDimensionInfo'})) {
		$self->_addMemberArrayInfo($tmpMemberHash, $tmpInstHash);
	    }
	    push(@memberHashList, $tmpMemberHash);
	    # for the remaining names in the hierarchical name list,
	    # start with the addressmap defintion for this instance.
	    # $addrmapName = $tmpInstHash->{'name'};
	    $addrmapName .= '_' . $tmpInstHash->{'name'};
	    $instInRefHash = $tmpInstHash;
	} elsif(defined($tmpGrpHash = $self->_isHierMemberGrpName($hierNameListRef->[$hierNameCount],
								  $currAddrmapHashRef,
								  $addrmapName))) {
	    if(defined($tmpGrpHash->{'arrayDimensionInfo'})) {
		$self->_addMemberArrayInfo($tmpMemberHash, $tmpGrpHash);
	    }
	    push(@memberHashList, $tmpMemberHash);
	    # for the remaining names in the hierarchical name list,
	    # start with the addressmap defintion for this instance.
	    # $addrmapName = $tmpGrpHash->{'name'};
	    $addrmapName .= '_' . $tmpGrpHash->{'name'};
	    $grpInRefHash = $tmpGrpHash;
	} elsif(defined($tmpCsrHash = $self->_isHierMemberRegName($hierNameListRef->[$hierNameCount],
								  $regHash,
								  $currAddrmapHashRef,
								  $addrmapName))) {
	    if(defined($tmpCsrHash->{'arrayDimensionInfo'})) {
		$self->_addMemberArrayInfo($tmpMemberHash, $tmpCsrHash);
	    }
	    my $subRegFlag = 0;
	    if (defined($subRegFlag = $tmpCsrHash->{'subRegisterFlag'}) && ($subRegFlag eq 'true')) {
		$subRegFlag = 1;
		$tmpMemberHash->{'memberName'} = $tmpCsrHash->{'name'};
	    }

	    if (!defined($instInRefHash) &&
		($currAddrmapHashRef->{'nodeType'} eq 'addressmapInstance') &&
		($currAddrmapHashRef->{'name'} ne $topAddrmapNameInRefName)) {

		# No address map instance name is included in the reference name;
		# There is more than one level of address map hierarchy, and this
		# reference name refers to an object in a sub-addressmap.

		if (defined($subRegFlag = $tmpCsrHash->{'subRegisterFlag'}) && ($subRegFlag eq 'true')) {
		    # The hierarchical ref name includes the group name.
		    $self->_prependParentMember(\@memberHashList, $tmpCsrHash);
		}

		my $nextLevelUpAddrmap = $currAddrmapHashRef;
		my $nextLevelUpAddrmapName = $currAddrmapHashRef->{'name'};
		$self->_prependWithHierarchy($currAddrmapHashRef,
					     $topAddrmapNameInRefName,
					     $tmpCsrHash,
					     \@memberHashList);
	    } elsif (defined($grpInRefHash) &&
		     ($currAddrmapHashRef->{'nodeType'} eq 'group') &&
		     ($currAddrmapHashRef->{'name'} ne $topAddrmapNameInRefName)) {
		if (defined($subRegFlag = $tmpCsrHash->{'subRegisterFlag'}) && ($subRegFlag eq 'true')) {
		    # The hierarchical ref name includes the group name.
		    my $memberHash = {};
		    $memberHash->{'memberName'} = $tmpCsrHash->{'parentName'};
		    push (@memberHashList, $memberHash);
		}
	    } else {
		if (defined($subRegFlag = $tmpCsrHash->{'subRegisterFlag'}) &&
		    ($subRegFlag eq 'true')) {
		    my $memberHash = {};
		    $memberHash->{'memberName'} = $tmpCsrHash->{'parentName'};
		    push (@memberHashList, $memberHash);
		}
	    }
	    push(@memberHashList, $tmpMemberHash);
	    $csrHash = $tmpCsrHash;
	} else {
	    # must be a field name, in which case it's a field in $tmpCsrHash.
	    foreach my $fieldHash (@{$csrHash->{'bitfields'}}) {
		my ($subRegFlag, $fieldNameInRef);
		$fieldNameInRef = $hierNameListRef->[$hierNameCount];
		if (defined($subRegFlag = $csrHash->{'subRegisterFlag'}) && ($subRegFlag eq 'true')) {
		    my $subPart = $fieldHash->{'name'};
		    $subPart =~ s/$fieldNameInRef//;
		    $fieldNameInRef .= $subPart;
		}
		if($fieldHash->{'name'} eq $fieldNameInRef) {
		    if(defined($fieldHash->{'arrayDimensionInfo'})) {
			$tmpMemberHash->{'arrayInfo'} = $fieldHash->{'arrayDimensionInfo'};
			$tmpMemberHash->{'arrayMinIndices'} = $designDBRef->_getArrayMinIndices($fieldHash->{'arrayDimensionInfo'});
			$tmpMemberHash->{'arrayMaxIndices'} = $designDBRef->_getArrayMaxIndices($fieldHash->{'arrayDimensionInfo'});
			$tmpMemberHash->{'size'} = $designDBRef->_getArraySize($fieldHash->{'arrayDimensionInfo'});
			$tmpMemberHash->{'memberIndex'} = $designDBRef->_getArrayMinIndices($fieldHash->{'arrayDimensionInfo'});
		    } else {
			$tmpMemberHash->{'memberName'} = $fieldNameInRef;
		    }
		    push(@memberHashList, $tmpMemberHash);
		    last;
		}
	    }
	}
    }
    my $tmpInstMemberHash = {};
    $tmpInstMemberHash->{'memberName'} = $topAddrmapNameInRefName;
    unshift(@memberHashList, $tmpInstMemberHash);
    return \@memberHashList;
}

sub _getAddrmapName {
    my ($self,
	$currAddrmapHashRef,
	$topAddrmapNameInRefName,
	$currMemberHierName) = @_;

    my $addrmapName;
    if (($currAddrmapHashRef->{'nodeType'} eq 'group') ||
	($currAddrmapHashRef->{'nodeType'} eq 'addressmapInstance')) {
	if ($currAddrmapHashRef->{'name'} ne $topAddrmapNameInRefName) {
	    my $parentAddrmap = $self->_getParentHash($currAddrmapHashRef);
	    my $currMemberHierLevels = @$currMemberHierName;
	    my @parentHierId = split('\.', $parentAddrmap->{'hierarchicalIdentifier'});
	    my $parentLevels = @parentHierId;
	    while ($currMemberHierLevels <= $parentLevels) {
		$parentAddrmap = $self->_getParentHash($parentAddrmap);
		$currMemberHierLevels = @$currMemberHierName;
		@parentHierId = split('\.', $parentAddrmap->{'hierarchicalIdentifier'});
		$parentLevels = @parentHierId;
	    }
	    $addrmapName = $parentAddrmap->{'hierarchicalName'};
	} else {
	    $addrmapName = $currAddrmapHashRef->{'hierarchicalName'};
	}
    } else {
	$addrmapName = $currAddrmapHashRef->{'hierarchicalName'}
    }
    return $addrmapName;
}

sub _prependParentMember {
    my ($self, $memberHashList, $memberObj) = @_;

    my $memberHash = {};
    $memberHash->{'memberName'} = $memberObj->{'parentName'};
    unshift (@$memberHashList, $memberHash);
}

sub _prependWithHierarchy {
    my ($self,
	$currAddrmapHashRef,
	$topAddrmapNameInRefName,
	$csrHash,
	$memberHashList) = @_;

    my $nextLevelUpAddrmap = $currAddrmapHashRef;
    my $nextLevelUpAddrmapName = $currAddrmapHashRef->{'name'};
    while ($nextLevelUpAddrmapName ne $topAddrmapNameInRefName) {
	my $tmpInstMemberHash = {};
	$tmpInstMemberHash->{'memberName'} = $nextLevelUpAddrmapName;
	if(defined($nextLevelUpAddrmap->{'arrayDimensionInfo'})) {
	    $self->_addMemberArrayInfo($tmpInstMemberHash, $nextLevelUpAddrmap);
	}
	unshift(@$memberHashList, $tmpInstMemberHash);
	my $tmpNextLevelUpAddrmap = $self->_getParentHash($nextLevelUpAddrmap);
	$nextLevelUpAddrmapName = $tmpNextLevelUpAddrmap->{'name'};
	$nextLevelUpAddrmap = $tmpNextLevelUpAddrmap;
    }
}

sub _addMemberArrayInfo {
    my ($self, $memberHash, $instHash) = @_;

    $memberHash->{'arrayInfo'} = $instHash->{'arrayDimensionInfo'};
    $memberHash->{'size'} = $designDBRef->_getArraySize($instHash->{'arrayDimensionInfo'});
    $memberHash->{'arrayMinIndices'} = $self->_getArrayMinIndices($instHash->{'arrayDimensionInfo'});
    $memberHash->{'arrayMaxIndices'} = $self->_getArrayMaxIndices($instHash->{'arrayDimensionInfo'});
    $memberHash->{'memberIndex'} = $self->_getArrayMinIndices($instHash->{'arrayDimensionInfo'});
}

sub _isHierMemberInstName {
    my($self, $hierMemberName, $addrmapName) = @_;

    my $addrmapHashRef = $self->_findAddressmap($addrmapName);
    my $instanceHashList = $self->_getInstanceHashList($addrmapHashRef);
    while (my $nextInstHash = $self->_getNextInstanceHash($instanceHashList)) {
	if($nextInstHash->{'name'} eq $hierMemberName) {
	    $self->set_nextInstHashIndex(-1);
	    return $nextInstHash;
	}
    }
    return;
}
sub _isHierMemberGrpName {
    my($self,
       $hierMemberName,
       $currAddrmapHashRef,
       $addrmapName) = @_;

    my $addrmapHashRef = $self->_findAddressmap($addrmapName);
    my $groupHashList = $self->_getGroupHashList($addrmapHashRef);
    while (my $nextGrpHash = $self->_getNextGroupHash($groupHashList)) {
	if($nextGrpHash->{'name'} eq $hierMemberName) {
	    $self->set_nextGrpHashIndex(-1);
	    return $nextGrpHash;
	}
    }
    return;
}

sub _isHierMemberRegName {
    my($self,
       $hierMemberName,
       $regHash,
       $currAddrmapHashRef,
       $addrmapName) = @_;

    my ($tmpRegHash, $registerList);
    my ($subRegFlag, $parentName, $subRegPart, $hierMemberName_SubReg);

    if (defined($subRegFlag = $regHash->{'subRegisterFlag'}) &&
	($subRegFlag eq 'true')) {
	$subRegPart = $self->_getPartName($regHash);
	$hierMemberName_SubReg = $hierMemberName;
	$hierMemberName_SubReg .= $subRegPart;
    }
    if ($addrmapName ne $currAddrmapHashRef->{'hierarchicalName'}) {
	my $tmpAddrmapNode = $self->_findAddressmap($addrmapName);
	$registerList = $self->_getRegHashList($tmpAddrmapNode);
    } else {
	$registerList = $self->_getRegHashList($currAddrmapHashRef);
    }
    if (defined($hierMemberName_SubReg) &&
	defined($tmpRegHash = $self->_findRegWithHierMemberName($hierMemberName_SubReg, $registerList))) {
	return $tmpRegHash;
    } elsif (defined($tmpRegHash = $self->_findRegWithHierMemberName($hierMemberName, $registerList))) {
	return $tmpRegHash;
    }
    return;
}

sub _findRegWithHierMemberName {
    my ($self,
	$hierMemberName,
	$registerList) = @_;

    while(my $nextRegHash = $self->_getNextRegHashInAddrMap($registerList)) {
	if($nextRegHash->{'name'} eq $hierMemberName) {
	    $self->set_nextRegHashIndex(-1);
	    return $nextRegHash;
	}
	if ($nextRegHash->{'nodeType'} eq 'wideRegister') {
	    foreach my $nextSubReg (@{$nextRegHash->{'subRegisters'}}) {
 		if($nextSubReg->{'name'} eq $hierMemberName) {
		    $self->set_nextRegHashIndex(-1);
		    return $nextSubReg;
		}
	    }
	}
    }
    return;
}

sub _getPartName {
    my ($self, $objHashRef) = @_;

    my $parentName = $objHashRef->{'parentName'};
    my $subPart = $objHashRef->{'name'};
    $subPart =~ s/$parentName//;
    return $subPart;
}

sub _getInstanceHashList {
    my ($self, $addrmapHashRef) = @_;
    for(my $count = 0; $count < @{$addrmapHashRef->{'definition'}}; $count++) {
	if(defined($addrmapHashRef->{'definition'}->[$count]->{'instances'})) {
	    return ($addrmapHashRef->{'definition'}->[$count]->{'instances'});
	}
    }
    return;
}
sub _getGroupHashList {
    my ($self, $addrmapHashRef) = @_;

    # memory objects may have virtual groups and/or registers, in which
    # case there will be a definition block;

    return if ((($addrmapHashRef->{'nodeType'} eq 'memory') ||
                ($addrmapHashRef->{'nodeType'} eq 'wideMemory')) &&
	       !defined($addrmapHashRef->{'definition'}));
    for(my $count = 0; $count < @{$addrmapHashRef->{'definition'}}; $count++) {
	if(defined($addrmapHashRef->{'definition'}->[$count]->{'groups'})) {
	    return ($addrmapHashRef->{'definition'}->[$count]->{'groups'});
	}
    }
    return;
}
sub _getNextInstanceHash {
    my ($self, $instanceHashList) = @_;
    return if(!defined($instanceHashList));
    my $instIndex = $self->get_nextInstHashIndex();
    if ($instIndex >= @{$instanceHashList}-1) {
	$self->set_nextInstHashIndex(-1);
	return;
    } else {
	$self->set_nextInstHashIndex(++$instIndex);
	return $instanceHashList->[$instIndex];
    }
}
sub _getNextGroupHash {
    my ($self, $groupHashList) = @_;
    return if(!defined($groupHashList));
    my $grpIndex = $self->get_nextGrpHashIndex();
    if ($grpIndex >= @{$groupHashList}-1) {
	$self->set_nextGrpHashIndex(-1);
	return;
    } else {
	$self->set_nextGrpHashIndex(++$grpIndex);
	return $groupHashList->[$grpIndex];
    }
}
sub _getUnionHashList {
    my ($self, $addrmapHashRef) = @_;
    return if (!defined($addrmapHashRef->{'definition'}));
    for(my $count = 0; $count < @{$addrmapHashRef->{'definition'}}; $count++) {
	if(defined($addrmapHashRef->{'definition'}->[$count]->{'unions'})) {
	    return ($addrmapHashRef->{'definition'}->[$count]->{'unions'});
	}
    }
    return;
}
sub _getRegHashList {
    my ($self, $addrmapHashRef) = @_;
    return if (!defined($addrmapHashRef->{'definition'}));
    for(my $count = 0; $count < @{$addrmapHashRef->{'definition'}}; $count++) {
	if(defined($addrmapHashRef->{'definition'}->[$count]->{'registers'})) {
	    return ($addrmapHashRef->{'definition'}->[$count]->{'registers'});
	}
    }
    return;
}
sub _getMemoryHashList {
    my ($self, $addrmapHashRef) = @_;
    if ($addrmapHashRef->{'nodeType'} eq 'wideMemory') {
	if(defined($addrmapHashRef->{'subMemories'})) {
	    return ($addrmapHashRef->{'subMemories'});
	}
        return undef;
    }
    for(my $count = 0; $count < @{$addrmapHashRef->{'definition'}}; $count++) {
	if(defined($addrmapHashRef->{'definition'}->[$count]->{'memories'})) {
	    return ($addrmapHashRef->{'definition'}->[$count]->{'memories'});
	}
    }
    return undef;
}
sub _getNextRegHashInAddrMap {
    my ($self, $regHashList) = @_;
    return if(!defined($regHashList));
    my $regIndex = $self->get_nextRegHashIndex();
    if ($regIndex >= @{$regHashList}-1) {
	$self->set_nextRegHashIndex(-1);
	return undef;
    } else {
	$self->set_nextRegHashIndex(++$regIndex);
	return $regHashList->[$regIndex];
    }
}
sub _regHasPorts {
    my ($self, $regHashRef) = @_;
    my ($portCount, @portType);

    if(defined($regHashRef->{'ports'})) {
	for($portCount = 0; $portCount < @{$regHashRef->{'ports'}}; $portCount++) {
	    @portType = keys %{$regHashRef->{'ports'}->[$portCount]};
	    if ($portType[0] ne 'resetPort') {
		return 1;
	    }
	}
	return 1;
    } else {
	foreach my $fieldHash (@{$regHashRef->{'bitfields'}})  {
	    if(defined($fieldHash->{'ports'})) {
		for ($portCount = 0; $portCount < @{$fieldHash->{'ports'}}; $portCount++) {
		    @portType = keys %{$fieldHash->{'ports'}->[$portCount]};
		    if ($portType[0] ne 'resetPort') {
			return 1;
		    }
		}
	    }
	}
    }
    return 0;
}
sub getTopLevelAddressmapCount {
    my $self = shift;
    if(defined($self->{'topLevelAddressmaps'})) {
	return @{$self->{'topLevelAddressmaps'}};
    } else {
	my $errMsg = "\t" . 'topLevelAddressmaps array not defined in design package.';
	$self->_printError ($errMsg);
	exit(1);
    }
}
sub getAddrmapDefCount {
    my $self = shift;
    if (defined($self->{'definitions'})) {
	return @{$self->{'definitions'}};
    } else {
	return 0;
    }
}
sub _findAddressmap {
    my($self, $addrmapName) = @_;

    my $count=0;
    my $elemCount = $self->getTopLevelAddressmapCount();
    while ($count < $elemCount) {
	if ($self->{'topLevelAddressmaps'}->[$count]->{'hierarchicalIdentifier'} eq $addrmapName) {
	    return $self->{'topLevelAddressmaps'}->[$count];
	}
	$count++;
    }
    $count=0;
    $elemCount = $self->getAddrmapDefCount();
    while ($count < $elemCount) {
	if ($self->{'definitions'}->[$count]->{'hierarchicalName'} eq $addrmapName) {
	    return $self->{'definitions'}->[$count];
	}
	$count++;
    }
    return;
}
sub _findAddressmapForInstanceHash {
    my($self, $instanceHashRef) = @_;

    my $count=0;
    my $elemCount = $self->getAddrmapDefCount();
    while ($count < $elemCount) {
	if ($self->{'definitions'}->[$count]->{'hierarchicalIdentifier'} eq
            $instanceHashRef->{'hierarchicalIdentifier'}) {
	    return $self->{'definitions'}->[$count];
	}
	$count++;
    }
    return;
}

sub _getMultiplierFromArrayIndex {
    my ($self,
	$arrayIndexList,
	$arrayDimensionInfo) = @_;

    return 0 if (!defined($arrayIndexList));
    my $multiplier = 0;
    my $tmpMulVal = 0;
    my @mulValList;
    if (@$arrayDimensionInfo > 1) {
	# Start with the next array dimension that changes least frequently.
	for (my $index = 0; $index < @$arrayIndexList-1; $index++) {
	    my $currDimValue = $arrayIndexList->[$index] - $arrayDimensionInfo->[$index]->{'arrayMin'};
	    # Continue with dimensions that change more frequently
	    my $tmpCount = 1;
	    for (my $lowerIndex = $index+1; $lowerIndex < @$arrayIndexList; $lowerIndex++) {
		$tmpCount *= $arrayDimensionInfo->[$lowerIndex]->{'arrayCount'};
	    }
	    my $newMulVal = $tmpCount * $currDimValue;
	    push (@mulValList, $newMulVal);
	}
	foreach my $nextMulVal (@mulValList) {
	    $tmpMulVal += $nextMulVal;
	}
    }
    # Array dimension that changes the fastest.
    $multiplier = $tmpMulVal + $arrayIndexList->[@$arrayDimensionInfo-1] -
                  $arrayDimensionInfo->[0]->{'arrayMin'};
    return $multiplier;
}

sub _getArraySize {
    my ($self, $arrayDimList) = @_;

    my $arraySize = 1;
    for (my $index=0; $index < @$arrayDimList; $index++) {
	$arraySize *= $arrayDimList->[$index]->{'arrayCount'};
    }
    return Math::BigInt->new($arraySize);
}

sub _getArrayElementName {
    my ($self, $nameStr, $arrayIndexList) = @_;

    for (my $index = 0; $index < @{$arrayIndexList}; $index++) {
	$nameStr .= '[' . $arrayIndexList->[$index] . ']'; 
    }
    return $nameStr;
}

sub _getArrayMinIndices {
    my ($self, $arrayDimensionInfo) = @_;

    my @minIndexList;
    for (my $index = 0; $index < @$arrayDimensionInfo; $index++) {
	push (@minIndexList, $arrayDimensionInfo->[$index]->{'arrayMin'});
    }
    return \@minIndexList;
}
sub _getArrayMaxIndices {
    my ($self, $arrayDimensionInfo) = @_;

    my @maxIndexList;
    for (my $index = 0; $index < @$arrayDimensionInfo; $index++) {
	push (@maxIndexList, $arrayDimensionInfo->[$index]->{'arrayMax'});
    }
    return \@maxIndexList;
}

sub _arraySizeGTOne {
    my ($self, $arrayDimensionInfo) = @_;
    for (my $index = 0; $index < @$arrayDimensionInfo; $index++) {
	return 1 if ($arrayDimensionInfo->[$index]->{'arrayMax'} >
		     $arrayDimensionInfo->[$index]->{'arrayMin'});
    }
    return 0;
}

sub _getArrayIndicesMidWay {
    my ($self, $arrayDimensionInfo) = @_;

    my @midwayIndex;
    for (my $index = @$arrayDimensionInfo-1; $index >= 0; $index--) {
	push (@midwayIndex, int($arrayDimensionInfo->[$index]->{'arrayCount'}/2));
    }
    return \@midwayIndex;
}

sub _compareArrayIndex {
    my ($self, $indexList1, $indexList2) = @_;

    foreach (my $index = 0; $index < @$indexList1; $index++) {
	return 1 if ($indexList1->[$index] > $indexList2->[$index]);
    }
    foreach (my $index = 0; $index < @$indexList1; $index++) {
	return -1 if ($indexList1->[$index] < $indexList2->[$index]);
    }
    # They're equal
    return 0;
}

sub _incrementArrayIndex {
    my ($self, $arrayDimList, $arrayDimensionInfo) = @_;

    my $arrayLen = @$arrayDimensionInfo;
    $arrayDimList->[$arrayLen-1]++;
    for (my $index = @$arrayDimList-1; $index >= 0; $index--) {
	if ($arrayDimList->[$index] > $arrayDimensionInfo->[$index]->{'arrayMax'}) {
	    my $done = 1;
	    for (my $tmp_index = $index-1; $tmp_index >= 0; $tmp_index--) {
		if ($arrayDimList->[$tmp_index] < $arrayDimensionInfo->[$tmp_index]->{'arrayMax'}) {
		    $done = 0;
		    last;
		}
	    }
	    if (!$done) {
		$arrayDimList->[$index] = $arrayDimensionInfo->[$index]->{'arrayMin'};
		$arrayDimList->[$index-1]++;
	    } else {
		last;
	    }
	}
    }
}

sub _decrementArrayIndex {
    my ($self, $arrayDimList, $arrayDimensionInfo) = @_;

    my $arrayLen = @$arrayDimensionInfo;
    $arrayDimList->[$arrayLen-1]--;
    for (my $index = @$arrayDimList-1; $index >= 0; $index--) {
	if ($arrayDimList->[$index] < $arrayDimensionInfo->[$index]->{'arrayMin'}) {
	    my $done = 1;
	    for (my $tmp_index = $index-1; $tmp_index >= 0; $tmp_index--) {
		if ($arrayDimList->[$tmp_index] > $arrayDimensionInfo->[$tmp_index]->{'arrayMin'}) {
		    $done = 0;
		    last;
		}
	    }
	    if (!$done) {
		$arrayDimList->[$index] = $arrayDimensionInfo->[$index]->{'arrayMax'};
		$arrayDimList->[$index-1]--;
	    } else {
		last;
	    }
	}
    }
}


package csrPerlAPIModule;

use strict;
use vars '$AUTOLOAD';

@csrPerlAPIModule::ISA = qw(csrPerlAPIBase);

{
    my %moduleAttrs = (
                       _hasSmfrAttributes => '',
                       _hasSmfrResources => '',
                       _definitionName => '',
                       _referenceName => '',
                       _referenceIdentifier => '',
                       _scopedName => '',
                       _rtlName => '',
                       _interfaceName => '',
                       _classNameType => '',
                       _address => '',
                       _byteAddress => '',
                       _addressUnitsInBytes => '',
                       _slaveAddressUnitsInBytes => '',
                       _offset => '',
                       _byteOffset => '',
                       _addressWidth => '',
                       _wordByteSize => '',
                       _alignByteSize => '',
                       _offsetByteSize => '',
                       _actualByteSize => '',
                       _addressmapRelativeByteSize => '',
                       _topAddressmapRelativeByteSize => '',
                       _arrayElementByteSize => '',
                       _arrayByteStride => '',
                       _instanceAlignByteSize => '',
                       _totalByteSize => '',
                       _reservedByteSize => '',
                       _bitNumbering => '',
                       _byteEndian => '',
                       _busProtocol => 'basic',
                       _inputPipelineStage => '',
                       _outputPipelineStage => '',
                       _alignmentMode => '',
                       _addressmapRelativeByteAddress => '',
                       _addressmapRelativeByteAddressWidth => '',
                       _sharedSet => '',
                       _sharedOf => '',
                       _timescale => '',
                       _resetTiming => '',
                       _parityProtect => '',
                       _externalBusPorts => '',
                       _ports => '',
                       _inputPortNamePrefix => '',
                       _parameters => '',
                       _outputPortNamePrefix => '',
                       _inputPortNameSuffix => '',
                       _outputPortNameSuffix => '',
                       _atomicAccess => '',
                       _synchronizerStyle => '',
                       _checkAddress => '',
                       _checkReadAccess => '',
                       _checkWriteAccess => '',
                       _checkWriteEnable => '',
                       _checkPrivilege => '',
                       _checkSecure => '',
                       _readDataZero => '',
                       _undefinedValue => '',
                       _hasArrayDecoder => '',
                       _nodeListRef => '',
                       _nextRegisterIndex => '-1',
                       _nextExternalBusPortIndex => '-1',
                       _parentHashRef => '',
                       _prefixNameList => '',
                       _totalNumPorts  => 0,
                       _publicAndNotEmpty => 1,
                       _visitedAddrmapsList => '',
                       _uvmPackageName => '',
                       _securePortActiveLow => '',
                       _sharedShadowBitWidth => '',
		       );

    sub _defaultAttrKeys {
	my $self = shift;
	return ($self->SUPER::_defaultAttrKeys(), map { s/^_//; $_ } keys %moduleAttrs);
    }
    sub new {
	my($caller,
	   $addrmapHashRef,
	   $arrayIndex,
	   $parentAddrmapHashRef,
	   $addrmapAddress,
	   $addrmapRelByteAddr,
	   $busProtocol,
	   $inputPipelineStage,
	   $outputPipelineStage,
	   $prefixNameListRef,
	   $hierPrefixNameListRef) = @_;

        my ($resetTiming, $external);
	my ($count, $addrmapPrefixName, $portListRef, $tmpPortListRef);

	# pass the caller's class name to the base class's constructor.
	my $self = $_[0]->SUPER::new($addrmapHashRef);
	$self->set_designPkgName($designPkgName);
	$self->set_nodeHashRef($addrmapHashRef);
	$self->set_addressmapName($addrmapHashRef->{'name'});
	$self->set_totalNumPorts(0);
	if($addrmapHashRef->{'nodeType'} ne 'addressmap') {
	    $self->set_parentHashRef($parentAddrmapHashRef);
	}
	my @newVisitedAddrmapsList = @visitedAddrmapsList;
	$self->set_visitedAddrmapsList(\@newVisitedAddrmapsList);
	my $expandedAddrmapName = '';
	if(@{$prefixNameListRef} > 0) {
	    $expandedAddrmapName = join('_', @{$prefixNameListRef});
            if ($viewName ne 'sv') {
                $expandedAddrmapName =~ s/\[/_/g;
                $expandedAddrmapName =~ s/\]//g;
            }
            $self->set_name(join('_', $expandedAddrmapName));
	}
	my $tmpHierName = join('_', @$hierPrefixNameListRef);
	$tmpHierName =~ s/\./_/g;
        if ($viewName ne 'sv') {
            $tmpHierName =~ s/\[/_/g;
            $tmpHierName =~ s/\]//g;
        }
	$self->set_hierarchicalName($tmpHierName);
	$self->set_hierarchicalNameDotted(join('.', @$hierPrefixNameListRef));

	if(defined($arrayIndex)) {
	    # copy the array indices before calling updateGroupAccounting;
	    $self->set_arrayIndex([@{$arrayIndex}]);
	}
	$self->assignAddressInfo($addrmapAddress,
				 $addrmapHashRef->{'byteOffset'},
				 $addrmapHashRef->{'offset'},
				 $addrmapRelByteAddr);
        if (!defined($self->getBusProtocol())) {
            $self->set_busProtocol($busProtocol);
        }
        if (defined($resetTiming = $self->getResetTiming())) {
            if ($resetTiming eq 'bus_protocol') {
                $self->set_resetTiming('synchronous');
            }
            $addressmapResetTiming = $self->getResetTiming();
            if (($addrmapHashRef->{'nodeType'} ne 'group') &&
                ($busProtocol eq 'Wishbone_3b') && ($addressmapResetTiming ne 'synchronous')) {
                $self->_printError('Bus protocol is: ' . $busProtocol . ', reset timing is: ' . $addressmapResetTiming .
                                   ', expected reset timing: synchronous; address map: ' . $addrmapHashRef->{'name'});
            }
        }
	$self->set_inputPipelineStage($inputPipelineStage);
	$self->set_outputPipelineStage($outputPipelineStage);
	if(exists($addrmapHashRef->{'externalBusPorts'})) {
	    $portListRef = csrPorts->new($self,
					 $addrmapHashRef,
					 $addrmapHashRef->{'externalBusPorts'},
					 $parentAddrmapHashRef,
					 $expandedAddrmapName);
	    $self->set_externalBusPorts($portListRef);
	    $self->set_nextExternalBusPortIndex(-1);
	}
	if(exists($addrmapHashRef->{'ports'})) {
	    $portListRef = csrPorts->new($self,
					 $addrmapHashRef,
					 $addrmapHashRef->{'ports'},
					 $parentAddrmapHashRef,
					 '');
	    $self->set_ports($portListRef);
	}
	if (defined($addrmapHashRef->{'parameters'})) {
	    my $parameterList = csrParameters->new($addrmapHashRef->{'parameters'});
	    $self->set_parameters($parameterList);
	}
	my $currAddrmapInfo = @visitedAddrmapsList[@visitedAddrmapsList-1];
	return $self;
    }
    sub hasRtlName {
	my $self = shift;
	return 1 if (defined($self->get_rtlName()));
	return 0;
    }
    sub isPublicAndNotEmpty {
        my $self = shift;
	my $isPANE;
	if (defined($isPANE = $self->get_isPublicAndNotEmpty())) {
	    return $isPANE;
	}
	return 0;
    }        
    sub getRtlName {
	my $self = shift;
	return $self->get_rtlName();
    }
    sub hasInterface {
        my $self = shift;
        return 1 if (defined($self->get_interfaceName()));
    }
    sub getInterfaceName {
        my $self = shift;
        return $self->get_interfaceName();
    }
    sub getDefinitionName {
	my $self = shift;
	my $defName;
	return $defName if (defined($defName = $self->getEffectiveTypeName()));
	return;
    }
    sub getReferenceName {
	my $self = shift;
	my $refName;
	return $refName	if (defined($refName = $self->get_referenceName()));
	return;
    }
    sub getReferenceIdentifier {
	my $self = shift;
	my $refId;
	return $refId if (defined($refId = $self->get_referenceIdentifier()));
	return;
    }
    sub getScopedName {
	my $self = shift;
	my $scopedName;
	return $scopedName if (defined($scopedName = $self->get_scopedName()));
	return;
    }
    sub getClassNameType {
	my $self = shift;
	return ($self->get_classNameType());
    }
    sub getByteAddress {
        my $self = shift;
        return Math::BigInt->new($self->get_byteAddress());
    }
    sub getAddressWidth {
        my $self = shift;
        return $self->get_addressWidth();
    }
    sub getAddressUnitsInBytes {
	my $self = shift;
	return $self->get_addressUnitsInBytes();
    }
    sub getSlaveAddressUnitsInBytes {
	my $self = shift;
	return $self->get_slaveAddressUnitsInBytes();
    }
    sub getAddressmapRelativeByteAddress {
        my $self = shift;
        return Math::BigInt->new($self->get_addressmapRelativeByteAddress());
    }
    sub getAddressmapRelativeByteAddressWidth {
        my $self = shift;
        return $self->get_addressmapRelativeByteAddressWidth();
    }
    sub getParentAddressmapHashRef {
	my $self = shift;
	return $self->get_parentHashRef();
    }
    sub getByteEndian {
        my $self = shift;
        return $self->get_byteEndian();
    }
    sub getByteOffset {
        my $self = shift;
        return Math::BigInt->new($self->get_byteOffset());
    }
    sub getResetTiming {
        my $self = shift;
        return $self->get_resetTiming();
    }
    sub getTimescale {
        my $self = shift;
        return $self->get_timescale();
    }
    sub getSharedShadowBitWidth {
        my $self = shift;
        return $self->get_sharedShadowBitWidth();
    }
    sub getBusProtocol {
        my $self = shift;
        return $self->get_busProtocol();
    }
    sub getInputPipelineStage {
        my $self = shift;
        return $self->get_inputPipelineStage();
    }
    sub getOutputPipelineStage {
        my $self = shift;
        return $self->get_outputPipelineStage();
    }
    sub getIOPipelineStage {
        my $self = shift;
        my $inputPipe = $self->getInputPipelineStage();
        my $outputPipe = $self->getOutputPipelineStage();
        return ($inputPipe, $outputPipe);
    }
    sub getOffsetByteSize {
        my $self = shift;
        return $self->get_offsetByteSize();
    }
    sub getWordByteSize {
        my $self = shift;
        return $self->get_wordByteSize();
    }
    sub getWordSizeBits {
        my $self = shift;
        return ($self->get_wordByteSize() * 8);
    }
    sub getActualByteSize {
	my $self = shift;
        return $self->get_actualByteSize();
    }
    sub getAddressmapRelativeByteSize {
	my $self = shift;
	return $self->get_addressmapRelativeByteSize();
    }
    sub getTopAddressmapRelativeByteSize {
	my $self = shift;
	return $self->get_topAddressmapRelativeByteSize();
    }
    sub getArrayByteStride {
        my $self = shift;
        return Math::BigInt->new($self->get_arrayByteStride());
    }
    sub getInstanceAlignByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_instanceAlignByteSize());
    }
    sub getReservedByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_reservedByteSize());
    }
    sub getAlignByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_alignByteSize());
    }
    sub getArrayElementByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_arrayElementByteSize());
    }
    sub getAlignmentMode {
	my $self = shift;
	return $self->get_alignmentMode();
    }
    sub getTotalByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_totalByteSize());
    }
    sub getPorts {
        my $self = shift;
        return $self->get_ports();
    }
    sub getExternalBusPorts {
        my $self = shift;
        return $self->get_externalBusPorts();
    }
    sub checkAddressError {
        my $self = shift;
	my $checkAddress;
	return 1 if (defined($checkAddress = $self->get_checkAddress()) &&
		     ($checkAddress eq 'true'));
	return 0;
    }
    sub getCheckAddress {
        my $self = shift;
        return $self->get_checkAddress();
    }
    sub getCheckReadAccess {
        my $self = shift;
        return $self->get_checkReadAccess();
    }
    sub checkReadAccessError {
        my $self = shift;
	my $checkRdAccess;
	return 1 if (defined($checkRdAccess = $self->get_checkReadAccess()) &&
		     ($checkRdAccess eq 'true'));
	return 0;
    }
    sub getCheckWriteAccess {
        my $self = shift;
        return $self->get_checkWriteAccess();
    }
    sub checkWriteAccessError {
        my $self = shift;
        my $checkWrtAccess;
        return 1 if (defined($checkWrtAccess = $self->get_checkWriteAccess()) &&
                    ($checkWrtAccess eq 'true'));
        return 0;
    }
    sub getCheckPrivilege {
        my $self = shift;
        return $self->get_checkPrivilege();
    }
    sub checkPrivilegeError {
        my $self = shift;
        my $checkPriv;
        return 1 if (defined($checkPriv = $self->get_checkPrivilege()) &&
                    ($checkPriv eq 'true'));
        return 0;
    }
    sub getCheckSecure {
        my $self = shift;
        return $self->get_checkPrivilege();
    }
    sub checkSecureError {
        my $self = shift;
        my $checkSec;
        return 1 if (defined($checkSec = $self->get_checkPrivilege()) &&
                    ($checkSec eq 'true'));
        return 0;
    }
    sub getReadDataZero {
	my $self = shift;
	return $self->get_readDataZero();
    }
    sub hasUndefinedValue {
        my $self = shift;
        return 0 if ($self->get_undefinedValue() eq '');
        return 1;
    }
    sub getUndefinedValue {
	my $self = shift;
	return $self->get_undefinedValue();
    }
    sub hasInputSynchronizer {
	my $self = shift;
	return 0;
    }
    sub hasSmfrAttributes {
        my $self = shift;
        return 1 if ($self->get_hasSmfrAttributes() eq 'true');
        return 0;
    }
    sub hasSmfrResources {
        my $self = shift;
        return 0 if (!defined($self->get_hasSmfrResources()));
        return 1 if ($self->get_hasSmfrResources() eq 'true');
        return 0;
    }
    sub hasPortType {
        my ($self, $portType) = @_;

        my ($portListRef);

	if ($portType =~ /external/) {
	    $portListRef = $self->getExternalBusPorts();
	} else {
	    $portListRef = $self->getPorts();
	}
        return 0 if(!defined($portListRef));
        foreach my $portObj (@{$portListRef}) {
            if($portObj->getPortType() eq $portType) {
                return 1;
            }
        }
        return 0;
    }
    sub hasParameters {
	my $self = shift;
	return 1 if (defined($self->get_parameters()));
	return 0;
    }
    sub getParameters {
	my $self = shift;
	return $self->get_parameters();
    }
    sub hasSharedSet {
	my $self = shift;
	return 1 if (defined($self->get_sharedSet()));
	return 0;
    }
    sub getSharedSet {
	my $self = shift;
	return $self->get_sharedSet();
    }
    sub hasSharedOf {
	my $self = shift;
	return 1 if (defined($self->get_sharedOf()));
	return 0;
    }
    sub getPortType {
        my ($self, $portType) = @_;

        my ($portListRef);
	if (!defined($portType)) {
	    $self->_printError('Port type not defined');
	    exit(1);
	}
	if ($portType =~ /external/) {
	    $portListRef = $self->getExternalBusPorts();
	} else {
	    $portListRef = $self->getPorts();
	}
        return if(!defined($portListRef));
        foreach my $portObj (@{$portListRef}) {
            if($portObj->getPortType() eq $portType) {
                return $portObj;
            }
        }
        return;
    }
    sub getUvmPackageName {
        my $self = shift;
        return $self->get_uvmPackageName();
    }
    sub isSecurePortActiveLow {
        my $self = shift;
        if (defined($self->get_securePortActiveLow()) &&
            ($self->get_securePortActiveLow() eq 'true')) {
            return 1;
        }
        return 0;
    }
    sub isReadAtomic {
	my $self = shift;
	return 0;
    }
    sub isWriteAtomic {
	my $self = shift;
	return 0;
    }
    sub getResetClassList {
	my $self = shift;
	my @resetClassList;
	$self->saveAddrmapInfoContext();
	while (my $nextReg = $self->getNextRegisterNode()) {
	    my $fieldNodes = $nextReg->getBitfields();
	    foreach my $nextField (@{$fieldNodes}) {
		if ($nextField->hasResetValue()) {
		    if ($nextField->hasResetClass()) {
			my $fieldResetClass = $nextField->getResetClass();
			foreach my $nextFieldResetClassName (@{$fieldResetClass}) {
			    $nextFieldResetClassName =~ s/\s//g;
			    my $found = 0;
			    foreach my $nextSavedClassName (@resetClassList) {
				$nextSavedClassName =~ s/\s//g;
				if ($nextSavedClassName eq $nextFieldResetClassName) {
				    $found = 1;
				    last;
				}
			    }
			    push (@resetClassList, $nextFieldResetClassName) if (!$found);
			}
		    }
		}
	    }
	}
	$self->restoreAddrmapInfoContext();
	return \@resetClassList;
    }
    sub getParentNode {
        my $self = shift;

        # get the parent address map object for the calling object.

        my $count = 0;
	my ($verilogFormat, $relAddr, $arrayIndex);
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage);
        my (@prefixList, $parentHashRef, $parentOfParentHashRef);
        my ($topLevelHashRef, $addrmapObj, $parentName, $bogusAddr);

        my $bigIntAddr = Math::BigInt->new('0');
        my $hierId = $self->getHierarchicalIdentifier();
        my @levelNameList = split('\.', $self->get_hierarchicalNameDotted());
	my @hierLevelNameList = split('\.', $self->get_hierarchicalNameDotted());

        return if(@levelNameList <= 1);

        ($topLevelHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);

        pop(@levelNameList);
        pop(@hierLevelNameList);

        # start from the last level name.
        while(@levelNameList) {
            $parentName = join('.', @levelNameList);
            ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
            if(($parentHashRef->{'nodeType'} eq 'addressmap') ||
               ($parentHashRef->{'nodeType'} eq 'addressmapInstance')) {
                ($busProtocol, $inputPipelineStage, $outputPipelineStage) =
                    $designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);
                @prefixList = @levelNameList;
                shift @prefixList;
                pop(@levelNameList);
                # parent of parent hash ref
                if(@levelNameList > 0) {
                    # get to the parent of parent node
                    if(@levelNameList == 1) {
                        $parentName = $levelNameList[0];
                    } else {
                        $parentName = join('.', @levelNameList);
                    }
                    ($parentOfParentHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($parentName);
                }
		($verilogFormat,
		 $relAddr) = $self->getVerilogBaseAndValue($parentHashRef->{'addressmapRelativeByteAddress'});
		my $relAddrBigInt = Math::BigInt->new($relAddr);
                $arrayIndex = $self->getParentArrayIndexFromHierId($hierId);
                if (($viewName eq 'sv') && (@$arrayIndex)) {
                    my $tmpIndex;
                    for (my $count=0; $count < @$arrayIndex; $count++) {
                        $tmpIndex .= '[' . $arrayIndex->[$count] . ']';
                    }
                    $hierLevelNameList[@hierLevelNameList-1] .= $tmpIndex;
                    $prefixList[@prefixList-1] .= $tmpIndex;
                }
                $addrmapObj = csrPerlAPIModule->new($parentHashRef,
						    $arrayIndex,
                                                    $parentOfParentHashRef,
                                                    $bigIntAddr,
						    $relAddrBigInt,
                                                    $busProtocol,
                                                    $inputPipelineStage,
                                                    $outputPipelineStage,
                                                    \@prefixList,
						    \@hierLevelNameList);
                return $addrmapObj;
            }
            if (defined($parentHashRef->{'arrayDimensionInfo'})) {
                my $numDims = scalar(@{$parentHashRef->{'arrayDimensionInfo'}});
                for (my $count=0; $count < $numDims; $count++) {
                    pop(@$arrayIndex);
                }
            }
            pop(@levelNameList);
	    pop(@hierLevelNameList);
        }
        return;
    }

    sub getBusSlavePorts {
        my $self = shift;
        my (@portList, $portListRef);

        if ($self->isExternal()) {
            $portListRef = $self->getExternalBusPorts();
        } else {
            $portListRef = $self->getPorts();
        }
        my $currPortIndex=0;
        while (my $portObj = $self->getNextPortNode($portListRef,
                                                    \$currPortIndex)) {
            push(@portList, $portObj);
        }
        return \@portList;
    }

    sub getInterfacePorts {
	my($self, $portDir) = @_;
	my $tmpPortListRef = [];
	my ($nextObj, @portList);
	my $noExternalBus = 0;
	while($nextObj = $self->getNextDesignNode()) {
	    $tmpPortListRef = $nextObj->getObjectPortsList($portDir,
							   $noExternalBus);
	    if(defined($tmpPortListRef) && (@$tmpPortListRef > 0)) {
		push(@portList, @$tmpPortListRef);
	    }
	}
	# remove duplicates
	my @newPortList;
	$self->removeDuplicatePorts(\@portList, \@newPortList);
	return \@newPortList;
    }
    sub getDeclaredExternalObjectPortList {
	my($self, $portDir) = @_;

	return if (!$self->isDeclaredExternal());
	return $self->getExternalPortList($portDir);
    }
    sub getObjectPortsList_NoExternalPorts {
	my($self, $portDir) = @_;
	my $noExternalPorts = 1;
	return ($self->getPortList($portDir, $noExternalPorts));
    }
    sub getObjectPortsList {
	my($self, $portDir, $noExternalBus) = @_;
	$noExternalBus = 0 if (!defined($noExternalBus));
	return ($self->getPortList($portDir, $noExternalBus));
    }
    sub getPortList {
        my($self,
           $direction,
           $noExternalBus) = @_;

        my ($portObj, @portList, $portType, $portListRef);
        my $parentNode = $self->getParentNode();
        return if (defined($parentNode) && $parentNode->isExternal() &&
                   !$self->getHierarchicalTBFlag());
        if ($self->isExternal() && !$noExternalBus) {
            $portListRef = $self->getExternalBusPorts();
        } else {
            $portListRef = $self->getPorts();
        }
        my $currPortIndex = 0;
        while ($portObj = $self->getNextPortNode($portListRef,
                                                 \$currPortIndex)) {
           push(@portList, $portObj) if($direction eq $portObj->getPortDirection());
        }
        return \@portList;
    }
    sub getAddressmapIOPorts {
	my $self = shift;
	my $addrmapIOPorts;

	my $inputPorts = $self->getPortList('input');
	if(defined($inputPorts)) {
	    push(@$addrmapIOPorts, @$inputPorts);
	}
	my $outputPorts = $self->getPortList('output');
	if(defined($outputPorts)) {
	    push(@$addrmapIOPorts, @$outputPorts);
	}
	return $addrmapIOPorts;
    }
    sub getExternalIOPorts {
	my $self = shift;
	my $addrmapIOPorts;

	my $inputPorts = $self->getExternalPortList('input');
	if(defined($inputPorts)) {
	    push(@$addrmapIOPorts, @$inputPorts);
	}
	my $outputPorts = $self->getExternalPortList('output');
	if(defined($outputPorts)) {
	    push(@$addrmapIOPorts, @$outputPorts);
	}
	return $addrmapIOPorts;
    }
    sub hasWideRegister {
        my $self = shift;
        $self->saveAddrmapInfoContext();
        my $addrmapNode = $self->get_addressmapNode();
        while(my $nextObj = $addrmapNode->getNextRegisterNode()) {
            if($nextObj->getNodeType() eq 'wideRegister') {
                $self->restoreAddrmapInfoContext();
                return 1;
            }
        }
        $self->restoreAddrmapInfoContext();
        return 0;
    }
    sub hasWideMemory {
        my $self = shift;
        $self->saveAddrmapInfoContext();
        while(my $nextObj = $self->getNextDesignNode()) {
            if($nextObj->getNodeType() eq 'wideMemory') {
                $self->restoreAddrmapInfoContext();
                return 1;
            }
        }
        $self->restoreAddrmapInfoContext();
        return 0;
    }
    sub hasWideAtomicRegister {
        my $self = shift;
        $self->saveAddrmapInfoContext();
        while(my $nextObj = $self->getNextRegisterNode()) {
            if($nextObj->isAtomic()) {
                $self->restoreAddrmapInfoContext();
                return $nextObj;
            }
        }
        $self->restoreAddrmapInfoContext();
        return;
    }
}

sub findRegisterInDB {
    my($self, $regName, $addrmapHashRef) = @_;

    my ($instModule, $nextRegHashRef, $regListRef, $count);
    my ($nextInstHashRef, $instListRef, $instAddrmap);
    if(defined($addrmapHashRef->{'definition'})) {
	for($count = 0; $count < @{$addrmapHashRef->{'definition'}}; $count++) {
	    if(defined($addrmapHashRef->{'definition'}->[$count]->{'registers'})) {
		$regListRef = $addrmapHashRef->{'definition'}->[$count]->{'registers'};
		foreach $nextRegHashRef (@{$regListRef}) {
		    if($regName eq $nextRegHashRef->{'name'}) {
			return $nextRegHashRef;
		    }
		}
	    } elsif(defined($addrmapHashRef->{'definition'}->[$count]->{'instances'})) {
		$instListRef = $addrmapHashRef->{'definition'}->[$count]->{'instances'};
		foreach $nextInstHashRef (@{$instListRef}) {
		    $instAddrmap = $designDBRef->_findAddressmapForInstanceHash($nextInstHashRef);
		    $self->findRegisterInDB($regName, $instAddrmap);
		}
	    } elsif(defined($addrmapHashRef->{'definition'}->[$count]->{'groups'})) {
		$instListRef = $addrmapHashRef->{'definition'}->[$count]->{'groups'};
		foreach $nextInstHashRef (@{$instListRef}) {
		    $instAddrmap = $designDBRef->_findAddressmapForInstanceHash($nextInstHashRef);
		    $self->findRegisterInDB($regName, $instAddrmap);
		}
	    }
	}
    }
}

sub getNextDesignNode {
    my $self = shift;

    my @prefixList = ();
    my (@hierarchicalPrefixList);
    my ($subRegHashListRef, $relAddr, $parentAddrmapHashRef, $nextObj);
    my ($prefixListRef, $offset, $regHashRef, $currAddrmapInfo, $verilogFormat);
    my ($addrmapObj, $bigIntAddr, $addrmapHashRef);

    $updateRefArrayIndex = 1;
    my $arrayIndex = undef;

    if(@visitedAddrmapsList == 0) {
	# We're just starting to process the nodes.
	$addrmapHashRef = $self->get_nodeHashRef();
	($verilogFormat, $offset) = $self->getVerilogBaseAndValue($addrmapHashRef->{'byteAddress'});
	$bigIntAddr = Math::BigInt->new($offset);
	($verilogFormat,
	 $relAddr) = $self->getVerilogBaseAndValue($addrmapHashRef->{'addressmapRelativeByteAddress'});
	my $relAddrBigInt = Math::BigInt->new($relAddr);
	push(@hierarchicalPrefixList, $addrmapHashRef->{'hierarchicalIdentifier'});
	$addrmapObj = csrPerlAPIModule->new($addrmapHashRef,
					    $arrayIndex,
					    $parentAddrmapHashRef,
					    $bigIntAddr,
					    $relAddrBigInt,
					    $addrmapHashRef->{'busProtocol'},
					    $addrmapHashRef->{'inputPipelineStage'},
					    $addrmapHashRef->{'outputPipelineStage'},
					    \@prefixList,
					    \@hierarchicalPrefixList);
	$currAddrmapInfo->{'currAddrmapHashRef'} = $addrmapHashRef;
	$currAddrmapInfo->{'currNode'} = $addrmapHashRef;
	$currAddrmapInfo->{'parentAddr'} = $bigIntAddr;
	$currAddrmapInfo->{'relativeAddress'} = $bigIntAddr;
	$self->setInitialAddrmapIndexes($currAddrmapInfo, $addrmapHashRef);
	push(@prefixList, $addrmapHashRef->{'name'}) if (!$self->getSemiforeEnvGenFlag());
	$currAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	$currAddrmapInfo->{'prefixListRef'} = \@prefixList;
	$currAddrmapInfo->{'currAddrmapObj'} = $addrmapObj;
	my $refNameHashListRef = [];
	$designDBRef->_genRefNameList($addrmapHashRef, $refNameHashListRef);
	$currAddrmapInfo->{'refNameHashListRef'} = $refNameHashListRef;
	push(@visitedAddrmapsList, $currAddrmapInfo);
	return $addrmapObj;
    }
    $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
    $addrmapHashRef = $currAddrmapInfo->{'currAddrmapHashRef'};
    $prefixListRef = $currAddrmapInfo->{'prefixListRef'};

    my $currNode = $currAddrmapInfo->{'currNode'};
    my $currNodeType = $currNode->{'nodeType'};

    # Memory objects may have virtual registers.
    if (($currNodeType eq 'addressmap') || ($currNodeType eq 'addressmapInstance') ||
        ($currNodeType eq 'group') || ($currNodeType eq 'memory') || ($currNodeType eq 'union')) {
        if ($self->moreRegistersAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
            $nextObj = $self->getNextRegister($currAddrmapInfo, $addrmapHashRef);
            # We may've just processed a wide register (and pushed it in the
            # LIFO. Wide register accounting is done somewhere else.
            $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
            if(($currAddrmapInfo->{'currNode'})->{'nodeType'} ne 'wideRegister') {
                $self->updateRegisterAccounting($currAddrmapInfo);
            }
            return $nextObj;
	} elsif ($self->moreUnionsAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
	    $nextObj = $self->getNextUnion($currAddrmapInfo, $addrmapHashRef);
	    return $nextObj;
        } elsif (($currNodeType ne 'memory') && $self->moreMemoriesAtThisLevel($currAddrmapInfo,
									       $addrmapHashRef)) {
            $nextObj = $self->getNextMemory($currAddrmapInfo, $addrmapHashRef);
            if ($includeWideMemoriesFlag && ($viewName eq 'sv')) {
                $self->doneWithWideMemory();
            }
            return $nextObj;
        } elsif (($currNodeType ne 'memory') && $self->moreInstancesAtThisLevel($currAddrmapInfo,
                                                                                $addrmapHashRef)) {
            $nextObj = $self->getNextInstance($currAddrmapInfo, $addrmapHashRef);
            return $nextObj;
	} elsif ($self->moreGroupsAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
	    $nextObj = $self->getNextGroup($currAddrmapInfo, $addrmapHashRef);
	    return $nextObj;
        } else {
	    pop(@visitedAddrmapsList);
            if(@visitedAddrmapsList == 0) {
                # We're done.
                return;
            } else {
                $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
                $addrmapHashRef = $currAddrmapInfo->{'currAddrmapHashRef'};
		if ($currNodeType eq 'addressmapInstance') {
		    $self->updateInstanceAccounting($currAddrmapInfo);
		} elsif ($currNodeType eq 'memory') {
		    $self->updateMemoryAccounting($currAddrmapInfo);
		} elsif ($currNodeType eq 'group') {
		    $self->updateGroupAccounting($currAddrmapInfo);
		} elsif ($currNodeType eq 'union') {
		    $self->updateUnionAccounting($currAddrmapInfo);
		} else {
		    $self->_printError('Node type: ' . $currNodeType . ' not handled');
		    exit(1);
		}
                $nextObj = $self->getNextDesignNode();
                return $nextObj;
            }
        }
    } elsif ($currNodeType eq 'wideRegister') {
	my $regHashRef = $currAddrmapInfo->{'currRegisterHashRef'};
	my $subRegHashListRef = $regHashRef->{'subRegisters'};
	if (($currAddrmapInfo->{'currAddrmapSubRegisterIndex'} >= @{$subRegHashListRef}) &&
	   (defined($regHashRef->{'arrayDimensionInfo'}))){
	    # we have exhausted the sub register list for the current array element.
	    if (!$self->getExpandArraysFlag()) {
		$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
		$currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
		return $nextObj;
	    } else {
		my $arrayMax = $designDBRef->_getArrayMaxIndices($regHashRef->{'arrayDimensionInfo'});
		if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
						     $arrayMax) <= 0) {
		    # There's at least another array element. We need to generate
		    # an object for the new wide register array element before
		    # continuing with sub-registers.
		    $nextObj = $self->getNextWideRegArrayElement($currAddrmapInfo, $addrmapHashRef);
		    return $nextObj;
		}
	    }
	} else {
	    $nextObj = $self->getNextSubRegister($currAddrmapInfo, $addrmapHashRef);
	    $self->updateWideRegisterAccounting($currAddrmapInfo, $addrmapHashRef);
	    return $nextObj;
	}
    } elsif ($currNodeType eq 'wideMemory') {
	my $memHashRef = $currAddrmapInfo->{'currMemoryHashRef'};
	my $subMemHashListRef = $memHashRef->{'subMemories'};
        if ($self->moreRegistersAtThisLevel($currAddrmapInfo, $memHashRef)) {
            $nextObj = $self->getNextRegister($currAddrmapInfo, $memHashRef);
            # We may've just processed a wide register (and pushed it in the
            # LIFO. Wide register accounting is done somewhere else.
            $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
            if(($currAddrmapInfo->{'currNode'})->{'nodeType'} ne 'wideRegister') {
                $self->updateRegisterAccounting($currAddrmapInfo);
            }
            return $nextObj;
        } elsif ($self->moreGroupsAtThisLevel($currAddrmapInfo, $memHashRef)) {
            $nextObj = $self->getNextGroup($currAddrmapInfo, $memHashRef);
            $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
            return $nextObj;
        } elsif (($currAddrmapInfo->{'currAddrmapSubMemoryIndex'} >= @{$subMemHashListRef}) &&
	   (defined($memHashRef->{'arrayDimensionInfo'}))){
	    # we have exhausted the sub memory list for the current array element.
	    if (!$self->getExpandArraysFlag()) {
		$currAddrmapInfo->{'currAddrmapMemoryArrayIndex'} = -1;
		$currAddrmapInfo->{'currAddrmapMemoryIndex'}++;
		return $nextObj;
	    } else {
		my $arrayMax = $designDBRef->_getArrayMaxIndices($memHashRef->{'arrayDimensionInfo'});
		if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapMemoryArrayIndex'},
						     $arrayMax) <= 0) {
                    # There's at least another array element. We need to generate
                    # an object for the new wide memory array element before
                    # continuing with sub-memories.
                    $nextObj = $self->getNextWideMemArrayElement($currAddrmapInfo, $addrmapHashRef);
                    return $nextObj;
                }
            }
        } else {
            $nextObj = $self->getNextSubMemory($currAddrmapInfo, $addrmapHashRef);
            $self->updateWideMemoryAccounting($currAddrmapInfo, $addrmapHashRef);
            return $nextObj;
        }
    }
    return;
}

sub getNextDesignNode_NoDepth {
    my $self = shift;

    my @prefixList = ();
    my (@hierarchicalPrefixList);
    my ($subRegHashListRef, $relAddr, $parentAddrmapHashRef, $nextObj);
    my ($prefixListRef, $offset, $regHashRef, $currAddrmapInfo, $verilogFormat);
    my ($addrmapObj, $bigIntAddr, $addrmapHashRef);

    $updateRefArrayIndex = 1;
    my $arrayIndex = undef;

    if(@visitedAddrmapsList == 0) {
	# We're just starting to process the nodes.
	$addrmapHashRef = $self->get_nodeHashRef();
	($verilogFormat, $offset) = $self->getVerilogBaseAndValue($addrmapHashRef->{'byteAddress'});
	$bigIntAddr = Math::BigInt->new($offset);
	($verilogFormat, $relAddr) = $self->getVerilogBaseAndValue($addrmapHashRef->{'addressmapRelativeByteAddress'});
	my $relAddrBigInt = Math::BigInt->new($relAddr);
	push(@hierarchicalPrefixList, split('\.', $addrmapHashRef->{'hierarchicalIdentifier'}));
	$addrmapObj = csrPerlAPIModule->new($addrmapHashRef,
					    $arrayIndex,
					    $parentAddrmapHashRef,
					    $bigIntAddr,
					    $relAddrBigInt,
					    $addrmapHashRef->{'busProtocol'},
					    $addrmapHashRef->{'inputPipelineStage'},
					    $addrmapHashRef->{'outputPipelineStage'},
					    \@prefixList,
					    \@hierarchicalPrefixList);
	$currAddrmapInfo->{'currAddrmapHashRef'} = $addrmapHashRef;
	$currAddrmapInfo->{'currNode'} = $addrmapHashRef;
	$currAddrmapInfo->{'parentAddr'} = $bigIntAddr;
	$currAddrmapInfo->{'relativeAddress'} = $bigIntAddr;
	$self->setInitialAddrmapIndexes($currAddrmapInfo, $addrmapHashRef);
	push(@prefixList, $addrmapHashRef->{'name'}) if (!$self->getSemiforeEnvGenFlag());
	$currAddrmapInfo->{'prefixListRef'} = \@prefixList;
	$currAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	$currAddrmapInfo->{'currAddrmapObj'} = $addrmapObj;
	my $refNameHashListRef = [];
	$designDBRef->_genRefNameList($addrmapHashRef, $refNameHashListRef);
	$currAddrmapInfo->{'refNameHashListRef'} = $refNameHashListRef;
	push(@visitedAddrmapsList, $currAddrmapInfo);
	$tmpIncludeWideRegistersFlag = $self->getIncludeWideRegistersFlag();
	$tmpIncludeWideMemoriesFlag = $self->getIncludeWideMemoriesFlag();
	$self->setIncludeWideRegistersFlag();
	$self->setIncludeWideMemoriesFlag();
	return $addrmapObj;
    }
    $currAddrmapInfo = $visitedAddrmapsList[0];
    $addrmapHashRef = $currAddrmapInfo->{'currAddrmapHashRef'};
    $prefixListRef = $currAddrmapInfo->{'prefixListRef'};
    my $currNode = $currAddrmapInfo->{'currNode'};
    my $currNodeType = $currNode->{'nodeType'};

    # Memory objects may have virtual registers.
    if (($currNodeType eq 'addressmap') || ($currNodeType eq 'addressmapInstance') ||
       ($currNodeType eq 'group') || ($currNodeType eq 'memory')) {
	if ($self->moreRegistersAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
            $nextObj = $self->getNextRegister($currAddrmapInfo, $addrmapHashRef);
	    $self->updateRegisterAccounting($currAddrmapInfo);
            return $nextObj;
	} elsif (($currNodeType ne 'memory') && $self->moreMemoriesAtThisLevel($currAddrmapInfo,
									       $addrmapHashRef)) {
            $nextObj = $self->getNextMemory($currAddrmapInfo, $addrmapHashRef);
	    pop(@visitedAddrmapsList);
	    $self->updateMemoryAccounting($currAddrmapInfo);
            return $nextObj;
        } elsif (($currNodeType ne 'memory') && $self->moreInstancesAtThisLevel($currAddrmapInfo,
										$addrmapHashRef)) {
            $nextObj = $self->getNextInstance($currAddrmapInfo, $addrmapHashRef);
	    pop(@visitedAddrmapsList);
	    $self->updateInstanceAccounting($currAddrmapInfo);
            return $nextObj;
	} elsif (($currNodeType ne 'memory') && $self->moreGroupsAtThisLevel($currAddrmapInfo,
									     $addrmapHashRef)) {
	    $nextObj = $self->getNextGroup($currAddrmapInfo, $addrmapHashRef);
	    pop(@visitedAddrmapsList);
	    $self->updateGroupAccounting($currAddrmapInfo);
	    return $nextObj;
        } else {
	    pop(@visitedAddrmapsList);
            if(@visitedAddrmapsList == 0) {
                # We're done.
		$self->setIncludeWideRegistersFlag($tmpIncludeWideRegistersFlag);
		$self->setIncludeWideMemoriesFlag($tmpIncludeWideMemoriesFlag);
                return;
            } else {
                $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
                $addrmapHashRef = $currAddrmapInfo->{'currAddrmapHashRef'};
		if ($currNodeType eq 'memory') {
		    $self->updateMemoryAccounting($currAddrmapInfo);
		} elsif ($currNodeType eq 'group') {
		    $self->updateGroupAccounting($currAddrmapInfo);
		} else {
		    $self->_printError('Node type: ' . $currNodeType . ' not handled');
		    exit(1);
		}
                $nextObj = $self->getNextDesignNode();
                return $nextObj;
            }
        }
    }
    $self->setIncludeWideRegistersFlag($tmpIncludeWideRegistersFlag);
    $self->setIncludeWideMemoriesFlag($tmpIncludeWideMemoriesFlag);
    return;
}

sub findAddressmapInVisitedNodes {
    my ($self, $addrmapHashRef) = @_;

    my ($tmpAddrmapInfo, $tmpAddrmapHashRef);
    for (my $index = @visitedAddrmapsList-1; $index >= 0; $index--) {
	$tmpAddrmapInfo = $visitedAddrmapsList[$index];
	$tmpAddrmapHashRef = $tmpAddrmapInfo->{'currAddrmapHashRef'};
	if (($tmpAddrmapHashRef->{'nodeType'} eq $addrmapHashRef->{'nodeType'}) &&
	    ($addrmapHashRef->{'name'} eq $tmpAddrmapHashRef->{'name'})) {
	    return $tmpAddrmapHashRef;
	}
    }
    return;
}

sub moreInstancesAtThisLevel {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my ($instIndex, $instanceHashListRef, $external);

    if(defined($instanceHashListRef = $designDBRef->_getInstanceHashList($addrmapHashRef))) {
	$instIndex = $currAddrmapInfo->{'currAddrmapInstanceIndex'};
	if($self->getSkipDeclaredExternalAddressmapsFlag()) {
	    while (($instIndex < @{$instanceHashListRef}) && 
		   ($instanceHashListRef->[$instIndex]->{'declaredExternal'} eq 'true')) {
		$instIndex++;
	    }
	} else {
	    if($self->getSkipExternalAddressmapsFlag()) {
		while(($instIndex < @{$instanceHashListRef}) &&
		      defined($external = $instanceHashListRef->[$instIndex]->{'external'}) &&
		      ($external eq 'true')) {
		    $instIndex++;
		}
	    }
	}
	$currAddrmapInfo->{'currAddrmapInstanceIndex'} = $instIndex;
	return 1 if ($currAddrmapInfo->{'currAddrmapInstanceIndex'} < @{$instanceHashListRef});
    }
    return 0;
}

sub getNextInstance {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my ($external, $declaredExternal, $relAddr, @hierarchicalPrefixList);
    my ($instIndex, $bigIntAddr, @prefixList, $instHashRef);
    my ($addrmapObj, $tmpAddrmapInfo, $instanceHashListRef);
    my ($verilogFormat, $offset, $newAddrmapInfo, $newAddrmapHashRef);

    $instanceHashListRef = $designDBRef->_getInstanceHashList($addrmapHashRef);
    $instIndex = $currAddrmapInfo->{'currAddrmapInstanceIndex'};
    my $currInstHashRef = $instanceHashListRef->[$instIndex];
    $currAddrmapInfo->{'currInstanceHashRef'} = $currInstHashRef;
    $instHashRef = $designDBRef->_findAddressmapForInstanceHash($currInstHashRef);

    if (defined($instHashRef->{'arrayDimensionInfo'}) && $self->getExpandArraysFlag()) {
	$addrmapObj = $self->getNextInstArrayElement($currAddrmapInfo,
						     $addrmapHashRef,
						     $instHashRef);
	return $addrmapObj;
    } else {
	($verilogFormat, $offset) = $self->getVerilogBaseAndValue($instHashRef->{'byteOffset'});
	$bigIntAddr = Math::BigInt->new($offset);
	$bigIntAddr->badd($currAddrmapInfo->{'parentAddr'});

	push(@prefixList, (@{$currAddrmapInfo->{'prefixListRef'}}, $instHashRef->{'name'}));
	push(@hierarchicalPrefixList, (@{$currAddrmapInfo->{'hierarchicalPrefixListRef'}},
				       $instHashRef->{'name'}));

	$newAddrmapHashRef = $instHashRef;
	$newAddrmapInfo->{'prefixListRef'} = \@prefixList;
	$newAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	$newAddrmapInfo->{'currNode'} = $instHashRef;
	$newAddrmapInfo->{'currAddrmapHashRef'} = $newAddrmapHashRef;
	$newAddrmapInfo->{'parentAddr'} = $bigIntAddr;
	push(@visitedAddrmapsList, $newAddrmapInfo);

	($verilogFormat, $relAddr) =
	    $self->getVerilogBaseAndValue($newAddrmapHashRef->{'addressmapRelativeByteAddress'});
	$newAddrmapInfo->{'arrayIndex'} = undef;
	my $addrmapRelByteAddr = Math::BigInt->new($relAddr);
	if (defined($currInstHashRef->{'parameters'})) {
	    $newAddrmapHashRef->{'parameters'} = $currInstHashRef->{'parameters'};
	}
	$self->setInitialAddrmapIndexes($newAddrmapInfo, $newAddrmapHashRef);
	my $arrayIndex = undef;
	$addrmapObj = csrPerlAPIModule->new($newAddrmapHashRef,
					    $arrayIndex,
					    $addrmapHashRef,
					    $bigIntAddr,
					    $addrmapRelByteAddr,
					    $addrmapHashRef->{'busProtocol'},
					    $newAddrmapHashRef->{'inputPipelineStage'},
					    $newAddrmapHashRef->{'outputPipelineStage'},
					    \@prefixList,
					    \@hierarchicalPrefixList);
	$newAddrmapInfo->{'currAddrmapObj'} = $addrmapObj;
	return $addrmapObj;
    }
}
sub getNextInstArrayElement {
    my ($self,
	$currAddrmapInfo,
	$addrmapHashRef,
	$instHashRef) = @_;

    my (@hierarchicalPrefixList);
    my (@prefixList, $addrmapObj, $arrayIndex, $tmpAddr);
    my ($instIndex, $bigIntAddr, @addrmapPrefixList, $relAddr);
    my ($verilogFormat, $offset, $newAddrmapInfo, $newAddrmapHashRef);

    $arrayIndex = $currAddrmapInfo->{'currAddrmapInstanceArrayIndex'};
    ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($instHashRef->{'byteOffset'});
    $bigIntAddr = Math::BigInt->new($instHashRef->{'arrayElementByteSize'});

    if($arrayIndex == -1) {
	# we're just starting to process the first element of the array.
	$arrayIndex = $designDBRef->_getArrayMinIndices($instHashRef->{'arrayDimensionInfo'});
	$currAddrmapInfo->{'currAddrmapInstanceArrayIndex'} = $arrayIndex;
    }
    my $multiplier = $designDBRef->_getMultiplierFromArrayIndex($arrayIndex,
								$instHashRef->{'arrayDimensionInfo'});
    $bigIntAddr->bmul($multiplier);
    $bigIntAddr->badd($offset);
    $bigIntAddr->badd($currAddrmapInfo->{'parentAddr'});
    @prefixList = @{$currAddrmapInfo->{'prefixListRef'}};
    my $tmpInstName = $designDBRef->_getArrayElementName($instHashRef->{'name'}, $arrayIndex);
    push(@prefixList, $tmpInstName);
    $newAddrmapInfo->{'prefixListRef'} = \@prefixList;

    push (@hierarchicalPrefixList, (@{$currAddrmapInfo->{'hierarchicalPrefixListRef'}},
				    $tmpInstName));
    $newAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;

    $newAddrmapHashRef = $instHashRef;
    $newAddrmapInfo->{'currNode'} = $instHashRef;
    $newAddrmapInfo->{'currAddrmapHashRef'} = $newAddrmapHashRef;
    $newAddrmapInfo->{'parentAddr'} = $bigIntAddr;
    $newAddrmapInfo->{'arrayIndex'} = $multiplier;
    push(@visitedAddrmapsList, $newAddrmapInfo);

    ($verilogFormat, $tmpAddr) =
	$self->getVerilogBaseAndValue($newAddrmapHashRef->{'addressmapRelativeByteAddress'});
    my $addrmapRelByteAddr = Math::BigInt->new($newAddrmapHashRef->{'arrayElementByteSize'});
    $addrmapRelByteAddr->bmul($multiplier);
    $addrmapRelByteAddr->badd($tmpAddr);
    $self->setInitialAddrmapIndexes($newAddrmapInfo, $addrmapHashRef);
    $addrmapObj = csrPerlAPIModule->new($newAddrmapHashRef,
					$arrayIndex,
					$addrmapHashRef,
					$bigIntAddr,
					$addrmapRelByteAddr,
					$addrmapHashRef->{'busProtocol'},
					$addrmapHashRef->{'inputPipelineStage'},
					$addrmapHashRef->{'outputPipelineStage'},
					\@prefixList,
					\@hierarchicalPrefixList);
    $newAddrmapInfo->{'currAddrmapObj'} = $addrmapObj;
    return $addrmapObj;
}

sub updateInstanceAccounting {
    my ($self, $currAddrmapInfo) = @_;

    my $instHashRef = $currAddrmapInfo->{'currInstanceHashRef'};
    my $newAddrmapHashRef = $designDBRef->_findAddressmapForInstanceHash($instHashRef);
    $instHashRef = $newAddrmapHashRef;
    if (defined($instHashRef->{'arrayDimensionInfo'})) {
	my $arrayMax = $designDBRef->_getArrayMaxIndices($instHashRef->{'arrayDimensionInfo'});
	if ($self->getExpandArraysFlag()) {
	    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapInstanceArrayIndex'},
					       $instHashRef->{'arrayDimensionInfo'});
	    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapInstanceArrayIndex'},
						 $arrayMax) == 1) { # first arg > 2nd arg
		$currAddrmapInfo->{'currAddrmapInstanceIndex'}++;
		$currAddrmapInfo->{'currAddrmapInstanceArrayIndex'} = -1;
	    }
	} else {
	    $currAddrmapInfo->{'currAddrmapInstanceIndex'}++;
	    $currAddrmapInfo->{'currAddrmapInstanceArrayIndex'} = -1;
	}
    } else {
	$currAddrmapInfo->{'currAddrmapInstanceIndex'}++;
    }
}

sub getNextRegisterNode {
    my $self = shift;
    my $nextObj = $self->getNextDesignNode();
    if(defined($nextObj)) {
	my $nodeType = $nextObj->getNodeType();
	if (($nodeType eq 'register') ||
	    ($includeWideRegistersFlag && ($nodeType eq 'wideRegister'))) {
	    return $nextObj;
	} else {
	    $self->getNextRegisterNode();
	}
    } else {
	return;
    }
}

sub hasAddressmapInstance {
    my $self = shift;
    my $addrmapHashRef = $self->get_nodeHashRef();

    for(my $count = 0; $count < @{$addrmapHashRef->{'definition'}}; $count++) {
	return 1 if (defined($addrmapHashRef->{'definition'}) &&
		     defined($addrmapHashRef->{'definition'}->[$count]->{'instances'}));
    }
    return 0;
}

sub getNextAddressmapNode {
    my $self = shift;
    my $nodeType;

    my $skipExtRegsFlag = $self->getSkipExternalRegistersFlag();
    $self->setSkipExternalRegistersFlag();

    my @prefixList = ();
    my (@hierarchicalPrefixList);
    my ($instIndex, $instHashRef, $parentAddrmapHashRef, $relAddr);
    my ($nextObj, $prefixListRef, $offset, $currAddrmapInfo);
    my ($verilogFormat, $addrmapObj, $bigIntAddr, $addrmapHashRef);

    $updateRefArrayIndex = 1;
    my $arrayIndex = undef;
    if(@visitedAddrmapsList == 0) {
	# We're just starting to process the nodes.
	$addrmapHashRef = $self->get_nodeHashRef();
	($verilogFormat, $offset) = $self->getVerilogBaseAndValue($addrmapHashRef->{'byteAddress'});
	$bigIntAddr = Math::BigInt->new($offset);
	($verilogFormat, $relAddr) = $self->getVerilogBaseAndValue($addrmapHashRef->{'addressmapRelativeByteAddress'});
	my $relAddrBigInt = Math::BigInt->new($relAddr);
	push(@hierarchicalPrefixList, $addrmapHashRef->{'hierarchicalIdentifier'});
	$addrmapObj = csrPerlAPIModule->new($addrmapHashRef,
					    $arrayIndex,
					    $parentAddrmapHashRef,
					    $bigIntAddr,
					    $relAddrBigInt,
					    $addrmapHashRef->{'busProtocol'},
					    $addrmapHashRef->{'inputPipelineStage'},
					    $addrmapHashRef->{'outputPipelineStage'},
					    \@prefixList,
					    \@hierarchicalPrefixList);
	$currAddrmapInfo->{'currAddrmapHashRef'} = $addrmapHashRef;
	$currAddrmapInfo->{'currNode'} = $addrmapHashRef;
	$currAddrmapInfo->{'parentAddr'} = $bigIntAddr;
	$currAddrmapInfo->{'relativeAddress'} = $bigIntAddr;
	$self->setInitialAddrmapIndexes($currAddrmapInfo, $addrmapHashRef);
	push(@prefixList, $addrmapHashRef->{'name'}) if (!$self->getSemiforeEnvGenFlag());
	$currAddrmapInfo->{'prefixListRef'} = \@prefixList;
	$currAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	$currAddrmapInfo->{'currAddrmapObj'} = $addrmapObj;
	my $refNameHashListRef = [];
	$designDBRef->_genRefNameList($addrmapHashRef, $refNameHashListRef);
	$currAddrmapInfo->{'refNameHashListRef'} = $refNameHashListRef;
	push(@visitedAddrmapsList, $currAddrmapInfo);
	$self->setSkipExternalRegistersFlag($skipExtRegsFlag);
	return $addrmapObj;
    }
    $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
    $addrmapHashRef = $currAddrmapInfo->{'currAddrmapHashRef'};
    $prefixListRef = $currAddrmapInfo->{'prefixListRef'};

    my $currNode = $currAddrmapInfo->{'currNode'};
    my $currNodeType = $currNode->{'nodeType'};

    if (($currNodeType eq 'addressmap') || ($currNodeType eq 'addressmapInstance')) {
        if (($currNodeType ne 'memory') && $self->moreInstancesAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
            $nextObj = $self->getNextInstance($currAddrmapInfo, $addrmapHashRef);
	    $self->setSkipExternalRegistersFlag($skipExtRegsFlag);
            return $nextObj;
        } else {
	    pop(@visitedAddrmapsList);
            if(@visitedAddrmapsList == 0) {
                # We're done.
		$self->setSkipExternalRegistersFlag($skipExtRegsFlag);
                return;
            } else {
                $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
                $addrmapHashRef = $currAddrmapInfo->{'currAddrmapHashRef'};
		if ($currNodeType eq 'addressmapInstance') {
		    $self->updateInstanceAccounting($currAddrmapInfo);
		} else {
		    $self->_printError('Node type: ' . $currNodeType . ' not handled');
		    exit(1);
		}
                $nextObj = $self->getNextAddressmapNode();
		$self->setSkipExternalRegistersFlag($skipExtRegsFlag);
                return $nextObj;
            }
        }
    }
    $self->setSkipExternalRegistersFlag($skipExtRegsFlag);
    return;
}

sub getRegisterCount {
    my $self = shift;
    my ($regCount, $nextObj);
    $regCount = 0;
    $self->saveAddrmapInfoContext();
    $self->setSkipExternalAddressmapsFlag();
    while(defined($nextObj = $self->getNextRegisterNode())) {
	$regCount++;
    }
    $self->clearSkipExternalAddressmapsFlag();
    $self->restoreAddrmapInfoContext();
    return($regCount);
}
sub designHasFieldType {
    my ($self, $fieldType) = @_;
    if($designDBRef->_designHasFieldType($fieldType, $self->get_nodeHashRef())) {
	return 1;
    } else {
	return 0;
    }
}

package csrPerlAPIGroup;

use strict;
use vars '$AUTOLOAD';

@csrPerlAPIGroup::ISA = qw(csrPerlAPIBase);

{
    my %moduleAttrs = (
		       _definitionName => '',
		       _address => '',
		       _byteAddress => '',
		       _addressUnitsInBytes => '',
		       _slaveAddressUnitsInBytes => '',
		       _offset => '',
		       _byteOffset => '',
		       _addressWidth => '',
		       _wordByteSize => '',
		       _alignByteSize => '',
		       _offsetByteSize => '',
		       _actualByteSize => '',
		       _addressmapRelativeByteSize => '',
		       _topAddressmapRelativeByteSize => '',
		       _arrayElementByteSize => '',
		       _arrayByteStride => '',
		       _instanceAlignByteSize => '',
		       _totalByteSize => '',
		       _reservedByteSize => '',
		       _bitNumbering => '',
		       _byteEndian => '',
		       _busProtocol => '',
		       _inputPipelineStage => '',
		       _outputPipelineStage => '',
		       _addressmapRelativeByteAddress => '',
		       _addressmapRelativeByteAddressWidth => '',
		       _timescale => '',
                       _resetTiming => '',
		       _ports => '',
		       _externalBusPorts => '',
		       _atomicAccess => '',
		       _synchronizerStyle => '',
                       _hasArrayDecoder => '',
		       _nodeListRef => '',
		       _nextRegisterIndex => '-1',
		       _nextExternalBusPortIndex => '-1',
		       _parentHashRef => '',
		       _prefixNameList => '',
		       _totalNumPorts  => 0,
                       _publicAndNotEmpty => 1,
		       _visitedAddrmapsList => 0,
		       );

    sub _defaultAttrKeys {
	my $self = shift;
	return ($self->SUPER::_defaultAttrKeys(), map { s/^_//; $_ } keys %moduleAttrs);
    }
    sub new {
	my($caller,
	   $groupHashRef,
	   $arrayIndex,
	   $parentHashRef,
	   $addrmapAddress,
	   $addrmapRelByteAddr,
	   $busProtocol,
	   $inputPipelineStage,
	   $outputPipelineStage,
	   $prefixListRef,
	   $hierPrefixListRef) = @_;

        my ($resetTiming, $expandedGroupName, $tmpHierName);
	my ($count, $addrmapPrefixName, $tmpPortListRef);

	# pass the caller's class name to the base class's constructor.
	my $self = $_[0]->SUPER::new($groupHashRef);
	$self->set_designPkgName($designPkgName);
	$self->set_nodeHashRef($groupHashRef);
	$self->set_parentHashRef($parentHashRef);
	$self->set_totalNumPorts(0);
	my @newVisitedAddrmapsList = @visitedAddrmapsList;
	$self->set_visitedAddrmapsList(\@newVisitedAddrmapsList);
	my $declaredExternal = 'false';
	my $hasArrayDecoder = 'false';

	if (defined($groupHashRef->{'arrayDimensionInfo'}) && ($groupHashRef->{'external'} eq 'true')) {
	    if (defined($declaredExternal = $groupHashRef->{'declaredExternal'}) &&
		($declaredExternal eq 'true') &&
		defined($groupHashRef->{'hasArrayDecoder'})) {
		$hasArrayDecoder = $groupHashRef->{'hasArrayDecoder'};
	    }
	}
	if(@{$prefixListRef} > 0) {
#	    if (($hasArrayDecoder eq 'true') && $self->getSemiforeEnvGenFlag()) {
#		$prefixListRef->[@{$prefixListRef}-1] =~ s/\[\d+\]$//;
#		$hierPrefixListRef->[@{$hierPrefixListRef}-1] =~ s/\[\d+\]$//;
#	    }
            $expandedGroupName = join('_', @$prefixListRef);
            if ($viewName ne 'sv') {
                $expandedGroupName =~ s/\[/_/g;
                $expandedGroupName =~ s/\]//g;
            }
	    $self->set_name(join('_', $expandedGroupName));
            my $hierId = join('.', @$hierPrefixListRef);
	    $tmpHierName = join('_', @$hierPrefixListRef);
            if ($viewName ne 'sv') {
                $tmpHierName =~ s/\[/_/g;
                $tmpHierName =~ s/\]//g;
            } else {
                my @arrayIndex;
                while ($tmpHierName =~ /(\[\d+\])/g) {
                    push (@arrayIndex, $1);
                }
                $tmpHierName =~ s/\[\d+\]//g;
                $tmpHierName .= join('', @arrayIndex);
                $hierId =~ s/\[\d+\]//g;
                $hierId .= join('', @arrayIndex);
            }
            $self->set_hierarchicalName($tmpHierName);
            $self->set_hierarchicalNameDotted(join('.', $hierId));
	} else {
	    $expandedGroupName = $self->getName();
	    $tmpHierName = join('_', @$hierPrefixListRef);
            if ($viewName ne 'sv') {
                $tmpHierName =~ s/\[/_/g;
                $tmpHierName =~ s/\]//g;
            }
	    $self->set_hierarchicalName($tmpHierName);
	    $self->set_hierarchicalNameDotted(join('.', @$hierPrefixListRef));
	}
	if(defined($arrayIndex)) {
	    # copy the array indices before calling updateGroupAccounting;
	    $self->set_arrayIndex([@{$arrayIndex}]);
	}
	$self->assignAddressInfo($addrmapAddress,
				 $groupHashRef->{'byteOffset'},
				 $groupHashRef->{'offset'},
				 $addrmapRelByteAddr);

	if (defined($parentHashRef) && ($parentHashRef->{'nodeType'} eq 'group')) {
	    my $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-2];
	    my $parentByteAddr = $currAddrmapInfo->{'parentAddr'};
	    $self->set_groupRelativeByteAddress($self->getByteAddress() - $parentByteAddr);
	}
	$self->setBusProtocol($groupHashRef,
			      $parentHashRef,
			      $busProtocol);
        if (defined($resetTiming = $self->getResetTiming())) {
	    $self->set_resetTiming('synchronous') if ($resetTiming eq 'bus_protocol');
        }
	$self->set_inputPipelineStage($inputPipelineStage);
	$self->set_outputPipelineStage($outputPipelineStage);
	my $portListRef = [];

	if (exists($groupHashRef->{'externalBusPorts'})) {
	    # Userdefined group arrays that have array decoder have only one set of ports.
	    $portListRef = csrPorts->new($self,
					 $groupHashRef,
					 $groupHashRef->{'externalBusPorts'},
					 $parentHashRef,
					 $expandedGroupName);
	    $self->set_externalBusPorts($portListRef);
	    $self->set_nextExternalBusPortIndex(-1);
        }
	if (exists($groupHashRef->{'ports'})) {
	    # Userdefined group arrays that have array decoder have only one set of ports.
	    $portListRef = csrPorts->new($self,
					 $groupHashRef,
					 $groupHashRef->{'ports'},
					 $parentHashRef,
					 $expandedGroupName);
	    $self->set_ports($portListRef);
        }
        my $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
        my $tmpAddrmapHash = $currAddrmapInfo->{'currAddrmapHashRef'};
	return $self;
    }

    sub getNextDesignNode {
	my $self = shift;

	# Iterate thru the children of a group node.

	my @prefixList = ();
	my ($currAddrmapInfo, $addrmapHashRef);
	my ($subRegHashListRef, $nextObj, $relAddr);

	if(@visitedAddrmapsList == 0) {
	    # We're just starting to process the nodes.
	    my $parentAddrmapHashRef;
	    my $arrayIndex = undef;
	    $addrmapHashRef = $self->get_nodeHashRef();
	    my ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($self->getByteAddress());
	    my $bigIntAddr = Math::BigInt->new($offset);
	    ($verilogFormat,
	     $relAddr) = $self->getVerilogBaseAndValue($addrmapHashRef->{'addressmapRelativeByteAddress'});
	    my $relAddrBigInt = Math::BigInt->new($relAddr);
	    my $hierId = $self->getHierarchicalIdentifier();
	    my @hierarchicalPrefixList = split('\.', $hierId);
	    my @prefixList = split('\.', $hierId);
	    my $addrmapObj = csrPerlAPIGroup->new($addrmapHashRef,
						  $arrayIndex,
						  $parentAddrmapHashRef,
						  $bigIntAddr,
						  $relAddrBigInt,
						  $addrmapHashRef->{'busProtocol'},
						  $addrmapHashRef->{'inputPipelineStage'},
						  $addrmapHashRef->{'outputPipelineStage'},
						  \@prefixList,
						  \@hierarchicalPrefixList);
	    $currAddrmapInfo->{'currAddrmapHashRef'} = $addrmapHashRef;
	    $currAddrmapInfo->{'currNode'} = $addrmapHashRef;
	    $currAddrmapInfo->{'parentAddr'} = $bigIntAddr;
	    $currAddrmapInfo->{'relativeAddress'} = $bigIntAddr;
	    $self->setInitialAddrmapIndexes($currAddrmapInfo, $addrmapHashRef);
	    push(@prefixList, $addrmapHashRef->{'name'}) if (!$self->getSemiforeEnvGenFlag());
	    $currAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	    $currAddrmapInfo->{'prefixListRef'} = \@prefixList;
	    push(@visitedAddrmapsList, $currAddrmapInfo);
	    return $addrmapObj;
	}
	return if(@visitedAddrmapsList == 0);

	$currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
	$addrmapHashRef = $currAddrmapInfo->{'currAddrmapHashRef'};
	my $prefixListRef = $currAddrmapInfo->{'prefixListRef'};

	if (!defined($currAddrmapInfo->{'groupParentAddressmapObj'})) {
	    $currAddrmapInfo->{'groupParentAddressmapObj'} = $self->getParentNode();
	}
	my $parentAddrmapNode = $currAddrmapInfo->{'groupParentAddressmapObj'};
	my $currNode = $currAddrmapInfo->{'currNode'};
	my $currNodeType = $currNode->{'nodeType'};
	# Memory objects may have virtual registers.
	if(($currNodeType eq 'group') || ($currNodeType eq 'memory')) {
	    if($self->moreRegistersAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
		$nextObj = $self->getNextRegister($currAddrmapInfo, $addrmapHashRef);
		# We may've just processed a wide register (and pushed it in the
		# LIFO. Wide register accounting is done somewhere else.
		$currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
		if(($currAddrmapInfo->{'currNode'})->{'nodeType'} ne 'wideRegister') {
		    $self->updateRegisterAccounting($currAddrmapInfo);
		}
		return $nextObj;
	    } elsif (($currNodeType ne 'memory') &&
		     $self->moreMemoriesAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
		$nextObj = $self->getNextMemory($currAddrmapInfo, $addrmapHashRef);
		return $nextObj;
	    } elsif ($self->moreGroupsAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
		$nextObj = $self->getNextGroup($currAddrmapInfo, $addrmapHashRef);
		return $nextObj;
	    } else {
		pop(@visitedAddrmapsList);
		if(@visitedAddrmapsList == 0) {
		    # We're done.
		    return;
		} else {
		    $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
		    $addrmapHashRef = $currAddrmapInfo->{'currAddrmapHashRef'};
		    if ($currNodeType eq 'memory') {
			$self->updateMemoryAccounting($currAddrmapInfo);
		    } elsif ($currNodeType eq 'group') {
			$self->updateGroupAccounting($currAddrmapInfo);
		    } else {
			$self->_printError('Node type: ' . $currNodeType . ' not handled');
			exit(1);
		    }
		    $nextObj = $self->getNextDesignNode();
		    return $nextObj;
		}
	    }
	} elsif($currNodeType eq 'wideRegister') {
	    my $regHashRef = $currAddrmapInfo->{'currRegisterHashRef'};
	    my $subRegHashListRef = $regHashRef->{'subRegisters'};
	    if(($currAddrmapInfo->{'currAddrmapSubRegisterIndex'} >= @{$subRegHashListRef}) &&
	       (defined($regHashRef->{'arrayDimensionInfo'}))) {
		# we have exhausted the sub register list for the current array element.
		my $arrayMax = $designDBRef->_getArrayMaxIndices($regHashRef->{'arrayDimensionInfo'});
		if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
						     $arrayMax) <= 0) {
		    # There's at least another array element. We need to generate
		    # an object for the new wide register array element before
		    # continuing with sub-registers.
		    $nextObj = $self->getNextWideRegArrayElement($currAddrmapInfo, $addrmapHashRef);
		    return $nextObj;
		}
	    }
	    $nextObj = $self->getNextSubRegister($currAddrmapInfo, $addrmapHashRef);
	    $self->updateWideRegisterAccounting($currAddrmapInfo, $addrmapHashRef);
	    return $nextObj;
	}
	return;
    }

    sub getNextDesignNode_NoDepth {
	my $self = shift;

	# Iterate thru the children of a group node.

	my @prefixList = ();
	my ($parentAddrmapHashRef);
	my ($currAddrmapInfo, $addrmapHashRef, $relAddr);
	my ($subRegHashListRef, $nextObj, $prefixListRef, $regHashRef);

	if(@visitedAddrmapsList == 0) {
	    # We're just starting to process the nodes.
	    my $arrayIndex = undef;
	    $addrmapHashRef = $self->get_nodeHashRef();
	    my ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($self->getByteAddress());
	    my $bigIntAddr = Math::BigInt->new($offset);
	    ($verilogFormat,
	     $relAddr) = $self->getVerilogBaseAndValue($addrmapHashRef->{'addressmapRelativeByteAddress'});
	    my $relAddrBigInt = Math::BigInt->new($relAddr);
	    my $hierId = $self->getHierarchicalIdentifier();
	    my @hierarchicalPrefixList = split('\.', $hierId);
	    my @prefixList = split('\.', $hierId);
            shift @prefixList;
	    my $addrmapObj = csrPerlAPIGroup->new($addrmapHashRef,
						  $arrayIndex,
						  $parentAddrmapHashRef,
						  $bigIntAddr,
						  $relAddrBigInt,
						  $addrmapHashRef->{'busProtocol'},
						  $addrmapHashRef->{'inputPipelineStage'},
						  $addrmapHashRef->{'outputPipelineStage'},
						  \@prefixList,
						  \@hierarchicalPrefixList);
	    $currAddrmapInfo->{'currAddrmapHashRef'} = $addrmapHashRef;
	    $currAddrmapInfo->{'currNode'} = $addrmapHashRef;
	    $currAddrmapInfo->{'parentAddr'} = $bigIntAddr;
	    $currAddrmapInfo->{'relativeAddress'} = $bigIntAddr;
	    $self->setInitialAddrmapIndexes($currAddrmapInfo, $addrmapHashRef);
	    push(@prefixList, $addrmapHashRef->{'name'}) if (!$self->getSemiforeEnvGenFlag());
	    $currAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	    $currAddrmapInfo->{'prefixListRef'} = \@prefixList;
	    push(@visitedAddrmapsList, $currAddrmapInfo);
	    $tmpIncludeWideRegistersFlag = $self->getIncludeWideRegistersFlag();
	    $tmpIncludeWideMemoriesFlag = $self->getIncludeWideMemoriesFlag();
	    $self->setIncludeWideRegistersFlag($tmpIncludeWideRegistersFlag);
	    $self->setIncludeWideMemoriesFlag($tmpIncludeWideMemoriesFlag);
	    return $addrmapObj;
        }
	$currAddrmapInfo = $visitedAddrmapsList[0];
	$addrmapHashRef = $currAddrmapInfo->{'currAddrmapHashRef'};
	$prefixListRef = $currAddrmapInfo->{'prefixListRef'};

	if (!defined($currAddrmapInfo->{'groupParentAddressmapObj'})) {
	    $currAddrmapInfo->{'groupParentAddressmapObj'} = $self->getParentNode();
	}
	my $parentAddrmapNode = $currAddrmapInfo->{'groupParentAddressmapObj'};
	my $currNode = $currAddrmapInfo->{'currNode'};
	my $currNodeType = $currNode->{'nodeType'};

	# Memory objects may have virtual registers.
	if(($currNodeType eq 'group') || ($currNodeType eq 'memory')) {
	    if($self->moreRegistersAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
		$nextObj = $self->getNextRegister($currAddrmapInfo, $addrmapHashRef);
		# We may've just processed a wide register (and pushed it in the
		# LIFO. Wide register accounting is done somewhere else.
		$currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
		if(($currAddrmapInfo->{'currNode'})->{'nodeType'} ne 'wideRegister') {
		    $self->updateRegisterAccounting($currAddrmapInfo);
		}
		return $nextObj;
	    } elsif (($currNodeType ne 'memory') &&
		     $self->moreMemoriesAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
		$nextObj = $self->getNextMemory($currAddrmapInfo, $addrmapHashRef);
		pop(@visitedAddrmapsList);
		$self->updateMemoryAccounting($currAddrmapInfo);
		return $nextObj;
	    } elsif ($self->moreGroupsAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
		$nextObj = $self->getNextGroup($currAddrmapInfo, $addrmapHashRef);
		pop(@visitedAddrmapsList);
		$self->updateGroupAccounting($currAddrmapInfo);
		return $nextObj;
	    } else {
		pop(@visitedAddrmapsList);
		if(@visitedAddrmapsList == 0) {
		    # We're done.
		    $self->setIncludeWideRegistersFlag($tmpIncludeWideRegistersFlag);
		    $self->setIncludeWideMemoriesFlag($tmpIncludeWideMemoriesFlag);
		    return;
		} else {
		    $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
		    $addrmapHashRef = $currAddrmapInfo->{'currAddrmapHashRef'};
		    if ($currNodeType eq 'memory') {
			$self->updateMemoryAccounting($currAddrmapInfo);
		    } elsif ($currNodeType eq 'group') {
			$self->updateGroupAccounting($currAddrmapInfo);
		    } else {
			$self->_printError('Node type: ' . $currNodeType . ' not handled');
			exit(1);
		    }
		    $nextObj = $self->getNextDesignNode();
		    return $nextObj;
		}
	    }
	    return $nextObj;
	}
	$self->setIncludeWideRegistersFlag($tmpIncludeWideRegistersFlag);
	$self->setIncludeWideMemoriesFlag($tmpIncludeWideMemoriesFlag);
	return;
    }

    sub getAddressmapIOPorts {
	my $self = shift;
	my $groupIOPorts;

	my $inputPorts = $self->getPortList('input');
	push(@$groupIOPorts, @$inputPorts) if(defined($inputPorts));
	my $outputPorts = $self->getPortList('output');
	push(@$groupIOPorts, @$outputPorts) if(defined($outputPorts));
	return $groupIOPorts;
    }
    sub getExternalIOPorts {
	my $self = shift;
	my $addrmapIOPorts;

	my $inputPorts = $self->getExternalPortList('input');
	if(defined($inputPorts)) {
	    push(@$addrmapIOPorts, @$inputPorts);
	}
	my $outputPorts = $self->getExternalPortList('output');
	if(defined($outputPorts)) {
	    push(@$addrmapIOPorts, @$outputPorts);
	}
	return $addrmapIOPorts;
    }
    sub getDeclaredExternalObjectPortList {
	my($self, $portDir) = @_;

	return if (!$self->isDeclaredExternal());
	return $self->getExternalPortList($portDir);
    }
    sub getObjectPortsList_NoExternalPorts {
	my($self, $portDir) = @_;
	my $noExternalPorts = 1;
	return ($self->getPortList($portDir, $noExternalPorts));
    }
    sub getObjectPortsList {
	my($self, $portDir) = @_;
	return ($self->getPortList($portDir));
    }
    sub getPortList {
	my($self, $direction, $noExternalPorts) = @_;

	my ($portObj, @portList);

	# skip ports when userdefined group within another group, or
	# when userdefined group within a userdefined address map.

	my $parentNode = $self->getParentNode();
	return if ($self->isExternal() &&
		   (defined($parentNode = $self->getParentNode()) && 
		    ($parentNode->isExternal() || ($parentNode->getNodeType() eq 'group'))));

	my $portListRef = $self->getPorts();
	if (defined($portListRef)) {
	    my $currPortIndex = 0;
	    while ($portObj = $self->getNextPortNode($portListRef,
						     \$currPortIndex)) {
		push(@portList, $portObj) if($direction eq $portObj->getPortDirection());
	    }
	}
	$noExternalPorts = 0 if (!defined($noExternalPorts));
	return \@portList if ($noExternalPorts);
	my $extPortListRef = $self->getExternalBusPorts();
	return \@portList if (!defined($extPortListRef));
	my $currPortIndex = 0;
	while ($portObj = $self->getNextExternalPortNode($extPortListRef,
							 \$currPortIndex)) {
	    push(@portList, $portObj) if ($direction eq $portObj->getPortDirection());
	}
	return \@portList;
    }
    sub getByteAddress {
        my $self = shift;
        return Math::BigInt->new($self->get_byteAddress());
    }
    sub getAddressUnitsInBytes {
        my $self = shift;
        return $self->get_addressUnitsInBytes();
    }
    sub getSlaveAddressUnitsInBytes {
        my $self = shift;
        return $self->get_slaveAddressUnitsInBytes();
    }
    sub getAddressWidth {
        my $self = shift;
        return $self->get_addressWidth();
    }
    sub getAddressmapRelativeByteAddress {
        my $self = shift;
        return Math::BigInt->new($self->get_addressmapRelativeByteAddress());
    }
    sub getAddressmapRelativeByteAddressWidth {
        my $self = shift;
        return $self->get_addressmapRelativeByteAddressWidth();
    }
    sub getGroupRelativeByteAddress {
        my $self = shift;
        return Math::BigInt->new($self->get_groupRelativeByteAddress());
    }
    sub getActualByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_actualByteSize());
    }
    sub getAddressmapRelativeByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_addressmapRelativeByteSize());
    }
    sub getTopAddressmapRelativeByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_topAddressmapRelativeByteSize());
    }
    sub getArrayByteStride {
        my $self = shift;
        return Math::BigInt->new($self->get_arrayByteStride());
    }
    sub getInstanceAlignByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_instanceAlignByteSize());
    }
    sub getReservedByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_reservedByteSize());
    }
    sub getArrayElementByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_arrayElementByteSize());
    }
    sub getAlignByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_alignByteSize());
    }
    sub getTotalByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_totalByteSize());
    }
    sub getByteEndian {
        my $self = shift;
        return $self->get_byteEndian();
    }
    sub getByteOffset {
        my $self = shift;
        return Math::BigInt->new($self->get_byteOffset());
    }
    sub getWordByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_wordByteSize());
    }
    sub getWordSizeBits {
        my $self = shift;
        return Math::BigInt->new(($self->get_wordByteSize()) * 8);
    }
    sub getResetTiming {
        my $self = shift;
        return $self->get_resetTiming();
    }
    sub getBusProtocol {
        my $self = shift;
        return $self->get_busProtocol();
    }
    sub getInputPipelineStage {
        my $self = shift;
        return $self->get_inputPipelineStage();
    }
    sub getOutputPipelineStage {
        my $self = shift;
        return $self->get_outputPipelineStage();
    }
    sub getIOPipelineStage {
        my $self = shift;
        my $inputPipe = $self->getInputPipelineStage();
        my $outputPipe = $self->getOutputPipelineStage();
        return ($inputPipe, $outputPipe);
    }
    sub getOffsetByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_offsetByteSize());
    }
    sub hasArrayDecoder {
	my $self = shift;
	my $hasArrayDecoder = 'false';
	if (defined($hasArrayDecoder = $self->get_hasArrayDecoder()) &&
	    ($hasArrayDecoder eq 'true')) {
	    return 1;
	} else {
	    return 0;
	}
    }

    sub hasInputSynchronizer {
	my $self = shift;
	return 0;
    }
    sub getDefinitionName {
	my $self = shift;
	my $defName;
	return $defName	if (defined($defName = $self->get_definitionName()));
	return;
    }
    sub getPorts {
        my $self = shift;
        return $self->get_ports();
    }
    sub getExternalBusPorts {
        my $self = shift;
        return $self->get_externalBusPorts();
    }
    sub hasPortType {
        my ($self, $portType) = @_;

        my ($portListRef, $port, $subRegIndex);
        my ($parentNode, @subRegList);

	if ($portType =~ /external/) {
	    $portListRef = $self->getExternalBusPorts();
	} else {
	    $portListRef = $self->getPorts();
	}
        return 0 if(!defined($portListRef));
        foreach my $portObj (@{$portListRef}) {
            if($portObj->getPortType() eq $portType) {
                return 1;
            }
        }
        return 0;
    }
    sub getPortType {
        my ($self, $portType) = @_;

        my ($portListRef);

	if (!defined($portType)) {
	    $self->_printError('Port type not defined');
	    exit(1);
	}
	if ($portType =~ /external/) {
	    $portListRef = $self->getExternalBusPorts();
	} else {
	    $portListRef = $self->getPorts();
	}
        return if (!defined($portListRef));
        foreach my $portObj (@{$portListRef}) {
	    return $portObj if($portObj->getPortType() eq $portType);
        }
        return;
    }
    sub hasWideRegister {
        my $self = shift;
        $self->saveAddrmapInfoContext();
        my $addrmapNode = $self->get_addressmapNode();
        while(my $nextObj = $addrmapNode->getNextRegisterNode()) {
            if($nextObj->getNodeType() eq 'wideRegister') {
                $self->restoreAddrmapInfoContext();
                return 1;
            }
        }
        $self->restoreAddrmapInfoContext();
        return 0;
    }
    sub hasWideMemory {
        my $self = shift;
        $self->saveAddrmapInfoContext();
        while(my $nextObj = $self->getNextDesignNode()) {
            if($nextObj->getNodeType() eq 'wideMemory') {
                $self->restoreAddrmapInfoContext();
                return 1;
            }
        }
        $self->restoreAddrmapInfoContext();
        return 0;
    }
    sub hasWideAtomicRegister {
        my $self = shift;
        $self->saveAddrmapInfoContext();
        while(my $nextObj = $self->getNextRegisterNode()) {
            if($nextObj->isAtomic()) {
                $self->restoreAddrmapInfoContext();
                return $nextObj;
            }
        }
        $self->restoreAddrmapInfoContext();
        return;
    }
    sub getParentNode {
        my $self = shift;

        # get the parent address map or group object for the calling object.

        my $count = 0;
	my ($verilogFormat, $relAddr, $arrayIndex);
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage);
        my (@prefixList, $parentHashRef, $parentOfParentHashRef);
        my ($topLevelHashRef, $addrmapObj, $parentName, $bogusAddr);

        my $bigIntAddr = Math::BigInt->new('0');
        my $hierId = $self->getHierarchicalIdentifier();
        my @levelNameList = split('\.', $self->get_hierarchicalNameDotted());
        my @hierLevelNameList = split('\.', $self->get_hierarchicalNameDotted());

        return if(@levelNameList <= 1);

        ($topLevelHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);

        pop(@levelNameList);
        pop(@hierLevelNameList);
        # start from the last level name.
        while(@levelNameList) {
            $parentName = join('.', @levelNameList);
            ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
            if(($parentHashRef->{'nodeType'} eq 'addressmap') ||
               ($parentHashRef->{'nodeType'} eq 'addressmapInstance') ||
               ($parentHashRef->{'nodeType'} eq 'memory') || # virtual groups have memory parent
               ($parentHashRef->{'nodeType'} eq 'wideMemory') ||
               ($parentHashRef->{'nodeType'} eq 'group')) {
                ($busProtocol, $inputPipelineStage, $outputPipelineStage) =
                    $designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);
                @prefixList = @levelNameList;
                shift @prefixList;
                pop(@levelNameList);
                # parent of parent hash ref
                if(@levelNameList > 0) {
                    # get to the parent of parent node
                    if(@levelNameList == 1) {
                        $parentName = $levelNameList[0];
                    } else {
                        $parentName = join('.', @levelNameList);
                    }
                    ($parentOfParentHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($parentName);
                }
		($verilogFormat, $relAddr) =
		    $self->getVerilogBaseAndValue($parentHashRef->{'addressmapRelativeByteAddress'});
		my $relAddrBigInt = Math::BigInt->new($relAddr);
                $arrayIndex = $self->getParentArrayIndexFromHierId($hierId);
                if (($viewName eq 'sv') && (@$arrayIndex)) {
                    my $tmpIndex;
                    for (my $count=0; $count < @$arrayIndex; $count++) {
                        $tmpIndex .= '[' . $arrayIndex->[$count] . ']';
                    }
                    $hierLevelNameList[@hierLevelNameList-1] .= $tmpIndex;
                    $prefixList[@prefixList-1] .= $tmpIndex;
                }
		if ($parentHashRef->{'nodeType'} eq 'group') {
		    $addrmapObj = csrPerlAPIGroup->new($parentHashRef,
						       $arrayIndex,
						       $parentOfParentHashRef,
						       $bigIntAddr,
						       $relAddrBigInt,
						       $busProtocol,
						       $inputPipelineStage,
						       $outputPipelineStage,
						       \@prefixList,
						       \@hierLevelNameList);
                } elsif (($parentHashRef->{'nodeType'} eq 'memory') ||
                         ($parentHashRef->{'nodeType'} eq 'wideMemory')) {
                    my $bigIntOffset;
                    ($verilogFormat, $bigIntOffset) =
                                     $self->getVerilogBaseAndValue($parentHashRef->{'byteOffset'});
                    $bigIntOffset = Math::BigInt->new($bigIntOffset);
                    $addrmapObj = csrPerlAPIMemory->new($parentHashRef,
						      $arrayIndex,
						      $parentOfParentHashRef,
						      $bigIntAddr,
						      $bigIntOffset,
						      $relAddrBigInt,
						      $busProtocol,
						      $inputPipelineStage,
						      $outputPipelineStage,
						      \@prefixList,
						      \@hierLevelNameList);
		} else { # the parent must be either an addressmap or instance 
		    $addrmapObj = csrPerlAPIModule->new($parentHashRef,
							$arrayIndex,
							$parentOfParentHashRef,
							$bigIntAddr,
							$relAddrBigInt,
							$busProtocol,
							$inputPipelineStage,
							$outputPipelineStage,
							\@prefixList,
							\@hierLevelNameList);
		}
                return $addrmapObj;
            }
            if (defined($parentHashRef->{'arrayDimensionInfo'})) {
                my $numDims = scalar(@{$parentHashRef->{'arrayDimensionInfo'}});
                for (my $count=0; $count < $numDims; $count++) {
                    pop(@$arrayIndex);
                }
            }
            pop(@levelNameList);
            pop(@hierLevelNameList);
        }
        return;
    }
    sub isPublicAndNotEmpty {
        my $self = shift;
	my $isPANE;
	if (defined($isPANE = $self->get_isPublicAndNotEmpty())) {
	    return $isPANE;
	}
	return 0;
    }        
}

package csrPerlAPIUnion;

use strict;
use vars '$AUTOLOAD';

@csrPerlAPIUnion::ISA = qw(csrPerlAPIBase);

{
    my %moduleAttrs = (
		       _definitionName => '',
		       _address => '',
		       _byteAddress => '',
		       _addressUnitsInBytes => '',
		       _slaveAddressUnitsInBytes => '',
		       _offset => '',
		       _byteOffset => '',
		       _addressWidth => '',
		       _wordByteSize => '',
		       _alignByteSize => '',
		       _offsetByteSize => '',
		       _actualByteSize => '',
		       _addressmapRelativeByteSize => '',
		       _topAddressmapRelativeByteSize => '',
		       _arrayElementByteSize => '',
		       _arrayByteStride => '',
		       _instanceAlignByteSize => '',
		       _totalByteSize => '',
		       _reservedByteSize => '',
		       _bitNumbering => '',
		       _byteEndian => '',
		       _busProtocol => '',
		       _inputPipelineStage => '',
		       _outputPipelineStage => '',
		       _addressmapRelativeByteAddress => '',
		       _addressmapRelativeByteAddressWidth => '',
		       _timescale => '',
                       _resetTiming => '',
		       _ports => '',
		       _externalBusPorts => '',
		       _atomicAccess => '',
		       _synchronizerStyle => '',
                       _hasArrayDecoder => '',
		       _nodeListRef => '',
		       _nextRegisterIndex => '-1',
		       _nextExternalBusPortIndex => '-1',
		       _parentHashRef => '',
		       _prefixNameList => '',
		       _totalNumPorts  => 0,
                       _publicAndNotEmpty => 1,
		       _visitedAddrmapsList => 0,
		       );

    sub _defaultAttrKeys {
	my $self = shift;
	return ($self->SUPER::_defaultAttrKeys(), map { s/^_//; $_ } keys %moduleAttrs);
    }
    sub new {
	my($caller,
	   $unionHashRef,
	   $arrayIndex,
	   $parentHashRef,
	   $addrmapAddress,
	   $addrmapRelByteAddr,
	   $busProtocol,
	   $inputPipelineStage,
	   $outputPipelineStage,
	   $prefixNameListRef,
	   $hierPrefixListRef) = @_;

        my ($resetTiming, $expandedUnionName, $tmpHierName);
        my ($count, $addrmapPrefixName, $tmpPortListRef);

        # pass the caller's class name to the base class's constructor.
        my $self = $_[0]->SUPER::new($unionHashRef);
        $self->set_designPkgName($designPkgName);
        $self->set_nodeHashRef($unionHashRef);
        $self->set_parentHashRef($parentHashRef);
        $self->set_totalNumPorts(0);
        my @newVisitedAddrmapsList = @visitedAddrmapsList;
        $self->set_visitedAddrmapsList(\@newVisitedAddrmapsList);
        my $declaredExternal = 'false';
        my $hasArrayDecoder = 'false';

        if (defined($unionHashRef->{'arrayDimensionInfo'}) &&
            defined($unionHashRef->{'hasArrayDecoder'})) {
            $hasArrayDecoder = $unionHashRef->{'hasArrayDecoder'};
        }
        if(@{$prefixNameListRef} > 0) {
            if (($hasArrayDecoder eq 'true') && $self->getSemiforeEnvGenFlag()) {
                $prefixNameListRef->[@{$prefixNameListRef}-1] =~ s/\[\d+\]$//;
                $hierPrefixListRef->[@{$hierPrefixListRef}-1] =~ s/\[\d+\]$//;
            }
            $expandedUnionName = join('_', @$prefixNameListRef);
            if ($viewName ne 'sv') {
                $expandedUnionName =~ s/\[/_/g;
                $expandedUnionName =~ s/\]//g;
            }
	    $self->set_name(join('_', $expandedUnionName));
            my $hierId = join('.', @$hierPrefixListRef);
	    $tmpHierName = join('_', @$hierPrefixListRef);
            if ($viewName ne 'sv') {
                $tmpHierName =~ s/\[/_/g;
                $tmpHierName =~ s/\]//g;
            } else {
                my @arrayIndex;
                while ($tmpHierName =~ /(\[\d+\])/g) {
                    push (@arrayIndex, $1);
                }
                $tmpHierName =~ s/\[\d+\]//g;
                $tmpHierName .= join('', @arrayIndex);
                $hierId =~ s/\[\d+\]//g;
                $hierId .= join('', @arrayIndex);
            }
	    $self->set_hierarchicalName($tmpHierName);
	    $self->set_hierarchicalNameDotted(join('.', $hierId));
	} else {
	    $expandedUnionName = $self->getName();
	    $tmpHierName = join('_', @$hierPrefixListRef);
            if ($viewName ne 'sv') {
                $tmpHierName =~ s/\[/_/g;
                $tmpHierName =~ s/\]//g;
            }
	    $self->set_hierarchicalName($tmpHierName);
	    $self->set_hierarchicalNameDotted(join('.', @$hierPrefixListRef));
	}
	if(defined($arrayIndex)) {
	    # copy the array indices before calling updateUnionAccounting;
	    $self->set_arrayIndex([@{$arrayIndex}]);
	}
	$self->assignAddressInfo($addrmapAddress,
				 $unionHashRef->{'byteOffset'},
				 $unionHashRef->{'offset'},
				 $addrmapRelByteAddr);

#	if (defined($parentHashRef) && ($parentHashRef->{'nodeType'} eq 'union')) {
#	    my $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-2];
#	    my $parentByteAddr = $currAddrmapInfo->{'parentAddr'};
#	    $self->set_unionRelativeByteAddress($self->getByteAddress() - $parentByteAddr);
#	}
	$self->setBusProtocol($unionHashRef,
			      $parentHashRef,
			      $busProtocol);
        if (defined($resetTiming = $self->getResetTiming())) {
	    $self->set_resetTiming('synchronous') if ($resetTiming eq 'bus_protocol');
        }
	$self->set_inputPipelineStage($inputPipelineStage);
	$self->set_outputPipelineStage($outputPipelineStage);
	my $portListRef = [];
	if (exists($unionHashRef->{'externalBusPorts'})) {
	    # Userdefined union arrays that have array decoder have only one set of ports.
	    $portListRef = csrPorts->new($self,
					 $unionHashRef,
					 $unionHashRef->{'externalBusPorts'},
					 $parentHashRef,
					 $expandedUnionName);
	    $self->set_externalBusPorts($portListRef);
	    $self->set_nextExternalBusPortIndex(-1);
        }
	if (exists($unionHashRef->{'ports'})) {
	    # Userdefined union arrays that have array decoder have only one set of ports.
	    $portListRef = csrPorts->new($self,
					 $unionHashRef,
					 $unionHashRef->{'ports'},
					 $parentHashRef,
					 $expandedUnionName);
	    $self->set_ports($portListRef);
        }
	return $self;
    }

    sub getAddressmapIOPorts {
	my $self = shift;
	my $unionIOPorts;

	my $inputPorts = $self->getPortList('input');
	push(@$unionIOPorts, @$inputPorts) if(defined($inputPorts));
	my $outputPorts = $self->getPortList('output');
	push(@$unionIOPorts, @$outputPorts) if(defined($outputPorts));
	return $unionIOPorts;
    }
    sub getExternalIOPorts {
	my $self = shift;
	my $addrmapIOPorts;

	my $inputPorts = $self->getExternalPortList('input');
	if(defined($inputPorts)) {
	    push(@$addrmapIOPorts, @$inputPorts);
	}
	my $outputPorts = $self->getExternalPortList('output');
	if(defined($outputPorts)) {
	    push(@$addrmapIOPorts, @$outputPorts);
	}
	return $addrmapIOPorts;
    }
    sub getDeclaredExternalObjectPortList {
	my($self, $portDir) = @_;

	return if (!$self->isDeclaredExternal());
	return $self->getExternalPortList($portDir);
    }
    sub getObjectPortsList_NoExternalPorts {
	my($self, $portDir) = @_;
	my $noExternalPorts = 1;
	return ($self->getPortList($portDir, $noExternalPorts));
    }
    sub getObjectPortsList {
	my($self, $portDir) = @_;
	return ($self->getPortList($portDir));
    }
    sub getPortList {
	my($self, $direction, $noExternalPorts) = @_;

	my ($portObj, @portList);

	# skip ports when userdefined group within another group, or
	# when userdefined group within a userdefined address map.

#	my $parentNode = $self->getParentNode();
#	return if ($self->isExternal() &&
#		   (defined($parentNode = $self->getParentNode()) && 
#		    ($parentNode->isExternal() || ($parentNode->getNodeType() eq 'group'))));

	my $portListRef = $self->getPorts();
	if (defined($portListRef)) {
	    my $currPortIndex = 0;
	    while ($portObj = $self->getNextPortNode($portListRef,
						     \$currPortIndex)) {
		push(@portList, $portObj) if($direction eq $portObj->getPortDirection());
	    }
	}
	$noExternalPorts = 0 if (!defined($noExternalPorts));
	return \@portList if ($noExternalPorts);
	my $extPortListRef = $self->getExternalBusPorts();
	return \@portList if (!defined($extPortListRef));
	my $currPortIndex = 0;
	while ($portObj = $self->getNextExternalPortNode($extPortListRef,
							 \$currPortIndex)) {
	    push(@portList, $portObj) if ($direction eq $portObj->getPortDirection());
	}
	return \@portList;
    }
    sub getByteAddress {
        my $self = shift;
        return Math::BigInt->new($self->get_byteAddress());
    }
    sub getAddressUnitsInBytes {
        my $self = shift;
        return $self->get_addressUnitsInBytes();
    }
    sub getSlaveAddressUnitsInBytes {
        my $self = shift;
        return $self->get_slaveAddressUnitsInBytes();
    }
    sub getAddressWidth {
        my $self = shift;
        return $self->get_addressWidth();
    }
    sub getAddressmapRelativeByteAddress {
        my $self = shift;
        return Math::BigInt->new($self->get_addressmapRelativeByteAddress());
    }
    sub getAddressmapRelativeByteAddressWidth {
        my $self = shift;
        return $self->get_addressmapRelativeByteAddressWidth();
    }
    sub getActualByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_actualByteSize());
    }
    sub getAddressmapRelativeByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_addressmapRelativeByteSize());
    }
    sub getTopAddressmapRelativeByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_topAddressmapRelativeByteSize());
    }
    sub getArrayByteStride {
        my $self = shift;
        return Math::BigInt->new($self->get_arrayByteStride());
    }
    sub getInstanceAlignByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_instanceAlignByteSize());
    }
    sub getReservedByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_reservedByteSize());
    }
    sub getArrayElementByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_arrayElementByteSize());
    }
    sub getAlignByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_alignByteSize());
    }
    sub getTotalByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_totalByteSize());
    }
    sub getByteEndian {
        my $self = shift;
        return $self->get_byteEndian();
    }
    sub getByteOffset {
        my $self = shift;
        return Math::BigInt->new($self->get_byteOffset());
    }
    sub getWordByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_wordByteSize());
    }
    sub getWordSizeBits {
        my $self = shift;
        return Math::BigInt->new(($self->get_wordByteSize()) * 8);
    }
    sub getResetTiming {
        my $self = shift;
        return $self->get_resetTiming();
    }
    sub getBusProtocol {
        my $self = shift;
        return $self->get_busProtocol();
    }
    sub getInputPipelineStage {
        my $self = shift;
        return $self->get_inputPipelineStage();
    }
    sub getOutputPipelineStage {
        my $self = shift;
        return $self->get_outputPipelineStage();
    }
    sub getIOPipelineStage {
        my $self = shift;
        my $inputPipe = $self->getInputPipelineStage();
        my $outputPipe = $self->getOutputPipelineStage();
        return ($inputPipe, $outputPipe);
    }
    sub getOffsetByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_offsetByteSize());
    }
    sub hasArrayDecoder {
	my $self = shift;
	my $hasArrayDecoder = 'false';
	if (defined($hasArrayDecoder = $self->get_hasArrayDecoder()) &&
	    ($hasArrayDecoder eq 'true')) {
	    return 1;
	} else {
	    return 0;
	}
    }

    sub hasInputSynchronizer {
	my $self = shift;
	return 0;
    }
    sub getDefinitionName {
	my $self = shift;
	my $defName;
	return $defName	if (defined($defName = $self->get_definitionName()));
	return;
    }
    sub getPorts {
        my $self = shift;
        return $self->get_ports();
    }
    sub getExternalBusPorts {
        my $self = shift;
	my $extPortList = $self->get_externalBusPorts();
        return $self->get_externalBusPorts();
    }
    sub hasPortType {
        my ($self, $portType) = @_;

        my ($portListRef, $port, $subRegIndex);
        my ($parentNode, @subRegList);

	if ($portType =~ /external/) {
	    $portListRef = $self->getExternalBusPorts();
	} else {
	    $portListRef = $self->getPorts();
	}
        return 0 if(!defined($portListRef));
        foreach my $portObj (@{$portListRef}) {
            if($portObj->getPortType() eq $portType) {
                return 1;
            }
        }
        return 0;
    }
    sub getPortType {
        my ($self, $portType) = @_;

        my ($portListRef);

	if (!defined($portType)) {
	    $self->_printError('Port type not defined');
	    exit(1);
	}
	if ($portType =~ /external/) {
	    $portListRef = $self->getExternalBusPorts();
	} else {
	    $portListRef = $self->getPorts();
	}
        return if (!defined($portListRef));
        foreach my $portObj (@{$portListRef}) {
	    return $portObj if($portObj->getPortType() eq $portType);
        }
        return;
    }
    sub getParentNode {
        my $self = shift;

        # get the parent address map or group object for the calling object.

        my $count = 0;
	my ($verilogFormat, $relAddr, $arrayIndex);
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage);
        my (@prefixList, $parentHashRef, $parentOfParentHashRef);
        my ($topLevelHashRef, $addrmapObj, $parentName, $bogusAddr);

        my $bigIntAddr = Math::BigInt->new('0');
        my $hierId = $self->getHierarchicalIdentifier();
        my @levelNameList = split('\.', $self->get_hierarchicalNameDotted());
        my @hierLevelNameList = split('\.', $self->get_hierarchicalNameDotted());

        return if(@levelNameList <= 1);

        ($topLevelHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);

        pop(@levelNameList);
        pop(@hierLevelNameList);
        # start from the last level name.
        while(@levelNameList) {
            $parentName = join('.', @levelNameList);
            ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
            if(($parentHashRef->{'nodeType'} eq 'addressmap') ||
               ($parentHashRef->{'nodeType'} eq 'addressmapInstance') ||
               ($parentHashRef->{'nodeType'} eq 'memory') || # virtual groups have memory parent
               ($parentHashRef->{'nodeType'} eq 'wideMemory') ||
               ($parentHashRef->{'nodeType'} eq 'group')) {
                ($busProtocol, $inputPipelineStage, $outputPipelineStage) =
                    $designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);
                @prefixList = @levelNameList;
                shift @prefixList;
                pop(@levelNameList);
                # parent of parent hash ref
                if(@levelNameList > 0) {
                    # get to the parent of parent node
                    if(@levelNameList == 1) {
                        $parentName = $levelNameList[0];
                    } else {
                        $parentName = join('.', @levelNameList);
                    }
                    ($parentOfParentHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($parentName);
                }
		($verilogFormat, $relAddr) =
		    $self->getVerilogBaseAndValue($parentHashRef->{'addressmapRelativeByteAddress'});
		my $relAddrBigInt = Math::BigInt->new($relAddr);
                $arrayIndex = $self->getParentArrayIndexFromHierId($hierId);
                if (($viewName eq 'sv') && (@$arrayIndex)) {
                    my $tmpIndex;
                    for (my $count=0; $count < @$arrayIndex; $count++) {
                        $tmpIndex .= '[' . $arrayIndex->[$count] . ']';
                    }
                    $hierLevelNameList[@hierLevelNameList-1] .= $tmpIndex;
                    $prefixList[@prefixList-1] .= $tmpIndex;
                }
		if ($parentHashRef->{'nodeType'} eq 'group') {
		    $addrmapObj = csrPerlAPIGroup->new($parentHashRef,
						       $arrayIndex,
						       $parentOfParentHashRef,
						       $bigIntAddr,
						       $relAddrBigInt,
						       $busProtocol,
						       $inputPipelineStage,
						       $outputPipelineStage,
						       \@prefixList,
						       \@hierLevelNameList);
		} else { # the parent must be either an addressmap or instance 
		    $addrmapObj = csrPerlAPIModule->new($parentHashRef,
							$arrayIndex,
							$parentOfParentHashRef,
							$bigIntAddr,
							$relAddrBigInt,
							$busProtocol,
							$inputPipelineStage,
							$outputPipelineStage,
							\@prefixList,
							\@hierLevelNameList);
		}
                return $addrmapObj;
            }
            if (defined($parentHashRef->{'arrayDimensionInfo'})) {
                my $numDims = scalar(@{$parentHashRef->{'arrayDimensionInfo'}});
                for (my $count=0; $count < $numDims; $count++) {
                    pop(@$arrayIndex);
                }
            }
            pop(@levelNameList);
            pop(@hierLevelNameList);
        }
        return;
    }
    sub isPublicAndNotEmpty {
        my $self = shift;
	my $isPANE;
	if (defined($isPANE = $self->get_isPublicAndNotEmpty())) {
	    return $isPANE;
	}
	return 0;
    }        
}

package csrPerlAPIRegisterBase;

use strict;
use vars '$AUTOLOAD';

@csrPerlAPIRegisterBase::ISA = qw(csrPerlAPIBase);

{
    my %registerAttrs = (_address => '',
			 _byteAddress => '',
			 _addressUnitsInBytes => '',
			 _offset => '',
			 _byteOffset => '',
			 _offsetUnitsInBytes => '',
			 _addressWidth => '',
			 _msb => '',
			 _lsb => '',
			 _width => '',
			 _bitNumbering => '',
			 _byteEndian => '',
                         _wordByteSize => '',
                         _alignByteSize => '',
			 _offsetByteSize => '',
			 _actualByteSize => '',
			 _addressmapRelativeByteSize => '',
			 _topAddressmapRelativeByteSize => '',
			 _arrayElementByteSize => '',
			 _arrayByteStride => '',
			 _instanceAlignByteSize => '',
			 _totalByteSize => '',
			 _reservedByteSize => '',
			 _reservedValue => '',
			 _busProtocol => '',
			 _inputPipelineStage => '',
			 _outputPipelineStage => '',
			 _addressmapRelativeByteAddress => '',
			 _addressmapRelativeByteAddressWidth => '',
			 _sharedSet => '',
			 _sharedOf => '',
			 _readMask => '',
			 _writeMask => '',
			 _readEffect => '',
			 _writeFunction => '',
			 _aliasOf => '',
			 _aliases => '',
			 _parentName => '',
			 _ports => '',
			 _sharedPorts => '',
			 _parityProtect => '',
                         _privilegedReadAccess => '',
                         _privilegedWriteAccess => '',
                         _secureReadAccess => '',
                         _secureWriteAccess => '',
			 _externalBusPorts => '',
                         _inputPortNamePrefix => '',
                         _outputPortNamePrefix => '',
                         _inputPortNameSuffix => '',
                         _outputPortNameSuffix => '',
			 _parentHashRef => '',
			 _prefixNameList => '',
			 _childHierInt => '',
			 _totalNumPorts  => 0,
                         _publicAndNotEmpty => 1,
			 _visitedAddrmapsList => '',
			 );

    sub _defaultAttrKeys {
	my $self = shift;
	return ($self->SUPER::_defaultAttrKeys(), map { s/^_//; $_ } keys %registerAttrs);
    }

    sub new {
        my($caller,
           $regHashRef,
	   $arrayIndex,
           $parentHashRef,
           $bigIntCsrAddr,
           $bigIntByteOffset,
	   $addrmapRelByteAddr,
           $busProtocol,
           $inputPipelineStage,
           $outputPipelineStage,
           $prefixListRef,
	   $hierPrefixListRef,
	   $aliasPrefixListRef,
	   $expandedAliasName) = @_;
        
	my ($tmpHierName);
        my ($tmpPrefixListRef, $parentNode);
        my ($portObj, @updatePortList, $expandedRegName);
        my $bogusListRef = [];

        my $self = $_[0]->SUPER::new($regHashRef);
        $self->set_nodeHashRef($regHashRef);
        $self->set_parentHashRef($parentHashRef);
	my $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
	$self->set_visitedAddrmapsList(\@visitedAddrmapsList);
        $self->set_designPkgName($designPkgName);
	$self->set_totalNumPorts(0);

        # When copying an object, and there are no arguments
        # passed to the constructor, 'name', 'hierarchicalName', etc,
        # will be copied from the source object, and arguments such as
        # $prefixListRef will not be needed. Also, $expandedRegName will
        # be the same as the 'name'.

        if(defined($prefixListRef)) {
            $expandedRegName = join('_', @{$prefixListRef});
            if ($viewName ne 'sv') {
                $expandedRegName =~ s/\[/_/g;
                $expandedRegName =~ s/\]//g;
            } else {
                my @arrayIndex;
                while ($expandedRegName =~ /(\[\d+\])/g) {
                    push (@arrayIndex, $1);
                }
                $expandedRegName =~ s/\[\d+\]//g;
                $expandedRegName .= join('', @arrayIndex);
            }
            $self->set_name($expandedRegName);
        }
	if (defined($hierPrefixListRef)) {
            my $hierId = join('.', @$hierPrefixListRef);
	    $tmpHierName = join('_', @$hierPrefixListRef);
	    $tmpHierName =~ s/\./_/g;
            if ($viewName ne 'sv') {
                $tmpHierName =~ s/\[/_/g;
                $tmpHierName =~ s/\]//g;
            } else {
                my @arrayIndex;
                while ($tmpHierName =~ /(\[\d+\])/g) {
                    push (@arrayIndex, $1);
                }
                $tmpHierName =~ s/\[\d+\]//g;
                $tmpHierName .= join('', @arrayIndex);
                $hierId =~ s/\[\d+\]//g;
                $hierId .= join('', @arrayIndex);
            }
            $self->set_hierarchicalName($tmpHierName);
            $self->set_hierarchicalNameDotted($hierId);
	} else { # when generating error address
	    $tmpHierName = $self->get_hierarchicalIdentifier();
	    $tmpHierName =~ s/\./_/g;
            if ($viewName ne 'sv') {
                $tmpHierName =~ s/\[/_/g;
                $tmpHierName =~ s/\]//g;
            }
            $self->set_hierarchicalName($tmpHierName);
            $self->set_hierarchicalNameDotted($self->get_hierarchicalIdentifier());
	}
	if (defined($regHashRef->{'aliasOf'})) {
            my $aliasHierId = join('.', @$aliasPrefixListRef);
            if ($viewName eq 'sv') {
                my @arrayIndex;
                while ($expandedAliasName =~ /(\[\d+\])/g) {
                    push (@arrayIndex, $1);
                }
                $expandedAliasName =~ s/\[\d+\]//g;
                $expandedAliasName .= join('', @arrayIndex);
                $aliasHierId =~ s/\[\d+\]//g;
                $aliasHierId .= join('', @arrayIndex);
            }
            $self->set_aliasName(join('_', $expandedAliasName));
            $self->set_aliasHierarchicalIdentifier($aliasHierId);
	}
	if(defined($arrayIndex)) {
	    # copy the array indices before updateRegisterAccounting;
	    $self->set_arrayIndex([@{$arrayIndex}]);
	}
        $self->assignAddressInfo($bigIntCsrAddr,
                                 $bigIntByteOffset,
                                 $regHashRef->{'offset'},
				 $addrmapRelByteAddr);
	$self->setBusProtocol($regHashRef,
			      $parentHashRef,
			      $busProtocol);
	if (defined($parentHashRef)) {
	    if (($parentHashRef->{'nodeType'} eq 'group') ||
                ($parentHashRef->{'nodeType'} eq 'wideRegister')) {
		$self->set_numSubRegisters(@{$parentHashRef->{'subRegisters'}});
		if ($parentHashRef->{'nodeType'} eq 'group') {
		    my $parentByteAddr = $currAddrmapInfo->{'parentAddr'};
		    $self->set_groupRelativeByteAddress($self->getByteAddress() - $parentByteAddr);
		} elsif ($parentHashRef->{'nodeType'} eq 'wideRegister') {
		    if ($self->isGrandParentGroupType()) {
			my $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-2];
			my $parentByteAddr = $currAddrmapInfo->{'parentAddr'};
			$self->set_groupRelativeByteAddress($self->getByteAddress() - $parentByteAddr);
		    }
		}
	    }
	}
	if (defined($parentHashRef) && ($parentHashRef->{'nodeType'} eq 'wideRegister')) {
	    my $numSubRegs = @{$parentHashRef->{'subRegisters'}};
	    $self->set_numSubRegisters($numSubRegs);
	}
        if(defined($inputPipelineStage)) {
            $self->set_inputPipelineStage($inputPipelineStage);
        } elsif(defined($parentHashRef)) {
            $self->set_inputPipelineStage($parentHashRef->{'inputPipelineStage'});
        }
        if(defined($outputPipelineStage)) {
            $self->set_outputPipelineStage($outputPipelineStage);
        } elsif(defined($parentHashRef)) {
            $self->set_outputPipelineStage($parentHashRef->{'outputPipelineStage'});
        }
        if(defined($regHashRef->{'resetValueInfo'})) {
            my $resetValueObj = csrResetValue->new($regHashRef->{'resetValueInfo'});
            $self->set_resetValueNode($resetValueObj);
        }

	$expandedRegName = $self->getName() if(!defined($expandedRegName));
	my $declaredExternal = 'false';
	my $hasArrayDecoder = 'false';

	# Userdefined register arrays that have array decoder have only one set of ports,
	# but we'll copy those ports to all register array elements because we'll
	# have to schedule read_data on the external_read_data_port for every one of those
	# registers.

	if (($viewName ne 'sv') &&
            (defined($regHashRef->{'externalBusPorts'}) ||
             defined($regHashRef->{'ports'}))) {
            $expandedRegName = $self->getRegExpandedName($regHashRef,
                                                         $expandedRegName);
        }
        if (defined($regHashRef->{'externalBusPorts'})) {
            my $portListRef = csrPorts->new($self,
                                            $regHashRef,
                                            $regHashRef->{'externalBusPorts'},
                                            $parentHashRef,
                                            $expandedRegName);
            $self->set_externalBusPorts($portListRef);
        }
        # User-defined wide registers non-atomic
        if ($self->isSubRegisterNode() && $self->isDeclaredExternal() &&
            !$self->isAtomic()) {
	    my $portListRef = [];
	    my $parentNode = $self->getParentNode();
	    my $portList = $parentNode->getExternalBusPorts();
	    foreach my $nextP (@$portList) {
		if ($nextP->getPortType() eq 'externalWriteAccessPorts') {
		    my $multiPort = $nextP->get_multiPortList();
		    my $subRegIndex = $self->getSubRegisterIndex();
		    my $extWriteAccessPort = $multiPort->[$subRegIndex];
		    $extWriteAccessPort->set_portType('externalWriteAccessPort');
		    push (@$portListRef, $extWriteAccessPort);
		} else {
		    push (@$portListRef, $nextP);
		}
	    }
	    $self->set_externalBusPorts($portListRef);
	} elsif ($self->isSubMemoryNode()) {
	    my $portListRef = [];
            my $parentNode = $self->getParentNode();
            my $portList = $parentNode->getExternalBusPorts();
	    foreach my $nextP (@{$portList}) {
		if ($nextP->getPortType() eq 'externalWriteAccessPorts') {
		    my $multiPort = $nextP->get_multiPortList();
                    my $subMemIndex = $currAddrmapInfo->{'currAddrmapSubMemoryIndex'};
                    my $extWriteAccessPort = $multiPort->[$subMemIndex];
                    $extWriteAccessPort->set_portType('externalWriteAccessPort');
                    push (@$portListRef, $extWriteAccessPort);
		} else {
		    push (@$portListRef, $nextP);
		}
	    }
	    $self->set_externalBusPorts($portListRef);
	}
        if (defined($regHashRef->{'ports'}) && (@{$regHashRef->{'ports'}} > 0)) {
	    my $portListRef = csrPorts->new($self,
                                            $regHashRef,
                                            $regHashRef->{'ports'},
                                            $parentHashRef,
                                            $expandedRegName);
	    $self->set_ports($portListRef);
        }
        return $self;
    }
    sub getRegExpandedName {
	my ($self, $regHashRef, $expandedRegName) = @_;

	my ($declaredExternal, $hasArrayDecoder);
	if (defined($regHashRef->{'arrayDimensionInfo'}) && ($regHashRef->{'external'} eq 'true')) {
	    if (defined($declaredExternal = $regHashRef->{'declaredExternal'}) &&
                ($declaredExternal eq 'true') && 
		(defined($hasArrayDecoder = $regHashRef->{'hasArrayDecoder'}) &&
                 ($hasArrayDecoder eq 'true'))) {
		for (my $index=0; $index < scalar(@{$regHashRef->{'arrayDimensionInfo'}}); $index++) {
		    $expandedRegName =~ s/_\d+$//;
		}
	    }
	}
	return $expandedRegName;
    }
    sub getBusSlavePorts {
	my $self = shift;
	return $self->getExternalBusPorts();
    }
    sub getExternalBusPorts {
        my $self = shift;
        return $self->get_externalBusPorts();
    }
    sub getByteAddress {
        my $self = shift;
        return Math::BigInt->new($self->get_byteAddress());
    }
    sub getAddressWidth {
        my $self = shift;
        return $self->get_addressWidth();
    }
    sub getAddressUnitsInBytes {
	my $self = shift;
	my $parentAddrmap = $self->getParentAddressmapNode();
	return $parentAddrmap->getAddressUnitsInBytes();
    }
    sub getAddressmapRelativeByteAddress {
        my $self = shift;
        return Math::BigInt->new($self->get_addressmapRelativeByteAddress());
    }
    sub getAddressmapRelativeByteAddressWidth {
        my $self = shift;
        return $self->get_addressmapRelativeByteAddressWidth();
    }
    sub getGroupRelativeByteAddress {
        my $self = shift;
        return Math::BigInt->new($self->get_groupRelativeByteAddress());
    }
    sub getBusProtocol {
        my $self = shift;
        return $self->get_busProtocol();
    }
    sub getInputPipelineStage {
        my $self = shift;
        return $self->get_inputPipelineStage();
    }
    sub getOutputPipelineStage {
        my $self = shift;
        return $self->get_outputPipelineStage();
    }
    sub getIOPipelineStage {
        my $self = shift;
        my $inputPipe = $self->getInputPipelineStage();
        my $outputPipe = $self->getOutputPipelineStage();
        return ($inputPipe, $outputPipe);
    }
    sub getWidth {
        my $self = shift;
        return $self->get_width();
    }
    sub getMsb {
        my $self = shift;
        return $self->get_msb();
    }
    sub getLsb {
        my $self = shift;
        return $self->get_lsb();
    }
    sub getMsbLsb {
        my $self = shift;
        return ($self->get_msb(), $self->get_lsb());
    }
    sub getByteEndian {
        my $self = shift;
        return $self->get_byteEndian();
    }
    sub getByteOffset {
        my $self = shift;
        return Math::BigInt->new($self->get_byteOffset());
    }
    sub getOffsetUnitsInBytes {
	my $self = shift;
	return $self->get_offsetUnitsInBytes();
    }
    sub getWordByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_wordByteSize());
    }
    sub getActualByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_actualByteSize());
    }
    sub getAddressmapRelativeByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_addressmapRelativeByteSize());
    }
    sub getTopAddressmapRelativeByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_topAddressmapRelativeByteSize());
    }
    sub getArrayByteStride {
        my $self = shift;
        return Math::BigInt->new($self->get_arrayByteStride());
    }
    sub getInstanceAlignByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_instanceAlignByteSize());
    }
    sub getReservedByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_reservedByteSize());
    }
    sub getArrayElementByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_arrayElementByteSize());
    }
    sub getTotalByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_totalByteSize());
    }
    sub getAlignByteSize {
        my $self = shift;
        return Math::BigInt->new($self->get_alignByteSize());
    }    
    sub getOffsetByteSize {
	my $self = shift;
	return Math::BigInt->new($self->get_offsetByteSize());
    }
    sub getAliasOf {
        my $self = shift;
        return $self->get_aliasOf();
    }
    sub hasSharedShadow {
        my $self = shift;
        my $sharedShadow;
        return 1 if (defined($sharedShadow = $self->get_sharedShadow()) &&
                     ($sharedShadow eq 'true'));
        return 0;
    }
    sub hasPrivilegedReadAccess {
        my $self = shift;
        return 1 if ($self->get_privilegedReadAccess() == 1);
        return 0;
    }
    sub hasPrivilegedWriteAccess {
        my $self = shift;
        return 1 if ($self->get_privilegedWriteAccess() == 1);
        return 0;
    }
    sub hasSecureReadAccess {
        my $self = shift;
        return 1 if ($self->get_secureReadAccess() == 1);
        return 0;
    }
    sub hasSecureWriteAccess {
        my $self = shift;
        return 1 if ($self->get_secureWriteAccess() == 1);
        return 0;
    }
    sub hasInputSynchronizer {
	my $self = shift;
	return 0;
    }
    sub getPorts {
        my $self = shift;
        return($self->get_ports());
    }
    sub hasSharedPort {
	my ($self, $portType) = @_;

	foreach my $fieldObj (@{$self->getBitfields()}) {
	    return 1 if ($fieldObj->hasSharedPort($portType));
	}
	return 0;
    }
    sub hasExternalField {
	my $self = shift;
	my $bitfieldListRef;

	return 0 if(!defined($bitfieldListRef = $self->getBitfields()));
	foreach my $fieldObj (@{$bitfieldListRef}) {
	    return 1 if ($fieldObj->isExternal());
	}
	return 0;
    }
    sub hasDeclaredExternalField {
	my $self = shift;
	my $bitfieldListRef;

	return 0 if(!defined($bitfieldListRef = $self->getBitfields()));
	foreach my $fieldObj (@{$bitfieldListRef}) {
	    return 1 if ($fieldObj->isDeclaredExternal());
	}
	return 0;
    }
    sub getReadMask {
        my $self = shift;
        return $self->get_readMask();
    }
    sub getWriteMask {
        my $self = shift;
        return $self->get_writeMask();
    }
    sub getParentName {
	my $self = shift;
	return $self->get_parentName();
    }
    sub get_resetCompareString {
        my$self = shift;
        my $resetObj = $self->get_resetValueNode();
        if(defined($resetObj)) {
            return $resetObj->get_resetCompareString();
        } else {
            return;
        }
    }

    sub get_readMaskValue {
        my$self = shift;
        my ($verilogFormat, $readMask);
        if(defined($readMask = $self->get_readMask())) {
            ($verilogFormat, $readMask) = $self->getVerilogBaseAndValue($readMask);
            return Math::BigInt->new($readMask);
        }
        return;
    }

    sub get_writeMaskValue {
        my$self = shift;
        my ($verilogFormat, $writeMask);
        if(defined($writeMask = $self->get_writeMask())) {
            ($verilogFormat, $writeMask) = $self->getVerilogBaseAndValue($writeMask);
            return Math::BigInt->new($writeMask);
        }
        return;
    }

    sub getReadMaskValue {
        my$self = shift;
        return $self->get_readMaskValue();
    }

    sub getWriteMaskValue {
        my$self = shift;
        return $self->get_writeMaskValue();
    }

    sub regHasPortType {
	my ($self, $portType) = @_;
	my $bitfields = $self->getBitfields();
	return 0 if (!defined($bitfields));
	foreach my $fieldObj (@$bitfields) {
	    return 1 if ($fieldObj->hasPortType($portType));
	}
	return 0;
    }
    sub hasPortType {
        my ($self, $portType) = @_;

        my ($portListRef);
	if ($portType =~ /external/) {
	    $portListRef = $self->getExternalBusPorts();
	} else {
	    $portListRef = $self->getPorts();
	}
        return 0 if(!defined($portListRef));
	if ($portType eq 'externalWriteAccessPorts') {
	    foreach my $nextP (@$portListRef) {
		return 1 if (defined($nextP->get_multiPortList()));
	    }
	    return 0;
	}
        foreach my $portObj (@{$portListRef}) {
	    return 1 if($portObj->get_portType() eq $portType);
        }
        return 0;
    }
    sub getPortType {
        my ($self, $portType) = @_;

        my ($portListRef);
	if (!defined($portType)) {
	    $self->_printError('Port type not defined');
	    exit(1);
	}
	if ($portType =~ /external/) {
	    $portListRef = $self->getExternalBusPorts();
	} else {
	    $portListRef = $self->getPorts();
	}
        return if(!defined($portListRef));
	if ($portType eq 'externalWriteAccessPorts') {
	    foreach my $nextP (@$portListRef) {
		if (defined($nextP->get_multiPortList())) {
		    return $nextP->get_multiPortList();
		}
	    }
	}
        foreach my $portObj (@{$portListRef}) {
	    return $portObj if($portObj->getPortType() eq $portType);
        }
        return;
    }

    sub isGrandParentGroupType {
	my $self = shift;

	# Is the parent of the parent node a group object?

	my $hierName = $self->getHierarchicalIdentifier();
	my @levelNameList = split('\.', $hierName);
	if (@levelNameList > 2) {
	    pop(@levelNameList);
	    pop(@levelNameList);
	    my $parentName = join('.', @levelNameList);
	    my ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
	    return 1 if (defined($parentHashRef) && ($parentHashRef->{'nodeType'} eq 'group'));
	    return 0;
	}
    }
    sub isPublicAndNotEmpty {
        my $self = shift;
	my $isPANE;
	if (defined($isPANE = $self->get_isPublicAndNotEmpty())) {
	    return $isPANE;
	}
	return 0;
    }        
}

package csrPerlAPIRegister;

use strict;
@csrPerlAPIRegister::ISA = qw(csrPerlAPIRegisterBase);

{
    my %registerAttrs = (_bitfields => '',
                         _registerSize => '',
                         _flop => '',
                         _resetValueNode => '',
                         _parentIsArray => '',
                         _resetTiming => '',
                         _atomicAccess => '',
                         _synchronizerStyle => '',
                         _virtualRegister => '',
                         _subRegisters => '',
                         _subRegisterFlag => '',
                         _subRegisterIndex => '',
                         _nextSubRegisterIndex => '',
                         _nextFieldIndex => -1,
                         _nextExternalBusPortIndex => '-1',
                         _hasArrayDecoder => '',
                         _sharedShadow => '',
                         _aliasPrefixListRef => '',
                         _hasOverlappingFields => '',
                         _registerFitDatapath => '',
                         _broadcastList => '',
                         _broadcastMembers => '',
                         _unionSelectList => '',
                         );

    sub _defaultAttrKeys {
	my $self = shift;
	return ($self->SUPER::_defaultAttrKeys(), map { s/^_//; $_ } keys %registerAttrs);
    }

    sub new {
        my($caller,
           $regHashRef,
	   $arrayIndex,
           $parentHashRef,
           $bigIntCsrAddr,
           $bigIntOffset,
	   $addrmapRelByteAddr,
           $busProtocol,
           $inputPipelineStage,
           $outputPipelineStage,
           $prefixListRef,
           $aliasPrefixListRef,
	   $hierPrefixListRef) = @_;

	my ($expandedAliasName);
        my ($tmpPrefixListRef, $external);
        my ($portObj, @updatePortList, $expandedRegName, $subRegFlag);
        my $bogusListRef = [];

        if(defined($regHashRef->{'aliasOf'})) {
            $expandedAliasName = join('_', @$aliasPrefixListRef);
            if ($viewName ne 'sv') {
                $expandedAliasName =~ s/\[/_/g;
                $expandedAliasName =~ s/\]//g;
                $expandedAliasName =~ s/\./_/g;
            }
            $tmpPrefixListRef = $aliasPrefixListRef;
        } else {
            $tmpPrefixListRef = $prefixListRef;
        }
        my $self = $_[0]->SUPER::new($regHashRef,
				     $arrayIndex,
                                     $parentHashRef,
                                     $bigIntCsrAddr,
                                     $bigIntOffset,
				     $addrmapRelByteAddr,
                                     $busProtocol,
                                     $inputPipelineStage,
                                     $outputPipelineStage,
                                     $prefixListRef,
				     $hierPrefixListRef,
				     $aliasPrefixListRef,
				     $expandedAliasName);
	$self->set_nextExternalBusPortIndex(-1);
	if ($designDBRef->_isExternalHashInHierarchy($regHashRef)) {
	    $self->set_external('true') if ($regHashRef->{'hierarchicalIdentifier'} !~
                                            /SMFR_bogusRegister/);
        }
	$self->set_nextSubRegisterIndex(0) if($self->getNodeType() eq 'wideRegister');

        $self->set_nextFieldIndex(-1);
        $self->addBitfields($regHashRef, $tmpPrefixListRef, $hierPrefixListRef);
	if (defined($subRegFlag = $regHashRef->{'subRegisterFlag'}) &&
	    ($subRegFlag eq 'true')) {
	    $self->updateWideRegPortRefNames();
	}
	$self->markOverlappingFields()
            if ($self->hasBitfields() &&
                ($regHashRef->{'hierarchicalIdentifier'} !~ /SMFR_bogusRegister/));
        return $self;
    }

    sub markOverlappingFields {
	my $self = shift;

	my $overlapFlag = 0;
	my $fieldList = $self->getBitfields();
	for (my $index=0; $index < @$fieldList; $index++) {
	    my $fieldObj = $fieldList->[$index];
	    my $fieldHasROAccess = $fieldObj->hasROAccess();
	    my $fieldHasWOAccess = $fieldObj->hasWOAccess();
	    my ($msb, $lsb) = $fieldObj->getMsbLsb();
	    for (my $tmpIndex = $index+1; $tmpIndex < @$fieldList; $tmpIndex++) {
		my $tmpFieldObj = $fieldList->[$tmpIndex];
		my ($tmpMsb, $tmpLsb) = $tmpFieldObj->getMsbLsb();
		my $tmpFieldHasROAccess = $tmpFieldObj->hasROAccess();
		my $tmpFieldHasWOAccess = $tmpFieldObj->hasWOAccess();
		if ((($fieldHasROAccess && $tmpFieldHasWOAccess) ||
		     ($fieldHasWOAccess && $tmpFieldHasROAccess)) &&
		    ((($msb <= $tmpMsb) && ($lsb >= $tmpLsb)) ||
		     (($msb >= $tmpMsb) && ($lsb <= $tmpLsb)))) {
		    $fieldObj->set_overlapFlag(1);
		    $tmpFieldObj->set_overlapFlag(1);
		    $overlapFlag = 1;
		} 
	    }
	}
	return $overlapFlag;
    }
    sub fieldsHavePortType {
        my ($self, $portType) = @_;
        my ($bitfieldListRef, $port);
        return if(!defined($bitfieldListRef = $self->getBitfields()));
        foreach my $fieldObj (@{$bitfieldListRef}) {
            if(defined($port = $fieldObj->getPortType($portType))) {
                return 1;
            }
        }
        return 0;
    }
    sub hasInputSynchronizer {
	my $self = shift;
	return 0;
    }
    sub hasFlop {
	my $self = shift;
	my $flop;
	return 1 if (defined($flop = $self->get_flop()) && ($flop == 1));
	return 0;
    }
    sub hasResetClass {
	my $self = shift;
	foreach my $nextField (@{$self->getBitfields()}) {
	    return 1 if ($nextField->hasResetClass());
	}
	return 0;
    }
    sub hasResetClassType {
	my ($self, $resetClassType) = @_;
	foreach my $nextField (@{$self->getBitfields()}) {
	    return 1 if ($nextField->hasResetClassType($resetClassType));
	}
	return 0;
    }
    sub getResetTiming {
        my $self = shift;
        return $self->get_resetTiming();
    }
    sub hasAliases {
	my $self = shift;
	return 1 if (defined($self->get_aliases()));
	return 0;
    }
    sub getAliases {
	my $self = shift;
	return $self->get_aliases();
    }
    sub isAlias {
	my $self = shift;
	return 1 if (defined($self->get_aliasOf()));
	return 0;
    }
    sub getAliasName {
        my $self = shift;
        return $self->get_aliasName();
    }
    sub getAliasHierarchicalIdentifier {
	my $self = shift;
	return $self->get_aliasHierarchicalIdentifier();
    }
    sub hasAliasedRegister {
	my $self = shift;
	# does this register have aliased registers?
	my $addrmapNode = $self->getParentAddressmapNode();
	my $regId = $self->getHierarchicalIdentifier();
	my @regIdLevels = split('\.', $regId);
	# remove the address map name from the hierarchical name
	shift(@regIdLevels);
	$regId = join('.', @regIdLevels);
        if ($viewName ne 'sv') {
            $regId =~ s/\[/_/g;
            $regId =~ s/\]//;
        }
	$regId =~ s/\./_/g;
	my $found = 0;
	my $includeWideRegsFlag = $self->getIncludeWideRegistersFlag();
	my $parentType = $addrmapNode->getNodeType();
	my $parentAddrmapId = $addrmapNode->getHierarchicalIdentifier();
	my @tmpId = split('\.', $parentAddrmapId);
	shift(@tmpId);
	$parentAddrmapId = join('.', @tmpId);
	my $addrmapHierId = $addrmapNode->getHierarchicalIdentifier();
	$self->clearIncludeWideRegistersFlag();
	$self->saveAddrmapInfoContext();
	while (my $nextRegObj = $addrmapNode->getNextRegisterNode()) {
	    if ($nextRegObj->isAlias()) {
		my $aliasOfName = $nextRegObj->getAliasName();
		if ($parentType eq 'addressmapInstance') {
		    $aliasOfName = $parentAddrmapId . '_' . $aliasOfName;
		}
		if ($nextRegObj->getParentAddressmapNode()->getHierarchicalIdentifier() eq
		    $addrmapHierId) {
		    if ($regId eq $aliasOfName) {
			$found = 1;
			last;
		    }
		}
	    }
	}
	$self->restoreAddrmapInfoContext();
        $self->setIncludeWideRegistersFlag($includeWideRegsFlag);
	return ($found);
    }
    sub getAliasedRegisters {
	my $self = shift;

	# find the registers that are aliased to this register
	my @aliasedRegList;
	$self->saveAddrmapInfoContext();
	my $addrmapNode = $self->getParentAddressmapNode();
	my $regId = $self->getHierarchicalIdentifier();
	my @regIdLevels = split('\.', $regId);
	shift(@regIdLevels);
	$regId = join('.', @regIdLevels);
	$regId =~ s/\[/_/g;
	$regId =~ s/\]//;
	$regId =~ s/\./_/g;
	while (my $nextRegObj = $addrmapNode->getNextRegisterNode()) {
	    if ($nextRegObj->isAlias()) {
		my $aliasOfName = $nextRegObj->getAliasName();
		if ($regId eq $aliasOfName) {
		    push (@aliasedRegList, $nextRegObj);
		}
	    }
	}
	$self->restoreAddrmapInfoContext();
	return (\@aliasedRegList);
    }
    sub getAliasRegister {
	my $regObj = shift;

	# find the register to which this register (regObj) is aliased.
	my @aliasedRegList;
	my $found = 0;
	my $addrmapNode = $regObj->getParentAddressmapNode();
	my $aliasOfId = $regObj->getAliasHierarchicalIdentifier();
	$aliasOfId = join('.', $addrmapNode->getHierarchicalIdentifier(), $aliasOfId);
	$addrmapNode->saveAddrmapInfoContext();
	while (my $nextRegObj = $addrmapNode->getNextRegisterNode()) {
	    if ($nextRegObj->hasAliases()) {
		my $regId = $nextRegObj->getHierarchicalIdentifier();
		if ($regId eq $aliasOfId) {
		    $addrmapNode->restoreAddrmapInfoContext();
		    return $nextRegObj;
		}
	    }
	}
	$addrmapNode->restoreAddrmapInfoContext();
	return;
    }
    sub hasUnionSelectList {
	my $self = shift;
	return 1 if (defined($self->get_unionSelectList()));
	return 0;
    }
    sub hasSharedSet {
	my $self = shift;
	return 1 if (defined($self->get_sharedSet()));
	return 0;
    }
    sub getSharedSet {
	my $self = shift;
	return $self->get_sharedSet();
    }
    sub isBroadcastMember {
	my $self = shift;
	return 0 if !defined(my $broadcastList = $self->get_broadcastList());
	return 1;
    }
    sub isBroadcastRegister {
	my $self = shift;
	return 1 if (defined($self->get_broadcastMembers()));
	return 0;
    }
    sub getBroadcastMemberRegisters {
	my $self = shift;

	my ($nextRegObj, $bogusObj, @regList, $tmpItem);

	my $broadcastMemberList = $self->get_broadcastMembers();
	foreach my $broadcastItem (@{$broadcastMemberList}) {
	    my @levels;
	    @levels = split('\.', $broadcastItem);
	    $self->set_topLevelAddressmapName($levels[0]);
	    $tmpItem = join('.', @levels);
	    ($nextRegObj, $bogusObj) = $self->getRegisterNodeByName($tmpItem);
	    push (@regList, $nextRegObj);
	}
	return \@regList;
    }
    sub hasOutputSynchronizerField {
	my $self = shift;
	foreach my $nextField (@{$self->getBitfields()}) {
	    return 1 if ($nextField->hasOutputSynchronizer());
	}
	return 0;
    }
    sub getFieldsWithSynchronizerStyle {
	my ($self, $syncType, $syncDirection) = @_;
	my @fieldList;
	foreach my $nextField (@{$self->getBitfields()}) {
	    push (@fieldList, $nextField) if ($nextField->hasSynchronizerStyleWithDir($syncType,
										      $syncDirection));
	}
	return \@fieldList;
    }
    sub hasOverlapFields {
	my $self = shift;
	return 1 if ($self->get_hasOverlappingFields() eq 'true');
	return 0;
    }
    sub hasBitfields {
	my $self = shift;
	return 1 if (defined($self->getBitfields()));
	return 0;
    }
    sub getBitfields {
        my $self = shift;
        return $self->get_bitfields();
    }
    sub getAtomicAccess {
        my $self = shift;
        return $self->get_atomicAccess();
    }
    sub hasArrayDecoder {
	my $self = shift;
	my $hasArrayDecoder = 'false';
	if (defined($hasArrayDecoder = $self->get_hasArrayDecoder()) &&
	    ($hasArrayDecoder eq 'true')) {
	    return 1;
	} else {
	    return 0;
	}
    }
    sub getSubRegisterFlag {
        my $self = shift;
        return $self->get_subRegisterFlag();
    }
    sub getSubRegisterIndex {
        my $self = shift;
        return $self->get_subRegisterIndex();
    }
    sub getNumSubRegisters {
        my $self = shift;

        # must be called either with a wide register object
        # or a sub-register object of a wide register.
        my $parentHashRef = $self->get_parentHashRef();
        my $nodeType = $self->getNodeType();
        return if(($nodeType ne 'wideRegister') && ($parentHashRef->{'nodeType'} ne 'wideRegister'));
        if($nodeType eq 'wideRegister') {
            return scalar(@{($self->get_nodeHashRef())->{'subRegisters'}});
        } else {
            return scalar(@{$parentHashRef->{'subRegisters'}});
        }
    }

    sub updateWideRegPortRefNames {
	my $self = shift;

	# update the port names of sub-register fields that
	# are references to a wide register field (such as
	# an interrupt enable, etc).

	my ($newHierRefName, $newName, @hierNameList, $fieldName);
	my ($portList, $refRegObj, $refFieldObj, $subRegIndex);

	my $bitNumbering = $self->getBitNumbering();
	my $wordSize = $self->getWordByteSize()*8;
	my $numSubRegs = $self->get_numSubRegisters();
	my $bitfieldList = $self->getBitfields();
	foreach my $nextField (@{$bitfieldList}) {
	    $portList = $nextField->getPorts();
	    foreach my $nextPort (@{$portList}) {
		next if ($nextPort->hasConcatFields());
		$self->do_updateWideRegPortRefNames($nextPort,
						    $numSubRegs,
						    $wordSize,
						    $bitNumbering);
	    }
	}
    }

    sub do_updateWideRegPortRefNames {
	my ($self, $nextPort, $numSubRegs) = @_;

	if ($nextPort->isSourceTypeFieldReference()) {
	    my ($refRegObj, $refFieldObj) = $nextPort->getReferenceInfo();
	    if ($refRegObj->getNodeType() eq 'wideRegister') {
		# reference to a sub-register field
		my $subRegIndex = $self->getSubRegisterIndex();
		my $newHierRefName = $nextPort->getHierarchicalRefName();
		my @hierNameList = split('\.', $newHierRefName);
		my $fieldName = pop(@hierNameList);
		$newHierRefName = $refRegObj->getName() . '.' .
		    join('.', @hierNameList) . '_' .
		    $subRegIndex . '_' . $numSubRegs . '.' . $fieldName;

		my $newName = $nextPort->getName();
		$newName =~ s/$fieldName//;
		$newName .= $subRegIndex . '_' . $fieldName;
		$nextPort->set_hierarchicalRefName($newHierRefName);
		$nextPort->set_name($newName);
	    }
	}
    }

    sub getSubRegistersList {
        my $self = shift;

        my ($verilogFormat, $offset, $relAddr, @hierNameList);
        my ($topLevelHashRef, $parentHashRef, $wideRegHashRef);
        my (@wideRegPrefixList, $parentName);
        my (@subRegObjectList, @levelNameList, @aliasPrefixList);
        my ($wideRegName, $subRegObj, @prefixList);
        my $bigIntAddr = Math::BigInt->new('0');

        $wideRegName = $self->getHierarchicalIdentifier();
        @levelNameList = split('\.', $wideRegName);
        @wideRegPrefixList = split('\.', $wideRegName);

        ($topLevelHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);
        if(@levelNameList > 2) {
            # more than one level of hierarchy
            pop(@levelNameList);
            $parentName = join('.', @levelNameList);
            ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
        } else {
            $parentHashRef = $topLevelHashRef;
        }
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage) = 
            $designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);

        ($wideRegHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($wideRegName);
	my $arrayIndex = undef;
#	if ($self->isDeclaredExternal() && $self->isArray()) {
#	    # Power of 2 alignment required
#	    my $multiplier = $designDBRef->_getMultiplierFromArrayIndex($self->getArrayIndex(),
#									$wideRegHashRef->{'arrayDimensionInfo'});
#	    if (!$self->isPowerOf2Aligned($bigIntAddr)) {
#		$bigIntAddr = $self->powerOf2Align($bigIntAddr);
#	    }
#	}
        foreach my $subRegHashRef (@{$wideRegHashRef->{'subRegisters'}}) {
	    my @hierNameList;
            @prefixList = ();
            @aliasPrefixList = ();
            push(@prefixList, (@wideRegPrefixList, join('_', $subRegHashRef->{'name'})));
            push(@hierNameList, (@wideRegPrefixList, $subRegHashRef->{'name'}));
            if(defined($subRegHashRef->{'aliasOf'})) {
                push(@aliasPrefixList, (@wideRegPrefixList, $subRegHashRef->{'aliasOf'}));
            };
            ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($subRegHashRef->{'byteOffset'});
            $bigIntAddr->badd($offset);
            ($verilogFormat, $relAddr) =
		$self->getVerilogBaseAndValue($subRegHashRef->{'addressmapRelativeByteAddress'});
	    my $relAddrBigInt = Math::BigInt->new($relAddr);
	    $updateRefArrayIndex = 0;
            $subRegObj = csrPerlAPIRegister->new($subRegHashRef,
						 $arrayIndex,
                                                 $wideRegHashRef,
                                                 $bigIntAddr,
                                                 $offset,
                                                 $relAddrBigInt,
                                                 $busProtocol,
                                                 $inputPipelineStage,
                                                 $outputPipelineStage,
                                                 \@prefixList,
                                                 \@aliasPrefixList,
						 \@hierNameList);
            $bigIntAddr->bsub($offset);
            push(@subRegObjectList, $subRegObj);
        }
        return @subRegObjectList;
    }

    sub getResetValueNode {
        my $self = shift;
        return $self->get_resetValueNode();
    }
    sub getNextRegisterNode {
        my $self = shift;

        # Iterator for the sub-registers of a wide register.

        my $found = 0;
        my ($errMsg, $regObj, $addrmapNode, $subRegObj);
        my ($tmpIndex, $currVisitedIndex);

        if($self->getNodeType ne 'wideRegister') {
            $errMsg = 'getNextRegisterNode() is not a method of ' . ref($self) . ' class type.' . "\n";
            $self->_printError($errMsg);
            exit(1);
        }
        if(@visitedSubRegsList == 0) {
            # we first need to initialize the list of sub registers.
            @visitedSubRegsList = $self->getSubRegistersList();
            $self->set_currVisitedSubRegIndex(0);
        }
        if(@visitedSubRegsList == 0) {
            $errMsg = 'Could not find sub registers of ' . $self->getName() . ' register!';
            $self->_printError($errMsg);
        }
        $currVisitedIndex = $self->get_currVisitedSubRegIndex();
        if($currVisitedIndex < @visitedSubRegsList) {
            $tmpIndex = $currVisitedIndex;
            $self->set_currVisitedSubRegIndex($tmpIndex+1);
            return $visitedSubRegsList[$tmpIndex];
        }
        @visitedSubRegsList = ();
        $self->set_currVisitedSubRegIndex(0);
        return;
    }

    sub setHasTriggerValueField {
        my ($self,
            $flag) = @_;

        $self->set_hasTriggerValueField($flag);
    }
    sub hasTriggerValueField {
        my ($self,
            $flag) = @_;

        return $self->get_hasTriggerValueField();
    }
    sub addBitfields {
        my($self,
	   $regHashRef,
	   $prefixListRef,
	   $hierPrefixListRef) = @_;

        my $fieldIndex;
        my ($fieldHashListRef, @hierNameList);
        my ($tmpFieldName, @hierarchicalPrefixList);
        my ($lsb, $msb, $expandedFieldName, $fieldObjList, $fieldObj);

        return if  (!defined($fieldHashListRef = $regHashRef->{'bitfields'}));

        $self->setHasTriggerValueField(0);
        my $bitNumbering = $regHashRef->{'bitNumbering'};
        my $fieldCount = @{$fieldHashListRef} - 1;
        for (my $hashIndex = ($bitNumbering eq 'lsb_0') ? $fieldCount : 0;
	     ($bitNumbering eq 'lsb_0') ? $hashIndex >= 0 : $hashIndex <= $fieldCount;
	     ($bitNumbering eq 'lsb_0') ? $hashIndex-- : $hashIndex++) {
	    my $fieldHash = $fieldHashListRef->[$hashIndex];
            if (defined($fieldHash->{'triggerValue'})) {
                $self->setHasTriggerValueField(1);
            }
	    @hierarchicalPrefixList = ();
	    if (defined($fieldHash->{'arrayDimensionInfo'})) {
                my $arraySize = $designDBRef->_getArraySize($fieldHash->{'arrayDimensionInfo'});
                if ($arraySize > 1) {
                    if ($self->getExpandFieldArraysFlag()) {
                        my $arrayBitStride = 0;
                        if (defined($fieldHash->{'arrayBitStride'})) {
                            $arrayBitStride = $fieldHash->{'arrayBitStride'};
                        }
                        if ($self->getByteEndian() eq 'little') {
                            my @tmpFieldList;
                            my $arrayMinIndex = $designDBRef->_getArrayMinIndices($fieldHash->{'arrayDimensionInfo'});
                            my $arrayMaxIndex = $designDBRef->_getArrayMaxIndices($fieldHash->{'arrayDimensionInfo'});
                            my $arrayElemCnt = 0;
                            $fieldIndex = $arrayMinIndex;
                            for ($arrayElemCnt = 0; $arrayElemCnt < $arraySize; $arrayElemCnt++) {
                                @hierarchicalPrefixList = ();
                                my $tmpFieldName = $designDBRef->_getArrayElementName($fieldHash->{'name'},
                                                                                      $fieldIndex);
                                $expandedFieldName = join('_',
                                                          @$prefixListRef,
                                                          $tmpFieldName);
                                push(@hierarchicalPrefixList, (@$hierPrefixListRef, $tmpFieldName));
                                my $multiplier = $designDBRef->_getMultiplierFromArrayIndex($fieldIndex,
                                                                                            $fieldHash->{'arrayDimensionInfo'});
                                my $stride;
                                if (($arrayBitStride != 0) &&
                                    ($arrayElemCnt != 0)) {
                                    $stride = ($fieldHash->{'width'} * $multiplier) +
                                              ($arrayBitStride - $fieldHash->{'width'}) * $multiplier;
                                } else {
                                    $stride = $fieldHash->{'width'} * $multiplier;
                                }
                                if ($bitNumbering eq 'lsb_0') {
                                    $lsb = $fieldHash->{'lsb'} + $stride;
                                    $msb = $fieldHash->{'msb'} + $stride;
                                } else {
                                    $lsb = $fieldHash->{'lsb'} - $stride;
                                    $msb = $fieldHash->{'msb'} - $stride;
                                }
                                $fieldObj = csrPerlAPIField->new($fieldHash,
                                                                 $expandedFieldName,
                                                                 \@hierarchicalPrefixList,
                                                                 $lsb,
                                                                 $msb,
                                                                 $fieldIndex,
                                                                 $regHashRef);
                                if ($bitNumbering eq 'lsb_0') {
                                    push(@tmpFieldList, $fieldObj);
                                } else {
                                    unshift(@tmpFieldList, $fieldObj);
                                }
                                $designDBRef->_incrementArrayIndex($fieldIndex,
                                                                   $fieldHash->{'arrayDimensionInfo'});
			    }
			    push (@$fieldObjList, @tmpFieldList) if (@tmpFieldList > 0);
			} else { # byte endian big
			    my @tmpFieldList;
                            my $arrayMinIndex = $designDBRef->_getArrayMinIndices($fieldHash->{'arrayDimensionInfo'});
                            my $arrayMaxIndex = $designDBRef->_getArrayMaxIndices($fieldHash->{'arrayDimensionInfo'});
                            my $arrayElemCnt = 0;
                            $fieldIndex = $arrayMinIndex;
                            for ($arrayElemCnt = 0; $arrayElemCnt < $arraySize; $arrayElemCnt++) {
				@hierarchicalPrefixList = ();
                                my $tmpFieldName = $designDBRef->_getArrayElementName($fieldHash->{'name'},
                                                                                      $fieldIndex);
                                $expandedFieldName = join('_',
                                                          @$prefixListRef,
                                                          $tmpFieldName);
				push(@hierarchicalPrefixList, (@$hierPrefixListRef, $tmpFieldName));
                                my $multiplier = $designDBRef->_getMultiplierFromArrayIndex($fieldIndex,
                                                                                            $fieldHash->{'arrayDimensionInfo'});
                                my $stride;
                                if (($arrayBitStride != 0) &&
                                    ($arrayElemCnt != 0)) {
                                    $stride = ($fieldHash->{'width'} * $multiplier) +
                                              ($arrayBitStride - $fieldHash->{'width'}) * $multiplier;
                                } else {
                                    $stride = $fieldHash->{'width'} * $multiplier;
                                }

#                                my $stride = $fieldHash->{'width'} * $multiplier;
                                if ($bitNumbering eq 'lsb_0') {
                                    $lsb = $fieldHash->{'lsb'} - $stride;
                                    $msb = $fieldHash->{'msb'} - $stride;
                                } else {
                                    $lsb = $fieldHash->{'lsb'} + $stride;
                                    $msb = $fieldHash->{'msb'} + $stride;
                                }
                                $fieldObj = csrPerlAPIField->new($fieldHash,
                                                                 $expandedFieldName,
                                                                 \@hierarchicalPrefixList,
                                                                 $lsb,
                                                                 $msb,
                                                                 $fieldIndex,
                                                                 $regHashRef);
                                if ($bitNumbering eq 'msb_0') {
                                    push(@tmpFieldList, $fieldObj);
                                } else {
                                    unshift(@tmpFieldList, $fieldObj);
                                }
                                $designDBRef->_incrementArrayIndex($fieldIndex,
                                                                   $fieldHash->{'arrayDimensionInfo'});
                            }
                            push (@$fieldObjList, @tmpFieldList) if (@tmpFieldList > 0);
                        }
                    } else { # do not expand field array
                        @hierarchicalPrefixList = ();
                        my $tmpFieldName = $fieldHash->{'name'};
                        $expandedFieldName = join('_',
                                                  @$prefixListRef,
                                                  $fieldHash->{'name'});
                        push(@hierarchicalPrefixList, (@$hierPrefixListRef, $tmpFieldName));
                        $lsb = $fieldHash->{'lsb'};
                        $msb = $fieldHash->{'msb'};
                        $fieldObj = csrPerlAPIField->new($fieldHash,
                                                         $expandedFieldName,
							 \@hierarchicalPrefixList,
							 $lsb,
							 $msb,
							 $fieldIndex,
							 $regHashRef);
			push (@$fieldObjList, $fieldObj);
		    }
		} else { # array of one element
		    my $arrayMaxIndex = $fieldHash->{'arrayDimensionInfo'}->[0]->{'arrayMax'};
		    @hierarchicalPrefixList = ();
		    my $tmpFieldName = $fieldHash->{'name'} . '[' . $arrayMaxIndex . ']';
		    $expandedFieldName = join('_',
					      @$prefixListRef,
					      $fieldHash->{'name'},
					      $arrayMaxIndex);
		    push(@hierarchicalPrefixList, (@$hierPrefixListRef, $tmpFieldName));
		    $lsb = $fieldHash->{'lsb'};
		    $msb = $fieldHash->{'msb'};
		    $fieldObj = csrPerlAPIField->new($fieldHash,
						     $expandedFieldName,
						     \@hierarchicalPrefixList,
						     $lsb,
						     $msb,
						     $arrayMaxIndex,
						     $regHashRef);
		    push (@$fieldObjList, $fieldObj);
		}
	    } else {
                my ($tmpFieldName);
                if (($viewName eq 'sv') && defined($fieldHash->{'wideRegisterName'}) &&
                    defined($fieldHash->{'parentFieldIsArray'})) {
                    my @segmentsIndex = split(', ', $fieldHash->{'segmentParentArrayUserIndex'});
                    my @indeces;
                    for (my $index = 0; $index < @segmentsIndex; $index++) {
                        my $nextIndex = join('', ('[', $segmentsIndex[$index], ']'));
                        push(@indeces, $nextIndex);
                    }
                    $tmpFieldName = $fieldHash->{'segmentOf'} . join('', @indeces);
                } else {
                    $tmpFieldName = $fieldHash->{'name'};
                }
                push(@hierarchicalPrefixList, (@$hierPrefixListRef, $tmpFieldName));
                my $prefixName = join('_', @$prefixListRef, $tmpFieldName);
		$fieldObj = csrPerlAPIField->new($fieldHash,
                                                 $prefixName,
						 \@hierarchicalPrefixList,
						 $fieldHash->{'lsb'},
						 $fieldHash->{'msb'},
						 $fieldIndex,
						 $regHashRef);
		push(@$fieldObjList, $fieldObj); 
	    }
	}	    
	$self->set_bitfields($fieldObjList);
    }

    sub getDeclaredExternalObjectPortList {
	my($self, $portDir) = @_;

	if ($self->isDeclaredExternal()) {
	    return $self->getExternalPortList($portDir);
	} else {
	    my @portList;
	    my $fieldList = $self->getBitfields();
	    foreach my $nextField (@{$fieldList}) {
		if ($nextField->isDeclaredExternal()) {
		    push (@portList, @{$nextField->getDeclaredExternalObjectPortList($portDir)});
		}
	    }
	    return \@portList;
	}
    }
    sub getObjectPortsList_NoExternalPorts {
	my($self, $direction) = @_;

        my $portListRef = [];
        my $tmpPortList = [];
        my ($fieldObj);
	my $noExternalPorts = 1;
        $portListRef = $self->getRegPortsList($direction, $noExternalPorts);
	while (my $fieldObj = $self->getNextFieldNode()) {
            next if(!defined($fieldObj->getPorts()));
            if(defined($tmpPortList = $fieldObj->getObjectPortsList_NoExternalPorts($direction))) {
                push(@$portListRef, @$tmpPortList);
            }
        }
        return $portListRef;
    }
    sub getRegPortsList {
        my($self, $direction, $noExternalPorts) = @_;

        my @portListRef;
        my ($parentNode, $portObj, $portDir, $portType, $sourceType);

        return undef if(defined($self->getAliasOf()));
	my $regPortListRef = $self->getPorts();
	my $currPortIndex = 0;
	while ($portObj = $self->getNextPortNode($regPortListRef,
						 \$currPortIndex)) {
            if(defined($portDir = $portObj->getPortDirection())) {
                if ($self->isExternal() && defined($portType = $portObj->getPortType())) {
                    next if (($portType eq 'wasReadPort') || ($portType eq 'writtenPort') ||
                             ($portType eq 'triggerPort') || ($portType eq 'modifiedPort') ||
			     ($portType eq 'interruptOutputPort'));
                }
		if (defined($sourceType = $portObj->getSourceType())) {
		    if (($sourceType ne 'fieldRef') && ($sourceType ne 'registerRef')) {
			push(@portListRef, $portObj) if($portDir eq $direction);
		    }
		} else {
		    push(@portListRef, $portObj) if($portDir eq $direction);
		}
            }
        }
	$noExternalPorts = 0 if (!defined($noExternalPorts));
	return \@portListRef if ($noExternalPorts);
	my $extPortListRef = $self->getExternalBusPorts();
	return \@portListRef if (!defined($extPortListRef));
	$currPortIndex = 0;
	while ($portObj = $self->getNextExternalPortNode($extPortListRef,
							 \$currPortIndex)) {
	    $portDir = $portObj->getPortDirection();
	    push(@portListRef, $portObj) if($portDir eq $direction);
	}
        return \@portListRef;
    }
    sub getObjectPortsList {
        my($self, $direction) = @_;

        my $portListRef = [];
        my $tmpPortList = [];
        my ($portObjList, $portDir, $fieldObj);
        return if (($self->childOfExternalObject() && !$self->getHierarchicalTBFlag()) ||
		   ($self->isBroadcastRegister() && $self->isExternal()));
        $portListRef = $self->getRegPortsList($direction);
	while (my $fieldObj = $self->getNextFieldNode()) {
            next if(!defined($fieldObj->getPorts()));
            if(defined($tmpPortList = $fieldObj->getObjectPortsList($direction))) {
                push(@$portListRef, @$tmpPortList);
            }
        }
        return $portListRef;
    }

    sub getPortList {
        my($self, $direction) = @_;

        my $portListRef = $self->getRegPortsList($direction);
        my $fieldListRef = $self->getBitfields();
        foreach my $fieldObj (@$fieldListRef) {
            next if(!defined($fieldObj->getPorts()));
            my $tmpPortList = $fieldObj->getPortList($direction);
            push(@$portListRef, @$tmpPortList);
        }
        return $portListRef;
    }
    sub getNextFieldNode {
        my $self = shift;
        my $bitfieldObjListRef;
        return if(!defined($bitfieldObjListRef = $self->getBitfields()));
        my $nextFieldIndex = $self->get_nextFieldIndex();
        if ($nextFieldIndex >= @{$bitfieldObjListRef}-1) {
            $self->set_nextFieldIndex(-1);
            return undef;
        }
        $self->set_nextFieldIndex(++$nextFieldIndex);
        return $bitfieldObjListRef->[$nextFieldIndex];
    }
    sub hasInterruptStateStatus {
	my $self = shift;

	# Does the register have a field that records the state of the
	# interrupt (after being ANDed with the enable bit) in a field
	# of type 'status'?

	my ($bitfieldListRef);

	$bitfieldListRef = $self->getBitfields();
	foreach my $fieldObj (@{$bitfieldListRef}) {
	    return 1 if ($fieldObj->isInterruptStateStatus());
	}
	return 0;
    }

    sub countSignalIsInterruptRef {
	my ($self, $counterType) = @_;

	my ($countPort, $countPortType, $fieldsList, $refType);

	if ($self->hasFieldType('counter')) {
	    $fieldsList = $self->getBitfields();
	    $countPortType = $counterType . 'CounterPort';
	    foreach my $fieldObj (@{$fieldsList}) {
		if ($fieldObj->getFieldType() eq 'counter') {
		    if($fieldObj->isPortReference($countPortType)) {
			$refType = $fieldObj->getPortReferenceType($countPortType);
			return 1 if ($refType eq 'interrupt');
		    }
		}
	    }
	}
	return 0;
    }

    sub hasFieldType {
        my($self, $fieldType) = @_;
        my ($bitfieldListRef);

        return 0 if(!defined($bitfieldListRef = $self->getBitfields()));
        foreach my $fieldObj (@{$bitfieldListRef}) {
            if ($fieldObj->getFieldType() eq $fieldType) {
                return 1;
            }
        }
        return 0;
    }
    
    sub getFieldType {
        my($self, $fieldType) = @_;
        my ($bitfieldListRef);
        return if(!defined($bitfieldListRef = $self->getBitfields()));
        foreach my $fieldObj (@{$bitfieldListRef}) {
            if ($fieldObj->getFieldType() eq $fieldType) {
                return $fieldObj;
            }
        }
        return;
    }
    sub getWidthBytes {
        my $self = shift;
        return $self->get_registerSize();
    }
    sub getReadEffect {
        my $self = shift;
        return $self->get_readEffect();
    }
    sub getResetSourceType {
	my $self = shift;
        my $resetObj = $self->getResetValueNode();
	return $resetObj->getSourceType();
    }
    sub getResetValue {
        my $self = shift;
        my $verilogFormat;
        my ($resetValue, $resetValueBigInt);
        my $resetObj = $self->getResetValueNode();
        if(defined($resetObj) && defined($resetObj->getValue())) {
            ($verilogFormat, $resetValue) = $self->getVerilogBaseAndValue($resetObj->getValue());
            $resetValueBigInt = Math::BigInt->new($resetValue);
        }
        return $resetValueBigInt;
    }
    sub regHasOutputPort {
        my $self = shift;
        my $portsListRef;
        if(defined($portsListRef = $self->getRegPortsList('output'))) {
            foreach my $portObj (@{$portsListRef}) {
                if($portObj->get_portType() eq 'outputPort') {
                    return $portObj;
                }
            }
        }
        return;
    }
    sub regHasReadEffect {
        my $self = shift;
        my $readEffect = $self->get_readEffect();
        return 1 if (($readEffect eq 'clear') || ($readEffect eq 'set'));
        return 0;
    }
    sub hasReadEffect {
        my $self = shift;

	my $fieldNodeList;
	return 1 if ($self->regHasReadEffect());
	return 0 if (!defined($fieldNodeList = $self->getBitfields()));
        foreach my $fieldObj (@{$fieldNodeList}) {
            return 1 if($fieldObj->hasReadEffect());
        }
        return 0;
    }
    sub hasResetValue {
        my $self = shift;
        my $fieldsListRef;
        return 0 if(!defined($fieldsListRef = $self->getBitfields()));
        foreach my $fieldObj (@{$fieldsListRef}) {
           return 1 if($fieldObj->hasResetValue());
        }
        return 0;
    }
    sub regHasWriteFunction {
        my $self = shift;
	return 1 if ($self->get_writeFunction() ne 'write');
	return 0;
    }
    sub hasWriteFunction {
        my $self = shift;
        foreach my $fieldObj (@{$self->getBitfields()}) {
            return 1 if (defined($fieldObj->hasWriteFunction()));
        }
        return 0;
    }
    sub getWriteFunction {
	my $self = shift;
	return $self->get_writeFunction();
    }
    sub hasEnum {
	my $self = shift;
	return 0 if (!defined($self->getBitfields()));
        foreach my $fieldObj (@{$self->getBitfields()}) {
	    return 1 if(defined($fieldObj->get_fieldEnum()));
        }
        return 0;
    }
    sub hasMinimumValue {
        my $self = shift;
        return 0 if(!defined($self->getBitfields()));
        foreach my $fieldObj (@{$self->getBitfields()}) {
	    return 1 if(defined($fieldObj->get_minimumValue()));
        }
        return 0;
    }
    sub hasMaximumValue {
        my $self = shift;
        return 0 if(!defined($self->getBitfields));
        foreach my $fieldObj (@{$self->getBitfields()}) {
	    return 1 if (defined($fieldObj->get_maximumValue()));
        }
        return 0;
    }
    sub hasReturnToZero {
        my $self = shift;
        return 0 if(!defined($self->getBitfields));
        foreach my $fieldObj (@{$self->getBitfields()}) {
            if($fieldObj->hasReturnToZero()) {
                return 1;
            }
        }
        return 0;
    }
    sub isReadAtomic {
        my $self = shift;

        my ($parentRegObj, $atomicAccess);
	if ($self->getNodeType() eq 'wideRegister') {
	    $parentRegObj = $self;
	} else {
	    if(defined($self->get_subRegisterFlag())) {
		$parentRegObj = $self->getParentNode();
	    } else {
		return 0;
	    }
	}
	if(defined($atomicAccess = $parentRegObj->get_atomicAccess()) &&
	   (($atomicAccess eq 'lsbRead') || ($atomicAccess eq 'msbRead') ||
	    ($atomicAccess eq 'lsb') || ($atomicAccess eq 'msb') ||
	    ($atomicAccess eq 'lsbReadMsbWrite') || ($atomicAccess eq 'msbReadLsbWrite'))) {
	    return 1;
        }
        return 0;
    }
    sub isWriteAtomic {
        my $self = shift;

        my ($parentRegObj, $atomicAccess);
	if ($self->getNodeType() eq 'wideRegister') {
	    $parentRegObj = $self;
	} else {
	    if(defined($self->get_subRegisterFlag())) {
		$parentRegObj = $self->getParentNode();
	    } else {
		return 0;
	    }
	}
	if(defined($atomicAccess = $parentRegObj->get_atomicAccess()) &&
	   (($atomicAccess eq 'lsbWrite') || ($atomicAccess eq 'msbWrite') ||
	    ($atomicAccess eq 'lsb') || ($atomicAccess eq 'msb') ||
	    ($atomicAccess eq 'msbReadLsbWrite') || ($atomicAccess eq 'lsbReadMsbWrite'))) {
	    return 1;
        }
        return 0;
    }
    sub isAtomic {
        my $self = shift;

        my ($parentRegObj, $atomicAccess);
	if ($self->getNodeType() eq 'wideRegister') {
	    $parentRegObj = $self;
	} else {
	    if(defined($self->get_subRegisterFlag())) {
		$parentRegObj = $self->getParentNode();
	    } else {
		return 0;
	    }
	}
	if(defined($atomicAccess = $parentRegObj->get_atomicAccess()) &
	   (($atomicAccess eq 'lsb') || ($atomicAccess eq 'msb') ||
	    ($atomicAccess eq 'lsbRead') || ($atomicAccess eq 'msbRead') ||
	    ($atomicAccess eq 'lsbWrite') || ($atomicAccess eq 'msbWrite') ||
	    ($atomicAccess eq 'lsbReadMsbWrite') || ($atomicAccess eq 'msbReadLsbWrite'))) {
	    return 1;
        }
        return 0;
    }

    sub isResetValueConstant {
        my $self = shift;
        my $type;
        my $resetObj;
        # is the reset value of all the fields of the register
        # a constant value(i.e., it is NOT a field reference AND
        # not an input port value)?
        foreach my $fieldObj (@{$self->getBitfields()}) {
            if($fieldObj->hasResetValue()) {
                $type = $fieldObj->getResetSourceType();
                if(($type eq 'inputPort') || ($type eq 'fieldRef')) {
                    return 0;
                }
            }
        }
        return 1;
    }
    sub getParentNode {
        my $self = shift;

        # Get the parent object of the calling object.
        # can be called by:
        # 1 - the sub-register object of a wide register
        #     to obtain the parent wide register object

        my ($relAddr, $verilogFormat, $bigIntOffset);
        my (@aliasPrefixList, $parentObject, $bogusAddr, $topLevelHashRef);
        my ($regHashRef, $parentName, $parentHashRef, $parentOfParentHashRef);
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage);
        my $bigIntAddr = Math::BigInt->new('0');
        my $hierName = $self->get_hierarchicalNameDotted();
        my @levelNameList = split('\.', $hierName);
        my @hierPrefixList = split('\.', $hierName);

        if(@levelNameList <= 0) {
            push(@levelNameList, $self->getHierarchicalIdentifier());
        }
        my @tmpLevelNameList = @levelNameList;
        ($topLevelHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);

        if(@levelNameList > 2) {
            # more than one level of hierarchy
            pop(@tmpLevelNameList);
            $parentName = join('.', @tmpLevelNameList);
            ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
            if(@levelNameList > 3) {
                # get to the parent of parent node
                pop(@tmpLevelNameList);
                $parentName = join('.', @tmpLevelNameList);
                ($parentOfParentHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($parentName);
            } else {
                $parentOfParentHashRef = $topLevelHashRef;
            }
        } else {
            $parentHashRef = $topLevelHashRef;
            # there is no parent of parent node.
        }
        ($busProtocol, $inputPipelineStage, $outputPipelineStage) =
            $designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);

	($verilogFormat, $relAddr) =
	    $self->getVerilogBaseAndValue($parentHashRef->{'addressmapRelativeByteAddress'});
	my $relAddrBigInt = Math::BigInt->new($relAddr);
        pop(@levelNameList); # get to the parent
	pop(@hierPrefixList);
        if(defined($parentHashRef)) {
            my $arrayIndex = $self->getParentArrayIndexFromHierId($hierName);
            if (($viewName eq 'sv') && (@$arrayIndex)) {
                my $tmpIndex;
                for (my $count=0; $count < @$arrayIndex; $count++) {
                    $tmpIndex .= '[' . $arrayIndex->[$count] . ']';
                }
                $levelNameList[@levelNameList-1] .= $tmpIndex;
                $hierPrefixList[@hierPrefixList-1] .= $tmpIndex;
            }
            if ($parentHashRef->{'nodeType'} eq 'wideRegister') {
		shift @levelNameList;
                if(defined($parentHashRef->{'aliasOf'})) {
                    @tmpLevelNameList = @levelNameList;
                    pop(@tmpLevelNameList);
                    push(@aliasPrefixList, (@tmpLevelNameList, $parentHashRef->{'aliasOf'}));
                    if (($viewName eq 'sv') && (@$arrayIndex)) {
                        my $tmpIndex;
                        for (my $count=0; $count < @$arrayIndex; $count++) {
                            $tmpIndex .= '[' . $arrayIndex->[$count] . ']';
                        }
                        $aliasPrefixList[@aliasPrefixList-1] .= $tmpIndex;
                    }
                }
		($verilogFormat, $bigIntOffset) =
                    $self->getVerilogBaseAndValue($parentHashRef->{'byteOffset'});
		$updateRefArrayIndex = 0;
                $parentObject = csrPerlAPIRegister->new($parentHashRef,
							$arrayIndex,
                                                        $parentOfParentHashRef,
                                                        $bigIntAddr,
                                                        $bigIntOffset,
                                                        $relAddrBigInt,
                                                        $busProtocol,
                                                        $inputPipelineStage,
                                                        $outputPipelineStage,
                                                        \@levelNameList,
                                                        \@aliasPrefixList,
							\@hierPrefixList);
            } elsif (($parentHashRef->{'nodeType'} eq 'memory') ||
                     ($parentHashRef->{'nodeType'} eq 'wideMemory')) {
		shift @levelNameList;
		($verilogFormat, $bigIntOffset) = $self->getVerilogBaseAndValue($parentHashRef->{'byteOffset'});
		$bigIntOffset = Math::BigInt->new($bigIntOffset);
		$parentObject = csrPerlAPIMemory->new($parentHashRef,
						      $arrayIndex,
						      $parentOfParentHashRef,
						      $bigIntAddr,
						      $bigIntOffset,
						      $relAddrBigInt,
						      $busProtocol,
						      $inputPipelineStage,
						      $outputPipelineStage,
						      \@levelNameList,
						      \@hierPrefixList);
	    } elsif ($parentHashRef->{'nodeType'} eq 'group') {
                $parentObject = csrPerlAPIGroup->new($parentHashRef,
						     $arrayIndex,
						     $parentOfParentHashRef,
						     $bigIntAddr,
						     $relAddrBigInt,
						     $busProtocol,
						     $inputPipelineStage,
						     $outputPipelineStage,
						     \@levelNameList,
						     \@hierPrefixList);
	    } elsif ($parentHashRef->{'nodeType'} eq 'union') {
                $parentObject = csrPerlAPIUnion->new($parentHashRef,
						     $arrayIndex,
						     $parentOfParentHashRef,
						     $bigIntAddr,
						     $relAddrBigInt,
						     $busProtocol,
						     $inputPipelineStage,
						     $outputPipelineStage,
						     \@levelNameList,
						     \@hierPrefixList);
            } else {
                $parentObject = csrPerlAPIModule->new($parentHashRef,
						      $arrayIndex,
                                                      $parentOfParentHashRef,
                                                      $bigIntAddr,
						      $relAddrBigInt,
                                                      $busProtocol,
                                                      $inputPipelineStage,
                                                      $outputPipelineStage,
                                                      \@levelNameList,
						      \@hierPrefixList);
            }
            return $parentObject;
        }
        return;
    }

}

package csrPerlAPIMemory;

use strict;
@csrPerlAPIMemory::ISA = qw(csrPerlAPIRegisterBase);

{
    my %memoryAttrs = (_wordCount => '',
                       _memWordAddress => '',
                       _nextExternalBusPortIndex => -1,
                       _atomicAccess => '',
                       _subMemoryFlag => '',
                       _subMemoryIndex => '',
                       _hasArrayDecoder => '',
                       _sharedShadow => '',
                       );

    sub _defaultAttrKeys {
        my $self = shift;
        return ($self->SUPER::_defaultAttrKeys(), map { s/^_//; $_ } keys %memoryAttrs);
    }

    sub new {
        my($caller,
           $memHashRef,
	   $arrayIndex,
           $parentHashRef,
           $bigIntCsrAddr,
           $bigIntOffset,
	   $addrmapRelByteAddr,
           $busProtocol,
           $inputPipelineStage,
           $outputPipelineStage,
           $prefixListRef,
	   $hierPrefixListRef) = @_;
        
	my ($verilogFormat, $relAddr, $aliasPrefixListRef, $expandedAliasName);
        my $self = $_[0]->SUPER::new($memHashRef,
				     $arrayIndex,
                                     $parentHashRef,
                                     $bigIntCsrAddr,
                                     $bigIntOffset,
				     $addrmapRelByteAddr,
                                     $busProtocol,
                                     $inputPipelineStage,
                                     $outputPipelineStage,
                                     $prefixListRef,
				     $hierPrefixListRef,
				     $aliasPrefixListRef,
				     $expandedAliasName);
        $self->set_nextExternalBusPortIndex(-1);
        return $self;
    }
    sub getNextDesignNode {
        my $self = shift;

        # Iterate thru the virtual registers of a memory node, if they exist.

        my @prefixList = ();
        my ($currAddrmapInfo, $addrmapHashRef, $bigIntOffset);
        my ($nextObj, $relAddr);
        return if ($self->isSubMemoryNode());

        if(@visitedAddrmapsList == 0) {
            # We're just starting to process the nodes.
            my $parentAddrmapHashRef;
            my $arrayIndex = undef;
            $addrmapHashRef = $self->get_nodeHashRef();
            my ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($self->getByteAddress());
            my $bigIntAddr = Math::BigInt->new($offset);
	    ($verilogFormat,
             $relAddr) = $self->getVerilogBaseAndValue($addrmapHashRef->{'addressmapRelativeByteAddress'});
	    my $relAddrBigInt = Math::BigInt->new($relAddr);
            ($verilogFormat, $bigIntOffset) = $self->getVerilogBaseAndValue($self->getByteOffset());
            my $hierId = $self->getHierarchicalIdentifier();
            my @hierarchicalPrefixList = split('\.', $hierId);
            my @prefixList = split('\.', $hierId);
            my $memObj = csrPerlAPIMemory->new($addrmapHashRef,
                                               $arrayIndex,
                                               $parentAddrmapHashRef,
                                               $bigIntAddr,
                                               $bigIntOffset,
                                               $relAddrBigInt,
                                               $addrmapHashRef->{'busProtocol'},
                                               $addrmapHashRef->{'inputPipelineStage'},
                                               $addrmapHashRef->{'outputPipelineStage'},
                                               \@prefixList,
                                               \@hierarchicalPrefixList);
	    $currAddrmapInfo->{'currAddrmapHashRef'} = $addrmapHashRef;
	    $currAddrmapInfo->{'currNode'} = $addrmapHashRef;
	    $currAddrmapInfo->{'parentAddr'} = $bigIntAddr;
	    $currAddrmapInfo->{'relativeAddress'} = $bigIntAddr;
	    $self->setInitialAddrmapIndexes($currAddrmapInfo, $addrmapHashRef);
	    push(@prefixList, $addrmapHashRef->{'name'}) if (!$self->getSemiforeEnvGenFlag());
	    $currAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	    $currAddrmapInfo->{'prefixListRef'} = \@prefixList;
	    push(@visitedAddrmapsList, $currAddrmapInfo);
            if ($memObj->isWideMemoryNode()) {
                $currAddrmapInfo->{'currMemoryHashRef'} = $addrmapHashRef;
                $self->setInitialWideMemoryIndexes($currAddrmapInfo,
                                                   $addrmapHashRef,
                                                   $addrmapHashRef);
            }
	    return $memObj;
	}
        return if(@visitedAddrmapsList == 0);

        $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
        $addrmapHashRef = $currAddrmapInfo->{'currAddrmapHashRef'};
        my $prefixListRef = $currAddrmapInfo->{'prefixListRef'};

        if (!defined($currAddrmapInfo->{'memoryParentAddressmapObj'})) {
            $currAddrmapInfo->{'memoryParentAddressmapObj'} = $self->getParentNode();
        }
        my $parentAddrmapNode = $currAddrmapInfo->{'memoryParentAddressmapObj'};
        my $currNode = $currAddrmapInfo->{'currNode'};
        my $currNodeType = $currNode->{'nodeType'};

        # Memory objects may have virtual registers.
        if (($currNodeType eq 'memory') ||
            ($currNodeType eq 'wideMemory') ||
            ($currNodeType eq 'group')) {
            if($self->moreRegistersAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
                $nextObj = $self->getNextRegister($currAddrmapInfo, $addrmapHashRef);
                # We may've just processed a wide register (and pushed it in the
                # LIFO. Wide register accounting is done somewhere else.
                $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
                if(($currAddrmapInfo->{'currNode'})->{'nodeType'} ne 'wideRegister') {
                    $self->updateRegisterAccounting($currAddrmapInfo);
                }
                return $nextObj;
	    } elsif (($currNodeType ne 'memory') &&
                $self->moreMemoriesAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
                $nextObj = $self->getNextMemory($currAddrmapInfo, $addrmapHashRef);
		return $nextObj;
            } elsif ($self->moreGroupsAtThisLevel($currAddrmapInfo, $addrmapHashRef)) {
                $nextObj = $self->getNextGroup($currAddrmapInfo, $addrmapHashRef);
                return $nextObj;
            } else {
                pop(@visitedAddrmapsList);
                if(@visitedAddrmapsList == 0) {
                    # We're done.
                    return;
                } else {
                    $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
                    $addrmapHashRef = $currAddrmapInfo->{'currAddrmapHashRef'};
                    if ($currNodeType eq 'group') {
                        $self->updateGroupAccounting($currAddrmapInfo);
                    } elsif ($currNodeType eq 'memory') {
                        $self->updateMemoryAccounting($currAddrmapInfo);
                    } else {
                        $self->_printError('getNextDesignNode, mem pkg, Node type: ' . $currNodeType . ' not handled');
                        exit(1);
                    }
                    $nextObj = $self->getNextDesignNode();
                    return $nextObj;
                }
            }
	} elsif($currNodeType eq 'wideRegister') {
	    my $regHashRef = $currAddrmapInfo->{'currRegisterHashRef'};
	    my $subRegHashListRef = $regHashRef->{'subRegisters'};
	    if(($currAddrmapInfo->{'currAddrmapSubRegisterIndex'} >= @{$subRegHashListRef}) &&
	       (defined($regHashRef->{'arrayDimensionInfo'}))) {
		# we have exhausted the sub register list for the current array element.
		my $arrayMax = $designDBRef->_getArrayMaxIndices($regHashRef->{'arrayDimensionInfo'});
		if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
						     $arrayMax) <= 0) {
		    # There's at least another array element. We need to generate
		    # an object for the new wide register array element before
		    # continuing with sub-registers.
		    $nextObj = $self->getNextWideRegArrayElement($currAddrmapInfo, $addrmapHashRef);
		    return $nextObj;
		}
	    }
	    $nextObj = $self->getNextSubRegister($currAddrmapInfo, $addrmapHashRef);
	    $self->updateWideRegisterAccounting($currAddrmapInfo, $addrmapHashRef);
	    return $nextObj;
        }
	return;
    }

    sub getDeclaredExternalObjectPortList {
	my($self, $direction) = @_;
        my ($portDir, $portObj, $extPortListRef, @portList);

        return $extPortListRef if (!defined($extPortListRef = $self->getExternalBusPorts()));
	my $currPortIndex = 0;
	while ($portObj = $self->getNextExternalPortNode($extPortListRef,
							 \$currPortIndex)) {
	    if (defined($portDir = $portObj->getPortDirection()) &&
		 ($portDir eq $direction)) {
		push(@portList, $portObj);
	    }
        }
	return \@portList;
    }
    sub getObjectPortsList_NoExternalPorts {
	my($self, $portDir) = @_;
	return;
    }
    sub getObjectPortsList {
        my($self, $direction) = @_;

        my ($portDir, $portObj, @portListRef, $extPortListRef);

        return undef if (!defined($extPortListRef = $self->getExternalBusPorts()));
        return if ($self->childOfExternalObject() && !$self->getHierarchicalTBFlag());
        if (($viewName eq 'sv') && ($self->getNodeType() eq 'wideMemory') &&
            $includeWideMemoriesFlag) {
            my $currPortIndex = 0;
            while ($portObj = $self->getNextExternalPortNode($extPortListRef,
                                                             \$currPortIndex)) {
                if(defined($portDir = $portObj->getPortDirection())) {
                    push(@portListRef, $portObj) if($portDir eq $direction);
                }
            }
            return \@portListRef;
        }
        return undef if ($self->getNodeType() eq 'wideMemory');
	my $parentNode = $self->getParentAddressmapOrGroupNode();
	if (!$parentNode->isExternal()) {
	    my $currPortIndex = 0;
	    while ($portObj = $self->getNextExternalPortNode($extPortListRef,
							     \$currPortIndex)) {
		if(defined($portDir = $portObj->getPortDirection())) {
		    push(@portListRef, $portObj) if($portDir eq $direction);
		}
	    }
        }
        return \@portListRef;
    }
    sub getExternalIOPorts {
	my $self = shift;
	my $ioPorts;

	my $inputPorts = $self->getExternalPortList('input');
	if(defined($inputPorts)) {
	    push(@$ioPorts, @$inputPorts);
	}
	my $outputPorts = $self->getExternalPortList('output');
	if(defined($outputPorts)) {
	    push(@$ioPorts, @$outputPorts);
	}
	return $ioPorts;
    }
    sub hasArrayDecoder {
	my $self = shift;
	my $hasArrayDecoder = 'false';
	if (defined($hasArrayDecoder = $self->get_hasArrayDecoder()) &&
	    ($hasArrayDecoder eq 'true')) {
	    return 1;
	} else {
	    return 0;
	}
    }
    sub getSubMemoryIndex {
        my $self = shift;
        return $self->get_subMemoryIndex();
    }
    sub getSubMemoryFlag {
        my $self = shift;
        return $self->get_subMemoryFlag();
    }
    sub getNumSubMemories {
        my $self = shift;

        # must be called either with a wide memory object
        # or a sub-memory object of a wide memory.
        my $parentHashRef = $self->get_parentHashRef();
        my $nodeType = $self->getNodeType();
        return if(($nodeType ne 'wideMemory') && ($parentHashRef->{'nodeType'} ne 'wideMemory'));
        if($nodeType eq 'wideMemory') {
            return scalar(@{($self->get_nodeHashRef())->{'subMemories'}});
        } else {
            return scalar(@{$parentHashRef->{'subMemories'}});
        }
    }
    sub isAlias {
        return 0;
    }
    sub getSubMemoriesList {
        my $self = shift;

        my ($verilogFormat, $offset, $relAddr, @hierNameList);
        my ($topLevelHashRef, $parentHashRef, $wideMemHashRef);
        my (@wideMemPrefixList, $parentName);
        my (@subMemObjectList, @levelNameList, @aliasPrefixList);
        my ($wideMemName, $subMemObj, @prefixList);
        my $bigIntAddr = Math::BigInt->new('0');

        $wideMemName = $self->getHierarchicalIdentifier();
        @levelNameList = split('\.', $wideMemName);
        @wideMemPrefixList = split('\.', $wideMemName);

        ($topLevelHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);
        if(@levelNameList > 2) {
            # more than one level of hierarchy
            pop(@levelNameList);
            $parentName = join('.', @levelNameList);
            ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
        } else {
            $parentHashRef = $topLevelHashRef;
        }
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage) = 
            $designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);

        ($wideMemHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($wideMemName);
	my $arrayIndex = undef;
        for (my $index=0; $index < (@{$wideMemHashRef->{'subMemories'}}); $index++) {
            my $subMemHashRef = $wideMemHashRef->{'subMemories'}->[$index];
	    my @hierNameList;
            @prefixList = ();
            push(@prefixList, (@wideMemPrefixList, join('_', $subMemHashRef->{'name'})));
            push(@hierNameList, (@wideMemPrefixList, $subMemHashRef->{'name'}));
            ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($subMemHashRef->{'byteOffset'});
            $bigIntAddr->badd($offset);
            ($verilogFormat, $relAddr) =
               $self->getVerilogBaseAndValue($subMemHashRef->{'addressmapRelativeByteAddress'});
            my $relAddrBigInt = Math::BigInt->new($relAddr);
            $updateRefArrayIndex = 0;
            $subMemObj = csrPerlAPIMemory->new($subMemHashRef,
                                               $arrayIndex,
                                               $wideMemHashRef,
                                               $bigIntAddr,
                                               $offset,
                                               $relAddrBigInt,
                                               $busProtocol,
                                               $inputPipelineStage,
                                               $outputPipelineStage,
                                               \@prefixList,
                                               \@hierNameList);
            $bigIntAddr->bsub($offset);
            $subMemObj->set_subMemoryIndex($index);
            $subMemObj->set_subMemoryFlag('true');
            push(@subMemObjectList, $subMemObj);
        }
        return @subMemObjectList;
    }

    sub getAtomicAccess {
        my $self = shift;
        return $self->get_atomicAccess();
    }
    sub isReadAtomic {
        my $self = shift;

        my ($parentMemObj, $atomicAccess);
	if ($self->getNodeType() eq 'wideMemory') {
	    $parentMemObj = $self;
	} else {
	    if(defined($self->get_subMemoryFlag())) {
		$parentMemObj = $self->getParentNode();
	    } else {
		return 0;
	    }
	}
	if(defined($atomicAccess = $parentMemObj->get_atomicAccess()) &&
	   (($atomicAccess eq 'lsbRead') || ($atomicAccess eq 'msbRead') ||
	    ($atomicAccess eq 'lsb') || ($atomicAccess eq 'msb') ||
	    ($atomicAccess eq 'lsbReadMsbWrite') || ($atomicAccess eq 'msbReadLsbWrite'))) {
	    return 1;
        }
        return 0;
    }
    sub isWriteAtomic {
        my $self = shift;

        my ($parentMemObj, $atomicAccess);
	if ($self->getNodeType() eq 'wideMemory') {
	    $parentMemObj = $self;
	} else {
	    if(defined($self->get_subMemoryFlag())) {
		$parentMemObj = $self->getParentNode();
	    } else {
		return 0;
	    }
	}
	if(defined($atomicAccess = $parentMemObj->get_atomicAccess()) &&
	   (($atomicAccess eq 'lsbWrite') || ($atomicAccess eq 'msbWrite') ||
	    ($atomicAccess eq 'lsb') || ($atomicAccess eq 'msb') ||
	    ($atomicAccess eq 'msbReadLsbWrite') || ($atomicAccess eq 'lsbReadMsbWrite'))) {
	    return 1;
        }
        return 0;
    }
    sub isAtomic {
        my $self = shift;

        my ($parentMemObj, $atomicAccess);
	if ($self->getNodeType() eq 'wideMemory') {
	    $parentMemObj = $self;
	} else {
	    if(defined($self->get_subMemoryFlag())) {
		$parentMemObj = $self->getParentNode();
	    } else {
		return 0;
	    }
	}
	if(defined($atomicAccess = $parentMemObj->get_atomicAccess()) &
	   (($atomicAccess eq 'lsb') || ($atomicAccess eq 'msb') ||
	    ($atomicAccess eq 'lsbRead') || ($atomicAccess eq 'msbRead') ||
	    ($atomicAccess eq 'lsbWrite') || ($atomicAccess eq 'msbWrite') ||
	    ($atomicAccess eq 'lsbReadMsbWrite') || ($atomicAccess eq 'msbReadLsbWrite'))) {
	    return 1;
        }
        return 0;
    }
    sub getWordCount {
        my $self = shift;
        return $self->get_wordCount();
    }
    sub hasInputSynchronizer {
	my $self = shift;
	return 0;
    }
    sub hasBitfields {
	my $self = shift;
	return 0;
    }
    sub getBitfields {
        my $self = shift;
        return;
    }
    sub getWidthBytes {
	my $self = shift;
	my $widthBits = $self->getWidth();
	my $widthBytes = int($widthBits/8);
	$widthBytes++ if ($widthBits % 8 > 0);
	return $widthBytes;
    }
    sub getParentNode {
        my $self = shift;

        # get the parent address map or group object for the calling object.
        # if calling object is sub-memory object, returns parent wide memory object.
        my $count = 0;
	my ($bigIntOffset, $arrayIndex);
	my ($verilogFormat, $relAddr, @hiearchicalPrefixList);
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage);
        my (@prefixList, $parentHashRef, $parentOfParentHashRef);
        my ($topLevelHashRef, $parentName, $bogusAddr);

        my $bigIntAddr = Math::BigInt->new('0');
        my $hierId = $self->getHierarchicalIdentifier();
        my @levelNameList = split('\.', $self->get_hierarchicalNameDotted());
        my @hierarchicalPrefixList = split('\.', $self->get_hierarchicalNameDotted());

        return if(@levelNameList <= 1);

        ($topLevelHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);

        pop(@levelNameList);
	pop(@hierarchicalPrefixList);

	$parentName = join('.', @levelNameList);
	($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
	if($parentHashRef->{'nodeType'} eq 'wideMemory') {
	    ($busProtocol, $inputPipelineStage, $outputPipelineStage) =
		$designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);
	    @prefixList = @levelNameList;
	    shift @prefixList;
	    # parent of parent hash ref
	    if(@levelNameList > 0) {
		# get to the parent of parent node
		if(@levelNameList == 1) {
		    $parentName = $levelNameList[0];
		} else {
		    $parentName = join('.', @levelNameList);
		}
		($parentOfParentHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($parentName);
	    }
	    ($verilogFormat, $relAddr) =
		$self->getVerilogBaseAndValue($parentHashRef->{'addressmapRelativeByteAddress'});
	    my $relAddrBigInt = Math::BigInt->new($relAddr);
	    ($verilogFormat, $bigIntOffset) = $self->getVerilogBaseAndValue($parentHashRef->{'byteOffset'});
            my $arrayIndex = $self->getParentArrayIndexFromHierId($hierId);
            if (($viewName eq 'sv') && (@$arrayIndex)) {
                my $tmpIndex;
                for (my $count=0; $count < @$arrayIndex; $count++) {
                    $tmpIndex .= '[' . $arrayIndex->[$count] . ']';
                }
                $levelNameList[@levelNameList-1] .= $tmpIndex;
                $prefixList[@prefixList-1] .= $tmpIndex;
            }
	    my $memoryObj = csrPerlAPIMemory->new($parentHashRef,
						  $arrayIndex,
						  $parentOfParentHashRef,
						  $bigIntAddr,
						  $bigIntOffset,
						  $relAddrBigInt,
						  $busProtocol,
						  $inputPipelineStage,
						  $outputPipelineStage,
						  \@prefixList,
						  \@levelNameList);
	    return $memoryObj;
	} else {
	    my $parentObj = $self->getParentAddressmapOrGroupNode();
	    return $parentObj;
	}
    }
}


package csrPerlAPIField;

@csrPerlAPIField::ISA = qw(csrPerlAPIBase);

use strict;
use vars '$AUTOLOAD';

my $currFieldEnumIndex = -1;

{
    my %fieldAttrs = (
		      _lsb => '',
		      _msb => '',
		      _width => '',
		      _range => '',
		      _rangeMsb => '',
		      _rangeLsb => '',
		      _rangeWidth => '',
		      _fieldType => '',
		      _bitNumbering => '',
		      _byteEndian => '',
		      _readEffect => '',
		      _writeFunction => '',
		      _segmentOf => '',
		      _segmentIndex => '',
		      _segmentParentArrayUserIndex => '',
		      _parentWidth => '',
		      _parentMsb => '',
		      _parentLsb => '',
		      _resetValueNode => '',
		      _resetClass => '',
                      _resetTiming => '',
		      _incrementCount => '',
		      _decrementCount => '',
		      _latching => '',
		      _interrupt => '',
		      _writeAsRead => '',
		      _fieldEnum => '',
		      _ports => '',
		      _lowPrioritySWWrite => '',
		      _uniqueResetPort => '',
		      _synchronizerStyle => '',
		      _hasInputSynchronizer => '',
		      _hasOutputSynchronizer => '',
                      _inputPortNamePrefix => '',
                      _outputPortNamePrefix => '',
                      _inputPortNamedSuffix => '',
                      _outputPortNameSuffix => '',
                      _arrayBitStride => '',
		      _sharedSet => '',
		      _sharedOf => '',
		      _aliasOf => '',
		      _parentAliasOf => '',
		      _aliases => '',
                      _parityProtect => '',
		      _minimumValue => '',
		      _maximumValue => '',
                      _returnToZero => '',
		      _hardwareVolatile => '',
                      _dontTest => '',
                      _dontCompare => '',
		      _wideRegisterName => '',
		      _parentHashRef => '',
		      _overlapFlag => '',
		      _nextExternalBusPortIndex => -1,
		      _totalNumPorts  => 0,
#                      _triggerValueNode => '',
		      );
    sub _defaultAttrKeys {
	my $self = shift;
	return ($self->SUPER::_defaultAttrKeys(), map { s/^_//; $_ } keys %fieldAttrs);
    }
    sub new {
        my($caller,
           $fieldHash,
           $expandedName,
           $prefixListRef,
           $lsb,
           $msb,
           $fieldIndex,
           $parentHashRef) = @_;

        my $inputPortHash = {};
        my ($portListRef, @portType, $resetTiming, $expandedAliasName);
        my ($inputPortIndex, $subRegName, $subRegNode, $subRegHashRef);
        my ($bitfieldListRef, $count, $bogusAddr, $numSubRegisters);
        my (@prefixNameList, $intObj, $wideRegName, $wideRegHashRef);
        my $self = $_[0]->SUPER::new($fieldHash);

        my $hierId = join('.', @$prefixListRef);
        if ($viewName ne 'sv') {
            $expandedName =~ s/\[/_/g;
            $expandedName =~ s/\]//g;
        } else {
            my @arrayIndex;
            while ($expandedName =~ /(\[\d+\])/g) {
                push (@arrayIndex, $1);
            }
            $expandedName =~ s/\[\d+\]//g;
            $expandedName .= join('', @arrayIndex);
            $hierId =~ s/\[\d+\]//g;
            $hierId .= join('', @arrayIndex);
	 }
        $self->set_name($expandedName);
        $self->set_hierarchicalName($expandedName);
        $self->set_hierarchicalNameDotted($hierId);
        $self->set_lsb($lsb);
        $self->set_msb($msb);
        $self->set_nextExternalBusPortIndex(-1);
        $self->set_nodeHashRef($fieldHash);
        $self->set_parentHashRef($parentHashRef);
	$self->set_totalNumPorts(0);
	$self->set_overlapFlag(0);
        if(($parentHashRef->{'declaredExternal'} eq 'false') &&
	   ($parentHashRef->{'nodeType'} eq 'wideRegister') &&
           ($fieldHash->{'fieldType'} eq 'status')) {
            $portListRef = $fieldHash->{'ports'};
            for($count = 0; $count < @$portListRef; $count++) {
                @portType = keys %{$portListRef->[$count]};
                if($portType[0] eq 'inputPort') {
                    $fieldHash->{'inputPortPrevMsb'} = $portListRef->[$count]->{'inputPort'}->{'msb'};
                    $fieldHash->{'inputPortPrevLsb'} = $portListRef->[$count]->{'inputPort'}->{'lsb'};
                    last;
                }
            }
        }
	if (defined($fieldHash->{'aliasOf'})) {
	    my $aliasPrefixListRef = $self->expandAliasArrayMembers($fieldHash,
								    $prefixListRef);
            my $aliasHierId = join('.', @$aliasPrefixListRef);
            $expandedAliasName = join('_', @$aliasPrefixListRef);
            if ($viewName ne 'sv') {
                $expandedAliasName =~ s/\[/_/g;
                $expandedAliasName =~ s/\]//g;
            } else {
                my @arrayIndex;
                while ($expandedAliasName =~ /(\[\d+\])/g) {
                    push (@arrayIndex, $1);
                }
                $expandedAliasName =~ s/\[\d+\]//g;
                $expandedAliasName .= join('', @arrayIndex);
                $aliasHierId =~ s/(\[\d+\])+//g;
                $aliasHierId .= join('', @arrayIndex);
            }
            $expandedAliasName =~ s/\./_/g;
            $self->set_aliasName(join('_', $expandedAliasName));
            $self->set_aliasHierarchicalIdentifier($aliasHierId);
	}
        if(exists($fieldHash->{'segmentOf'})) {
            # For this field hash in the sub-register, get its counterpart field hash
            # in the wide register.
            @prefixNameList = @$prefixListRef;
            pop(@prefixNameList); pop(@prefixNameList);
            $wideRegName = join('.', @prefixNameList);
            ($wideRegHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($wideRegName);
            $bitfieldListRef = $wideRegHashRef->{'bitfields'};
            for($count=0; $count < @$bitfieldListRef; $count++) {
                if($bitfieldListRef->[$count]->{'name'} eq $fieldHash->{'segmentOf'}) {
                    $self->set_wideRegisterFieldHash($bitfieldListRef->[$count]);
		    $self->set_hasInputSynchronizer($bitfieldListRef->[$count]->{'hasInputSynchronizer'});
		    $self->set_hasOutputSynchronizer($bitfieldListRef->[$count]->{'hasOutputSynchronizer'});
                    last;
                }
            }
            $self->set_segmentOf($fieldHash->{'segmentOf'});
            $self->set_parentLsb($fieldHash->{'parentLsb'});
            $self->set_parentMsb($fieldHash->{'parentMsb'});
        }
	$self->set_resetTiming($addressmapResetTiming) if ($self->getResetTiming() eq 'addressmap_bus');

	if (defined($fieldHash->{'fieldEnum'})) {
	    $self->set_fieldEnum(csrFieldEnum->new($fieldHash->{'fieldEnum'}));
	}
        if(exists($fieldHash->{'resetValueInfo'})) {
            my $resetValueObj = csrResetValue->new($fieldHash->{'resetValueInfo'});
            if(exists($fieldHash->{'arrayDimensionInfo'})) {
                my $resetValue = $resetValueObj->getValue();
                # for array of fields where each field has a different reset value.
                if (ref($resetValue) eq 'ARRAY') {
                    my $multiplier = $designDBRef->_getMultiplierFromArrayIndex($fieldIndex,
                                                                                $fieldHash->{'arrayDimensionInfo'});
                    $resetValueObj->set_value($resetValue->[$multiplier]);
                }
            }
            $self->set_resetValueNode($resetValueObj);
        }
        if(exists($fieldHash->{'triggerValue'})) {
            my $triggerValueObj = csrTriggerValue->new($fieldHash->{'triggerValue'});
            $self->set_triggerValueNode($triggerValueObj);
        }
        # set arrayIndex by copying the fieldIndex array
        $self->set_arrayIndex([@{$fieldIndex}]) if(defined($fieldIndex));
        if(defined($fieldHash->{'interruptInfo'})) {
            $intObj = csrInterrupt->new($self, $fieldHash);
            $self->set_interrupt($intObj);
        }
	$self->set_readEffect($fieldHash->{'readEffect'}) if(defined($fieldHash->{'readEffect'}));
        if(defined($fieldHash->{'writeFunction'})) {
            $self->set_writeFunction($fieldHash->{'writeFunction'});
        }
        if(($parentHashRef->{'declaredExternal'} eq 'false') &&
	    defined($fieldHash->{'segmentOf'}) && ($fieldHash->{'fieldType'} eq 'status')) {
            # Add the input port object (for status type field) to the list
            # of ports of this field object. Use the wideRegisterFieldHash node that
            # we calculated above.
	    my $wideRegFieldHash = $self->get_wideRegisterFieldHash();
	    my $portListRef = csrPorts->new($self,
					    $fieldHash,
					    $wideRegFieldHash->{'ports'},
					    $parentHashRef,
					    $expandedName);
            $self->set_ports($portListRef);
        }
	if(exists($fieldHash->{'externalBusPorts'})) {
	    $portListRef = csrPorts->new($self,
					 $fieldHash,
					 $fieldHash->{'externalBusPorts'},
					 $parentHashRef,
					 $expandedName);
	    $self->set_externalBusPorts($portListRef);
	}
        if (defined($fieldHash->{'ports'})) {
            my $portListRef = csrPorts->new($self,
                                            $fieldHash,
                                            $fieldHash->{'ports'},
                                            $parentHashRef,
                                            $expandedName);
            $self->set_ports($portListRef);
	    if ($self->hasPortType('outputPort') &&
		$self->hasPortType('outputClockDomain') &&
		$self->hasOutputSynchronizer()) {
		my $outputPort = $self->getPortType('outputPort');
		my $clockDomain = $self->getPortType('outputClockDomain');
		$outputPort->setOutputClockDomainName($clockDomain->getName());
	    }
        }
        if (($self->getFieldType() eq 'counter') &&
	    (exists($fieldHash->{'incrAmountInfo'}) || $self->hasPortType('incrementCounterPort'))) {
            my $incrCountObj = csrCounter->new($fieldHash, 'increment');
            $self->set_incrementCount($incrCountObj);
        }
        if (($self->getFieldType() eq 'counter') &&
	    (exists($fieldHash->{'decrAmountInfo'}) || $self->hasPortType('decrementCounterPort'))) {
            my $decrCountObj = csrCounter->new($fieldHash, 'decrement');
            $self->set_decrementCount($decrCountObj);
        }
	if ($self->hasResetValue() && !$self->hasResetClass()) {
	    my @resetClass;
	    push (@resetClass, $defaultResetKind);
	    $self->set_resetClass(\@resetClass);
	}
        return $self;
    }

    sub getInputPortWidth {
        my $self = shift;

        my ($portObj, $subRegHashRef);
        my ($fieldHierName, @levelNameList, @tmpLevelNameList, $msb, $lsb);
        my ($subRegName, $wideRegFieldHash, $portWidth, $count, $bogusAddr);
        my (@portType, @portInfo, $portListRef, $wideRegInputPortHash);

        if(defined($self->getSegmentOf())) {
            $fieldHierName = $self->get_hierarchicalIdentifier();
            @levelNameList = split('\.', $fieldHierName);
            if(@levelNameList <= 0) {
                push(@levelNameList, $fieldHierName);
            }
            @tmpLevelNameList = @levelNameList;
            pop(@tmpLevelNameList);
            $subRegName = join('.', @tmpLevelNameList);
            ($subRegHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($subRegName);
            $wideRegFieldHash = $self->get_wideRegisterFieldHash();
            $portListRef = $wideRegFieldHash->{'ports'};
            for($count = 0; $count < @$portListRef; $count++) {
                @portType = keys %{$portListRef->[$count]};
                @portInfo = values %{$portListRef->[$count]};
                if($portType[0] eq 'inputPort') {
                    $wideRegInputPortHash = $portListRef->[$count]->{'inputPort'};
                    last;
                }
            }
            if(defined($self->get_parentFieldIsArray())) {
                if($self->getWidth() < $wideRegInputPortHash->{'width'}) {
                    $msb = $self->get_parentMsb();
                    $lsb = $self->get_parentLsb();
                } else {
                    $msb = $wideRegInputPortHash->{'msb'};
                    $lsb = $wideRegInputPortHash->{'lsb'};
                }
                if($subRegHashRef->{'bitNumbering'} eq 'lsb_0') {
                    $portWidth = $msb - $lsb + 1;
                } else {
                    $portWidth = $lsb - $msb + 1;
                }
            } else {
                $portWidth = $self->getWidth();
            }
            return $portWidth;
        } else {
            $portObj = $self->getPortType('inputPort');
            return $portObj->getWidth();
        }
    }

    sub getWidth {
        my $self = shift;
        return $self->get_width();
    }
    sub getMsb {
        my $self = shift;
        return $self->get_msb();
    }
    sub getLsb {
        my $self = shift;
        return $self->get_lsb();
    }
    sub getMsbLsb {
        my $self = shift;
        return ($self->get_msb(), $self->get_lsb());
    }
    sub hasWritAsRead {
	my $self = shift;
	my $writeAsRead;
	return 1 if (defined($writeAsRead = $self->get_writeAsRead) &&
		     ($writeAsRead eq 'true'));
	return 0;
    }
    sub hasRange {
	my $self = shift;
	return 1 if (defined($self->get_range()));
	return 0;
    }
    sub getRange {
	my $self = shift;
	return $self->get_range();
    }
    sub getRangeWidth {
        my $self = shift;
        return $self->get_rangeWidth();
    }
    sub getRangeMsb {
        my $self = shift;
        return $self->get_rangeMsb();
    }
    sub getRangeLsb {
        my $self = shift;
        return $self->get_rangeLsb();
    }
    sub getRangeMsbLsb {
        my $self = shift;
        return ($self->get_rangeMsb(), $self->get_rangeLsb());
    }
    sub isLowPrioritySWWrite {
	my $self = shift;
	return 1 if ($self->get_lowPrioritySWWrite() eq 'true');
	return 0;
    }
    sub hasTriggerValue {
        my $self = shift;
        return 1 if (defined($self->get_triggerValueNode()));
        return 0;
    }
    sub getTriggerValueNode {
        my $self = shift;
        return $self->get_triggerValueNode();
    }
    sub getTriggerValue {
        my $self = shift;
        my $trigObj = $self->get_triggerValueNode();
        return undef if (!defined($trigObj));
        return $trigObj->getValue();
    }
    sub getTriggerValueSourceType {
        my $self = shift;
        my $trigObj = $self->get_triggerValueNode();
        return undef if (!defined($trigObj));
        return $trigObj->getSourceType();
    }
    sub hasUniqueResetPort {
	my $self = shift;
	return 1 if ($self->get_uniqueResetPort() eq 'true');
	return 0;
    }
    sub isHardwareVolatile {
	my $self = shift;
	return 1 if ($self->get_hardwareVolatile() eq 'true');
	return 0;
    }
    sub isLatching {
	my $self = shift;
	my $latching = $self->getLatching();
	return 1 if ($latching ne 'none');
	return 0;
    }
    sub getLatching {
        my $self = shift;
        return $self->get_latching();
    }
    sub getByteEndian {
        my $self = shift;
        return $self->get_byteEndian();
    }
    sub getDecrementCount {
        my $self = shift;
        return $self->get_decrementCount();
    }
    sub getIncrementCount {
        my $self = shift;
        return $self->get_incrementCount();
    }
    sub getResetTiming {
        my $self = shift;
        return $self->get_resetTiming();
    }
    sub getSegmentOf {
        my $self = shift;
        return $self->get_segmentOf();
    }
    sub getParentWidth {
	my $self = shift;
	return $self->get_parentWidth();
    }
    sub getParentMsbLsb {
        my $self = shift;
        return ($self->get_parentMsb(), $self->get_parentLsb());
    }
    sub hasAliases {
	my $self = shift;
	return 1 if (defined($self->get_aliases()));
	return 0;
    }
    sub getAliases {
	my $self = shift;
	return $self->get_aliases();
    }
    sub isAlias {
	my $self = shift;
	return 1 if (defined($self->get_aliasOf()));
	return 0;
    }
    sub getAliasName {
        my $self = shift;
        return $self->get_aliasName();
    }
    sub isParentAlias {
	my $self = shift;
	return 1 if (defined($self->get_parentAliasOf()));
	return 0;
    }
    sub getAliasHierarchicalIdentifier {
        my $self = shift;
        return $self->get_aliasHierarchicalIdentifier();
    }
    sub getInterruptType {
        my $self = shift;
        my $interruptObj;
        if(defined($interruptObj = $self->get_interrupt())) {
            return $interruptObj->getInterruptType();
        } else {
            my $i=0;
            while (my($pack, $file, $line, $subname, $hasargs, $wantarray) = caller($i++)) {
                print 'In file: ' . $file . ', line \#: ' . $line . ', subroutine name: ' .
                      $subname . "\n";
            }
            my $errMsg = "\tInterrupt node not defined for field: " . $self->getHierarchicalIdentifier();
            $self->_printError($errMsg);
            exit (1);
        }
    }
    sub isCascadedCounter {
	my ($self, $counterPortType) = @_;

	my ($refType);
	if (($counterPortType ne 'incrementCounterPort') &&
	    ($counterPortType ne 'decrementCounterPort')) {
	    my $errMsg = '\tCounter Port Type is: ' . $counterPortType . "\n" .
		'\tMust be one of incrementCounterPort or decrementCounterPort.';
	    $self->_printError($errMsg);
	    exit (1);
	}
 	my $counterPortObj = $self->getPortType($counterPortType);
	return 0 if (!defined($counterPortObj));
	if ($counterPortObj->isSourceTypeReference()) {
	    $refType = $counterPortObj->getReferenceType();
	    if (($refType eq 'overflow') || ($refType eq 'underflow')) {
		return 1;
	    }
	}
	return 0;
    }

    sub isInterruptStateStatus {
	my $self = shift;

	if ($self->getFieldType() eq 'status') {
	    my $inputPortObj = $self->getPortType('inputPort');
	    return 0 if (!defined($inputPortObj));
	    if ($inputPortObj->isSourceTypeReference()) {
		my $refType = $inputPortObj->getReferenceType();
		return 1 if ($refType eq 'interrupt');
	    }
	}
	return 0;
    }

    sub isPortReference {
	my ($self, $portType) = @_;

	my ($portObj);
	if(defined($portObj = $self->getPortType($portType))) {
	    if($portObj->isSourceTypeReference()) {
		return 1;
	    }
	}
	return 0;
    }
    sub hasEnum {
	my $self = shift;
	return 1 if (defined($self->getFieldEnum()));
	return 0;
    }
    sub getFieldEnum {
	my $self = shift;
	return $self->get_fieldEnum();
    }
    sub getEnumIdentifier {
	my $self = shift;
	my $fieldEnumObj = $self->get_fieldEnum();
	return $fieldEnumObj->get_identifier();
    }
    sub hasEnumTitle {
	my $self = shift;
	my $fieldEnumObj = $self->get_fieldEnum();
	return 1 if (defined($fieldEnumObj->get_title()));
	return 0;
    }
    sub getEnumTitle {
	my $self = shift;
	my $fieldEnumObj = $self->get_fieldEnum();
	return $fieldEnumObj->get_title();
    }
    sub hasEnumDescription {
	my $self = shift;
	my $fieldEnumObj = $self->get_fieldEnum();
	return 1 if (defined($fieldEnumObj->get_description()));
	return 0;
    }
    sub getEnumDescription {
	my $self = shift;
	my $fieldEnumObj = $self->get_fieldEnum();
	return $fieldEnumObj->get_description();
    }
    sub hasEnumeratorsHaveTitles {
	my $self = shift;
	my $fieldEnumObj = $self->get_fieldEnum();
	return $fieldEnumObj->hasEnumeratorsHaveTitles();
    }
    sub hasEnumeratorsHaveDescriptions {
	my $self = shift;
	my $fieldEnumObj = $self->get_fieldEnum();
	return $fieldEnumObj->hasEnumeratorsHaveDescriptions();
    }
    sub getEnumItemCount {
	my $self = shift;
	my $fieldEnumObj = $self->getFieldEnum();
	return $fieldEnumObj->getEnumItemCount();
    }
    sub getNextEnumItem {
	my $self = shift;

	# Field enumeration item iterator.

	my $enumItemCount = $self->getEnumItemCount();
	if ($currFieldEnumIndex < ($enumItemCount-1)) {
	    my $fieldEnumObj = $self->getFieldEnum();
	    my $enumItemList = $fieldEnumObj->getEnumItems();
	    return $enumItemList->[++$currFieldEnumIndex];
	} else {
	    $currFieldEnumIndex = -1;
	    return;
	}
    }
    sub getEnumValueList {
	my $self = shift;
	my @valueList;
	while (my $enumItem = $self->getNextEnumItem()) {
	    my $value = $enumItem->getValue();
	    push (@valueList, $value);
	}
	return \@valueList;
    }
    sub getEnumSourceFileName {
	my $self = shift;
	my $enumObj = $self->getFieldEnum();
	my $sourceFile = $enumObj->getSourceFileName();
	return $sourceFile;
    }
    sub getEnumLineNumber {
	my $self = shift;
	my $enumObj = $self->getFieldEnum();
	my $lineNum = $enumObj->getLineNumber();
	return $lineNum;
    }
    sub hasInputSynchronizer {
	my $self = shift;
	return 1 if ($self->get_hasInputSynchronizer() eq 'true');
	return 0;
    }
    sub getSynchronizerStyle {
	my $self = shift;
	return $self->get_synchronizerStyle();
    }
    sub hasSynchronizerStyle {
	my ($self, $syncType) = @_;
	return 1 if ($self->getSynchronizerStyle() eq $syncType);
    }
    sub hasSynchronizerStyleWithDir {
	my ($self, $syncType, $direction) = @_;
	my $hasSync=0;

	if (($direction ne 'output') && ($direction ne 'input')) {
	    $self->_printError('direction must be either input or output');
	    exit(1);
	}
	if ($direction eq 'output') {
	    $hasSync = $self->hasOutputSynchronizer();
	} else {
	    $hasSync = $self->hasinputSynchronizer();
	}
	return 1 if (($self->getSynchronizerStyle() eq $syncType) && $hasSync);
    }
    sub hasOutputSynchronizer {
	my $self = shift;
	return 1 if ($self->get_hasOutputSynchronizer() eq 'true');
	return 0;
    }
    sub hasLogicOpOutputPorts {
	my $self = shift;

	my @outputPortList;
	my $portList = $self->getPorts();
	foreach my $nextPort (@{$portList}) {
	    my $portType = $nextPort->getPortType();
	    if (($portType eq 'andPort') || ($portType eq 'orPort') ||
		($portType eq 'xorPort')) {
		return 1;
	    }
	}
	return 0;
    }
    sub getLogicOpOutputPorts {
	my $self = shift;

	my @outputPortList;
	my $portList = $self->getPorts();
	foreach my $nextPort (@{$portList}) {
	    my $portType = $nextPort->getPortType();
	    if (($portType eq 'andPort') || ($portType eq 'orPort') ||
		($portType eq 'xorPort')) {
		push (@outputPortList, $nextPort);
	    }
	}
	return \@outputPortList;
    }
    sub isOverlap{
	my $self = shift;
	return $self->get_overlapFlag();
    }
    sub getPortReferenceType {
	my ($self, $portType) = @_;

	my ($portObj);
	if(defined($portObj = $self->getPortType($portType))) {
	    return($portObj->getReferenceType()) if($portObj->isSourceTypeReference());
	}
	return;
    }
    sub getPortReferenceNodes {
	my ($self, $portType) = @_;
	my ($regObj, $fieldObj, $portObj, $refType);
	$portObj = $self->getPortType($portType);
	if (defined($portObj)) {
	    ($regObj, $fieldObj) = $portObj->getReferenceInfo();
	}
	return ($regObj, $fieldObj);
    }
    sub getDeclaredExternalObjectPortList {
	my($self, $portDir) = @_;

	return if (!$self->isDeclaredExternal());
	return $self->getExternalPortList($portDir);
    }
    sub getObjectPortsList_NoExternalPorts {
	my($self, $direction) = @_;

	my ($fieldPorts, $fieldExtPorts);
        my ($portType, $portListRef, $portDir, $portObj);

	return if (!defined($self->getPorts()));

	$fieldPorts = $self->getPorts() if (defined($self->getPorts()));
	foreach $portObj (@{$fieldPorts}) {
            $portType = $portObj->getPortType();
	    if (defined($portDir = $portObj->getPortDirection())) {
		if ($portDir eq 'input') {
		    my ($sourceType, $concatPort);
		    if ($portObj->hasConcatFields()) {
			my $concatPortList = $portObj->getConcatList();
			for (my $index=0; $index < @{$concatPortList}; $index++) {
			    $concatPort = $concatPortList->[$index];
			    $sourceType = $concatPort->getSourceType();
			    if (($sourceType eq 'fieldRef') || ($sourceType eq 'registerRef')) {
				splice(@$concatPortList, $index, 1);
			    }
			}
			push(@$portListRef, $portObj) if($direction eq $portDir);
		    } else {
			$sourceType = $portObj->getSourceType();
			if (($sourceType ne 'fieldRef') && ($sourceType ne 'registerRef')) {
			    push(@$portListRef, $portObj) if($direction eq $portDir);
			}
		    }
		} else {
		    push(@$portListRef, $portObj) if($direction eq $portDir);
		}
	    }
	}
        return $portListRef;
    }

    sub getObjectPortsList {
        my($self, $direction) = @_;

	my ($fieldPorts, $fieldExtPorts);
        my ($portType, $portListRef, $portDir, $portObj);

	return if (!defined($self->getPorts()) && !defined($self->getExternalBusPorts()));

	$fieldPorts = $self->getPorts() if (defined($self->getPorts()));
	$fieldExtPorts = $self->getExternalBusPorts();
	if (defined($fieldExtPorts)) {
	    push (@{$fieldPorts}, @{$fieldExtPorts});
	}

	foreach $portObj (@{$fieldPorts}) {
            $portType = $portObj->getPortType();
            if($self->isExternal()) {
                # For interface ports, we're only interested in those ports for
                # which bus transactions are defined.
                next if(($portType ne 'externalReadAccessPort') &&
                        ($portType ne 'externalReadDataPort') &&
                        ($portType ne 'externalWriteDataPort') &&
                        ($portType ne 'externalWriteAccessPort') &&
                        ($portType ne 'externalEnablePort') &&
                        ($portType ne 'externalSelectPort') &&
                        ($portType ne 'externalReadClearPort') &&
                        ($portType ne 'externalReadSetPort') &&
                        ($portType ne 'externalReadModifyPort') &&
                        ($portType ne 'externalWaitPort'));
            }
	    if (defined($portDir = $portObj->getPortDirection())) {
		if ($portDir eq 'input') {
		    my ($sourceType, $concatPort);
		    if ($portObj->hasConcatFields()) {
			my $concatPortList = $portObj->getConcatList();
			for (my $index=0; $index < @{$concatPortList};) {
			    $concatPort = $concatPortList->[$index];
			    $sourceType = $concatPort->getSourceType();
			    if (($sourceType eq 'fieldRef') || ($sourceType eq 'registerRef') ||
				($sourceType eq 'number')) {
				my $tmpPort = splice(@$concatPortList, $index, 1);
			    } else {
				$index++;
			    }
			}
			push(@$portListRef, @$concatPortList) if (($direction eq $portDir) &&
								  (@$concatPortList > 0));
		    } else {
			$sourceType = $portObj->getSourceType();
			if (($sourceType ne 'fieldRef') && ($sourceType ne 'registerRef')) {
			    push(@$portListRef, $portObj) if($direction eq $portDir);
			}
		    }
		} else {
		    push(@$portListRef, $portObj) if($direction eq $portDir);
		}
	    }
	}
        return $portListRef;
    }
    sub getPorts {
        my $self = shift;
        return($self->get_ports());
    }
    sub getPortList {
        my($self, $direction) = @_;

        my ($portType, @portList, $portDir);
        my $portListRef = $self->getPorts();
        return \@portList if(!defined($portListRef));
	my $currPortIndex = 0;
	while (my $portObj = $self->getNextPortNode($portListRef,
						    \$currPortIndex)) {
            if(defined($portDir = $portObj->getPortDirection())) {
                push(@portList, $portObj) if($direction eq $portDir);
            }
	    $currPortIndex++;
        }
        return \@portList;
    }
    sub getExternalBusPorts {
        my $self = shift;
        return $self->get_externalBusPorts();
    }
    sub hasReturnToZero {
        my $self = shift;
        my $rtz;
        if (defined($rtz = $self->get_returnToZero()) && ($rtz eq 'true')) {
            return 1;
        }
        return 0;
    }
    sub hasSharedPort {
	my ($self, $portType) = @_;
	my $portObj = $self->getPortType($portType);
	return 1 if (defined($portObj) && ($portObj->isSharedPort()));
	return 0;
    }
    sub hasReadEffect {
        my $self = shift;

        my $readEffect = $self->getReadEffect();
        return 1 if(($readEffect eq 'clear') || ($readEffect eq 'set'));
        return 0;
    }
    sub getReadEffect {
        my $self = shift;
        return $self->get_readEffect();
    }
    sub hasWriteFunction {
        my $self = shift;

        my $writeFunction = $self->getWriteFunction();
        return 1 if ($writeFunction ne 'write');
        return 0;
    }
    sub getWriteFunction {
        my $self = shift;
        return $self->get_writeFunction();
    }
    sub getResetValueNode {
        my $self = shift;
        return $self->get_resetValueNode();
    }
    sub getFieldType {
        my $self = shift;
        return $self->get_fieldType();
    }
    sub hasResetValue {
        my $self = shift;
        return 1 if(defined($self->getResetValueNode()));
        return 0;
    }
    sub getResetSourceType {
	my $self = shift;
	my $resetObj = $self->getResetValueNode();
	if (defined($resetObj)) {
	    return $resetObj->getSourceType();
	} else {
	    my $errMsg = 'Field: ' . $self->getHierarchicalIdentifier() . ' has no reset node.';
	    $self->_printError($errMsg);
	    exit(1);
	}
    }
    sub getResetValue {
        my $self = shift;

        my ($value, $verilogFormat, $bigIntReset, $resetInputPortValue);

        my $resetObj = $self->getResetValueNode();
	return if (!defined($resetObj));

        my $resetValueType = $resetObj->getSourceType();
        if ($resetValueType eq 'constant') {
            ($verilogFormat, $value) = $self->getVerilogBaseAndValue($resetObj->getValue());
            $bigIntReset = Math::BigInt->new($value);
            return $bigIntReset;
        } elsif ($resetValueType eq 'fieldRef') {
	    foreach my $resetValueSourceObj (@{$self->getPorts()}) {
		if ($resetValueSourceObj->getPortType() eq 'resetValueSourcePort') {
		    my($resetRegObj, $resetFieldObj) = $resetValueSourceObj->getReferenceInfo();
		    $bigIntReset = $resetFieldObj->getResetValue();
		    return $bigIntReset;
		}
	    }
        } elsif ($resetValueType eq 'inputPort') {
	    my $i=0;
	    while (my($pack, $file, $line, $subname, $hasargs, $wantarray) = caller($i++)) {
		print 'In file: ' . $file . ', line \#: ' . $line . ', subroutine name: ' . $subname . "\n";
	    }
	    my $errMsg = "\n\tReset source type of field: " . $self->getHierarchicalIdentifier() .
		" is 'inputPort';\n\treset value is unknown."; 
	    $self->_printError($errMsg);
	    exit(1);
        }
    }

    sub hasResetClass {
	my $self = shift;
	my $rstClass;
	return 1 if (defined($rstClass = $self->get_resetClass()) &&
		     ($rstClass->[0] ne ""));
	return 0;
    }
    sub getResetClass {
	my $self = shift;
	return $self->get_resetClass() if ($self->hasResetClass());
	return;
    }
    sub hasResetClassType {
	my ($self, $rstClassType) = @_;
	my ($fieldRstClass, $myType);

	if (defined($fieldRstClass = $self->get_resetClass())) {
	    foreach my $nextClassName (@{$fieldRstClass}) {
		return 1 if ($nextClassName eq $rstClassType);
	    }
	}
	return 0;
    }
    sub isFreeRunningCounter {
	my $self = shift;

	if (($self->getFieldType() eq 'counter') &&
	    !$self->hasPortType('incrementCounterPort') &&
	    !$self->hasPortType('decrementCounterPort')) {
	    return 1;
	}
	return 0;
    }

    sub getCounterObject {
        my ($self, $counterType) = @_;

        my $countObj;
        if($counterType eq 'increment') {
            $countObj = $self->getIncrementCount();
        } else {
            $countObj = $self->getDecrementCount();
        }
        return $countObj;
    }
    sub getCounterType {
        my $self = shift;
	return 'updown' if (defined($self->getIncrementCount()) &&
			    defined($self->getDecrementCount()));
	return 'increment' if (defined($self->getIncrementCount()));
        return 'decrement' if (defined($self->getDecrementCount()));
	return 'none';
    }
    sub getCountAmountType {
        my ($self, $counterType) = @_;
        my $counterObj = $self->getCounterObject($counterType);
        return $counterObj->getCountAmountType();
    }
    sub getCountAmount {
        my ($self, $counterType) = @_;
        my $counterObj = $self->getCounterObject($counterType);
        return $counterObj->getCountAmount();
    }

    sub hasThreshold {
        my ($self, $counterType) = @_;

        my $counterObj;
        if ($counterType eq 'any') {
            if (defined($counterObj = $self->getCounterObject('increment'))) {
                return $counterObj->hasThreshold();
            }
            if(defined($counterObj = $self->getCounterObject('decrement'))) {
                return $counterObj->hasThreshold();
            }
            return 0;
        } else {
            $counterObj = $self->getCounterObject($counterType);
            return $counterObj->hasThreshold();
        }
    }
    sub getThresholdType {
        my ($self, $counterType) = @_;
        my $counterObj = $self->getCounterObject($counterType);
        return $counterObj->getThresholdType();
    }
    sub getSaturatingType {
        my ($self, $counterType) = @_;
        my $counterObj = $self->getCounterObject($counterType);
        return $counterObj->getSaturatingType();
    }

    sub getThresholdValue {
        my ($self, $counterType) = @_;
        my $counterObj = $self->getCounterObject($counterType);
        return $counterObj->getThresholdValue();
    }

    sub hasSaturating {
        my ($self, $counterType) = @_;
        my $counterObj = $self->getCounterObject($counterType);
        return $counterObj->hasSaturating();
    }

    sub getSaturateValue {
        my ($self, $counterType) = @_;
        my $counterObj = $self->getCounterObject($counterType);
        return $counterObj->getSaturateValue();
    }
    sub hasPorts {
	my $self = shift;
	return 1 if (defined($self->getPorts()));
	return 0;
    }
    sub hasPortType {
        my($self, $portType) = @_;

	my $portList;
	if ($portType =~ /external/) {
	    $portList = $self->getExternalBusPorts();
	} else {
	    $portList = $self->getPorts();
	}
        return 0 if(!defined($portList));
        foreach my $portObj (@{$portList}) {
            if($portObj->getPortType() eq $portType) {
                return 1;
            }
        }
        return 0;
    }
    sub getPortType {
        my($self, $portType) = @_;

	if (!defined($portType)) {
	    $self->_printError('Port type not defined');
	    exit(1);
	}
	my $portList;
	if ($portType =~ /external/) {
	    $portList = $self->getExternalBusPorts();
	} else {
	    $portList = $self->getPorts();
	}
        return if(!defined($portList));
        foreach my $portObj (@{$portList}) {
            if($portObj->getPortType() eq $portType) {
                return $portObj;
            }
        }
        return;
    }
    sub hasMinimumValue {
        my $self = shift;
        if(defined($self->get_minimumValue())) {
            return 1;
        }
        return 0;
    }
    sub getMinimumValue {
        my $self = shift;
        my $min;
        if($self->hasMinimumValue()) {
            return $self->get_minimumValue();
        }
        return $min;
    }
    sub hasMaximumValue {
        my $self = shift;
        if(defined($self->get_maximumValue())) {
            return 1;
        }
        return 0;
    }
    sub getMaximumValue {
        my $self = shift;
        my $max;
        if($self->hasMaximumValue()) {
            return $self->get_maximumValue();
        }
        return $max;
    }

    sub getParentNode {
        my $self = shift;

        # Get the parent object of the calling object.
        # can be called by:
        # 1 - a field object of a register to obtain
        #     the parent register object.

        my ($relAddr);
        my ($aliasPrefixList, $parentObject, $bogusAddr, $topLevelHashRef);
        my ($regHashRef, $parentName, $parentHashRef, $parentOfParentHashRef);
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage);
        my $bigIntAddr = Math::BigInt->new('0');
        my $hierName = $self->get_hierarchicalNameDotted();
        my @levelNameList = split('\.', $self->get_hierarchicalNameDotted());
        my @hierLevelNameList = split('\.', $self->get_hierarchicalNameDotted());

        if(@levelNameList <= 0) {
            push(@levelNameList, $self->getHierarchicalIdentifier());
        }
        my @tmpLevelNameList = @levelNameList;

        ($topLevelHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);
        if(@levelNameList > 2) {
            # more than one level of hierarchy
            pop(@tmpLevelNameList);
            $parentName = join('.', @tmpLevelNameList);
            ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
            if(@levelNameList > 3) {
                # get to the parent of parent node
                pop(@tmpLevelNameList);
                $parentName = join('.', @tmpLevelNameList);
                ($parentOfParentHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($parentName);
            } else {
                $parentOfParentHashRef = $topLevelHashRef;
            }
        } else {
            $parentHashRef = $topLevelHashRef;
            # there is no parent of parent node.
        }
        ($busProtocol, $inputPipelineStage, $outputPipelineStage) =
            $designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);

        my $arrayIndex = $self->getParentArrayIndexFromHierId($hierName);
        pop(@levelNameList); # get to the parent
        pop(@hierLevelNameList);
        if(defined($parentHashRef)) {
            if(defined($parentHashRef->{'aliasOf'})) {
		$aliasPrefixList = $self->expandAliasArrayMembers($parentHashRef,
								  \@hierLevelNameList);
	    }
            my ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($parentHashRef->{'byteOffset'});
            ($verilogFormat, $relAddr) =
		$self->getVerilogBaseAndValue($parentHashRef->{'addressmapRelativeByteAddress'});
	    my $relAddrBigInt = Math::BigInt->new($relAddr);
	    $updateRefArrayIndex = 0;
            if (($viewName eq 'sv') && (@$arrayIndex)) {
                my $tmpIndex;
                for (my $count=0; $count < @$arrayIndex; $count++) {
                    $tmpIndex .= '[' . $arrayIndex->[$count] . ']';
                }
                $levelNameList[@levelNameList-1] .= $tmpIndex;
                if (defined($aliasPrefixList)) {
                    $aliasPrefixList->[@$aliasPrefixList-1] .= $tmpIndex;
                }
                $hierLevelNameList[@hierLevelNameList-1] .= $tmpIndex;
            }
            $parentObject = csrPerlAPIRegister->new($parentHashRef,
						    $arrayIndex,
                                                    $parentOfParentHashRef,
                                                    $bigIntAddr,
                                                    Math::BigInt->new($offset),
                                                    $relAddrBigInt,
                                                    $busProtocol,
                                                    $inputPipelineStage,
                                                    $outputPipelineStage,
                                                    \@levelNameList,
                                                    $aliasPrefixList,
						    \@hierLevelNameList);
            return ($parentObject);
        }

    }

    sub getParentWideFieldObject {
        my $self = shift;

        # Given a field in a sub-register, find the parent
        # node in the wide register.

        my (@levelNameList, $fieldIndex);
        my $parentSubRegObj = $self->getParentNode();
        my $parentWideRegObj = $parentSubRegObj->getParentNode();
        my $wideFieldHashRef = $self->get_wideRegisterFieldHash();
        push (@levelNameList, ($parentWideRegObj->getName(), $self->getSegmentOf()));
        my $prefixName = join('_', $parentWideRegObj->getAddressmapRelativeName, $self->getSegmentOf());
        my $wideFieldObj = csrPerlAPIField->new($wideFieldHashRef,
                                                $prefixName,
                                                \@levelNameList,
                                                $wideFieldHashRef->{'lsb'},
                                                $wideFieldHashRef->{'msb'},
#                                                0,
                                                $fieldIndex,
                                                $parentWideRegObj->get_parentHashRef());
        return $wideFieldObj;
    }
}

package csrParameterBase;

@csrParameterBase::ISA = qw(csrPerlAPIBase);

use strict;
use vars '$AUTOLOAD';

{
    my %parameterAttrs = (_name => '',
			  _isOverridden => '',
			  _defaultValue => '',
			  _value => '',
			  );

    sub _defaultAttrKeys {
	my $self = shift;
	return ($self->SUPER::_defaultAttrKeys(), map { s/^_//; $_ } keys %parameterAttrs);
    }
    sub new {
	my ($caller, $parameterHash) = @_;
	my $self = $_[0]->SUPER::new($parameterHash);
	return $self;
    }
    sub getName {
	my $self = shift;
	return $self->get_name();
    }
    sub getValue {
	my $self = shift;
	return $self->get_value();
    }
    sub getDefaultValue {
	my $self = shift;
	return $self->get_defaultValue();
    }
    sub isOverridden {
	my $self = shift;
	return $self->get_isOverridden();
    }
}

package csrParameters;

@csrParameters::ISA = qw(csrParameterBase);

use strict;
use vars '$AUTOLOAD';

{
    sub new {
	my ($caller, $parameterList) = @_;

	my @paramList;
	for (my $index=0; $index < @{$parameterList}; $index++) {
	    my $nextParam = csrParameters->SUPER::new($parameterList->[$index]);
	    push(@paramList, $nextParam);
	}
	return \@paramList;
    }
}


package csrFieldEnum;

@csrFieldEnum::ISA = qw(csrPerlAPIBase);

use strict;
use vars '$AUTOLOAD';

{
    my %fieldEnumAttrs = (_identifier => '',
			  _enumeratorsHaveTitles => '',
			  _enumeratorsHaveDescriptions => '',
			  _enumItems => '',
			  _sourceFilename => '',
			  _lineNumber => '',
			  );

    sub _defaultAttrKeys {
	my $self = shift;
	return ($self->SUPER::_defaultAttrKeys(), map { s/^_//; $_ } keys %fieldEnumAttrs);
    }

    sub new {
	my ($caller, $fieldEnumHash) = @_;

	my ($enumItemObj, @enumItemList);

        my $self = $_[0]->SUPER::new($fieldEnumHash);
	foreach my $nextItem (@{$fieldEnumHash->{'enumItems'}}) {
	    $enumItemObj = csrFieldEnumItem->new($nextItem);
	    push (@enumItemList, $enumItemObj);
	}
	$self->set_enumItems(\@enumItemList);
	return $self;
    }
    sub hasEnumeratorsHaveTitles {
	my $self = shift;
	return 1 if ($self->get_enumeratorsHaveTitles() eq 'true');
	return 0;
    }
    sub hasEnumeratorsHaveDescriptions {
	my $self = shift;
	return 1 if ($self->get_enumeratorsHaveDescriptions() eq 'true');
	return 0;
    }
    sub getEnumItemCount {
	my $self = shift;
	return @{$self->get_enumItems()};
    }
    sub getEnumItems {
	my $self = shift;
	return $self->get_enumItems;
    }
    sub getSourceFileName {
	my $self = shift;
	return $self->get_sourceFilename();
    }
    sub getLineNumber {
	my $self = shift;
	return $self->get_lineNumber();
    }
}

package csrFieldEnumItem;

@csrFieldEnumItem::ISA = qw(csrPerlAPIBase);

use strict;
use vars '$AUTOLOAD';

{
    my %fieldEnumItemAttrs = (_identifier => '',
			      _value => '',
			      _readAccess => '',
			      _writeAccess => '',
			      );

    sub _defaultAttrKeys {
	my $self = shift;
	return ($self->SUPER::_defaultAttrKeys(), map { s/^_//; $_ } keys %fieldEnumItemAttrs);
    }

    sub new {
	my ($caller, $enumItemHash) = @_;
	my $self = $_[0]->SUPER::new($enumItemHash);
	return $self;
    }
    sub getValue {
	my $self = shift;
	return $self->get_value();
    }
    sub hasReadAccess {
	my $self = shift;
	return 1 if ($self->get_readAccess() eq 'true');
	return 0;
    }
    sub hasWriteAccess {
	my $self = shift;
	return 1 if ($self->get_writeAccess() eq 'true');
	return 0;
    }
}

package csrInterrupt;

@csrInterrupt::ISA = qw(csrPerlAPIBase);

use strict;
use vars '$AUTOLOAD';

{
    my %interruptAttrs = (_interruptType => '',
			  _hierarchicalName => '',
			  _topLevelAddressmapName => '',
			  );

    sub new {
        my($caller, $parentNode, $parentHashRef) = @_;
        my $class = ref($caller) || $caller;
        my $self = bless {}, $class;
        my $interruptHash = $parentHashRef->{'interruptInfo'};
        $self->set_interruptType($interruptHash->{'interruptType'});
        $self->set_topLevelAddressmapName($interruptHash->{'topLevelAddressmapName'});
        $self->set_parentHashRef($parentHashRef);
        if(defined($interruptHash->{'hierarchicalRefName'})) {
            $self->setHierarchicalRefName($interruptHash->{'hierarchicalRefName'}, $parentNode);
        }
        return $self;
    }

    sub getInterruptType {
        my $self = shift;
        return $self->get_interruptType();
    }
    sub getHierarchicalName {
        my $self = shift;
        return $self->get_hierarchicalName();
    }
    sub getHierarchicalIdentifier {
        my $self = shift;
        return $self->get_hierarchicalNameDotted();
    }
}


package csrCounter;

@csrCounter::ISA = qw(csrPerlAPIBase);

use strict;
use vars '$AUTOLOAD';

{
    my %counterAttrs = (_amountType => '',
			_amountValue => '',         # increment/decrement amount
			_thresholdType => '',
			_thresholdValue => '',
			_saturatingType => '',
			_saturateValue => '',
			);
}

sub new {
    my($caller, $fieldHash, $flag) = @_;
    my $class = ref($caller) || $caller;
    my $self = bless {}, $class;
    if($flag eq 'increment') {
	if(exists($fieldHash->{'incrAmountInfo'})) {
	    $self->set_amountType($fieldHash->{'incrAmountInfo'}->{'sourceType'});
	    if(exists($fieldHash->{'incrAmountInfo'}->{'value'})) {
		$self->set_amountValue($fieldHash->{'incrAmountInfo'}->{'value'});
	    }
	} else {
	    $self->set_amountType('constant');
	    $self->set_amountValue('1');
	}
	if(exists($fieldHash->{'incrThresholdInfo'})) {
	    $self->set_thresholdType($fieldHash->{'incrThresholdInfo'}->{'sourceType'});
	    if(exists($fieldHash->{'incrThresholdInfo'}->{'value'})) {
		$self->set_thresholdValue($fieldHash->{'incrThresholdInfo'}->{'value'});
	    }
	}
	if(exists($fieldHash->{'incrSaturatingInfo'})) {
	    $self->set_saturatingType($fieldHash->{'incrSaturatingInfo'}->{'sourceType'});
	    if(exists($fieldHash->{'incrSaturatingInfo'}->{'value'})) {
		$self->set_saturateValue($fieldHash->{'incrSaturatingInfo'}->{'value'});
	    }
	}
    } else {
	if(exists($fieldHash->{'decrAmountInfo'})) {
	    $self->set_amountType($fieldHash->{'decrAmountInfo'}->{'sourceType'});
	    if (exists($fieldHash->{'decrAmountInfo'}->{'value'})) {
		$self->set_amountValue($fieldHash->{'decrAmountInfo'}->{'value'});
	    }
	} else {
	    $self->set_amountType('constant');
	    $self->set_amountValue('1');
	}
	if(exists($fieldHash->{'decrThresholdInfo'})) {
	    $self->set_thresholdType($fieldHash->{'decrThresholdInfo'}->{'sourceType'});
	    if(exists($fieldHash->{'decrThresholdInfo'}->{'value'})) {
		$self->set_thresholdValue($fieldHash->{'decrThresholdInfo'}->{'value'});
	    }
	}
	if(exists($fieldHash->{'decrSaturatingInfo'})) {
	    $self->set_saturatingType($fieldHash->{'decrSaturatingInfo'}->{'sourceType'});
	    if(exists($fieldHash->{'decrSaturatingInfo'}->{'value'})) {
		$self->set_saturateValue($fieldHash->{'decrSaturatingInfo'}->{'value'});
	    }
	}
    }
    return $self;
}

sub getCountAmountType {
    my $self = shift;
    return $self->get_amountType();
}
sub getCountAmount {
    my $self = shift;
    return $self->get_amountValue();
}
sub hasThreshold {
    my $self = shift;
    return 0 if(!defined($self->get_thresholdType()));
    return 1;
}
sub getThresholdType {
    my $self = shift;
    return $self->get_thresholdType();
}
sub getThresholdValue {
    my $self = shift;
    my ($verilogFormat, $value) = $self->getVerilogBaseAndValue($self->get_thresholdValue());
    return Math::BigInt->new($value);
}
sub hasSaturating {
    my $self = shift;
    return 0 if(!defined($self->get_saturatingType()));
    return 1;
}
sub getSaturatingType {
    my $self = shift;
    return $self->get_saturatingType();
}
sub getSaturateValue {
    my $self = shift;
    my ($verilogFormat, $value) = $self->getVerilogBaseAndValue($self->get_saturateValue());
    return Math::BigInt->new($value);
}

package csrPortsBase;

use strict;
use vars '$AUTOLOAD';

@csrPortsBase::ISA = qw(csrPerlAPIBase);

{
    my %basePortAttrs = (_portType => '',
			 _width => '',
			 _lsb => '',
			 _msb => '',
			 _range => '',
			 _rangeLsb => '',
			 _rangeMsb => '',
			 _rangeWidth => '',
			 _constantValue => '',
			 _declaredPortName => '',
			 _registerPortFlag => '',
			 _flop => '',
			 _activeLow => '',
			 _sourceType => '',
			 _hierarchicalName => '',
			 _hierarchicalIdentifier => '',
			 _hierarchicalRefName => '',
			 _topLevelAddressmapName => '',
			 _referenceType => '',
			 _resetTiming => 'synchronous',
			 _parentHashRef => '',
			 _sharedPort => '',
			 _inputPortNamePrefix => '',
			 _inputPortNameSuffix => '',
			 _outputPortNamePrefix => '',
			 _outputPortNameSuffix => '',
			 _portDirection => '',
			 _bitNumbering   => '',
			 );
    sub _defaultAttrKeys {
	my $self = shift;
	return ($self->SUPER::_defaultAttrKeys(), map { s/^_//; $_ } keys %basePortAttrs);
    }

    sub new {
        my($caller,
	   $portInfo) = @_;
	my $portObj = $_[0]->SUPER::new($portInfo);
	return $portObj;
    }


    sub clone {
        my $self = shift;
        my $portObj = bless { %{$self} }, ref($self);
        return $portObj;
    }
    sub getPortType {
        my $self = shift;
        return $self->get_portType();
    }
    sub getPortDirection {
	my $self = shift;
	return $self->get_portDirection();
    }
    sub hasOutputClockDomain {
	my $self = shift;
	return 1 if (defined($self->get_outputClockDomainName()));
	return 0;
    }
    sub setOutputClockDomainName {
	my ($self, $domainName) = @_;
	return $self->set_outputClockDomainName($domainName);
    }
    sub getOutputClockDomainName {
	my $self = shift;
	return $self->get_outputClockDomainName();
    }
    sub hasSourceType {
        my $self = shift;
        return 1 if (defined($self->get_sourceType()));
	return 0;
    }
    sub getSourceType {
        my ($self, $bitIndex) = @_;

	if ($self->checkConcatItemIndex($bitIndex)) {
	    if ($self->hasConcatFields()) {
		my ($concatItem, $tmpIndex) = $self->getConcatItem($bitIndex);
		return $concatItem->getSourceType();
	    } else {
		return $self->get_sourceType();
	    }
	}
    }
    sub isSourceTypeFieldReference {
	my $self = shift;
	my $sourceType = $self->getSourceType(0);
	return 1 if (defined($sourceType) && ($sourceType eq 'fieldRef'));
	return 0;
    }
    sub isSourceTypeReference {
	my $self = shift;
	my $sourceType = $self->getSourceType(0);
	return 1 if (($sourceType eq 'fieldRef') || ($sourceType eq 'registerRef'));
	return 0;
    }
    sub getReferenceType {
        my ($self, $bitIndex) = @_;
	if ($self->checkConcatItemIndex($bitIndex)) {
	    if ($self->hasConcatFields()) {
		my $concatPortList = $self->getConcatList();
		my ($concatItem, $tmpIndex) = $self->getConcatItem($bitIndex);
		return $concatItem->getReferenceType();
	    } else {
		return $self->get_referenceType();
	    }
	}
    }
    sub getResetTiming {
        my $self = shift;
        return $self->get_resetTiming();
    }
    sub getConstantValue {
	my $self = shift;
	return $self->get_constantValue();
    }
    sub getWidth {
        my $self = shift;
        return $self->get_width();
    }
    sub getMsb {
        my $self = shift;
        return $self->get_msb();
    }
    sub getLsb {
        my $self = shift;
        return $self->get_lsb();
    }
    sub getMsbLsb {
        my ($self, $bitIndex) = @_;
	if ($self->checkConcatItemIndex($bitIndex)) {
	    if (defined(my $concatPortList = $self->getConcatList())) {
		return ($concatPortList->[$bitIndex]->get_msb(), $concatPortList->[$bitIndex]->get_lsb());
	    } else {
		return ($self->get_msb(), $self->get_lsb());
	    }
	}
    }
    sub hasRange {
	my $self = shift;
	return 1 if (defined($self->get_range()));
	return 0;
    }
    sub getRange {
	my $self = shift;
	return $self->get_range();
    }
    sub getRangeWidth {
        my $self = shift;
        return $self->get_rangeWidth();
    }
    sub getRangeMsb {
        my $self = shift;
        return $self->get_rangeMsb();
    }
    sub getRangeLsb {
        my $self = shift;
        return $self->get_rangeLsb();
    }
    sub getRangeMsbLsb {
        my ($self, $bitIndex) = @_;

	if ($self->checkConcatItemIndex($bitIndex)) {
	    if (defined(my $concatPortList = $self->getConcatList())) {
		return ($concatPortList->[$bitIndex]->getRangeMsb(), $concatPortList->[$bitIndex]->getRangeLsb());
	    } else {
		return ($self->getRangeMsb(), $self->getRangeLsb());
	    }
	}
    }
    sub isSharedPort {
	my $self = shift;
	return 1 if ($self->getSharedPort() eq 'true');
	return 0;
    }
    sub getSharedPort {
        my $self = shift;
        return $self->get_sharedPort();
    }
    sub isDeclaredPortName {
	my $self = shift;
	my $declared;
	return 1 if (defined($declared = $self->get_declaredPortName()) &&
		     ($declared eq 'true'));
	return 0;
    }
    sub getDeclaredPortName {
	my $self = shift;
	return $self->get_declaredPortName();
    }
    sub getInactiveEdge {
        my ($self, $bitIndex) = @_;

        my $activeLow;
        if (!defined($activeLow = $self->get_activeLow())) {
            $self->_printError('Active edge property missing for clock port: ' . $self->getName());
            exit(1);
        }
        my $value = ($activeLow eq 'true') ? 'posedge' : 'negedge';
        return $value;
    }
    sub getActiveEdge {
        my $self = shift;

        my $activeLow;
        if (!defined($activeLow = $self->get_activeLow())) {
            my $i=0;
            while (my($pack, $file, $line, $subname, $hasargs, $wantarray) = caller($i++)) {
                print 'In file: ' . $file . ', line \#: ' . $line . ', subroutine name: ' .
                      $subname . "\n";
            }
            $self->_printError('Active edge property missing for clock port: ' . $self->getName());
            exit(1);
        }
        my $value = ($activeLow eq 'true') ? 'negedge' : 'posedge';
        return $value;
    }

    sub isActiveLow {
        my ($self, $bitIndex) = @_;

        my ($activeLow);
	if (defined($bitIndex)) {
	    if ($self->checkConcatItemIndex($bitIndex)) {
		if (defined(my $concatPortList = $self->getConcatList())) {
		    if (!defined($activeLow = $concatPortList->[$bitIndex]->get_activeLow())) {
			$self->_printError('Active low property missing for port: ' . $self->get_name());
			exit(1);
		    }
		} else {
		    if (!defined($activeLow = $self->get_activeLow())) {
			$self->_printError('Active low property missing for port: ' . $self->get_name());
			exit(1);
		    }
		}
	    }
	} else {
	    $activeLow = $self->get_activeLow();
	}
	return 1 if ($activeLow eq 'true');
	return 0;
    }
    sub getActiveLevel {
        my $self = shift;

        my ($activeLevel);
        if (!defined($activeLevel = $self->get_activeLow())) {
            $self->_printError('Active low property missing for port: ' . $self->getName());
            exit(1);
        }
	return $activeLevel;
    }
    sub getHierarchicalRefName {
	my $self = shift;
	return $self->get_hierarchicalRefName();
    }
    sub getParentNode {
        my $self = shift;

        my ($verilogFormat, $arrayIndex);
        my ($topLevelHashRef, @aliasPrefixList, $parentHashRef, $relAddr);
        my ($regName, $parentObj, $regHashRef, $fieldHashRef, $parentName);
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage);

        my $bigIntAddr = Math::BigInt->new('0');
        my $hierId = $self->get_parentHierarchicalName();
        my @levelNameList = split('\.', $self->get_parentHierarchicalName());
        my @tmpLevelNameList = @levelNameList;
        my $nodeType = ($self->get_parentHashRef())->{'nodeType'};
        if ($viewName eq 'sv') {
            $arrayIndex = $self->getObjIndexesSV($hierId);
        }
        if($nodeType eq 'field') {
            # First, get the register hash ref.
            pop(@tmpLevelNameList);
            ($regHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($tmpLevelNameList[@tmpLevelNameList-1]);
            # Then, get the field hash ref.
            $fieldHashRef = $self->get_parentHashRef();
            $parentObj = csrPerlAPIField->new($fieldHashRef,
                                              $fieldHashRef->{'name'}, #expandedName
                                              \@levelNameList,         #prefixList
                                              $fieldHashRef->{'lsb'},
                                              $fieldHashRef->{'msb'},
                                              $arrayIndex,              #fieldIndex
                                              $regHashRef);
        } elsif($nodeType eq 'register') {
            ($topLevelHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);
            if(@levelNameList > 2) {
                # more than one level of hierarchy
                pop(@tmpLevelNameList);
                $parentName = join('.', @tmpLevelNameList);
                ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
            } else {
                $parentHashRef = $topLevelHashRef;
            }
            ($busProtocol, $inputPipelineStage, $outputPipelineStage) =
                $designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);
            $regName = $self->get_parentHierarchicalName();
            ($regHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($regName);
            my $offset = $self->getVerilogBaseAndValue($regHashRef->{'byteOffset'});
            ($verilogFormat, $relAddr) =
		$self->getVerilogBaseAndValue($regHashRef->{'addressmapRelativeByteAddress'});
	    my $relAddrBigInt = Math::BigInt->new($relAddr);
	    $updateRefArrayIndex = 0;
            $parentObj = csrPerlAPIRegister->new($regHashRef,
						 $arrayIndex,
                                                 $parentHashRef,
                                                 $bigIntAddr,
                                                 Math::BigInt->new($offset),
                                                 $relAddrBigInt,
                                                 $busProtocol,
                                                 $inputPipelineStage,
                                                 $outputPipelineStage,
                                                 \@levelNameList,
                                                 \@aliasPrefixList,
						 \@levelNameList);
        }
        return $parentObj;
    }

    sub setPortName {
        my($self,
           $referentHash,
           $parentOfReferentHash,
           $referentObj,
           $portName,
           $portType,
           $portInfo,
           $expandedName) = @_;

	my ($sharedPortName, $hierId, $referentHierId, $referentName);
        my ($widePortObj, $parentAddrmap, $widePortInfo, $sharedPorts);
        my ($bogusAddr, $parentFieldHashRef, $aliasName, $wideRegObj);
        my ($wideRegName, @levelNameList, @tmpLevelNameList, $tmpName);
        my ($fieldHash, $wideRegHashRef, $tmpPortName, $wideFieldHash);

	if (defined($referentHash->{'aliasOf'})) {
	    $hierId = $referentObj->getAliasHierarchicalIdentifier();
	    $referentHierId = $referentObj->getAliasHierarchicalIdentifier();
	    $referentName = $referentObj->getAliasName();
	    $expandedName = $referentObj->getAliasName();
	} else {
	    $hierId = $referentObj->getHierarchicalIdentifier();
	    $referentHierId = $referentObj->getHierarchicalIdentifier();
	    $referentName = $referentObj->getName();
	}
	$hierId =~ s/\[/_/g;
	$hierId =~ s/\]//g;
        @levelNameList = split('\.', $hierId);

	if ($self->isDeclaredPortName() || $self->isSharedPort()) {
	    if ($self->isSharedPort()) {
                my $portAddrmapRelId = $self->getAddressmapRelativeIdentifier();
		$tmpPortName = $self->getSharedPortName($referentHash,
                                                        $referentObj,
                                                        $parentOfReferentHash,
                                                        $portInfo,
                                                        $portName,
                                                        $portAddrmapRelId,
                                                        \@levelNameList,
                                                        $expandedName);
	    } else {
		$tmpPortName = $portName;
	    }
	} elsif ($referentHash->{'nodeType'} eq 'field') {
            $fieldHash = $referentHash;
	    if($portName eq '') {
                if(defined($fieldHash->{'wideRegisterName'})) {
                    # construct the wide register name from the field hierarchical name.
                    @tmpLevelNameList = @levelNameList;
                    # pop the top two names (field name and sub-register name)
                    # to get the wide register name. Also, shift out the name of
		    # the address map.
		    shift @tmpLevelNameList;
                    pop(@tmpLevelNameList); pop(@tmpLevelNameList);
                    $wideRegName = join('_', @tmpLevelNameList);
                    if(defined($fieldHash->{'parentFieldIsArray'})) {
                        $wideFieldHash = $referentObj->get_wideRegisterFieldHash();
                        if($fieldHash->{'width'} < $wideFieldHash->{'width'}) {
                            if ($viewName eq 'sv') {
                                $tmpPortName = $self->getFldPortName_EmptyName_Wide_SV($wideFieldHash,
                                                                                       \@tmpLevelNameList,
                                                                                       $expandedName);
                            } else {
                                $tmpName = $fieldHash->{'name'};
                                $tmpName =~ s/_\d+_\d+$//;
                                $tmpPortName = join('_', join('_', @tmpLevelNameList), $tmpName);
                            }
                        } else {
                            if ($viewName eq 'sv') {
                                $tmpPortName = $self->getFldPortName_EmptyName_Wide_SV($wideFieldHash,
                                                                                       \@tmpLevelNameList,
                                                                                       $expandedName);
                            } else {
                                $tmpPortName = join('_', join('_', @tmpLevelNameList), $fieldHash->{'name'});
                            }
                        }
                    } else {
			$widePortInfo = $self->getPortHashInWideReg($fieldHash,
								    $portType,
								    $referentObj,
								    $parentOfReferentHash);
			my $portNameStyle = $widePortInfo->{'portNameStyle'};
			if ($portNameStyle eq 'identifierBaseName') {
                            $tmpPortName = $fieldHash->{'segmentOf'};
                        } elsif ($portNameStyle eq 'addressmapRelativeBaseName') {
                            $tmpPortName = join('_', $wideRegName, $fieldHash->{'segmentOf'});
                            if (($viewName eq 'sv') && ($self->getExpandArraysFlag() == 1)) {
                                if ($expandedName =~ /((\[\d+\])+)/) {
                                    my $index = $1;
                                    $tmpPortName .= $index;
                                }
                            }
                        } else { # basename
			    $tmpPortName = $fieldHash->{'name'};
			}
                    }
                } else {
                    if($portInfo->{'portNameStyle'} eq 'identifierBaseName') {
                        $tmpPortName = $levelNameList[@levelNameList-1];
                    } elsif ($portInfo->{'portNameStyle'} eq 'addressmapRelativeBaseName') {
			$tmpPortName = $expandedName;
                    } else { # basename
			$tmpPortName = $fieldHash->{'name'};
		    }
                }
            } elsif($portType eq 'resetPort') {
		if (defined($fieldHash->{'uniqueResetPort'}) &&
		    ($fieldHash->{'uniqueResetPort'} eq 'true')) {
                    if ($viewName eq 'sv') {
                        if ($referentName =~ /((\[\d+\])+)/) {
                            my $index = $1;
                            $referentName =~ s/((\[\d+\])+)//g;
                            $tmpPortName = $referentName . $portName . $index;
                        } else {
                            $tmpPortName = $referentName . $portName;
                        }
                        if ($self->getExpandArraysFlag() eq 0) {
                            my $dimInfo = '';
                            if (defined($referentHash->{'arrayDimensionInfo'})) {
                                my $arraySize = $self->getDimMinMaxInfo($referentObj,
                                                                        $referentHash,
                                                                        $portInfo,
                                                                        $portName);
                                $dimInfo = $arraySize;
                            }
                            $dimInfo = $self->getDimInfoHierarchically($referentHash,
                                                                       $parentOfReferentHash,
                                                                       $portInfo,
                                                                       $portName,
                                                                       $dimInfo);
                            $tmpPortName .= $dimInfo;
                        }
                    } else {
                        $tmpPortName = $referentName . $portName;
                    }
		} else {
		    $tmpPortName = $portName;
		}
            } else {
                if (defined($fieldHash->{'wideRegisterName'})) {
                    @levelNameList = split('\.', $referentHierId);
		    shift @levelNameList;
                    pop(@levelNameList); pop(@levelNameList);
                    $wideRegName = join('.', @levelNameList);
                    ($wideRegHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($wideRegName);

                    if(defined($fieldHash->{'parentFieldIsArray'})) {
                        $wideFieldHash = $referentObj->get_wideRegisterFieldHash();
                        if($fieldHash->{'width'} < $wideFieldHash->{'width'}) {
                            if ($viewName eq 'sv') {
                                $tmpPortName = $self->getFldPortName_EmptyName_Wide_SV($wideFieldHash,
                                                                                       \@levelNameList,
                                                                                       $expandedName);
                            } else {
                                $tmpName = $fieldHash->{'name'};
                                $tmpName =~ s/_\d+_\d+$//;
                                $tmpPortName = join('_', join('_', @levelNameList), $tmpName);
                            }
                        } else {
                            if ($viewName eq 'sv') {
                                $tmpPortName = $self->getFldPortName_EmptyName_Wide_SV($wideFieldHash,
                                                                                       \@levelNameList,
                                                                                       $expandedName);
                            } else {
                                $tmpPortName = join('_', join('_', @levelNameList), $fieldHash->{'name'});
                            }
                        }
                        $tmpPortName .= $portName;
                        if ($viewName ne 'sv') {
                            $tmpPortName =~ s/\[/_/g;
                            $tmpPortName =~ s/\]//g;
                        }
		    } else {
			$widePortInfo = $self->getPortHashInWideReg($fieldHash,
								    $portType,
								    $referentObj,
								    $parentOfReferentHash);

			my $portNameStyle = $widePortInfo->{'portNameStyle'};
			$wideRegName = join('_', @levelNameList);
			if (defined($portNameStyle)) {
			    if($portNameStyle eq 'identifierBaseName') {
				$tmpPortName = $fieldHash->{'segmentOf'} . $portName;
			    } elsif ($portNameStyle eq 'addressmapRelativeBaseName') {
                                $wideRegName =~ s/\[/_/g;
                                $wideRegName =~ s/\]//g;
                                $tmpPortName = join('_',
                                                    $wideRegName, $fieldHash->{'segmentOf'}) .
                                                    $portName;
                                if (($viewName eq 'sv') && ($self->getExpandArraysFlag() == 1)) {
                                    if ($expandedName =~ /((\[\d+\])+)/) {
                                        my $index = $1;
                                        $tmpPortName .= $index;
                                    }
                                }
			    } else { #basename
				$tmpPortName = $portName;
			    }
			} else { # register or field ref.
			    $tmpPortName = join('_',
                                                $fieldHash->{'wideRegisterName'},
                                                $fieldHash->{'segmentOf'}) .
                                           $portName;
			}
		    }

		    if ($portType eq 'externalWriteAccessPort') {
			my $wideRegAtomicAccess = $wideRegHashRef->{'atomicAccess'};
			#
			# How externalWriteAccess port is generated is different depending on
			# whether the addressmap has atomic_access property specified, or not.
			# If address map has atomic_access there's only one externalWriteAccess
			# port for all the segments.
			#
			if (($wideRegAtomicAccess eq 'lsbRead') || ($wideRegAtomicAccess eq 'msbRead') ||
			    ($wideRegAtomicAccess eq 'none')) {
			    $parentFieldHashRef = $designDBRef->_findFieldNodeByName($fieldHash->{'segmentOf'},
										     $wideRegHashRef);
			    my $topAddrmap = $self->getTopLevelAddressmapNode();
			    if ($topAddrmap->getBusProtocol() ne 'AMBA_4_APB') {
				if ($parentFieldHashRef->{'width'} > $fieldHash->{'width'}) {
				    $tmpPortName .= '_' . join('_', $referentObj->get_parentMsb(),
							       $referentObj->get_parentLsb());
				}
			    }
			}
		    }
                } else { # not wide
		    if (defined($portInfo->{'portNameStyle'})) {
			if ($portInfo->{'portNameStyle'} eq 'identifierBaseName') {
			    $tmpPortName = $levelNameList[@levelNameList-1] . $portName;
			} elsif ($portInfo->{'portNameStyle'} eq 'addressmapRelativeBaseName') {
			    $tmpPortName = $expandedName . $portName;
			} else { # basename
			    $tmpPortName = $portName;
			}
		    } else { # register or field ref.
			$tmpPortName = $portName;
		    }
                }
                if (($viewName eq 'sv') && ($self->getExpandArraysFlag() == 1)) {
                    if ($tmpPortName =~ /((\[\d+\])+)/) {
                        my $index = $1;
                        $tmpPortName =~ s/((\[\d+\])+)//g;
                        $tmpPortName .= $index;
                    }
                }
            }
        } else {  # register or addressmap port
	    if ($referentObj->isSubRegisterNode()) {
		if ($portName ne '') {
		    if ($portType eq 'resetPort') {
			$tmpPortName = $portName;
		    } else {
			$wideRegObj = $referentObj->getParentNode();
			# shift out the addressmap part of the name;
			# also pop the sub-register portion of the name
			shift @levelNameList;
			pop(@levelNameList);
			$tmpPortName = join('', join('_', @levelNameList), $portName);
		    }
                } else {
                    $wideRegObj = $referentObj->getParentNode();
                    $widePortObj = $wideRegObj->getPortType($portType);
                    $tmpPortName = $widePortObj->getName();
                }
	    } elsif ($portType eq 'externalWriteAccessPorts') {
                if ($viewName eq 'sv') {
                    $tmpPortName = $self->adjustArrayNameForSV($portName,
                                                               $expandedName,
                                                               $referentObj);
                } else {
                    $tmpPortName = join('', $expandedName, $portName);
                }
            } else {
                if (defined($aliasName = $referentHash->{'aliasOf'})) {
                    my $hierId = $referentObj->getAliasHierarchicalIdentifier();
                    $hierId =~ s/\[\d+\]//g;
                    @levelNameList = split('\.', $hierId);
                    $expandedName =~ s/$referentHash->{'name'}((_(\w+)*(\d+)*)*)$/$aliasName$1/;
                }
                if($portName eq '') {
                    $tmpPortName = $expandedName;
                } else {
                    if(!defined($portInfo->{'hierarchicalRefName'})) {
                        if (($referentHash->{'nodeType'} eq 'register') ||
			    ($referentHash->{'nodeType'} eq 'wideRegister') ||
			    ($referentHash->{'nodeType'} eq 'union') ||
			    ($referentHash->{'nodeType'} eq 'group')) {
			    if ($portType eq 'resetPort') {
				$tmpPortName = $portName;
			    } elsif ($referentObj->isExternal() && !$referentObj->isDeclaredExternal() &&
				     !defined($referentHash->{'aliasOf'})) {
				$tmpPortName = $referentHash->{'name'} . $portName;
			    } else {
                                if ($viewName eq 'sv') {
                                    my $hierId = $referentObj->getHierarchicalIdentifier();
                                    my @tmpLevelNameList = split('\.', $hierId);
                                    my $topAddrmap = $tmpLevelNameList[0];
                                    $topAddrmap .= '_';
                                    my $tmpExpandedName = $expandedName;
                                    # if expandedName includes the top addressmap name,
                                    # remove it; it is not needed for port name.
                                    if ($tmpExpandedName =~ /^$topAddrmap/) {
                                        $tmpExpandedName =~ s/^$topAddrmap//;
                                    }
                                    $tmpPortName = $self->adjustArrayNameForSV($portName,
                                                                               $tmpExpandedName,
                                                                               $referentObj);
                                } else {
                                    if ($referentObj->isDeclaredExternal()) {
                                        my $arrayDecoder;
                                        if (defined($arrayDecoder = $referentHash->{'hasArrayDecoder'}) &&
                                            ($arrayDecoder eq 'true')) {
                                            my $hierId = $referentObj->getHierarchicalIdentifier();
                                            $hierId =~ s/\[\d+\]$//g;
                                            $hierId =~ s/\[/_/g;
                                            $hierId =~ s/\]//g;
                                            @levelNameList = split('\.', $hierId);
                                        }
                                        shift @levelNameList if (!defined($referentHash->{'aliasOf'}));
                                        if (defined($designDBRef->{'component'})) {
                                            shift @levelNameList;
                                        }
                                        $tmpPortName = join('', join('_', @levelNameList), $portName);
                                    } else {
                                        $tmpPortName = $expandedName . $portName;
                                    }
                                }
			    }
                        } else {
                            if (($viewName eq 'sv') && ($self->getExpandArraysFlag() == 1)) {
                                my $hierId = $referentObj->getHierarchicalIdentifier();
                                my @lvlList = split('\.', $hierId);
                                shift(@lvlList);
                                my $tmpExpanded = join('_', @lvlList);
                                $tmpPortName = $self->adjustArrayNameForSV($portName,
                                                                           $expandedName,
                                                                           $referentObj);
                            } else {
                                if (($referentHash->{'nodeType'} eq 'memory') ||
                                    ($referentHash->{'nodeType'} eq 'wideMemory')) {
                                    shift @levelNameList;
                                    if (defined($designDBRef->{'component'})) {
                                        shift @levelNameList;
                                    }
                                    $tmpPortName = join('', join('_', @levelNameList), $portName);
                                } else {
                                   $tmpPortName = $expandedName . $portName;
                                }
                            }
			}
                    } else {
                        # otherwise, we set the name and the hierarchical ref name in
                        # setHierarchicalRefName()
                        return;
                    }
                }
            }
        }
        my ($inputPortPrefix, $outputPortPrefix);
        my ($inputPortSuffix, $outputPortSuffix);
        my $nodeType = $referentHash->{'nodeType'};
        if (!$self->isSharedPort()) {
            $inputPortPrefix = $referentHash->{'inputPortNamePrefix'};
            $inputPortSuffix = $referentHash->{'inputPortNameSuffix'};
            $outputPortPrefix = $referentHash->{'outputPortNamePrefix'};
            $outputPortSuffix = $referentHash->{'outputPortNameSuffix'};
        } else {
            $inputPortPrefix = $self->get_inputPortNamePrefix();
            $inputPortSuffix = $self->get_inputPortNameSuffix();
        }
        if(!defined($portInfo->{'hierarchicalRefName'}) &&
            (defined($inputPortPrefix) || defined($inputPortSuffix) ||
            defined($outputPortPrefix) || defined($outputPortSuffix))) {
            my @indexList;
            if ($tmpPortName =~ /(\[\d+\])/) {
                while ($tmpPortName =~ /(\[\d+\])$/) {
                    unshift (@indexList, $1);
                    $tmpPortName =~ s/(\[\d+\])$//;
                }
            }
            $self->setPrefixSuffix(\$tmpPortName,
                                   $portInfo,
                                   $referentHash,
                                   $inputPortPrefix,
                                   $inputPortSuffix,
                                   $outputPortPrefix,
                                   $outputPortSuffix);
            if (@indexList) {
                $tmpPortName .= join('', @indexList);
            }
        }
        my $dimInfo = '';
        if (($viewName eq 'sv') && ($self->getExpandArraysFlag() == 0) &&
            ($portType ne 'resetPort')) {
            my $hasArrayDecoder;
            if (!$referentObj->isDeclaredExternal() ||
                ($referentObj->isDeclaredExternal() &&
                 (defined($hasArrayDecoder = $referentObj->get_hasArrayDecoder()) &&
                  ($hasArrayDecoder eq 'false')) || !defined($hasArrayDecoder))) {
                if (defined($referentHash->{'arrayDimensionInfo'})) {
                    my $arraySize = $self->getDimMinMaxInfo($referentObj,
                                                            $referentHash,
                                                            $portInfo,
                                                            $portName);
                    $dimInfo = $arraySize;
                }
                $dimInfo = $self->getDimInfoHierarchically($referentHash,
                                                           $parentOfReferentHash,
                                                           $portInfo,
                                                           $portName,
                                                           $dimInfo);
            } elsif ($referentObj->isDeclaredExternal() &&
                     (defined($hasArrayDecoder = $referentObj->get_hasArrayDecoder()) &&
                     ($hasArrayDecoder eq 'true'))) {
                $dimInfo = $self->getDimInfoHierarchically($referentHash,
                                                           $parentOfReferentHash,
                                                           $portInfo,
                                                           $portName,
                                                           $dimInfo);
            } elsif ($referentObj->isDeclaredExternal()) {
                $dimInfo = $self->getDimInfoHierarchically($referentHash,
                                                           $parentOfReferentHash,
                                                           $portInfo,
                                                           $portName,
                                                           $dimInfo);
            }
            if (($portType ne 'resetPort') ||
                (($portType eq 'resetPort') && ($dimInfo ne '[1]'))) {
                $tmpPortName .= $dimInfo;
            }
        }
        $self->set_name($tmpPortName);
        if ($self->isSharedPort()) {
            $self->set_addressmapRelativeName($tmpPortName);
        }
    }

    sub getFldPortName_EmptyName_Wide_SV {
        my ($self,
            $wideFieldHash,
            $tmpLevelNameList,
            $expandedName) = @_;

        my $tmpPortName;
        if ($self->getExpandArraysFlag() == 1) {
            if ($expandedName =~ /((\[\d+\])+)/) {
                my $index = $1;
                $tmpPortName = join('_', @{$tmpLevelNameList}, $wideFieldHash->{'name'});
                $tmpPortName .= $index;
            } else {
                $tmpPortName = join('_', @{$tmpLevelNameList}, $wideFieldHash->{'name'});
            }
        } else {
            $tmpPortName = join('_', @{$tmpLevelNameList}, $wideFieldHash->{'name'});
        }
        return $tmpPortName;
    }

    sub getDimMinMaxInfo {
        my ($self,
            $referentObj,
            $referentHash,
            $portInfo,
            $portName) = @_;

        my $arrayDimInfo = $referentHash->{'arrayDimensionInfo'};
        my $numDims = scalar(@{$arrayDimInfo});
        my $arraySize = '';
        if (($portInfo->{'sharedPort'} eq 'true') &&
            !$self->sharedPortDeclaredCurrentScope($referentHash,
                                                   $portName)) {
            return $arraySize;
        }
        for (my $index=0; $index < $numDims; $index++) {
            my $arrayMin = $referentObj->getArrayMinIndexForDimension($index);
            my $arrayMax = $referentObj->getArrayMaxIndexForDimension($index);
            if ($arrayMin == 0) {
               $arraySize = '[' . ($arrayMax+1) . ']' . $arraySize;
            } else {
               $arraySize = '[' . $arrayMin . ':' . $arrayMax . ']' . $arraySize;
            }
        }
        return $arraySize;
    }

    sub getDimInfoHierarchically {
        my ($self,
            $referentHash,
            $parentOfReferentHash,
            $portInfo,
            $portName,
            $dimInfo) = @_;

        my $sharedPortDeclaredInChild = 0;
        if (($portInfo->{'sharedPort'} eq 'true') &&
            $self->sharedPortDeclaredCurrentScope($referentHash,
                                                  $portName)) {
            $sharedPortDeclaredInChild = 1;
        }
        if (defined($parentOfReferentHash->{'name'})) {
            if (($portInfo->{'sharedPort'} eq 'false') ||
                (($portInfo->{'sharedPort'} eq 'true') &&
                 (($sharedPortDeclaredInChild == 1) ||
                  $self->sharedPortDeclaredCurrentScope($parentOfReferentHash,
                                                        $portName)))) {
                $sharedPortDeclaredInChild = 1;
                if (defined($parentOfReferentHash->{'arrayDimensionInfo'})) {
                    my $arrayDimInfo = $parentOfReferentHash->{'arrayDimensionInfo'};
                    my $numDims = scalar(@{$arrayDimInfo});
                    for (my $index=0; $index < $numDims; $index++) {
                        my $minArray = $designDBRef->_getArrayMinIndices($arrayDimInfo);
                        my $arrayMin = $minArray->[$numDims - 1 - $index];
                        my $maxArray = $designDBRef->_getArrayMaxIndices($arrayDimInfo);
                        my $arrayMax = $maxArray->[$numDims - 1 - $index];
                        my $arraySize;
                        if ($arrayMin == 0) {
                            $arraySize = '[' . ($arrayMax+1) . ']';
                        } else {
                            $arraySize = '[' . $arrayMin . ':' . $arrayMax . ']';
                        }
                        $dimInfo = $arraySize . $dimInfo;
                    }
                } else {
                    if (($portInfo->{'sharedPort'} eq 'true') &&
                        $self->sharedPortDeclaredCurrentScope($parentOfReferentHash,
                                                              $portName)) {
                        my $foundDecl = $self->getShdPortArraySize($parentOfReferentHash,
                                                                   $portInfo,
                                                                   \$dimInfo);
                    }
                }
            }
        }
        my $foundDecl = 0;
        my $parentHash = $designDBRef->_getParentHash($referentHash);
        if (defined($parentOfReferentHash->{'name'})) {
            if ($parentOfReferentHash->{'hierarchicalIdentifier'} eq
                $parentHash->{'hierarchicalIdentifier'}) {
                $parentHash = $designDBRef->_getParentHash($parentHash);
            }
        }
        while (defined($parentHash) && !$foundDecl) {
            my $shPortDcldInCrrntScope = 0;
            if (($portInfo->{'sharedPort'} eq 'true') &&
                $self->sharedPortDeclaredCurrentScope($parentHash,
                                                      $portName)) {
                $shPortDcldInCrrntScope = 1;
            }
            if (defined($parentHash->{'arrayDimensionInfo'})) {
                if (($portInfo->{'sharedPort'} eq 'false') ||
                    (($portInfo->{'sharedPort'} eq 'true') &&
                     (($sharedPortDeclaredInChild == 1) ||
                      ($shPortDcldInCrrntScope == 1)))) {
                    $sharedPortDeclaredInChild = 1;
                    my $arrayDimInfo = $parentHash->{'arrayDimensionInfo'};
                    my $numDims = scalar(@{$arrayDimInfo});
                    for (my $index=0; $index < $numDims; $index++) {
                        my $arraySize = '[' . $arrayDimInfo->[$numDims -1 - $index]->{'arrayCount'} . ']';
                            $dimInfo = $arraySize . $dimInfo;
                    }
                }
            } else {
                if ($shPortDcldInCrrntScope) {
                    $foundDecl = $self->getShdPortArraySize($parentHash,
                                                            $portInfo,
                                                            \$dimInfo);
                    last if ($foundDecl);
                }
            }
            $parentHash = $designDBRef->_getParentHash($parentHash);
        }
        return $dimInfo;
    }
    sub getShdPortArraySize {
        my ($self,
            $parentHash,
            $portInfo,
            $dimInfo) = @_;

        my $foundDecl = 0;
        my $shPortsRef = $parentHash->{'sharedPorts'};
        for (my $count = 0; $count < @{$shPortsRef}; $count++) {
            my $nextPort = $shPortsRef->[$count];
            my @shPortLevelList = split('\.', $nextPort->{'addressmapRelativeIdentifier'});
            my $rawPortName = pop(@shPortLevelList);
            if ($portInfo->{'name'} eq $rawPortName) {
                if (defined($nextPort->{'arrayDimensionInfo'})) {
                    my $arrayDimInfo = $nextPort->{'arrayDimensionInfo'};
                    my $numDims = scalar(@{$arrayDimInfo});
                    for (my $index=0; $index < $numDims; $index++) {
                        my $arraySize = '[' . $arrayDimInfo->[$numDims -1 - $index]->{'arrayCount'} . ']';
                        $$dimInfo = $arraySize . $$dimInfo;
                    }
                    $foundDecl = 1;
                    last;
                }
            }
            last if ($foundDecl);
        }
        return $foundDecl;
    }
    sub adjustArrayNameForSV {
        my ($self,
            $portName,
            $expandedName,
            $referentObj) = @_;

        my ($tmpPortName, $hasArrayDecoder);
        if ($referentObj->isDeclaredExternal() &&
            defined($hasArrayDecoder = $referentObj->get_hasArrayDecoder()) &&
            ($hasArrayDecoder eq 'true')) {
            if ($referentObj->isArray()) {
                my $numDims = $referentObj->getNumArrayDimensions();
                my @indexList;
                for (my $count = 0; $count < $numDims; $count++) {
#                    $expandedName =~ /(\[\d+\])$/;
                    $expandedName =~ s/(\[\d+\])$//;
                }
                while ($expandedName =~ /(\[\d+\])$/) {
                    my $index = $1;
                    unshift (@indexList, $index);
                    $expandedName =~ s/(\[\d+\])$//;
                }
                $tmpPortName = $expandedName . $portName . join('', @indexList);
            } else {
                my @indexList;
                while ($expandedName =~ /(\[\d+\])$/) {
                    my $index = $1;
                    unshift (@indexList, $index);
                    $expandedName =~ s/(\[\d+\])$//;
                }
                $tmpPortName = $expandedName . $portName . join('', @indexList);
            }
        } else {
            if ($expandedName =~ /((\[\d+\])+)/) {
                my $index = $1;
                $expandedName =~ s/((\[\d+\])+)//g;
                $tmpPortName = $expandedName . $portName . $index;
            } else {
                $tmpPortName = $expandedName . $portName;
            }
        }
        return $tmpPortName;
    }
    sub sharedPortDeclaredCurrentScope {
        my ($self,
            $referentHash,
            $portName) = @_;

        # is portName a sharedPort, and is it declared in the current scope?
        if (($viewName eq 'sv') && defined($referentHash->{'sharedPorts'})) {
	    foreach my $nextShPort (@{$referentHash->{'sharedPorts'}}) {
		my @shPortLevelList = split('\.', $nextShPort->{'addressmapRelativeIdentifier'});
		my $rawShPortName = pop(@shPortLevelList);
                if ($rawShPortName eq $portName) {
                    return 1;
                }
            }
        }
        return 0;
    }

    sub getSharedPortName {
	my ($self,
            $referentHash,
            $referentObj,
            $parentOfReferentHash,
            $portInfo,
            $portName,
            $portAddrmapRelId,
            $levelNameList,
            $expandedName) = @_;

	my ($tmpPortName);
	if (defined($referentHash->{'sharedPorts'})) {
	    foreach my $nextShPort (@{$referentHash->{'sharedPorts'}}) {
		my @shPortLevelList = split('\.', $nextShPort->{'addressmapRelativeIdentifier'});
		my $rawShPortName = pop(@shPortLevelList);
                my $shdPortAddrmapRelId = $nextShPort->{'addressmapRelativeIdentifier'};
                if ($shdPortAddrmapRelId eq $portAddrmapRelId) {
                    if (($viewName eq 'sv') && ($self->getExpandArraysFlag() == 1)) {
                        my $shdPortId = $nextShPort->{'addressmapRelativeIdentifier'};
                        $shdPortId =~ s/\./_/g;
                        my @indexList;
                        if ($referentObj->isArray()) {
                            while ($expandedName =~ /(\[\d+\])$/) {
                                my $index = $1;
                                unshift (@indexList, $index);
                                $expandedName =~ s/(\[\d+\])$//;
                            }
                            $tmpPortName = $shdPortId . join('', @indexList);
                        } else {
                            $tmpPortName = $shdPortId;
                        }
                    } else {
                        $tmpPortName = $referentObj->getHierarchicalName() . '_' . $portName;
                        shift @$levelNameList;
                        $tmpPortName = join('_', (@$levelNameList, $portName));
                    }
                    last;
                }
            }
            $tmpPortName = $self->getSharedPortNameInParent($referentHash,
                                                            $referentObj,
                                                            $parentOfReferentHash,
                                                            $portInfo,
                                                            $portName,
                                                            $levelNameList,
                                                            $expandedName) if (!defined($tmpPortName));
	} else {
	    $tmpPortName = $self->getSharedPortNameInParent($referentHash,
                                                            $referentObj,
                                                            $parentOfReferentHash,
                                                            $portInfo,
                                                            $portName,
                                                            $levelNameList,
                                                            $expandedName);
	}
	return $tmpPortName;
    }

    sub getSharedPortNameInParent {
	my ($self,
            $referentHash,
            $referentObj,
            $parentOfReferentHash,
            $portInfo,
            $portName,
            $levelNameList,
            $expandedName) = @_;

        my ($tmpPortName, $bogusAddr);

	my $found = 0;
	my $isAliasOf = 0;
	my $parentHash = $parentOfReferentHash;
        my $shdPortArrayIndex = '';
        my $objHierId = $referentObj->getHierarchicalIdentifier();
        if ($referentObj->isAlias()) {
            my @tmpLevelNameList = split('\.', $objHierId);
            pop(@tmpLevelNameList);
            ($parentHash, $bogusAddr) = $designDBRef->_getHashRefForName(join('.', @tmpLevelNameList));
            $levelNameList = \@tmpLevelNameList;
        }
        my $arrayIndex = '';
        my $objIsArray = $referentObj->isArray();
        if ($objIsArray &&
            ($viewName eq 'sv') &&
            ($self->getExpandArraysFlag() == 1)) {
            my $arrayDimInfo = $referentHash->{'arrayDimensionInfo'};
            my $numDims = scalar(@{$arrayDimInfo});
            for (my $index=0; $index < $numDims; $index++) {
                 $expandedName =~ /(\[\d+\])$/;
                 my $index = $1;
                 $arrayIndex = $index . $arrayIndex;
                 $expandedName =~ s/(\[\d+\])$//;
            }
        }
        my $refObjIsArray = $referentObj->isArray();
	while (defined($parentHash)) {
            pop(@{$levelNameList});
            pop(@{$levelNameList}) if (defined($parentHash->{'subRegisterFlag'}));
	    if (defined($parentHash->{'aliasOf'})) {
		my ($actualReg, $bigIntAddr) = $designDBRef->_getHashRefForName(join('.', (@{$levelNameList},
                                                                                           $parentHash->{'aliasOf'})));
		$isAliasOf = 1;
		$parentHash = $actualReg;
	    }
	    if (defined($parentHash->{'sharedPorts'})) {
		foreach my $nextShPort (@{$parentHash->{'sharedPorts'}}) {
		    my @shPortLevelList = split('\.', $nextShPort->{'addressmapRelativeIdentifier'});
		    my $rawShPortName = pop(@shPortLevelList);
		    if ($rawShPortName eq $portName) {
                        # if the sharedPort is not declared as array, remove any array
                        # information recorded for it so far from the hierarchy. 
                        $tmpPortName = $nextShPort->{'addressmapRelativeName'};
                        if (defined($nextShPort->{'arrayDimensionInfo'})) {
                            if ($refObjIsArray) {
                                if ($viewName ne 'sv') {
                                    my $tmpIndex = join('_', @{$referentObj->getArrayIndex()});
                                    if ($shdPortArrayIndex ne '') { 
                                        $shdPortArrayIndex = join('_', $shdPortArrayIndex, @{$referentObj->getArrayIndex()});
                                    } else {
                                        $shdPortArrayIndex = join('_', @{$referentObj->getArrayIndex()});
                                    }
                                } else {
                                    if ($self->getExpandArraysFlag() == 1) {
                                        $shdPortArrayIndex .= $arrayIndex;
                                    } else {
                                        my $dimInfo = $self->getDimMinMaxInfo($referentObj,
                                                                              $referentHash,
                                                                              $portInfo,
                                                                              $portName);
					    $parentHash->{'name'} . ', portName: ' . $portName . "\n";
                                        $dimInfo = $self->getDimInfoHierarchically($referentHash,
                                                                                   $parentHash,
                                                                                   $portInfo,
                                                                                   $portName,
                                                                                   $dimInfo);
                                        my $tmpPortName .= $dimInfo;
                                    }
                                }
                            }
                            my $parentIsArray;
                            if (defined($parentHash->{'arrayDimensionInfo'}) ||
                                defined($parentHash->{'subRegisterFlag'}) &&
                                defined($parentIsArray = $parentHash->{'parentIsArray'}) &&
                                        ($parentIsArray eq 'true')) {
                                my $parentArrayIndex = $self->getParentArrayIndex($parentHash,
                                                                                  $levelNameList,
                                                                                  $expandedName);
                                $shdPortArrayIndex = $parentArrayIndex . $shdPortArrayIndex;
                            } else {
                                $shdPortArrayIndex = $shdPortArrayIndex;
                            }
                        } else { # shared port is not array
                            if (defined($parentHash->{'arrayDimensionInfo'})) {
                                my @tmpParentIdList = split('\.', $objHierId);
                                shift(@tmpParentIdList);
                                pop(@tmpParentIdList);
                                my $tmpParentName = join('_', @tmpParentIdList);
                                if ($viewName ne 'sv') {
                                    $tmpParentName =~ s/\[/_/g;
                                    $tmpParentName =~ s/\]//g;
                                } else {
                                    $shdPortArrayIndex = $self->getParentArrayIndex($parentHash,
                                                                                    $levelNameList,
                                                                                    $expandedName);
                                }
                                $tmpPortName = $nextShPort->{'addressmapRelativeIdentifier'};
                                my @tmpPortIdList = split('\.', $tmpPortName);
                                $tmpPortName = pop(@tmpPortIdList);
                                $tmpPortName = join('_', $tmpParentName, $tmpPortName);
                            } else {
                                $shdPortArrayIndex = '';
                            }
                        }
			$found = 1;
			last;
		    }
		}
	    } else {
                my ($parentIsArray, $addr);
                # if parentHash is sub-register, check parentIsArray flag
                if (defined($parentHash->{'arrayDimensionInfo'}) ||
                    defined($parentHash->{'subRegisterFlag'}) &&
                    defined($parentIsArray = $parentHash->{'parentIsArray'}) &&
                    ($parentIsArray eq 'true')) {
                    if ($viewName ne 'sv') {
                        my $parentArrayIndexRef = $self->getParentArrayIndexFromHierId($objHierId);
                        my $tmpIndex = join('', @{$parentArrayIndexRef});
                        $tmpIndex =~ s/\[/_/g;
                        $tmpIndex =~ s/\]//g;
                        $shdPortArrayIndex = $tmpIndex . $shdPortArrayIndex;
                    } else {
                        my $parentArrayIndex = $self->getParentArrayIndex($parentHash,
                                                                          $levelNameList,
                                                                          $expandedName);
                        $shdPortArrayIndex = $parentArrayIndex . $shdPortArrayIndex;
                    }
                }
            }
	    last if ($found);
	    $parentHash = $designDBRef->_getParentHash($parentHash);
	}
        if ($viewName ne 'sv') {
            if ($shdPortArrayIndex ne '') {
                $tmpPortName = $tmpPortName . '_' . $shdPortArrayIndex;
            }
        } else {
            $tmpPortName .= $shdPortArrayIndex;
        }
	return $tmpPortName;
    }

    sub getParentArrayIndex {
        my ($self,
            $parentHash,
            $levelNameList,
            $expandedName) = @_;

        my ($parentIsArray, $addr);
        my $parentArrayIndex = '';
        # if parentHash is sub-register, check parentIsArray flag
        if (defined($parentHash->{'arrayDimensionInfo'}) ||
            defined($parentHash->{'subRegisterFlag'}) &&
            defined($parentIsArray = $parentHash->{'parentIsArray'}) &&
            ($parentIsArray eq 'true')) {
            if ($viewName ne 'sv') {
                return $parentArrayIndex;
            } else {
                if ($self->getExpandArraysFlag() == 1) {
                    if (defined($parentIsArray = $parentHash->{'parentIsArray'}) &&
                        ($parentIsArray eq 'true')) { # sub-register; get wideReg
                        ($parentHash, $addr) = $designDBRef->_getHashRefForName(join('.', @{$levelNameList}));                    }
                    my $arrayDimInfo = $parentHash->{'arrayDimensionInfo'};
                    my $numDims = scalar(@{$arrayDimInfo});
                    my @indexList;
                    for (my $count = 0; $count < $numDims; $count++) {
                        $expandedName =~ /(\[\d+\])$/;
                        my $tmpIndex = $1;
                        unshift(@indexList, $tmpIndex);
                        $expandedName =~ s/(\[\d+\])$//;
                    }
                    $parentArrayIndex = join('', @indexList);
                } else {
                }
            }
        }
        return $parentArrayIndex;
    }

    sub getPortHashInWideReg {
	my ($self,
	    $fieldHash,
	    $portType,
	    $referentObj,
	    $parentOfReferentHash) = @_;

	my $portTypeRef;
	my $wideFieldHash = $referentObj->get_wideRegisterFieldHash();
	if (($wideFieldHash->{'external'} eq 'true') && ($portType =~ /external/)) {
	    $portTypeRef = $wideFieldHash->{'externalBusPorts'};
	} else {
	    $portTypeRef = $wideFieldHash->{'ports'};
	}
        for(my $count = 0; $count < @{$portTypeRef}; $count++) {
	    next if (($fieldHash->{'nodeType'} eq 'field') &&
		     defined($parentOfReferentHash->{'broadcastMembers'}));
            my @portType = keys %{$portTypeRef->[$count]};
            my @portInfo = values %{$portTypeRef->[$count]};
	    return $portInfo[0] if ($portType[0] eq $portType);
	}
	$self->_printError('Port type: ' . $portType . ' not found in wide field: ' . $wideFieldHash->{'name'});
	exit(1);
    }

    sub setPrefixSuffix {
        my ($self,
            $portName,
            $portInfo,
            $referentHash,
            $inputPortPrefix,
            $inputPortSuffix,
            $outputPortPrefix,
            $outputPortSuffix) = @_;

        my $portDir = $portInfo->{'portDirection'};
        if(defined($portDir) && ($portDir eq 'input')) {
            if(defined($inputPortPrefix) && ($inputPortPrefix ne '')) {
                $$portName = $inputPortPrefix . $$portName;
            }
            if(defined($inputPortSuffix) && ($inputPortSuffix ne '')) {
                $$portName .= $inputPortSuffix;
            }
        } elsif(defined($portDir) && ($portDir eq 'output')) {
            if(defined($outputPortPrefix) && ($outputPortPrefix ne '')) {
                $$portName = $outputPortPrefix . $$portName;
            }
            if(defined($outputPortSuffix) && ($outputPortSuffix ne '')) {
                $$portName .= $outputPortSuffix;
            }
        }
    }

    sub getConcatItem {
	my ($self, $bitIndex) = @_;

	# get the port object in the concatenation list
	# at index: bitIndex.

	my ($currItemWidth, $tmpIndex);
	my $currTotal = 0;
	my $concatPortList = $self->getConcatList();
	my $bitNumbering = $concatPortList->[0]->getBitNumbering();
	for (my $index = ($bitNumbering eq 'lsb_0') ? @{$concatPortList}-1 : 0;
	     ($bitNumbering eq 'lsb_0') ? $index >= 0 : $index < @{$concatPortList};
	     ($bitNumbering eq 'lsb_0') ? $index-- : $index++) {
	    my $nextConcatItem = $concatPortList->[$index];
	    if ($nextConcatItem->getSourceType() eq 'fieldRef') {
		my ($regRefObj, $fieldRefObj) = $nextConcatItem->getReferenceInfo();
		if ($nextConcatItem->hasRange()) {
		    $currItemWidth = $nextConcatItem->getRangeWidth();
		} else {
		    $currItemWidth = $fieldRefObj->getWidth();
		}
	    } else {
		if ($nextConcatItem->hasRange()) {
		    $currItemWidth = $nextConcatItem->getRangeWidth();
		} else {
		    $currItemWidth = $nextConcatItem->getWidth();
		}
	    }
	    if (($currTotal + $currItemWidth - 1) >= $bitIndex) {
		return ($nextConcatItem, ($bitIndex - $currTotal));
	    } elsif (($currTotal + $currItemWidth - 1) < $bitIndex) {
		$currTotal += $currItemWidth;
	    }
	}
    }

    sub checkConcatItemIndex {
        my ($self, $bitIndex) = @_;

        my $concatPortList;
        my $parentHashRef = $self->get_parentHashRef();
        if (defined($concatPortList = $self->getConcatList())) {
            if (!defined($bitIndex)) {
                my $i=0;
                while (my($pack, $file, $line, $subname, $hasargs, $wantarray) = caller($i++)) {
                    print 'In file: ' . $file . ', line \#: ' . $line .
                          ', subroutine name: ' . $subname . "\n";
                }
                $self->_printError('Port Type: ' . $self->getPortType() . ' in ' .
                                   $parentHashRef->{'nodeType'} . ': '
                                   . $parentHashRef->{'name'} . "\n\t\t" .
                                   ' is of type concatenation, but no indexing ' .
                                   'information is specified.');
                exit(1);
            }
        }
        return 1;
    }
}

package csrPorts;

use strict;
@csrPorts::ISA = qw(csrPortsBase);

{
    my %portAttrs = (_concatList     => '',
		     _ports          => '',   # list of concatenated ports
		     _isConcatMember => 0,
		     _isMultiPort => 0,
		     _referentObj => '',
		     );

    sub _defaultAttrKeys {
	my $self = shift;
	return ($self->SUPER::_defaultAttrKeys(), map { s/^_//; $_ } keys %portAttrs);
    }

    sub new {
        my($caller,
           $referentObj,
           $referentHash,
	   $portTypeRef,
           $parentOfReferentHash,
           $expandedName) = @_;

	my ($nextConcatPortInfo, $nextConcatPortObj);
        my ($portObj, $tmpPortName, $sourceType);
        my (@portList, @portType, @portInfo, $concatPortDir);

	my $hasConcatFlag = 0;
        for(my $count = 0; $count < @{$portTypeRef}; $count++) {
	    next if (($referentHash->{'nodeType'} eq 'field') &&
		     defined($parentOfReferentHash->{'broadcastMembers'}));
            @portType = keys %{$portTypeRef->[$count]};
            @portInfo = values %{$portTypeRef->[$count]};
            next if ($referentObj->isVirtual() && ($portType[0] eq 'outputPort'));
	    my (@portConcatList, @multiPortList);
	    if (Scalar::Util::reftype($portInfo[0]) eq 'ARRAY') {
		for (my $portIndex=0; $portIndex < @{$portInfo[0]}; $portIndex++) {
		    my $nextPortMember = csrPorts->SUPER::new($portInfo[0]->[$portIndex]);
		    $nextPortMember->set_bitNumbering($parentOfReferentHash->{'bitNumbering'});
		    $nextPortMember->set_referentObj($referentObj);
		    my $rawPortName = $portInfo[0]->[$portIndex]->{'name'};
		    $nextPortMember->setPortName($referentHash,
						 $parentOfReferentHash,
						 $referentObj,
						 $rawPortName,
						 $portType[0],
						 $portInfo[0]->[$portIndex],
						 $expandedName);
		    my $newPortType;
		    if ($portType[0] eq 'externalWriteAccessPorts') {
			$newPortType = 'externalWriteAccessPort';
		    } else {
			my $i=0;
			while (my($pack, $file, $line, $subname, $hasargs, $wantarray) = caller($i++)) {
			    print 'In file: ' . $file . ', line \#: ' . $line . ', subroutine name: ' . $subname . "\n";
			}
			$nextPortMember->_printError('Type of multiport: ' . $portType[0] . ' not recognized');
			exit(1);
		    }
		    $nextPortMember->set_portType($newPortType);
		    push (@multiPortList, $nextPortMember);
		}
		my $bogusHash = {};
		$portObj = csrPorts->SUPER::new($bogusHash);
		$portObj->set_portDirection($portInfo[0]->[0]->{'portDirection'});
	    } elsif (defined($portInfo[0]->{'concatenation'})) {
		$hasConcatFlag = 1;
		my $concatPortHash = $portInfo[0]->{'concatenation'};
		for (my $concatIndex = 0; $concatIndex < @{$concatPortHash}; $concatIndex++) {
		    $nextConcatPortInfo = $concatPortHash->[$concatIndex];
		    $nextConcatPortObj = csrPorts->SUPER::new($nextConcatPortInfo);
		    $nextConcatPortObj->set_referentObj($referentObj);

		    if(defined($nextConcatPortInfo->{'hierarchicalRefName'})) {
			$nextConcatPortObj->setHierarchicalRefName($nextConcatPortInfo->{'hierarchicalRefName'},
								   $referentObj);
			$tmpPortName = $nextConcatPortObj->getName();
			$nextConcatPortObj->set_name($tmpPortName);
                    } elsif (defined($nextConcatPortInfo->{'addressmapRelativeIdentifier'})) {
                        $nextConcatPortObj->set_addressmapRelativeIdentifier($nextConcatPortInfo->{'addressmapRelativeIdentifier'});
                        $nextConcatPortObj->set_addressmapRelativeName($nextConcatPortInfo->{'addressmapRelativeName'});
                    }
		    if ($nextConcatPortObj->get_sourceType() eq 'number') {
			my ($verilogFormat, $constVal) = $nextConcatPortObj->getVerilogBaseAndValue($nextConcatPortObj->get_name());
			$nextConcatPortObj->set_constantValue($constVal);
		    }
		    $nextConcatPortObj->set_portType($portType[0]);
		    $nextConcatPortObj->set_isConcatMember(1);
		    my $sourceType = $nextConcatPortObj->get_sourceType();
		    if ($sourceType eq 'fieldRef') {
			my ($fieldHashRef, $bogusAddr) =
			    $designDBRef->_getHashRefForName($nextConcatPortObj->get_hierarchicalRefName());
			if (!defined($nextConcatPortObj->{'range'})) {
			    $nextConcatPortObj->set_msb($fieldHashRef->{'msb'});
			    $nextConcatPortObj->set_lsb($fieldHashRef->{'lsb'});
			    $nextConcatPortObj->set_width(abs($fieldHashRef->{'msb'} - $fieldHashRef->{'lsb'})+1);
			} else {
			    $nextConcatPortObj->set_msb($nextConcatPortInfo->{'rangeMsb'});
			    $nextConcatPortObj->set_lsb($nextConcatPortInfo->{'rangeLsb'});
			    $nextConcatPortObj->set_width($nextConcatPortInfo->{'rangeWidth'});
			}
		    } elsif ($sourceType eq 'inputPort') {
			$nextConcatPortObj->set_msb($nextConcatPortInfo->{'msb'});
			$nextConcatPortObj->set_lsb($nextConcatPortInfo->{'lsb'});
			$nextConcatPortObj->set_width(abs($nextConcatPortInfo->{'msb'} - $nextConcatPortInfo->{'lsb'})+1);
			$nextConcatPortObj->set_parentHierarchicalName($referentObj->getHierarchicalIdentifier());
			$nextConcatPortObj->set_parentHashRef($referentHash);
		    }
		    $nextConcatPortObj->set_bitNumbering($parentOfReferentHash->{'bitNumbering'});
		    push (@portConcatList, $nextConcatPortObj);
		}
		my $bogusHash = {};
		$portObj = csrPorts->SUPER::new($bogusHash);
		$portObj->set_referentObj($referentObj);
		$portObj->set_portDirection($nextConcatPortObj->get_portDirection());
		$portObj->set_isMultiPort(0);
            } else {
		$portObj = csrPorts->SUPER::new($portInfo[0]);
		$portObj->set_referentObj($referentObj);
		$portObj->set_isConcatMember(0);
		$portObj->set_isMultiPort(0);
		$portObj->set_bitNumbering($parentOfReferentHash->{'bitNumbering'});
	    }
            $portObj->set_parentHashRef($referentHash);
            $portObj->set_parentHierarchicalName($referentObj->getHierarchicalIdentifier());
            $portObj->set_portType($portType[0]);
	    my $rawPortName;
	    if (@portConcatList) {
		$portObj->set_concatList(\@portConcatList);
		$portObj->set_ports(\@portConcatList);
		push(@portList, $portObj);
		next;
	    } elsif (@multiPortList) {
		$portObj->set_multiPortList(\@multiPortList);
		$portObj->set_ports(\@multiPortList);
		$portObj->set_isMultiPort(1);
		push(@portList, $portObj);
		next;
	    }
	    $rawPortName = $portTypeRef->[$count]->{$portType[0]}->{'name'};
	    $portObj->setPortName($referentHash,
				  $parentOfReferentHash,
				  $referentObj,
				  $rawPortName,
				  $portType[0],
				  $portInfo[0],
				  $expandedName);
            if($portType[0] eq 'resetPort') {
                $portObj->set_resetTiming($referentObj->getResetTiming());
            }
            if(defined($portInfo[0]->{'hierarchicalRefName'})) {
                $portObj->setHierarchicalRefName($portInfo[0]->{'hierarchicalRefName'},
						 $referentObj);
                $tmpPortName = $portObj->getName();
                $portObj->setPrefixSuffix(\$tmpPortName,
					  $portInfo[0],
					  $referentHash);
                $portObj->set_name($tmpPortName);
            }
            if(defined($sourceType = $portInfo[0]->{'sourceType'})) {
                $portObj->set_sourceType($sourceType);
            }
            push(@portList, $portObj);
        }
	if ($hasConcatFlag == 1) {
	    foreach my $nextPortNode (@portList) {
		if ($nextPortNode->hasConcatFields()) {
		    $referentObj->set_totalNumPorts($referentObj->get_totalNumPorts() + @{$nextPortNode->get_ports()});
		} else {
		    $referentObj->set_totalNumPorts($referentObj->get_totalNumPorts() + 1);
		}
	    }
	} else {
	    my $totalPorts = @portList;
	    $referentObj->set_totalNumPorts($totalPorts);
	}
        return (\@portList);
    }
    sub isConcatMember {
	my $self = shift;
	return $self->get_isConcatMember();
    }
    sub hasConcatFields {
	my $self = shift;
	return 1 if (defined($self->get_concatList()));
	return 0;
    }
    sub getConcatList {
	my $self = shift;
	return $self->get_concatList();
    }
}

package csrResetValue;

@csrResetValue::ISA = qw(csrPerlAPIBase);

use strict;
use vars '$AUTOLOAD';

{
    my %resetAttr = (_sourceType => '',
                     _value => '',
                     _mask => '',
                     _resetCompareString => '',
                    );

    sub new {
        my($caller, $resetHashRef) = @_;

        my $class = ref($caller) || $caller;
        my $self = bless {}, $class;
	$self->set_sourceType($resetHashRef->{'sourceType'}) if(defined($resetHashRef->{'sourceType'}));
	$self->set_value($resetHashRef->{'value'}) if(defined($resetHashRef->{'value'}));
	$self->set_mask($resetHashRef->{'mask'}) if(defined($resetHashRef->{'mask'}));
	$self->set_resetCompareString($resetHashRef->{'resetCompareString'})
	    if(defined($resetHashRef->{'resetCompareString'}));
        return $self;
    }
    sub getValue {
        my $self = shift;
        return $self->get_value();
    }
    sub getSourceType {
        my $self = shift;
        return $self->get_sourceType();
    }
    sub getMask {
        my $self = shift;
        return $self->get_mask();
    }
    sub getMaskValue {
        my $self = shift;
        my ($verilogFormat, $maskValue);
        if(defined($maskValue = $self->get_mask())) {
            ($verilogFormat, $maskValue) = $self->getVerilogBaseAndValue($maskValue);
            return Math::BigInt->new($maskValue);
        }
        return;
    }
}

package csrTriggerValue;

@csrTriggerValue::ISA = qw(csrPerlAPIBase);

use strict;
use vars '$AUTOLOAD';

{
    my %resetAttr = (_sourceType => '',
                     _value => '',
                    );

    sub new {
        my($caller, $trigValueHashRef) = @_;

        my $class = ref($caller) || $caller;
        my $self = bless {}, $class;
        $self->set_sourceType($trigValueHashRef->{'sourceType'})
            if(defined($trigValueHashRef->{'sourceType'}));
        $self->set_value($trigValueHashRef->{'value'}) if(defined($trigValueHashRef->{'value'}));
        return $self;
    }
    sub getSourceType {
        my $self = shift;
        return $self->get_sourceType();
    }
    sub getValue {
        my $self = shift;
        return $self->get_value();
    }
}

package csrPerlAPIBase;

use strict;
use vars '$AUTOLOAD';

{
    my %commonAttrs = (_nodeType => '',
		       _name => '',
		       _identifier => '',
		       _title => '',
		       _description => '',
		       _hierarchicalName => '',
		       _hierarchicalIdentifier => '',
		       _hierarchicalMacroName => '',
		       _macroIdentifier => '',
		       _macroName => '',
		       _templateName => '',
		       _templateIdentifier => '',
		       _templateMacroName => '',
		       _templateFilename => '',
		       _templateLinenumber =>  '',
		       _effectiveTypeName => '',
		       _addressmapRelativeName => '',
		       _addressmapRelativeIdentifier => '',
		       _accessType => 'r',
		       _arrayDimensionInfo => '',
		       _external => '',
                       _declaredExternal => '',
		       _isVolatile => 'true',
		       _isPrivate => '',
		       _privacyLevel => '',
		       _privacySet => '',
		       _sourceFilename => '',
		       _sourceFileVersion => '',
		       _lineNumber => 0,
		       _designPkgName => '',
		       _addressmapName => '',
		       _designDBRef => '',
		       _topLevelAddressmapNode => '',
                       _attributes => '',
		       _portIterationList => '',
		       );

    sub _defaultAttrKeys {
	map { s/^_//; $_ } keys %commonAttrs;
    }

    sub _printError {
	my ($self, $msg) = @_;
	my($packageId, $filename, $linenum, $subname) = caller(1);
	print "\n\tERROR - In " . $subname . ' - ' . $msg . "\n\n";
    }

    sub new {
        my($caller, $argsRef) = @_;
        my $class = ref($caller) || $caller;
        my $self = bless {}, $class;
        $self->_init($caller, $argsRef);
        return $self;
    }

    sub _init {
        my($self, $caller, $argsRef) = @_;
        foreach my $attrName ($self->_defaultAttrKeys()) {
            if(defined($argsRef) && defined($argsRef->{$attrName})) {
                $self->{"_$attrName"} = $argsRef->{$attrName};
            } elsif(ref($caller)) { # copy object data
                $self->{"_$attrName"} = $caller->{"_$attrName"};
            }
        }
        if(exists($argsRef->{'name'})) {
            $self->set_identifier($argsRef->{'name'});
        }
    }

    sub getDesignPkgName {
        my $self = shift;
        return $self->get_designPkgName();
    }
    sub getSourceFileName {
	my $self = shift;
	return $self->get_sourceFilename();
    }
    sub getName {
        my $self = shift;
        return $self->get_name();
    }
    sub getHierarchicalName {
        my $self = shift;
        return $self->get_hierarchicalName();
    }
    sub getHierarchicalIdentifier {
        my $self = shift;
        return $self->get_hierarchicalNameDotted();
    }
    sub getIdentifier {
        my $self = shift;
        return $self->get_identifier();
    }
    sub getMacroName {
        my $self = shift;
        return $self->get_macroName();
    }
    sub getMacroIdentifier {
        my $self = shift;
        return $self->get_macroIdentifier();
    }
    sub getHierarchicalMacroName {
        my $self = shift;
        return $self->get_hierarchicalMacroName();
    }
    sub getAddressmapRelativeName {
        my $self = shift;
        return $self->get_addressmapRelativeName();
    }
    sub getAddressmapRelativeIdentifier {
        my $self = shift;
        return $self->get_addressmapRelativeIdentifier();
    }
    sub getReservedValue {
        my $self = shift;
        return $self->get_reservedValue();
    }
    sub hasTitle {
        my $self = shift;
        return 1 if (defined($self->get_title()));
	return 0;
    }
    sub getTitle {
        my $self = shift;
        return $self->get_title();
    }
    sub hasDescription {
        my $self = shift;
        return 1 if (defined($self->get_description()));
	return 0;
    }
    sub getDescription {
        my $self = shift;
        return $self->get_description();
    }
    sub getNodeType {
        my $self = shift;
        return $self->get_nodeType();
    }
    sub getBitNumbering {
        my $self = shift;
        return $self->get_bitNumbering();
    }
    sub getAccessType {
        my $self = shift;
        return $self->get_accessType();
    }
    sub isSubRegisterNode {
	my $self = shift;
	if ($self->getNodeType() eq 'register') {
	    return 1 if (defined($self->getSubRegisterFlag()));
	}
	return 0;
    }
    sub isSubMemoryNode {
	my $self = shift;
	if ($self->getNodeType() eq 'memory') {
	    return 1 if (defined($self->get_subMemoryFlag()));
	}
	return 0;
    }
    sub isWideMemoryNode {
	my $self = shift;
	return 1 if ($self->getNodeType() eq 'wideMemory');
	return 0;
    }
    sub isArray {
	my $self = shift;
	return 1 if (defined($self->get_arrayDimensionInfo()));
	return 0;
    }
    sub getArrayIndex {
	my $self = shift;
	return $self->get_arrayIndex();
    }
    sub getArraySize {
	my $self = shift;
	my $arraySize = $designDBRef->_getArraySize($self->get_arrayDimensionInfo());
	return $arraySize;
    }
    sub getNumArrayDimensions {
	my $self = shift;
        return(scalar(@{$self->get_arrayDimensionInfo()}));
    }
    sub getArrayMinIndexForDimension {
	my ($self, $index) = @_;
	my $numDims = $self->getNumArrayDimensions();
	my $minArray = $self->getArrayMin();
	return($minArray->[$numDims - 1 - $index]);
    }
    sub getArrayMaxIndexForDimension {
	my ($self, $index) = @_;
	my $numDims = $self->getNumArrayDimensions();
	my $arrayDimensionInfo = $self->get_arrayDimensionInfo();
	my $maxArray = $self->getArrayMax();
	return($maxArray->[$numDims -1 - $index]);
    }
    sub getArrayStrideForDimension {
	my ($self, $index) = @_;
	return $self->getArrayElementByteSize() if ($index == 0);
	my $stride = 1;
	my $arrayDimensionInfo = $self->get_arrayDimensionInfo();
	my $numDims = $self->getNumArrayDimensions();
	my $recordedIndex = ($numDims - 1 - $index);
	for (my $tmpIndex = ($numDims-1); $tmpIndex > $recordedIndex; $tmpIndex--) {
	    $stride *= $arrayDimensionInfo->[$tmpIndex]->{'arrayCount'};
	}
	return ($stride * $self->getArrayElementByteSize());
    }

    sub getArraySizeForDimension {
	my ($self, $index) = @_;
	my $numDims = $self->getNumArrayDimensions();
	my $arrayDimensionInfo = $self->get_arrayDimensionInfo();
	return ($arrayDimensionInfo->[$numDims -1 - $index]->{'arrayCount'});
    }
    sub getArrayMin {
        my $self = shift;
        return $designDBRef->_getArrayMinIndices($self->get_arrayDimensionInfo());
    }
    sub getArrayMax {
        my $self = shift;
        return $designDBRef->_getArrayMaxIndices($self->get_arrayDimensionInfo());
    }
    sub getTemplateName {
        my $self = shift;
        return $self->get_templateName();
    }
    sub getTemplateMacroName {
        my $self = shift;
        return $self->get_templateMacroName();
    }
    sub getTemplateIdentifier {
        my $self = shift;
        return $self->get_templateIdentifier();
    }
    sub getTemplateFileName {
        my $self = shift;
        return $self->get_templateFilename();
    }
    sub getTemplateLineNumber {
        my $self = shift;
        return $self->get_templateLinenumber();
    }
    sub getEffectiveTypeName {
	my $self = shift;
	return $self->get_effectiveTypeName();
    }
    sub setHierarchicalRefName {
        my($self, $hierarchicalName, $parentNode) = @_;

        my $refName = '';
        my $hierRefName = '';
        my $tmpName = '';
        my $subHierNameCount=0;
        my $numExpandedNames=0;
        my (@parentArrayIndex);
        my ($memberNameHashListRef, $expandedMemNameList, $regName);
        my ($tmpRegName, $byteEndian, $aliasOf, $bogusAddr, $hierName);
        my ($aliasedRegHash, $regHashRef, @levelNameList, $tmpIndex);
        my $nextExpandedRefNameHash= {};

        $aliasOf = '-';
        if($parentNode->getNodeType() eq 'field') {
            $regHashRef = $parentNode->get_parentHashRef();
        } else {
            $hierName = $parentNode->get_hierarchicalIdentifier();
            if($hierName !~ /\./) {
                $tmpRegName = $hierName;
            } else {
                @levelNameList = split('\.', $hierName);
                pop(@levelNameList);
                $tmpRegName = join('.', @levelNameList);
            }
            ($regHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($tmpRegName);
        }
        $byteEndian = $regHashRef->{'byteEndian'};
        if(defined($regHashRef->{'aliasOf'})) {
            $aliasOf = $regHashRef->{'aliasOf'};
        }
        my $refNameListRef = $visitedAddrmapsList[0]->{'refNameHashListRef'};

	return if !defined($refNameListRef);

	my $addrmapHierId = $parentNode->getParentAddressmapOrGroupNode()->getHierarchicalIdentifier();
        if ($viewName eq 'sv') {
            while ($addrmapHierId =~ /(\[\d+\])/g) {
                push (@parentArrayIndex, $1);
            }
            $addrmapHierId =~ s/\[\d+\]//g;
        } else {
            $addrmapHierId =~ s/\[\d+\]//g;
        }
	my $found = 0;
        for(my $refNameCount=0; $refNameCount < @$refNameListRef; $refNameCount++) {
            $nextExpandedRefNameHash = $refNameListRef->[$refNameCount];
            if(((defined($regHashRef->{'aliasOf'}) &&
                 defined($aliasedRegHash = $nextExpandedRefNameHash->{'aliasedReg'}) &&
                 ($regHashRef->{'name'} =~ /$aliasedRegHash->{'name'}/)) ||
                (!defined($regHashRef->{'aliasOf'}) &&
                 !defined($nextExpandedRefNameHash->{'aliasedReg'}))) &&
               ($hierarchicalName eq $nextExpandedRefNameHash->{'hierarchicalRefName'}) &&
	       ($addrmapHierId eq $nextExpandedRefNameHash->{'addressmapHierarchy'})) {

		next if (defined($nextExpandedRefNameHash->{'parentName'}) &&
			 ($parentNode->getName() !~ /$nextExpandedRefNameHash->{'parentName'}/));

		$found = 1;
                $memberNameHashListRef = $nextExpandedRefNameHash->{'expandedNameList'};
		my $memberIndexWasResetFlag = 0;
		my $hasArrayMember = 0;
		my $updatedFlag = 0;
                for ($subHierNameCount = @{$memberNameHashListRef}-1;
		     $subHierNameCount >= 0;
		     $subHierNameCount--) {
                    $tmpName = '';
		    if (defined($memberNameHashListRef->[$subHierNameCount]->{'size'})) {
			# member is array
			if (!$updatedFlag) {
			    $hasArrayMember = 1;
			    $tmpName = $self->getArrayMemberName($memberNameHashListRef,
								 $subHierNameCount);
			    if ($self->isLeafArrayMember($memberNameHashListRef,
							 $subHierNameCount)) { # lowest member
				$self->updateArrayMemberIndex($memberNameHashListRef,
							      $subHierNameCount);
			    }
			    $updatedFlag = 1;
			} else {
			    $tmpName = $self->getArrayMemberName($memberNameHashListRef,
								 $subHierNameCount);
			}
                    } else {
                        $tmpName = $memberNameHashListRef->[$subHierNameCount]->{'memberName'};
                    }
                    if($refName ne '') {
                        $refName = join('_' , $tmpName, $refName);
                        $hierRefName = join('.' , $tmpName, $hierRefName);
                    } else {
                        $refName = $tmpName;
                        $hierRefName = $tmpName;
                    }
		}
		if ($hasArrayMember) {
		    for ($subHierNameCount = @{$memberNameHashListRef}-1;
			 $subHierNameCount >= 0;
			 $subHierNameCount--) {
			if (defined($memberNameHashListRef->[$subHierNameCount]->{'size'})) {
			    if ($self->isMemberArrayIndexGTMax($memberNameHashListRef,
							       $subHierNameCount)) {
				# this member has incremented to the max array value.
				$self->resetArrayMemberIndex($memberNameHashListRef,
							     $subHierNameCount);
				$self->updateParentMember($memberNameHashListRef,
							  $subHierNameCount);
			    }
			}
		    }
		}		    
		last if $found;
            }
        }
        if ($viewName ne 'sv') {
            $refName =~ s/\[/_/g;
            $refName =~ s/\]//g;
        } else {
            my @arrayIndex;
            while ($hierRefName =~ /(\[\d+\])/g) {
                push (@arrayIndex, $1);
            }
        }
	$self->set_hierarchicalName($refName);
	$self->set_name($refName);
        $self->set_hierarchicalNameDotted($hierRefName);
        $self->set_hierarchicalRefName($hierRefName);
    }

    sub isLeafArrayMember {
	my ($self,
	    $memberNameHashListRef,
	    $subHierNameCount) = @_;

	# Is there another member, lower in the hierarchicaly name
	# than this name, that is an array member? In other words,
	# is this the lowest member in the hierarchical name that is
	# an array member?

	my $arrayMemberCount = 0;
	for (my $index = @{$memberNameHashListRef}-1; $index >=0; $index--) {
	    if (defined($memberNameHashListRef->[$index]->{'size'})) {
		$arrayMemberCount++;
		return 0 if ($index > $subHierNameCount);
	    }
	}
	if ($arrayMemberCount > 0) {
	    return 1;
	} else {
	    return 0;
	}
    }
    sub updateParentMember {
	my ($self,
	    $memberNameHashListRef,
	    $subHierNameCount) = @_;

	for (my $tmpCount = $subHierNameCount-1; $tmpCount >= 0; $tmpCount--) {
	    if (defined($memberNameHashListRef->[$tmpCount]->{'size'})) {
		$self->updateArrayMemberIndex($memberNameHashListRef,
					      $tmpCount);
		if ($designDBRef->_compareArrayIndex($memberNameHashListRef->[$tmpCount]->{'memberIndex'},
						     $memberNameHashListRef->[$tmpCount]->{'arrayMaxIndices'}) == 1) {
		    $self->updateParentMember($memberNameHashListRef,
					      $tmpCount);
		} else {
		    return;
		}
	    }
	}
    }

    sub parentMemberWasUpdated {
	my ($self,
	    $memberNameHashListRef,
	    $subHierNameCount) = @_;

	for (my $tmpCount = $subHierNameCount-1; $tmpCount >= 0; $tmpCount--) {
	    if (defined($memberNameHashListRef->[$tmpCount]->{'size'})) {
		return 1 if ($designDBRef->_compareArrayIndex($memberNameHashListRef->[$tmpCount]->{'memberIndex'},
							      $memberNameHashListRef->[$tmpCount]->{'arrayMinIndices'}) == 1);
		return 0;
	    }
	}
	return 0;
    }
    sub isMemberArrayIndexMaxed {
	my ($self,
	    $memberNameHashListRef,
	    $memberIndex) = @_;

	my $isMax = 0;
	if ($designDBRef->_compareArrayIndex($memberNameHashListRef->[$memberIndex]->{'memberIndex'},
					     $memberNameHashListRef->[$memberIndex]->{'arrayMaxIndices'}) == 0) {
	    $isMax = 1;
	} else {
	    $isMax = 0;
	}
	return $isMax;
    }
    sub isMemberArrayIndexGTMax {
	my ($self,
	    $memberNameHashListRef,
	    $memberIndex) = @_;

	my $isMax = 0;
	if ($designDBRef->_compareArrayIndex($memberNameHashListRef->[$memberIndex]->{'memberIndex'},
					     $memberNameHashListRef->[$memberIndex]->{'arrayMaxIndices'}) == 1) {
	    $isMax = 1;
	} else {
	    $isMax = 0;
	}
	return $isMax;
    }
    sub getArrayMemberName {
	my ($self,
	    $memberNameHashListRef,
	    $memberIndex) = @_;

        my $tmpName = '';
	my @tmpIndex = @{$memberNameHashListRef->[$memberIndex]->{'memberIndex'}};
	if (!$updateRefArrayIndex) { # get the state before the previous update
	    $designDBRef->_decrementArrayIndex(\@tmpIndex,
					       $memberNameHashListRef->[$memberIndex]->{'arrayInfo'});
	    if ($designDBRef->_compareArrayIndex(\@tmpIndex,
						 $memberNameHashListRef->[$memberIndex]->{'arrayMinIndices'}) < 0) {
		@tmpIndex = @{$memberNameHashListRef->[$memberIndex]->{'arrayMinIndices'}};
	    }
	}
	$tmpName = $designDBRef->_getArrayElementName($memberNameHashListRef->[$memberIndex]->{'memberName'},
						      \@tmpIndex);
	return $tmpName;
    }
    sub resetArrayMemberIndex {
	my ($self,
	    $memberNameHashListRef,
	    $memberIndex) = @_;

	$memberNameHashListRef->[$memberIndex]->{'memberIndex'} =
	    $designDBRef->_getArrayMinIndices($memberNameHashListRef->[$memberIndex]->{'arrayInfo'});
    }

    sub updateArrayMemberIndex {
	my ($self,
	    $memberNameHashListRef,
	    $memberIndex) = @_;

	$designDBRef->_incrementArrayIndex($memberNameHashListRef->[$memberIndex]->{'memberIndex'},
					   $memberNameHashListRef->[$memberIndex]->{'arrayInfo'}) if ($updateRefArrayIndex);
    }
    sub getNextPortNode {
	my ($self,
	    $portListRef,
	    $currPortIndexRef) = @_;

	return undef if(!defined($portListRef));
	return undef if ($$currPortIndexRef >= @{$portListRef});
	return $portListRef->[$$currPortIndexRef++];
    }
    sub getNextExternalPortNode {
	my ($self,
	    $portListRef,
	    $currPortIndexRef) = @_;

	return undef if(!defined($portListRef));
	return undef if ($$currPortIndexRef >= @{$portListRef});
	return $portListRef->[$$currPortIndexRef++];
    }
    sub hasAttributes {
        my $self = shift;
        my $attributes = '';

        if (defined($attributes = $self->get_attributes()) &&
            ($attributes ne '')) {
            return 1;
        } else {
            return 0;
        }
    }
    sub hasAttribute {
        my ($self, $attrName) = @_;

        my ($attrHashRef);
        if ($self->hasAttributes()) {
            $attrHashRef = $self->get_attributes();
            foreach my $nextAttr (@{$attrHashRef}) {
                my @attrKeys = keys %{$nextAttr};
                foreach my $nextAttrKey (@attrKeys) {
                    if ($attrName eq $nextAttrKey) {
                        return 1;
                    }
                }
            }
        }
        return 0;
    }   
    sub attributeHasValue {
        my ($self, $attrName) = @_;

        my ($attrValue, $attrHashRef);
        if ($self->hasAttributes()) {
            $attrHashRef = $self->get_attributes();
            foreach my $nextAttr (@{$attrHashRef}) {
                my @attrKeys = keys %{$nextAttr};
                my @attrValues = values %{$nextAttr};
                foreach my $nextAttrKey (@attrKeys) {
                    if ($attrName eq $nextAttrKey) {
                        if (defined($attrValue = $nextAttr->{$attrName}) &&
                            ($attrValue ne '')) {
                            return 1;
                        }
                    }
                }
            }
        }
        return 0;
    }   
    sub getAttributeValue {
        my ($self, $attrName) = @_;

        my ($attrHashRef);
        if ($self->hasAttributes()) {
            $attrHashRef = $self->get_attributes();
            foreach my $nextAttr (@{$attrHashRef}) {
                if (defined($nextAttr->{$attrName})) {
                    return $nextAttr->{$attrName};
                }
            }
        }
        return;
    }   
    sub getAttributeType {
        my ($self, $attrName) = @_;

        my ($attrHashRef);
        if ($self->hasAttributes()) {
            $attrHashRef = $self->get_attributes();
            foreach my $nextAttr (@{$attrHashRef}) {
                my @attrKeys = keys %{$nextAttr};
                my @attrValues = values %{$nextAttr};
                foreach my $nextAttrKey (@attrKeys) {
                    if ($attrName eq $nextAttrKey) {
                        return $nextAttr->{'smfrAttrValueType'};
                    }
                }
            }
        }
        return;
    }   
    sub getAttributeNames {
        my $self = shift;

        my (@attrKeys, @attrValues, $attrHashRef, @names);
        if ($self->hasAttributes()) {
            $attrHashRef = $self->get_attributes();
            for (my $index=0; $index < @{$attrHashRef}; $index++) {
                my $nextAttr = $attrHashRef->[$index];
                my @attrKeys = keys %{$nextAttr};
                foreach my $nextAttrKey (@attrKeys) {
                    if ($nextAttrKey ne 'smfrAttrValueType') {
                        push (@names, $nextAttrKey);
                    }
                }
            }
            return \@names;
        }
        return;
    }   
    sub hasParityProtect {
	my $self = shift;
	my $pp;
	return 1 if (defined($pp = $self->get_parityProtect()) &&
		     ($pp eq 'true'));
	return 0;
    }
    sub hasROAccess {
        my $self = shift;
        my $accessType;
	return 1 if($self->hasReadAccess() && !$self->hasWriteAccess());
        return 0;
    }
    sub hasWOAccess {
        my $self = shift;
        my $accessType;
	return 1 if(!$self->hasReadAccess() && $self->hasWriteAccess());
        return 0;
    }
    sub hasRWAccess {
        my $self = shift;
        my $accessType;
        if(defined($accessType = $self->getAccessType()) &&
           ($accessType =~ /rw|wr/)) {
            return 1;
        }
        return 0;
    }
    sub hasReadAccess {
        my $self = shift;
        my $accessType;
        if(defined($accessType = $self->getAccessType()) &&
           ($accessType =~ /r|rw|wr/)) {
            return 1;
        }
        return 0;
    }
    sub hasWriteAccess {
        my $self = shift;
        my $accessType;
        if(defined($accessType = $self->getAccessType()) &&
           ($accessType =~ /w|rw|wr/)) {
            return 1;
        }
        return 0;
    }

    sub isAddressmapNode {
        my $self = shift;

        my $nodeType = $self->getNodeType();
        if (($nodeType eq 'addressmapInstance') ||
            ($nodeType eq 'addressmap')) {
            return 1;
        }
        return 0;
    }
    sub isGroupNode {
	my $self = shift;
	my $nodeType;
	return 1 if (defined($nodeType = $self->getNodeType()) &&
		     ($nodeType eq 'group'));
	return 0;
    }
    sub isUnionNode {
	my $self = shift;
	my $nodeType;
	return 1 if (defined($nodeType = $self->getNodeType()) &&
		     ($nodeType eq 'union'));
	return 0;
    }
    sub isMemoryNode {
	my $self = shift;
	my $nodeType;
	return 1 if (defined($nodeType = $self->getNodeType()) &&
		     ($nodeType eq 'memory'));
	return 0;
    }
    sub isWideRegisterNode {
        my $self = shift;
        my $nodeType;
        return 1 if (defined($nodeType = $self->getNodeType()) &&
                     ($nodeType eq 'wideRegister'));
        return 0;
    }
    sub isRegisterNode {
        my $self = shift;
        my $nodeType;
        return 1 if (defined($nodeType = $self->getNodeType()) &&
                     ($nodeType eq 'register'));
        return 0;
    }
    sub isVirtual {
	my $self = shift;
	my $isVirtual;
        return 1 if (($self->isRegisterNode() || $self->isWideRegisterNode()) &&
                     defined($isVirtual = $self->get_virtualRegister()) &&
                     ($isVirtual eq 'true'));
        return 0;
    }
    sub isExternal {
        my $self = shift;
        my $external;
        return 1 if(defined($external = $self->get_external()) &&
                    ($external eq 'true'));
        return 0;
    }
    sub isDeclaredExternal {
        my $self = shift;
        my $declaredExternal;
        return 1 if(defined($declaredExternal = $self->get_declaredExternal()) &&
                    ($declaredExternal eq 'true'));
        return 0;
    }
    sub isVolatile {
        my $self = shift;
        return 1 if($self->get_isVolatile() eq 'true');
        return 0;
    }
    sub isPrivate {
        my $self = shift;
        my $isPrivate;
        if (defined($isPrivate = $self->get_isPrivate())) {
            return $isPrivate;
        }
        return 0;
    }
    sub getPrivacyLevel {
        my $self = shift;
        my $privacyLevel;
        if (defined($privacyLevel = $self->get_privacyLevel())) {
            return $self->get_privacyLevel();
        }
        return 0;
    }
    sub getPrivacySet {
        my $self = shift;
        my $privacySet;
        if (defined($privacySet = $self->get_privacySet())) {
            return $self->get_privacySet();
        }
        return 0;
    }
    sub saveNextAddressmapIndex {
        my $self = shift;
        $nextAddressmapIndexShadow = $nextAddressmapIndex;
    }
    sub restoreNextAddressmapIndex {
        my $self = shift;
        $nextAddressmapIndex = $nextAddressmapIndexShadow;
    }
    sub saveAddrmapInfoContext {
        my $self = shift;
        my @tmpList = @visitedAddrmapsList;
        push(@visitedAddrmapsListShadow, \@tmpList);
        @visitedAddrmapsList = ();
    }
    sub restoreAddrmapInfoContext {
        my $self = shift;
        my $addrmapList = pop(@visitedAddrmapsListShadow);
        @visitedAddrmapsList = @{$addrmapList};
    }
    sub childOfExternalObject {
        my $self = shift;

        my $currObj = $self;
        my ($parentObj, $external);
        while (defined($parentObj = $currObj->getParentAddressmapNode())) {
	    return 1 if ($parentObj->isExternal());
            $currObj = $parentObj;
        }
        return 0;
    }

    sub getParentAddressmapNode {
        my $self = shift;

        # get the parent address map object for the calling object.

        my $count = 0;
	my ($verilogFormat, $relAddr, $arrayIndex);
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage);
        my (@prefixList, $parentHashRef, $parentOfParentHashRef);
        my ($topLevelHashRef, $addrmapObj, $parentName, $bogusAddr);

        my $bigIntAddr = Math::BigInt->new('0');
        my $hierId = $self->getHierarchicalIdentifier();
        my @levelNameList = split('\.', $self->get_hierarchicalIdentifier());
        my @hierPrefixList = split('\.', $self->get_hierarchicalNameDotted());
	if (defined($designDBRef->{'component'})) {
	    return if (@levelNameList <= 2);
	} else {
	    return if (@levelNameList <= 1);
	}
        ($topLevelHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);
        if ($viewName eq 'sv') {
            $arrayIndex = $self->getObjIndexesSV($hierId);
        }
        pop(@levelNameList);
        pop(@hierPrefixList);

        if ($self->isArray() && ($viewName eq 'sv')) {
            my $numDims = $self->getNumArrayDimensions();
            for (my $count=0; $count < $numDims; $count++) {
                pop(@$arrayIndex);
            }
        }
        # start from the last level name.
        while(@levelNameList) {
            $parentName = join('.', @levelNameList);
            ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
            if(($parentHashRef->{'nodeType'} eq 'addressmap') ||
               ($parentHashRef->{'nodeType'} eq 'addressmapInstance')) {
                ($busProtocol, $inputPipelineStage, $outputPipelineStage) =
                    $designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);
		if (@hierPrefixList == 0) {
		    $prefixList[0] = $parentName;
		} else {
		    $prefixList[0] = $hierPrefixList[@hierPrefixList-1];
		}
                if ($viewName ne 'sv') {
                    $arrayIndex = $self->getObjIndexesSV($prefixList[0]);
                    $prefixList[0] =~ s/\[/_/g;
                    $prefixList[0] =~ s/\]//g;
                }
                pop(@levelNameList);
                # parent of parent hash ref
                if ((@levelNameList > 0) && !defined($designDBRef->{'component'})) {
                    # get to the parent of parent node
                    if(@levelNameList == 1) {
                        $parentName = $levelNameList[0];
                    } else {
                        $parentName = join('.', @levelNameList);
                    }
                    ($parentOfParentHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($parentName);
                }
		($verilogFormat, $relAddr) =
		    $self->getVerilogBaseAndValue($parentHashRef->{'addressmapRelativeByteAddress'});
		my $relAddrBigInt = Math::BigInt->new($relAddr);
                if (($viewName eq 'sv') && (@$arrayIndex))  {
                    my $tmpIndex='';
                    for (my $count=0; $count < @$arrayIndex; $count++) {
                        $tmpIndex .= '[' . $arrayIndex->[$count] . ']';
                    }
                    $prefixList[@prefixList-1] .= $tmpIndex;
                    $hierPrefixList[@hierPrefixList-1] .= $tmpIndex;
                }
                $addrmapObj = csrPerlAPIModule->new($parentHashRef,
						    $arrayIndex,
                                                    $parentOfParentHashRef,
                                                    $bigIntAddr,
						    $relAddrBigInt,
                                                    $busProtocol,
                                                    $inputPipelineStage,
                                                    $outputPipelineStage,
                                                    \@prefixList,
						    \@hierPrefixList);
                return $addrmapObj;
            }
            if (defined($parentHashRef->{'arrayDimensionInfo'})) {
                my $numDims = scalar(@{$parentHashRef->{'arrayDimensionInfo'}});
                for (my $count=0; $count < $numDims; $count++) {
                    pop(@$arrayIndex);
                }
            }
            pop(@levelNameList);
            pop(@hierPrefixList);
        }
        return;
    }

    sub getParentAddressmapOrGroupNode {
        my $self = shift;

        # get the parent address map object for the calling object.

        my $count = 0;
	my ($verilogFormat, $relAddr, $arrayIndex);
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage);
        my (@prefixList, $parentHashRef, $parentOfParentHashRef);
        my ($topLevelHashRef, $addrmapObj, $parentName, $bogusAddr);

        my $bigIntAddr = Math::BigInt->new('0');
        my $hierId = $self->getHierarchicalIdentifier();
        my @levelNameList = split('\.', $self->get_hierarchicalIdentifier());
        my @levelNameListHierName = split('\.', $self->get_hierarchicalNameDotted());

        return if(@levelNameList <= 1);

        ($topLevelHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);
        if ($viewName eq 'sv') {
            $arrayIndex = $self->getObjIndexesSV($hierId);
        }
        pop(@levelNameList);
        pop(@levelNameListHierName);

        if ($self->isArray() && ($viewName eq 'sv')) {
            my $numDims = $self->getNumArrayDimensions();
            for (my $count=0; $count < $numDims; $count++) {
                pop(@$arrayIndex);
            }
        }
        # start from the last level name.
        while(@levelNameList) {
            $parentName = join('.', @levelNameList);
            ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
            if(($parentHashRef->{'nodeType'} eq 'addressmap') ||
               ($parentHashRef->{'nodeType'} eq 'addressmapInstance') ||
               ($parentHashRef->{'nodeType'} eq 'group') ||
               ($parentHashRef->{'nodeType'} eq 'union')) {
                ($busProtocol, $inputPipelineStage, $outputPipelineStage) =
                    $designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);
                pop(@levelNameList);
                # parent of parent hash ref
                if(@levelNameList > 0) {
                    # get to the parent of parent node
                    if(@levelNameList == 1) {
                        $parentName = $levelNameList[0];
                    } else {
                        $parentName = join('.', @levelNameList);
                    }
                    ($parentOfParentHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($parentName);
                }
                if (@levelNameListHierName == 0) {
		    $prefixList[0] = $parentName;
		} else {
		    $prefixList[0] = $levelNameListHierName[@levelNameListHierName-1];
		}
                if ($viewName ne 'sv') {
                    $arrayIndex = $self->getObjIndexesSV($prefixList[0]);
                    $prefixList[0] =~ s/\[/_/g;
                    $prefixList[0] =~ s/\]//g;
                }
		($verilogFormat, $relAddr) =
		    $self->getVerilogBaseAndValue($parentHashRef->{'addressmapRelativeByteAddress'});
		my $relAddrBigInt = Math::BigInt->new($relAddr);
                if (($viewName eq 'sv') && (@$arrayIndex)) {
                    my $tmpIndex;
                    for (my $count=0; $count < @$arrayIndex; $count++) {
                        $tmpIndex .= '[' . $arrayIndex->[$count] . ']';
                    }
                    if (@prefixList) {
                        $prefixList[@prefixList-1] .= $tmpIndex;
                    }
                    $levelNameListHierName[@levelNameListHierName-1] .= $tmpIndex;
                }
		if (($parentHashRef->{'nodeType'} eq 'addressmap') ||
		    ($parentHashRef->{'nodeType'} eq 'addressmapInstance')) {
		    $addrmapObj = csrPerlAPIModule->new($parentHashRef,
							$arrayIndex,
							$parentOfParentHashRef,
							$bigIntAddr,
							$relAddrBigInt,
							$busProtocol,
							$inputPipelineStage,
							$outputPipelineStage,
							\@prefixList,
							\@levelNameListHierName);
		} elsif ($parentHashRef->{'nodeType'} eq 'group') {
		    $addrmapObj = csrPerlAPIGroup->new($parentHashRef,
						       $arrayIndex,
						       $parentOfParentHashRef,
						       $bigIntAddr,
						       $relAddrBigInt,
						       $busProtocol,
						       $inputPipelineStage,
						       $outputPipelineStage,
						       \@prefixList,
						       \@levelNameListHierName);
		} else { # union
		    $addrmapObj = csrPerlAPIUnion->new($parentHashRef,
						       $arrayIndex,
						       $parentOfParentHashRef,
						       $bigIntAddr,
						       $relAddrBigInt,
						       $busProtocol,
						       $inputPipelineStage,
						       $outputPipelineStage,
#						       \@levelNameListHierName,
						       \@prefixList,
						       \@levelNameListHierName);
		}
                return $addrmapObj;
            }
            if (defined($parentHashRef->{'arrayDimensionInfo'})) {
                my $numDims = scalar(@{$parentHashRef->{'arrayDimensionInfo'}});
                for (my $count=0; $count < $numDims; $count++) {
                    pop(@$arrayIndex);
                }
            }
            pop(@levelNameList);
	    pop(@levelNameListHierName);
        }
        return;
    }

    sub getExternalParentObject {
        my $self = shift;

        # get the external parent address map object for the calling object.

        my $count = 0;
	my ($relAddr, $bigIntOffset, $verilogFormat, $arrayIndex);
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage);
        my (@prefixList, $parentHashRef, $parentOfParentHashRef);
        my ($topLevelHashRef, $extObj, $parentName, $bogusAddr);

        my $bigIntAddr = Math::BigInt->new('0');
        my $hierId = $self->getHierarchicalIdentifier();
        my @levelNameList = split('\.', $self->get_hierarchicalIdentifier());
        my @levelNameListHierName = split('\.', $self->get_hierarchicalNameDotted());
        return if(@levelNameList <= 1);

        ($topLevelHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);
        if ($viewName eq 'sv') {
            $arrayIndex = $self->getObjIndexesSV($hierId);
        }
        pop(@levelNameList);
        pop(@levelNameListHierName);
        if ($self->isArray()) {
            my $numDims = $self->getNumArrayDimensions();
            for (my $count=0; $count < $numDims; $count++) {
                pop(@{$arrayIndex});
            }
        }

        # start from the last level name.
        while(@levelNameList) {
            $parentName = join('.', @levelNameList);
            ($parentHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($parentName);
            if ((($parentHashRef->{'nodeType'} eq 'addressmap') ||
               ($parentHashRef->{'nodeType'} eq 'addressmapInstance') ||
               ($parentHashRef->{'nodeType'} eq 'group') ||
               ($parentHashRef->{'nodeType'} eq 'union')) &&
                ($parentHashRef->{'external'} eq 'true')) {
                ($busProtocol, $inputPipelineStage, $outputPipelineStage) =
                    $designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);
		if (($parentHashRef->{'nodeType'} eq 'union') &&
		    defined($designDBRef->{'component'})) {
		    my $topAddrmap = $self->getTopLevelAddressmapNode();
		    my $topAddrmapId = $topAddrmap->getHierarchicalIdentifier();
		    my $prefix = $self->getHierarchicalIdentifier();
		    $prefix =~ s/$topAddrmapId\.//;
		    @prefixList = split('\.', $prefix);
		    pop(@prefixList);
		} else {
                    $prefixList[0] = $levelNameListHierName[@levelNameListHierName-1];
                    if ($viewName ne 'sv') {
                        $prefixList[0] =~ s/\[/_/g;
                        $prefixList[0] =~ s/\]//g;
                    }
		}
                pop(@levelNameList);
                # parent of parent hash ref
                if(@levelNameList > 0) {
                    # get to the parent of parent node
                    if(@levelNameList == 1) {
                        $parentName = $levelNameList[0];
                    } else {
                        $parentName = join('.', @levelNameList);
                    }
                    ($parentOfParentHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($parentName);
                }
		($verilogFormat, $relAddr) =
		    $self->getVerilogBaseAndValue($parentHashRef->{'addressmapRelativeByteAddress'});
		my $relAddrBigInt = Math::BigInt->new($relAddr);
                if (($viewName eq 'sv') && (@$arrayIndex)) {
                    my $tmpIndex;
                    for (my $count=0; $count < @$arrayIndex; $count++) {
                        $tmpIndex .= '[' . $arrayIndex->[$count] . ']';
                    }
                    $prefixList[@prefixList-1] .= $tmpIndex;
                    $levelNameListHierName[@levelNameListHierName-1] .= $tmpIndex;
                }
		if (($parentHashRef->{'nodeType'} eq 'addressmap') ||
		    ($parentHashRef->{'nodeType'} eq 'addressmapInstance')) {
		    $extObj = csrPerlAPIModule->new($parentHashRef,
						    $arrayIndex,
						    $parentOfParentHashRef,
						    $bigIntAddr,
						    $relAddrBigInt,
						    $busProtocol,
						    $inputPipelineStage,
						    $outputPipelineStage,
						    \@prefixList,
						    \@levelNameListHierName);
		} elsif ($parentHashRef->{'nodeType'} eq 'group') {
		    $extObj = csrPerlAPIGroup->new($parentHashRef,
						   $arrayIndex,
						   $parentOfParentHashRef,
						   $bigIntAddr,
						   $relAddrBigInt,
						   $busProtocol,
						   $inputPipelineStage,
						   $outputPipelineStage,
						   \@prefixList,
						   \@levelNameListHierName);
		} elsif ($parentHashRef->{'nodeType'} eq 'union') {
		    $extObj = csrPerlAPIUnion->new($parentHashRef,
						   $arrayIndex,
						   $parentOfParentHashRef,
						   $bigIntAddr,
						   $relAddrBigInt,
						   $busProtocol,
						   $inputPipelineStage,
						   $outputPipelineStage,
						   \@prefixList,
						   \@levelNameListHierName);
		} else {
		    ($verilogFormat, $bigIntOffset) = $self->getVerilogBaseAndValue($parentHashRef->{'byteOffset'});
		    $extObj = csrPerlAPIMemory->new($parentHashRef,
						    $arrayIndex,
						    $parentOfParentHashRef,
						    $bigIntAddr,
						    $bigIntOffset,
						    $relAddrBigInt,
						    $busProtocol,
						    $inputPipelineStage,
						    $outputPipelineStage,
						    \@prefixList,
						    \@levelNameListHierName);
		}
                return $extObj;
            }
            if (defined($parentHashRef->{'arrayDimensionInfo'})) {
                my $numDims = scalar(@{$parentHashRef->{'arrayDimensionInfo'}});
                for (my $count=0; $count < $numDims; $count++) {
                    pop(@$arrayIndex);
                }
            }
            pop(@levelNameList);
	    pop(@levelNameListHierName);
        }
        return;
    }
    sub getRegisterNodeByName {
        my($self, $regName) = @_;

        my ($arrayIndex);
        my (@aliasPrefixList, $registerObj, $topLevelHashRef, $bogusAddr);
        my ($errMsg, $regHashRef, $parentName, $parentHashRef, $relAddr);
        my ($busProtocol, $inputPipelineStage, $outputPipelineStage);

        # This method is called by port objects that have field reference
        # information, such as interruptEnablePort, incrementCounterPort
        # (with "access" field reference type), etc.

        my $bigIntAddr = Math::BigInt->new('0');
	my @tmpNameList = split('\.', $regName);
	my @hierNameList = split('\.', $regName);
        my @levelNameList = split('\.', $regName);
        if(@levelNameList <= 0) {
            push(@levelNameList, $regName);
        }

        my @tmpLevelNameList = @levelNameList;
        $topLevelHashRef = $designDBRef->_findAddressmapNodeByName($self->get_topLevelAddressmapName(),
                                                                   $designDBRef->get_topAddressmapHashRefNode());
        if(@levelNameList == 1) {
            # We have only a register name and no hierarchical info.
            # The register is at top level. Add the address map name
	    # to the hierarchical name list, if it isn't declared external.
            $parentHashRef = $topLevelHashRef;
            ($regHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($levelNameList[0]);
        } else {
            # at least two levels of hierarchy
            ($regHashRef, $bigIntAddr) = $designDBRef->_getHashRefForName($regName);
            pop(@tmpLevelNameList);
            $parentName = join('.', @tmpLevelNameList);
            ($parentHashRef, $bogusAddr) = $designDBRef->_getHashRefForName($parentName);
        }
        ($busProtocol, $inputPipelineStage, $outputPipelineStage) =
            $designDBRef->_getBusProtocol($topLevelHashRef, $parentHashRef);
        if(defined($regHashRef)) {
            if(defined($regHashRef->{'aliasOf'})) {
                @tmpLevelNameList = @levelNameList;
                pop(@tmpLevelNameList);
                push(@aliasPrefixList, (@tmpLevelNameList, $regHashRef->{'aliasOf'}));
            }
            my ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($regHashRef->{'byteOffset'});
            ($verilogFormat, $relAddr) =
		$self->getVerilogBaseAndValue($regHashRef->{'addressmapRelativeByteAddress'});
	    my $relAddrBigInt = Math::BigInt->new($relAddr);
	    $updateRefArrayIndex = 0;

	    if ($tmpNameList[0] eq $topLevelHashRef->{'name'}) {
		shift(@tmpNameList);
	    }
	    my $arrayIndex = undef;
            $registerObj = csrPerlAPIRegister->new($regHashRef,
						   $arrayIndex,
                                                   $parentHashRef,
                                                   $bigIntAddr,
                                                   $offset,
                                                   $relAddrBigInt,
                                                   $busProtocol,
                                                   $inputPipelineStage,
                                                   $outputPipelineStage,
						   \@tmpNameList,
                                                   \@aliasPrefixList,
						   \@hierNameList);
            return ($registerObj, $regHashRef);
        }
        return;
    }
    sub getTopLevelAddressmapNode {
        my $self = shift;
        my @prefixList = ();
	my $arrayIndex = undef;
        my ($parentAddrmapHashRef, $relAddr, @hierarchicalPrefixList);
        my $addrmapHashRef = $designDBRef->get_topAddressmapHashRefNode();
        my ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($addrmapHashRef->{'byteAddress'});
        my $bigIntAddr = Math::BigInt->new($offset);
	($verilogFormat, $relAddr) =
	    $self->getVerilogBaseAndValue($addrmapHashRef->{'addressmapRelativeByteAddress'});
	my $relAddrBigInt = Math::BigInt->new($relAddr);
	push (@hierarchicalPrefixList, $addrmapHashRef->{'hierarchicalIdentifier'});
        my $addrmapObj = csrPerlAPIModule->new($addrmapHashRef,
					       $arrayIndex,
                                               $parentAddrmapHashRef,
                                               $bigIntAddr,
					       $relAddrBigInt,
                                               $addrmapHashRef->{'busProtocol'},
                                               $addrmapHashRef->{'inputPipelineStage'},
                                               $addrmapHashRef->{'outputPipelineStage'},
                                               \@prefixList,
					       \@hierarchicalPrefixList);
        return $addrmapObj;
    }
    sub getTopLevelAddressmapCount {
        my $self = shift;
        return $designDBRef->getTopLevelAddressmapCount();
    }
    sub getVerilogBaseAndValue {
        my ($self, $verilogNumber) = @_;

        my $base;
        my $bigIntBase;
        my $pureNumber;
        my $verilogFormat;
        my ($hexNumber, $tmpNumber);

        if($verilogNumber =~ /^\d+$/) { # decimal format
            $verilogFormat = '';
            $base = 'd'; #decimal
            $pureNumber = $verilogNumber;
        } else {
            $verilogNumber =~ m/((\d+)\'(b|h|d|o))([0-9a-fA-F]+)/;
            $base = $3;
            $verilogFormat = $2 . "'" . $base;
            $pureNumber = $4;
        }
        if($base eq 'b') {
            $bigIntBase = '0b';
            $tmpNumber = Math::BigInt->new($bigIntBase . $pureNumber);
        } elsif ($base eq 'h') {
            # the number is already in hex
            $bigIntBase = '0x';
            $tmpNumber = Math::BigInt->new($bigIntBase . $pureNumber);
        } elsif ($base eq 'o') {
            $bigIntBase = '0';
            $tmpNumber = Math::BigInt->new($bigIntBase . oct($pureNumber));
        } else { # decimal base
            $tmpNumber = Math::BigInt->new($pureNumber);
        }
        return($verilogFormat, $tmpNumber);
    }
    sub getObjIndexesSV {
        my ($self, $hierId) = @_;
        # given the hierarchical identifier, get the SystemVerilog format
        # object indexes.
        my @arrayIndex;
        while ($hierId =~ /\[(\d+)\]$/) {
            my $index = $1;
            unshift (@arrayIndex, $index);
            $hierId =~ s/(\[\d+\])$//;
        }
        return \@arrayIndex;
    }
    sub assignAddressInfo {
        my($self,
	   $nodeAddr,
	   $nodeByteOffset,
	   $nodeOffset,
	   $addrmapRelByteAddr) = @_;

        my $addrBigInt = Math::BigInt->new($nodeAddr);
        $self->set_byteAddress($addrBigInt);
        my ($verilogFormat, $value) = $self->getVerilogBaseAndValue($nodeByteOffset);
        my $byteOffsetBigInt = Math::BigInt->new($value);
        $self->set_byteOffset($byteOffsetBigInt);
	if (defined($nodeOffset)) {
	    ($verilogFormat, $value) = $self->getVerilogBaseAndValue($nodeOffset);
	    my $offsetBigInt = Math::BigInt->new($value);
	    $self->set_offset($offsetBigInt);
	}
        $self->set_addressmapRelativeByteAddress($addrmapRelByteAddr);
    }

    sub AUTOLOAD {
        no strict 'refs';

        my $attrName;
        my($self, $newVal) = @_;

        my $attr = $AUTOLOAD;
        $attr =~ s/.*:://;
        return unless $attr =~ /[^A-Z]/; # skip DESTROY and other all-caps

        if($AUTOLOAD =~ /.*::get(_\w+)/) {
            $attrName = $1;
            *{$AUTOLOAD} = sub {
                return $_[0]->{$attrName};
            };
            return $self->{$attrName};
        } elsif($AUTOLOAD =~ /.*::set(_\w+)/) {
            $attrName = $1;
            *{$AUTOLOAD} = sub {
                $_[0]->{$attrName} = $_[1];
                return;
            };
            $self->{$attrName} = $newVal;
            return;
        }
        die 'Error - unrecognized function call ' . $AUTOLOAD . "\n";
        return;
    }

    sub getReferenceInfo {
        my $self = shift;

        # Get the field and register nodes in the name of the
        # calling object (the calling object is a port object).
        # This method is intended for field reference names.

        my ($fieldHashRef, $tmpFieldName, $regName, $fieldName);
        my ($errMsg, $regNode, $fieldNode, $refIndex, $regHashRef);

        my $hierRefName = $self->get_hierarchicalRefName();
        my @levelNameList = split('\.', $hierRefName);
        if(@levelNameList <= 0) {
            push(@levelNameList, $hierRefName);
        }
        my @tmpLevelNameList = @levelNameList;
        foreach my $nextRef (@referenceInfoList) {
            if($nextRef->{'referenceName'} eq $hierRefName) {
                return ($nextRef->{'registerObject'}, $nextRef->{'fieldObject'});
            }
        }
        my $arrayIndex;
        my $srcTypeFieldRef = $self->isSourceTypeFieldReference();
        if ($srcTypeFieldRef) {
            if ($viewName eq 'sv') {
                my $referentObj = $self->get_referentObj();
                $arrayIndex = $referentObj->getParentArrayIndexFromHierId($hierRefName);
            }
            pop(@tmpLevelNameList);
        }
	$regName = join('.', @tmpLevelNameList);
        if (($viewName eq 'sv') && defined($arrayIndex)) {
            for (my $count=(@$arrayIndex -1); $count >=0; $count--) {
                $regName .= '[' . $arrayIndex->[$count] . ']';
            }
        }
        ($regNode, $regHashRef) = $self->getRegisterNodeByName($regName);
        if (!defined($regNode)) {
            $errMsg = 'Internal Error: Referenced register: ' . $regName .
                ' not found in the list!';
            my $i=0;
            while (my($pack, $file, $line, $subname, $hasargs, $wantarray) = caller($i++)) {
                print 'In file: ' . $file . ', line \#: ' . $line .
		    ', subroutine name: ' . $subname . "\n";
            }
            $self->_printError($errMsg);
            exit(1);
        }
        if (!$srcTypeFieldRef) {
	    return ($regNode, $fieldNode);
	}
        $fieldName = $levelNameList[@levelNameList-1];
        my $fieldListRef = $regNode->getBitfields();
        for(my $count=0; $count < @{$fieldListRef}; $count++) {
            $fieldNode = $fieldListRef->[$count];
	    my @fieldLevelNameList = split('\.', $fieldNode->getHierarchicalIdentifier());
            my $tmpFieldName = pop(@fieldLevelNameList);
            if ($viewName eq 'sv') {
                if ($fieldNode->isArray()) {
                    my $regNumDim = $fieldNode->getNumArrayDimensions();
                    my @indexList;
                    for (my $count = 0; $count < $regNumDim; $count++) {
                        $tmpFieldName =~ /(\[\d+\])$/;
                        unshift (@indexList, $1);
                        $tmpFieldName =~ s/(\[\d+\])$//;
                    }
                    $tmpFieldName .= join('', @indexList);
                } elsif ($tmpFieldName =~ /(\[\d+\])$/) {
                    $tmpFieldName =~ s/\[\d+\]$//g;
                }
            }
            if($tmpFieldName eq $fieldName) {
                last;
            }
        }
        if(!defined($fieldNode)) {
            $errMsg = 'Internal Error: Referenced field: ' . $fieldName .
                ' not found in the list!';
            $self->_printError($errMsg);
            exit(1);
        }
	$refIndex = @referenceInfoList;
        $referenceInfoList[$refIndex]->{'referenceName'} = $self->get_hierarchicalRefName();
        $referenceInfoList[$refIndex]->{'registerObject'} = $regNode;
        $referenceInfoList[$refIndex]->{'fieldObject'} = $fieldNode;
        return ($regNode, $fieldNode);
    }

    sub removeDuplicatePorts {
        my($self, $portListRef, $newPortList) = @_;

        for(my $portIndex=0; $portIndex < @$portListRef;) {
	    if (($portListRef->[$portIndex]->getPortDirection() eq 'input') &&
		($portListRef->[$portIndex]->getSourceType(0) eq 'number')) {
		$portIndex++;
		next;
	    }
	    if ($portListRef->[$portIndex]->hasConcatFields()) {
		my $concatPortListRef = $portListRef->[$portIndex]->getConcatList();
		$self->removeDuplicatePorts($concatPortListRef, $newPortList);
		$portIndex++;
	    } elsif ($portListRef->[$portIndex]->get_isMultiPort()) {
		my $multiPortList = $portListRef->[$portIndex]->get_multiPortList();
		for (my $memIndex=0; $memIndex < @{$multiPortList}; $memIndex++) {
		    if (!$self->isPortInList($newPortList, $multiPortList->[$memIndex])) {
			my $nextPort = $multiPortList->[$memIndex];
			push (@$newPortList, $nextPort);
		    }
		}
		$portIndex++;
	    } elsif (!$self->isPortInList($newPortList, $portListRef->[$portIndex])) {
		my $nextPort = splice(@$portListRef, $portIndex, 1);
		push (@$newPortList, $nextPort);
	    } else {
		$portIndex++;
	    }
	}
    }

    sub isPortInList {
	my ($self, $newPortList, $nextPort) = @_;

	return 0 if (!defined($newPortList));
	my $portName = $nextPort->getName();
	for (my $index=0; $index < @$newPortList; $index++) {
            return 1 if($newPortList->[$index]->getName() eq $portName);
	}
	return 0;
    }

    sub setInitialAddrmapIndexes {
        my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

        $currAddrmapInfo->{'currAddrmapInstanceIndex'} = 0;
        $currAddrmapInfo->{'currAddrmapGroupIndex'} = 0;
        $currAddrmapInfo->{'currAddrmapUnionIndex'} = 0;
        $currAddrmapInfo->{'currAddrmapRegisterIndex'} = 0;
        $currAddrmapInfo->{'currAddrmapMemoryIndex'} = 0;
	$currAddrmapInfo->{'currAddrmapInstanceArrayIndex'} = -1;
        $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
        $currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
	$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
	$currAddrmapInfo->{'currAddrmapMemoryArrayIndex'} = -1;
    }

    sub setInitialWideRegisterIndexes {
        my ($self, $currAddrmapInfo, $addrmapHashRef, $wideRegHashRef) = @_;

        my $subRegHashListRef = $wideRegHashRef->{'subRegisters'};
        $currAddrmapInfo->{'currAddrmapSubRegisterIndex'} = 0;
	$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
    }
    sub setInitialWideMemoryIndexes {
        my ($self, $currAddrmapInfo, $addrmapHashRef, $wideMemHashRef) = @_;

        my $subMemHashListRef = $wideMemHashRef->{'subMemories'};
        $currAddrmapInfo->{'currAddrmapSubMemoryIndex'} = 0;
	$currAddrmapInfo->{'currAddrmapMemoryArrayIndex'} = -1;
    }
    sub resetSubRegisterInfoStack {
        my $self = shift;
        while (@visitedSubRegsList > 0) {
            pop(@visitedSubRegsList);
        }
        $self->set_currVisitedSubRegIndex(0);
    }
    sub resetAddressmapInfoStack {
        my $self = shift;
        while(@visitedAddrmapsList > 0) {
            pop(@visitedAddrmapsList);
        }
    }

    sub setHierarchicalTBFlag {
	my ($self, $flag) = @_;
        if(defined($flag)) {
            $hierarchicalTBFlag = $flag;
        } else {
            $hierarchicalTBFlag = 1;
        }
    }
    sub clearHierarchicalTBFlag {
	my $self = shift;
	$hierarchicalTBFlag = 0;
    }
    sub getHierarchicalTBFlag {
	my $self = shift;
	return $hierarchicalTBFlag;
    }
    sub setSkipExternalAddressmapsFlag {
        my($self, $flag) = @_;
        if(defined($flag)) {
            $skipExternalAddressmaps = $flag;
        } else {
            $skipExternalAddressmaps = 1;
        }
    }
    sub clearSkipExternalAddressmapsFlag {
        my $self = shift;
        $skipExternalAddressmaps = 0;
    }
    sub getSkipExternalAddressmapsFlag {
        my $self = shift;
        return($skipExternalAddressmaps);
    }

    sub setSkipDeclaredExternalAddressmapsFlag {
        my($self, $flag) = @_;
        if(defined($flag)) {
            $skipDeclaredExternalAddressmaps = $flag;
        } else {
            $skipDeclaredExternalAddressmaps = 1;
        }
    }
    sub clearSkipDeclaredExternalAddressmapsFlag {
        my $self = shift;
        $skipDeclaredExternalAddressmaps = 0;
    }
    sub getSkipDeclaredExternalAddressmapsFlag {
        my $self = shift;
        return($skipDeclaredExternalAddressmaps);
    }

    sub setSkipExternalRegistersFlag {
        my($self, $flag) = @_;
        if(defined($flag)) {
            $skipExternalRegisters = $flag;
        } else {
            $skipExternalRegisters = 1;
        }
    }
    sub clearSkipExternalRegistersFlag {
        my $self = shift;
        $skipExternalRegisters = 0;
    }
    sub getSkipExternalRegistersFlag {
        return($skipExternalRegisters);
    }
    sub setSkipDeclaredExternalRegistersFlag {
        my($self, $flag) = @_;
        if(defined($flag)) {
            $skipDeclaredExternalRegisters = $flag;
        } else {
            $skipDeclaredExternalRegisters = 1;
        }
    }
    sub clearSkipDeclaredExternalRegistersFlag {
        my $self = shift;
        $skipDeclaredExternalRegisters = 0;
    }
    sub getSkipDeclaredExternalRegistersFlag {
        return($skipDeclaredExternalRegisters);
    }

    sub setLimitExternalRegisterArrayFlag {
        my $self = shift;
        $limitExternalRegArrayFlag = 1;
    }
    sub getLimitExternalRegisterArrayFlag {
        my $self = shift;
        return $limitExternalRegArrayFlag;
    }
    sub clearLimitExternalRegisterArrayFlag {
        my $self = shift;
        $limitExternalRegArrayFlag = 0;
    }
    sub setLimitExternalGroupArrayFlag {
        my $self = shift;
        $limitExternalGroupArrayFlag = 1;
    }
    sub getLimitExternalGroupArrayFlag {
        my $self = shift;
        return $limitExternalGroupArrayFlag;
    }
    sub clearLimitExternalGroupArrayFlag {
        my $self = shift;
        $limitExternalGroupArrayFlag = 0;
    }
    sub setLimitExternalUnionArrayFlag {
        my $self = shift;
        $limitExternalUnionArrayFlag = 1;
    }
    sub getLimitExternalUnionArrayFlag {
        my $self = shift;
        return $limitExternalUnionArrayFlag;
    }
    sub clearLimitExternalUnionArrayFlag {
        my $self = shift;
	$limitExternalUnionArrayFlag = 0;
    }
    # Semifore test environment generation flag
    sub setSemiforeEnvGenFlag {
        my ($self, $value) = @_;
	if (!defined($value)) {
	    $semiforeEnvGenFlag = 1;
	} else {
	    $semiforeEnvGenFlag = $value;
	}
    }
    sub getSemiforeEnvGenFlag {
        my $self = shift;
        return $semiforeEnvGenFlag;
    }
    sub clearSemiforeEnvGenFlag {
        my $self = shift;
        $semiforeEnvGenFlag = 0;
    }
    # Semifore testbench only generation flag
    sub setSemiforeTBGenFlag {
        my $self = shift;
        $semiforeTBGenFlag = 1;
    }
    sub getSemiforeTBGenFlag {
        my $self = shift;
        return $semiforeTBGenFlag;
    }
    sub clearSemiforeTBGenFlag {
        my $self = shift;
        $semiforeTBGenFlag = 0;
    }
    sub setExpandArraysFlag {
        my ($self, $value) = @_;
	if (!defined($value)) {
	    $expandArraysFlag = 1;
	} else {
	    $expandArraysFlag = $value;
	}
    }
    sub getExpandArraysFlag {
        my $self = shift;
        return $expandArraysFlag;
    }
    sub clearExpandArraysFlag {
        my $self = shift;
        $expandArraysFlag = 0;
    }
    sub setExpandFieldArraysFlag {
        my $self = shift;
        return $expandFieldArraysFlag = 1;
    }
    sub getExpandFieldArraysFlag {
        my $self = shift;
        return $expandFieldArraysFlag;
    }
    sub clearExpandFieldArraysFlag {
        my $self = shift;
        $expandFieldArraysFlag = 0;
    }
    sub setView {
        my ($self, $vu) = @_;
        $viewName = $vu;
    }
    sub getView {
        my $self = shift;
        return $viewName;
    }
    sub setSkipContentsFlag {
        my ($self, $flag) = @_;
        if(defined($flag)) {
            $skipContentsFlag = $flag;
        } else {
            $skipContentsFlag = 1;
        }
    }
    sub getSkipContentsFlag {
        my $self = shift;
        return $skipContentsFlag;
    }
    sub clearSkipContentsFlag {
        my $self = shift;
        $skipContentsFlag = 0;
    }
    sub setIncludeWideRegistersFlag {
	my ($self, $value) = @_;
	if (!defined($value)) {
	    $includeWideRegistersFlag = 1;
	} else {
	    $includeWideRegistersFlag = $value;
	}
    }
    sub clearIncludeWideRegistersFlag {
	my $self = shift;
	$includeWideRegistersFlag = 0;
    }
    sub getIncludeWideRegistersFlag {
	my $self = shift;
	return $includeWideRegistersFlag;
    }
    sub setIncludeWideMemoriesFlag {
	my ($self, $value) = @_;
	if (!defined($value)) {
	    $includeWideMemoriesFlag = 1;
	} else {
	    $includeWideMemoriesFlag = $value;
	}
    }
    sub clearIncludeWideMemoriesFlag {
	my $self = shift;
	$includeWideMemoriesFlag = 0;
    }
    sub getIncludeWideMemoriesFlag {
	my $self = shift;
	return $includeWideMemoriesFlag;
    }
    sub getExternalPortList {
	my($self, $direction) = @_;
	my ($portObj, @portList, $portListRef);

	return if(!defined($portListRef = $self->getExternalBusPorts()));
	my $currPortIndex = 0;
	while ($portObj = $self->getNextExternalPortNode($portListRef,
							 \$currPortIndex)) {
	    push(@portList, $portObj) if($direction eq $portObj->getPortDirection());
	}
	return \@portList;
    }
    sub getDesignDBRef {
	my $self = shift;
	return $designDBRef;
    }
    sub getParentArrayIndexFromHierId {
        my ($self, $hierId) = @_;

        my @indexList;
        if ($viewName eq 'sv') {
            # if the object is an array, remove the array indices
            # for the object from the end
            if ($self->isArray()) {
                my $regNumDim = $self->getNumArrayDimensions();
                for (my $count = 0; $count < $regNumDim; $count++) {
                    $hierId =~ s/(\[\d+\])$//;
                }
            }
            while ($hierId =~ /\[(\d+)\]$/) {
                my $index = $1;
                unshift (@indexList, $index);
                $hierId =~ s/(\[\d+\])$//;
            }
        } else {
            my @tmpHierPrefixList = split('\.', $hierId);
            pop(@tmpHierPrefixList);
            if (@tmpHierPrefixList > 0) {
                my $tmpParentName = pop(@tmpHierPrefixList);
                while ($tmpParentName =~ /\[(\d+)\]$/) {
                    my $index = $1;
                    unshift (@indexList, $index);
                    $tmpParentName =~ s/(\[\d+\])$//;
                }
            }
        }
        return \@indexList;
    }
}

sub parentIsGroupArray {
    my ($self, $parentObj) = @_;

    if (($parentObj->{'nodeType'} eq 'group') &&
	defined($parentObj->{'arrayDimensionInfo'}) &&
	$designDBRef->_arraySizeGTOne($parentObj->{'arrayDimensionInfo'})) {
	return 1;
    }
    return 0;
}
sub parentIsUnionArray {
    my ($self, $addrmapHashRef) = @_;

    if (($addrmapHashRef->{'nodeType'} eq 'union') &&
	defined($addrmapHashRef->{'arrayDimensionInfo'}) &&
	$designDBRef->_arraySizeGTOne($addrmapHashRef->{'arrayDimensionInfo'})) {
	return 1;
    }
    return 0;
}

sub parentIsAddressmapInstanceArray {
    my ($self, $addrmapHashRef) = @_;

    if (($addrmapHashRef->{'nodeType'} eq 'addressmapInstance') &&
	defined($addrmapHashRef->{'arrayDimensionInfo'}) &&
	$designDBRef->_arraySizeGTOne($addrmapHashRef->{'arrayDimensionInfo'})) {
	return 1;
    }
    return 0;
}

sub getVisitedAddrmapsList {
    my $self = shift;
    return $self->get_visitedAddrmapsList();
}

sub getParentArrayRelAddr_ArrayIndex {
    my ($self,
	$addrmapHashRef,
	$parentType) = @_;

    my ($tmpAddrmapHashRef, $tmpAddrmapInfo, $relativeAddress);
    my $parentIndexType = 'currAddrmap' . "\L\u$parentType\E" . 'ArrayIndex';
    my $parentArrayIndex;

    for (my $index = @visitedAddrmapsList-1; $index >= 0; $index--) {
	$tmpAddrmapInfo = $visitedAddrmapsList[$index];
	$tmpAddrmapHashRef = $tmpAddrmapInfo->{'currAddrmapHashRef'};
	if (($tmpAddrmapHashRef->{'nodeType'} eq $parentType) &&
	    ($addrmapHashRef->{'name'} eq $tmpAddrmapHashRef->{'name'})) {
	    $relativeAddress = $tmpAddrmapInfo->{'relativeAddress'};
	}
	if (($tmpAddrmapHashRef->{'name'} ne $addrmapHashRef->{'name'}) &&
	    ($tmpAddrmapInfo->{$parentIndexType} != -1)) {
	    $parentArrayIndex = $tmpAddrmapInfo->{$parentIndexType};
	}
    }
    return ($relativeAddress, $parentArrayIndex);
}

sub moreRegistersAtThisLevel {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my ($registerHashListRef, $regHashRef, $external);
    my ($declaredExternal, $hasArrayDecoder);

    if(defined($registerHashListRef = $designDBRef->_getRegHashList($addrmapHashRef)) &&
       ($currAddrmapInfo->{'currAddrmapRegisterIndex'} < @{$registerHashListRef})) {
	if($self->getSkipContentsFlag()) {
	    $currAddrmapInfo->{'currAddrmapRegisterIndex'} = @{$registerHashListRef};
	    $self->clearSkipContentsFlag();
	    return 0;
	}
	return 1;
    }
    return 0;
}

sub getNextRegister {
    my ($self,
	$currAddrmapInfo,
	$addrmapHashRef) = @_;


    my $prefixListRef = ();
    my ($relAddrBigInt, @hierarchicalPrefixList);
    my ($currParentArrayIndex, $parentRelAddr, $groupRelAddr, $parentRelAddrBigInt);
    my ($registerHashListRef, $bigIntOffset);
    my ($regIndex, $regHashRef, $verilogFormat, $offset, $newAddrmapInfo);
    my ($bigIntAddr, @prefixList, @aliasPrefixList, $registerObj, $nextObj);

    $registerHashListRef = $designDBRef->_getRegHashList($addrmapHashRef);
    $regIndex = $currAddrmapInfo->{'currAddrmapRegisterIndex'};
    $regHashRef = $registerHashListRef->[$regIndex];
    $currAddrmapInfo->{'currRegisterHashRef'} = $regHashRef;
    my $parentIsArray = 0;

    if (defined($regHashRef->{'arrayDimensionInfo'}) && $self->getExpandArraysFlag()) {
	$registerObj = $self->getNextRegArrayElement($currAddrmapInfo, $addrmapHashRef);
    } else {
	($verilogFormat, $offset) = $self->getVerilogBaseAndValue($regHashRef->{'byteOffset'});
	$bigIntOffset = Math::BigInt->new($offset);
	$bigIntAddr = Math::BigInt->new($offset);
        if ($addrmapHashRef->{'nodeType'} eq 'group') {
            ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($addrmapHashRef->{'byteOffset'});
            $bigIntOffset->badd(Math::BigInt->new($offset));
        }
	$bigIntAddr->badd($currAddrmapInfo->{'parentAddr'});
	my $addrmapRelByteAddr;
	my $registerIndex; # undefined
	($verilogFormat, $addrmapRelByteAddr) =
	    $self->getVerilogBaseAndValue($regHashRef->{'addressmapRelativeByteAddress'});
	if ($self->parentIsGroupArray($addrmapHashRef)) {
	    my ($grpRelAddr, $grpArrayIndex) = $self->getParentArrayRelAddr_ArrayIndex($addrmapHashRef,
										       'group');
	    my $groupMultiplier = $designDBRef->_getMultiplierFromArrayIndex($grpArrayIndex,
									     $addrmapHashRef->{'arrayDimensionInfo'});
	    my $groupElementSize = Math::BigInt->new($addrmapHashRef->{'arrayElementByteSize'});
	    $groupElementSize->bmul(Math::BigInt->new($groupMultiplier));
	    $addrmapRelByteAddr->badd($groupElementSize);
	}
	my $hierPrefixListRef = $currAddrmapInfo->{'hierarchicalPrefixListRef'};
	push (@hierarchicalPrefixList, (@{$hierPrefixListRef}, $regHashRef->{'name'}));
	$currAddrmapInfo->{'arrayIndex'} = $registerIndex;
	$prefixListRef = $currAddrmapInfo->{'prefixListRef'};
	push(@prefixList, (@$prefixListRef, $regHashRef->{'name'}));
	if (defined($regHashRef->{'aliasOf'})) {
	    my $tmpPrefix = join('.', @$prefixListRef);
	    if ($tmpPrefix =~ /\[/) {
		my $expandedMemberList = $self->expandAliasArrayMembers($regHashRef,
									\@hierarchicalPrefixList);
		push (@aliasPrefixList, @$expandedMemberList);
	    } else {
		push(@aliasPrefixList, split('\.', $regHashRef->{'aliasOf'}));
	    }
	}
	my $arrayIndex = undef;
	$registerObj = csrPerlAPIRegister->new($regHashRef,
					       $arrayIndex,
					       $addrmapHashRef,
					       $bigIntAddr,
					       $bigIntOffset,
					       $addrmapRelByteAddr,
					       $addrmapHashRef->{'busProtocol'},
					       $addrmapHashRef->{'inputPipelineStage'},
					       $addrmapHashRef->{'outputPipelineStage'},
					       \@prefixList,
					       \@aliasPrefixList,
					       \@hierarchicalPrefixList);
	if (($regHashRef->{'nodeType'} eq 'wideRegister') && (!$includeWideRegistersFlag)) {
	    $newAddrmapInfo->{'prefixListRef'} = \@prefixList;
	    $newAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	    $newAddrmapInfo->{'parentAddr'} = $bigIntAddr;
	    $newAddrmapInfo->{'currRegisterHashRef'} = $regHashRef;
	    $newAddrmapInfo->{'currNode'} = $regHashRef;
	    $newAddrmapInfo->{'currAddrmapHashRef'} = $addrmapHashRef;
            # $newAddrmapInfo->{'currAddrmapHashRef'} = $regHashRef;
	    $self->setInitialWideRegisterIndexes($newAddrmapInfo, $addrmapHashRef, $regHashRef);
	    $newAddrmapInfo->{'currAddrmapObj'} = $currAddrmapInfo->{'currAddrmapObj'};
	    push(@visitedAddrmapsList, $newAddrmapInfo);
	}
    }
    return $registerObj;
}

sub expandAliasArrayMembers {
    my ($self,
	$regHashRef,
	$prefixListRef) = @_;

    my $aliasOfValue = $regHashRef->{'aliasOf'};
    my @aliasOfMembers = split('\.', $aliasOfValue);
    my $prefixListLen = @{$prefixListRef} - 1;
    for (my $index = (@aliasOfMembers-1); $index >= 0; $index--) {
	my $nextPrefixMember = $prefixListRef->[$prefixListLen];
	if ($nextPrefixMember =~ /((\[\d+\])+)/) {
            $aliasOfMembers[$index] .= $1;
	}
	$prefixListLen--;
    }
    return \@aliasOfMembers;
}

sub getNextRegArrayElement {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my $prefixListRef = ();
    my ($relativeAddrBigInt, $elementSize, @hierarchicalPrefixList);
    my ($registerObj, $newAddrmapInfo);
    my (@prefixList, @aliasPrefixList, $relativeAddr);

    my $regHashRef = $currAddrmapInfo->{'currRegisterHashRef'};
    my $arrayIndex = $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'};
    my ($verilogFormat, $byteOffset) = $self->getVerilogBaseAndValue($regHashRef->{'byteOffset'});
    my $bigIntAddr = Math::BigInt->new($regHashRef->{'arrayElementByteSize'});
    if($arrayIndex == -1) {
	# we're just starting to process the first array element.
	$arrayIndex = $designDBRef->_getArrayMinIndices($regHashRef->{'arrayDimensionInfo'});
	$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $arrayIndex;
    }
    my $multiplier = $designDBRef->_getMultiplierFromArrayIndex($arrayIndex,
								$regHashRef->{'arrayDimensionInfo'});
    # offset is wrt the current parent.
    my $tmpSize = Math::BigInt->new($regHashRef->{'arrayElementByteSize'});
    $tmpSize->bmul($multiplier);
    my ($addrmapRelByteAddr);
    ($verilogFormat, $addrmapRelByteAddr) =
	$self->getVerilogBaseAndValue($regHashRef->{'addressmapRelativeByteAddress'});
    my $addrmapRelByteAddrBigInt = Math::BigInt->new($addrmapRelByteAddr);
    $addrmapRelByteAddrBigInt->badd($tmpSize);

    if ($self->parentIsGroupArray($addrmapHashRef)) {
	my ($grpRelAddr, $grpArrayIndex) = $self->getParentArrayRelAddr_ArrayIndex($addrmapHashRef,
										   'group');
	my $groupMultiplier = $designDBRef->_getMultiplierFromArrayIndex($grpArrayIndex,
									 $addrmapHashRef->{'arrayDimensionInfo'});
	my $groupElementSize = Math::BigInt->new($addrmapHashRef->{'arrayElementByteSize'});
	$groupElementSize->bmul(Math::BigInt->new($groupMultiplier));
	$addrmapRelByteAddrBigInt->badd($groupElementSize);
    }
    $currAddrmapInfo->{'regArrayIndex'} = $multiplier;
    my $offsetBigInt = Math::BigInt->new($regHashRef->{'arrayElementByteSize'});
    $offsetBigInt->bmul($multiplier);
    $offsetBigInt->badd($byteOffset);

    if ($addrmapHashRef->{'nodeType'} eq 'memory') {
	my $topAddrmapInfo = $visitedAddrmapsList[0];
	my $topAddrmap = $topAddrmapInfo->{'currAddrmapHashRef'};
	my $regElementSize = Math::BigInt->new($regHashRef->{'arrayElementByteSize'});
	if ($topAddrmap->{'alignByteSize'} > $regElementSize) {
	    my $tmpMul = $topAddrmap->{'alignByteSize'} / $regElementSize;
	    $bigIntAddr->bmul($multiplier)->bmul($tmpMul);
	} else {
	    $bigIntAddr->bmul($multiplier);
	}
    } else {
	$bigIntAddr->bmul($multiplier);
    }
    $bigIntAddr->badd($byteOffset);
    $bigIntAddr->badd($currAddrmapInfo->{'parentAddr'});

    $prefixListRef = $currAddrmapInfo->{'prefixListRef'};
    my $tmpRegName = $designDBRef->_getArrayElementName($regHashRef->{'name'}, $arrayIndex);
    push(@prefixList, (@$prefixListRef, $tmpRegName));

    my $hierPrefixListRef = $currAddrmapInfo->{'hierarchicalPrefixListRef'};
    push(@hierarchicalPrefixList, (@{$hierPrefixListRef}, $tmpRegName));

    if(defined($regHashRef->{'aliasOf'})) {
	my $tmpAliasOfName = $designDBRef->_getArrayElementName($regHashRef->{'aliasOf'}, $arrayIndex);
	push(@aliasPrefixList, $tmpAliasOfName);
    }
    $registerObj = csrPerlAPIRegister->new($regHashRef,
					   $arrayIndex,
					   $addrmapHashRef,
					   $bigIntAddr,
					   $offsetBigInt,
                                           $addrmapRelByteAddrBigInt,
					   $addrmapHashRef->{'busProtocol'},
					   $addrmapHashRef->{'inputPipelineStage'},
					   $addrmapHashRef->{'outputPipelineStage'},
					   \@prefixList,
					   \@aliasPrefixList,
					   \@hierarchicalPrefixList);
    if (($regHashRef->{'nodeType'} eq 'wideRegister') && (!$includeWideRegistersFlag)) {
	$newAddrmapInfo->{'prefixListRef'} = \@prefixList;
	$newAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	$newAddrmapInfo->{'parentAddr'} = $currAddrmapInfo->{'parentAddr'};
#	if ($registerObj->isDeclaredExternal() && $registerObj->isArray() &&
#	    ($multiplier > 0)) {
#	    # Power of 2 alignment required
#	    my $tmpBigIntAddr = Math::BigInt->new($byteOffset);
#	    $tmpBigIntAddr->badd($currAddrmapInfo->{'parentAddr'});
#	    $bigIntAddr = $self->powerOf2Align($tmpBigIntAddr, $multiplier);
#	}
	$newAddrmapInfo->{'parentWideRegAddr'} = $bigIntAddr;
	$newAddrmapInfo->{'currAddrmapSubRegisterIndex'} = 0;
	$newAddrmapInfo->{'currRegisterHashRef'} = $regHashRef;
	$newAddrmapInfo->{'currNode'} = $regHashRef;
	$newAddrmapInfo->{'currAddrmapHashRef'} = $addrmapHashRef;
	$newAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
	$newAddrmapInfo->{'currAddrmapObj'} = $currAddrmapInfo->{'currAddrmapObj'};
	push(@visitedAddrmapsList, $newAddrmapInfo);
    }
    return $registerObj;
}

sub updateRegisterAccounting {
    my ($self, $currAddrmapInfo) = @_;

    my ($arrayMax, $arrayMin, $arrayCount, $midWayIndex, $regHashRef);
    my ($external, $declaredExternal, $hasArrayDecoder);

    $regHashRef = $currAddrmapInfo->{'currRegisterHashRef'};
    if (defined($regHashRef->{'arrayDimensionInfo'})) {
	$arrayMin = $designDBRef->_getArrayMinIndices($regHashRef->{'arrayDimensionInfo'});
	$arrayMax = $designDBRef->_getArrayMaxIndices($regHashRef->{'arrayDimensionInfo'});
	$arrayCount = $designDBRef->_getArraySize($regHashRef->{'arrayDimensionInfo'});
	if (($regHashRef->{'external'} eq 'true') ||
	    $designDBRef->_isExternalHashInHierarchy($regHashRef)) {
	    $declaredExternal = $regHashRef->{'declaredExternal'};
	    if ($declaredExternal eq 'false') {
		# This register has a parent in the hierarchy which is declared external.
		if ($arrayCount > 1) {
		    if ($self->getExpandArraysFlag()) {
			if ($self->getLimitExternalRegisterArrayFlag()) {
			    if ($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} == -1) {
				$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $arrayMin;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
								      $arrayMin) == 0) {
				$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $arrayMax;
			    } else {
				$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
			    }
			} else {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
							       $regHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
								 $arrayMax) == 1) { # first arg > 2nd arg
				$currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
				$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
			    }
			}
		    } else {
			$currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
			$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
		    }
		} else {
		    $currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
		    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
		}
	    } else { # is declared external
		$hasArrayDecoder = 'false';
		if (defined($hasArrayDecoder = $regHashRef->{'hasArrayDecoder'}) &&
		    ($hasArrayDecoder eq 'true')) {
		    if (!$self->getExpandArraysFlag()) {
			$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
			$currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
		    } elsif ($self->getLimitExternalRegisterArrayFlag()) {
			if ($arrayCount > 1) {
			    if ($self->getSemiforeEnvGenFlag() && !$self->getSemiforeTBGenFlag()) {
				my $midwayIndex = $designDBRef->_getArrayIndicesMidWay($regHashRef->{'arrayDimensionInfo'});
				if ($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} == -1) {
				    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $arrayMin;
				} elsif (($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
									  $arrayMin) == 0) &&
					 ($designDBRef->_compareArrayIndex($arrayMin, $midwayIndex) != 0)) {
				    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $midwayIndex;
				} elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
									  $midwayIndex) == 0) {
				    # for the case where arrayCount = 2 & arrayMax = 1
				    if ($designDBRef->_compareArrayIndex($midwayIndex, $arrayMax) == 0) {
					$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
					$currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
				    } else {
					$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $arrayMax;
				    }
				} else {
				    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
				    $currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
				}
			    } else {
				$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
			    }
			} else {
			    $currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
			    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
			}
		    } elsif (!$self->getSemiforeEnvGenFlag()) {
			$designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
							   $regHashRef->{'arrayDimensionInfo'});
			if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
							     $arrayMax) == 1) { # first arg > 2nd arg
			    $currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
			    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
			}
		    } else {
			$currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
			$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
		    }
		} else { # does not have array decoder
		    if ($self->getExpandArraysFlag() && $self->getLimitExternalRegisterArrayFlag()) {
			$designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
							   $regHashRef->{'arrayDimensionInfo'});
			if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
							     $arrayMax) == 1) { # first arg > 2nd arg
			    $currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
			    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
			}
		    } elsif ($self->getExpandArraysFlag()) {
			$designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
							   $regHashRef->{'arrayDimensionInfo'});
			if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
							     $arrayMax) == 1) { # first arg > 2nd arg
			    $currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
			    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
			}
		    } else {
			$currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
			$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
		    }
		}
	    }
	} else { # not external
	    if ($self->getExpandArraysFlag()) {
		$designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
						   $regHashRef->{'arrayDimensionInfo'});
		if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
						     $arrayMax) == 1) { # first arg > 2nd arg
		    $currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
		    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
		}
	    } else {
		$currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
		$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = -1;
	    }
	}
    } else {
	$currAddrmapInfo->{'currAddrmapRegisterIndex'}++;
    }
}

sub moreMemoriesAtThisLevel {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my ($memHashListRef, $memHashRef);
    if(defined($memHashListRef = $designDBRef->_getMemoryHashList($addrmapHashRef)) &&
       ($currAddrmapInfo->{'currAddrmapMemoryIndex'} < @{$memHashListRef})) {
        return 1;
    }
    return 0;
}

sub getNextMemory {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my $prefixListRef = ();
    my ($newAddrmapHashRef, $relAddr, @hierarchicalPrefixList);
    my ($memoryHashListRef, $bigIntOffset, $addrmapRelByteAddr);
    my ($memIndex, $memHashRef, $verilogFormat, $offset, $newAddrmapInfo);
    my ($bigIntAddr, @prefixList, $memoryObj, $nextObj);

    $memoryHashListRef = $designDBRef->_getMemoryHashList($addrmapHashRef);
    $memIndex = $currAddrmapInfo->{'currAddrmapMemoryIndex'};
    $memHashRef = $memoryHashListRef->[$memIndex];
    $currAddrmapInfo->{'currMemoryHashRef'} = $memHashRef;
    if (defined($memHashRef->{'arrayDimensionInfo'}) && $self->getExpandArraysFlag()) {
	$memoryObj = $self->getNextMemArrayElement($currAddrmapInfo, $addrmapHashRef);
    } else {
	($verilogFormat, $offset) = $self->getVerilogBaseAndValue($memHashRef->{'byteOffset'});
	$bigIntOffset = Math::BigInt->new($offset);
	$bigIntAddr = Math::BigInt->new($offset);
	$bigIntAddr->badd($currAddrmapInfo->{'parentAddr'});
	my $relAddrBigInt = Math::BigInt->new(0);

	($verilogFormat, $addrmapRelByteAddr) =
	    $self->getVerilogBaseAndValue($memHashRef->{'addressmapRelativeByteAddress'});

	$prefixListRef = $currAddrmapInfo->{'prefixListRef'};
	push(@prefixList, (@{$prefixListRef}, $memHashRef->{'name'}));

	my $hierPrefixListRef = $currAddrmapInfo->{'hierarchicalPrefixListRef'};
	push(@hierarchicalPrefixList, (@{$hierPrefixListRef}, $memHashRef->{'name'}));

	$newAddrmapHashRef = $memHashRef;
	$newAddrmapInfo->{'prefixListRef'} = \@prefixList;
	$newAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	$newAddrmapInfo->{'currNode'} = $memHashRef;
	$newAddrmapInfo->{'currAddrmapHashRef'} = $newAddrmapHashRef;
	$newAddrmapInfo->{'parentAddr'} = $bigIntAddr;
	$self->setInitialAddrmapIndexes($newAddrmapInfo, $newAddrmapHashRef);
	my $arrayIndex = undef;
        $memoryObj = csrPerlAPIMemory->new($memHashRef,
					   $arrayIndex,
                                           $addrmapHashRef,
                                           $bigIntAddr,
                                           $bigIntOffset,
					   $addrmapRelByteAddr,
                                           $addrmapHashRef->{'busProtocol'},
                                           $addrmapHashRef->{'inputPipelineStage'},
                                           $addrmapHashRef->{'outputPipelineStage'},
                                           \@prefixList,
					   \@hierarchicalPrefixList);
       # if (($memHashRef->{'nodeType'} eq 'wideMemory') && (!$includeWideMemoriesFlag)) {
       if ($memHashRef->{'nodeType'} eq 'wideMemory') {
            $newAddrmapInfo->{'prefixListRef'} = \@prefixList;
            $newAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
            $newAddrmapInfo->{'parentAddr'} = $bigIntAddr;
            $newAddrmapInfo->{'currMemoryHashRef'} = $memHashRef;
            $newAddrmapInfo->{'currNode'} = $memHashRef;
            $newAddrmapInfo->{'currAddrmapHashRef'} = $memHashRef;
            $self->setInitialWideMemoryIndexes($newAddrmapInfo,
                                               $addrmapHashRef,
                                               $memHashRef);
            $newAddrmapInfo->{'currAddrmapObj'} = $currAddrmapInfo->{'currAddrmapObj'};
            push(@visitedAddrmapsList, $newAddrmapInfo);
        #} elsif ($memHashRef->{'nodeType'} eq 'wideMemory') {
        #    $newAddrmapInfo->{'currMemoryHashRef'} = $memHashRef;
        #    $newAddrmapInfo->{'currNode'} = $memHashRef;
        } else {
            $newAddrmapInfo->{'currAddrmapObj'} = $memoryObj;
            push(@visitedAddrmapsList, $newAddrmapInfo);
	}
    }
    return $memoryObj;
}

sub getNextMemArrayElement {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my $prefixListRef = ();
    my ($addrmapRelByteAddr);
    my ($newAddrmapHashRef, $tmpAddr, @hierarchicalPrefixList);
    my ($arrayIndex, $memHashRef, $memoryObj, $newAddrmapInfo);
    my (@prefixList, @aliasPrefixList, $bigIntAddr, $relativeAddr);

    $memHashRef = $currAddrmapInfo->{'currMemoryHashRef'};
    $arrayIndex = $currAddrmapInfo->{'currAddrmapMemoryArrayIndex'};
    my ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($memHashRef->{'byteOffset'});
    $bigIntAddr = Math::BigInt->new($memHashRef->{'arrayElementByteSize'});
    # offset is wrt to the current parent.
    my $offsetBigInt = Math::BigInt->new($memHashRef->{'arrayElementByteSize'});
    if($arrayIndex == -1) {
	# we're just starting to process the first array element.
	$arrayIndex = $designDBRef->_getArrayMinIndices($memHashRef->{'arrayDimensionInfo'});
	$currAddrmapInfo->{'currAddrmapMemoryArrayIndex'} = $arrayIndex;
    }
    my $multiplier = $designDBRef->_getMultiplierFromArrayIndex($arrayIndex,
								$memHashRef->{'arrayDimensionInfo'});
    $offsetBigInt->bmul($multiplier);
    $offsetBigInt->badd($offset);
    $bigIntAddr->bmul($multiplier);
    $bigIntAddr->badd($offset);
    $bigIntAddr->badd($currAddrmapInfo->{'parentAddr'});

    ($verilogFormat, $addrmapRelByteAddr) =
	    $self->getVerilogBaseAndValue($memHashRef->{'addressmapRelativeByteAddress'});
    my $tmpSize = Math::BigInt->new($memHashRef->{'arrayElementByteSize'});
    $tmpSize->bmul($multiplier);
    $addrmapRelByteAddr->badd($tmpSize);

    $prefixListRef = $currAddrmapInfo->{'prefixListRef'};
    my $tmpMemName = $designDBRef->_getArrayElementName($memHashRef->{'name'}, $arrayIndex);
    push(@prefixList, (@$prefixListRef, $tmpMemName));
    if(defined($memHashRef->{'aliasOf'})) {
        my $tmpAliasName = $designDBRef->_getArrayElementName($memHashRef->{'aliasOf'}, $arrayIndex);
	push(@aliasPrefixList, (@$prefixListRef, $tmpAliasName));
    }

    my $hierPrefixListRef = $currAddrmapInfo->{'hierarchicalPrefixListRef'};
    push (@hierarchicalPrefixList, (@{$hierPrefixListRef}, $tmpMemName));

    $newAddrmapHashRef = $memHashRef;
    $newAddrmapInfo->{'prefixListRef'} = \@prefixList;
    $newAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
    $newAddrmapInfo->{'currNode'} = $memHashRef;
    $newAddrmapInfo->{'currAddrmapHashRef'} = $newAddrmapHashRef;
    $newAddrmapInfo->{'parentAddr'} = $bigIntAddr;
    $self->setInitialAddrmapIndexes($newAddrmapInfo, $newAddrmapHashRef);
    $memoryObj = csrPerlAPIMemory->new($memHashRef,
				       $arrayIndex,
                                       $addrmapHashRef,
                                       $bigIntAddr,
                                       $offsetBigInt,
				       $addrmapRelByteAddr,
                                       $addrmapHashRef->{'busProtocol'},
                                       $addrmapHashRef->{'inputPipelineStage'},
                                       $addrmapHashRef->{'outputPipelineStage'},
                                       \@prefixList,
				       \@hierarchicalPrefixList);
    if (($memHashRef->{'nodeType'} eq 'wideMemory') && (!$includeWideMemoriesFlag)) {
	$newAddrmapInfo->{'prefixListRef'} = \@prefixList;
	$newAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	$newAddrmapInfo->{'parentAddr'} = $currAddrmapInfo->{'parentAddr'};
	$newAddrmapInfo->{'parentWideMemAddr'} = $bigIntAddr;
	$newAddrmapInfo->{'currAddrmapSubMemoryIndex'} = 0;
	$newAddrmapInfo->{'currMemoryHashRef'} = $memHashRef;
	$newAddrmapInfo->{'currNode'} = $memHashRef;
	$newAddrmapInfo->{'currAddrmapHashRef'} = $addrmapHashRef;
	$newAddrmapInfo->{'currAddrmapMemoryArrayIndex'} = -1;
	$newAddrmapInfo->{'currAddrmapObj'} = $currAddrmapInfo->{'currAddrmapObj'};
	$self->setInitialWideMemoryIndexes($newAddrmapInfo,
					   $addrmapHashRef,
					   $memHashRef);
	$newAddrmapInfo->{'currAddrmapObj'} = $currAddrmapInfo->{'currAddrmapObj'};
    } else {
	$newAddrmapInfo->{'currAddrmapObj'} = $memoryObj;
    }
    push(@visitedAddrmapsList, $newAddrmapInfo);
    return $memoryObj;
}

sub getNextSubMemory {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my $prefixListRef = ();
    my ($tmpSubMemName, $arrayIndex, $wideRelAddrBigInt);
    my ($relAddr, $relAddrBigInt, @hierarchicalPrefixList);
    my ($subMemIndex, $subMemHashRef, $verilogFormat, $offset);
    my ($bigIntAddr, @prefixList, $memoryObj, @aliasPrefixList);

    my $memMultiplier = 0;
    my $memHashRef = $currAddrmapInfo->{'currMemoryHashRef'};
    my $subMemHashListRef = $memHashRef->{'subMemories'};
    $subMemIndex = $currAddrmapInfo->{'currAddrmapSubMemoryIndex'};
    $subMemHashRef = $subMemHashListRef->[$subMemIndex];
    ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($subMemHashRef->{'byteOffset'});
    ($verilogFormat, $relAddr) = $self->getVerilogBaseAndValue($subMemHashRef->{'addressmapRelativeByteAddress'});
    $relAddrBigInt = Math::BigInt->new($relAddr);
    $prefixListRef = $currAddrmapInfo->{'prefixListRef'};
    my $hierPrefixListRef = $currAddrmapInfo->{'hierarchicalPrefixListRef'};
    my $addrmapRelByteAddr;
    if(defined($memHashRef->{'arrayDimensionInfo'})) {
	$arrayIndex = $currAddrmapInfo->{'currAddrmapMemoryArrayIndex'};
	if($arrayIndex == -1) {
	    $arrayIndex = $designDBRef->_getArrayMinIndices($memHashRef->{'arrayDimensionInfo'});
	    $currAddrmapInfo->{'currAddrmapMemoryArrayIndex'} = $arrayIndex;
	}
	$memMultiplier = $designDBRef->_getMultiplierFromArrayIndex($arrayIndex,
								    $memHashRef->{'arrayDimensionInfo'});
	$subMemIndex = $currAddrmapInfo->{'currAddrmapSubMemoryIndex'};
        my $subMemListLen = @{$subMemHashListRef};
	$bigIntAddr = Math::BigInt->new($offset);
	$bigIntAddr->badd($currAddrmapInfo->{'parentWideMemAddr'});

	my $elemSizeBigInt = Math::BigInt->new($subMemHashRef->{'arrayElementByteSize'});
	$elemSizeBigInt->bmul($memMultiplier);
	$elemSizeBigInt->bmul($subMemListLen);
	$relAddrBigInt->badd($elemSizeBigInt);

	my $tmpMemName = $designDBRef->_getArrayElementName($subMemHashRef->{'name'}, $arrayIndex);
	$tmpSubMemName = join('_', $subMemHashRef->{'name'}, $subMemIndex);

        if ($viewName eq 'sv') {
            my $numDim = scalar(@{$memHashRef->{'arrayDimensionInfo'}});
            for (my $count = 0; $count < $numDim; $count++) {
                $prefixListRef->[@$prefixListRef-1] =~ s/(\[\d+\])$//;
                $hierPrefixListRef->[@$hierPrefixListRef-1] =~ s/(\[\d+\])$//;
            }
        }

	push(@prefixList, (@$prefixListRef, $tmpMemName));

	if(defined($memHashRef->{'aliasOf'})) {
	    my $tmpAliasOfName = $designDBRef->_getArrayElementName($memHashRef->{'aliasOf'},
								    $subMemIndex);
	    push(@aliasPrefixList, (@$prefixListRef, $tmpAliasOfName));
	}
	push (@hierarchicalPrefixList, (@$hierPrefixListRef, $tmpMemName));

	my $arraySize = $designDBRef->_getArraySize($memHashRef->{'arrayDimensionInfo'});
	my $parentAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
	$parentAddrmapInfo->{'wideMemArraySize'} = $arraySize;
	$parentAddrmapInfo->{'wideMemArrayIndex'} = $memMultiplier;
	$addrmapRelByteAddr = $relAddrBigInt;
    } else {
	$arrayIndex = undef;
	$bigIntAddr = Math::BigInt->new($offset);
	$bigIntAddr->badd($currAddrmapInfo->{'parentAddr'});
	$prefixListRef = $currAddrmapInfo->{'prefixListRef'};
	$tmpSubMemName = join('_', $subMemHashRef->{'name'});
	push(@prefixList, (@$prefixListRef, $tmpSubMemName));
	if (defined($memHashRef->{'aliasOf'})) {
	    push(@aliasPrefixList, (@$prefixListRef, $memHashRef->{'aliasOf'}));
	}
	push (@hierarchicalPrefixList, (@$hierPrefixListRef, $tmpSubMemName));
	$addrmapRelByteAddr = $relAddr;
    }
    $memoryObj = csrPerlAPIMemory->new($subMemHashRef,
				       $arrayIndex,
				       $memHashRef,
				       $bigIntAddr,
				       Math::BigInt->new($offset),
				       $addrmapRelByteAddr,
				       $addrmapHashRef->{'busProtocol'},
				       $addrmapHashRef->{'inputPipelineStage'},
				       $addrmapHashRef->{'outputPipelineStage'},
				       \@prefixList,
				       \@hierarchicalPrefixList);
    $memoryObj->set_subMemoryIndex($currAddrmapInfo->{'currAddrmapSubMemoryIndex'});
    $memoryObj->set_subMemoryFlag('true');
    return $memoryObj;
}

sub updateMemoryAccounting {
    my ($self, $currAddrmapInfo) = @_;

    my ($arrayMax, $memHashRef);

    $memHashRef = $currAddrmapInfo->{'currMemoryHashRef'};
    if (defined($memHashRef->{'arrayDimensionInfo'})) {
	$arrayMax = $designDBRef->_getArrayMaxIndices($memHashRef->{'arrayDimensionInfo'});
	if ($self->getExpandArraysFlag()) {
	    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapMemoryArrayIndex'},
					       $memHashRef->{'arrayDimensionInfo'});
	    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapMemoryArrayIndex'},
						 $arrayMax) == 1) { # first arg > 2nd arg
		$currAddrmapInfo->{'currAddrmapMemoryIndex'}++;
		$currAddrmapInfo->{'currAddrmapMemoryArrayIndex'} = -1;
	    }
	} else {
	    $currAddrmapInfo->{'currAddrmapMemoryIndex'}++;
	}
    } else {
	$currAddrmapInfo->{'currAddrmapMemoryIndex'}++;
    }
}

sub updateWideMemoryAccounting {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my ($declaredExternal);

    my $memHashRef = $currAddrmapInfo->{'currMemoryHashRef'};
    my $arrayIndex = $currAddrmapInfo->{'currAddrmapMemoryArrayIndex'};
    my $subMemHashListRef = $memHashRef->{'subMemories'};
    $currAddrmapInfo->{'currAddrmapSubMemoryIndex'}++;
    if($currAddrmapInfo->{'currAddrmapSubMemoryIndex'} >= @{$subMemHashListRef}) {
	# exhausted the sub memory list.
	if(defined($memHashRef->{'arrayDimensionInfo'})) {
	    my $arrayMin = $designDBRef->_getArrayMinIndices($memHashRef->{'arrayDimensionInfo'});
	    my $arrayMax = $designDBRef->_getArrayMaxIndices($memHashRef->{'arrayDimensionInfo'});
	    my $arrayCount = $designDBRef->_getArraySize($memHashRef->{'arrayDimensionInfo'});
	    if ($self->getExpandArraysFlag()) {
		$designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapMemoryArrayIndex'},
						   $memHashRef->{'arrayDimensionInfo'});
		if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapMemoryArrayIndex'},
						     $arrayMax) == 1) { # first arg > 2nd arg
		    $self->doneWithWideMemory();
		    $currAddrmapInfo->{'currAddrmapMemoryArrayIndex'} = -1;
		}
	    } else {
		$self->doneWithWideMemory();
	    }
	} else {
	    # we're done with the wide memory.
	    $self->doneWithWideMemory();
	}
    }
}

sub moreGroupsAtThisLevel {
    my ($self,
        $currAddrmapInfo,
        $addrmapHashRef) = @_;

    my ($groupIndex, $groupHashListRef, $external);

    if(defined($groupHashListRef = $designDBRef->_getGroupHashList($addrmapHashRef))) {
	$groupIndex = $currAddrmapInfo->{'currAddrmapGroupIndex'};
	if($self->getSkipExternalAddressmapsFlag()) {
	    while(($groupIndex < @{$groupHashListRef}) &&
		  defined($external = $groupHashListRef->[$groupIndex]->{'external'}) &&
                  ($external eq 'true')) {
		$groupIndex++;
	    }
	}
	$currAddrmapInfo->{'currAddrmapGroupIndex'} = $groupIndex;
	if ($currAddrmapInfo->{'currAddrmapGroupIndex'} < @{$groupHashListRef}) {
	    return 1;
	}
    }
    return 0;
}

sub getNextGroup {
    my ($self,
        $currAddrmapInfo,
        $addrmapHashRef) = @_;

    my ($external, $declaredExternal, $relAddr);
    my ($groupIndex, $bigIntAddr, @prefixList, @hierarchicalPrefixList);
    my ($groupObj, $tmpAddrmapInfo, $groupHashListRef);
    my ($verilogFormat, $offset, $newAddrmapInfo, $newGroupHashRef);

    $groupHashListRef = $designDBRef->_getGroupHashList($addrmapHashRef);
    $groupIndex = $currAddrmapInfo->{'currAddrmapGroupIndex'};
    my $currGroupHashRef = $groupHashListRef->[$groupIndex];
    $currAddrmapInfo->{'currGroupHashRef'} = $currGroupHashRef;
    my $groupHashRef = $designDBRef->_findAddressmapForInstanceHash($currGroupHashRef);
    # only 'group' type objects (and registers) can have 'hasArrayDecoder' property.
    if (defined($groupHashRef->{'arrayDimensionInfo'}) && $self->getExpandArraysFlag()) {
	$groupObj = $self->getNextGroupArrayElement($currAddrmapInfo,
						    $addrmapHashRef,
						    $groupHashRef);
    } else {
	($verilogFormat, $offset) = $self->getVerilogBaseAndValue($groupHashRef->{'byteOffset'});
	$bigIntAddr = Math::BigInt->new($offset);
	$bigIntAddr->badd($currAddrmapInfo->{'parentAddr'});

	($verilogFormat, $relAddr) = $self->getVerilogBaseAndValue($groupHashRef->{'addressmapRelativeByteAddress'});
	push(@prefixList, (@{$currAddrmapInfo->{'prefixListRef'}}, $groupHashRef->{'name'}));
	push(@hierarchicalPrefixList, (@{$currAddrmapInfo->{'hierarchicalPrefixListRef'}},
				       $groupHashRef->{'name'}));

	$newGroupHashRef = $groupHashRef;
	$newAddrmapInfo->{'prefixListRef'} = \@prefixList;
	$newAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	$newAddrmapInfo->{'currNode'} = $groupHashRef;
	$newAddrmapInfo->{'currAddrmapHashRef'} = $newGroupHashRef;
	$newAddrmapInfo->{'parentAddr'} = $bigIntAddr;
	$newAddrmapInfo->{'arrayIndex'} = undef;

	push(@visitedAddrmapsList, $newAddrmapInfo);
	$self->setInitialAddrmapIndexes($newAddrmapInfo, $newGroupHashRef);
	my $arrayIndex = undef;
	$groupObj = csrPerlAPIGroup->new($newGroupHashRef,
					 $arrayIndex,
					 $addrmapHashRef,
					 $bigIntAddr,
					 $relAddr,
					 $addrmapHashRef->{'busProtocol'},
					 $addrmapHashRef->{'inputPipelineStage'},
					 $addrmapHashRef->{'outputPipelineStage'},
					 \@prefixList,
					 \@hierarchicalPrefixList);
	$newAddrmapInfo->{'currAddrmapObj'} = $groupObj;
	return $groupObj;
    }
}

sub getNextGroupArrayElement {
    my ($self, $currAddrmapInfo, $addrmapHashRef, $groupHashRef) = @_;

    my (@prefixList, $groupObj, @hierarchicalPrefixList);
    my (@addrmapPrefixList, $relAddr, $tmpAddr);
    my ($newAddrmapInfo, $newGroupHashRef);

    my $arrayIndex = $currAddrmapInfo->{'currAddrmapGroupArrayIndex'};
    my ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($groupHashRef->{'byteOffset'});
    my $bigIntAddr = Math::BigInt->new($groupHashRef->{'arrayElementByteSize'});

    if($arrayIndex == -1) {
	# we're just starting to process the first element of the array.
	$arrayIndex = $designDBRef->_getArrayMinIndices($groupHashRef->{'arrayDimensionInfo'});
	$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = $arrayIndex;
    }
    my $multiplier = $designDBRef->_getMultiplierFromArrayIndex($arrayIndex,
								$groupHashRef->{'arrayDimensionInfo'});
    $bigIntAddr->bmul($multiplier);
    $bigIntAddr->badd($offset);
    $bigIntAddr->badd($currAddrmapInfo->{'parentAddr'});
    @prefixList = @{$currAddrmapInfo->{'prefixListRef'}};
    my $tmpGrpName = $designDBRef->_getArrayElementName($groupHashRef->{'name'}, $arrayIndex);
    push(@prefixList, $tmpGrpName);

    my $hierPrefixListRef = $currAddrmapInfo->{'hierarchicalPrefixListRef'};
    push (@hierarchicalPrefixList, (@{$hierPrefixListRef}, $tmpGrpName));
    $newAddrmapInfo->{'prefixListRef'} = \@prefixList;
    $newAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;

    ($verilogFormat, $tmpAddr) = $self->getVerilogBaseAndValue($groupHashRef->{'addressmapRelativeByteAddress'});
    my $addrmapRelByteAddr = Math::BigInt->new($groupHashRef->{'arrayElementByteSize'});
    $addrmapRelByteAddr->bmul($multiplier);
    $addrmapRelByteAddr->badd($tmpAddr);

    $newGroupHashRef = $groupHashRef;
    $newAddrmapInfo->{'currNode'} = $groupHashRef;
    $newAddrmapInfo->{'currAddrmapHashRef'} = $newGroupHashRef;
    $newAddrmapInfo->{'parentAddr'} = $bigIntAddr;
    $newAddrmapInfo->{'arrayIndex'} = $multiplier;
    push(@visitedAddrmapsList, $newAddrmapInfo);
    $self->setInitialAddrmapIndexes($newAddrmapInfo, $addrmapHashRef);
    my $busProtocol;
    if ($groupHashRef->{'declaredExternal'} eq 'true') {
	$busProtocol = 'basic';
    } else {
	$busProtocol = $addrmapHashRef->{'busProtocol'};
    }
    $groupObj = csrPerlAPIGroup->new($newGroupHashRef,
				     $arrayIndex,
				     $addrmapHashRef,
				     $bigIntAddr,
				     $addrmapRelByteAddr,
				     $busProtocol,
				     $addrmapHashRef->{'inputPipelineStage'},
				     $addrmapHashRef->{'outputPipelineStage'},
				     \@prefixList,
				     \@hierarchicalPrefixList);
    $newAddrmapInfo->{'currAddrmapObj'} = $groupObj;
    return $groupObj;
}

sub updateGroupAccounting {
    my ($self, $currAddrmapInfo) = @_;

    my ($declaredExternal, $arrayCount);
    my ($arrayMax, $arrayMin, $external, $hasArrayDecoder);

    my $groupHashRef = $currAddrmapInfo->{'currGroupHashRef'};
    my $newAddrmapHashRef = $designDBRef->_findAddressmapForInstanceHash($groupHashRef);
    $groupHashRef = $newAddrmapHashRef;

    if(defined($groupHashRef->{'arrayDimensionInfo'})) {
	$arrayMax = $designDBRef->_getArrayMaxIndices($groupHashRef->{'arrayDimensionInfo'});
	$arrayMin = $designDBRef->_getArrayMinIndices($groupHashRef->{'arrayDimensionInfo'});
	$arrayCount = $designDBRef->_getArraySize($groupHashRef->{'arrayDimensionInfo'});
	if ($groupHashRef->{'byteEndian'} eq 'little') {
	    if (($groupHashRef->{'external'} eq 'true') ||
		$designDBRef->_isExternalHashInHierarchy($groupHashRef)) {
		$declaredExternal = $groupHashRef->{'declaredExternal'};
		if ($declaredExternal eq 'false') {
		    if ($arrayCount > 1) {
                        if (!$self->getExpandArraysFlag()) {
			    $currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
                        } elsif ($self->getLimitExternalGroupArrayFlag()) {
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMin) == 0) { # first arg eq 2nd arg
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = $arrayMax;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								      $arrayMax) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    } else {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    }
			} else {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
							       $groupHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMax) == 1) {
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			    }
			}
		    } else {
			$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		    }
		} else { # declared external
		    $hasArrayDecoder = 'false';
		    if (defined($hasArrayDecoder = $groupHashRef->{'hasArrayDecoder'}) &&
			($hasArrayDecoder eq 'true')) {
			$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		    } else { # no array decoder
			if ($self->getExpandArraysFlag() && $self->getLimitExternalGroupArrayFlag() &&
			    $self->getSemiforeEnvGenFlag() && !$self->getSemiforeTBGenFlag()) {
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								$arrayMax) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								      $arrayMin) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = $arrayMax;
			    }
			} elsif ($self->getExpandArraysFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
							       $groupHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMax) == 1) {
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			    }
			} else {
			    $currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			}
		    }
		}
	    } else { # not external
		if ($self->getExpandArraysFlag()) {
		    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
						       $groupHashRef->{'arrayDimensionInfo'});
		    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
							 $arrayMax) == 1) {
			$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		    }
		} else {
		    $currAddrmapInfo->{'currAddrmapGroupIndex'}++;
		    $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		}
	    }
	} else { #big endian
	    if($groupHashRef->{'external'} eq 'true') {
		$declaredExternal = $groupHashRef->{'declaredExternal'};
		if ($declaredExternal eq 'false') {
		    if ($arrayCount > 1) {
			if ($self->getLimitExternalGroupArrayFlag()) {
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMin) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = $arrayMax;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								      $arrayMax) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    } else {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    }
			} elsif ($self->getExpandArraysFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
							       $groupHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMax) == 1) {
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			    }
			} else {
			    $currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			}
		    } else {
			$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		    }
		} else {
		    $hasArrayDecoder = 'false';
		    if (defined($hasArrayDecoder = $groupHashRef->{'hasArrayDecoder'}) &&
			($hasArrayDecoder eq 'true')) {
			$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		    } else { # no array decoder
			if ($self->getExpandArraysFlag() && $self->getLimitExternalGroupArrayFlag() &&
			    $self->getSemiforeEnvGenFlag() && !$self->getSemiforeTBGenFlag()) {
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMax) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								      $arrayMin) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = $arrayMax;
			    }
			} elsif ($self->getExpandArraysFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
							       $groupHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMax) == 1) {
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			    }
			} else {
			    $currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			}
		    }
		}
	    } else {
		if ($self->getExpandArraysFlag()) {
		    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
						       $groupHashRef->{'arrayDimensionInfo'});
		    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
							 $arrayMax) == 1) {
			$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		    }
		} else {
		    $currAddrmapInfo->{'currAddrmapGroupIndex'}++;
		    $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		}
	    }
	}
    } else {
	$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
    }
}

sub updateGroupAccounting_old {
    my ($self, $currAddrmapInfo) = @_;

    my ($declaredExternal, $arrayCount);
    my ($arrayMax, $arrayMin, $external, $hasArrayDecoder);

    my $groupHashRef = $currAddrmapInfo->{'currGroupHashRef'};
    my $newAddrmapHashRef = $designDBRef->_findAddressmapForInstanceHash($groupHashRef);
    $groupHashRef = $newAddrmapHashRef;

    if(defined($groupHashRef->{'arrayDimensionInfo'})) {
	$arrayMax = $designDBRef->_getArrayMaxIndices($groupHashRef->{'arrayDimensionInfo'});
	$arrayMin = $designDBRef->_getArrayMinIndices($groupHashRef->{'arrayDimensionInfo'});
	$arrayCount = $designDBRef->_getArraySize($groupHashRef->{'arrayDimensionInfo'});
	if ($groupHashRef->{'byteEndian'} eq 'little') {
	    if (($groupHashRef->{'external'} eq 'true') ||
		$designDBRef->_isExternalHashInHierarchy($groupHashRef)) {
		$declaredExternal = $groupHashRef->{'declaredExternal'};
		if ($declaredExternal eq 'false') {
		    if ($arrayCount > 1) {
			if ($self->getLimitExternalGroupArrayFlag()) {
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMin) == 0) { # first arg eq 2nd arg
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = $arrayMax;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								      $arrayMax) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    } else {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    }
			} elsif ($self->getExpandArraysFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
							       $groupHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMax) == 1) {
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			    }
			} else {
			    $currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			}
		    } else {
			$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		    }
		} else { # declared external
		    $hasArrayDecoder = 'false';
		    if (defined($hasArrayDecoder = $groupHashRef->{'hasArrayDecoder'}) &&
			($hasArrayDecoder eq 'true')) {
			$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		    } else { # no array decoder
			if ($self->getExpandArraysFlag() && $self->getLimitExternalGroupArrayFlag() &&
			    $self->getSemiforeEnvGenFlag() && !$self->getSemiforeTBGenFlag()) {
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								$arrayMax) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								      $arrayMin) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = $arrayMax;
			    }
			} elsif ($self->getExpandArraysFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
							       $groupHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMax) == 1) {
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			    }
			} else {
			    $currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			}
		    }
		}
	    } else { # not external
		if ($self->getExpandArraysFlag()) {
		    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
						       $groupHashRef->{'arrayDimensionInfo'});
		    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
							 $arrayMax) == 1) {
			$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		    }
		} else {
		    $currAddrmapInfo->{'currAddrmapGroupIndex'}++;
		    $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		}
	    }
	} else { #big endian
	    if($groupHashRef->{'external'} eq 'true') {
		$declaredExternal = $groupHashRef->{'declaredExternal'};
		if ($declaredExternal eq 'false') {
		    if ($arrayCount > 1) {
			if ($self->getLimitExternalGroupArrayFlag()) {
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMin) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = $arrayMax;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								      $arrayMax) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    } else {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    }
			} elsif ($self->getExpandArraysFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
							       $groupHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMax) == 1) {
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			    }
			} else {
			    $currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			}
		    } else {
			$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		    }
		} else {
		    $hasArrayDecoder = 'false';
		    if (defined($hasArrayDecoder = $groupHashRef->{'hasArrayDecoder'}) &&
			($hasArrayDecoder eq 'true')) {
			$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		    } else { # no array decoder
			if ($self->getExpandArraysFlag() && $self->getLimitExternalGroupArrayFlag() &&
			    $self->getSemiforeEnvGenFlag() && !$self->getSemiforeTBGenFlag()) {
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMax) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								      $arrayMin) == 0) {
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = $arrayMax;
			    }
			} elsif ($self->getExpandArraysFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
							       $groupHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
								 $arrayMax) == 1) {
				$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
				$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			    }
			} else {
			    $currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			    $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
			}
		    }
		}
	    } else {
		if ($self->getExpandArraysFlag()) {
		    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
						       $groupHashRef->{'arrayDimensionInfo'});
		    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapGroupArrayIndex'},
							 $arrayMax) == 1) {
			$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
			$currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		    }
		} else {
		    $currAddrmapInfo->{'currAddrmapGroupIndex'}++;
		    $currAddrmapInfo->{'currAddrmapGroupArrayIndex'} = -1;
		}
	    }
	}
    } else {
	$currAddrmapInfo->{'currAddrmapGroupIndex'}++;
    }
}

sub moreUnionsAtThisLevel {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my ($unionIndex, $unionHashListRef, $external);

    if(defined($unionHashListRef = $designDBRef->_getUnionHashList($addrmapHashRef))) {
	$unionIndex = $currAddrmapInfo->{'currAddrmapUnionIndex'};
	if($self->getSkipExternalAddressmapsFlag()) {
	    while(($unionIndex < @{$unionHashListRef}) &&
		  defined($external = $unionHashListRef->[$unionIndex]->{'external'}) &&
                  ($external eq 'true')) {
		$unionIndex++;
	    }
	}
	$currAddrmapInfo->{'currAddrmapUnionIndex'} = $unionIndex;
	if ($currAddrmapInfo->{'currAddrmapUnionIndex'} < @{$unionHashListRef}) {
	    return 1;
	}
    }
    return 0;
}

sub updateUnionAccounting {
    my ($self, $currAddrmapInfo) = @_;

    my ($declaredExternal, $arrayCount);
    my ($arrayMax, $arrayMin, $external, $hasArrayDecoder);

    my $unionHashRef = $currAddrmapInfo->{'currUnionHashRef'};
    my $newAddrmapHashRef = $designDBRef->_findAddressmapForInstanceHash($unionHashRef);
    $unionHashRef = $newAddrmapHashRef;
    if(defined($unionHashRef->{'arrayDimensionInfo'})) {
	$arrayMax = $designDBRef->_getArrayMaxIndices($unionHashRef->{'arrayDimensionInfo'});
	$arrayMin = $designDBRef->_getArrayMinIndices($unionHashRef->{'arrayDimensionInfo'});
	$arrayCount = $designDBRef->_getArraySize($unionHashRef->{'arrayDimensionInfo'});
	if ($unionHashRef->{'byteEndian'} eq 'little') {
	    if (($unionHashRef->{'external'} eq 'true') ||
		$designDBRef->_isExternalHashInHierarchy($unionHashRef)) {
		$declaredExternal = $unionHashRef->{'declaredExternal'};
		if ($declaredExternal eq 'false') {
		    if ($arrayCount > 1) {
			if ($self->getLimitExternalUnionArrayFlag()) {
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
								 $arrayMin) == 0) { # first arg eq 2nd arg
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = $arrayMax;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
								      $arrayMax) == 0) {
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			    } else {
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			    }
			} elsif ($self->getExpandArraysFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
							       $unionHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
								 $arrayMax) == 1) {
				$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
			    }
			} else {
			    $currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			    $currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
			}
		    } else {
			$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
		    }
		} else { # declared external
		    $hasArrayDecoder = 'false';
		    if (defined($hasArrayDecoder = $unionHashRef->{'hasArrayDecoder'}) &&
			($hasArrayDecoder eq 'true')) {
			$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
		    } else { # no array decoder
			if ($self->getExpandArraysFlag() && $self->getLimitExternalUnionArrayFlag() &&
			    $self->getSemiforeEnvGenFlag() && !$self->getSemiforeTBGenFlag()) {
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
								$arrayMax) == 0) {
				$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
								      $arrayMin) == 0) {
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = $arrayMax;
			    }
			} elsif ($self->getExpandArraysFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
							       $unionHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
								 $arrayMax) == 1) {
				$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
			    }
			} else {
			    $currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			    $currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
			}
		    }
		}
	    } else { # not external
		if ($self->getExpandArraysFlag()) {
		    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
						       $unionHashRef->{'arrayDimensionInfo'});
		    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
							 $arrayMax) == 1) {
			$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
		    }
		} else {
		    $currAddrmapInfo->{'currAddrmapUnionIndex'}++;
		    $currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
		}
	    }
	} else { #big endian
	    if($unionHashRef->{'external'} eq 'true') {
		$declaredExternal = $unionHashRef->{'declaredExternal'};
		if ($declaredExternal eq 'false') {
		    if ($arrayCount > 1) {
			if ($self->getLimitExternalUnionArrayFlag()) {
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
								 $arrayMin) == 0) {
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = $arrayMax;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
								      $arrayMax) == 0) {
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			    } else {
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
				$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			    }
			} elsif ($self->getExpandArraysFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
							       $unionHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
								 $arrayMax) == 1) {
				$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
			    }
			} else {
			    $currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			    $currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
			}
		    } else {
			$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
		    }
		} else {
		    $hasArrayDecoder = 'false';
		    if (defined($hasArrayDecoder = $unionHashRef->{'hasArrayDecoder'}) &&
			($hasArrayDecoder eq 'true')) {
			$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
		    } else { # no array decoder
			if ($self->getExpandArraysFlag() && $self->getLimitExternalUnionArrayFlag() &&
			    $self->getSemiforeEnvGenFlag() && !$self->getSemiforeTBGenFlag()) {
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
								 $arrayMax) == 0) {
				$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
			    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
								      $arrayMin) == 0) {
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = $arrayMax;
			    }
			} elsif ($self->getExpandArraysFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
							       $unionHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
								 $arrayMax) == 1) {
				$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
				$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
			    }
			} else {
			    $currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			    $currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
			}
		    }
		}
	    } else {
		if ($self->getExpandArraysFlag()) {
		    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
						       $unionHashRef->{'arrayDimensionInfo'});
		    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapUnionArrayIndex'},
							 $arrayMax) == 1) {
			$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
			$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
		    }
		} else {
		    $currAddrmapInfo->{'currAddrmapUnionIndex'}++;
		    $currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = -1;
		}
	    }
	}
    } else {
	$currAddrmapInfo->{'currAddrmapUnionIndex'}++;
    }
}

sub getNextUnion {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my ($external, $declaredExternal, $relAddr);
    my ($unionIndex, $bigIntAddr, @prefixList, @hierarchicalPrefixList);
    my ($unionObj, $tmpAddrmapInfo, $unionHashListRef);
    my ($verilogFormat, $offset, $newAddrmapInfo, $newUnionHashRef);

    $unionHashListRef = $designDBRef->_getUnionHashList($addrmapHashRef);
    $unionIndex = $currAddrmapInfo->{'currAddrmapUnionIndex'};
    my $unionHashRef = $unionHashListRef->[$unionIndex];
    $currAddrmapInfo->{'currUnionHashRef'} = $unionHashRef;
    $unionHashRef = $designDBRef->_findAddressmapForInstanceHash($unionHashRef);

    # only 'group' type objects (and registers) can have 'hasArrayDecoder' property.
    if (defined($unionHashRef->{'arrayDimensionInfo'}) && $self->getExpandArraysFlag()) {
	$unionObj = $self->getNextUnionArrayElement($currAddrmapInfo,
						    $addrmapHashRef,
						    $unionHashRef);
    } else {
	($verilogFormat, $offset) = $self->getVerilogBaseAndValue($unionHashRef->{'byteOffset'});
	$bigIntAddr = Math::BigInt->new($offset);
	$bigIntAddr->badd($currAddrmapInfo->{'parentAddr'});
	($verilogFormat, $relAddr) = $self->getVerilogBaseAndValue($unionHashRef->{'addressmapRelativeByteAddress'});
	push(@prefixList, (@{$currAddrmapInfo->{'prefixListRef'}}, $unionHashRef->{'name'}));
	push(@hierarchicalPrefixList, (@{$currAddrmapInfo->{'hierarchicalPrefixListRef'}},
				       $unionHashRef->{'name'}));

	$newUnionHashRef = $unionHashRef;
	$newAddrmapInfo->{'prefixListRef'} = \@prefixList;
	$newAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;
	$newAddrmapInfo->{'currNode'} = $unionHashRef;
	$newAddrmapInfo->{'currAddrmapHashRef'} = $newUnionHashRef;
	$newAddrmapInfo->{'parentAddr'} = $bigIntAddr;
	$newAddrmapInfo->{'arrayIndex'} = undef;

	push(@visitedAddrmapsList, $newAddrmapInfo);
	$self->setInitialAddrmapIndexes($newAddrmapInfo, $newUnionHashRef);
	my $arrayIndex = undef;
	$unionObj = csrPerlAPIUnion->new($newUnionHashRef,
					 $arrayIndex,
					 $addrmapHashRef,
					 $bigIntAddr,
					 $relAddr,
					 $addrmapHashRef->{'busProtocol'},
					 $addrmapHashRef->{'inputPipelineStage'},
					 $addrmapHashRef->{'outputPipelineStage'},
					 \@prefixList,
					 \@hierarchicalPrefixList);
	$newAddrmapInfo->{'currAddrmapObj'} = $unionObj;
	return $unionObj;
    }
}

sub getNextUnionArrayElement {
    my ($self,
	$currAddrmapInfo,
	$addrmapHashRef,
	$unionHashRef) = @_;

    my (@prefixList, @hierarchicalPrefixList);
    my (@addrmapPrefixList, $tmpAddr);
    my ($newAddrmapInfo, $newUnionHashRef);

    my $arrayIndex = $currAddrmapInfo->{'currAddrmapUnionArrayIndex'};
    my ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($unionHashRef->{'byteOffset'});
    my $bigIntAddr = Math::BigInt->new($unionHashRef->{'arrayElementByteSize'});
    if($arrayIndex == -1) {
	# we're just starting to process the first element of the array.
	$arrayIndex = $designDBRef->_getArrayMinIndices($unionHashRef->{'arrayDimensionInfo'});
	$currAddrmapInfo->{'currAddrmapUnionArrayIndex'} = $arrayIndex;
    }
    my $multiplier = $designDBRef->_getMultiplierFromArrayIndex($arrayIndex,
								$unionHashRef->{'arrayDimensionInfo'});
    $bigIntAddr->bmul($multiplier);
    $bigIntAddr->badd($offset);
    $bigIntAddr->badd($currAddrmapInfo->{'parentAddr'});
    @prefixList = @{$currAddrmapInfo->{'prefixListRef'}};
    my $tmpUnionName = $designDBRef->_getArrayElementName($unionHashRef->{'name'}, $arrayIndex);
    push(@prefixList, $tmpUnionName);

    my $hierPrefixListRef = $currAddrmapInfo->{'hierarchicalPrefixListRef'};
    push (@hierarchicalPrefixList, (@{$hierPrefixListRef}, $tmpUnionName));
    $newAddrmapInfo->{'prefixListRef'} = \@prefixList;
    $newAddrmapInfo->{'hierarchicalPrefixListRef'} = \@hierarchicalPrefixList;

    ($verilogFormat, $tmpAddr) = $self->getVerilogBaseAndValue($unionHashRef->{'addressmapRelativeByteAddress'});
    my $addrmapRelByteAddr = Math::BigInt->new($unionHashRef->{'arrayElementByteSize'});
    $addrmapRelByteAddr->bmul($multiplier);
    $addrmapRelByteAddr->badd($tmpAddr);
    $newUnionHashRef = $unionHashRef;
    $newAddrmapInfo->{'currNode'} = $unionHashRef;
    $newAddrmapInfo->{'currAddrmapHashRef'} = $newUnionHashRef;
    $newAddrmapInfo->{'parentAddr'} = $bigIntAddr;
    $newAddrmapInfo->{'arrayIndex'} = $multiplier;
    push(@visitedAddrmapsList, $newAddrmapInfo);
    $self->setInitialAddrmapIndexes($newAddrmapInfo, $addrmapHashRef);
    my $unionObj = csrPerlAPIUnion->new($newUnionHashRef,
					$arrayIndex,
					$addrmapHashRef,
					$bigIntAddr,
					$addrmapRelByteAddr,
					$addrmapHashRef->{'busProtocol'},
					$addrmapHashRef->{'inputPipelineStage'},
					$addrmapHashRef->{'outputPipelineStage'},
					\@prefixList,
					\@hierarchicalPrefixList);
    $newAddrmapInfo->{'currAddrmapObj'} = $unionObj;
    return $unionObj;
}

sub getNextSubRegister {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my $prefixListRef = ();
    my ($tmpSubRegName, $arrayIndex, $wideRelAddrBigInt);
    my ($relAddr, $relAddrBigInt, @hierarchicalPrefixList);
    my ($subRegIndex, $subRegHashRef, $verilogFormat, $offset);
    my ($bigIntAddr, @prefixList, $registerObj, @aliasPrefixList);

    my $regMultiplier = 0;
    my $regHashRef = $currAddrmapInfo->{'currRegisterHashRef'};
    my $subRegHashListRef = $regHashRef->{'subRegisters'};
    $subRegIndex = $currAddrmapInfo->{'currAddrmapSubRegisterIndex'};
    $subRegHashRef = $subRegHashListRef->[$subRegIndex];
    ($verilogFormat, $offset) = $self->getVerilogBaseAndValue($subRegHashRef->{'byteOffset'});
    ($verilogFormat, $relAddr) = $self->getVerilogBaseAndValue($subRegHashRef->{'addressmapRelativeByteAddress'});
    $relAddrBigInt = Math::BigInt->new($relAddr);
    $prefixListRef = $currAddrmapInfo->{'prefixListRef'};
    my $hierPrefixListRef = $currAddrmapInfo->{'hierarchicalPrefixListRef'};

    my $addrmapRelByteAddr;
    if(defined($regHashRef->{'arrayDimensionInfo'})) {
	$arrayIndex = $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'};
	if($arrayIndex == -1) {
	    $arrayIndex = $designDBRef->_getArrayMinIndices($regHashRef->{'arrayDimensionInfo'});
	    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $arrayIndex;
        }
	$regMultiplier = $designDBRef->_getMultiplierFromArrayIndex($arrayIndex,
								    $regHashRef->{'arrayDimensionInfo'});
	$subRegIndex = $currAddrmapInfo->{'currAddrmapSubRegisterIndex'};
        my $subRegListLen = @{$subRegHashListRef};
	$bigIntAddr = Math::BigInt->new($offset);
	$bigIntAddr->badd($currAddrmapInfo->{'parentWideRegAddr'});

	my $elemSizeBigInt = Math::BigInt->new($subRegHashRef->{'arrayElementByteSize'});
	$elemSizeBigInt->bmul($regMultiplier);
	$elemSizeBigInt->bmul($subRegListLen);
	$relAddrBigInt->badd($elemSizeBigInt);

	my $tmpRegName = $designDBRef->_getArrayElementName($subRegHashRef->{'name'}, $arrayIndex);
	$tmpSubRegName = join('_', $subRegHashRef->{'name'}, $subRegIndex);
        if ($viewName eq 'sv') {
            my $regNumDim = scalar(@{$regHashRef->{'arrayDimensionInfo'}});
            for (my $count = 0; $count < $regNumDim; $count++) {
                $prefixListRef->[@$prefixListRef-1] =~ s/(\[\d+\])$//;
                $hierPrefixListRef->[@$hierPrefixListRef-1] =~ s/(\[\d+\])$//;
            }
        }
	push(@prefixList, (@$prefixListRef, $tmpRegName));
	if(defined($regHashRef->{'aliasOf'})) {
	    my $tmpAliasOfName = $designDBRef->_getArrayElementName($regHashRef->{'aliasOf'},
								    $subRegIndex);
	    push(@aliasPrefixList, (@$prefixListRef, $tmpAliasOfName));
	}
	push (@hierarchicalPrefixList, (@$hierPrefixListRef, $tmpRegName));

	my $arraySize = $designDBRef->_getArraySize($regHashRef->{'arrayDimensionInfo'});
	my $parentAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
	$parentAddrmapInfo->{'wideRegArraySize'} = $arraySize;
	$parentAddrmapInfo->{'wideRegArrayIndex'} = $regMultiplier;
	$addrmapRelByteAddr = $relAddrBigInt;
    } else {
	$arrayIndex = undef;
	$bigIntAddr = Math::BigInt->new($offset);
	$bigIntAddr->badd($currAddrmapInfo->{'parentAddr'});
	$prefixListRef = $currAddrmapInfo->{'prefixListRef'};
	$tmpSubRegName = join('_', $subRegHashRef->{'name'});
	push(@prefixList, (@$prefixListRef, $tmpSubRegName));
	if (defined($regHashRef->{'aliasOf'})) {
	    push(@aliasPrefixList, (@$prefixListRef, $regHashRef->{'aliasOf'}));
	}
	push (@hierarchicalPrefixList, (@$hierPrefixListRef, $tmpSubRegName));
	$addrmapRelByteAddr = $relAddr;
    }
    $currAddrmapInfo->{'arrayIndex'} = $currAddrmapInfo->{'currAddrmapSubRegisterIndex'};
    $subRegHashRef->{'subRegisterIndex'} = $currAddrmapInfo->{'currAddrmapSubRegisterIndex'};
    $registerObj = csrPerlAPIRegister->new($subRegHashRef,
					   $arrayIndex,
					   $regHashRef,
					   $bigIntAddr,
					   Math::BigInt->new($offset),
					   $addrmapRelByteAddr,
					   $addrmapHashRef->{'busProtocol'},
					   $addrmapHashRef->{'inputPipelineStage'},
					   $addrmapHashRef->{'outputPipelineStage'},
					   \@prefixList,
					   \@aliasPrefixList,
					   \@hierarchicalPrefixList);
    $registerObj->set_subRegisterIndex($currAddrmapInfo->{'currAddrmapSubRegisterIndex'});
    $registerObj->set_subRegisterFlag('true');
    return $registerObj;
}

sub getNextWideRegArrayElement {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my $oldWideRegRecord = pop(@visitedAddrmapsList);
    # update the prefixList with the parent's prefixList.
    $oldWideRegRecord->{'prefixListRef'} = $visitedAddrmapsList[@visitedAddrmapsList-1]->{'prefixListRef'};
    $oldWideRegRecord->{'hierarchicalPrefixListRef'} =
	$visitedAddrmapsList[@visitedAddrmapsList-1]->{'hierarchicalPrefixListRef'};
    my $registerObj = $self->getNextRegArrayElement($oldWideRegRecord, $addrmapHashRef);
    # update the new register array element record at the end of LIFO. The old
    # register array element is in $currAddrmapInfo record.
    $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $oldWideRegRecord->{'currAddrmapRegisterArrayIndex'};
    $currAddrmapInfo->{'currAddrmapSubRegisterIndex'} = 0;
    $currAddrmapInfo->{'wideRegArraySize'} = $oldWideRegRecord->{'wideRegArraySize'};
    $currAddrmapInfo->{'wideRegArrayIndex'} = $oldWideRegRecord->{'wideRegArrayIndex'};
    return $registerObj;
}
sub updateWideRegisterAccounting {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my ($registerObj, $declaredExternal);

    my $regHashRef = $currAddrmapInfo->{'currRegisterHashRef'};
    my $arrayIndex = $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'};
    my $subRegHashListRef = $regHashRef->{'subRegisters'};
    $currAddrmapInfo->{'currAddrmapSubRegisterIndex'}++;
    if($currAddrmapInfo->{'currAddrmapSubRegisterIndex'} >= @{$subRegHashListRef}) {
	# exhausted the sub register list.
	if(defined($regHashRef->{'arrayDimensionInfo'})) {
	    my $arrayMin = $designDBRef->_getArrayMinIndices($regHashRef->{'arrayDimensionInfo'});
	    my $arrayMax = $designDBRef->_getArrayMaxIndices($regHashRef->{'arrayDimensionInfo'});
	    my $arrayCount = $designDBRef->_getArraySize($regHashRef->{'arrayDimensionInfo'});
	    if (($regHashRef->{'external'} eq 'true') ||
		$designDBRef->_isExternalHashInHierarchy($regHashRef)) {
		$declaredExternal = $regHashRef->{'declaredExternal'};

		if ($declaredExternal eq 'false') {
		    # This register has a parent in the hierarchy which is declared external.
		    if ($arrayCount > 1) {
			if ($self->getExpandArraysFlag()) {
			    if ($self->getLimitExternalRegisterArrayFlag()) {
				if ($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} == -1) {
				    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $arrayMin;
				} elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
									  $arrayMin) == 0) {
				    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $arrayMax;
				} else {
				    $self->doneWithWideRegister();
				}
			    } else {
				$designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
								   $regHashRef->{'arrayDimensionInfo'});
				if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
								     $arrayMax) == 1) { # first arg > 2nd arg
				    $self->doneWithWideRegister();
				}
			    }
			} else {
			    $self->doneWithWideRegister();
			}
		    } else {
			$self->doneWithWideRegister();
		    }
		} else { # is declared external
		    my $hasArrayDecoder = 'false';
		    if (defined($hasArrayDecoder = $regHashRef->{'hasArrayDecoder'}) &&
			($hasArrayDecoder eq 'true')) {
			if (!$self->getExpandArraysFlag()) {
			    $self->doneWithWideRegister();
			} elsif ($self->getLimitExternalRegisterArrayFlag()) {
			    if ($arrayCount > 1) {
				if ($self->getSemiforeEnvGenFlag() && !$self->getSemiforeTBGenFlag()) {
				    my $midwayIndex = $designDBRef->_getArrayIndicesMidWay($regHashRef->{'arrayDimensionInfo'});
				    if ($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} == -1) {
					$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $arrayMin;
				    } elsif (($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
									       $arrayMin) == 0) &&
					     ($designDBRef->_compareArrayIndex($arrayMin, $midwayIndex) != 0)) {
					$currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $midwayIndex;
				    } elsif ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
									      $midwayIndex) == 0) {
					# for the case where arrayCount = 2 & arrayMax = 1
					if ($designDBRef->_compareArrayIndex($midwayIndex, $arrayMax) == 0) {
					    $self->doneWithWideRegister();
					} else {
					    $currAddrmapInfo->{'currAddrmapRegisterArrayIndex'} = $arrayMax;
					}
				    } else {
					$self->doneWithWideRegister();
				    }
				} else {
				    $self->doneWithWideRegister();
				}
			    } else {
				$self->doneWithWideRegister();
			    }
			} elsif (!$self->getSemiforeEnvGenFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
							       $regHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
								 $arrayMax) == 1) { # first arg > 2nd arg
				$self->doneWithWideRegister();
			    }
			} else {
			    $self->doneWithWideRegister();
			}
		    } else { # does not have array decoder
			if ($self->getExpandArraysFlag() && $self->getLimitExternalRegisterArrayFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
							       $regHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
								 $arrayMax) == 1) { # first arg > 2nd arg
				$self->doneWithWideRegister();
			    }
			} elsif ($self->getExpandArraysFlag()) {
			    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
							       $regHashRef->{'arrayDimensionInfo'});
			    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
								 $arrayMax) == 1) { # first arg > 2nd arg
				$self->doneWithWideRegister();
			    }
			} else {
			    $self->doneWithWideRegister();
			}
		    }
		}		    
	    } else {
		if ($self->getExpandArraysFlag()) {
		    $designDBRef->_incrementArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
						       $regHashRef->{'arrayDimensionInfo'});
		    if ($designDBRef->_compareArrayIndex($currAddrmapInfo->{'currAddrmapRegisterArrayIndex'},
							 $arrayMax) == 1) { # first arg > 2nd arg
			$self->doneWithWideRegister();
		    }
		} else {
		    $self->doneWithWideRegister();
		}
	    }
	} else {
	    # we're done with the wide register.
	    $self->doneWithWideRegister();
	}
    }
}

sub getNextWideMemArrayElement {
    my ($self, $currAddrmapInfo, $addrmapHashRef) = @_;

    my $oldWideMemRecord = pop(@visitedAddrmapsList);
    # update the prefixList with the parent's prefixList.
    $oldWideMemRecord->{'prefixListRef'} = $visitedAddrmapsList[@visitedAddrmapsList-1]->{'prefixListRef'};
    $oldWideMemRecord->{'hierarchicalPrefixListRef'} =
	$visitedAddrmapsList[@visitedAddrmapsList-1]->{'hierarchicalPrefixListRef'};

    my $memoryObj = $self->getNextMemArrayElement($oldWideMemRecord, $addrmapHashRef);
    # update the new memory array element record at the end of LIFO. The old
    # memory array element is in $currAddrmapInfo record.
    $currAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
    $currAddrmapInfo->{'currAddrmapMemoryArrayIndex'} = $oldWideMemRecord->{'currAddrmapMemoryArrayIndex'};
    $currAddrmapInfo->{'currAddrmapSubMemoryIndex'} = 0;
    $currAddrmapInfo->{'wideMemArraySize'} = $oldWideMemRecord->{'wideMemArraySize'};
    $currAddrmapInfo->{'wideMemArrayIndex'} = $oldWideMemRecord->{'wideMemArrayIndex'};
    return $memoryObj;
}

sub doneWithWideRegister {
    my $self = shift;
    # we're done with the wide register.
    my $topOfStack = pop(@visitedAddrmapsList);
    my $parentAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
    $parentAddrmapInfo->{'currAddrmapRegisterIndex'}++;
    if (defined($topOfStack->{'wideRegArraySize'})) {
	$parentAddrmapInfo->{'wideRegArraySize'} = $topOfStack->{'wideRegArraySize'};
	$parentAddrmapInfo->{'wideRegArrayIndex'} = $topOfStack->{'wideRegArrayIndex'};
    }
}

sub doneWithWideMemory {
    my $self = shift;
    # we're done with the wide memory.
    my $topOfStack = pop(@visitedAddrmapsList);
    my $parentAddrmapInfo = $visitedAddrmapsList[@visitedAddrmapsList-1];
    $parentAddrmapInfo->{'currAddrmapMemoryIndex'}++;
    $parentAddrmapInfo->{'currAddrmapSubMemoryIndex'} = 0;
    if (defined($topOfStack->{'wideMemArraySize'})) {
	$parentAddrmapInfo->{'wideMemArraySize'} = $topOfStack->{'wideMemArraySize'};
	$parentAddrmapInfo->{'wideMemArrayIndex'} = $topOfStack->{'wideMemArrayIndex'};
    }
}

sub setBusProtocol {
    my ($self,
	$hashRef,
	$parentHashRef,
	$busProtocol) = @_;

    if(defined($hashRef->{'busProtocol'})) {
	$self->set_busProtocol($hashRef->{'busProtocol'});
    } elsif(defined($busProtocol)) {
	$self->set_busProtocol($busProtocol);
    } elsif(defined($parentHashRef) && defined($parentHashRef->{'busProtocol'})) {
	$self->set_busProtocol($parentHashRef->{'busProtocol'});
    } else {
	if(defined(my $parentNode = $self->getParentAddressmapNode())) {
	    $self->set_busProtocol($parentNode->getBusProtocol());
	} else {
	    $self->set_busProtocol(($self->getTopLevelAddressmapNode())->getBusProtocol());
	}
    }
}

sub isInAddressmapInst {
    my $self = shift;

    for (my $index = (@visitedAddrmapsList-1); $index >= 0; $index--) {
	my $addrmap = $visitedAddrmapsList[$index]->{'currAddrmapHashRef'};
	if ($addrmap->{'nodeType'} eq 'addressmapInstance') {
	    return 1;
	}
    }
    return 0;
}
sub getParentAddrmapInst {
    my $self = shift;

    for (my $index = (@visitedAddrmapsList-1); $index >= 0; $index--) {
	my $addrmap = $visitedAddrmapsList[$index]->{'currAddrmapObj'};
	if ($addrmap->getNodeType() eq 'addressmapInstance') {
            for (my $tmpIndex = $index-1; $tmpIndex >=0; $tmpIndex--) {
                my $tmpAddrmap = $visitedAddrmapsList[$tmpIndex]->{'currAddrmapObj'};
                if ($tmpAddrmap->getNodeType() ne 'addressmapInstance') {
                    return $visitedAddrmapsList[$tmpIndex+1]->{'currAddrmapObj'};
                }
            }
            return $addrmap;
	}
    }
    return;
}

sub getOutermostParentAddrmapInst {
    my $self = shift;

    for (my $index = (@visitedAddrmapsList-1); $index >= 0; $index--) {
	my $addrmap = $visitedAddrmapsList[$index]->{'currAddrmapObj'};
	if ($addrmap->getNodeType() eq 'addressmapInstance') {
            my $addrmapInst = $addrmap;
            while ($addrmap->getNodeType() eq 'addressmapInstance') {
                $addrmapInst = $addrmap;
                $addrmap = $addrmap->getParentAddressmapNode();
            }
            return $addrmapInst;
	}
    }
    return;
}

sub isPowerOf2Aligned {
    my ($self, $value) = @_;

    # align value to power of 2, if not power of 2 aligned

    my $tmpAddr = Math::BigInt->new($value);
    # Is it power of 2?
    my $packed = pack('I', $tmpAddr);
    my $numBits = unpack("%32b*", $packed); # count the # of set bits in $packed
    return 1 if ($numBits == 1);
    return 0;
}
sub powerOf2Align {
    my ($self, $value, $index) = @_;

    # align value to power of 2, if not power of 2 aligned

    my $tmpAddr = Math::BigInt->new($value);
    # Is it power of 2?
    my $packed = pack('I', $tmpAddr);
    my $numBits = unpack("%32b*", $packed); # count the # of set bits in $packed
    my $alignedAddr;
    if ($numBits > 1)  { # not power of 2
	if ($value == 0) {
	    $alignedAddr = Math::BigInt->new($value);
	} else {
	    $alignedAddr = Math::BigInt->new(1);
	}
	while ($tmpAddr > 0) {
	    $tmpAddr->brsft(1);
	    $alignedAddr->blsft(1);
	}
    } else {
	$alignedAddr = Math::BigInt->new($value);
    }
    while ($index) {
	$alignedAddr->blsft(1);
	$index--;
    }
    $value = $alignedAddr;
    return $value;
}

sub isRegisterFitDatapath {
    my $self = shift;

    my $flag=0;
    return 1 if ($self->isRegisterNode() &&
                 defined($flag = $self->get_registerFitDatapath()) && ($flag == 1));
    return 0;
}
1;
