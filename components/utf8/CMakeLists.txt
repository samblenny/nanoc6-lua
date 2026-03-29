# see instructions at
# https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32c6/api-guides/unit-tests.html

idf_component_register(
    SRCS "."
    INCLUDE_DIRS "."
	REQUIRES unity
)
