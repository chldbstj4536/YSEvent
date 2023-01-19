if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(FATAL_ERROR YSEvent must be subdirectory)
endif()

target_sources(${target_name} PUBLIC FILE_SET HEADERS FILES ysEvent.h)

install(FILES ysEvent.h DESTINATION ${CMAKE_SOURCE_DIR}/inc)