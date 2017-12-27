

tmr.create():alarm(2000, tmr.ALARM_SINGLE,
	function(t)
		dofile("Fireworks.lua");
		dofile("MQTT.lua");

		onMQTTConnect(function()

			print("MQTT Connected!");

			currentIgnition = 1;
			homeQTT:publish("NYEv3/Current", 1, 2, 1);

			subscribeTo("NYEv3/Set", 2,
				function(data)
					data = tonumber(data);
					if(data) then
						print("New target number:" .. data);

						currentIgnition = data;
						homeQTT:publish("NYEv3/Current", currentIgnition, 2, 1);
					end
				end);

			subscribeTo("NYEv3/Fire", 2,
				function(data)
					if(data == "YES SIR") then
						print("Readying to fire!");

						raw_send_fire(currentIgnition);

						currentIgnition = currentIgnition +1;
						homeQTT:publish("NYEv3/Current", currentIgnition, 2, 1);
					end
				end);
		end);
	end);
