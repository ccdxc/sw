# Flow Classifier 

This library is used to perform an N-tuple search over a set of rules with 
multiple categories and find the best match (highest priority) for each category. 

## Features

- Multi field classifier with ternary match on key fields
- Genenric key structure with felxible field definition
- Supported key field types
    - Prefix match (e.g IP prefix 10.1.0.0/16)
    - Range ( e.g ip/port ranges)
    - Exact match with bit mask 
- Incremental insert/delete/update operation without expensive rule pre-processing
    -  K * O(log N)  lookup/update complexity ( K -- no.of key fields, N -- no.of rules)
    -  O(N) space complexity
- Rule priority as tie breaker among multiple matching rules
- Support for parallel lookup of multiple results using rule categories
- Atomic bulk rule updates
- Concurrent rule updates with no reader locks

## Design

### Key definition

For each classifier context, user can specify a set of fields over which packet 
classification will be performed.

Key is treated as a byte array and divided in to multiple key fields. Key fields are 
defined with the following properties
- field-type (prefix match,range match or exact match)
- size (1, 2, 4 or 8 bytes)
- offset

### Rule partitioning

Rules are partioned into K sub-tables (K is the no.of key fields). Each individual
field in the key is used as index into one of the sub-tables. Rules are stored 
in the sub-tables using one of the following  data structures based on the type 
of the key field

 - Prefix match - Multibit trie
 - Range match - Interval tree
 - Exact match - Hashtable

Each rule is stored in only one of the sub-tables - O(N) space complexity.

### Insertion

During insertion all sub-tables are evaluated and rule is inserted 
into the table with least heuristic lookup cost for that rule. Since
each underlying data-structurtes support O(log N) insertion, total insertion
complexity will be K * O(log N)

### Lookup

During lookup each of the sub-tables are searched in sequence and the highest 
priority result for each of the category is selected. Since
each underlying data-structurtes support O(log N) lookup, Lookup time complexity
is also K * O(log N)

### Updates

Sub-tables are implemnted using persistant data strcutures which always preserve 
one previous version of itself when they are modified. As their operations do not 
(visibly) update the structure in-place, but instead always yield a new updated 
structure, reader threads doesn't need to be locked during update.

Atomic batch updates are implementd by making all the changes to a new copy-on-write 
cloned version of the underlying data structure and atomically swapping the version
at the end of the batch update.

### Rule categories

Each rule uses a bit mask value to select the relevant category(s) for the rule. 
When a lookup is performed, the result for each category is returned. This effectively 
provides a “parallel lookup” by enabling a single search to return multiple results if, 
for example, there were four different sets of  rules, one for access control, 
one for routing, one for qos marking and so on. Each set could be assigned its own 
category and by combining them into a single database, one lookup returns a result 
for each of the four sets.


## Sample usage

Sample usage of ACL library to define a five tuple classifier with the following key struct

    struct flow_key_t {
        uint32_t src_ip;
        uint32_t dst_ip;
        uint8_t  proto;
        uint16_t sport;
        uint16_t dport;
    };
       
1. Define fields and create acl ctx

        RTE_ACL_RULE_DEF(ipv4_rule_t, 5);
        enum {
           FLD_SIP = 0, FLD_DIP, FLD_SPORT, FLD_DPORT, FLD_PROTO
        }
        
        acl_param_t params = {
            name: "ipv4_rules",
            num_categories: 1,
            num_fields: 5,
            fields: {
                { ACL_FIELD_TYPE_PREFIX, offsetof(flow_key_t, src_ip), 
                  sizeof(((flow_key_t*)0)->src_ip) },
                { ACL_FIELD_TYPE_PREFIX, offsetof(flow_key_t, dst_ip), 
                  sizeof(((flow_key_t*)0)->dst_ip) },
                { ACL_FIELD_TYPE_RANGE, offsetof(flow_key_t, sport), 
                  sizeof(((flow_key_t*)0)->sport) },
                { ACL_FIELD_TYPE_RANGE, offsetof(flow_key_t, dport), 
                  sizeof(((flow_key_t*)0)->dport) },
                { ACL_FIELD_TYPE_EXACT, offsetof(flow_key_t, proto), 
                  sizeof(((flow_key_t*)0)->proto) },
            }
        };

        acl_ctx_t *ctx = acl_create(&params);
         

2. Define rules

        ipv4_rule_t rule = {};
          
        /* src: 10.0.0.0/16, dst: 192.168.10.0/24, proto=TCP, sport=* dport=0-1024 */
        rule.data.category_mask = 0x01;
        rule.data.priority = 1;
        rule.data.userdata = <>;
        
        rule.filed[FLD_SIP].value.u32 = IPv4(10,0,0,0);
        rule.filed[FLD_SIP].mask_range.u32 = 16;
        rule.filed[FLD_DIP].value.u32 = IPv4(192,168,10,0);
        rule.filed[FLD_DIP].mask_range.u32 = 24;
        rule.filed[FLD_PROTO].value.u8 = TCP;
        rule.filed[FLD_PROTO].mask_range.u8 = 0xff;
        rule.filed[FLD_DPORT].value.u16 = 0;
        rule.filed[FLD_DPORT].mask_range.u32 = 1024;
        

3. Add rules to acl ctx

        acl_update_ctx_t *upd = acl_update_begin(ctx);
        acl_add_rule(upd, &rule);
        // add more rules
        acl_update_commit(upd);

4. Do lookup 

        flow_key_t key = { src_ip: IPv4(10,0,1,100), dst_ip: IPv4(192,168,10,10), 
                           proto: TCP, sport: 100, dport:80 };
        uint8_t results = {};  // one category
        acl_classify(ctx, &key, &result, 0x01);
        
