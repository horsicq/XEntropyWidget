include_directories(${CMAKE_CURRENT_LIST_DIR})

if (NOT DEFINED ENTROPYPROCESS_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/entropyprocess.cmake)
    set(XENTROPYWIDGET_SOURCES ${XENTROPYWIDGET_SOURCES} ${ENTROPYPROCESS_SOURCES})
endif()
if (NOT DEFINED XQWT_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../XQwt/xqwt.cmake)
    set(XENTROPYWIDGET_SOURCES ${XENTROPYWIDGET_SOURCES} ${XQWT_SOURCES})
endif()
if (NOT DEFINED XLINEEDITHEX_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../Controls/xlineedithex.cmake)
    set(XENTROPYWIDGET_SOURCES ${XENTROPYWIDGET_SOURCES} ${XLINEEDITHEX_SOURCES})
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../Controls/xtableview.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../FormatDialogs/xdialogprocess.cmake)

set(XENTROPYWIDGET_SOURCES
    ${XENTROPYWIDGET_SOURCES}
    ${ENTROPYPROCESS_SOURCES}
    ${XQWT_SOURCES}
    ${XLINEEDITHEX_SOURCES}
    ${XTABLEVIEW_SOURCES}
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
