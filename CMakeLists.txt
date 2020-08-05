cmake_minimum_required (VERSION 3.1)

project(QtWAW)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools WebEngineWidgets)
find_package(KF5DBusAddons)
find_package(KF5Notifications)
find_package(KF5NetworkManagerQt)

SET(SRCS
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/webenginepage.cpp
    src/webenginepage.h
)

file(GLOB TS_FILES
     LIST_DIRECTORIES false
     languages/qtwaw_*.ts
)

qt5_create_translation(QM_FILES ${SRCS} ${TS_FILES} OPTIONS -no-obsolete)

add_executable(
    qtwaw
    ${SRCS}
    ${QM_FILES}
)

target_link_libraries(
    qtwaw
    ${Qt5Widgets_LIBRARIES}
    ${Qt5WebEngineWidgets_LIBRARIES}
    KF5::Notifications
    KF5::DBusAddons
    KF5::NetworkManagerQt
)

install(TARGETS qtwaw RUNTIME DESTINATION bin)
install(FILES resources/loading.html DESTINATION share/qtwaw)
install(FILES resources/eu.scarpetta.QtWAW.desktop DESTINATION share/applications)
install(FILES resources/icon.svg DESTINATION share/icons/hicolor/scalable/apps RENAME eu.scarpetta.QtWAW.svg)
install(FILES resources/icon_48.png DESTINATION share/icons/hicolor/48x48/apps RENAME eu.scarpetta.QtWAW.png)
install(FILES resources/icon_64.png DESTINATION share/icons/hicolor/64x64/apps RENAME eu.scarpetta.QtWAW.png)
install(FILES resources/icon_128.png DESTINATION share/icons/hicolor/128x128/apps RENAME eu.scarpetta.QtWAW.png)
install(FILES resources/icon_256.png DESTINATION share/icons/hicolor/256x256/apps RENAME eu.scarpetta.QtWAW.png)
install(FILES ${QM_FILES} DESTINATION share/qtwaw/translations)
install(FILES resources/qtwaw.notifyrc DESTINATION share/knotifications5)

add_custom_command(
    TARGET qtwaw
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR} -P cmake_install.cmake
)
