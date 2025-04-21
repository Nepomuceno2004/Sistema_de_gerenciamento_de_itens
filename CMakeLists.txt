# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Sistema_de_gerenciamento_de_itens C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Sistema_de_gerenciamento_de_itens 
    Sistema_de_gerenciamento_de_itens.c 
    lib/ssd1306.c
    )

pico_set_program_name(Sistema_de_gerenciamento_de_itens "Sistema_de_gerenciamento_de_itens")
pico_set_program_version(Sistema_de_gerenciamento_de_itens "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Sistema_de_gerenciamento_de_itens 1)
pico_enable_stdio_usb(Sistema_de_gerenciamento_de_itens 1)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(Sistema_de_gerenciamento_de_itens ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)


# Add the standard library to the build
target_link_libraries(Sistema_de_gerenciamento_de_itens
        pico_stdlib
        hardware_pio
        hardware_pwm
        hardware_i2c
        hardware_adc
        )

# Add the standard include files to the build
target_include_directories(Sistema_de_gerenciamento_de_itens PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(Sistema_de_gerenciamento_de_itens)

