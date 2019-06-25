--
-- Pass to Warning Security Level
-- 

commandArray={}

securityCtrlUrl='#securityCtrlUrl#'
armedDelaySec=#armedDelaySec#;           	-- delay de passage a Armed
alarmDelaySec=#alarmDelaySec#;				-- delay de passage a Alarm
faultDelaySec=#faultDelaySec#;              -- delay to test fault detection and return to Armed
rearmDelaySec=#rearmDelaySec#;              -- delay to rearm system after an Alarm
blindDelaySec=#blindDelaySec#;

function timeDiff(timeStr)
  year = string.sub(timeStr, 1, 4)
  month = string.sub(timeStr, 6, 7)
  day = string.sub(timeStr, 9, 10)
  hour = string.sub(timeStr, 12, 13)
  minutes = string.sub(timeStr, 15, 16)
  seconds = string.sub(timeStr, 18, 19)
  t1 = os.time()
  t2 = os.time{year=year, month=month, day=day, hour=hour, min=minutes, sec=seconds}
  return os.difftime (t1, t2)
end

-- print("**** Running Security2Warning.lua")
currentEvent=''

function isSomethingAppening()
    local securityEvent=false
    
    if  (otherdevices['DetecteurMouvementEntree'] == "On" and timeDiff(otherdevices_lastupdate['DetecteurMouvementEntree'])>blindDelaySec) then
       currentEvent=currentEvent..(" DetecteurMouvementEntree ON")
       securityEvent=true
    end
    return securityEvent
end

if (not isSomethingAppening()) then
    return commandArray
end

deltaTimeSec=timeDiff(otherdevices_lastupdate['SecurityLevel'])

if (otherdevices["SecurityLevel"] == "Armed") then
    print("*_* Something Appens => Warn1 ["..currentEvent.."]")
    commandArray['SecurityLevel']='Set Level 30' -- Warn1
elseif (otherdevices["SecurityLevel"] == "Warn1" and deltaTimeSec<faultDelaySec) then
    print("O_O Something Appens  => Warn2 ["..currentEvent.."]")
    commandArray['SecurityLevel']='Set Level 40' -- Warn2
end

return commandArray


