@echo off
rem echo "Invoke win_parent_process2.test"
rem echo "Parameters: #0 : <batch file>, #1 : pin, #2 : -slow_asserts, #3 : <pin command line switches>"
rem echo "Parameters: #4 : pintool, #5: pintool params, #6 : -- <application> <params>"
"%~f1" %~2 %~3 "%~f4" %~5 -pin_path "%~dp1\" -tools_path "%~dp4\"  %~6
