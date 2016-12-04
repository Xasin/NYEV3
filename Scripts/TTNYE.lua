
ttNYE = 0;

tLeft = {
	days = 0,
	hours = 0,
	minutes = 0,
	seconds = 0
}
tNames = {"days", "hours", "minutes", "seconds"};

autospeakInterval = 30;

function updateTime()
	ttNYE = 1483228799 - (os.time() + 60*60);

	temp = os.date("*t", ttNYE);

	tLeft.days = temp.yday -1;
	tLeft.hours = temp.hour -1;
	tLeft.minutes = temp.min;
	tLeft.seconds = temp.sec;
end

function speakTime()
	updateTime();
	timeString = "Time until New Year: ";

	for v = 1,4 do
		timeString = timeString .. tostring(tLeft[tNames[v]]) .. " " .. tNames[v];
		if(not (v == 4)) then timeString = timeString .. ", "; end
	end

	os.execute("espeak \"" .. timeString .. "\" &");
	print(timeString);
end

function getNextMoment() 
	return math.ceil((os.time() +1)/autospeakInterval) * autospeakInterval;
end

function waitUntil(timestamp)
	rVal, eReason, eNum =  os.execute("sleep " .. tostring(timestamp - os.time()));
	if(not( eNum == 0)) then
		os.exit();
	end
end

function autospeak() 
	while(true) do
		waitUntil(getNextMoment());
		speakTime();
	end
end

autospeak();
	
