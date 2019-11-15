#! /usr/bin/python3

import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.route as route
import apollo.config.utils as utils

class RouteTableObjectHelper:
    def __init__(self):
        return

    def __is_lmapping_match(self, route_obj, lobj):
        if lobj.AddrFamily == 'IPV4':
            return lobj.AddrFamily == route_obj.AddrFamily and\
               lobj.VNIC.SUBNET.V4RouteTableId == route_obj.RouteTblId
        if lobj.AddrFamily == 'IPV6':
            return lobj.AddrFamily == route_obj.AddrFamily and\
               lobj.VNIC.SUBNET.V6RouteTableId == route_obj.RouteTblId

    def GetMatchingConfigObjects(self, selectors, all_objs):
        objs = []
        rtype = selectors.route.GetValueByKey('RouteType')
        for route_obj in route.client.Objects():
            if not route_obj.IsFilterMatch(selectors):
                continue
            if rtype != 'empty' and 0 == len(route_obj.routes):
                # skip route tables with no routes
                continue
            for lobj in lmapping.GetMatchingObjects(selectors):
                if self.__is_lmapping_match(route_obj, lobj):
                    route_obj.l_obj = lobj
                    objs.append(route_obj)
                    break
        if all_objs is True:
            maxlimits = 0
        else:
            maxlimits = selectors.maxlimits
        return utils.GetFilteredObjects(objs, maxlimits)

RouteTableHelper = RouteTableObjectHelper()

def GetMatchingObjects(selectors):
    return RouteTableHelper.GetMatchingConfigObjects(selectors, False)
