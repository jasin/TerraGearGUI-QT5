cmake_minimum_required (VERSION 2.6.4)

project(TerraGearGUI)

FIND_PACKAGE(Qt4 REQUIRED)

SET(QT_USE_QTNETWORK TRUE)

INCLUDE(${QT_USE_FILE})

SET(terrageargui_SOURCES
        main.cpp
        mainwindow.cpp
        newbucket.cpp
        tggui_utils.cpp
        QMapControl/src/curve.cpp
        QMapControl/src/geometry.cpp
        QMapControl/src/imagemanager.cpp
        QMapControl/src/layer.cpp
        QMapControl/src/layermanager.cpp
        QMapControl/src/linestring.cpp
        QMapControl/src/mapadapter.cpp
        QMapControl/src/mapcontrol.cpp
        QMapControl/src/mapnetwork.cpp
        QMapControl/src/point.cpp
        QMapControl/src/tilemapadapter.cpp
        QMapControl/src/circlepoint.cpp
        QMapControl/src/osmmapadapter.cpp
        QMapControl/src/maplayer.cpp
        QMapControl/src/geometrylayer.cpp
        QMapControl/src/emptymapadapter.cpp
        QMapControl/src/wmsmapadapter.cpp
        )
SET(terrageargui_HEADERS
        mainwindow.h
        QMapControl/src/curve.h
        QMapControl/src/geometry.h
        QMapControl/src/imagemanager.h
        QMapControl/src/layer.h
        QMapControl/src/layermanager.h
        QMapControl/src/linestring.h
        QMapControl/src/mapadapter.h
        QMapControl/src/mapcontrol.h
        QMapControl/src/mapnetwork.h
        QMapControl/src/point.h
        QMapControl/src/tilemapadapter.h
        QMapControl/src/osmmapadapter.h
        QMapControl/src/maplayer.h
        QMapControl/src/geometrylayer.h
        QMapControl/src/emptymapadapter.h
        )
SET(terrageargui_FORMS
        mainwindow.ui
        )

QT4_WRAP_CPP(terrageargui_HEADERS_MOC ${terrageargui_HEADERS})
QT4_WRAP_UI(terrageargui_FORMS_HEADERS ${terrageargui_FORMS})

add_executable(terrageargui 
	${terrageargui_SOURCES}
	${terrageargui_HEADERS_MOC}
	${terrageargui_FORMS_HEADERS}
	)
target_link_libraries(terrageargui
	${QT_LIBRARIES}
	)

install(TARGETS terrageargui RUNTIME DESTINATION bin)
