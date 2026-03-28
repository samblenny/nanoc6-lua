idf_component_register(
	SRCS "nanoc6-lua.c"
	INCLUDE_DIRS "."
	REQUIRES
		console
		esp_driver_usb_serial_jtag
		lua-5.4.8
		utf8
		mini_readline
)
