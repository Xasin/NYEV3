
include_files = { "*.lua" }

stds.nodemcu = {
	read_globals = {
		"tmr",
		"i2c",
		"mqtt",
		"uart",
		"wifi",
		"adc",
		"node",
		"sjson",
		"file",
		"encoder",
		"sntp",
		"rtctime",

		"homeQTT",
		"subscribeTo",
		"raw_send_fire",
	}
}

std="min+nodemcu"

allow_defined 			= true

ignore = {
	"131", -- Unused global variables
	"212", -- Unused function argument
	"213", -- Unused loop variables
}
