//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: #pdb.set_trace()
//::
//:: if pddict['p4plus']:
//::    api_prefix = 'p4pd_' + pddict['p4program']
//::    if pddict['p4plus_module'] == 'rxdma':
//::        start_table_base = 101
//::    elif pddict['p4plus_module'] == 'txdma':
//::        start_table_base = 201
//::    else:
//::        start_table_base = 301
//::    #endif
//:: else:
//::    api_prefix = 'p4pd'
//::	start_table_base = 1
//:: #endif
//:: cli_name_import = pddict['pipeline'] + "_lib" +  pddict['cli-name'] + "_p4pdcli"
#!/usr/bin/python
# This file is auto-generated. Changes will be overwritten!
#
# ${pddict['cli-name']} DEBUG CLI
#

import os
import json
import glob
from   ${cli_name_import} import *
import ${cli_name_import} as ${pddict['cli-name']}
//::    tabledict = {}
//::    tableid = start_table_base
//::    for table in pddict['tables']:
//::        if pddict['tables'][table]['type'] == 'Hash':
//::            tabledict[table] = tableid
//::            tableid += 1
//::        #endif
//::    #endfor
//::    for table in pddict['tables']:
//::        if pddict['tables'][table]['type'] == 'Hash_OTcam':
//::            tabledict[table] = tableid
//::            tableid += 1
//::        #endif
//::    #endfor
//::    for table in pddict['tables']:
//::        if pddict['tables'][table]['type'] == 'Ternary':
//::            tabledict[table] = tableid
//::            tableid += 1
//::        #endif
//::    #endfor
//::    for table in pddict['tables']:
//::        if pddict['tables'][table]['type'] == 'Index':
//::            tabledict[table] = tableid
//::            tableid += 1
//::        #endif
//::    #endfor
//::    for table in pddict['tables']:
//::        if pddict['tables'][table]['type'] == 'Mpu':
//::            tabledict[table] = tableid
//::            tableid += 1
//::        #endif
//::    #endfor

table_name_to_id_dict = {}
//::    for table in pddict['tables']:
table_name_to_id_dict['${table}'] = ${tabledict[table]}
//::    #endfor

array_cols = 16

${api_prefix}_table_types_enum = [
    'HASH',
    'HASH_OTCAM',
    'TERNARY',
    'INDEX',
    'MPU',
]

def docstring_parameter(*sub):
    def dec(obj):
        obj.__doc__ = obj.__doc__.format(*sub)
        return obj

    return dec

        # for each table
//::    for table in pddict['tables']:
//::        table_type = pddict['tables'][table]['type']

class ${table}():

    def __init__(self):
        self.table_name   = '${table}'
        self.table_id     = ${tabledict[table]}
        self.table_type   = '${table_type}'
        self.swkey_p      = None
        self.swkey_mask_p = None
        self.actiondata_p = None

//::        if pddict['tables'][table]['type'] != 'Index':
        self.swkey        = ${table}_swkey_t()
        self.swkey_p      = self.swkey.this
//::        #endif
//::        if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':
        self.swkey_mask   = ${table}_swkey_mask_t()
        self.swkey_mask_p = self.swkey_mask.this
//::        #endif
        self.actiondata   = ${table}_actiondata_t()
        self.actiondata_p = self.actiondata.this

//::        if len(pddict['tables'][table]['actions']):
//::            tbl = table.upper()
//::            var = 0
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionfldlist, _) = action
//::            if actionname.startswith("__"):
//::                continue
//::            #endif
//::                actname = actionname.upper()
        self.${tbl}_${actname}_ID = ${var}
//::                var = var + 1
//::            #endfor
//::        #endif

    def populate_table(self, ctx):

            # if table type != Index
//::        if pddict['tables'][table]['type'] != 'Index':
//::            i = 1
                # for each fldunion_keys
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
//::                    cnt = 0
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if p4fldname.startswith("__"):
//::                           continue
//::                        p4fldname = 'data_' + str(cnt) + '.' + p4fldname
//::                        cnt += 1
//::                        #endif
//::                        if (p4fldwidth <= 32):
        if ('${p4fldname}' in ctx['swkey']):
            values = ctx['swkey']['${p4fldname}'].split()
            if len(values) != 1:
                return
            self.swkey.${table}_u${i}.${p4fldname} = int(values[0], 0)

//::                        else:

        if ('${p4fldname}' in ctx['swkey']):
            values = ctx['swkey']['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ( int((${p4fldwidth} + 7) / 8)):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey.${table}_u${i}.${p4fldname}, i, int(values[i]))
//::                        #endif
                        # endfor un_fields
//::                    #endfor
//::                    i+=1
//::                #endif
//::            #endfor
                # endfor fldunion_keys
                # foreach keys
//::            for fields in pddict['tables'][table]['keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if p4fldname.startswith("__"):
//::                    continue
//::                #endif
//::                if (p4fldwidth <= 32):
        if ('${p4fldname}' in ctx['swkey']):
            values = ctx['swkey']['${p4fldname}'].split()
            if len(values) != 1:
                return
            self.swkey.${p4fldname} = int(values[0], 0)

//::                else:

        if ('${p4fldname}' in ctx['swkey']):
            values = ctx['swkey']['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ( int((${p4fldwidth} + 7) / 8)):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey.${p4fldname}, i, int(values[i]))
//::                #endif
//::            #endfor
                # endfor keys

//::            i = 1
                # for each fldunion_keys
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if p4fldname.startswith("__"):
//::                           continue
//::                        #endif
//::                        if (p4fldwidth <= 32):
        if ('${p4fldname}' in ctx['swkey']):
            values = ctx['swkey']['${p4fldname}'].split()
            if len(values) != 1:
                return

            self.swkey.${p4fldname} = int(values[0], 0)
//::                        else:
        if ('${p4fldname}' in ctx['swkey']):
            values = ctx['swkey']['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ( int((${p4fldwidth} + 7) / 8)):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey.${p4fldname}, i, int(values[i]))
//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
                # endfor fldunion_keys
                # for each hdrunion_keys
//::            for fields in pddict['tables'][table]['hdrunion_keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if p4fldname.startswith("__"):
//::                    continue
//::                #endif
//::                if (p4fldwidth <= 32):
        if ('${p4fldname}' in ctx['swkey']):
            values = ctx['swkey']['${p4fldname}'].split()
            if len(values) != 1:
                return

            self.swkey.${p4fldname} = int(values[0], 0)
//::                 else:
        if ('${p4fldname}' in ctx['swkey']):
            values = ctx['swkey']['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ( int((${p4fldwidth} + 7) / 8)):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey.${p4fldname}, i, int(values[i]))
//::                 #endif
//::            #endfor
                # endfor hdrunion_keys
//::        #endif
            # endif table type != Index

            # if table type is Ternary or OTcam
//::        if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':
//::            i = 1
                # for each fldunion_keys
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
//::                    cnt = 0
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if p4fldname.startswith("__"):
//::                           continue
//::                        #endif
//::                        p4fldname = 'data_' + str(cnt) + '.' + p4fldname
//::                        cnt += 1
//::                        if (p4fldwidth <= 32):
        if ('${p4fldname}_mask' in ctx['swkey_mask']):
            values = ctx['swkey_mask']['${p4fldname}'].split()
            if len(values) != 1:
                return

            self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask = int(values[0], 0)

//::                        else:
        if ('${p4fldname}_mask' in ctx['swkey_mask']):
            values = ctx['swkey_mask']['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ( int((${p4fldwidth} + 7) / 8)):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, i, int(values[i], 0))

//::                        #endif
//::                    #endfor
//::                    i+=1
//::                #endif
//::            #endfor
                # endfor fldunion_keys

                # for each keys
//::            for fields in pddict['tables'][table]['keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if p4fldname.startswith("__"):
//::                    continue
//::                #endif
//::                if (p4fldwidth <= 32):
        if ('${p4fldname}_mask' in ctx['swkey_mask']):
            values = ctx['swkey_mask']['${p4fldname}'].split()
            if len(values) != 1:
                return

            self.swkey_mask.${p4fldname}_mask = int(values[0], 0)

//::                        else:
        if ('${p4fldname}_mask' in ctx['swkey_mask']):
            values = ctx['swkey_mask']['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ( int((${p4fldwidth} + 7) / 8)):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey_mask.${p4fldname}_mask, i, int(values[i],0))
//::                #endif
//::            #endfor
                # endfor keys

//::            i = 1
                # for each fldunion_keys
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if p4fldname.startswith("__"):
//::                           continue
//::                        #endif
//::                        if (p4fldwidth <= 32):
        if ('${p4fldname}_mask' in ctx['swkey_mask']):
            values = ctx['swkey_mask']['${p4fldname}'].split()
            if len(values) != 1:
                return

            self.swkey_mask.${p4fldname}_mask = int(values[0], 0)
//::                        else:
        if ('${p4fldname}_mask' in ctx['swkey_mask']):
            values = ctx['swkey_mask']['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ( int((${p4fldwidth} + 7) / 8)):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey_mask.${p4fldname}_mask, i, int(values[i], 0))
//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
                # endfor fldunion_keys
                # for each hdrunion_keys
//::            for fields in pddict['tables'][table]['hdrunion_keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if p4fldname.startswith("__"):
//::                    continue
//::                #endif
//::                if (p4fldwidth <= 32):
        if ('${p4fldname}_mask' in ctx['swkey_mask']):
            values = ctx['swkey_mask']['${p4fldname}'].split()
            if len(values) != 1:
                return

            self.swkey_mask.${p4fldname}_mask = int(values[0], 0)
//::                else:
        if ('${p4fldname}_mask' in ctx['swkey_mask']):
            values = ctx['swkey_mask']['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ( int((${p4fldwidth} + 7) / 8)):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey_mask.${p4fldname}_mask, i, int(values[i], 0))
//::                #endif
//::            #endfor
                # endfor hdrunion_keys
//::        #endif
            # endif table type Ternary or OTcam

            # for each actions
//::        for action in pddict['tables'][table]['actions']:
//::            (actionname, actionfldlist, _) = action
//::            if actionname.startswith("__"):
//::                continue
//::            #endif
//::            action_name_upper = actionname.upper()
//::            table_upper = table.upper()
        if ctx['action_name'] == '${actionname}':
            self.actiondata.action_id = self.${table_upper}_${action_name_upper}_ID

//::            if len(actionfldlist):
                    # for each action field
//::                for actionfld in actionfldlist:
//::                    actionfldname = actionfld['p4_name']
//::                    actionfldwidth = actionfld['len']
//::                    if actionfldname.startswith("__"):
//::                        continue
//::                    #endif
//::                    if (actionfldwidth <= 32):
            if '${actionfldname}' in ctx['actionfld']:
                values = ctx['actionfld']['${actionfldname}'].split()
                if len(values) != 1:
                    return

                self.actiondata.action_u.${table}_${actionname}.${actionfldname} = int(values[0],0)
//::                    else:
            if '${actionfldname}' in ctx['actionfld']:
                values = ctx['actionfld']['${actionfldname}'].split()
                if len(values) == 0 or len(values) > int((${actionfldwidth} + 7) / 8):
                    return

                for i in range(len(values)):
                    ${pddict['cli-name']}.uint8_array_t_setitem(self.actiondata.action_u.${table}_${actionname}.${actionfldname}, i, int(values[i], 0))
//::                    #endif
//::                #endfor
                    # endfor action fields
//::            #endif
//::        #endfor
            # endfor actions


    def create_entry(self, ctx):

        self.populate_table(ctx)

        hwentry_p = ${pddict['cli-name']}.new_uint8_ptr_t()

        ret = ${pddict['cli-name']}.${api_prefix}_entry_create(self.table_id, self.swkey_p, self.swkey_mask_p, self.actiondata_p, hwentry_p)

        if ret < 0:
            print('${api_prefix}_entry_create() returned %d!' % (ret))

        print('Entry was created successfully')

        ${pddict['cli-name']}.delete_uint8_ptr_t(hwentryp)

    def write_entry(self, ctx):
        try:
            self.populate_table(ctx)

            index = ctx['index']

//::        if pddict['tables'][table]['type'] != 'Index':

            hwkey_len_p = ${pddict['cli-name']}.new_uint32_ptr_t()
            hwkeymask_len_p = ${pddict['cli-name']}.new_uint32_ptr_t()
            hwactiondata_len_p = ${pddict['cli-name']}.new_uint32_ptr_t()

            ${pddict['cli-name']}.${api_prefix}_hwentry_query(self.table_id, hwkey_len_p, hwkeymask_len_p, hwactiondata_len_p)

            hwkey_p = ${pddict['cli-name']}.malloc_uint8_t((${pddict['cli-name']}.uint32_ptr_t_value(hwkey_len_p)+7)/8)
            hwkeymask_p = ${pddict['cli-name']}.malloc_uint8_t((${pddict['cli-name']}.uint32_ptr_t_value(hwkeymask_len_p)+7)/8)

            ret = ${pddict['cli-name']}.${api_prefix}_hwkey_hwmask_build(self.table_id, self.swkey_p, self.swkey_mask_p, hwkey_p, hwkeymask_p)
            if ret < 0:
                raise RuntimeError('${api_prefix}_hwkey_hwmask_build() returned %d!' % (ret))

//::        else:
            hwkey_p = None
            hwkeymask_p = None
//::        #endif

            ret = ${pddict['cli-name']}.${api_prefix}_entry_write(self.table_id, index, hwkey_p, hwkeymask_p, self.actiondata_p)

            if ret < 0:
                raise RuntimeError('${api_prefix}_entry_write() returned %d!' % (ret))

            print('Entry was written successfully at index %d' % (index))

        except Exception as error:
            print(repr(error))

        finally:

//::        if pddict['tables'][table]['type'] != 'Index':

            ${pddict['cli-name']}.free_uint8_t(hwkey_p)
            ${pddict['cli-name']}.free_uint8_t(hwkeymask_p)
            ${pddict['cli-name']}.delete_uint32_ptr_t(hwkey_len_p)
            ${pddict['cli-name']}.delete_uint32_ptr_t(hwkeymask_len_p)
            ${pddict['cli-name']}.delete_uint32_ptr_t(hwactiondata_len_p)

//::        else:
            pass
//::        #endif

    def modify_entry(self, ctx):

        index = ctx['index']
        rsp_msg = ${pddict['cli-name']}.allocate_debug_response_msg()
        size    = new_intp()

        ret = ${pddict['cli-name']}.${api_prefix}_entry_read(self.table_id, index, self.swkey_p, self.swkey_mask_p, self.actiondata_p, rsp_msg, size)
        if ret < 0:
            print('Error: ${api_prefix}_entry_read() returned %d!' % (ret))
            return;

        for i in range (0, intp_value(size)):
            ret = ${pddict['cli-name']}.${api_prefix}_entry_populate(self.table_id, self.swkey_p, self.swkey_mask_p, self.actiondata_p, rsp_msg, i)
            if ret < 0:
                print('Error: ${api_prefix}_entry_populate() returned %d!' % (ret))
                return;

        delete_intp(size)
        ${pddict['cli-name']}.free_debug_response_msg(rsp_msg)

        print('Entry was read successfully at index %d' % (index))

        self.populate_table(ctx)

        try:
//::        if pddict['tables'][table]['type'] != 'Index':
            #hwkey_len_p = ${pddict['cli-name']}.new_uint32_ptr_t()
            #hwkeymask_len_p = ${pddict['cli-name']}.new_uint32_ptr_t()
            #hwactiondata_len_p = ${pddict['cli-name']}.new_uint32_ptr_t()

            #${pddict['cli-name']}.${api_prefix}_hwentry_query(self.table_id, hwkey_len_p, hwkeymask_len_p, hwactiondata_len_p)

            #hwkey_p = ${pddict['cli-name']}.malloc_uint8_t((${pddict['cli-name']}.uint32_ptr_t_value(hwkey_len_p)+7)/8)
            #hwkeymask_p = ${pddict['cli-name']}.malloc_uint8_t((${pddict['cli-name']}.uint32_ptr_t_value(hwkeymask_len_p)+7)/8)

            #ret = ${pddict['cli-name']}.${api_prefix}_hwkey_hwmask_build(self.table_id, self.swkey_p, self.swkey_mask_p, hwkey_p, hwkeymask_p)

            #if ret < 0:
            #    raise RuntimeError('${api_prefix}_hwkey_hwmask_build() returned %d!' % (ret))

//::        else:
            #hwkey_p = None
            #hwkeymask_p = None
//::        #endif

            ret = ${pddict['cli-name']}.${api_prefix}_entry_write(self.table_id, index, self.swkey_p, self.swkey_mask_p, self.actiondata_p)

            if ret < 0:
                raise RuntimeError('${api_prefix}_entry_write() returned %d!' % (ret))

            print('Entry was written successfully at index %d' % (index))

        except Exception as error:
            print(repr(error))

        finally:
//::        if pddict['tables'][table]['type'] != 'Index':
            #${pddict['cli-name']}.free_uint8_t(hwkey_p)
            #${pddict['cli-name']}.free_uint8_t(hwkeymask_p)
            #${pddict['cli-name']}.delete_uint32_ptr_t(hwkey_len_p)
            #${pddict['cli-name']}.delete_uint32_ptr_t(hwkeymask_len_p)
            #${pddict['cli-name']}.delete_uint32_ptr_t(hwactiondata_len_p)
            pass
//::        else:
            pass
//::        #endif

    def read_entry(self, ctx):

        self.populate_table(ctx)

//::    if pddict['tables'][table]['is_raw']:
        ret = ${pddict['cli-name']}.${api_prefix}_raw_table_entry_read(self.table_id, ctx['actionid'], self.actiondata_p, ctx['hwaddr'])
        if ret < 0:
            print('Error: ${api_prefix}_raw_table_entry_read() returned %d!' % (ret))
            return;
        self.print_entry();
//::    else:
        index   = ctx['index']

        ret = ${pddict['cli-name']}.${api_prefix}_entry_read(self.table_id, index, self.swkey_p, self.swkey_mask_p, self.actiondata_p, None, None)
        if ret < 0:
            print('Error: ${api_prefix}_entry_read() returned %d!' % (ret))
            return;

//::    #endif

    def print_entry(self):
//::        if pddict['tables'][table]['type'] != 'Index':

        print('${table}_swkey_t:')

                # for each keys
//::            for fields in pddict['tables'][table]['keys']:
//::            (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if p4fldname.startswith("__"):
//::                    continue
//::                #endif
//::                if (p4fldwidth <= 8):
        print('  ${p4fldname}: %d (0x%02x)' % (self.swkey.${p4fldname}, self.swkey.${p4fldname}))
//::                elif (p4fldwidth <= 16):
        print('  ${p4fldname}: %d (0x%04x)' % (self.swkey.${p4fldname}, self.swkey.${p4fldname}))
//::                elif (p4fldwidth <= 32):
        print('  ${p4fldname}: %d (0x%08x)' % (self.swkey.${p4fldname}, self.swkey.${p4fldname}))
//::                elif (p4fldwidth <= 64):
        print('  ${p4fldname}: %d (0x%016x)' % (self.swkey.${p4fldname}, self.swkey.${p4fldname}))
//::                else:
        valstr = '  ${p4fldname}[]: '
        lenstr = len(valstr)
        for i in range( int((${p4fldwidth} + 7) / 8)):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(self.swkey.${p4fldname}, i))
        print(valstr)
//::                #endif
//::            #endfor
                # endfor keys

//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::            (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
//::                    cnt = 0
        print('  ${table}_union${i}_t:')
//::                    for fields in un_fields:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if p4fldname.startswith("__"):
//::                           continue
//::                        #endif
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        p4fldname = 'data_' + str(cnt) + '.' + p4fldname
//::                        cnt += 1
//::                        if (p4fldwidth <= 8):
        print('    ${p4fldname}: %d (0x%02x)' % (self.swkey.${table}_u${i}.${p4fldname}, self.swkey.${table}_u${i}.${p4fldname}))
//::                        elif (p4fldwidth <= 16):
        print('    ${p4fldname}: %d (0x%04x)' % (self.swkey.${table}_u${i}.${p4fldname}, self.swkey.${table}_u${i}.${p4fldname}))
//::                        elif (p4fldwidth <= 32):
        print('    ${p4fldname}: %d (0x%08x)' % (self.swkey.${table}_u${i}.${p4fldname}, self.swkey.${table}_u${i}.${p4fldname}))
//::                        elif (p4fldwidth <= 64):
        print('    ${p4fldname}: %d (0x%016x)' % (self.swkey.${table}_u${i}.${p4fldname}, self.swkey.${table}_u${i}.${p4fldname}))
//::                        else:
        valstr = '  ${p4fldname}[]: '
        lenstr = len(valstr)
        for i in range( int((${p4fldwidth} + 7) / 8)):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(self.swkey.${table}_u${i}.${p4fldname}, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if p4fldname.startswith("__"):
//::                           continue
//::                        #endif
//::                        if (p4fldwidth <= 8):
        print('  ${p4fldname}: %d (0x%02x)' % (self.swkey.${p4fldname}, self.swkey.${p4fldname}))
//::                        elif (p4fldwidth <= 16):
        print('  ${p4fldname}: %d (0x%04x)' % (self.swkey.${p4fldname}, self.swkey.${p4fldname}))
//::                        elif (p4fldwidth <= 32):
        print('  ${p4fldname}: %d (0x%08x)' % (self.swkey.${p4fldname}, self.swkey.${p4fldname}))
//::                        elif (p4fldwidth <= 64):
        print('  ${p4fldname}: %d (0x%016x)' % (self.swkey.${p4fldname}, self.swkey.${p4fldname}))
//::                        else:
        valstr = '  ${p4fldname}[]: '
        lenstr = len(valstr)
        for i in range( int((${p4fldwidth} + 7) / 8)):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(self.swkey.${p4fldname}, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
//::            for fields in pddict['tables'][table]['hdrunion_keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if p4fldname.startswith("__"):
//::                    continue
//::                #endif
//::                if (p4fldwidth <= 8):
        print('  ${p4fldname}: %d (0x%02x)' % (self.swkey.${p4fldname}, self.swkey.${p4fldname}))
//::                elif (p4fldwidth <= 16):
        print('  ${p4fldname}: %d (0x%04x)' % (self.swkey.${p4fldname}, self.swkey.${p4fldname}))
//::                elif (p4fldwidth <= 32):
        print('  ${p4fldname}: %d (0x%08x)' % (self.swkey.${p4fldname}, self.swkey.${p4fldname}))
//::                elif (p4fldwidth <= 64):
        print('  ${p4fldname}: %d (0x%016x)' % (self.swkey.${p4fldname}, self.swkey.${p4fldname}))
//::                else:
        valstr = '  ${p4fldname}[]: '
        lenstr = len(valstr)
        for i in range( int((${p4fldwidth} + 7) / 8)):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(self.swkey.${p4fldname}, i))
        print(valstr)

//::                #endif
//::            #endfor
//::        #endif
//::        if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':

        print('${table}_swkey_mask_t:')
//::            for fields in pddict['tables'][table]['keys']:
//::            (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if p4fldname.startswith("__"):
//::                    continue
//::                #endif
//::                if (p4fldwidth <= 8):
        print('  ${p4fldname}_mask: %d (0x%02x)' % (self.swkey_mask.${p4fldname}_mask, self.swkey_mask.${p4fldname}_mask))
//::                elif (p4fldwidth <= 16):
        print('  ${p4fldname}_mask: %d (0x%04x)' % (self.swkey_mask.${p4fldname}_mask, self.swkey_mask.${p4fldname}_mask))
//::                elif (p4fldwidth <= 32):
        print('  ${p4fldname}_mask: %d (0x%08x)' % (self.swkey_mask.${p4fldname}_mask, self.swkey_mask.${p4fldname}_mask))
//::                elif (p4fldwidth <= 64):
        print('  ${p4fldname}_mask: %d (0x%016x)' % (self.swkey_mask.${p4fldname}_mask, self.swkey_mask.${p4fldname}_mask))
//::                else:
        valstr = '  ${p4fldname}_mask[]: '
        lenstr = len(valstr)
        for i in range( int((${p4fldwidth} + 7) / 8)):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(self.swkey_mask.${p4fldname}_mask, i))
        print(valstr)

//::                #endif
//::            #endfor
//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::            (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:

        print('  ${table}_union_mask_${i}_t:')
//::                    cnt = 0
//::                    for fields in un_fields:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if p4fldname.startswith("__"):
//::                           continue
//::                        #endif
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        p4fldname = 'data_' + str(cnt) + '.' + p4fldname
//::                        cnt += 1
//::                        if (p4fldwidth <= 8):
        print('    ${p4fldname}_mask: %d (0x%02x)' % (self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 16):
        print('    ${p4fldname}_mask: %d (0x%04x)' % (self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 32):
        print('    ${p4fldname}_mask: %d (0x%08x)' % (self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 64):
        print('    ${p4fldname}_mask: %d (0x%016x)' % (self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask))
//::                        else:
        valstr = '  ${p4fldname}_mask[]: '
        lenstr = len(valstr)
        for i in range( int((${p4fldwidth} + 7) / 8)):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                    i += 1
//::                else:
//::                    for fields in un_fields:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if p4fldname.startswith("__"):
//::                           continue
//::                        #endif
//::                        if (p4fldwidth <= 8):
        print('  ${p4fldname}_mask: %d (0x%02x)' % (self.swkey_mask.${p4fldname}_mask, self.swkey_mask.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 16):
        print('  ${p4fldname}_mask: %d (0x%04x)' % (self.swkey_mask.${p4fldname}_mask, self.swkey_mask.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 32):
        print('  ${p4fldname}_mask: %d (0x%08x)' % (self.swkey_mask.${p4fldname}_mask, self.swkey_mask.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 64):
        print('  ${p4fldname}_mask: %d (0x%016x)' % (self.swkey_mask.${p4fldname}_mask, self.swkey_mask.${p4fldname}_mask))
//::                        else:
        valstr = '  ${p4fldname}_mask[]: '
        lenstr = len(valstr)
        for i in range( int((${p4fldwidth} + 7) / 8)):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(self.swkey_mask.${p4fldname}_mask, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
//::            for fields in pddict['tables'][table]['hdrunion_keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if p4fldname.startswith("__"):
//::                    continue
//::                #endif
//::                if (p4fldwidth <= 8):
        print('  ${p4fldname}_mask: %d (0x%02x)' % (self.swkey_mask.${p4fldname}_mask, self.swkey_mask.${p4fldname}_mask))
//::                elif (p4fldwidth <= 16):
        print('  ${p4fldname}_mask: %d (0x%04x)' % (self.swkey_mask.${p4fldname}_mask, self.swkey_mask.${p4fldname}_mask))
//::                elif (p4fldwidth <= 32):
        print('  ${p4fldname}_mask: %d (0x%08x)' % (self.swkey_mask.${p4fldname}_mask, self.swkey_mask.${p4fldname}_mask))
//::                elif (p4fldwidth <= 64):
        print('  ${p4fldname}_mask: %d (0x%016x)' % (self.swkey_mask.${p4fldname}_mask, self.swkey_mask.${p4fldname}_mask))
//::                else:
        valstr = '  ${p4fldname}_mask[]: '
        lenstr = len(valstr)
        for i in range( int((${p4fldwidth} + 7) / 8)):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(self.swkey_mask.${p4fldname}_mask, i))
        print(valstr)

//::                #endif
//::            #endfor
//::        #endif

        #print('${table}_actiondata:')
        #if self.actiondata.action_id < len(${table}.actiondata.${table}_actions_en):
        #    print('  actionid: %s (%d)' % (${table}.actiondata.${table}_actions_en[self.actiondata.action_id], self.actiondata.action_id))
        #else:
        #    print('  actionid: UNKNOWN (%d)' % (self.actiondata.action_id))
        print('  action_u:')
//::        table_upper = table.upper()
//::        for action in pddict['tables'][table]['actions']:
//::            (actionname, actionfldlist, _) = action
//::            actionname_upper = actionname.upper()
//::            if actionname.startswith("__"):
//::                continue
//::            #endif
        if self.actiondata.action_id == self.${table_upper}_${actionname_upper}_ID:
//::            if len(actionfldlist):
            print('    ${table}_${actionname}_t:')
//::                for actionfld in actionfldlist:
//::                    actionfldname = actionfld['p4_name']
//::                    actionfldwidth = actionfld['len']
//::                    if actionfldname.startswith("__"):
//::                        continue
//::                    #endif
//::                    if (actionfldwidth <= 8):
            print('      ${actionfldname}: %d (0x%02x)' % (self.actiondata.action_u.${table}_${actionname}.${actionfldname}, self.actiondata.action_u.${table}_${actionname}.${actionfldname}))
//::                    elif (actionfldwidth <= 16):
            print('      ${actionfldname}: %d (0x%04x)' % (self.actiondata.action_u.${table}_${actionname}.${actionfldname}, self.actiondata.action_u.${table}_${actionname}.${actionfldname}))
//::                    elif (actionfldwidth <= 32):
            print('      ${actionfldname}: %d (0x%08x)' % (self.actiondata.action_u.${table}_${actionname}.${actionfldname}, self.actiondata.action_u.${table}_${actionname}.${actionfldname}))
//::                    elif (actionfldwidth <= 64):
            print('      ${actionfldname}: %d (0x%016x)' % (self.actiondata.action_u.${table}_${actionname}.${actionfldname}, self.actiondata.action_u.${table}_${actionname}.${actionfldname}))
//::                    else:
            valstr =   '      ${actionfldname}[]: '
            lenstr = len(valstr)
            for i in range(int((${actionfldwidth} + 7) / 8)):
                if i != 0 and (i % array_cols) == 0:
                    valstr += ('\n' + (' ' * lenstr))
                valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(self.actiondata.${table}_action_u.${table}_${actionname}.${actionfldname}, i))
            print(valstr)

//::                    #endif
//::                #endfor
//::            else:
            print('    ${table}_${actionname}')
//::            #endif
//::        #endfor

//::    #endfor
        # endfor table

def populate_table(ctx):
    ctx['table_id'] = table_name_to_id_dict[ctx['table_name']]
        # for each table
//::    first_time = 1
//::    for table in pddict['tables']:
//::        table_type = pddict['tables'][table]['type']
//::        if (first_time == 1):
//::            if_condn = "if"
//::            first_time = 0
//::        else:
//::            if_condn = "elif"
//::        #endif

    # table: ${table}, type: ${table_type}
    ${if_condn} (ctx['table_id'] == ${tabledict[table]}):
        table = ${table}()

        if (ctx['opn'] == 'read'):
            table.read_entry(ctx)
        elif (ctx['opn'] == 'write'):
            table.modify_entry(ctx)

//::    #endfor
        # endfor table

def table_dump(out_json=False):
    tbl_dir = os.environ['HAL_CONFIG_PATH']
    with open(tbl_dir + "/${pddict['pipeline']}/p4_table_map.json") as data_file:
        data = json.load(data_file)
        if out_json:
            print("---")
            tables = data['tables']
            print(json.dumps(tables))
            return

        print("=================================================================================")
        print( "{:<30} {:<6} {:<10} {:<10} {:<5} {:<10} {:<7}".format("Table","TblId", "Type", "In/Egress", "Stage", "StageTblID", "Size"))
        print("=================================================================================")
        for table in data['tables']:
            print( "{:<30} {:<6} {:<10} {:<10} {:<5} {:<10} {:<7}".format(str(table['name']), table_name_to_id_dict[table['name']], str(table['match_type']), str(table['direction']),  table['stage'], table['stage_table_id'], table['num_entries']))


def populate_register(ctx):
    if (ctx['opn'] == 'read'):
        ${pddict['cli-name']}.${api_prefix}_register_entry_read(str(ctx['block_name']), str(ctx['reg_name']), str(ctx['file_name']))
    elif (ctx['opn'] == 'list'):
        ${pddict['cli-name']}.${api_prefix}_register_list(str(ctx['block_name']), str(ctx['reg_name']), str(ctx['file_name']))
