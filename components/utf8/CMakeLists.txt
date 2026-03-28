# components/utf8/CMakeLists.txt

idf_component_register(
    SRCS utf8.c
    INCLUDE_DIRS "."     # for .h includes
)

# Make headers available to other components
target_include_directories(${COMPONENT_LIB} PUBLIC src)

