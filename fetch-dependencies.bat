@echo off
rem ***************************************************************************
rem * This script will fetch openssl, strawberry-perl (for openssl build) and curl
rem * and build openssl and curl with openssl support
rem ***************************************************************************

:begin
  echo Compiler:
  echo.
  echo vc10      - Use Visual Studio 2010
  echo vc11      - Use Visual Studio 2012
  echo vc12      - Use Visual Studio 2013
  echo vc14      - Use Visual Studio 2015
  echo vc14.1    - Use Visual Studio 2017
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
    if /i "%vc_version%" == "vc10" (
      set VC_VER=10.0
      set VC_DESC=VC10
      set "VC_PATH=Microsoft Visual Studio 10.0"
    ) else if /i "%vc_version%" == "vc11" (
      set VC_VER=11.0
      set VC_DESC=VC11
      set "VC_PATH=Microsoft Visual Studio 11.0"
    ) else if /i "%vc_version%" == "vc12" (
      set VC_VER=12.0
      set VC_DESC=VC12
      set "VC_PATH=Microsoft Visual Studio 12.0"
    ) else if /i "%vc_version%" == "vc14" (
      set VC_VER=14.0
      set VC_DESC=VC14
      set "VC_PATH=Microsoft Visual Studio 14.0"
    ) else if /i "%vc_version%" == "vc14.1" (
      set VC_VER=14.1
      set VC_DESC=VC14.10
	  set VC_generate=vc14.10

      rem Determine the VC14.1 path based on the installed edition in descending
      rem order (Enterprise, then Professional and finally Community)
      if exist "%PF%\Microsoft Visual Studio\2017\Enterprise" (
        set "VC_PATH=Microsoft Visual Studio\2017\Enterprise"
      ) else if exist "%PF%\Microsoft Visual Studio\2017\Professional" (
        set "VC_PATH=Microsoft Visual Studio\2017\Professional"
      ) else (
        set "VC_PATH=Microsoft Visual Studio\2017\Community"
      )
    ) else if /i "%vc_version%" == "vc14.2" (
      set VC_VER=14.2
      set VC_DESC=VC14.20
	  set VC_generate=vc14.20

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

      rem Determine the VC14.3 path based on the installed edition in descending
      rem order (Enterprise, then Professional and finally Community)
      if exist "%PF%\Microsoft Visual Studio\2022\Enterprise" (
        set "VC_PATH=Microsoft Visual Studio\2022\Enterprise"
      ) else if exist "%PF%\Microsoft Visual Studio\2022\Professional" (
        set "VC_PATH=Microsoft Visual Studio\2022\Professional"
      ) else (
        set "VC_PATH=Microsoft Visual Studio\2022\Community"
      )
    )
  )

:fetchDeps
	rem assume we are being called inside a project, don't fetch the stuff inside of the project
	cd ..
	echo Fetching Dependencies...
	if not exist "openssl" (
		echo openssl...
		call powershell "$source= ((Invoke-RestMethod -Method GET -Uri """https://api.github.com/repos/openssl/openssl/releases""")[0].assets | Where-Object name -like """openssl*.tar.gz""" ).browser_download_url;Write-Host $source;Invoke-WebRequest -Uri $source -Out $(Split-Path -Path $source -Leaf)"
		call powershell "$file = Get-ChildItem """openssl*"""; tar -xzf $file"
		call powershell "Get-ChildItem """openssl*""" | move-item -Destination """openssl\""
	)
	
	rem OpenSSL needs perl to build, strawberry perl portable will do it, add it to PATH
	if not exist "strawberry-perl" (
		echo strawberry-perl...
		call powershell "$source= ((Invoke-RestMethod -Method GET -Uri """https://api.github.com/repos/StrawberryPerl/Perl-Dist-Strawberry/releases""")[0].assets | Where-Object name -like """*portable.zip""" ).browser_download_url;Write-Host $source;Invoke-WebRequest -Uri $source -Out $(Split-Path -Path $source -Leaf)"
		call powershell "Expand-Archive -Path """strawberry-perl*portable.zip""" -DestinationPath """strawberry-perl""""
	)
	set PATH=%cd%\strawberry-perl\perl\bin;%PATH%
	
	rem I couldn't get curl release to work for Windows. Use the source archive instead (zipball) that includes GIT-INFO and VS tmpl files to generate VS solution files
	if not exist "curl" (
		echo curl...
		call powershell "$source= (Invoke-RestMethod -Method GET -Uri """https://api.github.com/repos/curl/curl/releases""")[0].zipball_url;Write-Host $source;Invoke-WebRequest -Uri $source -Out $(Split-Path -Path $source -Leaf)"
		call powershell "Get-ChildItem """curl*""" | move-item -Destination """curl.zip""""
		call powershell "Expand-Archive -Path """curl*.zip""" -DestinationPath """curl""""
		call powershell "Get-ChildItem """curl\*\*""" | move-item -Destination """curl\""
	)

:buildDeps
	call "%PF%\%VC_PATH%\VC\Auxiliary\Build\vcvars32.bat"
	set ROOT_DEV_DIR=%cd%
	cd curl
	call buildconf.bat
	cd projects
	call generate.bat %VC_generate%
	call build-openssl.bat %vc_version% x86 release ..\..\openssl 
	cd Windows\%VC_DESC%
	call "%PF%\%VC_PATH%\Common7\IDE\devenv.exe" curl-all.sln /Build "DLL Release - DLL OpenSSL"
:harvest
	cd %ROOT_DEV_DIR%\curl
	mkdir bin
	call powershell "Get-ChildItem """build\Win32\%VC_DESC%\DLL Release - DLL OpenSSL\*.dll""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """build\Win32\%VC_DESC%\DLL Release - DLL OpenSSL\*.lib""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """build\Win32\%VC_DESC%\DLL Release - DLL OpenSSL\*.pdb""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """build\Win32\%VC_DESC%\DLL Release - DLL OpenSSL\*.exe""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """..\openssl\build\Win32\%VC_DESC%\DLL Release\*.dll""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """..\openssl\build\Win32\%VC_DESC%\DLL Release\*.lib""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """..\openssl\build\Win32\%VC_DESC%\DLL Release\*.pdb""" | copy-item -Destination """bin\""
	call powershell "Get-ChildItem """..\openssl\build\Win32\%VC_DESC%\DLL Release\*.exe""" | copy-item -Destination """bin\""
	echo Done
	pause
