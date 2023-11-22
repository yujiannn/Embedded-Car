add_executable(magnometer magnometer.c)

# pull in common dependencies
target_link_libraries(magnometer pico_stdlib hardware_i2c)

# enable usb output, enable uart output
pico_enable_stdio_usb(magnometer 1)
pico_enable_stdio_uart(magnometer 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(magnometer)

# add url via pico_set_program_url
example_auto_set_url(magnometer)

