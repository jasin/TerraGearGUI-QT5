cmake_minimum_required (VERSION 2.6.4)

project(TerraGUI)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
#FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
#INCLUDE(${QT_USE_FILE})


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

if(WIN32)
        set(CMAKE_CXX_FLAGS "/DWIN32 /EHsc /D_CRT_SECURE_NO_WARNINGS /MT")
endif(WIN32)

add_executable(TerraGUI 
	${TerraGUI_SOURCES}
	)
target_link_libraries(TerraGUI
	#${QT_LIBRARIES}
	Qt5::Widgets
	Qt5::Network
	)

install(TARGETS TerraGUI RUNTIME DESTINATION bin)
