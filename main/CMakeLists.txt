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

# Force gcc to rewrite __FILE__ with shorter relative paths.  Since
# CMAKE_SOURCE_DIR is .../main rather than the project root directory, we need
# to remove the /main part to get a match on the long prefix.
#
string(REGEX REPLACE "/main$" "" PARENT_DIR "${CMAKE_SOURCE_DIR}")
add_compile_options(-ffile-prefix-map=${PARENT_DIR}=.)
