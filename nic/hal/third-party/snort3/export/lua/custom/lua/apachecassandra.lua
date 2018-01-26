--[[
detection_name: ApacheCassandra
version: 1
description: ApacheCassandra
--]]

require "DetectorCommon"

local DC = DetectorCommon
local proto = DC.ipproto.tcp;
local HT = hostServiceTrackerModule
local FT = flowTrackerModule

gServiceName = "cassandra"

gPorts = {
    {DC.ipproto.tcp, 9042},
}

--patterns used in DetectorInit()
gPatterns = {       
    --patternName        Pattern                                  offset
    --------------------------------------------------------------------
    --cli      = {'COMPRESSION', 0, gAppId },
    --srv      = {'\006', 4, gAppId },
    cli      = {'\004\000\000\000\005', 0, gAppId },
    srv      = {'\132\000\000\000\006', 0, gAppId },
}

gFastPatterns = {
    --protocol patternName
    ------------------------------------
    {DC.ipproto.tcp, gPatterns.cli},
}

gAppRegistry = {
	--AppIdValue          Extracts Info
	---------------------------------------
	{gAppId,		         0}
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
    print('Detected, packetCount: ', context.packetCount);
    return DC.serviceStatus.success
end

function serviceFail(context)
    local flowFlag = context.detectorFlow:getFlowFlag(DC.flowFlags.serviceDetected)

    if ((not flowFlag) or (flowFlag == 0)) then
        gDetector:failService()
    end

    context.detectorFlow:clearFlowFlag(DC.flowFlags.continue)
    DC.printf('%s: Failed, packetCount: %d\n', gServiceName, context.packetCount);
    print('%s: Failed, packetCount: %d\n', gServiceName, context.packetCount);
    return DC.serviceStatus.nomatch
end

function registerPortsPatterns()

    --register port based detection
    for i,v in ipairs(gPorts) do
        gDetector:addPort(v[1], v[2])
        print(gServiceName .. ': registering port ' .. v[2]);
    end

    --register pattern based detection
    for i,v in ipairs(gFastPatterns) do
        if ( gDetector:registerPattern(v[1], v[2][1], #v[2][1], v[2][2], gAppId) ~= 0) then
            print (gServiceName .. ': register pattern failed for ' .. v[2])
        else
            print (gServiceName .. ': register pattern successful for ' .. i)
        end
    end
end

function DetectorInit(detectorInstance)

  gDetector = detectorInstance;
  gAppId = gDetector:open_createApp(gServiceName);

  gDetector:init(gServiceName, 'DetectorValidator', 'DetectorFini')
  registerPortsPatterns()

  print("Done registration for ApacheCassandra custom appid: ", gAppId);
  return gDetector;
end

function DetectorFini()
    --print (gServiceName .. ': DetectorFini()')
end

function DetectorClean()
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
    local size = context.packetDataLen
    local dir = context.packetDir
    local srcPort = context.srcPort
    local dstPort = context.dstPort
    local flowKey = context.flowKey
    local rft = FT.getFlowTracker(flowKey)

    print(gServiceName .. ':DetectorValidator(): packetCount ' .. context.packetCount .. ', dir ' .. dir .. ', size ' .. size);
 
    if (size == 0 or dir == 0) then
        return serviceInProcess(context)
    end

    if (not rft) then
        rft = FT.addFlowTracker(flowKey, {server_received=0})
        print('DetectorValidator(): adding FT entry');
    end

    -- seen the client request, now look for server to say "COMPRESSION"
    if (dir == 1 and 
        rft.server_received == 0 and 
        gDetector:memcmp(gPatterns.srv[1], #gPatterns.srv[1], gPatterns.srv[2]) == 0) then
        rft.server_received = 1
        print ('DetectorValidator: application detected at packetCount ', context.packetCount, 'dir: ', dir);
        FT.delFlowTracker(flowKey)
        return serviceSuccess(context)    
    end

    return serviceFail(context)
end 

DetectorPackageInfo = {
    name = gServiceName,
    proto = proto,
    server = {
        init =  'DetectorInit',
        validate =  'DetectorValidator',
        minimum_matches = 1
    }
}
