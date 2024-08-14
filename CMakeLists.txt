cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(pico_dali)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
set(APP_DIRECTORIES_COMMON
        PUBLIC dali
        PUBLIC dali/lib
        )



#add_subdirectory(dali)
#add_subdirectory(dali/lib)

set(APP_SRC
        "main.c"
        "dali/dali.c"
        "dali/lib/dali_addressing.c"
        "dali/lib/dali_commands.c"
        "dali/lib/dali_d4i.c"
        "dali/lib/dali_dexal.c"
        "dali/lib/dali_driver.c"
        "dali/lib/dali_identify.c"
        "dali/lib/dali_LED_Load.c"
        "dali/lib/dali_MemeoryBank.c"
        "dali/lib/dali_power.c"
        "dali/lib/dali_sequences.c"
        "dali/lib/dali_src."
        "dali/lib/dali_temperature.c"
        "dali/lib/manchester.c"
)

add_executable(pico_dali
"main.c"
"dali/dali.c"
"dali/lib/dali_addressing.c"
"dali/lib/dali_commands.c"
"dali/lib/dali_d4i.c"
"dali/lib/dali_dexal.c"
"dali/lib/dali_driver.c"
"dali/lib/dali_identify.c"
"dali/lib/dali_LED_Load.c"
"dali/lib/dali_MemoryBank.c"
"dali/lib/dali_power.c"
"dali/lib/dali_sequences.c"
"dali/lib/dali_sr.c"
"dali/lib/dali_temperature.c"
"dali/lib/manchester.c"
    )

# pull in common dependencies
target_include_directories(pico_dali PUBLIC dali dali/lib)
target_link_libraries(pico_dali pico_stdlib hardware_spi hardware_dma hardware_irq)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(pico_dali pico_cyw43_arch_none)
endif()

pico_enable_stdio_usb(pico_dali 1)
pico_enable_stdio_uart(pico_dali 0)


# create map/bin/hex file etc.

pico_add_extra_outputs(pico_dali)

# add url via pico_set_program_url
#example_auto_set_url(blink_HelloWorldUSB)