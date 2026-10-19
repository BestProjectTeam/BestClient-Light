@echo off
setlocal EnableExtensions

if "%~1"=="" goto :usage
if /I "%~1"=="-h" goto :usage
if /I "%~1"=="--help" goto :usage
if /I "%~1"=="/?" goto :usage

set "ROOT=%~f1"

if not exist "%ROOT%\CMakeLists.txt" (
	echo error: not a project root, CMakeLists.txt missing: "%ROOT%"
	exit /b 1
)

if not exist "%ROOT%\ddnet-libs\sdl\windows\lib64\SDL2.lib" (
	echo error: ddnet-libs is not initialized in "%ROOT%"
	echo run: git -C "%ROOT%" submodule update --init ddnet-libs
	exit /b 1
)

set "VCVARS=E:\visual-studio\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
	echo error: vcvars64.bat not found: "%VCVARS%"
	exit /b 1
)

call "%VCVARS%"
if errorlevel 1 (
	echo error: vcvars64.bat failed
	exit /b 1
)

cmake -S "%ROOT%" -B "%ROOT%\build" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DDEV=ON -DPREFER_BUNDLED_LIBS=ON -DDOWNLOAD_GTEST=OFF -DCLIENT=ON -DSERVER=OFF -DTOOLS=OFF
if errorlevel 1 exit /b 1

ninja -C "%ROOT%\build"
if errorlevel 1 exit /b 1

echo Built: "%ROOT%\build\DDNet.exe"
exit /b 0

:usage
echo Usage: build_client_debug.bat ^<project-root^>
echo Example: build_client_debug.bat E:\BestClient-Light
exit /b 1
