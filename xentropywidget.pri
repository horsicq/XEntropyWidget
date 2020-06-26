QT       += svg

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/dialogentropyprocess.h \
    $$PWD/xentropywidget.h \
    $$PWD/entropyprocess.h \
    $$PWD/dialogentropy.h

SOURCES += \
    $$PWD/dialogentropyprocess.cpp \
    $$PWD/xentropywidget.cpp \
    $$PWD/entropyprocess.cpp \
    $$PWD/dialogentropy.cpp

FORMS += \
    $$PWD/dialogentropyprocess.ui \
    $$PWD/xentropywidget.ui \
    $$PWD/dialogentropy.ui

!contains(XCONFIG, xformats) {
    XCONFIG += xformats
    include($$PWD/../Formats/xformats.pri)
}

!contains(XCONFIG, xqwt) {
    XCONFIG += xqwt
    include($$PWD/../XQwt/xqwt.pri)
}

!contains(XCONFIG, xlineedithex) {
    XCONFIG += xlineedithex
    include($$PWD/../Controls/xlineedithex.pri)
}
