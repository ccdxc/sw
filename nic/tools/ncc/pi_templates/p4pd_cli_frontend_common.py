//:: prog_name = _context['prog_name']
#!/usr/bin/python
# This file is auto-generated. Changes will be overwritten!
#
# Debug CLI
#

import os
import click
import ${prog_name}_backend as backend

//:: #register_list =  ["ppa","sgi","rpc","intr","pxb","sge","pr","pp","pt","tsi","pcr","txs","tse","pct","pb","pm","db","ssi","sse","bx","md","tpc","dpr","mc","dpp","sema","mp", "ms","mx"]
//:: register_list = []
@click.group()
def dbg_cli():
    pass

# show
#@dbg_cli.group()
#@click.pass_context
#def show(ctx):
#    pass

# show system
#@show.group()
#@click.pass_context
#def system(ctx):
#    pass

# show sytem register
#@system.group()
#@click.pass_context
#def register(ctx):
#    ctx.obj={}
#    pass

# show system register read
#@register.group()
#@click.pass_context
#def read(ctx):
#    ctx.obj['opn'] = 'read'

# show system register read reg_name
#@read.group(invoke_without_command=True)
#@click.argument('reg_name', type=click.STRING)
#@click.pass_context
#def reg_name(ctx, reg_name):
#    config= {}
#    config['reg_name'] = reg_name
#    config['block_name'] = ""
#    config['is_reg_name'] = True
#    config['opn'] = ctx.obj['opn']
#    backend.populate_register(config)
#    pass

# show system register read all
#@read.group(invoke_without_command=True)
#@click.option("-f", "--file", "file_name", default="", multiple=False)
#@click.pass_context
#def all(ctx, file_name):
#    config={}
#    config['block_name'] = 'all'
#    config['reg_name'] = ""
#    config['file_name'] = file_name
#    config['is_reg_name'] = False
#    config['opn'] = ctx.obj['opn']
#    backend.populate_register(config)
#    pass

# show system register read block
#@read.group()
#@click.pass_context
#def block(ctx):
#    pass

# show system register read block block_name
#@block.group(invoke_without_command=True)
#@click.pass_context
#def block_name(ctx):
#    pass

//:: for block in register_list:
#@block_name.group(invoke_without_command=True)
#@click.option("-f", "--file", "file_name", default="", multiple=False)
#@click.pass_context
#def ${block}(ctx,file_name):
#    ctx.obj['block_name'] = 'cap0.' + '${block}'
#    config = {}
#    config['reg_name'] = ""
#    config['opn'] = ctx.obj['opn']
#    config['block_name'] = ctx.obj['block_name']
#    config['is_reg_name'] = False
#    config['file_name'] = file_name
#    backend.populate_register(config)
#    pass

//::    #endfor

# show system register list
#@register.command(name="list")
#@click.option("-f", "--file", "file_name", default="")
#@click.pass_context
#def list_register(ctx,file_name):
#    config = {}
#    config['reg_name'] = ""
#    config['opn'] = 'list'
#    config['block_name'] = "all"
#    config['is_reg_name'] = False
#    config['file_name'] = file_name
#    backend.populate_register(config)
#    pass

# show system table
#@system.group()
#@click.pass_context
#def table(ctx):
#    ctx.obj={}
#    pass

# show system table list_table
#@table.command(name="list")
#@click.option("-f", "--file", "file_name", default="", multiple=False)
#def list_table():
#    backend.table_dump()
#    pass

# show system rawtable
#@system.group()
#@click.pass_context
#def rawtable(ctx):
#    ctx.obj={}
#    ctx.obj['opn'] ='read'
#    pass

# debug
@dbg_cli.group()
@click.pass_context
def debug(ctx):
    pass

# debug hardware
@debug.group()
@click.pass_context
def hardware(ctx):
    pass

# debug hardware table
@hardware.group()
@click.pass_context
def table(ctx):
    ctx.obj = {}
    ctx.obj['is_reg'] = False
    pass

# debug hardware table list
@table.command(name="list")
@click.option("--out_json", is_flag=True)
def list_table(out_json):
    backend.table_dump(out_json)
    pass

# debug hardware table read
@table.group()
@click.pass_context
def read(ctx):
    ctx.obj['opn'] ='read'

# debug hardware table write
@table.group()
@click.pass_context
def write(ctx):
    ctx.obj['opn']='write'

# exit
@dbg_cli.command(name="exit")
def exit():
    os._exit(0)
