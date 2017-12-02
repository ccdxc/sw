Parser:
Separate capri_parser objects are created for ingress and egress. It maintains
information about the parse graph - stages, paths, extracted fields, ohi fields.
A capri_state object is used to represent each state in parser. Capri state 
stores capri specific information in addition to state information obtained
from the P4 program.

* Initialize()
    Locate start state. The 'start' state for ingress must be named as 'start'
    while start state for egress is 'egress_start'. The egress_start is 
    optional, if not specified, 'start' state is used for egress as well.

    Parser states are topologically sorted to get an allocation order for 
    the header fields in phv. Special pragma "header_ordering" is used to 
    handle loops that are caused by TLV/option parsing states. Parser loops
    that use header-stack (aka virtual headers) can be un-rolled to the max
    header stack depth. A 'virtual' state is added to parser graph as a place
    holder so that all parent states can continue to point to that. The virtual
    state is not programmed in the hardware. As part of unrolling, a stage is
    added to extract 'next' (converted to positional index) header and then
    un-conditionally transition to the next-state until end of stack. The last
    state preserves the branch information (_on, _to) of the original state to
    exit the loop.

    For every state (in topo-order) in the parser graph, collect the field
    extraction and set_metadata operations. Header fields that are not used in
    P4 match-action pipeline are extracted using OHI (original header 
    information)
    A OHI is a set of (offset, len) pairs which hardware uses to preserve
    packet bytes from a given offset. The bytes are available to deparser to
    add them back to the the packet as necessary. Due to hardware requirements,
    only 24 OHI pairs are allowed per parser and only 3 pairs per state.

* Parser state transition:
    Each parser state is given a unique id. Parser sate transition uses current 
    state's id and user specified fields as a key to lookup a state transition
    TCAM.  Each TCAM and associated SRAM entry represents a branch in a parse 
    graph. 
    Capri-ncc uses a 'pre-extraction' mode, in which a headers extracted in 
    state-A are extracted on every branch entering state-A. The current_offset 
    in the packet is set past the headers extracted in state-A when entering 
    the state. This approach saves on number of exit states. Since harware does
    not allow to specify extraction instructions on very first state, any
    headers extracted in the start state are extracted on exit branch along
    with the header pre-extraction of the next state. This is an exception to
    pre-extraction logic
    Other approach considered as 'post-extract' performs header extraction of 
    headers in a state-A on every branch leaving that state. This approach is 
    not used since it requires more TCAM/SRAM entries. Also when a variable len
    header is extracted, next lkp values must be loaded using current_offset +
    var_len_expression. This is not possible in the Capri hardware. Capri h/w
    does not provide a way to load from packet[current_ofset + var_len_expr]

* Parser-Local fields:

* TLV parsing:
    next_offset: 
    lkp_offset:

    Loops, hardware abilities TBD

* Saved matches:

* pragmas:
