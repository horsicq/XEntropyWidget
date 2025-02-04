include_directories(${CMAKE_CURRENT_LIST_DIR})

if (NOT DEFINED XFORMATS_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../Formats/xformats.cmake)
    set(ENTROPYPROCESS_SOURCES ${ENTROPYPROCESS_SOURCES} ${XFORMATS_SOURCES})
endif()

set(ENTROPYPROCESS_SOURCES
    ${ENTROPYPROCESS_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/entropyprocess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/entropyprocess.h
    )
