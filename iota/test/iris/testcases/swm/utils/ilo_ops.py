from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
import iota.harness.api as api
from .response import validate_resp
import traceback

import sys
import time

def poll_task_monitor( context, response ):
    """
    Monitors a task monitor until it's complete and prints out progress
    NOTE: This call will block until the task is complete
    Args:
        context: The Redfish client object with an open session
        response: The initial response from the operation that produced the task
    Returns:
        The final response of the request
    """

    # No task was produced; just return the existing response
    if not response.is_processing:
        return response

    # Poll the task until completion
    task_monitor = response
    while task_monitor.is_processing:
        # Print the progress
        task_state = None
        task_percent = None
        if task_monitor.dict is not None:
            task_state = task_monitor.dict.get( "TaskState", None )
            task_percent = task_monitor.dict.get( "PercentComplete", None )
        if task_state is None:
            task_state = "Running"
        if task_percent is None:
            progress_str = "Task is {}\r".format( task_state )
        else:
            progress_str = "Task is {}: {}% complete\r".format( task_state, task_percent )
        sys.stdout.write( "\x1b[2K" )
        sys.stdout.write( progress_str )
        sys.stdout.flush()

        # Sleep for the requested time
        retry_time = response.retry_after
        if retry_time is None:
            retry_time = 1
        time.sleep( retry_time )

        # Check the monitor for an update
        task_monitor = response.monitor( context )
    sys.stdout.write( "\x1b[2K" )
    api.Logger.info( "Task is Done!" )

    return task_monitor

def reset_ilo(_redfishobj):
    managers_members_response = None
    #if we do not have a resource directory or want to force it's non use to find the
    #relevant URI
    managers_uri = _redfishobj.root['Managers']['@odata.id']
    managers_response = _redfishobj.get(managers_uri)
    managers_members_uri = next(iter(managers_response.obj['Members']))['@odata.id']
    managers_members_response = _redfishobj.get(managers_members_uri)
    
    if managers_members_response:
        reset_ilo_uri = managers_members_response.obj['Actions']['#Manager.Reset']['target']
        body = {'Action': 'Manager.Reset'}
        resp = _redfishobj.post(reset_ilo_uri, body=body)
        resp = poll_task_monitor(_redfishobj, resp)
        ret = validate_resp(resp)
        if ret != api.types.status.SUCCESS:
            raise RuntimeError("ILO reset was not sucessful")
    else:
        raise RuntimeError('Could not get mgmt obj')
