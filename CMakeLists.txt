cmake_minimum_required(VERSION 3.16)

project(wdrvr VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS QuickControls2)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS Positioning)
find_package(Qt6 COMPONENTS Location)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(wdrvr-app WIN32 MACOSX_BUNDLE
    main.cpp

    locationmodel.h locationmodel.cpp
    icons/icons8-god-of-war-64.png
)
target_link_libraries(wdrvr-app PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::OpenGL
    Qt::Quick
    Qt::QuickControls2
    Qt::Location
    Qt::Positioning
)

qt_add_qml_module(appwdrvr
    URI wdrvr
    VERSION 1.0
    QML_FILES
        Main.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appwdrvr PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appwdrvr
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS appwdrvr
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Resources:
set(wdrvr_resource_files
    "marker.png"
    "Main.qml"
    "PermissionsScreen.qml"
    "PlacesMap.qml"
)

qt6_add_resources(wdrvr-app "wdrvr"
    PREFIX
        "/"
    FILES
        ${wdrvr_resource_files}
)
