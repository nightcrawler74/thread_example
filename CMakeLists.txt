cmake_minimum_required(VERSION 3.12)


include(pico_sdk_import.cmake)

project(thread C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})


pico_sdk_init()


add_executable(thread
		main.cpp
		)

	target_link_libraries(thread pico_stdlib hardware_pio hardware_adc pico_multicore)

	pico_add_extra_outputs(thread)


#	pico_generate_pio_header(led ${CMAKE_CURRENT_LIST_DIR}/led.pio)
