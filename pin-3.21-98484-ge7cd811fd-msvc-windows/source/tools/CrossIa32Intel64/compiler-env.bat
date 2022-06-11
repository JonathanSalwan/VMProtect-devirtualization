
@echo off

REM Batch File Commad line: 
REM compiler-env.bat [amd64/x86/icl_amd64/icl_x86] make TARGET=[ia32/ia32e] <target name> 

REM all settings will return to the previous state by the end of the batch file
setlocal

REM Finding the value of TARGET
if "%1"=="x86"       goto :setEnv
if "%1"=="amd64"     goto :setEnv
if "%1"=="icl_x86"   goto :setIclEnv32
if "%1"=="icl_amd64" goto :setIclEnv64
goto :badInputValue

REM Set environment variables for given target
:setEnv
if "%VCINSTALLDIR%"=="" goto runMake
if not exist "%VCINSTALLDIR%\vcvarsall.bat" goto runMake
call "%VCINSTALLDIR%\vcvarsall.bat" %1
goto :runMake

:setIclEnv32
if "%ICPP_COMPILER10%"=="" goto :checkIcl11Env32
if not exist "%ICPP_COMPILER10%\bin\iclvars.bat" goto :runMake
call "%ICPP_COMPILER10%\bin\iclvars.bat" ia32 vs2010
goto runMake
:checkIcl11Env32
if "%ICPP_COMPILER11%"=="" goto :runMake
if not exist "%ICPP_COMPILER11%\bin\ia32\iclvars_ia32.bat" goto :runMake
call "%ICPP_COMPILER11%\bin\ia32\iclvars_ia32.bat" ia32 vs2010
goto runMake

:setIclEnv64
if "%ICPP_COMPILER10%"=="" goto :checkIcl11Env64
if not exist "%ICPP_COMPILER10%\EM64T\Bin\iclvars.bat" goto :runMake
call "%ICPP_COMPILER10%\EM64T\Bin\iclvars.bat" intel64 vs2010
goto runMake
:checkIcl11Env64
if "%ICPP_COMPILER11%"=="" goto :runMake
if not exist "%ICPP_COMPILER11%\bin\intel64\iclvars_intel64.bat" goto :runMake
call "%ICPP_COMPILER11%\bin\intel64\iclvars_intel64.bat"
goto runMake

REM Bad input
:badInputValue
echo:
echo compiler-env.bat: First value can be either amd64, x86, icl_amd64 or icl_x86
echo:
goto :end

REM run make
:runMake
%2 %3=%4 %5
goto :end

:end
endlocal 

