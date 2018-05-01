//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: #pdb.set_trace()
//::
//:: if pddict['p4plus']:
//::    api_prefix = 'p4pd_' + pddict['p4program']
//:: else:
//::    api_prefix = 'p4pd'
//:: #endif
//::
#!/usr/bin/python
# This file is auto-generated. Changes will be overwritten!
#
# Debug CLI
#

//:: register_list =  ["ppa","sgi","rpc","intr","pxb","sge","pr","pp","pt","tsi","pcr","txs","tse","pct","pb","pm","db","ssi","sse","bx","md","tpc","dpr","mc","dpp","sema","mp", "ms","mx"]

import click
from click_repl import register_repl
import ${pddict['cli-name']}_debug_cli as backend
from   ${pddict['cli-name']} import *
import ${pddict['cli-name']}
import os

array_cols = 16

${api_prefix}_table_types_enum = [
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
def show(ctx):
    pass

@show.group()
@click.pass_context
def system(ctx):
    pass

# SHOW SYSTEM TABLE
@system.group()
@click.pass_context
def table(ctx):
    ctx.obj={}
    pass

@table.command(name="list")
#@click.option("-f", "--file", "file_name", default="", multiple=False)
def list_table():
    backend.table_dump()
    pass

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
    pass

@${table}.group(invoke_without_command=True)
@click.pass_context
def all(ctx):
    config = {}
    config['table_name'] = ctx.obj['table_name']
    config['action_name'] = ''
    config['opn'] = ctx.obj['opn']
    config['index'] = 0xffffffff
    config['actionid_vld'] = False
    config['actionfld'] = {}
    config['swkey']={}
    config['swkey_mask']={}
    backend.populate_table(config)
    pass

//::    #endfor


## show sytem register
@system.group()
@click.pass_context
def register(ctx):
    ctx.obj={}
    pass

@register.group()
@click.pass_context
def read(ctx):
    ctx.obj['opn'] = 'read'

@read.group(invoke_without_command=True)
@click.argument('reg_name', type=click.STRING)
@click.pass_context
def reg_name(ctx, reg_name):
    config= {}
    config['reg_name'] = reg_name
    config['block_name'] = ""
    config['is_reg_name'] = True
    config['opn'] = ctx.obj['opn']
    backend.populate_register(config)
    pass


@read.group(invoke_without_command=True)
@click.option("-f", "--file", "file_name", default="", multiple=False)
@click.pass_context
def all(ctx, file_name):
    config={}
    config['block_name'] = 'all'
    config['reg_name'] = ""
    config['file_name'] = file_name
    config['is_reg_name'] = False
    config['opn'] = ctx.obj['opn']
    backend.populate_register(config)
    pass

@read.group()
@click.pass_context
def block(ctx):
    pass

@block.group(invoke_without_command=True)
@click.pass_context
def block_name(ctx):
    pass

//:: for block in register_list:
@block_name.group(invoke_without_command=True)
@click.option("-f", "--file", "file_name", default="", multiple=False)
@click.pass_context
def ${block}(ctx,file_name):
    ctx.obj['block_name'] = 'cap0.' + '${block}'
    config = {}
    config['reg_name'] = ""
    config['opn'] = ctx.obj['opn']
    config['block_name'] = ctx.obj['block_name']
    config['is_reg_name'] = False
    config['file_name'] = file_name
    backend.populate_register(config)
    pass
//::    #endfor

@register.command(name="list")
@click.option("-f", "--file", "file_name", default="")
@click.pass_context
def list_register(ctx,file_name):
    config = {}
    config['reg_name'] = ""
    config['opn'] = 'list'
    config['block_name'] = "all"
    config['is_reg_name'] = False
    config['file_name'] = file_name
    backend.populate_register(config)
    pass

# debug hardware table write
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
    ctx.obj['is_reg'] = False
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

register_repl(dbg_cli)
dbg_cli.add_command(debug)
grpc_server_port = "localhost:50054"

if 'HAL_GRPC_PORT' in os.environ.keys():
    grpc_server_port = "localhost:" + os.environ['HAL_GRPC_PORT']

${pddict['cli-name']}.${api_prefix}_cli_init(grpc_server_port)

if __name__ == '__main__':
    dbg_cli()

