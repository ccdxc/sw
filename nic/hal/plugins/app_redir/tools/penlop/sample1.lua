
--[[
# Copyright 2018 Pensando Systems, Inc.
--]]
--[[
detection_name: PTTP
version: 1
description: 
--]]

require "DetectorCommon"
local DC = DetectorCommon
local FT = flowTrackerModule
DetectorPackageInfo = {
  name = "PTTP",
  proto = DC.ipproto.tcp,
  server = {
    init = 'DetectorInit',
    validate = 'DetectorValidator',
  }
}

gServiceName = "PTTP"

gPorts = {
  { DC.ipproto.tcp, 70 },
  { DC.ipproto.tcp, 7070 },
}

gPatterns = {
  pat1 = {'PET', 0 },
  pat2 = {'FEAD', 0 },
  pat3 = {'HUSH', 0 },
  pat4 = {'GOST', 0 },
  pat5 = {'PTTP/1', 0 },
}

gFastPatterns = {
  { DC.ipproto.tcp, gPatterns.pat1 },
  { DC.ipproto.tcp, gPatterns.pat2 },
  { DC.ipproto.tcp, gPatterns.pat3 },
  { DC.ipproto.tcp, gPatterns.pat4 },
  { DC.ipproto.tcp, gPatterns.pat5 },
}


function serviceInProcess(context)
    local flowFlag = context.detectorFlow:getFlowFlag(DC.flowFlags.serviceDetected)

    if ((not flowFlag) or (flowFlag == 0)) then
        gDetector:inProcessService()
    end

    DC.printf('%s: Inprocess, packetCount: %d\n', gServiceName, context.packetCount);
    return DC.serviceStatus.inProcess
end

function serviceSuccess(context)
    local flowFlag = context.detectorFlow:getFlowFlag(DC.flowFlags.serviceDetected)

    if ((not flowFlag) or (flowFlag == 0)) then
        gDetector:addService(gAppId, "", "", gAppId)
    end

    DC.printf('%s: Detected, packetCount: %d\n', gServiceName, context.packetCount);
    return DC.serviceStatus.success
end

function serviceFail(context)
    local flowFlag = context.detectorFlow:getFlowFlag(DC.flowFlags.serviceDetected)

    if ((not flowFlag) or (flowFlag == 0)) then
        gDetector:failService()
    end

    context.detectorFlow:clearFlowFlag(DC.flowFlags.continue)
    DC.printf('%s: Failed, packetCount: %d\n', gServiceName, context.packetCount);
    return DC.serviceStatus.nomatch
end

function getFlowTrackerItem(rft, flowField, defStr)
    if rft == nil then
        return defStr
    end
    if rft.flowField == nil then
        return defStr
    end
    return rft.flowField
end

function registerPortsPatterns()
    if (gPorts) then
        for i,v in ipairs(gPorts) do
            gDetector:addPort(v[1], v[2])
            DC.printf('%s: registering port %d\n', gServiceName, v[2]);
        end
    end

    if (gPairs) then
        for i,v in ipairs(gFastPatterns) do
            if ( gDetector:registerPattern(v[1], v[2][1], #v[2][1], v[2][2], gAppId) ~= 0) then
                DC.printf('%s: register pattern failed for %s\n', gServiceName, v[2])
            else
                DC.printf('%s: register pattern successful for %d\n', gServiceName, i)
            end
        end
    end
end

function DetectorInit(detectorInstance)
  gDetector = detectorInstance;
  gAppId = gDetector:open_createApp(gServiceName);

  gDetector:init(gServiceName, 'DetectorValidator', 'DetectorFini')
  registerPortsPatterns()

  DC.printf("Done registration for %s custom appid: %d\n", gServiceName, gAppId);
  return gDetector;
end

function DetectorFini()
  DC.printf('%s:DetectorFini()\n', gServiceName);
end

function DetectorValidator()
  local context = {}
  context.detectorFlow = gDetector:getFlow()
  context.packetDataLen = gDetector:getPacketSize()
  context.packetDir = gDetector:getPacketDir()
  context.srcIp = gDetector:getPktSrcAddr()
  context.dstIp = gDetector:getPktDstAddr()
  context.srcPort = gDetector:getPktSrcPort()
  context.dstPort = gDetector:getPktDstPort()
  context.flowKey = context.detectorFlow:getFlowKey()
  context.packetCount = gDetector:getPktCount()

  DC.printf('%s:DetectorValidator(): packetCount %d, dir %d\n', gServiceName, context.packetCount, context.packetDir);

  if (context.packetDataLen == 0) then
    return serviceInProcess(context)
  end

  init(context)
  
  return serviceInProcess(context)
end

function init(context)
  DC.printf('%s: Entering state init\n', gServiceName)
  if ((context.packetDir==1)) then
    resp_init(context)
  else
    req_init(context)
  end
end

function req_init(context)
  DC.printf('%s: Entering state req_init\n', gServiceName)
  matched = nil
  if (not matched) then
    matched = gDetector:getPcreGroups("PET ", 0)
    if (matched) then
      method_known(context)
    end
  end
  if (not matched) then
    matched = gDetector:getPcreGroups("FEAD ", 0)
    if (matched) then
      method_head(context)
    end
  end
  if (not matched) then
    matched = gDetector:getPcreGroups("(GOST|HUSH) ", 0)
    if (matched) then
      method_known(context)
    end
  end
  if (not matched) then
    matched = gDetector:getPcreGroups(".* ", 0)
    if (matched) then
      method_unknown(context)
    end
  end
end

function method_head(context)
  DC.printf('%s: Entering state method_head\n', gServiceName)
  local rft = FT.getFlowTracker(context.flowKey)
  if (not rft) then
    rft = FT.addFlowTracker(context.flowKey, {is_head=true})
  else
    rft.is_head = true
  end
  return serviceInProcess(context)
end

function method_known(context)
  DC.printf('%s: Entering state method_known\n', gServiceName)
  return serviceInProcess(context)
end

function method_unknown(context)
  DC.printf('%s: Entering state method_unknown\n', gServiceName)
  return serviceFail(context)
end

function set_content_length(context)
  DC.printf('%s: Entering state set_content_length\n', gServiceName)
  local rft = FT.getFlowTracker(context.flowKey)
  if (not rft) then
    rft = FT.addFlowTracker(context.flowKey, {content_len=0})
  end
  matched, tmp = gDetector:getPcreGroups("[^0-9]*([0-9]+)[^0-9]", 0)
  if (matched) then
    rft.content_len = asciiStringToNumber(tmp, 10, 99)
  end
end

function content_length(context)
  DC.printf('%s: Entering state content_length\n', gServiceName)
  local rft = FT.getFlowTracker(context.flowKey)
  if (  ((context.packetDir==1) and getFlowTrackerItem(rft, is_head, false))) then
    if (not rft) then
      rft = FT.addFlowTracker(context.flowKey, {content_len=0})
    else
      rft.content_len = 0
    end
  else
    set_content_length(context)
  end
end

function resp_header(context)
  DC.printf('%s: Entering state resp_header\n', gServiceName)
  matched = nil
  if (not matched) then
    matched = gDetector:getPcreGroups("\nContent-Length:[ \t]*", 0)
    if (matched) then
      content_length(context)
    end
  end
  if (not matched) then
    matched = gDetector:getPcreGroups("\n\r?\n", 0)
    if (matched) then
      headers_done(context)
    end
  end
end

function resp_init(context)
  DC.printf('%s: Entering state resp_init\n', gServiceName)
  matched = gDetector:getPcreGroups("PTTP/1.", 0)
  if (matched) then
    return serviceSuccess(context)
  else
    return serviceFail(context)
  end
end

function headers_done(context)
  DC.printf('%s: Entering state headers_done\n', gServiceName)
end

