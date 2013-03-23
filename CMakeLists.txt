cmake_minimum_required (VERSION 2.6.4)

project(TerraGUI)

FIND_PACKAGE(Qt4 REQUIRED)

SET(QT_USE_QTNETWORK TRUE)

INCLUDE(${QT_USE_FILE})

SET(TerraGUI_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/newbucket.cpp
        src/tggui_utils.cpp
        src/QMapControl/src/curve.cpp
        src/QMapControl/src/geometry.cpp
        src/QMapControl/src/imagemanager.cpp
        src/QMapControl/src/layer.cpp
        src/QMapControl/src/layermanager.cpp
        src/QMapControl/src/linestring.cpp
        src/QMapControl/src/mapadapter.cpp
        src/QMapControl/src/mapcontrol.cpp
        src/QMapControl/src/mapnetwork.cpp
        src/QMapControl/src/point.cpp
        src/QMapControl/src/tilemapadapter.cpp
        src/QMapControl/src/osmmapadapter.cpp
        src/QMapControl/src/maplayer.cpp
        src/QMapControl/src/geometrylayer.cpp
        src/QMapControl/src/wmsmapadapter.cpp
        )
SET(TerraGUI_HEADERS
        src/mainwindow.h
        src/QMapControl/src/curve.h
        src/QMapControl/src/geometry.h
        src/QMapControl/src/imagemanager.h
        src/QMapControl/src/layer.h
        src/QMapControl/src/layermanager.h
        src/QMapControl/src/linestring.h
        src/QMapControl/src/mapadapter.h
        src/QMapControl/src/mapcontrol.h
        src/QMapControl/src/mapnetwork.h
        src/QMapControl/src/point.h
        src/QMapControl/src/tilemapadapter.h
        src/QMapControl/src/osmmapadapter.h
        src/QMapControl/src/maplayer.h
        src/QMapControl/src/geometrylayer.h
        )
SET(TerraGUI_FORMS
        src/mainwindow.ui
        )

QT4_WRAP_CPP(TerraGUI_HEADERS_MOC ${TerraGUI_HEADERS})
QT4_WRAP_UI(TerraGUI_FORMS_HEADERS ${TerraGUI_FORMS})

INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )

add_executable(TerraGUI 
	${TerraGUI_SOURCES}
	${TerraGUI_HEADERS_MOC}
	${TerraGUI_FORMS_HEADERS}
	)
target_link_libraries(TerraGUI
	${QT_LIBRARIES}
	)

install(TARGETS TerraGUI RUNTIME DESTINATION bin)
