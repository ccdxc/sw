//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: #pdb.set_trace()
#!/usr/bin/python
# This file is auto-generated. Changes will be overwritten!
#
# p4pd CLI
#

import glob
from cmd2 import Cmd
import p4pd
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

//::    for table in pddict['tables']:
class ${table}(Cmd):

    prompt = "${table}> "
    intro = "Interact with the ${table} table."
    tableid = ${tabledict[table]}

    def __init__(self):
        Cmd.__init__(self)

    def do_quit(self, args):
        """Exits to the previous level"""
        return True

    do_EOF = do_quit

    do_exit = do_quit

    do_done = do_quit

//::        if pddict['tables'][table]['type'] != 'Index':
//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
    class ${table}_union${i}_t(Cmd):

        prompt = "${table}_union${i}_t> "
        intro = "Enter values for ${table}_union${i}_t fields, or type 'done to accept default."
        swkey = None

        def __init__(self, swkey):
            Cmd.__init__(self)
            self.swkey = swkey

        def do_quit(self, args):
            """Exits to the previous level"""
            return True

        do_EOF = do_quit

        do_exit = do_quit
        
        do_done = do_quit

//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
        def do_${p4fldname}(self, args):
            """Usage: ${p4fldname} <value>"""
            values = args.split()
            if len(values) != 1:
                print('Usage: ${p4fldname} <value>')
                return

            self.swkey.${table}_u${i}.${p4fldname} = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        def do_${p4fldname}(self, args):
            """Usage: ${p4fldname} <value0> [value1] [value2] ... [value{0}]"""
            values = args.split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                print('Usage: ${p4fldname} <value0> [value1] [value2] ... [value%d]' % (((${p4fldwidth} + 7) / 8) - 1))
                return

            for i in range(len(values)):
                p4pd.uint8_array_t_setitem(self.swkey.${table}_u${i}.${p4fldname}, i, int(values[i]))
//::                        #endif

//::                    #endfor
//::                    i+=1
//::                #endif
//::            #endfor
//::            for hdrunion in pddict['tables'][table]['hdrunion_keys']:
//::                (union_name, un_fields) = hdrunion
//::                if len(un_fields) > 1:
    class ${table}_hdr_union${i}_t(Cmd):

        prompt = "${table}_hdr_union${i}_t> "
        intro = "Enter values for ${table}_hdr_union${i}_t fields, or type 'done to accept default."
        swkey = None

        def __init__(self, swkey):
            Cmd.__init__(self)
            self.swkey = swkey

        def do_quit(self, args):
            """Exits to the previous level"""
            return True

        do_EOF = do_quit

        do_exit = do_quit
        
        do_done = do_quit

//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
        def do_${p4fldname}(self, args):
            """Usage: ${p4fldname} <value>"""
            values = args.split()
            if len(values) != 1:
                print('Usage: ${p4fldname} <value>')
                return

            self.swkey.${table}_hdr_u${i}.${p4fldname} = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        def do_${p4fldname}(self, args):
            """Usage: ${p4fldname} <value0> [value1] [value2] ... [value{0}]"""
            values = args.split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                print('Usage: ${p4fldname} <value0> [value1] [value2] ... [value%d]' % (((${p4fldwidth} + 7) / 8) - 1))
                return

            for i in range(len(values)):
                p4pd.uint8_array_t_setitem(self.swkey.${table}_hdr_u${i}.${p4fldname}, i, int(values[i]))

//::                        #endif
//::                    #endfor
//::                    i+=1
//::                #endif
//::            #endfor

    class ${table}_swkey_t(Cmd):

        prompt = "${table}_swkey_t> "
        intro = "Enter values for ${table}_swkey_t fields, or type 'done to accept default."
        swkey = None

        def __init__(self, swkey):
            Cmd.__init__(self)
            self.swkey = swkey

        def do_quit(self, args):
            """Exits to the previous level"""
            return True

        do_EOF = do_quit

        do_exit = do_quit
        
        do_done = do_quit

//::            for fields in pddict['tables'][table]['keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if (p4fldwidth <= 32):
        def do_${p4fldname}(self, args):
            """Usage: ${p4fldname} <value>"""
            values = args.split()
            if len(values) != 1:
                print('Usage: ${p4fldname} <value>')
                return

            self.swkey.${p4fldname} = int(values[0])
//::                else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        def do_${p4fldname}(self, args):
            """Usage: ${p4fldname} <value0> [value1] [value2] ... [value{0}]"""
            values = args.split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                print('Usage: ${p4fldname} <value0> [value1] [value2] ... [value%d]' % (((${p4fldwidth} + 7) / 8) - 1))
                return

            for i in range(len(values)):
                p4pd.uint8_array_t_setitem(self.swkey.${p4fldname}, i, int(values[i]))

//::                #endif

//::            #endfor
//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
        def do_${table}_u${i}(self, args):
            """Usage: ${table}_u${i}"""
            values = args.split()
            if len(values) != 0:
                print('Usage: ${table}_u${i}')
                return

            cmd = ${table}.${table}_union${i}_t(self.swkey)
            cmd.cmdloop()
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
        def do_${p4fldname}(self, args):
            """Usage: ${p4fldname} <value>"""
            values = args.split()
            if len(values) != 1:
                print('Usage: ${p4fldname} <value>')
                return

            self.swkey.${p4fldname} = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        def do_${p4fldname}(self, args):
            """Usage: ${p4fldname} <value0> [value1] [value2] ... [value{0}]"""
            values = args.split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                print('Usage: ${p4fldname} <value0> [value1] [value2] ... [value%d]' % (((${p4fldwidth} + 7) / 8) - 1))
                return

            for i in range(len(values)):
                p4pd.uint8_array_t_setitem(self.swkey.${p4fldname}, i, int(values[i]))

//::                        #endif

//::                    #endfor
//::                #endif
//::            #endfor

//::            for hdrunion in pddict['tables'][table]['hdrunion_keys']:
//::                (union_name, un_fields) = hdrunion
//::                if len(un_fields) > 1:
        def do_${table}_u${i}(self, args):
            """Usage: ${table}_hdr_u${i}"""
            values = args.split()
            if len(values) != 0:
                print('Usage:${table}_hdr_u${i}')
                return

            cmd = ${table}.${table}_hdr_union${i}_t(self.swkey)
            cmd.cmdloop()
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
        def do_${p4fldname}(self, args):
            """Usage: ${p4fldname} <value>"""
            values = args.split()
            if len(values) != 1:
                print('Usage: ${p4fldname} <value>')
                return

            self.swkey.${p4fldname} = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        def do_${p4fldname}(self, args):
            """Usage: ${p4fldname} <value0> [value1] [value2] ... [value{0}]"""
            values = args.split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                print('Usage: ${p4fldname} <value0> [value1] [value2] ... [value%d]' % (((${p4fldwidth} + 7) / 8) - 1))
                return

            for i in range(len(values)):
                p4pd.uint8_array_t_setitem(self.swkey.${p4fldname}, i, int(values[i]))

//::                        #endif

//::                    #endfor
//::                #endif
//::            #endfor
//::        #endif
//::        if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':
//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
    class ${table}_mask_union${i}_t(Cmd):

        prompt = "${table}_mask_union${i}_t> "
        intro = "Enter values for ${table}_mask_union${i}_t fields, or type 'done to accept default."
        swkey_mask = None

        def __init__(self, swkey_mask):
            Cmd.__init__(self)
            self.swkey_mask = swkey_mask

        def do_quit(self, args):
            """Exits to the previous level"""
            return True

        do_EOF = do_quit

        do_exit = do_quit
        
        do_done = do_quit

//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
        def do_${p4fldname}_mask(self, args):
            """Usage: ${p4fldname}_mask <value>"""
            values = args.split()
            if len(values) != 1:
                print('Usage: ${p4fldname}_mask <value>')
                return

            self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        def do_${p4fldname}_mask(self, args):
            """Usage: ${p4fldname}_mask <value0> [value1] [value2] ... [value{0}]"""
            values = args.split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                print('Usage: ${p4fldname}_mask <value0> [value1] [value2] ... [value%d]' % (((${p4fldwidth} + 7) / 8) - 1))
                return

            for i in range(len(values)):
                p4pd.uint8_array_t_setitem(self.swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, i, int(values[i]))

//::                        #endif

//::                    #endfor
//::                    i+=1
//::                #endif
//::            #endfor
//::            for hdrunion in pddict['tables'][table]['hdrunion_keys']:
//::                (union_name, un_fields) = hdrunion
//::                if len(un_fields) > 1:
    class ${table}_mask_hdr_union${i}_t(Cmd):

        prompt = "${table}_mask_hdr_union${i}_t> "
        intro = "Enter values for ${table}_mask_hdr_union${i}_t fields, or type 'done to accept default."
        swkey_mask = None

        def __init__(self, swkey_mask):
            Cmd.__init__(self)
            self.swkey_mask = swkey_mask

        def do_quit(self, args):
            """Exits to the previous level"""
            return True

        do_EOF = do_quit

        do_exit = do_quit
        
        do_done = do_quit

//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
        def do_ ${p4fldname}_mask(self, args):
            """Usage:  ${p4fldname}_mask <value>"""
            values = args.split()
            if len(values) != 1:
                print('Usage:  ${p4fldname}_mask <value>')
                return

            self.swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        def do_ ${p4fldname}_mask(self, args):
            """Usage: ${p4fldname}_mask <value0> [value1] [value2] ... [value{0}]"""
            values = args.split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                print('Usage: ${p4fldname}_mask <value0> [value1] [value2] ... [value%d]' % (((${p4fldwidth} + 7) / 8) - 1))
                return

            for i in range(len(values)):
                p4pd.uint8_array_t_setitem(self.swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask, i, int(values[i]))

//::                        #endif
//::                    #endfor
//::                    i+=1
//::                #endif
//::            #endfor

    class ${table}_swkey_mask_t(Cmd):

        prompt = "${table}_swkey_mask_t> "
        intro = "Enter values for ${table}_swkey_mask_t fields, or type 'done to accept default."
        swkey_mask = None

        def __init__(self, swkey_mask):
            Cmd.__init__(self)
            self.swkey_mask = swkey_mask

        def do_quit(self, args):
            """Exits to the previous level"""
            return True

        do_EOF = do_quit

        do_exit = do_quit
        
        do_done = do_quit

//::            for fields in pddict['tables'][table]['keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if (p4fldwidth <= 32):
        def do_${p4fldname}_mask(self, args):
            """Usage: ${p4fldname}_mask <value>"""
            values = args.split()
            if len(values) != 1:
                print('Usage: ${p4fldname}_mask <value>')
                return

            self.swkey_mask.${p4fldname}_mask = int(values[0])
//::                else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        def do_${p4fldname}_mask(self, args):
            """Usage: ${p4fldname}_mask <value0> [value1] [value2] ... [value{0}]"""
            values = args.split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                print('Usage: ${p4fldname}_mask <value0> [value1] [value2] ... [value%d]' % (((${p4fldwidth} + 7) / 8) - 1))
                return

            for i in range(len(values)):
                p4pd.uint8_array_t_setitem(self.swkey_mask.${p4fldname}_mask, i, int(values[i]))

//::                #endif

//::            #endfor
//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
        def do_${table}_mask_u${i}(self, args):
            """Usage: ${table}_mask_u${i}"""
            values = args.split()
            if len(values) != 0:
                print('Usage: ${table}_mask_u${i}')
                return

            cmd = ${table}.${table}_mask_union${i}_t(self.swkey_mask)
            cmd.cmdloop()
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
        def do_${p4fldname}_mask(self, args):
            """Usage: ${p4fldname}_mask <value>"""
            values = args.split()
            if len(values) != 1:
                print('Usage: ${p4fldname}_mask <value>')
                return

            self.swkey_mask.${p4fldname}_mask = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        def do_${p4fldname}_mask(self, args):
            """Usage: ${p4fldname}_mask <value0> [value1] [value2] ... [value{0}]"""
            values = args.split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                print('Usage: ${p4fldname}_mask <value0> [value1] [value2] ... [value%d]' % (((${p4fldwidth} + 7) / 8) - 1))
                return

            for i in range(len(values)):
                p4pd.uint8_array_t_setitem(self.swkey_mask.${p4fldname}_mask, i, int(values[i]))

//::                        #endif

//::                    #endfor
//::                #endif
//::            #endfor

//::            for hdrunion in pddict['tables'][table]['hdrunion_keys']:
//::                (union_name, un_fields) = hdrunion
//::                if len(un_fields) > 1:
        def do_${table}_mask_u${i}(self, args):
            """Usage: ${table}_mask_u${i}"""
            values = args.split()
            if len(values) != 0:
                print('Usage: ${table}_mask_u${i}')
                return

            cmd = ${table}.${table}_mask_hdr_u${i}(self.swkey_mask)
            cmd.cmdloop()
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
        def do_${p4fldname}_mask(self, args):
            """Usage: ${p4fldname}_mask <value>"""
            values = args.split()
            if len(values) != 1:
                print('Usage: ${p4fldname}_mask <value>')
                return

            self.swkey_mask.${p4fldname}_mask = int(values[0])
//::                        else:
        @docstring_parameter(str((((${p4fldwidth} + 7) / 8) - 1)))
        def do_${p4fldname}_mask(self, args):
            """Usage: ${p4fldname}_mask <value0> [value1] [value2] ... [value{0}]"""
            values = args.split()
            if len(values) == 0 or len(values) > ((${p4fldwidth} + 7) / 8):
                print('Usage: ${p4fldname}_mask <value0> [value1] [value2] ... [value%d]' % (((${p4fldwidth} + 7) / 8) - 1))
                return

            for i in range(len(values)):
                p4pd.uint8_array_t_setitem(self.swkey_mask.$${p4fldname}_mask, i, int(values[i]))

//::                        #endif

//::                    #endfor
//::                #endif
//::            #endfor
//::        #endif
//::        for action in pddict['tables'][table]['actions']:
//::            (actionname, actionfldlist) = action
//::            if len(actionfldlist):
    class ${table}_${actionname}_t(Cmd):

        prompt = "${table}_${actionname}_t> "
        intro = "Enter values for ${table}_${actionname}_t fields, or type 'done to accept default."
        actiondata = None

        def __init__(self, actiondata):
            Cmd.__init__(self)
            self.actiondata = actiondata

        def do_quit(self, args):
            """Exits to the previous level"""
            return True

        do_EOF = do_quit

        do_exit = do_quit
        
        do_done = do_quit

//::                for actionfld in actionfldlist:
//::                    actionfldname, actionfldwidth = actionfld
//::                    if (actionfldwidth <= 32):
        def do_${actionfldname}(self, args):
            """Usage: ${actionfldname} <value>"""
            values = args.split()
            if len(values) != 1:
                print('Usage: ${actionfldname} <value>')
                return

            self.actiondata.${table}_action_u.${table}_${actionname}.${actionfldname} = int(values[0])
//::                    else:
        @docstring_parameter(str((((${actionfldwidth} + 7) / 8) - 1)))
        def do_${actionfldname}(self, args):
            """Usage: ${actionfldname} <value0> [value1] [value2] ... [value{0}]"""
            values = args.split()
            if len(values) == 0 or len(values) > ((${actionfldwidth} + 7) / 8):
                print('Usage: ${actionfldname} <value0> [value1] [value2] ... [value%d]' % (((${actionfldwidth} + 7) / 8) - 1))
                return

            for i in range(len(values)):
                p4pd.uint8_array_t_setitem(self.actiondata.${table}_action_u.${table}_${actionname}.${actionfldname}, i, int(values[i]))

//::                    #endif

//::                #endfor
//::            #endif
//::        #endfor

    class ${table}_action_union_t(Cmd):

        prompt = "${table}_action_union_t> "
        intro = "Enter values for ${table}_action_union_t fields, or type 'done to accept default."
        actiondata = None

        def __init__(self, actiondata):
            Cmd.__init__(self)
            self.actiondata = actiondata

        def do_quit(self, args):
            """Exits to the previous level"""
            return True

        do_EOF = do_quit

        do_exit = do_quit
        
        do_done = do_quit

//::        for action in pddict['tables'][table]['actions']:
//::            (actionname, actionfldlist) = action
//::            if len(actionfldlist):
        def do_${table}_${actionname}(self, args):
            """Usage: ${table}_${actionname}"""
            values = args.split()
            if len(values) != 0:
                print('Usage: ${table}_${actionname}')
                return

            cmd = ${table}.${table}_${actionname}_t(self.actiondata)
            cmd.cmdloop()

//::            #endif
//::        #endfor
    class ${table}_actiondata(Cmd):

        prompt = "${table}_actiondata> "
        intro = "Enter values for ${table}_actiondata fields, or type 'done to accept default."
        actiondata = None

        def __init__(self, actiondata):
            Cmd.__init__(self)
            self.actiondata = actiondata

        def do_quit(self, args):
            """Exits to the previous level"""
            return True

        do_EOF = do_quit

        do_exit = do_quit
        
        do_done = do_quit

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

        def do_actionid(self, args):
            """Usage: actionid <value>"""
            values = args.split()
            if len(values) != 1:
                print('Usage: actionid <value>')
                return

            self.actiondata.actionid = ${table}.${table}_actiondata.${table}_actions_en.index(values[0])

        def complete_actionid(self, text, line, start_index, end_index):
            if text:
                return [
                    enum for enum in ${table}.${table}_actiondata.${table}_actions_en
                    if enum.startswith(text)
                ]
            else:
                return ${table}.${table}_actiondata.${table}_actions_en

        def do_${table}_action_u(self, args):
            """Usage: ${table}_action_u"""

            values = args.split()
            if len(values) != 0:
                print('Usage: ${table}_action_u')
                return

            cmd = ${table}.${table}_action_union_t(self.actiondata)
            cmd.cmdloop()

    class ${table}_prompt_for_values(Cmd):

        prompt = "${table}_entry> "
        intro = "Enter values for the ${table} entry fields, or type 'done to accept default."
//::        if pddict['tables'][table]['type'] != 'Index':
//::            if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':
        swkey = None
        swkey_mask = None
        actiondata = None

        def __init__(self, swkey, swkey_mask, actiondata):
            Cmd.__init__(self)
            self.swkey = swkey
            self.swkey_mask = swkey_mask
            self.actiondata = actiondata

        def do_${table}_swkey_t(self, args):
            """Usage: ${table}_swkey_t"""
            cmd = ${table}.${table}_swkey_t(self.swkey)
            cmd.cmdloop()

        def do_${table}_swkey_mask_t(self, args):
            """Usage: ${table}_swkey_mask_t"""
            cmd = ${table}.${table}_swkey_mask_t(self.swkey_mask)
            cmd.cmdloop()
//::            else:
        swkey = None
        actiondata = None

        def __init__(self, swkey, actiondata):
            Cmd.__init__(self)
            self.swkey = swkey
            self.actiondata = actiondata

        def do_${table}_swkey_t(self, args):
            """Usage: ${table}_swkey_t"""
            cmd = ${table}.${table}_swkey_t(self.swkey)
            cmd.cmdloop()
//::            #endif
//::        else:
        actiondata = None

        def __init__(self, actiondata):
            Cmd.__init__(self)
            self.actiondata = actiondata
//::        #endif

        def do_${table}_actiondata(self, args):
            """Usage: ${table}_actiondata"""
            cmd = ${table}.${table}_actiondata(self.actiondata)
            cmd.cmdloop()

        def do_quit(self, args):
            """Exits to the previous level"""
            return True

        do_EOF = do_quit

        do_exit = do_quit
        
        do_done = do_quit

    def do_get_properties(self, args):
        """Usage: get_properties"""

        values = args.split()
        if len(values) != 0:
            print('Usage: get_properties')
            return

        rootCmd.get_table_properties(self.tableid)

    def do_create_entry(self, args):
        """Usage: create_entry"""

        values = args.split()
        if len(values) != 0:
            print('Usage: create_entry')
            return

        hwentryp   = p4pd.new_uint8_ptr_t()
        actiondata = p4pd.${table}_actiondata()
//::        if pddict['tables'][table]['type'] != 'Index':
        swkey  = p4pd.${table}_swkey_t()
        swkeyp = swkey.this
//::            if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':
        swkey_mask  = p4pd.${table}_swkey_mask_t()
        swkey_maskp = swkey_mask.this

        cmd = self.${table}_prompt_for_values(swkey, swkey_mask, actiondata)
        cmd.cmdloop()
//::            else:
        swkey_maskp = None

        cmd = self.${table}_prompt_for_values(swkey, actiondata)
        cmd.cmdloop()
//::            #endif
//::        else:
        swkeyp = None
        swkey_maskp = None

        cmd = self.${table}_prompt_for_values(actiondata)
        cmd.cmdloop()
//::        #endif

        ret = p4pd.p4pd_entry_create(self.tableid, swkeyp, swkey_maskp, actiondata.this, hwentryp)

        if ret < 0:
            print('p4pd_entry_create() returned %d!' % (ret))

        print('Entry was created successfully')
        p4pd.delete_uint8_ptr_t(hwentryp)

    def do_write_entry(self, args):
        """Usage: write_entry <index>"""

        values = args.split()
        if len(values) != 1:
            print('Usage: write_entry <index>')
            return

        try:
            index = int(values[0])
            actiondata = p4pd.${table}_actiondata()
//::        if pddict['tables'][table]['type'] != 'Index':
            hwkey_len_p = p4pd.new_uint32_ptr_t()
            hwkeymask_len_p = p4pd.new_uint32_ptr_t()
            hwactiondata_len_p = p4pd.new_uint32_ptr_t()

            p4pd.p4pd_hwentry_query(self.tableid, hwkey_len_p, hwkeymask_len_p, hwactiondata_len_p)

            hwkey_p = p4pd.malloc_uint8_t((p4pd.uint32_ptr_t_value(hwkey_len_p)+7)/8)
            hwkeymask_p = p4pd.malloc_uint8_t((p4pd.uint32_ptr_t_value(hwkeymask_len_p)+7)/8)

            swkey  = p4pd.${table}_swkey_t()
            swkeyp = swkey.this
//::            if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':
            swkey_mask  = p4pd.${table}_swkey_mask_t()
            swkey_maskp = swkey_mask.this

            cmd = self.${table}_prompt_for_values(swkey, swkey_mask, actiondata)
            cmd.cmdloop()
//::            else:
            swkey_maskp = None

            cmd = self.${table}_prompt_for_values(swkey, actiondata)
            cmd.cmdloop()
//::            #endif

            ret = p4pd.p4pd_hwkey_hwmask_build(self.tableid, swkeyp, swkey_maskp, hwkey_p, hwkeymask_p)

            if ret < 0:
                raise RuntimeError('p4pd_hwkey_hwmask_build() returned %d!' % (ret))
//::        else:
            hwkey_p = None
            hwkeymask_p = None

            cmd = self.${table}_prompt_for_values(actiondata)
            cmd.cmdloop()
//::        #endif

            ret = p4pd.p4pd_entry_write(self.tableid, index, hwkey_p, hwkeymask_p, actiondata.this)

            if ret < 0:
                raise RuntimeError('p4pd_entry_write() returned %d!' % (ret))

            print('Entry was written successfully at index %d' % (index))

        except Exception as error:
            print(repr(error))

        finally:
//::        if pddict['tables'][table]['type'] != 'Index':
            p4pd.free_uint8_t(hwkey_p)
            p4pd.free_uint8_t(hwkeymask_p)
            p4pd.delete_uint32_ptr_t(hwkey_len_p)
            p4pd.delete_uint32_ptr_t(hwkeymask_len_p)
            p4pd.delete_uint32_ptr_t(hwactiondata_len_p)
//::        else:
            pass
//::        #endif

    def do_modify_entry(self, args):
        """Usage: modify_entry <index>"""

        values = args.split()
        if len(values) != 1:
            print('Usage: modify_entry <index>')
            return

        index = int(values[0])
//::        if pddict['tables'][table]['type'] != 'Index':
        swkey  = p4pd.${table}_swkey_t()
        swkeyp = swkey.this
//::            if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':
        swkey_mask  = p4pd.${table}_swkey_mask_t()
        swkey_maskp = swkey_mask.this
//::            else:
        swkey_maskp = None
//::            #endif
//::        else:
        swkeyp = None
        swkey_maskp = None
//::        #endif
        actiondata = p4pd.${table}_actiondata()

        ret = p4pd.p4pd_entry_read(self.tableid, index, swkeyp, swkey_maskp, actiondata.this)

        if ret < 0:
            print('Error: p4pd_entry_read() returned %d!' % (ret))
            return;

        print('Entry was read successfully at index %d' % (index))

        try:
//::        if pddict['tables'][table]['type'] != 'Index':
            hwkey_len_p = p4pd.new_uint32_ptr_t()
            hwkeymask_len_p = p4pd.new_uint32_ptr_t()
            hwactiondata_len_p = p4pd.new_uint32_ptr_t()

            p4pd.p4pd_hwentry_query(self.tableid, hwkey_len_p, hwkeymask_len_p, hwactiondata_len_p)

            hwkey_p = p4pd.malloc_uint8_t((p4pd.uint32_ptr_t_value(hwkey_len_p)+7)/8)
            hwkeymask_p = p4pd.malloc_uint8_t((p4pd.uint32_ptr_t_value(hwkeymask_len_p)+7)/8)

//::            if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':
            cmd = self.${table}_prompt_for_values(swkey, swkey_mask, actiondata)
            cmd.cmdloop()
//::            else:
            cmd = self.${table}_prompt_for_values(swkey, actiondata)
            cmd.cmdloop()
//::            #endif

            ret = p4pd.p4pd_hwkey_hwmask_build(self.tableid, swkeyp, swkey_maskp, hwkey_p, hwkeymask_p)

            if ret < 0:
                raise RuntimeError('p4pd_hwkey_hwmask_build() returned %d!' % (ret))
//::        else:
            hwkey_p = None
            hwkeymask_p = None

            cmd = self.${table}_prompt_for_values(actiondata)
            cmd.cmdloop()
//::        #endif

            ret = p4pd.p4pd_entry_write(self.tableid, index, hwkey_p, hwkeymask_p, actiondata.this)

            if ret < 0:
                raise RuntimeError('p4pd_entry_write() returned %d!' % (ret))

            print('Entry was written successfully at index %d' % (index))

        except Exception as error:
            print(repr(error))

        finally:
//::        if pddict['tables'][table]['type'] != 'Index':
            p4pd.free_uint8_t(hwkey_p)
            p4pd.free_uint8_t(hwkeymask_p)
            p4pd.delete_uint32_ptr_t(hwkey_len_p)
            p4pd.delete_uint32_ptr_t(hwkeymask_len_p)
            p4pd.delete_uint32_ptr_t(hwactiondata_len_p)
//::        else:
            pass
//::        #endif

    def do_read_entry(self, args):
        """Usage: read_entry <index>"""

        values = args.split()
        if len(values) != 1:
            print('Usage: read_entry <index>')
            return

        index = int(values[0])
//::        if pddict['tables'][table]['type'] != 'Index':
        swkey  = p4pd.${table}_swkey_t()
        swkeyp = swkey.this
//::            if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':
        swkey_mask  = p4pd.${table}_swkey_mask_t()
        swkey_maskp = swkey_mask.this
//::            else:
        swkey_maskp = None
//::            #endif
//::        else:
        swkeyp = None
        swkey_maskp = None
//::        #endif
        actiondata = p4pd.${table}_actiondata()

        ret = p4pd.p4pd_entry_read(self.tableid, index, swkeyp, swkey_maskp, actiondata.this)

        if ret < 0:
            print('Error: p4pd_entry_read() returned %d!' % (ret))
            return;
//::        if pddict['tables'][table]['type'] != 'Index':

        print('${table}_swkey_t:')
//::            for fields in pddict['tables'][table]['keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
            valstr += '0x%02x ' % (p4pd.uint8_array_t_getitem(swkey.${p4fldname}, i))
        print(valstr)
//::                #endif
//::            #endfor
//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:

        print('  ${table}_union${i}_t:')
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
            valstr += '0x%02x ' % (p4pd.uint8_array_t_getitem(swkey.${table}_u${i}.${p4fldname}, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
            valstr += '0x%02x ' % (p4pd.uint8_array_t_getitem(swkey.${p4fldname}, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
//::            for hdrunion in pddict['tables'][table]['hdrunion_keys']:
//::                (union_name, un_fields) = hdrunion
//::                if len(un_fields) > 1:

        print('  ${table}_hdr_union${i}_t:')
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
            valstr += '0x%02x ' % (p4pd.uint8_array_t_getitem(swkey.${table}_hdr_union${i}_t.${p4fldname}, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
            valstr += '0x%02x ' % (p4pd.uint8_array_t_getitem(swkey.${p4fldname}, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
//::        #endif
//::        if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':

        print('${table}_swkey_mask_t:')
//::            for fields in pddict['tables'][table]['keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
            valstr += '0x%02x ' % (p4pd.uint8_array_t_getitem(swkey_mask.${p4fldname}_mask, i))
        print(valstr)

//::                #endif
//::            #endfor
//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                    (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:

        print('  ${table}_union${i}_t:')
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
            valstr += '0x%02x ' % (p4pd.uint8_array_t_getitem(swkey_mask.${table}_mask_u${i}.${p4fldname}_mask, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                    i += 1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
            valstr += '0x%02x ' % (p4pd.uint8_array_t_getitem(swkey_mask.${p4fldname}_mask, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
//::            for fldunion in pddict['tables'][table]['hdrunion_keys']:
//::                (unionname, un_fields) = fldunion
//::                if len(un_fields) > 1:

        print('  ${table}_hdr_union${i}_t:')
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
            valstr += '0x%02x ' % (p4pd.uint8_array_t_getitem(swkey_mask.${table}_mask_hdr_u${i}.${p4fldname}_mask, i))
        print(valstr)

//::                        #endif
//::                    #endfor
//::                    i += 1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
            valstr += '0x%02x ' % (p4pd.uint8_array_t_getitem(swkey_mask.${p4fldname}_mask, i))
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
//::            (actionname, actionfldlist) = action
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
            valstr += '0x%02x ' % (p4pd.uint8_array_t_getitem(actiondata.${table}_action_u.${table}_${actionname}.${actionfldname}, i))
        print(valstr)

//::                    #endif
//::                #endfor
//::            #endif
//::        #endfor

//::    #endfor

class rootCmd(Cmd):

    prompt = "${pddict['cli-name']}> "
    intro = "Interact with ${pddict['cli-name']} P4 tables"

    def __init__(self):
        Cmd.__init__(self)

    def do_quit(self, args):
        """Exits to the previous level"""
        return True

    do_EOF = do_quit

    do_exit = do_quit

    do_done = do_quit

//::    for table in pddict['tables']:
    def do_${table}(self, args):
        """Interact with the ${table} table"""

        cmd = ${table}()
        cmd.cmdloop()
//::    #endfor

    @staticmethod
    def get_table_properties(tbl_idx):
        """Usage: get_table_properties <table-id>"""

        tbl_ctx = p4pd.p4pd_table_properties_t()
        ret = p4pd.p4pd_table_properties_get(tbl_idx, tbl_ctx.this)
        if ret < 0:
            print('p4pd_table_properties_get() returned %d' % (ret))
            return

        print 'tableid: ', tbl_ctx.tableid
        print 'stage_tableid: ', tbl_ctx.stage_tableid
        print 'tablename: ', tbl_ctx.tablename
        if  tbl_ctx.table_type < len(p4pd_table_types_enum):
            print 'table_type: ', p4pd_table_types_enum[tbl_ctx.table_type]
        else:
            print 'table_type: UNKNOWN (%d)' % (tbl_ctx.table_type)
        print 'table_location: ', tbl_ctx.table_location
        print 'gress: ', tbl_ctx.gress
        print 'hash_type: ', tbl_ctx.hash_type
        print 'has_oflow_table: ', tbl_ctx.has_oflow_table
        print 'is_oflow_table: ', tbl_ctx.is_oflow_table
        print 'key_struct_size: ', tbl_ctx.key_struct_size
        print 'actiondata_struct_size: ', tbl_ctx.actiondata_struct_size
        print 'tabledepth: ',  tbl_ctx.tabledepth
        print 'stage: ', tbl_ctx.stage
        print 'oflow_table_id: ', tbl_ctx.oflow_table_id
        print 'sram_layout:'
        print '  entry_width: ', tbl_ctx.sram_layout.entry_width
        print '  entry_width_bits: ', tbl_ctx.sram_layout.entry_width_bits
        print '  start_index: ', tbl_ctx.sram_layout.start_index
        print '  end_index: ', tbl_ctx.sram_layout.end_index
        print '  top_left_x: ', tbl_ctx.sram_layout.top_left_x
        print '  top_left_y: ', tbl_ctx.sram_layout.top_left_y
        print '  top_left_block: ', tbl_ctx.sram_layout.top_left_block
        print '  btm_right_x: ', tbl_ctx.sram_layout.btm_right_x
        print '  btm_right_y: ', tbl_ctx.sram_layout.btm_right_y
        print '  btm_right_block: ', tbl_ctx.sram_layout.btm_right_block
        print '  num_buckets: ', tbl_ctx.sram_layout.num_buckets
        print 'tcam_layout:'
        print '  entry_width: ', tbl_ctx.tcam_layout.entry_width
        print '  entry_width_bits: ', tbl_ctx.tcam_layout.entry_width_bits
        print '  start_index: ', tbl_ctx.tcam_layout.start_index
        print '  end_index: ', tbl_ctx.tcam_layout.end_index
        print '  top_left_x: ', tbl_ctx.tcam_layout.top_left_x
        print '  top_left_y: ', tbl_ctx.tcam_layout.top_left_y
        print '  top_left_block: ', tbl_ctx.tcam_layout.top_left_block
        print '  btm_right_x: ', tbl_ctx.tcam_layout.btm_right_x
        print '  btm_right_y: ', tbl_ctx.tcam_layout.btm_right_y
        print '  btm_right_block: ', tbl_ctx.tcam_layout.btm_right_block
        print '  num_buckets: ', tbl_ctx.tcam_layout.num_buckets
        print 'hbm_layout:'
        print '  entry_width: ', tbl_ctx.hbm_layout.entry_width
        print '  entry_width_bits: ', tbl_ctx.hbm_layout.entry_width_bits
        print '  start_index: ', tbl_ctx.hbm_layout.start_index
        print '  end_index: ', tbl_ctx.hbm_layout.end_index
        print '  top_left_x: ', tbl_ctx.hbm_layout.top_left_x
        print '  top_left_y: ', tbl_ctx.hbm_layout.top_left_y
        print '  top_left_block: ', tbl_ctx.hbm_layout.top_left_block
        print '  btm_right_x: ', tbl_ctx.hbm_layout.btm_right_x
        print '  btm_right_y: ', tbl_ctx.hbm_layout.btm_right_y
        print '  btm_right_block: ', tbl_ctx.hbm_layout.btm_right_block

    def do_read_reg(self, args):
        """Usage: read_reg <addr>"""

        values = args.split()
        if len(values) != 1:
            print('Usage: read_reg <addr>')
            return

        addr = int(values[0], 16)
        data_p = p4pd.new_uint32_ptr_t()
        p4pd.read_reg(addr, data_p)
        print("Value @0x%08x: %d (0x%08x)" % (addr, p4pd.uint32_ptr_t_value(data_p), p4pd.uint32_ptr_t_value(data_p)))
        p4pd.delete_uint32_ptr_t(data_p)

    def do_write_reg(self, args):
        """Usage: write_reg <addr> <value>"""

        values = args.split()
        if len(values) != 2:
            print('Usage: write_reg <addr> <value>')
            return

        addr = int(values[0], 16)
        value = int(values[1], 16)
        p4pd.write_reg(addr, value)

        print("Wrote @0x%08x: %d (0x%08x)" % (addr, value, value))

    def do_read_mem(self, args):
        """Usage: read_mem <addr> <size>"""

        values = args.split()
        if len(values) != 2:
            print('Usage: read_mem <addr> <size>')
            return

        addr = int(values[0], 16)
        size = int(values[1])
        array = p4pd.new_uint8_array_t(size)
        p4pd.read_mem(addr, array, size)
        valstr = "Content @0x%08x: {" % (addr)
        lenstr = len(valstr)
        for i in range(size):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += "0x%02x " % (p4pd.uint8_array_t_getitem(array, i))
        valstr += "}"
        print(valstr)
        p4pd.delete_uint8_array_t(array)

    def do_write_mem(self, args):
        """Usage: write_mem <addr> <size> <value0> <value1> ... <valueSize-1>"""

        values = args.split()
        if len(values) < 2 or len(values) != (int(values[1]) + 2):
            print('"Usage: write_mem <addr> <size> <value0> <value1> ... <valueSize-1>')
            return

        addr = int(values[0], 16)
        size = int(values[1])

        if size > 0:
            array = p4pd.new_uint8_array_t(size)
            for i in range(size):
                p4pd.uint8_array_t_setitem(array, i, int(values[i+2], 16))

            p4pd.write_mem(addr, array, size)
            p4pd.delete_uint8_array_t(array)

        print("Wrote %d bytes @0x%08x" % (size, addr))

    def do_dump_hbm(self, args):
        """Usage: dump_hbm"""

        values = args.split()
        if len(values) != 0:
            print('Usage: dump_hbm')
            return

        p4pd.dump_hbm()
        print("Dumped HBM.")

    def do_send_pkt(self, args):
        """Usage: send_pkt <filename> <port> [cos]"""

        values = args.split()
        if len(values) < 2 or len(values) > 3:
            print('Usage: send_pkt <filename> <port> [cos]')
            return

        filename = values[0]
        port = int(values[1])
        if len(values) > 2:
            cos = int(values[2])
        else:
            cos = 0

        try:
            in_pkt = p4pd.vector_uint8_t()
            with open(filename, "rb") as pkt_file:
                pkt_bytes = bytearray(open(filename, "rb").read())
                pkt_file.close()
                for i in range(len(pkt_bytes)):
                    in_pkt.push_back(pkt_bytes[i])
                p4pd.step_network_pkt(in_pkt, port)
                print("Sent packet %s to the model on port %d." % (filename, port))

                out_pkt = p4pd.vector_uint8_t()
                out_cos_p = p4pd.new_uint32_ptr_t()
                out_port_p = p4pd.new_uint32_ptr_t()
                p4pd.uint32_ptr_t_assign(out_cos_p, 0)
                p4pd.uint32_ptr_t_assign(out_port_p, 0)
                p4pd.get_next_pkt(out_pkt, out_port_p, out_cos_p)
                if out_pkt.size():
                    print("Got packet back from the model: %d bytes on port %d with cos %d" % (out_pkt.size(),
                                                                                               p4pd.uint32_ptr_t_value(out_port_p),
                                                                                               p4pd.uint32_ptr_t_value(out_cos_p)))
                else:
                    print("No packet back from the model.")

        except Exception as error:
            print(repr(error))

    def complete_send_pkt(self, text, line, begidx, endidx):
        before_arg = line.rfind(" ", 0, begidx)
        if before_arg == -1:
            return # arg not found

        fixed = line[before_arg+1:begidx]  # fixed portion of the arg
        arg = line[before_arg+1:endidx]
        pattern = arg + '*'

        completions = []
        for path in glob.glob(pattern):
            completions.append(path.replace(fixed, "", 1))

        return completions

def init():
    p4pd.p4pd_cli_init()

def cleanup():
    p4pd.p4pd_cli_cleanup()

if __name__ == '__main__':
    init()
    cmd = rootCmd()
    cmd.cmdloop()
    cleanup()
