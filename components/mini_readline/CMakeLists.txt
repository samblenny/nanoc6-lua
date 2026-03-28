# components/mini_readline/CMakeLists.txt

idf_component_register(
    SRCS mini_readline.c
    INCLUDE_DIRS "."     # for .h includes
	REQUIRES utf8
)

# Make headers available to other components
target_include_directories(${COMPONENT_LIB} PUBLIC src)

