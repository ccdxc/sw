#ifndef __P4PD_HPP__
#define __P4PD_HPP__

// NWSEC profile table
#define L4_PROF_DEFAULT_ENTRY       0

// Flow info table
#define FLOW_INFO_MISS_ENTRY        0
#define FLOW_INFO_DROP_ENTRY        1

// Session State table
#define SESSION_STATE_NOP_ENTRY     0

// MET(Replication List) table (From p4/nw/include/defins.h)
#define REPL_LIST_INDEX_FIN_COPY        P4_NW_MCAST_INDEX_FIN_COPY
#define REPL_LIST_INDEX_RST_COPY        P4_NW_MCAST_INDEX_RST_COPY
#define REPL_LIST_INDEX_FLOW_REL_COPY   P4_NW_MCAST_INDEX_FLOW_REL_COPY 


#endif  // __P4PD_HPP__
