--[[
detection_name: mongodb
version: 1
description: MongoDB
--]]

require "DetectorCommon"
local DC = DetectorCommon
local proto = DC.ipproto.tcp;
local FT = flowTrackerModule

gServiceName = "mongodb"

gPorts = {
    {DC.ipproto.tcp, 27017},
}

--patterns used in DetectorInit()
gPatterns = {
    --patternName Pattern offset
    --------------------------------------------------------------------
    --srv = {'ismaster', 0, gAppId},
    --cli = {'isMaster', 0, gAppId},
    op_query = {'\212\007\000\000', 12, gAppId},
    op_reply = {'\001\000\000\000', 12, gAppId},
}

gFastPatterns = {
    --protocol patternName
    ------------------------------------
    --{DC.ipproto.tcp, gPatterns.srv},
    {DC.ipproto.tcp, gPatterns.op_query},
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
        --if ( gDetector:registerPattern(v[1], v[2][1], #v[2][1], v[2][2], gAppId) ~= 0) then
        if ( gDetector:service_registerPattern(v[1], v[2][1], #v[2][1], v[2][2]) ~= 0) then
          print (gServiceName .. ': register pattern failed for ' .. i)
        else
          print (gServiceName .. ': register pattern successful for ' .. i)
        end
    end

    --for i,v in ipairs(gAppRegistry) do
    --    pcall(function () gDetector:registerAppId(v[1],v[2]) end)
    --end
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

    -- seen the client request, now look for server response
    if (dir == 1 and size >= 16 and
        rft.server_received == 0 and
        gDetector:memcmp(gPatterns.op_reply[1], #gPatterns.op_reply[1], gPatterns.op_reply[2]) == 0) then
        rft.server_received = 1
        print ('DetectorValidator: application detected at packetCount ', context.packetCount, 'dir: ', dir);
        FT.delFlowTracker(flowKey)
        return serviceSuccess(context)    
    end

    return serviceFail(context)
end 


function DetectorInit(detectorInstance)

	gDetector = detectorInstance;
	gAppId = gDetector:open_createApp("mongodb");

	gDetector:init(gServiceName, 'DetectorValidator', 'DetectorFini')
        registerPortsPatterns()
	--if gDetector.addPortPatternClient then
	--	gDetector:addPortPatternClient(proto, "MongoDB", 0, gAppId);
	--end

	--if gDetector.addHostPortApp then
	--    print("calling hostport for mongo appid new version: ", gAppId);
	--    retval = gDetector:addHostPortApp(0, gAppId, "127.0.0.1", 27017, DC.ipproto.tcp);
	--    retval = gDetector:addHostPortApp(0, gAppId, "64.0.0.2", 27017, DC.ipproto.tcp);
	--end

	print("Done registration for mongo appid new version: ", gAppId);
	return gDetector;
end

function DetectorFini()
    --print (gServiceName .. ': DetectorFini()')
end

function DetectorClean()
end



DetectorPackageInfo = {
	name = gServiceName,
	proto = proto,
	server = {
		init = 'DetectorInit',
		clean = 'DetectorClean',
		validate =  'DetectorValidator',
		minimum_matches = 1
	}
}
