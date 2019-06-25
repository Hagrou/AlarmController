
--
-- This script is call each 60 sec by domoticz
-- 



commandArray={}

securityCtrlUrl='#securityCtrlUrl#';

armedDelaySec=#armedDelaySec#;           	-- delay de passage a Armed
alarmDelaySec=#alarmDelaySec#;				-- delay de passage a Alarm
faultDelaySec=#faultDelaySec#;              -- delay to test fault detection and return to Armed
rearmDelaySec=#rearmDelaySec#;              -- delay to rearm system after an Alarm

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

deltaTimeSec=timeDiff(otherdevices_lastupdate['SecurityLevel'])

if (otherdevices["SecurityLevel"] == "Disarmed") then        
	commandArray['Alarm']='Off'
	print("-_- Disarmed")
elseif (otherdevices["SecurityLevel"] == "Arming" and deltaTimeSec>armedDelaySec) then
    commandArray['SecurityLevel']='Set Level 20'    -- Armed
    print("@_@ Armed")
elseif (otherdevices["SecurityLevel"] == "Warn1" and deltaTimeSec>=faultDelaySec) then
    commandArray['SecurityLevel']= "Set Level 20"  -- Armed
    print("^_^' Fault Detection -> Armed")
elseif (otherdevices["SecurityLevel"] == "Warn2" and deltaTimeSec>alarmDelaySec) then
    commandArray['SecurityLevel']= "Set Level 50"  -- Alarm
	commandArray['Alarm']='On'
    print("!>_<! Alarm!")
elseif (otherdevices["SecurityLevel"] == "Alarm" and deltaTimeSec>rearmDelaySec) then
    commandArray['SecurityLevel']= "Set Level 20"  -- Armed
	commandArray['Alarm']='Off'
    print("@_@ Rearming System")
end
return commandArray


