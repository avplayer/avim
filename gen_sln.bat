
@echo off

echo "setting build environment"

echo "make sure your boost installed on c:\boost"
echo "make sure your openssl installed on c:\openssl"

echo "make sure protobuf is built first !!!!"

set BOOST_ROOT="c:/boost"
set OPENSSL_ROOT="c:/openssl"
set PROTOBUF_SRC_ROOT_FOLDER=%~dp0\third_party\protobuf-2.6.1\


echo "making build directory"
md build

cd build

ECHO 1.Win64 build
ECHO 2.Win32 build
ECHO 3.Exit script
ECHO.

CHOICE /C 123 /M "Enter your choice:"

:: Note - list ERRORLEVELS in decreasing order
IF ERRORLEVEL 3 GOTO ENDSCRIPT
IF ERRORLEVEL 2 GOTO Win64
IF ERRORLEVEL 1 GOTO Win32

echo "calling cmake to generate MSVC projects now!"

:Win64
cmake.exe %~dp0 -G "Visual Studio 12 2013 Win64"
GOTO End

:Win32
cmake.exe %~dp0 -G "Visual Studio 12 2013"
GOTO End

ENDSCRIPT:
GOTO End

:End1
