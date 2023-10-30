

isEmpty(THIRDPARTY_VCPKG_BASE_PATH){
    THIRDPARTY_VCPKG_BASE_PATH = $$PWD/../../../3rdParty_desktop
}

win32 {
    contains(QMAKE_TARGET.arch, x86_64):VCPKG_TRIPLET = x64-windows-mega
    !contains(QMAKE_TARGET.arch, x86_64):VCPKG_TRIPLET = x86-windows-mega
}

macx{
    contains(QT_ARCH, x86_64):VCPKG_TRIPLET = x64-osx-mega
    contains(QT_ARCH, arm64){
        VCPKG_TRIPLET = arm64-osx-mega
        contains(QMAKE_HOST.arch, arm64):QMAKE_APPLE_DEVICE_ARCHS=arm64
    }
    message("Building for macOS $$QT_ARCH in a $$QMAKE_HOST.arch host.")
}

unix:!macx:VCPKG_TRIPLET = x64-linux

message("THIRDPARTY_VCPKG_BASE_PATH: $$THIRDPARTY_VCPKG_BASE_PATH")
message("VCPKG_TRIPLET: $$VCPKG_TRIPLET")

THIRDPARTY_VCPKG_PATH = $$THIRDPARTY_VCPKG_BASE_PATH/vcpkg/installed/$$VCPKG_TRIPLET
exists($$THIRDPARTY_VCPKG_PATH) {
   CONFIG += vcpkg
}
vcpkg:debug:message("Building DEBUG with VCPKG 3rdparty at $$THIRDPARTY_VCPKG_PATH")
vcpkg:release:message("Building RELEASE with VCPKG 3rdparty at $$THIRDPARTY_VCPKG_PATH")
!vcpkg:message("vcpkg not used")


CONFIG -= qt
MEGASDK_BASE_PATH = $$PWD/../MEGASync/mega

CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
}
CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
}

TARGET = MEGAupdater
TEMPLATE = app

HEADERS += UpdateTask.h \
    Preferences.h \
    MacUtils.h

SOURCES += MegaUpdater.cpp \
    UpdateTask.cpp

vcpkg:INCLUDEPATH += $$THIRDPARTY_VCPKG_PATH/include
else:INCLUDEPATH += $$MEGASDK_BASE_PATH/bindings/qt/3rdparty/include

message("INCLUDEPATH: $$INCLUDEPATH")

macx {    
    OBJECTIVE_SOURCES +=  MacUtils.mm
    DEFINES += _DARWIN_FEATURE_64_BIT_INODE USE_OPENSSL CRYPTOPP_DISABLE_ASM

    contains(QT_ARCH, arm64):QMAKE_MACOSX_DEPLOYMENT_TARGET = 11.0
    else:QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

    !vcpkg:LIBS += -L$$MEGASDK_BASE_PATH/bindings/qt/3rdparty/libs/
    vcpkg:debug:LIBS += -L$$THIRDPARTY_VCPKG_PATH/debug/lib/
    vcpkg:release:LIBS += -L$$THIRDPARTY_VCPKG_PATH/lib/
    LIBS += -framework Cocoa -framework SystemConfiguration -framework CoreFoundation -framework Foundation -framework Security
    QMAKE_CXXFLAGS += -g
    LIBS += -lcryptopp

    clang {
        COMPILER_VERSION = $$system("$$QMAKE_CXX -dumpversion | cut -d'.' -f1")
        message($$COMPILER_VERSION)
        greaterThan(COMPILER_VERSION, 14) {
            message("Using Xcode 15 or above. Switching to ld_classic linking.")
            LIBS += -Wl,-ld_classic
        }
    }
}

win32 {
    contains(CONFIG, BUILDX64) {
       release {
            vcpkg:LIBS += -L"$$THIRDPARTY_VCPKG_PATH/lib"
            else:LIBS += -L"$$MEGASDK_BASE_PATH/bindings/qt/3rdparty/libs/x64"
        }
        else {
            vcpkg:LIBS += -L"$$THIRDPARTY_VCPKG_PATH/debug/lib"
            else:LIBS += -L"$$MEGASDK_BASE_PATH/bindings/qt/3rdparty/libs/x64d"
        }
    }

    !contains(CONFIG, BUILDX64) {
        release {
            vcpkg:LIBS += -L"$$THIRDPARTY_VCPKG_PATH/lib"
            else:LIBS += -L"$$MEGASDK_BASE_PATH/bindings/qt/3rdparty/libs/x32"
        }
        else {
            vcpkg:LIBS += -L"$$THIRDPARTY_VCPKG_PATH/debug/lib"
            else:LIBS += -L"$$MEGASDK_BASE_PATH/bindings/qt/3rdparty/libs/x32d"
        }
    }

    DEFINES += UNICODE _UNICODE NTDDI_VERSION=0x05010000 _WIN32_WINNT=0x0501
    vcpkg:LIBS += -lurlmon -lShlwapi -lShell32 -lAdvapi32 -lcryptopp-staticcrt
    else:LIBS += -lurlmon -lShlwapi -lShell32 -lAdvapi32 -lcryptoppmt

    QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

    QMAKE_CXXFLAGS_RELEASE += -MT
    QMAKE_CXXFLAGS_DEBUG += -MTd

    QMAKE_CXXFLAGS_RELEASE -= -MD
    QMAKE_CXXFLAGS_DEBUG -= -MDd

    RC_FILE = icon.rc
    QMAKE_LFLAGS += /LARGEADDRESSAWARE
    QMAKE_LFLAGS_WINDOWS += /SUBSYSTEM:WINDOWS,5.01
    QMAKE_LFLAGS_CONSOLE += /SUBSYSTEM:CONSOLE,5.01
    DEFINES += PSAPI_VERSION=1
}

unix:!macx {
    error("This tool (MEGAupdater) is only compatible with Windows and macOS. On Linux, MEGA apps can be updated using the official repository.")
}
