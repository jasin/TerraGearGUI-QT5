cmake_minimum_required (VERSION 2.6.4)

project(TerraGearGUI)

FIND_PACKAGE(Qt4 REQUIRED)

SET(QT_USE_QTNETWORK TRUE)

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

SET(terrageargui_SOURCES
	main.cpp
	mainwindow.cpp
	newbucket.cpp
	tggui_utils.cpp
	)
SET(terrageargui_HEADERS
	mainwindow.h
	newbucket.h
	tggui_utils.h
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
