cmake_minimum_required(VERSION 3.25)

project(DemoYSEvent
        LANGUAGES CXX)

set(target_name DemoYSEvent)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp)
add_executable(${target_name} ${sources})

add_subdirectory(../ ../build/YSEvent)

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target_name})
endif()