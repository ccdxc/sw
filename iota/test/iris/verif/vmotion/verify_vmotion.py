import iota.harness.api as api

def Main(tc):
    vmotion_enabled = getattr(tc, 'vmotion_enabled', False)
    if not vmotion_enabled:
        return api.types.status.SUCCESS

    if tc.skip: return api.types.status.SUCCESS

    if tc.vmotion_resp is None:
        api.Logger.error("No vmotion trigger/response available")
        return api.types.status.SUCCESS

    if not api.Trigger_IsSuccess(tc.vmotion_resp):
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

