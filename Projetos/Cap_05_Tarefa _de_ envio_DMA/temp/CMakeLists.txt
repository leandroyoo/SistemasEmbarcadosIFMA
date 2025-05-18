cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --------------------------------------------------------------------
# Bloco VS Code – permanece igual
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# --------------------------------------------------------------------

set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)

project(temp_mda C CXX ASM)
pico_sdk_init()

add_executable(temp_mda
    temp_mda.c
    inc/ssd1306_i2c.c          # ← driver I2C/SSD1306
    # inc/ssd1306.c            # se existir outro arquivo
)

pico_set_program_name    (temp_mda "temp_mda")
pico_set_program_version (temp_mda "0.1")

pico_enable_stdio_uart(temp_mda 0)
pico_enable_stdio_usb (temp_mda 1)

# Headers do seu projeto
target_include_directories(temp_mda PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/inc    
)

# Bibliotecas do SDK de que você precisa
target_link_libraries(temp_mda
    pico_stdlib
    hardware_i2c
    hardware_dma
    hardware_adc
)

pico_add_extra_outputs(temp_mda)
