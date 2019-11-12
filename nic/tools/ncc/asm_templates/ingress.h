//:: import os, pdb
//:: from collections import OrderedDict

//:: pddict = _context['pddict']
//:: #pdb.set_trace()

/*
 * ingress.h
 * Pensando Systems
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

//::     k_d_action_data_json = {}
//::     k_d_action_data_json['INGRESS_KD'] = {}
//::     for table in pddict['tables']:
//::        if pddict['tables'][table]['direction'] == "EGRESS":
//::            continue
//::        #endif
//::        if pddict['tables'][table]['hash_overflow'] and not pddict['tables'][table]['otcam']:
//::            # Skip generating KI and KD for hash overflow table.
//::            # Overflow table will use KI and KD of regular table.
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
//::                if fields[0] == 'unionized':
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
//::                            if phvbit or flit or flitoffset:
        ${multip4fldname} : ${p4fldwidth}; /* phvbit[${phvbit}], Flit[${flit}], FlitOffset[${flitoffset}] */
//::                            else:
        ${multip4fldname} : ${p4fldwidth}; 
//::                            #endif
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
//::                                        if _phvbit or _flit or _flitoffset:
            ${_multip4fldname} : ${_p4fldwidth}; /* phvbit[${_phvbit}], Flit[${_flit}], FlitOffset[${_flitoffset}] */
//::                                        else:
            ${_multip4fldname} : ${_p4fldwidth};
//::                                        #endif
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
//::                    if phvbit or flit or flitoffset:
    ${multip4fldname} : ${p4fldwidth}; /* phvbit[${phvbit}], Flit[${flit}], FlitOffset[${flitoffset}] */
//::                    else:
    ${multip4fldname} : ${p4fldwidth};
//::                    #endif
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
//::        k_d_action_data_json['INGRESS_KD'][table] = {}
//::        _start_kd_bit = 0
//::        if len(pddict['tables'][table]['actions']) > 1:
//::            if not (pddict['tables'][table]['is_raw']):
//::                _start_kd_bit = 8
//::            #endif
//::        #endif
//::        for action in pddict['tables'][table]['actions']:
//::            pad_to_512 = 0
//::            (actionname, actionflddict, _) = action
//::            kd_size = 0
//::            pad_data_bits = 0
//::            totaladatabits = 0
//::            kd_json = OrderedDict()
//::            _kdbit = _start_kd_bit
//::            if len(actionflddict):
//::                totaladatabits = 0
//::                for actionfld in actionflddict:
//::                    actionfldname  = actionfld['asm_name']
//::                    actionfldwidth = actionfld['len']
//::                    totaladatabits += actionfldwidth
//::                #endfor
struct ${table}_${actionname}_d {
//::                # Action Data before Key bits in case of Hash table.
//::                adata_bits_before_key = 0
//::                if not pddict['tables'][table]['is_toeplitz'] and pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
//::                    mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::                    if len(pddict['tables'][table]['actions']) > 1:
//::                        actionpc_bits = 8
//::                    else:
//::                        actionpc_bits = 0
//::                    #endif
//::                    adata_bits_before_key = mat_key_start_bit - actionpc_bits
//::                    axi_pad_bits = 0
//::                    spilled_adata_bits = 0
//::                    max_adata_bits_before_key = min(totaladatabits, adata_bits_before_key)
//::                    if totaladatabits < mat_key_start_bit and (mat_key_start_bit - totaladatabits) > 16:
//::                        spilled_adata_bits = totaladatabits % 16
//::                        max_adata_bits_before_key = totaladatabits - spilled_adata_bits
//::                    #endif
//::                    if adata_bits_before_key > max_adata_bits_before_key:
//::                        # Pad axi shift amount of bits
//::                        axi_pad_bits = ((adata_bits_before_key - max_adata_bits_before_key) >> 4) << 4
//::                        if axi_pad_bits:
    __pad_axi_shift_bits : ${axi_pad_bits};
//::                            kd_json[_kdbit] = {'bit': _kdbit, 'width': axi_pad_bits, 'field': '__pad_axi_shift_bits'}
//::                            _kdbit += axi_pad_bits
//::                            kd_size += axi_pad_bits
//::                        #endif
//::                    #endif
    
//::                    last_actionfld_bits = 0
//::                    if adata_bits_before_key:
//::                        total_adatabits_beforekey = 0
//::                        fill_adata = max_adata_bits_before_key
//::                        for actionfld in actionflddict:
//::                            actionfldname  = actionfld['asm_name']
//::                            actionfldwidth = actionfld['len']
//::                            dvec_start = actionfld['dvec_start']
//::                            fle_start = 511-dvec_start
//::                            little_str = ''
//::                            if actionname in pddict['tables'][table]['le_action_params'].keys():
//::                                if actionfldname in pddict['tables'][table]['le_action_params'][actionname]:
//::                                    little_str = ' (little)'
//::                                #endif
//::                            #endif
//::                            if actionfldwidth <= fill_adata:
    ${actionfldname} : ${actionfldwidth} ${little_str}; /* BE[${dvec_start}], FLE[${fle_start}] */
//::                                kd_json[_kdbit] = {'bit': _kdbit, 'width': actionfldwidth, 'field': actionfldname}
//::                                _kdbit += actionfldwidth
//::                                fill_adata -= actionfldwidth
//::                                last_actionfld_bits = actionfldwidth
//::                                total_adatabits_beforekey += actionfldwidth
//::                            else:
    ${actionfldname}_sbit0_ebit${fill_adata - 1} : ${fill_adata} ${little_str}; /* BE[${dvec_start}], FLE[${fle_start}] */
//::                                kd_json[_kdbit] = {'bit': _kdbit, 'width': fill_adata, 'field': actionfldname + '_sbit0_ebit' + str(fill_adata-1)}
//::                                _kdbit += fill_adata
//::                                last_actionfld_bits = fill_adata
//::                                total_adatabits_beforekey += fill_adata
//::                                fill_adata = 0
//::                            #endif
//::                            if not fill_adata:
//::                                last_actionfldname = actionfldname
//::                                break
//::                            #endif
//::                        #endfor
//::                        # Pad when adata size/bits < match-key-start-bit
//::                        if (axi_pad_bits + total_adatabits_beforekey + actionpc_bits) < mat_key_start_bit:
//::                            pad_data_bits = mat_key_start_bit - (axi_pad_bits + total_adatabits_beforekey + actionpc_bits)
    __pad_adata_key_gap_bits : ${pad_data_bits};
//::                            kd_json[_kdbit] = {'bit': _kdbit, 'width': pad_data_bits, 'field': '__pad_adata_key_gap_bits'}
//::                            _kdbit += pad_data_bits
//::                            kd_size += pad_data_bits
//::                        #endif
//::                    #endif
//::                    if pddict['tables'][table]['is_wide_key']:
//::                        _match_key_bit_length = pddict['tables'][table]['wide_key_len']
//::                    else:
//::                        _match_key_bit_length = pddict['tables'][table]['match_key_bit_length']
//::                    #endif
//::                    if pddict['tables'][table]['include_k_in_d']:
//::                        startindex = 0
//::                        endindex = len(pddict['tables'][table]['asm_ki_fields'])
//::                        keyencountered = False
//::                        for fields in pddict['tables'][table]['asm_ki_fields']:
//::                            if fields[0] == 'unionized':
//::                                ustr, uflds = fields
//::                                increment_index = False
//::                                for fields in uflds:
//::                                     (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                                     flit, flitoffset, typestr, hdrname) = fields
//::                                     if not keyencountered:
//::                                         if typestr != 'K':
//::                                             increment_index = True
//::                                         else:
//::                                             keyencountered = True
//::                                         #endif
//::                                     #endif
//::                                #endfor
//::                                if increment_index:
//::                                    startindex += 1
//::                                #endif
//::                            else:
//::                                (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                                flit, flitoffset, typestr, hdrname) = fields
//::                                if not keyencountered:
//::                                    if typestr != 'K':
//::                                        startindex += 1
//::                                    else:
//::                                        keyencountered = True
//::                                    #endif
//::                                #endif
//::                            #endif
//::                        #endfor
//::                        keyencountered = False
//::                        for fields in reversed(pddict['tables'][table]['asm_ki_fields']):
//::                            if fields[0] == 'unionized':
//::                                ustr, uflds = fields
//::                                decrement_index = False
//::                                for fields in uflds:
//::                                    (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                                    flit, flitoffset, typestr, hdrname) = fields
//::                                    if not keyencountered:
//::                                        if typestr != 'K':
//::                                            decrement_index = True 
//::                                        else:
//::                                            keyencountered = True
//::                                            break
//::                                        #endif
//::                                    #endif
//::                                #endfor
//::                                if decrement_index:
//::                                    endindex -= 1
//::                                #endif
//::                            else:
//::                                (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                                flit, flitoffset, typestr, hdrname) = fields
//::                                if not keyencountered:
//::                                    if typestr != 'K':
//::                                        endindex -= 1
//::                                    else:
//::                                        keyencountered = True
//::                                        break
//::                                    #endif
//::                                #endif
//::                            #endif
//::                        #endfor
//::                        for fields in pddict['tables'][table]['asm_ki_fields'][startindex:endindex]:
//::                            if fields[0] == 'unionized':
//::                                ustr, uflds = fields
//::                                all_fields_of_header_in_same_byte = {}
//::                                for fields in uflds:
//::                                    (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                                     flit, flitoffset, typestr, hdrname) = fields
//::                                    if hdrname in all_fields_of_header_in_same_byte.keys():
//::                                        all_fields_of_header_in_same_byte[hdrname] += p4fldwidth 
//::                                    else:
//::                                        all_fields_of_header_in_same_byte[hdrname] = p4fldwidth 
//::                                    #endif
//::                                #endfor
//::                                max_fld_union_key_len = max(x for x in all_fields_of_header_in_same_byte.values())
    union { /* Sourced from field/hdr union */
//::                                all_fields_of_header_in_same_byte = []
//::                                for fields in uflds:
//::                                    (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                                     flit, flitoffset, typestr, hdrname) = fields
//::                                    if p4fldwidth == max_fld_union_key_len:
        /* Field Type = ${typestr} */
//::                                        if phvbit or flit or flitoffset:
        ${multip4fldname} : ${p4fldwidth}; /* phvbit[${phvbit}], Flit[${flit}], FlitOffset[${flitoffset}] */
//::                                        else:
        ${multip4fldname} : ${p4fldwidth};
//::                                        #endif
//::                                    else:
//::                                        if hdrname not in all_fields_of_header_in_same_byte:
        struct {            
//::                                            all_fields_of_header_in_same_byte.append(hdrname)
//::                                            _total_p4fldwidth = 0
//::                                            for _fields in uflds:
//::                                                (_multip4fldname, _p4fldname, _p4fldwidth, _phvbit, \
//::                                                _flit, _flitoffset, _typestr, _hdrname) = _fields
//::                                                if _hdrname == hdrname:
            /* K/I = ${_typestr} */
//::                                                    if _phvbit or _flit or _flitoffset:
            ${_multip4fldname} : ${_p4fldwidth}; /* phvbit[${_phvbit}], Flit[${_flit}], FlitOffset[${_flitoffset}] */
//::                                                    else:
            ${_multip4fldname} : ${_p4fldwidth};
//::                                                    #endif
//::                                                    _total_p4fldwidth += _p4fldwidth
//::                                                #endif
//::                                            #endfor
//::                                            padlen = max_fld_union_key_len - _total_p4fldwidth
//::                                            if padlen:
            /* Padded to align with unionized p4field */
            _pad_${p4fldname} : ${padlen};
//::                                            #endif
        };
//::                                        #endif
//::                                    #endif
//::                                #endfor
    };
//::                            else:
//::                                (multip4fldname, p4fldname, p4fldwidth, phvbit, \
//::                                flit, flitoffset, typestr, hdrname) = fields
    /* FieldType= ${typestr} */
//::                                if phvbit or flit or flitoffset:
    ${multip4fldname} : ${p4fldwidth}; /* phvbit[${phvbit}], Flit[${flit}], FlitOffset[${flitoffset}] */
//::                                else:
    ${multip4fldname} : ${p4fldwidth};
//::                                #endif
//::                            #endif
//::                        #endfor
//::                    else:
//::                        # Table Key bits -- Pad it here
    __pad_key_bits : ${_match_key_bit_length}; /* Entire Contiguous Keybits */
//::                        kd_json[_kdbit] = {'bit': _kdbit, 'width': _match_key_bit_length, 'field': '__pad_key_bits'}
//::                        _kdbit += _match_key_bit_length
//::                    #endif
//::                    kd_size += _match_key_bit_length
//::                #endif
//::                if adata_bits_before_key:
//::                    # Pack remaining Action Data bits after Key
//::                    skip_adatafld  = True
//::                    for actionfld in actionflddict:
//::                        actionfldname  = actionfld['asm_name']
//::                        actionfldwidth = actionfld['len']
//::                        dvec_start = actionfld['dvec_start']
//::                        fle_start = 511-dvec_start
//::                        little_str = ''
//::                        if actionname in pddict['tables'][table]['le_action_params'].keys():
//::                            if actionfldname in pddict['tables'][table]['le_action_params'][actionname]:
//::                                little_str = ' (little)'
//::                            #endif
//::                        #endif
//::                        if skip_adatafld and actionfldname != last_actionfldname:
//::                            continue
//::                        elif skip_adatafld:
//::                            skip_adatafld  = False
//::                            if actionfldwidth > last_actionfld_bits:
    ${actionfldname}_sbit${last_actionfld_bits}_ebit${actionfldwidth - 1} : ${actionfldwidth - last_actionfld_bits} ${little_str}; /* BE[${dvec_start}], FLE[${fle_start}] */
//::                                kd_json[_kdbit] = {'bit': _kdbit, 'width': actionfldwidth - last_actionfld_bits, 'field': actionfldname + '_sbit' + str(last_actionfld_bits) + '_ebit' + str(actionfldwidth - 1)}
//::                                _kdbit += (actionfldwidth - last_actionfld_bits)
//::                            #endif
//::                            continue
//::                        #endif
    ${actionfldname} : ${actionfldwidth}; /* BE[${dvec_start}], FLE[${fle_start}] */
//::                            kd_json[_kdbit] = {'bit': _kdbit, 'width': actionfldwidth, 'field': actionfldname}
//::                            _kdbit += actionfldwidth
//::                    #endfor
//::                else:
//::                    for actionfld in actionflddict:
//::                        actionfldname  = actionfld['p4_name']
//::                        asmfldname  = actionfld['asm_name']
//::                        actionfldwidth = actionfld['len']
//::                        dvec_start = actionfld['dvec_start']
//::                        fle_start = 511-dvec_start
//::                        little_str = ''
//::                        if actionname in pddict['tables'][table]['le_action_params'].keys():
//::                            if actionfldname in pddict['tables'][table]['le_action_params'][actionname]:
//::                                little_str = ' (little)'
//::                            #endif
//::                        #endif
    ${asmfldname} : ${actionfldwidth} ${little_str}; /* BE[${dvec_start}], FLE[${fle_start}] */
//::                        kd_json[_kdbit] = {'bit': _kdbit, 'width': actionfldwidth, 'field': actionfldname}
//::                        _kdbit += actionfldwidth
//::                    #endfor
//::                #endif
//::                kd_size += totaladatabits
//::                if len(pddict['tables'][table]['actions']) > 1:
//::                    if not (pddict['tables'][table]['is_raw']):
//::                        pad_to_512 = 512 - (8 + kd_size)
//::                    else:
//::                        pad_to_512 = 512 - (kd_size)
//::                    #endif
//::                else:
//::                    pad_to_512 = 512 - (kd_size)
//::                #endif
//::                if (pad_to_512):
    __pad_to_512b : ${pad_to_512};
//::                    kd_json[_kdbit] = {'bit': _kdbit, 'width': pad_to_512, 'field': '__pad_to_512b'}
//::                    _kdbit += pad_to_512
//::                #endif
};
//::            #endif
//::        k_d_action_data_json['INGRESS_KD'][table][actionname] = kd_json
//::        #endfor

//::        add_kd_action_bits = False
//::        if len(pddict['tables'][table]['actions']) > 1:
struct ${table}_d {
//::            if not (pddict['tables'][table]['is_raw']):
    action_id : 8;
//::                add_kd_action_bits = True
//::                k_d_action_data_json['INGRESS_KD'][table][actionname][0] = {'bit': 0, 'width': 8, 'field': "__action_pc"}
//::            #endif
//::            empty_action = True
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionflddict, _) = action
//::                if len(actionflddict):
//::                    empty_action = False
//::                    break
//::                #endif
//::            #endfor
//::            if not empty_action:
    union {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionflddict, _) = action
//::                    if len(actionflddict):
//::                        if add_kd_action_bits:
//::                            k_d_action_data_json['INGRESS_KD'][table][actionname][0] = {'bit': 0, 'width': 8, 'field': "__action_pc"}
//::                        #endif
        struct ${table}_${actionname}_d  ${actionname}_d;
//::                    #endif
//::                #endfor
    } u;
//::            #endif
};
//::        elif len(pddict['tables'][table]['actions']) == 1:
//::            (actionname, actionflddict, _) = pddict['tables'][table]['actions'][0]
//::            if len(actionflddict):
struct ${table}_d {
    struct ${table}_${actionname}_d  ${actionname}_d;
};
//::            #endif
//::        #endif


//::     #endfor

//::     _context['KD_DICT'] = k_d_action_data_json


