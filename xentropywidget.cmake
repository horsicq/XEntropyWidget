include_directories(${CMAKE_CURRENT_LIST_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/entropyprocess.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XQwt/xqwt.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../Controls/xlineedithex.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../FormatDialogs/xdialogprocess.cmake)

set(XENTROPYWIDGET_SOURCES
    ${ENTROPYPROCESS_SOURCES}
    ${XQWT_SOURCES}
    ${XLINEEDITHEX_SOURCES}
    ${XDIALOGPROCESS_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/dialogentropy.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dialogentropy.h
    ${CMAKE_CURRENT_LIST_DIR}/dialogentropy.ui
    ${CMAKE_CURRENT_LIST_DIR}/dialogentropyprocess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dialogentropyprocess.h
    ${CMAKE_CURRENT_LIST_DIR}/xentropywidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xentropywidget.h
    ${CMAKE_CURRENT_LIST_DIR}/xentropywidget.ui
)
