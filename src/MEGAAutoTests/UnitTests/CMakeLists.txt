cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0091 NEW)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui Network Qml Quick QuickWidgets)

#-------------- MEGA Sync unit tests --------------------

add_executable(UnitTests)

set(CMAKE_AUTOUIC ON)

set_target_properties(UnitTests
    PROPERTIES
    AUTOUIC ON # Activates the User Interface Compiler generator for Qt.
    AUTOMOC ON # Activates the meta-object code generator for Qt.
)

target_compile_definitions(UnitTests
    PUBLIC
    $<$<BOOL:${WIN32}>:PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN UNICODE>
    $<$<BOOL:${ENABLE_ISOLATED_GFX}>:ENABLE_SDK_ISOLATED_GFX>
)

set(UNIT_TEST_FILES
    AllMegaIncludes.h
    main.cpp
    MegaCatchReporter.cpp MegaCatchReporter.h
    MegaCatchReporterUtilities.cpp MegaCatchReporterUtilities.h
    ScaleFactorManagerTestFixture.cpp ScaleFactorManagerTestFixture.h
    StringConversions.h
    ScaleFactorManagerTests.cpp
    control/TransferBatchTests.cpp
    control/TransferRemainingTimeTests.cpp
    control/UtilitiesTests.cpp
)

if (WIN32)
    find_package(Qt5 REQUIRED COMPONENTS WinExtras)
elseif (APPLE)
    find_package(Qt5 REQUIRED COMPONENTS MacExtras Svg)
else()
    find_package(Qt5 REQUIRED COMPONENTS Svg)
endif()

set_property(TARGET UnitTests
    PROPERTY AUTOUIC_SEARCH_PATHS
    ${MegaSyncDir}/gui/linux ${MegaSyncDir}/gui/node_selector/gui/linux ${MegaSyncDir}/gui/ui ${MegaSyncDir}/gui
)

target_sources(UnitTests
    PRIVATE
    ${UNIT_TEST_FILES}
    ${MEGA_DESKTOP_APP_SOURCES}
)

target_include_directories(UnitTests PRIVATE ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/google_breakpad
)

set(ExecutableTarget UnitTests)
set(DontUseResources ON)

include(${MegaSyncDir}/control/control.cmake)
include(${MegaSyncDir}/gui/gui.cmake)
include(${MegaSyncDir}/syncs/syncs.cmake)
include(${MegaSyncDir}/platform/platform.cmake)
include(${MegaSyncDir}/transfers/transfers.cmake)
include(${MegaSyncDir}/stalled_issues/stalledissues.cmake)
include(${MegaSyncDir}/notifications/notifications.cmake)
include(${MegaSyncDir}/UserAttributesRequests/userattributesrequests.cmake)

target_link_libraries(UnitTests
    PRIVATE
    trompeloeil
    MEGA::SDKlib
    MEGA::SDKQtBindings
    $<$<BOOL:${WIN32}>:Qt5::WinExtras>
    $<$<BOOL:${APPLE}>:Qt5::MacExtras>
    $<$<BOOL:${UNIX}>:Qt5::Svg>
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::Network
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickWidgets
    MEGA::google_breakpad
    )

target_include_directories(UnitTests PRIVATE ../3rdparty ${MegaSyncDir})
