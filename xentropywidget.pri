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
    $$PWD/dialogentropyprocess.ui \
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
