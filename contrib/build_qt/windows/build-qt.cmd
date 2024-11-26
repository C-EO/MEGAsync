@echo off
setlocal enabledelayedexpansion

SET MEGA_QT_MAJ_VER=5
SET MEGA_QT_MIN_VER=15
SET MEGA_QT_DOT_VER=16
SET MEGA_QT_VER=%MEGA_QT_MAJ_VER%.%MEGA_QT_MIN_VER%.%MEGA_QT_DOT_VER%

SET MEGA_QT_BASEURRL=https://download.qt.io/development_releases/prebuilt/llvmpipe/windows
SET MEGA_QT_OGLSW_64_FILE=opengl32sw-64-mesa_11_2_2-signed_sha256.7z
SET MEGA_QT_OGLSW_32_FILE=opengl32sw-32-mesa_11_2_2-signed_sha256.7z
SET MEGA_QT_OGLSW_SHA_FILE=sha256sums.txt

SET MEGA_PATCHES_DIR=%CD%\..\patches
SET MEGA_WORK_DIR=C:\Qt-build\%MEGA_QT_VER%

mkdir %MEGA_WORK_DIR%
cd %MEGA_WORK_DIR%

REM Get sources
git clone https://code.qt.io/qt/qt5.git Src
cd Src
git checkout v%MEGA_QT_VER%-lts-lgpl
perl init-repository --module-subset=essential,qtwinextras,qtimageformats,qtquickcontrols,qtsvg,qtgraphicaleffects,qtdeclarative,qtquickcontrols2

REM apply patches
FOR /D %%M in (%MEGA_PATCHES_DIR%\%MEGA_QT_VER%\*) DO (
	FOR %%I IN (%%M\*) DO (
		echo %%I|findstr /r /c:".*\\[0-9][0-9]\.all\..*">nul 2>&1
		IF !ERRORLEVEL! EQU 0 (
			echo "Applying %%I"
			git apply --directory=%%~nM --ignore-whitespace %%I
		) ELSE (
			echo %%I|findstr /r /c:".*\\[0-9][0-9]\.win\..*">nul 2>&1
			IF !ERRORLEVEL! EQU 0 (
				echo "Applying %%I"
				git apply --directory=%%~nM --ignore-whitespace %%I
			)	
		)
	)
)

SET _ROOT=%CD%
SET PATH=%_ROOT%\qtbase\bin;%_ROOT%\gnuwin32\bin;%PATH%
SET PATH=%_ROOT%\qtrepotools\bin;%PATH%
SET _ROOT=

REM Build x64 version
cd ..
mkdir build-x64
cd build-x64
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" amd64
call ..\Src\configure ^
	-opensource -confirm-license ^
	-nomake tests ^
	-nomake examples ^
	-schannel  ^
	-prefix %MEGA_WORK_DIR%\x64  ^
	-force-debug-info  ^
	-separate-debug-info  ^
	-qt-zlib  ^
	-no-jasper  ^
	-qt-libjpeg  ^
	-qt-libpng  ^
	-qt-freetype  ^
	-qt-pcre  ^
	-qt-harfbuzz ^
	-mp ^
	-opengl dynamic
call jom
call jom install

REM Install opengl32sw.dll
curl -L -o %MEGA_QT_OGLSW_64_FILE% %MEGA_QT_BASEURRL%/%MEGA_QT_OGLSW_64_FILE%
curl -L -o %MEGA_QT_OGLSW_SHA_FILE% %MEGA_QT_BASEURRL%/%MEGA_QT_OGLSW_SHA_FILE%
type %MEGA_QT_OGLSW_SHA_FILE% | findstr "%MEGA_QT_OGLSW_64_FILE%" >%MEGA_QT_OGLSW_64_FILE%.sha256
for /f "tokens=1" %%a in (%MEGA_QT_OGLSW_64_FILE%.sha256) do (
  set "GOOD_SHA=%%a"
)
set /a count=1
for /f "skip=1 delims=:" %%a in ('CertUtil -hashfile %MEGA_QT_OGLSW_64_FILE% SHA256') do (
  if !count! equ 1 set "DL_SHA=%%a"
  set/a count+=1
)
echo "SHA256 for %MEGA_QT_OGLSW_64_FILE%. Expected %GOOD_SHA%, got %DL_SHA%"
IF NOT %DL_SHA% == %GOOD_SHA% (
	echo "Bad SHA256 for %MEGA_QT_OGLSW_64_FILE%."
	GOTO :EOF
)

7z x -o%MEGA_WORK_DIR%\x64\bin %MEGA_QT_OGLSW_64_FILE%

REM Build x86 version
cd ..
mkdir build-x86
cd build-x86
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" x86
call ..\Src\configure ^
	-opensource -confirm-license ^
	-nomake tests ^
	-nomake examples ^
	-schannel  ^
	-prefix %MEGA_WORK_DIR%\x86  ^
	-force-debug-info  ^
	-separate-debug-info  ^
	-qt-zlib  ^
	-no-jasper  ^
	-qt-libjpeg  ^
	-qt-libpng  ^
	-qt-freetype  ^
	-qt-pcre  ^
	-qt-harfbuzz ^
	-mp ^
	-opengl dynamic
call jom
call jom install

REM Install opengl32sw.dll
curl -L -o %MEGA_QT_OGLSW_32_FILE% %MEGA_QT_BASEURRL%/%MEGA_QT_OGLSW_32_FILE%
curl -L -o %MEGA_QT_OGLSW_SHA_FILE% %MEGA_QT_BASEURRL%/%MEGA_QT_OGLSW_SHA_FILE%
type %MEGA_QT_OGLSW_SHA_FILE% | findstr "%MEGA_QT_OGLSW_32_FILE%" >%MEGA_QT_OGLSW_32_FILE%.sha256
for /f "tokens=1" %%a in (%MEGA_QT_OGLSW_32_FILE%.sha256) do (
  set "GOOD_SHA=%%a"
)
set /a count=1
for /f "skip=1 delims=:" %%a in ('CertUtil -hashfile %MEGA_QT_OGLSW_32_FILE% SHA256') do (
  if !count! equ 1 set "DL_SHA=%%a"
  set/a count+=1
)
echo "SHA256 for %MEGA_QT_OGLSW_32_FILE%. Expected %GOOD_SHA%, got %DL_SHA%"
IF NOT %DL_SHA% == %GOOD_SHA% (
	echo "Bad SHA256 for %MEGA_QT_OGLSW_32_FILE%."
	GOTO :EOF
)

7z x -o%MEGA_WORK_DIR%\x86\bin %MEGA_QT_OGLSW_32_FILE%
