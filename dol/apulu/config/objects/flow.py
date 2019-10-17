#! /usr/bin/python3
import pdb

import apollo.config.objects.flow as flow

FlowMapHelper = flow.FlowMapObjectHelper()

def GetMatchingObjects(selectors):
    return FlowMapHelper.GetMatchingConfigObjects(selectors)
