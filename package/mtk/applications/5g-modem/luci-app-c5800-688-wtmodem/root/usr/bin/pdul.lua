function encodeToPDU(smsc, phoneNumber, message)
    local function TONGen(input, isPhonenum)
        local TONBegin = "91"
        local orinInput = input
        if #input % 2 == 1 then
            input = input .. 'F'
        end
        -- 交换数位
        local transformed = {}
        for i = 1, #input, 2 do
            local firstChar = input:sub(i, i)
            local secondChar = input:sub(i + 1, i + 1)
            transformed[#transformed + 1] = secondChar
            transformed[#transformed + 1] = firstChar
        end
        local TONStr = TONBegin .. table.concat(transformed)
        local TONLength = 0
        if (isPhonenum == false) then
            TONLength = string.len(TONStr) / 2
        else
            TONLength = string.format("%02X", string.len(orinInput))
        end
        if (string.len(TONLength) < 2) then --当短信中心号码过短时，最开头需要补0
            TONLength = "0" .. TONLength
        end
        return TONLength .. TONStr
    end

    local function splitMessage(msg, subLen)
        local segments = {}
        local len = string.len(msg)
        local i = 1
        while i <= len do
            local segment = msg:sub(i, i + subLen - 1)
            segments[#segments + 1] = segment
            i = i + subLen
        end
        return segments
    end
    
    local function generateRandomInt8()
        math.randomseed(os.time())
        local randomInt8 = math.random(0, 255)
        return randomInt8
    end

    local SCA = TONGen(smsc, false)
    local MTI0 = '1'
    local MTI1 = '0'
    local RD = '0'
    local VPF0 = '0'
    local VPF1 = '0'
    local SR = '0'
    local UDHI = '0'
    local RP = '0'
    local pdu
    local TPMR = "00" -- TP-MR 消息基准
    local phoneNumEncode = TONGen(phoneNumber, true) -- DA
    local TPPID = "00" -- TP-PID
    local TPDCS = "08" -- TP-DCS
    local MSG = encodeToUCS2(message) 
    local sendLimit = 60 * 4 ---单条短信字符限制
  
    if string.len(MSG) >= sendLimit then
        UDHI = '1'
    end
    local PDUType = RP .. UDHI .. SR .. VPF1 .. VPF0 .. RD .. MTI1 .. MTI0 -- PDU-Type 

    local decimalValue = tonumber(PDUType, 2)  -- 将二进制字符串转换为十进制数
    PDUType = string.format("%02X", decimalValue)  -- 将十进制数转换为十六进制字符串    
    if (string.len(smsc) == 0) then
        pdu = "00" .. PDUType  .. TPMR .. phoneNumEncode
    else
        pdu = SCA .. PDUType  .. TPMR .. phoneNumEncode
    end

    local sendList = {}
    if string.len(MSG) <= sendLimit then
        local MSGLen = string.format("%02X", string.len(MSG) / 2)
        local AllMsgLen = 7 + string.len(phoneNumEncode) / 2 + string.len(MSG) / 2 - 2
        pdu = AllMsgLen .. '\\r' .. pdu .. TPPID .. TPDCS .. MSGLen .. MSG
        sendList[#sendList + 1] = pdu
    else
        local RefSeq = generateRandomInt8()
        local segments = splitMessage(MSG, sendLimit)
        for i, segment in ipairs(segments) do
            local UDHIHeader = string.format("05%02X%02X%02X%02X%02X", 0, 3, RefSeq, #segments, i) -- 长短信的UDHI头 --05 00 03 85 03 02.
            local MSGLen = string.format("%02X", string.len(segment) / 2 + 6)
            segment = UDHIHeader .. segment
            local AllMsgLen = 7 + string.len(phoneNumEncode) / 2 + string.len(segment) / 2 - 2 
            local currentPdu = AllMsgLen .. '\\r' .. pdu  .. TPPID .. TPDCS .. MSGLen .. segment
            sendList[#sendList + 1] = currentPdu
        end
    end
    return sendList
end

function encodeToUCS2(text)
    local ucs2 = {}
    local index = 1
    local length = string.len(text)

    while index <= length do
        local byte1 = string.byte(text, index)

        if byte1 < 128 then
            ucs2[#ucs2 + 1] = string.format("%04X", byte1)
            index = index + 1
        elseif byte1 >= 192 and byte1 < 224 then
            local byte2 = string.byte(text, index + 1)
            ucs2[#ucs2 + 1] = string.format("%04X", (byte1 - 192) * 64 + (byte2 - 128))
            index = index + 2
        elseif byte1 >= 224 then
            local byte2 = string.byte(text, index + 1)
            local byte3 = string.byte(text, index + 2)
            ucs2[#ucs2 + 1] = string.format("%04X", (byte1 - 224) * 4096 + (byte2 - 128) * 64 + (byte3 - 128))
            index = index + 3
        else
            return nil
        end
    end

    return table.concat(ucs2)
end

-- 获取命令行参数
local smsc = arg[1] or "" -- 短信中心，默认值为空
local phoneNumber = arg[2] -- 目标号码
local message = arg[3] -- 短信内容

if not phoneNumber or not message then
    print("Usage: lua script.lua [smsc] [phoneNumber] [message]")
    return
end

local pdu = encodeToPDU(smsc, phoneNumber, message)
for i, segment in ipairs(pdu) do
    print(segment)
end

