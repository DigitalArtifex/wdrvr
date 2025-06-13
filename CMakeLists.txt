cmake_minimum_required(VERSION 3.16)
file(GLOB resource_files *.qrc)
project(wdrvr VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS QuickControls2)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS Positioning)
find_package(Qt6 COMPONENTS Location)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS QuickEffects)
find_package(Qt6 COMPONENTS Graphs)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(wdrvr WIN32 MACOSX_BUNDLE
    main.cpp
    locationmodel.h
    locationmodel.cpp
    iconmodel.h
    iconmodel.cpp
    ${resource_files}
)
target_link_libraries(wdrvr PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::OpenGL
    Qt::Quick
    Qt::QuickControls2
    Qt::Location
    Qt::Positioning
    Qt::Widgets
    Qt::QuickEffects
    Qt::Graphs
)

qt_add_qml_module(appwdrvr
    URI appwdrvr
    VERSION 1.0
    QML_FILES
        Main.qml
        PermissionsScreen.qml
        PlacesMap.qml
        MapLegend.qml
        FloatingMenu.qml
        SettingsPanel.qml
        ChartPage.qml
        LoadingScreen.qml
        SOURCES
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
    "Main.qml"
    "PermissionsScreen.qml"
    "PlacesMap.qml"
    "MapLegend.qml"
    "FloatingMenu.qml"
    "SettingsPanel.qml"
    "ChartPage.qml"
    "LoadingScreen.qml"
    "images/white_textured_wallpaper_sq.jpg"
    "images/white_textured_wallpaper_small.png"
)

qt6_add_resources(wdrvr "wdrvr"
    PREFIX
        "/"
    FILES
        ${wdrvr_resource_files}
)

# qt6_add_resources(icons.qrc)
