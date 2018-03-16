#include <stdio.h>
#include <string.h>
#include "e_pse.h"
#include "pse_ec.h"
#include "pse_rsa.h"

/* OpenSSL includes */

/* pse engine id declarations */
static const char *engine_pse_id = "pse";
static const char *engine_pse_name = 
    "Pensando TLS/SSL offload engine";

/*
 * Description:
 *  Connect PSE engine to OpenSSL engine library
 */
static int pse_bind_helper(ENGINE* eng, const char *id)
{
    int ret = 0;
    if(!pse_init_log()) {
        return 0;
    }

    if(id && (strcmp(id, engine_pse_id) != 0)) {
        WARN("ENGINE_id defined already!");
        goto cleanup;
    }
    if(!ENGINE_set_id(eng, engine_pse_id)) {
        WARN("ENGINE_set_id failed \n");
        goto cleanup;
    }

    if(!ENGINE_set_name(eng, engine_pse_name)) {
        WARN("ENGINE_set_name failed \n");
        goto cleanup;
    }
    
    if(!ENGINE_set_EC(eng, pse_get_EC_methods())) {
        WARN("ENGINE_set_EC failed");
        goto cleanup;
    }
 
    if(!ENGINE_set_RSA(eng, pse_get_RSA_methods())) {
        WARN("ENGINE_set_RSA failed");
        goto cleanup;
    }
    
    ret = 1;
cleanup:
    INFO("pse bind completed. ret: %d", ret);
    return ret;
}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(pse_bind_helper);

