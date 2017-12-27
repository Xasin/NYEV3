
i2c.setup(0, 1, 2, i2c.SLOW);

function raw_send_fire(n)
	i2c.start(0);
	i2c.address(0, 0x66, i2c.TRANSMITTER);

	i2c.write(0, 0x66, n);

	i2c.stop(0);
end
