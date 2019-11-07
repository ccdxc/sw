from nats.aio.client import Client as NATS
from nats.aio.errors import ErrConnectionClosed, ErrTimeout, ErrNoServers
import ast
import asyncio
import iotaLogsHelper as ilh
import iotaPostgresLib as ipl
import re
import time
import traceback

async def run(loop):
    nc = NATS()
    pg = ipl.PgHelper('127.0.0.1', 'test_results', 'test_results_writer', 'test_results_writer')
    await nc.connect(servers=["nats://tracker.test.pensando.io:4222"], loop=loop)

    async def message_handler(msg):
        subject = msg.subject
        reply = msg.reply
        data = msg.data.decode()
        data = re.sub(":true",":True",data)
        data = re.sub(":false",":False",data)
        data = re.sub(":null",":None",data)
        try:
            d = ast.literal_eval(data)
            print("job {0} with id {1} finished at {2}".format(d['ID'],d['Name'],d['FinishedAt']))
        except:
            print("failed to process job info. error was:{0}".format(traceback.format_exc()))
            print(data)
            return
        try:
            tsRes = ilh.parseTsResultsText(d['ID'],"logs")
            if tsRes:
                print("updating database with {0}".format(tsRes))
                pg.updateTsResultsDb(tsRes)
        except:
            print("failed to find or parse test suite results file")
            print(traceback.format_exc())   
    await nc.subscribe("job.*.*", cb=message_handler)

    while True:
        await asyncio.sleep(1, loop=loop)

    await nc.close()

if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(run(loop))
    loop.close()


