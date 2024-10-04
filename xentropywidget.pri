#QT       += svg

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/dialogentropyprocess.h \
    $$PWD/xentropywidget.h \
    $$PWD/dialogentropy.h

SOURCES += \
    $$PWD/dialogentropyprocess.cpp \
    $$PWD/xentropywidget.cpp \
    $$PWD/dialogentropy.cpp

FORMS += \
    $$PWD/xentropywidget.ui \
    $$PWD/dialogentropy.ui

!contains(XCONFIG, entropyprocess) {
    XCONFIG += entropyprocess
    include($$PWD/entropyprocess.pri)
}

!contains(XCONFIG, xqwt) {
    XCONFIG += xqwt
    include($$PWD/../XQwt/xqwt.pri)
}

!contains(XCONFIG, xlineedithex) {
    XCONFIG += xlineedithex
    include($$PWD/../Controls/xlineedithex.pri)
}

!contains(XCONFIG, xtableview) {
    XCONFIG += xtableview
    include($$PWD/../Controls/xtableview.pri)
}

!contains(XCONFIG, xshortcuts) {
    XCONFIG += xshortcuts
    include($$PWD/../XShortcuts/xshortcuts.pri)
}

!contains(XCONFIG, xdialogprocess) {
    XCONFIG += xdialogprocess
    include($$PWD/../FormatDialogs/xdialogprocess.pri)
}

DISTFILES += \
    $$PWD/LICENSE \
    $$PWD/README.md \
    $$PWD/xentropywidget.cmake
