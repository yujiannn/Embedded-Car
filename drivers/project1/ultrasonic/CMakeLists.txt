if (PICO_ON_DEVICE)
    add_executable(ultrasonic
            ultrasonic.c)

    # Disable SDK alarm support for this lowlevel example
    set(PICO_TIME_DEFAULT_ALARM_POOL_DISABLED 1)

    target_link_libraries(ultrasonic pico_stdlib)

    # create map/bin/hex file etc.
    pico_add_extra_outputs(ultrasonic)

    # add url via pico_set_program_url
    example_auto_set_url(ultrasonic)
endif ()

# target_link_libraries(ultrasonic
#             pico_stdlib
#             #FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
#             hardware_adc
#             pico_multicore
            
#             )
target_link_libraries(ultrasonic 
            pico_stdlib 
            hardware_timer 
            hardware_gpio)
# create map/bin/hex file etc.
pico_add_extra_outputs(ultrasonic)

# add url via pico_set_program_url
example_auto_set_url(ultrasonic)
pico_enable_stdio_usb(ultrasonic 1)

