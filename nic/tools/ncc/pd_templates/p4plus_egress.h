//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: #pdb.set_trace()
//:: if pddict['p4plus']:
//::     caps_p4prog = '_' + pddict['p4program'].upper() + '_'
//::     p4prog = pddict['p4program']
//::     prefix = 'p4pd_' + pddict['p4program']
/*
 * ${p4prog}_p4plus_ingress.h
 * Pensando Systems
 */
/* This file contains action data bit field data structures for p4+
 * programs to update table actiond data. The data structure generated
 * here will match with ASM _d structures.
 *
 */
/*
 * This file is generated from P4+ program. Any changes made to this file will
 * be lost.
 */

#ifndef _${caps_p4prog}P4PLUS_EGRESS_H__
#define _${caps_p4prog}P4PLUS_EGRESS_H__

#include <inttypes.h>

//::     for table in pddict['tables']:
//::        if pddict['tables'][table]['direction'] == "INGRESS":
//::            continue
//::        #endif

/* K + D fields */
//::        pad_to_512 = 0
//::        for action in pddict['tables'][table]['actions']:
//::            (actionname, actionfldlist, _) = action
//::            kd_size = 0
//::            if len(actionfldlist):
struct __attribute__((__packed__)) ${table}_${actionname}_d {

//::                kd_size = pad_to_512
//::                for actionfld in actionfldlist:
//::                    actionfldname = actionfld['asm_name']
//::                    actionfldwidth = actionfld['len']
//::                    if actionfldwidth <= 64:
    uint64_t ${actionfldname} : ${actionfldwidth};
//::                    else:
    uint8_t ${actionfldname}[${actionfldwidth}/8]; // bitwidth = ${actionfldwidth}
//::                    #endif
//::                    kd_size += actionfldwidth 
//::                #endfor
};
//::            #endif
//::        #endfor

//::        if len(pddict['tables'][table]['actions']) > 1:
struct __attribute__((__packed__)) ${table}_d {
//::            if not (pddict['tables'][table]['is_raw']):
    uint64_t action_id : 8;
//::            #endif
//::            empty_action = True
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionfldlist, _) = action
//::                if len(actionfldlist):
//::                    empty_action = False
//::                    break
//::                #endif
//::            #endfor
//::            if not empty_action:
    union {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionfldlist, _) = action
//::                    if len(actionfldlist):
        struct ${table}_${actionname}_d  ${actionname}_d;
//::                    #endif
//::                #endfor
    } u;
//::            #endif
};
//::        elif len(pddict['tables'][table]['actions']) == 1:
//::            (actionname, actionfldlist, _) = pddict['tables'][table]['actions'][0]
//::            if len(actionfldlist):
struct __attribute__((__packed__)) ${table}_d {
    struct ${table}_${actionname}_d  ${actionname}_d;
};
//::            #endif
//::        #endif
//::     #endfor

#endif
//:: #endif
