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

global swkey
global swkey_mask
global actiondata

def docstring_parameter(*sub):
    def dec(obj):
        obj.__doc__ = obj.__doc__.format(*sub)
        return obj

    return dec

def populate_table(ctx):
        # for each table
//::    for table in pddict['tables']:
//::        table_type = pddict['tables'][table]['type']
    # table: ${table}, type: ${table_type}
    if (ctx['table_id'] == ${tabledict[table]}):
            # if table type != Index
//::        if pddict['tables'][table]['type'] != 'Index':
//::            i = 1
        swkey = ${table}_swkey_t()
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
            swkey.${table}_u${i}.${p4fldname} = int(values[0])

//::                        else:

        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(swkey.${table}_u${i}.${p4fldname}, i, int(values[i]))
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
            swkey.${table}_hdr_u${i}.${p4fldname} = int(values[0])

//::                        else:

        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(swkey.${table}_hdr_u${i}.${p4fldname}, i, int(values[i]))
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
            swkey.${p4fldname} = int(values[0])

//::                else:

        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(swkey.${p4fldname}, i, int(values[i]))
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

            swkey.${p4fldname} = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(swkey.${p4fldname}, i, int(values[i]))
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

            swkey.${p4fldname} = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_vld'] == true):
            values = ctx['${p4fldname}'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(swkey.${p4fldname}, i, int(values[i]))
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
        swkey_mask = ${table}_swkey_mask_t()
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

            swkey_mask.${table}_mask_u${i}.${p4fldname}_mask = int(values[0])

//::                        else:

        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, i, int(values[i]))

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

            swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask = int(values[0])

//::                        else:

        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask, i, int(values[i]))
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

            swkey_mask.${p4fldname}_mask = int(values[0])

//::                        else:

        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(swkey_mask.${p4fldname}_mask, i, int(values[i]))
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

            swkey_mask.${p4fldname}_mask = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(swkey_mask.${p4fldname}_mask, i, int(values[i]))
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

            swkey_mask.${p4fldname}_mask = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        if (ctx['${p4fldname}_mask_vld'] == true):
            values = ctx['${p4fldname}_mask'].split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(swkey_mask.${p4fldname}_mask, i, int(values[i]))
//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
                # endfor hdrunion_keys

//::        #endif
            # endif table type Ternary or OTcam

        actiondata = ${table}_actiondata()

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

            actiondata.${table}_action_u.${table}_${actionname}.${actionfldname} = int(values[0])
//::                    else:
        @docstring_parameter(str((((${actionfldwidth} + 7) / 8) - 1)))
        if (ctx['${actionfldname}_vld'] == true):
            values = ctx['${actionfldname}'].split()
            if len(values) == 0 or len(values) > ((${actionfldwidth} + 7) / 8):
                return

            for i in range(len(values)):
                ${pddict['cli-name']}.uint8_array_t_setitem(actiondata.${table}_action_u.${table}_${actionname}.${actionfldname}, i, int(values[i]))
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
            actiondata.actionid = ${table}.${table}_actiondata.${table}_actions_en.index(values[0])

//::    #endfor
        # endfor table

