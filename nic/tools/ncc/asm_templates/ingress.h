//:: import os, pdb
//:: from collections import OrderedDict

//:: pddict = _context['pddict']
//:: #pdb.set_trace()

/*
 * ingress.h
 * Mahesh Shirshyad (Pensando Systems)
 */

/* This file contains assembly level data structures for all Ingress processing
 * needed for MPU to read and act on action data and action input. 
 *
 * Every Ingress P4table after match hit can optionally provide
 *   1. Action Data (Parameters provided in P4 action functions)
 *   2. Action Input (Table Action routine using data extracted
 *                    into PHV either from header or result of
 *                    previous table action stored in PHV)
 */

/*
 * This file is generated from P4 program. Any changes made to this file will
 * be lost.
 */

/* TBD: In HBM case actiondata need to be packed before and after Key Fields
 * For now all actiondata follows Key 
 */

//::     for table in pddict['tables']:
//::        if pddict['tables'][table]['direction'] == "EGRESS":
//::            continue
//::        #endif

/* ASM Key Structure for p4-table '${table}' */
//::        if pddict['tables'][table]['type'] == 'Hash':
/* P4-table '${table}' is hash table */
//::        elif pddict['tables'][table]['type'] == 'Hash_OTcam':
/* P4-table '${table}' is hash table with over flow tcam */
//::        elif pddict['tables'][table]['type'] == 'Index':
/* P4-table '${table}' is index table */
//::        elif pddict['tables'][table]['type'] == 'Ternary':
/* P4-table '${table}' ternary table.*/
//::        else:
/* P4-table '${table}' Mpu/Keyless table.*/
//::        #endif

/* K + I fields */
//::        if len(pddict['tables'][table]['asm_ki_fields']):
//::            pad_to_512 = 0
struct ${table}_k {
//::            for fields in pddict['tables'][table]['asm_ki_fields']:
//::                if len(fields) == 2:
//::                    ustr, uflds = fields
//::                    all_fields_of_header_in_same_byte = {}
//::                    for fields in uflds:
//::                        (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                         flit, flitoffset, typestr, hdrname) = fields
//::                        if hdrname in all_fields_of_header_in_same_byte.keys():
//::                            all_fields_of_header_in_same_byte[hdrname] += p4fldwidth 
//::                        else:
//::                            all_fields_of_header_in_same_byte[hdrname] = p4fldwidth 
//::                        #endif
//::                    #endfor
//::                    max_fld_union_key_len = max(x for x in all_fields_of_header_in_same_byte.values())
//::                    pad_to_512 += max_fld_union_key_len 
    union { /* Sourced from field/hdr union */
//::                    all_fields_of_header_in_same_byte = []
//::                    for fields in uflds:
//::                        (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                         flit, flitoffset, typestr, hdrname) = fields
//::                        if p4fldwidth == max_fld_union_key_len:
        /* FieldType = ${typestr} */
        ${multip4fldname} : ${p4fldwidth}; /* phvbit[${phvbit}], Flit[${flit}], FlitOffset[${flitoffset}] */
//::                        else:
//::                            if hdrname not in all_fields_of_header_in_same_byte:
        struct {
//::                                all_fields_of_header_in_same_byte.append(hdrname)
//::                                _total_p4fldwidth = 0
//::                                for _fields in uflds:
//::                                    (_multip4fldname, _p4fldname, _p4fldwidth, _phvbit, \
//::                                    _flit, _flitoffset, _typestr, _hdrname) = _fields
//::                                    if _hdrname == hdrname:
            /* K/I = ${_typestr} */
            ${_multip4fldname} : ${_p4fldwidth}; /* phvbit[${_phvbit}], Flit[${_flit}], FlitOffset[${_flitoffset}] */
//::                                        _total_p4fldwidth += _p4fldwidth
//::                                    #endif
//::                                #endfor
//::                                padlen = max_fld_union_key_len - _total_p4fldwidth
//::                                if padlen:
            /* Padded to align with unionized p4field */
            _pad_${p4fldname} : ${padlen};
//::                                #endif
        };
//::                            #endif
//::                        #endif
//::                    #endfor
    };
//::                else:
//::                    (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                    flit, flitoffset, typestr, hdrname) = fields
    /* FieldType = ${typestr} */
    ${multip4fldname} : ${p4fldwidth}; /* phvbit[${phvbit}], Flit[${flit}], FlitOffset[${flitoffset}] */
//::                    pad_to_512 += p4fldwidth
//::                #endif
//::            #endfor
//::            if pad_to_512 < 512:
//::                pad_to_512 = 512 - pad_to_512
    __pad_to_512b : ${pad_to_512};
//::            #endif
};
//::        #endif # if k+i len > 0

/* K + D fields */
//::        max_kd_size = 0
//::        pad_to_512 = 0
//::        for action in pddict['tables'][table]['actions']:
//::            (actionname, actionfldlist) = action
//::            kd_size = 0
//::            if len(actionfldlist):
struct ${table}_${actionname}_d {
//::                if pddict['tables'][table]['type'] == 'Hash':
//::                    for fields in pddict['tables'][table]['asm_kd_fields']:
//::                        if len(fields) == 2:
//::                            ustr, uflds = fields
//::                            all_fields_of_header_in_same_byte = {}
//::                            for fields in uflds:
//::                                (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                                 flit, flitoffset, typestr, hdrname) = fields
//::                                if hdrname in all_fields_of_header_in_same_byte.keys():
//::                                    all_fields_of_header_in_same_byte[hdrname] += p4fldwidth 
//::                                else:
//::                                    all_fields_of_header_in_same_byte[hdrname] = p4fldwidth 
//::                                #endif
//::                            #endfor
//::                            max_fld_union_key_len = max(x for x in all_fields_of_header_in_same_byte.values())
//::                            pad_to_512 += max_fld_union_key_len 
    union { /* Sourced from field/hdr union */
//::                            all_fields_of_header_in_same_byte = []
//::                            for fields in uflds:
//::                                (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                                flit, flitoffset, typestr, hdrname) = fields
//::                                if p4fldwidth == max_fld_union_key_len:
        /* FieldType = ${typestr} */
        ${multip4fldname} : ${p4fldwidth}; /* phvbit[${phvbit}], Flit[${flit}], FlitOffset[${flitoffset}] */
//::                                else:
//::                                    if hdrname not in all_fields_of_header_in_same_byte:
        struct {
//::                                        all_fields_of_header_in_same_byte.append(hdrname)
//::                                        _total_p4fldwidth = 0
//::                                        for _fields in uflds:
//::                                            (_multip4fldname, _p4fldname, _p4fldwidth, _phvbit, \
//::                                            _flit, _flitoffset, _typestr, _hdrname) = _fields
//::                                            if _hdrname == hdrname:
            /* K/I = ${_typestr} */
            ${_multip4fldname} : ${_p4fldwidth}; /* phvbit[${_phvbit}], Flit[${_flit}], FlitOffset[${_flitoffset}] */
//::                                                _total_p4fldwidth += _p4fldwidth
//::                                            #endif
//::                                        #endfor
//::                                        padlen = max_fld_union_key_len - _total_p4fldwidth
//::                                        if padlen:
            /* Padded to align with unionized p4field */
            _pad_${p4fldname} : ${padlen};
//::                                        #endif
//::                                    #endif
        };
//::                                #endif
//::                            #endfor
    };
//::                        else:
//::                            (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                            flit, flitoffset, typestr, hdrname) = fields
    /* FieldType = ${typestr} */
    ${multip4fldname} : ${p4fldwidth}; /* phvbit[${phvbit}], Flit[${flit}], FlitOffset[${flitoffset}] */
//::                            pad_to_512 += p4fldwidth
//::                        #endif
//::                    #endfor
//::                #endif

//::                kd_size = pad_to_512
//::                for actionfld in actionfldlist:
//::                    actionfldname, actionfldwidth = actionfld
    ${actionfldname} : ${actionfldwidth};
//::                    kd_size += actionfldwidth 
//::                #endfor
};
//::            #endif
//::            if kd_size > max_kd_size:
//::                max_kd_size = kd_size
//::            #endif
//::        #endfor

//::        if len(pddict['tables'][table]['actions']) > 1:
struct ${table}_d {
    action_id : 8; /* TBD: Does this need to go after listing all actions? */
//::            empty_action = True
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionfldlist) = action
//::                if len(actionfldlist):
//::                    empty_action = False
//::                    break
//::                #endif
//::            #endfor
//::            if not empty_action:
    union {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionfldlist) = action
//::                    if len(actionfldlist):
        struct ${table}_${actionname}_d  ${actionname}_d;
//::                    #endif
//::                #endfor
    } u;
//::            #endif
//::            if (8 + max_kd_size) < 512:
//::                pad_to_512 = 512 - (8 + max_kd_size)
    __pad_to_512b : ${pad_to_512};
//::            #endif
};
//::        elif len(pddict['tables'][table]['actions']) == 1:
//::            (actionname, actionfldlist) = pddict['tables'][table]['actions'][0]
//::            if len(actionfldlist):
struct ${table}_d {
    struct ${table}_${actionname}_d  ${actionname}_d;
//::            if (max_kd_size) < 512:
//::                pad_to_512 = 512 - max_kd_size
    __pad_to_512b : ${pad_to_512};
//::            #endif
};
//::            #endif
//::        #endif


//::     #endfor


