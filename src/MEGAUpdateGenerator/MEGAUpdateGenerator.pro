CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
}
CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
}

isEmpty(THIRDPARTY_VCPKG_BASE_PATH){
    THIRDPARTY_VCPKG_BASE_PATH = $$PWD/../../../3rdParty_desktop
}

win32 {
    contains(QMAKE_TARGET.arch, x86_64):VCPKG_TRIPLET = x64-windows-mega
    !contains(QMAKE_TARGET.arch, x86_64):VCPKG_TRIPLET = x86-windows-mega
}
macx:VCPKG_TRIPLET = x64-osx-mega
unix:!macx:VCPKG_TRIPLET = x64-linux

THIRDPARTY_VCPKG_PATH = $$THIRDPARTY_VCPKG_BASE_PATH/vcpkg/installed/$$VCPKG_TRIPLET
exists($$THIRDPARTY_VCPKG_PATH) {
   CONFIG += vcpkg
}

TARGET = MEGAUpdateGenerator
TEMPLATE = app
CONFIG += console
CONFIG -= qt

DEFINES += USE_CRYPTOPP
DEPENDPATH += $$PWD

SOURCES += ../MEGASync/mega/src/crypto/cryptopp.cpp \
            ../MEGASync/mega/src/base64.cpp \
            ../MEGASync/mega/src/logging.cpp

SOURCES += MEGAUpdateGenerator.cpp

LIBS += -lcryptopp

win32 {
    release {
        LIBS += -L"$$_PRO_FILE_PWD_/../MEGAsync/mega/bindings/qt/3rdparty/libs/x32"
    }
    else {
        LIBS += -L"$$_PRO_FILE_PWD_/../MEGAsync/mega/bindings/qt/3rdparty/libs/x32d"
    }

    INCLUDEPATH += $$[QT_INSTALL_PREFIX]/src/3rdparty/zlib
    LIBS += -lws2_32
    DEFINES += USE_CURL
    DEFINES += NOMINMAX
}

macx {
    contains(QT_ARCH, arm64):QMAKE_MACOSX_DEPLOYMENT_TARGET = 11.0
    else:QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13
    QMAKE_CXXFLAGS += -DCRYPTOPP_DISABLE_ASM

    clang {
        COMPILER_VERSION = $$system("$$QMAKE_CXX -dumpversion | cut -d'.' -f1")
        message($$COMPILER_VERSION)
        greaterThan(COMPILER_VERSION, 14) {
            message("Using Xcode 15 or above. Switching to ld_classic linking.")
            LIBS += -Wl,-ld_classic
        }
    }
}

vcpkg {
    INCLUDEPATH += $$THIRDPARTY_VCPKG_PATH/include
    release:LIBS += -L"$$THIRDPARTY_VCPKG_PATH/lib"
    debug:LIBS += -L"$$THIRDPARTY_VCPKG_PATH/debug/lib"
}
else {
    INCLUDEPATH += $$PWD ../MEGASync/mega/bindings/qt/3rdparty/include \
                    ../MEGASync/mega/bindings/qt/3rdparty/include/cryptopp \
                    ../MEGASync/mega/bindings/qt/3rdparty/include/cares \
                    ../MEGASync/mega/bindings/qt/3rdparty/include/libsodium \
                    ../MEGASync/mega/include/
}

win32 {
    INCLUDEPATH += ../MEGASync/mega/include/mega/wincurl
}

unix {
    INCLUDEPATH += ../MEGASync/mega/include/mega/posix
    DEFINES += USE_PTHREAD
}
