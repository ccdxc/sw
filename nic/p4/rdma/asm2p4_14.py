#!/usr/bin/python

import os
import re
import sys

NUM_STAGES = 8
NUM_TABLES = 4

PHV_GLOBAL_COMMON_T = 'phv_global_common_t'

#execute this script from sw/nic/asm/rdma directory

#prefix = 'resp_rx'
#header_files = ['resp_rx/include/resp_rx.h', 'resp_rx/include/resp_rx_args.h']
#root_asms = ['resp_rx_rqcb_process.s']

global_fields = [('lif', 11), ('qid', 24), ('qtype', 3), ('cb_addr', 25), ('pt_base_addr_page_id', 22), ('log_num_pt_entries', 5), ('pad', 22)]
in_params = [
    (
    'req_tx', 
    ['req_tx/include/req_tx.h', 'req_tx/include/req_tx_args.h'],
    ['req_tx_sqcb_process.s'],
    ['error_disable_qp', 'incr_lsn', 'immeth_vld']
    ),
    (
    'req_rx', 
    ['req_rx/include/req_rx.h', 'req_rx/include/req_rx_args.h'],
    ['req_rx_sqcb1_process.s'],
    ['_feedback', '_rsvd0', '_rsvd1', '_rsvd2', '_rsvd3', '_atomic_aeth', '_aeth', '_completion', '_atomic_ack', '_ack', '_read_resp', '_only', '_last', '_middle', '_first', '_error_disable_qp']
    ),
    (
    'resp_tx', 
    ['resp_tx/include/resp_tx.h', 'resp_tx/include/resp_tx_args.h'],
    ['resp_tx_rqcb_process.s'],
    ['error_disable_qp']
    ),
    (
    'resp_rx', 
    ['resp_rx/include/resp_rx.h', 'resp_rx/include/resp_rx_args.h'],
    ['resp_rx_rqcb_process.s'],
    ['_ud', '_ring_dbell', '_ack_req', '_completion', '_inv_rkey', '_immdt', '_atomic_cswap', '_atomic_fna', '_write', '_read_req', '_send', '_only', '_last', '_middle', '_first', '_error_disable_qp']
    )
    ]
    

def glean_hdrs_keys (ofp, fname, headers, keys):
    fp = open(fname, 'r')

    count = 0
    
    hn = ''
    for line in fp:
        line.rstrip()
        if not line:
            continue
    
        res = re.search('\s*(struct) (\s*)(\w*)(\s*)\{(.*)', line, re.M)
        if res:
            count += 1
            hn = res.group(3)
            if hn.endswith('_k_t'):
                key = True
                keys[hn] = dict()
            else:
                key = False
                headers[hn] = []
        elif count == 1:
            #inside struct/union/header_type

            if key == False:
                res = re.search('\s*(\w+)\s*:\s*(\d+)\s*', line, re.M)
                if res:
                    fn = res.group(1)
                    fs = res.group(2)
                    headers[hn].append((fn, fs))
            else:
                res = re.search('\s*struct \s*(\w+) \s*(\w+);', line, re.M)
                if res:
                    keys[hn][res.group(2)] = res.group(1)

            
            res = re.search('\s*}\s*', line, re.M)
            if res:
                count -= 1
                hn = ''

def glean_hdrs_from_file (ofp, fname, headers):
    fp = open(fname, 'r')

    count = 0
    
    #hn = ''

    for line in fp:
        line.rstrip()
        if not line:
            continue
    
        # see if it is a struct starting
        res = re.search('\s*(struct|union) (\s*)(\w+)(\s*)\{(.*)', line, re.M)
        if res:
            if count == 0:
                hn = res.group(3)
                headers[hn] = (None, None)
                field_list = []
                header_type = 'base'
                #print hn
            count += 1
            #print count
        

        # see if it is  x:y; syntax
        #res = re.search('\s*(\w+)\s*:\s*(\d+)\s*;', line, re.M)
        res = re.match('\s*(\w+)\s*:\s*(\d+)\s*;', line, re.M)
        if res:
            fn = res.group(1)
            fs = res.group(2)
            if fn == 'bytes':
                fn = 'pyld_bytes'
            field_list.append((fn,fs))
            #print fn, fs

        # see if it matches struct x y; syntax
        res = re.search('\s*struct \s*(\w+) \s*(\w+)\s*;', line, re.M)
        if res:
            field_list.append((res.group(1), res.group(2)))
            header_type = 'compound'
            #print res.group(1), res.group(2)
            
        # see if it matches union { syntax
        res = re.search('\s*(union|struct) \s*{(.*)', line, re.M)
        if res:
            header_type = 'compound'
            count += 1
            #print count
            
        # see if it matches } syntax
        #res = re.search('\s*}\s*', line, re.M)
        res = re.match('\s*}\s*', line, re.M)
        if res:
            count -= 1
            #print count
            if count == 0:
                headers[hn] = (header_type, field_list)
                #print hn, header_type, field_list
                hn = ''

def glean_hdrs (ofp, header_files, headers):
    for h in header_files:
        glean_hdrs_from_file(ofp, h, headers)

def gen_hdrs (ofp, headers):

    print >> ofp, '/**** header definitions ****/\n'
    for hn, (ht, flist) in headers.items():
        if len(flist) == 0:
            continue
    
        if ht != 'base':
            continue

        print >> ofp, 'header_type' + ' ' + hn + ' {'
        print >> ofp, '    fields {'

        for fn, fs in flist:
            print >> ofp, '        %-32s : %4s;' %(fn, fs)
        
        print >> ofp, '    }'
        print >> ofp, '}'
        print >> ofp

    print >> ofp

def get_to_s_hv (prefix, s, hn):
    hv = hn[:-2]
    if hv.startswith(prefix):
        hv = ''.join(hv.split(prefix+'_')[1:])
    if hv.startswith('to_stage_'):
        hv = ''.join(hv.split('to_stage_')[1:])
    hv = 'to_s'+str(s)+'_'+hv
    return hv

def get_s2s_hv (prefix, t, hn):
    hv = hn[:-2]
    if hv.startswith(prefix):
        hv = ''.join(hv.split(prefix+'_')[1:])
    hv = 't'+str(t)+'_s2s_'+hv
    return hv

def gen_hdr_decls (ofp, prefix, gbl_hdrs, to_s_hdrs, s2s_hdrs):
    print >> ofp, '/**** header unions and scratch ****/'
    print >> ofp

    print >> ofp, '/**** global header unions ****/'
    print >> ofp
    for hn in gbl_hdrs:
        hv = hn[:-2]
        hv_scr = hv+'_scr'
    
        print >> ofp, '@pragma pa_header_union ingress common_global'
        print >> ofp, 'metadata %s %s;' %(hn, hv)
        print >> ofp, '@pragma scratch_metadata'
        print >> ofp, 'metadata %s %s;' %(hn, hv_scr)
        print >> ofp

    print >> ofp, '/**** to stage header unions ****/'
    print >> ofp
    for s in range(NUM_STAGES):
        for hn in to_s_hdrs[s]:
            hv = get_to_s_hv(prefix, s, hn)
            hv_scr = hv+'_scr'
    
            print >> ofp, '@pragma pa_header_union ingress to_stage_%d' %(s)
            print >> ofp, 'metadata %s %s;' %(hn, hv)
            print >> ofp, '@pragma scratch_metadata'
            print >> ofp, 'metadata %s %s;' %(hn, hv_scr)
            print >> ofp

    print >> ofp, '/**** stage to stage header unions ****/'
    print >> ofp
    for t in range(NUM_TABLES):
        for hn in s2s_hdrs[t]:
            hv = get_s2s_hv(prefix, t, hn)
            hv_scr = hv+'_scr'
    
            print >> ofp, '@pragma pa_header_union ingress common_t%d_s2s' %(t)
            print >> ofp, 'metadata %s %s;' %(hn, hv)
            print >> ofp, '@pragma scratch_metadata'
            print >> ofp, 'metadata %s %s;' %(hn, hv_scr)
            print >> ofp

    print >> ofp

def gen_modify_stmts (ofp, hn, hv, hv_scr, headers, trailer_str=''):
    ht, flist = headers[hn]
    for (fn,fs) in flist:
        print >> ofp, '    modify_field(%s.%s, %s.%s);%s' %(hv_scr, fn, hv, fn, trailer_str)
    print >> ofp

def gen_gbl_modify_stmts (ofp, hn, headers, trailer_str=''):
    hv = hn[:-2]
    hv_scr = hv+'_scr'
    gen_modify_stmts(ofp, hn, hv, hv_scr, headers, trailer_str) 

def gen_to_s_modify_stmts (ofp, prefix, s, hn, headers, trailer_str=''):
    hv = get_to_s_hv(prefix, s, hn)
    hv_scr = hv+'_scr'
    gen_modify_stmts(ofp, hn, hv, hv_scr, headers, trailer_str) 

def gen_s2s_modify_stmts (ofp, prefix, t, hn, headers, trailer_str=''):
    hv = get_s2s_hv(prefix, t, hn)
    hv_scr = hv+'_scr'
    gen_modify_stmts(ofp, hn, hv, hv_scr, headers, trailer_str) 

def get_s_t_for_an (call_seq, an):
    t_list = []
    s_list = []

    for s in range(NUM_STAGES):
        for t in range(NUM_TABLES):
            for p in call_seq[s][t]:
                if p == an:
                    if s not in s_list:
                        s_list.append(s)
                    if t not in t_list:
                        t_list.append(t)

    return s_list, t_list

def get_s_t_for_an (call_seq, an):
    for s in range(NUM_STAGES):
        for t in range(NUM_TABLES):
            for p in call_seq[s][t]:
                if p == an:
                    return s, t
    return None, None

def gen_actions (ofp, prefix, asm_files, call_seq, headers, action_k):

    for f in asm_files:
        to_s_hn = None
        s2s_hn = None
        #s_list = None
        #t_list = None
        s = None
        t = None
        

        an = f.split('.s')[0]
        if not an.endswith('_process'):
            an = an + '_process'

        if an in action_k:
            #s_list, t_list = get_s_t_for_an(call_seq, an)
            s, t = get_s_t_for_an(call_seq, an)
            kn, to_s_hn = action_k[an]
            if kn in headers:
                ht, flist = headers[kn]
                for ft, fn in flist:
                    if fn == 'args':
                        s2s_hn = ft

        #if not len(s_list) or not len(t_list):
        if s is None or t is None:
            #print an
            continue

        print >> ofp, 'action %s () {' %(an)
        print >> ofp, '    // from ki global'
        print >> ofp, '    GENERATE_GLOBAL_K'
        print >> ofp

        print >> ofp, '    // to stage'
        #for s in s_list:
        if to_s_hn is not None:
            gen_to_s_modify_stmts(ofp, prefix, s, to_s_hn, headers)
        else:
            print >> ofp

        print >> ofp, '    // stage to stage'
        #for t in t_list:
        if s2s_hn is not None:
            gen_s2s_modify_stmts(ofp, prefix, t, s2s_hn, headers)
        else:
            print >> ofp

        print >> ofp, '}'
    print >> ofp

def glean_actions_k (ofp, prefix, asm_files, headers, action_k):
    for fname in asm_files:
        fname1 = fname
        if not fname.endswith('_process.s'):
            fname1 = fname.split('.s')[0]+'_process.s'

        fp = open(prefix+'/'+fname, 'r')
        kn = None
        to_s_str = None
        to_s_hn = None
        for line in fp:
            line.rstrip()
            if not line:
                continue
            res = re.search('\s*struct \s*(\w+) \s*k;', line, re.M)
            if res:
                #print >> ofp, '%s %s' %(fname, res.group(1))
                #action_k[fname.split('.s')[0]] = res.group(1)
                if kn is None:
                    kn = res.group(1)

            res = re.search('.*k\.to_stage\.([\w|\.]*)', line, re.M)
            if res:
                if to_s_str is None:
                    to_s_str = res.group(1)
    
        if to_s_str is not None:
            u_list = to_s_str.split('.')[:-1]

            ht, flist = headers[kn]
            hn = [ ft for (ft, fn) in flist if fn == 'to_stage']
            #print hn

            for u in u_list:
                ht, flist = headers[hn[0]]
                hn = [ft for (ft, fn) in flist if fn == u]

            to_s_hn = hn[0]

        action_k[fname1.split('.s')[0]] = (kn, to_s_hn)


def gen_call_graph (ofp, prefix, call_seq, root_asms):
    fnames = []
    fnames1 = []
    # initialize root_asm with stage 0, table 0
    for r in root_asms:
        fnames.append((prefix+'/'+r, 0, 0))
        fnames1.append(prefix+'/'+r)
        call_seq[0][0].append(r.split('.s')[0])

    for (f, s, t) in fnames:
        #print >> ofp, f, s, t 
        with open(f, 'r') as fp:

            for line in fp:
                line.rstrip()
                if not line:
                    continue

                _n = None
                tl = None

                res = re.search('\s*CAPRI_NEXT_TABLE(\d)_READ_PC\(\s*\w+,\s*\w+,\s*(\w+),\s*[\w|\.]+\)', line, re.M)
                if res:
                    #print >> ofp, 'match1'
                    _n = res.group(2)
                    tl = [int(res.group(1))]
                
                res = re.search('\s*CAPRI_NEXT_TABLE_I_READ_PC\(\s*\w+, \s*\w+,\s*\w+,\s*(\w+),\s*\w+\)', line, re.M)
                if res:
                    #print >> ofp, 'match2'
                    _n = res.group(1)
                    #tl = [0, 1]
                    tl = [0]
        
                if _n is not None:
                    nf = prefix+'/'+ _n +'.s'
                    if not os.path.exists(nf):
                        #print >> ofp, '!!! %s doesnt exist !!!' %(nf)
                        if nf.endswith('_process.s'):
                            nf = nf.split('_process.s')[0]+'.s'
                            if not os.path.exists(nf):
                                #print >> ofp, '!!! %s still doesnt exist !!!' %(nf)
                                continue

                    for t in tl:
                        entry = (nf, s+1, t)
                        #if entry not in fnames:
                        if nf not in fnames1:
                            if s < (NUM_STAGES-1) and t < NUM_TABLES:
                                fnames.append(entry)
                                fnames1.append(nf)
                                call_seq[s+1][t].append(_n)

def call_seq_init (ofp, call_seq):
    for s in range(NUM_STAGES):
        tbl_list = []
        for t in range(NUM_TABLES):
            tbl_list.append([])
        call_seq.append(tbl_list)

def call_seq_print (ofp, call_seq):
    for s in range(NUM_STAGES):
        print >> ofp, "stage: %d" %(s)
        for t in range(NUM_TABLES):
            print >> ofp, "    table: %d" %(t)
            for p in call_seq[s][t]:
                print >> ofp, "        %s" %(p)

def gen_asm_files (ofp, prefix, asm_files):
    for f in os.listdir(prefix):
        if f.endswith('.s'):
            asm_files.append(f)

def gen_macros(ofp, headers):
    
    print >> ofp, '/**** Macros ****/'
    print >> ofp, '''
#define GENERATE_GLOBAL_K \\'''
    gen_gbl_modify_stmts(ofp, PHV_GLOBAL_COMMON_T, headers, '\\')

def gen_tbl_decls (ofp, prefix, call_seq):
    
    print >> ofp, '/**** table declarations ****/\n' 
    d = prefix.split('_')[1]

    for s in range(NUM_STAGES):
        for t in range(NUM_TABLES):
            print >> ofp, "#define %s_table_s%d_t%d %s_s%d_t%d" %(d, s, t, prefix, s, t)
        print >> ofp

    print >> ofp

    for s in range(NUM_STAGES):
        for t in range(NUM_TABLES):
            if len(call_seq[s][t]) == 0:
                continue
            pc = 0
            for p in call_seq[s][t]:
                action_str = 'action'
                if pc:
                    action_str += str(pc) 
                print >> ofp, '#define %s_table_s%d_t%d_%s %s' %(d, s, t, action_str, p)
                pc += 1
            print >> ofp
    
    print >> ofp, '#include "../common-p4+/common_%sdma.p4"' %(d)
    print >> ofp

def glean_hdr_usage (ofp, call_seq, headers, action_k, gbl_hdrs, to_s_hdrs, s2s_hdrs):
    
    for s in range(NUM_STAGES):
        for t in range(NUM_TABLES):
            if len(call_seq[s][t]) == 0:
                continue
            for p in call_seq[s][t]:
                if not p in action_k:
                    continue
                kn, to_s_hn = action_k[p]

                if not kn in headers:
                    continue
                _, flist = headers[kn]
        
                if to_s_hn is not None and not to_s_hn in to_s_hdrs[s]:
                    to_s_hdrs[s].append(to_s_hn)

                for ft, fn in flist:
                    if fn == 'global' and not ft in gbl_hdrs:
                        gbl_hdrs.append(ft)
                    if fn == 'args' and not ft in s2s_hdrs[t]:
                        s2s_hdrs[t].append(ft)
        

def asm2p4_one_file (prefix, header_files, root_asms, flags):

    ofname = 'rdma_'+prefix+'dma.p4'
    ofp = open(ofname, 'w')

    print >> ofp, '''\
/***********************************************************************/
/* %s */
/***********************************************************************/''' %(ofname)
    print >> ofp
     

    gbl_hdrs = []
    to_s_hdrs = []
    for s in range(NUM_STAGES):
        to_s_hdrs.append([])
    
    s2s_hdrs = []
    for t in range(NUM_TABLES):
        s2s_hdrs.append([])

    asm_files = []
    gen_asm_files(ofp, prefix, asm_files)

    headers = dict()
    headers[PHV_GLOBAL_COMMON_T] = ('base', global_fields[:])
    
    for f in flags:
        headers[PHV_GLOBAL_COMMON_T][1].append((f, 1))
    
    glean_hdrs(ofp, header_files, headers)
    #print headers

    call_seq = []
    call_seq_init(ofp, call_seq)
    gen_call_graph(ofp, prefix, call_seq, root_asms)
    #call_seq_print(ofp, call_seq)

    action_k = dict()
    glean_actions_k(ofp, prefix, asm_files, headers, action_k)

    glean_hdr_usage(ofp, call_seq, headers, action_k, gbl_hdrs, to_s_hdrs, s2s_hdrs)

    gen_tbl_decls(ofp, prefix, call_seq)
    gen_macros(ofp, headers)
    gen_hdrs(ofp, headers)
    gen_hdr_decls(ofp, prefix, gbl_hdrs, to_s_hdrs, s2s_hdrs)
    gen_actions(ofp, prefix, asm_files, call_seq, headers, action_k)

def main ():

    for (p, h, r, f) in in_params:
        asm2p4_one_file(p, h, r, f)

if __name__ == '__main__':
    main()
    
