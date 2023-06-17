cmake_minimum_required(VERSION 3.25)

project(YSEvent
    LANGUAGES CXX)

execute_process(COMMAND ${CMAKE_COMMAND} -S ${CMAKE_CURRENT_SOURCE_DIR}/submodules/YSDefine -B ${CMAKE_CURRENT_BINARY_DIR}/YSDefine -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/YSDefine --target install)

install(FILES ysEvent.hpp
    DESTINATION ${CMAKE_INSTALL_PREFIX}/inc)