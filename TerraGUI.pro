QT          += core widgets
QT          += xmlpatterns
QT          += network

TARGET      = TerraGUI
TEMPLATE    = app

MOC_DIR     = tmp
OBJECTS_DIR = obj
#DESTDIR    = bin

SOURCES     += src/main.cpp\
            src/mainwindow.cpp \
            src/airportsTab.cpp \
            src/constructTab.cpp \
            src/downloadManager.cpp \
            src/downloadTab.cpp \
            src/elevationTab.cpp \
            src/elevationTab.cpp \
            src/materialsTab.cpp \
            src/menu.cpp \
            src/newbucket.cpp \
            src/startTab.cpp \
            src/tggui_utils.cpp \
            src/QMapControl/src/curve.cpp \
            src/QMapControl/src/geometry.cpp \
            src/QMapControl/src/imagemanager.cpp \
            src/QMapControl/src/layer.cpp \
            src/QMapControl/src/layermanager.cpp \
            src/QMapControl/src/linestring.cpp \
            src/QMapControl/src/mapadapter.cpp \
            src/QMapControl/src/mapcontrol.cpp \
            src/QMapControl/src/mapnetwork.cpp \
            src/QMapControl/src/point.cpp \
            src/QMapControl/src/tilemapadapter.cpp \
            src/QMapControl/src/osmmapadapter.cpp \
            src/QMapControl/src/maplayer.cpp \
            src/QMapControl/src/geometrylayer.cpp \
            src/QMapControl/src/wmsmapadapter.cpp

HEADERS     += src/mainwindow.h \
            src/newbucket.h \
            src/tggui_utils.h \
            src/QMapControl/src/curve.h \
            src/QMapControl/src/geometry.h \
            src/QMapControl/src/imagemanager.h \
            src/QMapControl/src/layer.h \
            src/QMapControl/src/layermanager.h \
            src/QMapControl/src/linestring.h \
            src/QMapControl/src/mapadapter.h \
            src/QMapControl/src/mapcontrol.h \
            src/QMapControl/src/mapnetwork.h \
            src/QMapControl/src/point.h \
            src/QMapControl/src/tilemapadapter.h \
            src/QMapControl/src/osmmapadapter.h \
            src/QMapControl/src/maplayer.h \
            src/QMapControl/src/geometrylayer.h \
            src/QMapControl/src/wmsmapadapter.h \
            src/QMapControl/qmapcontrol.h

FORMS       += src/mainwindow.ui

