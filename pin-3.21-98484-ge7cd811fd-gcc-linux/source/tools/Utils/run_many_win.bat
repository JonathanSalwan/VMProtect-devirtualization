@echo off

REM Run the given command for 500 times. Stops if the command fails.
REM Usage example: run_many_win.bat make inscount1.test

echo.
echo ****************************************************
echo *** Going to run the above command for 500 times ***
echo ****************************************************

for /l %%x in (1, 1, 500) do (
	echo.
	echo Run number %%x:
	echo Line to run: %*
	%* || goto error
)
:error
if %errorlevel% neq 0 echo Command terminated with error %errorlevel%
exit /b %errorlevel%