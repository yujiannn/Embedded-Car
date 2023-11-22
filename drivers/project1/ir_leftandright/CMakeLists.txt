add_executable(ir_leftandright
  ir_leftandright.c 
)

# pull in common dependencies
target_link_libraries(ir_leftandright pico_stdlib hardware_pwm hardware_adc pico_time hardware_i2c)

# enable usb output, enable uart output
pico_enable_stdio_usb(ir_leftandright 1)
pico_enable_stdio_uart(ir_leftandright 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(ir_leftandright)

# add url via pico_set_program_url
example_auto_set_url(ir_leftandright)
