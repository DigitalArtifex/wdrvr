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

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(wdrvr-app WIN32 MACOSX_BUNDLE
    main.cpp
    locationmodel.h
    locationmodel.cpp
    ${resource_files}
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
    URI appwdrvr
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
    # "icons/theme/scalable/arrow-compass-maps-2.svg"
    # "icons/theme/scalable/arrow-compass-maps.svg"
    # "icons/theme/scalable/arrow-direction-maps-2.svg"
    # "icons/theme/scalable/arrow-direction-maps-3.svg"
    # "icons/theme/scalable/arrow-direction-maps.svg"
    # "icons/theme/scalable/compass-direction-gps-2.svg"
    # "icons/theme/scalable/compass-direction-gps.svg"
    # "icons/theme/scalable/deadlock-gps-maps.svg"
    # "icons/theme/scalable/direction-globe-location.svg"
    # "icons/theme/scalable/direction-gps-maps-2.svg"
    # "icons/theme/scalable/direction-gps-maps-3.svg"
    # "icons/theme/scalable/direction-gps-maps-4.svg"
    # "icons/theme/scalable/direction-gps-maps-5.svg"
    # "icons/theme/scalable/direction-gps-maps-6.svg"
    # "icons/theme/scalable/direction-gps-maps.svg"
    # "icons/theme/scalable/direction-maps-navigation.svg"
    # "icons/theme/scalable/globe-gps-location.svg"
    # "icons/theme/scalable/gps-location-maps-2.svg"
    # "icons/theme/scalable/gps-location-maps-3.svg"
    # "icons/theme/scalable/gps-location-maps-4.svg"
    # "icons/theme/scalable/gps-location-maps-5.svg"
    # "icons/theme/scalable/gps-location-maps-6.svg"
    # "icons/theme/scalable/gps-location-maps-7.svg"
    # "icons/theme/scalable/gps-location-maps-8.svg"
    # "icons/theme/scalable/gps-location-maps-9.svg"
    # "icons/theme/scalable/gps-location-maps-10.svg"
    # "icons/theme/scalable/gps-location-maps.svg"
    # "icons/theme/scalable/location-maps-navigation-2.svg"
    # "icons/theme/scalable/location-maps-navigation-3.svg"
    # "icons/theme/scalable/location-maps-navigation.svg"
    # "icons/theme/scalable/mimetypes/ai-document.svg"
    # "icons/theme/scalable/mimetypes/attachment-document.svg"
    # "icons/theme/scalable/mimetypes/audio-document.svg"
    # "icons/theme/scalable/mimetypes/box-notes.svg"
    # "icons/theme/scalable/mimetypes/csv-document.svg"
    # "icons/theme/scalable/mimetypes/eps-document.svg"
    # "icons/theme/scalable/mimetypes/excel-document.svg"
    # "icons/theme/scalable/mimetypes/exe-document.svg"
    # "icons/theme/scalable/mimetypes/flash-document.svg"
    # "icons/theme/scalable/mimetypes/folder.svg"
    # "icons/theme/scalable/mimetypes/gdoc-document.svg"
    # "icons/theme/scalable/mimetypes/gdocs-document.svg"
    # "icons/theme/scalable/mimetypes/gform-document.svg"
    # "icons/theme/scalable/mimetypes/gpres-document.svg"
    # "icons/theme/scalable/mimetypes/gsheet-document.svg"
    # "icons/theme/scalable/mimetypes/html-document.svg"
    # "icons/theme/scalable/mimetypes/image-document.svg"
    # "icons/theme/scalable/mimetypes/keynote-document.svg"
    # "icons/theme/scalable/mimetypes/library-folder.svg"
    # "icons/theme/scalable/mimetypes/link.svg"
    # "icons/theme/scalable/mimetypes/mp4-document.svg"
    # "icons/theme/scalable/mimetypes/overlay-document.svg"
    # "icons/theme/scalable/mimetypes/pack-document.svg"
    # "icons/theme/scalable/mimetypes/pages-document.svg"
    # "icons/theme/scalable/mimetypes/pdf-document.svg"
    # "icons/theme/scalable/mimetypes/ppt-document.svg"
    # "icons/theme/scalable/mimetypes/psd-document.svg"
    # "icons/theme/scalable/mimetypes/quip-doc.svg"
    # "icons/theme/scalable/mimetypes/quip-sheet-document.svg"
    # "icons/theme/scalable/mimetypes/quip-slide-document.svg"
    # "icons/theme/scalable/mimetypes/rtf-document.svg"
    # "icons/theme/scalable/mimetypes/slide-document.svg"
    # "icons/theme/scalable/mimetypes/stypi-document.svg"
    # "icons/theme/scalable/mimetypes/txt-document.svg"
    # "icons/theme/scalable/mimetypes/unknown-document.svg"
    # "icons/theme/scalable/mimetypes/video-document.svg"
    # "icons/theme/scalable/mimetypes/visio-document.svg"
    # "icons/theme/scalable/mimetypes/webex-document.svg"
    # "icons/theme/scalable/mimetypes/word-document.svg"
    # "icons/theme/scalable/mimetypes/xml-document.svg"
    # "icons/theme/scalable/mimetypes/zip-document.svg"
    # "icons/theme/index.theme"
)

qt6_add_resources(wdrvr-app "wdrvr-app"
    PREFIX
        "/"
    FILES
        ${wdrvr_resource_files}
)

# qt6_add_resources(icons.qrc)
