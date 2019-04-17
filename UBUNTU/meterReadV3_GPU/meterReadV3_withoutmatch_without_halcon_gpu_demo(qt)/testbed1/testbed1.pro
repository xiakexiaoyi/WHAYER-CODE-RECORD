TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

#unix:!macx: LIBS += -L$$PWD/../build-meterRecogV3-unknown-Debug/ -lmeterRecogV3

#INCLUDEPATH += $$PWD/../meterRecogV3
#DEPENDPATH += $$PWD/../meterRecogV3

unix:!macx: LIBS += -L$$PWD/../../../../../usr/local/lib/ -lopencv_core -lopencv_imgproc -lopencv_highgui

INCLUDEPATH += $$PWD/../../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../../usr/local/include

#unix:!macx: LIBS += -L$$PWD/../lib/x64-linux/ -lhalcon -lhalconcpp

#INCLUDEPATH += $$PWD/../include
#DEPENDPATH += $$PWD/../include

unix:!macx: LIBS += -L$$PWD/../yololib/ -ldarknetGPU -lcudnn

INCLUDEPATH += $$PWD/../yoloinclude
DEPENDPATH += $$PWD/../yoloinclude


