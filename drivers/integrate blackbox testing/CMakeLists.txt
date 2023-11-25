add_executable(integrate
  integrate.c 
)

# pull in common dependencies
target_link_libraries(integrate pico_stdlib hardware_pwm hardware_adc pico_time hardware_i2c)

# enable usb output, enable uart output
pico_enable_stdio_usb(integrate 1)
pico_enable_stdio_uart(integrate 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(integrate)

# add url via pico_set_program_url
example_auto_set_url(integrate)
