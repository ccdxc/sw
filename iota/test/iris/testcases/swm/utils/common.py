from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from iota.harness.infra.redfish.rest.v1 import InvalidCredentialsError
from iota.harness.infra.redfish import redfish_client

def get_redfish_obj(cimc_info, mode="dedicated"):
    if mode == "dedicated":
        curr_ilo_ip = cimc_info.GetIp()
    else:
        curr_ilo_ip = cimc_info.GetNcsiIp() 
    
    try:
        # Create a Redfish client object
        RF = redfish_client(base_url="https://%s" % curr_ilo_ip,
                                username=cimc_info.GetUsername(),
                                password=cimc_info.GetPassword(), max_retry=2)
        # Login with the Redfish client
        RF.login()
    except ServerDownOrUnreachableError:
        raise RuntimeError("ILO(%s) not reachable or does not support RedFish"
                           % curr_ilo_ip)
    except InvalidCredentialsError:
        raise RuntimeError("Invalid credentials for ILO(%s) login"
                           % curr_ilo_ip)
    return RF