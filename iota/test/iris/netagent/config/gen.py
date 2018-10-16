#! /usr/bin/python3

import iota.test.iris.netagent.config.objects.tenant as tenant

class _NetagentObjectStore:
    def __init__(self):
        return

class GenerateConfigParams:
    def __init__(self):
        self.num_tenants = 1
        self.num_namespaces_per_tenant = 1
        self.num_segments_per_namespace = 2
        self.num_eps_per_segment = 6
        self.num_eps_per_node = 2
        return

def GenerateConfig(params)
    assert(isinstance(params, GenerateConfigParams))
    return tenant.GenerateConfig(params)
