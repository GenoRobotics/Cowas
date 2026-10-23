@echo off
rem Starts tools\session_log.py with the PlatformIO Python (installed with PlatformIO on the laptop).
rem Usage, from the project folder:  tools\session_log.cmd --port COM6 --tag s1-1c
"%USERPROFILE%\.platformio\penv\Scripts\python.exe" "%~dp0session_log.py" %*
