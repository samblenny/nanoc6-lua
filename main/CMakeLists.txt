idf_component_register(
	SRCS "nanoc6-lua.c" "mini_readline.c"
	INCLUDE_DIRS "."
	REQUIRES console esp_driver_usb_serial_jtag lua-5.4.8
)
