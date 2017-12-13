import pdb
import sys
import os

'''
#
# sys.path.append is not required if PYTHONPATH is set
#

base_dir = os.path.dirname(__file__)

# import p4pd package
req_pkg_dir = os.path.join(base_dir, 'swig/pyfiles')
sys.path.append(req_pkg_dir)

#import hal debug grpc package
req_pkg_dir = os.path.join(base_dir, '../gen/proto/hal')
sys.path.append(req_pkg_dir)

#import grpcio, click, click-repl packages
req_pkg_dir = os.path.join(base_dir, '../third-party/lib64/python2.7')
sys.path.append(req_pkg_dir)
'''

import click
from click_repl import register_repl
import p4pd
import grpc
import debug_pb2
import debug_pb2_grpc

grpc_server = 'localhost:50052'

if 'GRPC_SERVER' in os.environ.keys():
    grpc_server = os.environ['GRPC_SERVER']

def debug_grpc_setup():
    global debug_client_stub

    # create grpc client connection
    debug_client_channel = grpc.insecure_channel(grpc_server)
    debug_client_stub = debug_pb2_grpc.DebugStub(debug_client_channel)

'''
@click.command()
@click.argument('name')
def ep(name):
    click.echo('Show ep object %s' %(name))
    return

@click.command()
@click.argument('name')
def tenant(name):
    click.echo('Show tenant object %s' %(name))
    return 


@click.group(invoke_without_command=True, chain=True)
@click.argument('ids')
#@click.argument('number', required=False)

def hal(ids):
    click.echo('Show hal %s'% (ids))

@hal.resultcallback()
def process_results(ctx, ids):
    click.echo('process end result %s' % (ids))

@click.group()
def agent():
    click.echo('Show agent')

@agent.command()
@click.option('--len', '-len', is_flag=True)
def security_group(len):
    if len:
        click.echo('len of security group is 5')
    click.echo('Show security group')

@click.command()
def platform():
    click.echo('Show platform')

@click.command()
def table():
    click.echo('Set table')

@click.group()
def show():
    click.echo('Show command')

@click.group()
def set_hardware():
    click.echo('Set command')

hal.add_command(ep)
hal.add_command(tenant)
show.add_command(platform)
show.add_command(hal)
show.add_command(agent)

set_hardware.add_command(table)

@click.group()
@click.pass_context
def pen_debug_cli(ctx):
    pass

pen_debug_cli.add_command(set_hardware)
'''

@click.group()
def pen_debug_cli():
    pass

@pen_debug_cli.group()
@click.pass_context
def debug(ctx):
    pass

def exit(ctx):
    sys.exit(0)


@debug.group()
@click.pass_context 
def hardware(ctx):
    pass

@hardware.group()
@click.argument('opn', metavar='<read/write>', type=click.Choice(['read', 'write']))
@click.pass_context
def table(ctx, opn):
    ctx.obj = {}
    ctx.obj['type'] = 'table'
    ctx.obj['opn'] = opn
    pass

@hardware.group()
@click.argument('opn', metavar='<read/write>', type=click.Choice(['read', 'write']))
@click.pass_context
def register(ctx, opn):
    ctx.obj = {}
    ctx.obj['type'] = 'register'
    ctx.obj['opn'] = opn
    pass

@register.group()
@click.argument('addr', type=click.INT)
@click.pass_context
def address(ctx, addr):
    ctx.obj['address'] = addr
    pass

@address.command(name='data')
@click.argument('reg_data', type=click.INT)
@click.pass_context
def reg_data_cmd(ctx, reg_data):
    ctx.obj['reg_data'] = reg_data
    return

@table.group()
@click.pass_context
def policer(ctx):
    pass

@policer.group(invoke_without_command=True, chain=True)
@click.argument('ids', metavar='policer_id', type=click.INT)
@click.pass_context
def policer_index(ctx, ids):
    pass

def debug_opn_get(opn):
    if (opn == 'read'):
        return debug_pb2.DEBUG_OP_TYPE_READ

    if (opn == 'write'):
        return debug_pb2.DEBUG_OP_TYPE_WRITE

    return debug_pb2.DEBUG_OP_TYPE_NONE

def debug_msg_send(ctx, debug_request_msg):
    debug_grpc_setup()

    global debug_client_stub
    return debug_client_stub.DebugInvoke(debug_request_msg)


@register.resultcallback()
@click.pass_context
def process_reg_result(ctx, rslt, opn):

    # create the grpc DEBUG Request
    debug_request_msg = debug_pb2.DebugRequestMsg()

    debug_request = debug_request_msg.request.add()
    debug_request.opn_type = debug_opn_get(ctx.obj['opn'])
    debug_request.key_or_handle.reg_id = 100
    debug_request.addr = ctx.obj['address']

    if (ctx.obj['opn'] == 'write'):
        debug_request.reg_data = ctx.obj['reg_data']

    rsp = debug_msg_send(ctx, debug_request_msg)

    response = rsp.response.pop()
    print 'Debug CLI status: ' + str(response.debug_status)

    if (ctx.obj['opn'] == 'read'):
        print 'Read data: '  + str(hex(response.ret_data))


@policer_index.resultcallback()
@click.pass_context
def process_results(ctx, rslt, ids):

    # populate the structure
    egress_policer_execute_egress_policer_ = \
                                p4pd.egress_policer_execute_egress_policer_t()
    egress_policer_execute_egress_policer_.entry_valid = ctx.obj['entry_valid']

    egress_policer_action_union_ = p4pd.egress_policer_action_union_t()
    egress_policer_action_union_.egress_policer_execute_egress_policer = \
                                        egress_policer_execute_egress_policer_

    egress_policer_actiondata_ = p4pd.egress_policer_actiondata()
    egress_policer_actiondata_.actionid = 200
    egress_policer_actiondata_.egress_policer_action_u = egress_policer_action_union_

    # create the grpc Debug CLI Request
    debug_request_msg = debug_pb2.DebugRequestMsg()

    debug_request = debug_request_msg.request.add()
    debug_request.opn_type = debug_opn_get(ctx.obj['opn'])
    debug_request.key_or_handle.table_id = 100
    debug_request.str_data = p4pd.get_data(egress_policer_actiondata_)

    debug_msg_send(ctx, debug_request_msg)

    config = {}
    for arg, value in rslt:
        config[arg] = value
    print(config)


@policer_index.command(name='entry_valid')
@click.argument('arg_valid', type=click.INT)
@click.pass_context
def entry_valid_cmd(ctx, arg_valid):
    ctx.obj['entry_valid'] = arg_valid
    return ('arg_valid', arg_valid)

@policer_index.command(name='pkt_rate')
@click.argument('arg_rate', type=click.INT)
def pkt_rate_cmd(arg_rate):
    return ('arg_rate', arg_rate)

@policer_index.command(name='rlimit_en')
@click.argument('arg_limit_en', type=click.INT)
def rlimit_en_cmd(arg_limit_en):
    return ('arg_limit_en', arg_limit_en)    

@policer_index.command(name='rlimit_prof')
@click.argument('arg_rlimit_prof', type=click.INT)
def rlimit_prof_cmd(arg_rlimit_prof):
    return ('arg_rlimit_prof', arg_rlimit_prof)

'''
@policer_index.command(name='color_aware')
@click.argument('color_aware', type=click.INT)
def color_aware_cmd(arg_color_aware)
    config.color_aware = arg_color_aware
    pass

@policer_index.command(name='axi_wr_pend')

@click.argument('axi_wr_pend', type=click.INT)
def axi_wr_pend_cmd(arg_axi_wr_pend)
    return ('axi_wr_pend', arg_axi_wr_pend)
    pass

@policer_index.command()
@click.argument('burst', type=click.INT)
def burst(arg_burst)
    return ('burst', burst)
    pass

@policer_index.command()

@click.argument('rate', type=click.INT)
def rate(arg_rate)
    return ('rate', arg_rate)
    pass

@policer_index.command()
@click.argument('tbkt', type=click.INT)
def tbkt(arg_tbkt)
    return ('tbkt', arg_tbkt)
    pass
'''

register_repl(pen_debug_cli)
pen_debug_cli.add_command(debug)

if __name__ == '__main__':
    debug_grpc_setup()
    pen_debug_cli()

