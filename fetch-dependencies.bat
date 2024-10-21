@echo off
rem ***************************************************************************
rem * This script will fetch:
rem * openssl, strawberry-perl (for openssl build)
rem * curl, and libjpeg-turbo
rem * and build openssl and curl with openssl support
rem ***************************************************************************

:begin
  echo Compiler:
  echo.
  echo vc14.2    - Use Visual Studio 2019
  echo vc14.3    - Use Visual Studio 2022
  echo.
  echo Enter Compiler:
  set /p vc_version=
  rem Calculate the program files directory
  if defined PROGRAMFILES (
    set "PF=%PROGRAMFILES%"
    set OS_PLATFORM=x86
  )
  if defined PROGRAMFILES(x86) (
    rem Visual Studio was x86-only prior to 14.3
    if /i "%vc_version%" == "vc14.3" (
      set "PF=%PROGRAMFILES%"
    ) else (
      set "PF=%PROGRAMFILES(x86)%"
    )
    set OS_PLATFORM=x64
  )

:parseArgs
  if not "%vc_version%" == "" (
	if /i "%vc_version%" == "vc14.2" (
      set VC_VER=14.2
      set VC_DESC=VC14.20
	  set VC_generate=vc14.20
	  set "cmake_makefiles=Visual Studio 16 2019"
      rem Determine the VC14.2 path based on the installed edition in descending
      rem order (Enterprise, then Professional and finally Community)
      if exist "%PF%\Microsoft Visual Studio\2019\Enterprise" (
        set "VC_PATH=Microsoft Visual Studio\2019\Enterprise"
      ) else if exist "%PF%\Microsoft Visual Studio\2019\Professional" (
        set "VC_PATH=Microsoft Visual Studio\2019\Professional"
      ) else (
        set "VC_PATH=Microsoft Visual Studio\2019\Community"
      )
    ) else if /i "%vc_version%" == "vc14.3" (
      set VC_VER=14.3
      set VC_DESC=VC14.30
	  set VC_generate=vc14.30
	  set "cmake_makefiles=Visual Studio 17 2022"
      rem Determine the VC14.3 path based on the installed edition in descending
      rem order (Enterprise, then Professional and finally Community)
      if exist "%PF%\Microsoft Visual Studio\2022\Enterprise" (
        set "VC_PATH=Microsoft Visual Studio\2022\Enterprise"
      ) else if exist "%PF%\Microsoft Visual Studio\2022\Professional" (
        set "VC_PATH=Microsoft Visual Studio\2022\Professional"
      ) else (
        set "VC_PATH=Microsoft Visual Studio\2022\Community"
      )
    ) else (
		echo Unsupported compiler
		echo Recommend installing Visual Studio 2022 Community Edition - free
		pause
		exit
	)
  )

:checkEnvironment
	if not exist "README.md" (
		echo Error: Change directory to the rtcwPro repository before running this script
		echo Recommend running this file by double clicking on it in the file explorer
		pause
		exit
	)
  
	if not exist "%PF%\%VC_PATH%\Common7\IDE\devenv.exe" (
		echo "%PF%\%VC_PATH%\Common7\IDE\devenv.exe"
		echo Error: could not find devenv.exe - Visual Studio
		echo Recommend installing Visual Studio 2022 Community Edition - free
		pause
		exit
	)
	
	where /q cmake
	if %errorlevel% neq 0 (
		echo Error: cmake not installed or not in the PATH environment variable
		echo Recommend installing cmake using the cmake installer
		pause
		exit
	)

:fetchDeps
	rem assume we are being called inside a project, do stuff inside a directory
	if not exist "deps" (
		mkdir deps
	)
	cd deps

	echo Fetching Dependencies...

	if not exist "curl" (
		echo curl...
		call powershell "Invoke-WebRequest -Uri https://curl.se/windows/latest.cgi?p=win32-mingw.zip -Out curl.zip"
		call powershell "Expand-Archive -Path curl.zip -DestinationPath curl"
		call powershell "Get-ChildItem """curl\*\*""" | move-item -Destination """curl\""
		call powershell "rm curl.zip"
	) else (
		set SKIP_CURL=1
	)
	
	if not exist "libjpeg-turbo" (
		echo libjpeg-turbo...
		call powershell "$source= (Invoke-RestMethod -Method GET -Uri https://api.github.com/repos/libjpeg-turbo/libjpeg-turbo/releases)[0].zipball_url;"^
						"Write-Host $source;"^
						"$file=$(Split-Path -Path $source -Leaf);"^
						"Invoke-WebRequest -Uri $source -Out $file;"^
						"Get-ChildItem $file | move-item -Destination libjpeg-turbo.zip"
		call powershell "Expand-Archive -Path """libjpeg-turbo*.zip""" -DestinationPath """libjpeg-turbo""""
		call powershell "Get-ChildItem """libjpeg-turbo\*\*""" | move-item -Destination """libjpeg-turbo\""
		call powershell "rm libjpeg-turbo.zip"
	) else (
		set SKIP_JPEG=1
	)
	
	if not exist "jansson" (
		echo jansson...
		call powershell "$source= (Invoke-RestMethod -Method GET -Uri https://api.github.com/repos/akheron/jansson/releases)[0].zipball_url;"^
						"Write-Host $source;"^
						"$file=$(Split-Path -Path $source -Leaf);"^
						"Invoke-WebRequest -Uri $source -Out $file;"^
						"Get-ChildItem $file | move-item -Destination jansson.zip"
		call powershell "Expand-Archive -Path """jansson.zip""" -DestinationPath """jansson""""
		call powershell "Get-ChildItem """jansson\*\*""" | move-item -Destination """jansson\""
		call powershell "rm jansson.zip"
	) else (
		set SKIP_JANSSON=1
	)
:buildDeps
	call "%PF%\%VC_PATH%\VC\Auxiliary\Build\vcvars32.bat"
	set ROOT_DEP_DIR=%cd%
:buildCurl
	if defined SKIP_CURL (
		goto buildLibJPEG
	)
	cd "%ROOT_DEP_DIR%\curl"
	cd bin
	lib /def:libcurl.def /OUT:libcurl.lib /MACHINE:X86
	
:buildLibJPEG
	if defined SKIP_JPEG (
		goto buildJansson
	)
	cd "%ROOT_DEP_DIR%\libjpeg-turbo"
	mkdir build
	cd build
	call cmake -G"%cmake_makefiles%" -A Win32 -DCMAKE_BUILD_TYPE=Release ..
	call "%PF%\%VC_PATH%\Common7\IDE\devenv.exe" libjpeg-turbo.sln /Build Release
	call powershell "Get-ChildItem """..\src\*.h""" | copy-item -Destination """..\""
	call powershell "Get-ChildItem """*.h""" | copy-item -Destination """..\""
	
:buildJansson
	if defined SKIP_JANSSON (
		goto harvest
	)
	cd "%ROOT_DEP_DIR%\jansson"
	mkdir build
	cd build
	call cmake -G"%cmake_makefiles%" -A Win32 -DCMAKE_BUILD_TYPE=Release ..
	call "%PF%\%VC_PATH%\Common7\IDE\devenv.exe" jansson.sln /Build Release
	
:harvest
	cd %ROOT_DEP_DIR%
	if not exist "bin" (
		mkdir bin
	)
	call powershell "Get-ChildItem """curl\bin\*.dll""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """curl\bin\*.lib""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """libjpeg-turbo\build\Release\*.dll""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """libjpeg-turbo\build\Release\*.lib""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """jansson\build\lib\Release\*.lib""" | copy-item -Destination """bin\""
	echo Copy the DLL files from deps/bin to your RtcwPro install location where wolfMP.exe is
	pause
