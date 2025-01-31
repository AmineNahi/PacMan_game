TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
include  (mingl/mingl.pri)
SOURCES += \
    Correc_Prof/game.cpp

DISTFILES += \
    Nos_fichiers/config.yaml

HEADERS += \
    Correc_Prof/game.h \
    Correc_Prof/type.h
