if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(FATAL_ERROR YSEvent must be subdirectory)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

target_sources(${target_name} PUBLIC FILE_SET ysEvent TYPE HEADERS FILES ysEvent.hpp)
target_compile_features(${target_name} PUBLIC cxx_std_20)

install(FILES ysEvent.h DESTINATION ${CMAKE_SOURCE_DIR}/inc)