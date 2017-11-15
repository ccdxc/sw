//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: #pdb.set_trace()
#!/usr/bin/python
# This file is auto-generated. Changes will be overwritten!
#
# Debug CLI
#

import click
from click_repl import register_repl
import iris_debug_cli as backend
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

@click.group()
def dbg_cli():
    pass

@dbg_cli.group()
@click.pass_context
def debug(ctx):
    pass

@debug.group()
@click.pass_context
def hardware(ctx):
    pass

@hardware.group()
@click.pass_context
def table(ctx):
    ctx.obj = {}
    pass

@table.group()
@click.pass_context
def write(ctx):
    ctx.obj['opn']='write'

//::    for table in pddict['tables']:
@write.group()
@click.pass_context
def ${table}(ctx):
    ctx.obj['table_name']='${table}'
    pass

@${table}.group(invoke_without_command=True)
@click.argument('ids', metavar='${table}_id', type=click.INT)
@click.pass_context
def ${table}_index(ctx, ids):
    pass

@${table}_index.resultcallback()
@click.pass_context
def process_results(ctx, rslt, ids):
    config = {}
    config['action_name'] = ''
    config['table_name'] = ctx.obj['table_name']
    config['opn'] = ctx.obj['opn']
    config['index'] = ids
    config['action_name'] = ctx.obj['action_name']
    config['actionid_vld'] = False
    config['actionfld'] = {}
    config['swkey'] = {}
    config['swkey_mask'] = {}
    for arg, value in rslt:
        config['actionfld'][arg] = value
    backend.populate_table(config)
    print config

//::        for action in pddict['tables'][table]['actions']:
//::            (actionname, actionfldlist) = action
//::            fld_len = len(actionfldlist)
//::            if fld_len == 0:
@${table}_index.group(invoke_without_command=True, chain=True)
//::            else:
@${table}_index.group(chain=True)
//::            #endif
@click.pass_context
def ${actionname}(ctx):
    ctx.obj['action_name'] =  '${actionname}'
    pass

//::            if (len(actionfldlist) > 0):
//::                for actionfld in actionfldlist:
//::                    actionfldname,actionfldwidth = actionfld
@${actionname}.command(name='${actionfldname}')
@click.argument('arg_${actionfldname}', type=click.STRING)
def ${actionfldname}_cmd(arg_${actionfldname}):
    return ('${actionfldname}', arg_${actionfldname})

//::                #endfor
//::            #endif
//::        #endfor
//::    #endfor
@table.group()
@click.pass_context
def read(ctx):
    ctx.obj['opn'] ='read'

//::    for table in pddict['tables']:
@read.group()
@click.pass_context
def ${table}(ctx):
    ctx.obj['table_name'] ='${table}'
    pass

@${table}.group(invoke_without_command=True)
@click.argument('ids', metavar='${table}_id', type=click.INT)
@click.pass_context
def ${table}_index(ctx, ids):
    config = {}
    config['table_name'] = ctx.obj['table_name']
    config['action_name'] = ''
    config['opn'] = ctx.obj['opn']
    config['index'] = ids
    config['actionid_vld'] = False
    config['actionfld'] = {}
    config['swkey']={}
    config['swkey_mask']={}
    backend.populate_table(config)
    print config
    pass

#@${table}_index.resultcallback()
#@click.pass_context
#def process_results(ctx, rslt, ids):
#    config = {}
#    config['table_name'] = ctx.obj['table_name']
#    config['opn'] = ctx.obj['opn']
#    config['index'] = ids
#    print config
//::    #endfor
register_repl(dbg_cli)
dbg_cli.add_command(debug)

if __name__ == '__main__':
    dbg_cli()

