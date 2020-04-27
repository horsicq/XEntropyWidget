QT       += svg

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/dialogentropyprocess.h \
    $$PWD/xentropywidget.h \
    $$PWD/entropyprocess.h

SOURCES += \
    $$PWD/dialogentropyprocess.cpp \
    $$PWD/xentropywidget.cpp \
    $$PWD/entropyprocess.cpp

FORMS += \
    $$PWD/dialogentropyprocess.ui \
    $$PWD/xentropywidget.ui

!contains(XCONFIG, xformats) {
    XCONFIG += xformats
    include(../Formats/xformats.pri)
}

!contains(XCONFIG, xqwt) {
    XCONFIG += xqwt
    include(../XQwt/xqwt.pri)
}
