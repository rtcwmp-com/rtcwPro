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
	if not exist "openssl" (
		echo openssl...
		call powershell "$source= ((Invoke-RestMethod -Method GET -Uri """https://api.github.com/repos/openssl/openssl/releases""")[0].assets | Where-Object name -like """openssl*.tar.gz""" ).browser_download_url;Write-Host $source;Invoke-WebRequest -Uri $source -Out $(Split-Path -Path $source -Leaf)"
		call powershell "$file = Get-ChildItem """openssl*"""; tar -xzf $file"
		call powershell "Get-ChildItem """openssl*""" | move-item -Destination """openssl\""
	) else (
		set SKIP_SSL=1
	)
	
	rem OpenSSL needs perl to build, strawberry perl portable will do it, add it to PATH
	if not exist "strawberry-perl" (
		echo strawberry-perl...
		call powershell "$source= ((Invoke-RestMethod -Method GET -Uri """https://api.github.com/repos/StrawberryPerl/Perl-Dist-Strawberry/releases""")[0].assets | Where-Object name -like """*portable.zip""" ).browser_download_url;Write-Host $source;Invoke-WebRequest -Uri $source -Out $(Split-Path -Path $source -Leaf)"
		call powershell "Get-ChildItem """strawberry*portable.zip""" | move-item -Destination """strawberry-perl.zip""""
		call powershell "Expand-Archive -Path """strawberry-perl*portable.zip""" -DestinationPath """strawberry-perl""""
		call powershell "rm strawberry-perl.zip"
	)
	set PATH=%cd%\strawberry-perl\perl\bin;%PATH%
	
	rem I couldn't get curl release to work for Windows. Use the source archive instead (zipball) that includes GIT-INFO and VS tmpl files to generate VS solution files
	if not exist "curl" (
		echo curl...
		call powershell "$source= (Invoke-RestMethod -Method GET -Uri """https://api.github.com/repos/curl/curl/releases""")[0].zipball_url;Write-Host $source;Invoke-WebRequest -Uri $source -Out $(Split-Path -Path $source -Leaf)"
		call powershell "Get-ChildItem """curl*""" | move-item -Destination """curl.zip""""
		call powershell "Expand-Archive -Path """curl*.zip""" -DestinationPath """curl""""
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
:buildOpenSSL
	if defined SKIP_SSL (
		goto buildCurl
	)
	cd "%ROOT_DEP_DIR%\curl\projects\"
	call build-openssl.bat %vc_version% x86 release ..\..\openssl 
:buildCurl
	if defined SKIP_CURL (
		goto buildLibJPEG
	)
	
	cd "%ROOT_DEP_DIR%\curl"
	call buildconf.bat
	cd "%ROOT_DEP_DIR%\curl\projects\"
	call generate.bat %VC_generate%
	cd "%ROOT_DEP_DIR%\curl\projects\Windows\%VC_DESC%"
	call "%PF%\%VC_PATH%\Common7\IDE\devenv.exe" curl-all.sln /Build "DLL Release - DLL OpenSSL"
:buildLibJPEG
	if defined SKIP_JPEG (
		goto buildJansson
	)
	cd "%ROOT_DEP_DIR%\libjpeg-turbo"
	mkdir build
	cd build
	call cmake -G"%cmake_makefiles%" -A Win32 -DCMAKE_BUILD_TYPE=Release ..
	call "%PF%\%VC_PATH%\Common7\IDE\devenv.exe" libjpeg-turbo.sln /Build Release
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
	call powershell "Get-ChildItem """curl\build\Win32\%VC_DESC%\DLL Release - DLL OpenSSL\*.dll""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """curl\build\Win32\%VC_DESC%\DLL Release - DLL OpenSSL\*.lib""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """curl\build\Win32\%VC_DESC%\DLL Release - DLL OpenSSL\*.pdb""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """curl\build\Win32\%VC_DESC%\DLL Release - DLL OpenSSL\*.exe""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """openssl\build\Win32\%VC_DESC%\DLL Release\*.dll""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """openssl\build\Win32\%VC_DESC%\DLL Release\*.lib""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """openssl\build\Win32\%VC_DESC%\DLL Release\*.pdb""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """openssl\build\Win32\%VC_DESC%\DLL Release\*.exe""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """libjpeg-turbo\build\Release\*.dll""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """libjpeg-turbo\build\Release\*.lib""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """jansson\build\lib\Release\*.lib""" | copy-item -Destination """bin\""
	echo Copy the DLL files from deps/bin to your RtcwPro install location where wolfMP.exe is
	pause
