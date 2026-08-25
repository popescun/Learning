cmake_minimum_required(VERSION 3.16)

project(samegame1 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)

qt_add_executable(samegame1
    main.cpp
)

qt_add_qml_module(samegame1
    URI samegame1
    VERSION 1.0
    QML_FILES
        samegame.qml
        Button.qml
    RESOURCES
        pics/background.jpg
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(samegame1 PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.samegame1
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(samegame1
    PRIVATE Qt6::Quick)

include(GNUInstallDirs)
install(TARGETS samegame1
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
