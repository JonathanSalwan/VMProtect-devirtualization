@echo off
rem echo "Expand directory and full path and call win_parent_process.exe"
"%~dp0%1win_parent_process.exe"
"%~dp0%1win_parent_process.exe"
"%~dp0%1win_parent_process.exe"
rem echo "Expand directory and full path and call win_child_process.exe with expected params"
"%~dp0%1win_child_process.exe" "param1 param2" param3
"%~dp0%1win_child_process.exe" "param1 param2" param3
"%~dp0%1win_child_process.exe" "param1 param2" param3
