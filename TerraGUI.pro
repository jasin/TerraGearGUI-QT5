#-------------------------------------------------
#
# Project created by QtCreator 2010-09-12T19:46:38
#
#-------------------------------------------------

QT          += core gui
QT          += xmlpatterns
QT          += network

TARGET      = TerraGUI
TEMPLATE    = app

MOC_DIR = tmp
OBJECTS_DIR = obj
DESTDIR = bin

SOURCES     += main.cpp\
            mainwindow.cpp \
            newbucket.cpp \
            tggui_utils.cpp \
            QMapControl/src/curve.cpp \
            QMapControl/src/geometry.cpp \
            QMapControl/src/imagemanager.cpp \
            QMapControl/src/layer.cpp \
            QMapControl/src/layermanager.cpp \
            QMapControl/src/linestring.cpp \
            QMapControl/src/mapadapter.cpp \
            QMapControl/src/mapcontrol.cpp \
            QMapControl/src/mapnetwork.cpp \
            QMapControl/src/point.cpp \
            QMapControl/src/tilemapadapter.cpp \
            QMapControl/src/circlepoint.cpp \
            QMapControl/src/osmmapadapter.cpp \
            QMapControl/src/maplayer.cpp \
            QMapControl/src/geometrylayer.cpp \
            QMapControl/src/emptymapadapter.cpp \
            QMapControl/src/wmsmapadapter.cpp

HEADERS     += mainwindow.h \
            newbucket.h \
            tggui_utils.h \
            QMapControl/src/curve.h \
            QMapControl/src/geometry.h \
            QMapControl/src/imagemanager.h \
            QMapControl/src/layer.h \
            QMapControl/src/layermanager.h \
            QMapControl/src/linestring.h \
            QMapControl/src/mapadapter.h \
            QMapControl/src/mapcontrol.h \
            QMapControl/src/mapnetwork.h \
            QMapControl/src/point.h \
            QMapControl/src/tilemapadapter.h \
            QMapControl/src/circlepoint.h \
            QMapControl/src/osmmapadapter.h \
            QMapControl/src/maplayer.h \
            QMapControl/src/geometrylayer.h \
            QMapControl/src/emptymapadapter.h \
            QMapControl/src/wmsmapadapter.h \
            QMapControl/qmapcontrol.h

FORMS       += mainwindow.ui
