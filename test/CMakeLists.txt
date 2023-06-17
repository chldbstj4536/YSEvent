cmake_minimum_required(VERSION 3.25)

project(YSEventTest
    LANGUAGES CXX)

# 외부 프로젝트 설치
execute_process(COMMAND ${CMAKE_COMMAND} -S ./.. -B ./../build -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
execute_process(COMMAND ${CMAKE_COMMAND} --build ./../build --target install)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(ysEventTest main.cpp)

target_include_directories(ysEventTest
    PUBLIC ${CMAKE_INSTALL_PREFIX}/inc)

# VS 시작프로젝트 설정
if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ysEventTest)
endif()