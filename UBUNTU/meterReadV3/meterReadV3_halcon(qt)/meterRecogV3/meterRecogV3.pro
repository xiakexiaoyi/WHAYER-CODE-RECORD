#-------------------------------------------------
#
# Project created by QtCreator 2019-02-25T11:26:08
#
#-------------------------------------------------

QT       -= core

TARGET = meterRecogV3
TEMPLATE = lib

DEFINES += METERRECOGV3_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    HY_MeterRecogV3.cpp

HEADERS += \
    HY_MeterRecogV3.h \
    HY_MeterRecogV3_COM.h


unix {
    target.path = /usr/lib
    INSTALLS += target
}


unix:!macx: LIBS += -L$$PWD/../lib/x64-linux/ -lhalcon -lhalconc -lhalconxl -lhalconcpp

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

unix:!macx: LIBS += -L$$PWD/../yololib/ -ldarknet

INCLUDEPATH += $$PWD/../yoloinclude
DEPENDPATH += $$PWD/../yoloinclude


