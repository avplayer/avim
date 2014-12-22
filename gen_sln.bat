﻿
@echo off

echo setting build environment

echo make sure your boost installed on c:\boost
echo make sure your openssl installed on c:\openssl

echo make sure protobuf is built first !!!!

:: 每人的硬盘及使用情况都不一样，请勿统一目录,
:: 各位根据自家情况把boost根目录设为BOOST_ROOT环境变量
set BOOST_ROOT=%BOOST_ROOT%
set PROTOBUF_SRC_ROOT_FOLDER=C:\protobuf-2.6.1\
set CMAKE_PREFIX_PATH=c:\Qt\lib\cmake

echo "making build directory"
md %~dp0\build

cd %~dp0\build

ECHO 1.Win64 build
ECHO 2.Win32 build
ECHO 3.Exit script
ECHO.

CHOICE /C 123 /M "Enter your choice:"

IF ERRORLEVEL 3 GOTO ENDSCRIPT
IF ERRORLEVEL 2 GOTO Win32
IF ERRORLEVEL 1 GOTO Win64

echo "calling cmake to generate MSVC projects now!"

:Win64
cmake.exe %~dp0 -G "Visual Studio 12 2013 Win64"
GOTO End

:Win32
cmake.exe %~dp0 -G "Visual Studio 12 2013"
GOTO End

ENDSCRIPT:
GOTO End

:End
cd ..
pause
