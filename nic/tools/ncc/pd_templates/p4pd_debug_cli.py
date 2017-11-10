//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: #pdb.set_trace()
#!/usr/bin/python
# This file is auto-generated. Changes will be overwritten!
#
# ${pddict['cli-name']} DEBUG CLI
#

import glob
from cmd2 import Cmd
import ${pddict['cli-name']}
//::    tabledict = {}
//::    tableid = 1
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

array_cols = 16

p4pd_table_types_enum = [
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
        self.actiondata   = ${table}_actiondata()
        self.actiondata_p = self.actiondata.this

    def populate_table(ctx):

            # if table type != Index
//::        if pddict['tables'][table]['type'] != 'Index':
//::            i = 1
                # for each fldunion_keys
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):

        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) != 1:
                return
            self.swkey.${table}_u${i}.${p4fldname} = int(values[0])

//::                        else:

        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
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
                # for each hdrunion_keys
//::            for hdrunion in pddict['tables'][table]['hdrunion_keys']:
//::                (union_name, un_fields) = hdrunion
//::                if len(un_fields) > 1:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):

        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) != 1:
                return
            self.swkey.${table}_hdr_u${i}.${p4fldname} = int(values[0])

//::                        else:

        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey.${table}_hdr_u${i}.${p4fldname}, i, int(values[i]))
//::                        #endif
//::                    #endfor
//::                    i+=1
//::                #endif
//::            #endfor
                # endfor hdrunion_keys
                # foreach keys
//::            for fields in pddict['tables'][table]['keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if (p4fldwidth <= 32):

        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) != 1:
                return
            self.swkey.${p4fldname} = int(values[0])

//::                else:

        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
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
//::                        if (p4fldwidth <= 32):
        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) != 1:
                return

            self.swkey.${p4fldname} = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey.${p4fldname}, i, int(values[i]))
//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
                # endfor fldunion_keys
                # for each hdrunion_keys
//::            for fldunion in pddict['tables'][table]['hdrunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) != 1:
                return

            self.swkey.${p4fldname} = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey.${p4fldname}, i, int(values[i]))
//::                        #endif
//::                    #endfor
//::                #endif
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

//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) != 1:
                return

            self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask = int(values[0])

//::                        else:

        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, i, int(values[i]))

//::                        #endif
//::                    #endfor
//::                    i+=1
//::                #endif
//::            #endfor
                # endfor fldunion_keys
                # for each hdrunion_keys
//::            for hdrunion in pddict['tables'][table]['hdrunion_keys']:
//::                (union_name, un_fields) = hdrunion
//::                if len(un_fields) > 1:

//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) != 1:
                return

            self.swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask = int(values[0])

//::                        else:

        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask, i, int(values[i]))
//::                        #endif
//::                    #endfor
//::                    i+=1
//::                #endif
//::            #endfor
                # endfor hdrunion_keys

                # for each keys
//::            for fields in pddict['tables'][table]['keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if (p4fldwidth <= 32):
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) != 1:
                return

            self.swkey_mask.${p4fldname}_mask = int(values[0])

//::                        else:

        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey_mask.${p4fldname}_mask, i, int(values[i]))
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
//::                        if (p4fldwidth <= 32):
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) != 1:
                return

            self.swkey_mask.${p4fldname}_mask = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey_mask.${p4fldname}_mask, i, int(values[i]))
//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
                # endfor fldunion_keys
                # for each hdrunion_keys
//::            for fldunion in pddict['tables'][table]['hdrunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) != 1:
                return

            self.swkey_mask.${p4fldname}_mask = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.swkey_mask.${p4fldname}_mask, i, int(values[i]))
//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
                # endfor hdrunion_keys

//::        #endif
            # endif table type Ternary or OTcam

            # for each actions
//::        for action in pddict['tables'][table]['actions']:
//::            (actionname, actionfldlist) = action
//::            if len(actionfldlist):

                    # for each action field
//::                for actionfld in actionfldlist:
//::                    actionfldname, actionfldwidth = actionfld
//::                    if (actionfldwidth <= 32):

        if (ctx['${actionfldname}_vld'] == true):
            values = ctx['${actionfldname}'].split()
            if len(values) != 1:
                return

            self.actiondata.${table}_action_u.${table}_${actionname}.${actionfldname} = int(values[0])
//::                    else:
        @docstring_parameter(str((((${actionfldwidth} + 7) / 8) - 1)))
        if (ctx['${actionfldname}_vld'] == true):
            values = ctx['${actionfldname}'].split()
            if len(values) == 0 or len(values) > ((${actionfldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(self.actiondata.${table}_action_u.${table}_${actionname}.${actionfldname}, i, int(values[i]))
//::                    #endif
//::                #endfor
                    # endfor action fields
//::            #endif
//::        #endfor
            # endfor actions

//::        if len(pddict['tables'][table]['actions']):
//::            tbl = table.upper()
        ${table}_actions_en = [
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionfldlist) = action
//::                actname = actionname.upper()
            '${tbl}_${actname}_ID',
//::            #endfor
        ]
//::        #endif

        if (ctx['actionid_vld'] == true):
            values = ctx['action_vld'].split()
            if len(values) != 1:
                return
            self.actiondata.actionid = ${table}_actions_en.index(values[0])


    def create_entry(ctx):
        hwentry_p = ${pddict['cli-name']}.new_uint8_ptr_t()

        ret = ${pddict['cli-name']}.p4pd_entry_create(self.table_id, self.swkey_p, self.swkey_mask_p, self.actiondata_p, hwentry_p)

        if ret < 0:
            print('p4pd_entry_create() returned %d!' % (ret))

        print('Entry was created successfully')

        ${pddict['cli-name']}.delete_uint8_ptr_t(hwentryp)

    def write_entry(ctx):
        try:
            index = ctx['index']

//::        if pddict['tables'][table]['type'] != 'Index':

            hwkey_len_p = ${pddict['cli-name']}.new_uint32_ptr_t()
            hwkeymask_len_p = ${pddict['cli-name']}.new_uint32_ptr_t()
            hwactiondata_len_p = ${pddict['cli-name']}.new_uint32_ptr_t()

            ${pddict['cli-name']}.p4pd_hwentry_query(self.table_id, hwkey_len_p, hwkeymask_len_p, hwactiondata_len_p)

            hwkey_p = ${pddict['cli-name']}.malloc_uint8_t((${pddict['cli-name']}.uint32_ptr_t_value(hwkey_len_p)+7)/8)
            hwkeymask_p = ${pddict['cli-name']}.malloc_uint8_t((${pddict['cli-name']}.uint32_ptr_t_value(hwkeymask_len_p)+7)/8)

            ret = ${pddict['cli-name']}.p4pd_hwkey_hwmask_build(self.table_id, self.swkey_p, self.swkey_mask_p, hwkey_p, hwkeymask_p)
            if ret < 0:
                raise RuntimeError('p4pd_hwkey_hwmask_build() returned %d!' % (ret))

//::        else:
            hwkey_p = None
            hwkeymask_p = None
//::        #endif

            ret = ${pddict['cli-name']}.p4pd_entry_write(self.table_id, index, hwkey_p, hwkeymask_p, self.actiondata_p)

            if ret < 0:
                raise RuntimeError('p4pd_entry_write() returned %d!' % (ret))

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

    def modify_entry(ctx):

        index = ctx['index']

        ret = ${pddict['cli-name']}.p4pd_entry_read(self.table_id, index, self.swkey_p, self.swkey_mask_p, self.actiondata_p)

        if ret < 0:
            print('Error: p4pd_entry_read() returned %d!' % (ret))
            return;

        print('Entry was read successfully at index %d' % (index))

        try:
//::        if pddict['tables'][table]['type'] != 'Index':
            hwkey_len_p = ${pddict['cli-name']}.new_uint32_ptr_t()
            hwkeymask_len_p = ${pddict['cli-name']}.new_uint32_ptr_t()
            hwactiondata_len_p = ${pddict['cli-name']}.new_uint32_ptr_t()

            ${pddict['cli-name']}.p4pd_hwentry_query(self.table_id, hwkey_len_p, hwkeymask_len_p, hwactiondata_len_p)

            hwkey_p = ${pddict['cli-name']}.malloc_uint8_t((${pddict['cli-name']}.uint32_ptr_t_value(hwkey_len_p)+7)/8)
            hwkeymask_p = ${pddict['cli-name']}.malloc_uint8_t((${pddict['cli-name']}.uint32_ptr_t_value(hwkeymask_len_p)+7)/8)

            ret = ${pddict['cli-name']}.p4pd_hwkey_hwmask_build(self.table_id, self.swkey_p, self.swkey_mask_p, hwkey_p, hwkeymask_p)

            if ret < 0:
                raise RuntimeError('p4pd_hwkey_hwmask_build() returned %d!' % (ret))

//::        else:
            hwkey_p = None
            hwkeymask_p = None
//::        #endif

            ret = ${pddict['cli-name']}.p4pd_entry_write(self.table_id, index, hwkey_p, hwkeymask_p, self.actiondata_p)

            if ret < 0:
                raise RuntimeError('p4pd_entry_write() returned %d!' % (ret))

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

    def read_entry(ctx):

        index = ctx['index']

        ret = ${pddict['cli-name']}.p4pd_entry_read(self.table_id, index, self.swkey_p, self.swkey_mask_p, self.actiondata_p)

        if ret < 0:
            print('Error: p4pd_entry_read() returned %d!' % (ret))
            return;

//::        if pddict['tables'][table]['type'] != 'Index':

        print('${table}_swkey_t:')

                # for each keys
//::            for fields in pddict['tables'][table]['keys']:
//::            (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if (p4fldwidth <= 8):
        print('  ${p4fldname}: %d (0x%02x)' % (swkey.${p4fldname}, swkey.${p4fldname}))
//::                elif (p4fldwidth <= 16):
        print('  ${p4fldname}: %d (0x%04x)' % (swkey.${p4fldname}, swkey.${p4fldname}))
//::                elif (p4fldwidth <= 32):
        print('  ${p4fldname}: %d (0x%08x)' % (swkey.${p4fldname}, swkey.${p4fldname}))
//::                else:
        valstr = '  ${p4fldname}[]: '
        lenstr = len(valstr)
        for i in range((${p4fldwidth} + 7) / 8):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(swkey.${p4fldname}, i))
        print(valstr)
//::                #endif
//::            #endfor
                # endfor keys

//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::            (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:

        print('  ${table}_union${i}_t:')
//::                    for fields in un_fields:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 8):
        print('    ${p4fldname}: %d (0x%02x)' % (swkey.${table}_u${i}.${p4fldname}, swkey.${table}_u${i}.${p4fldname}))
//::                        elif (p4fldwidth <= 16):
        print('    ${p4fldname}: %d (0x%04x)' % (swkey.${table}_u${i}.${p4fldname}, swkey.${table}_u${i}.${p4fldname}))
//::                        elif (p4fldwidth <= 32):
        print('    ${p4fldname}: %d (0x%08x)' % (swkey.${table}_u${i}.${p4fldname}, swkey.${table}_u${i}.${p4fldname}))
//::                        else:
        valstr = '  ${p4fldname}[]: '
        lenstr = len(valstr)
        for i in range((${p4fldwidth} + 7) / 8):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(swkey.${table}_u${i}.${p4fldname}, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 8):
        print('  ${p4fldname}: %d (0x%02x)' % (swkey.${p4fldname}, swkey.${p4fldname}))
//::                        elif (p4fldwidth <= 16):
        print('  ${p4fldname}: %d (0x%04x)' % (swkey.${p4fldname}, swkey.${p4fldname}))
//::                        elif (p4fldwidth <= 32):
        print('  ${p4fldname}: %d (0x%08x)' % (swkey.${p4fldname}, swkey.${p4fldname}))
//::                        else:
        valstr = '  ${p4fldname}[]: '
        lenstr = len(valstr)
        for i in range((${p4fldwidth} + 7) / 8):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(swkey.${p4fldname}, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
//::            for hdrunion in pddict['tables'][table]['hdrunion_keys']:
//::            (union_name, un_fields) = hdrunion
//::                if len(un_fields) > 1:

        print('  ${table}_hdr_union${i}_t:')
//::                    for fields in un_fields:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 8):
        print('    ${p4fldname}: %d (0x%02x)' % (swkey.${table}_hdr_union${i}_t.${p4fldname}, swkey.${table}_hdr_union${i}_t.${p4fldname}))
//::                        elif (p4fldwidth <= 16):
        print('    ${p4fldname}: %d (0x%04x)' % (swkey.${table}_hdr_union${i}_t.${p4fldname}, swkey.${table}_hdr_union${i}_t.${p4fldname}))
//::                        elif (p4fldwidth <= 32):
        print('    ${p4fldname}: %d (0x%08x)' % (swkey.${table}_hdr_union${i}_t.${p4fldname}, swkey.${table}_hdr_union${i}_t.${p4fldname}))
//::                        else:
        valstr = '  ${p4fldname}[]: '
        lenstr = len(valstr)
        for i in range((${p4fldwidth} + 7) / 8):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(swkey.${table}_hdr_union${i}_t.${p4fldname}, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 8):
        print('  ${p4fldname}: %d (0x%02x)' % (swkey.${p4fldname}, swkey.${p4fldname}))
//::                        elif (p4fldwidth <= 16):
        print('  ${p4fldname}: %d (0x%04x)' % (swkey.${p4fldname}, swkey.${p4fldname}))
//::                        elif (p4fldwidth <= 32):
        print('  ${p4fldname}: %d (0x%08x)' % (swkey.${p4fldname}, swkey.${p4fldname}))
//::                        else:
        valstr = '  ${p4fldname}[]: '
        lenstr = len(valstr)
        for i in range((${p4fldwidth} + 7) / 8):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(swkey.${p4fldname}, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
//::        #endif
//::        if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':

        print('${table}_swkey_mask_t:')
//::            for fields in pddict['tables'][table]['keys']:
//::            (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if (p4fldwidth <= 8):
        print('  ${p4fldname}_mask: %d (0x%02x)' % (swkey_mask.${p4fldname}_mask, swkey_mask.${p4fldname}_mask))
//::                elif (p4fldwidth <= 16):
        print('  ${p4fldname}_mask: %d (0x%04x)' % (swkey_mask.${p4fldname}_mask, swkey_mask.${p4fldname}_mask))
//::                elif (p4fldwidth <= 32):
        print('  ${p4fldname}_mask: %d (0x%08x)' % (swkey_mask.${p4fldname}_mask, swkey_mask.${p4fldname}_mask))
//::                else:
        valstr = '  ${p4fldname}_mask[]: '
        lenstr = len(valstr)
        for i in range((${p4fldwidth} + 7) / 8):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(swkey_mask.${p4fldname}_mask, i))
        print(valstr)

//::                #endif
//::            #endfor
//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::            (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:

        print('  ${table}_union${i}_t:')
//::                    for fields in un_fields:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 8):
        print('    ${p4fldname}_mask: %d (0x%02x)' % (swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, swkey_mask.${table}_mask_u${i}.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 16):
        print('    ${p4fldname}_mask: %d (0x%04x)' % (swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, swkey_mask.${table}_mask_u${i}.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 32):
        print('    ${p4fldname}_mask: %d (0x%08x)' % (swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, swkey_mask.${table}_mask_u${i}.${p4fldname}_mask))
//::                        else:
        valstr = '  ${p4fldname}_mask[]: '
        lenstr = len(valstr)
        for i in range((${p4fldwidth} + 7) / 8):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                    i += 1
//::                else:
//::                    for fields in un_fields:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 8):
        print('  ${p4fldname}_mask: %d (0x%02x)' % (swkey_mask.${p4fldname}_mask, swkey_mask.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 16):
        print('  ${p4fldname}_mask: %d (0x%04x)' % (swkey_mask.${p4fldname}_mask, swkey_mask.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 32):
        print('  ${p4fldname}_mask: %d (0x%08x)' % (swkey_mask.${p4fldname}_mask, swkey_mask.${p4fldname}_mask))
//::                        else:
        valstr = '  ${p4fldname}_mask[]: '
        lenstr = len(valstr)
        for i in range((${p4fldwidth} + 7) / 8):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(swkey_mask.${p4fldname}_mask, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
//::            for fldunion in pddict['tables'][table]['hdrunion_keys']:
//::            (unionname, un_fields) = fldunion
//::                if len(un_fields) > 1:

        print('  ${table}_hdr_union${i}_t:')
//::                    for fields in un_fields:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 8):
        print('    ${p4fldname}_mask: %d (0x%02x)' % (swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask, swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 16):
        print('    ${p4fldname}_mask: %d (0x%04x)' % (swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask, swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 32):
        print('    ${p4fldname}_mask: %d (0x%08x)' % (swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask, swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask))
//::                        else:
        valstr = '    ${p4fldname}_mask[]: '
        lenstr = len(valstr)
        for i in range((${p4fldwidth} + 7) / 8):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                    i += 1
//::                else:
//::                    for fields in un_fields:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 8):
        print('  ${p4fldname}_mask: %d (0x%02x)' % (swkey_mask.${p4fldname}_mask, swkey_mask.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 16):
        print('  ${p4fldname}_mask: %d (0x%04x)' % (swkey_mask.${p4fldname}_mask, swkey_mask.${p4fldname}_mask))
//::                        elif (p4fldwidth <= 32):
        print('  ${p4fldname}_mask: %d (0x%08x)' % (swkey_mask.${p4fldname}_mask, swkey_mask.${p4fldname}_mask))
//::                        else:
        valstr = '  ${p4fldname}_mask[]: '
        lenstr = len(valstr)
        for i in range((${p4fldwidth} + 7) / 8):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(swkey_mask.${p4fldname}_mask, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
//::        #endif

        print('${table}_actiondata:')
        if actiondata.actionid < len(${table}.${table}_actiondata.${table}_actions_en):
            print('  actionid: %s (%d)' % (${table}.${table}_actiondata.${table}_actions_en[actiondata.actionid], actiondata.actionid))
        else:
            print('  actionid: UNKNOWN (%d)' % (actiondata.actionid))
        print('  ${table}_action_u:')
//::        for action in pddict['tables'][table]['actions']:
//::        (actionname, actionfldlist) = action
//::            if len(actionfldlist):
        print('    ${table}_${actionname}_t:')
//::                for actionfld in actionfldlist:
//::                    actionfldname, actionfldwidth = actionfld
//::                    if (actionfldwidth <= 8):
        print('      ${actionfldname}: %d (0x%02x)' % (actiondata.${table}_action_u.${table}_${actionname}.${actionfldname}, actiondata.${table}_action_u.${table}_${actionname}.${actionfldname}))
//::                    elif (actionfldwidth <= 16):
        print('      ${actionfldname}: %d (0x%04x)' % (actiondata.${table}_action_u.${table}_${actionname}.${actionfldname}, actiondata.${table}_action_u.${table}_${actionname}.${actionfldname}))
//::                    elif (actionfldwidth <= 32):
        print('      ${actionfldname}: %d (0x%08x)' % (actiondata.${table}_action_u.${table}_${actionname}.${actionfldname}, actiondata.${table}_action_u.${table}_${actionname}.${actionfldname}))
//::                    else:
        valstr =   '      ${actionfldname}[]: '
        lenstr = len(valstr)
        for i in range((${actionfldwidth} + 7) / 8):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += '0x%02x ' % (${pddict['cli-name']}.uint8_array_t_getitem(actiondata.${table}_action_u.${table}_${actionname}.${actionfldname}, i))
        print(valstr)

//::                    #endif
//::                #endfor
//::            #endif
//::        #endfor

//::    #endfor
        # endfor table

def populate_table(ctx):
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
        table.populate_table(ctx)
//::    #endfor
        # endfor table

