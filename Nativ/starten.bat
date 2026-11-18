@echo off
setlocal EnableExtensions
cd /d "%~dp0.."
set "EXE=%CD%\build\SCHWEIF_PORT.EXE"
if not exist "%EXE%" (
    echo Die Datei build\SCHWEIF_PORT.EXE fehlt.
    echo Bauanleitung: release\BAUEN.md
    exit /b 1
)
call :readpf "%~dp0SPIELPFAD.txt"
if not defined DSA_GAMEDIR call :readpf "%~dp0..\SPIELPFAD.txt"
"%EXE%" %*
exit /b %ERRORLEVEL%

:readpf
if not exist "%~1" exit /b 0
for /f "usebackq eol=# delims=" %%L in ("%~1") do (
    if not defined DSA_GAMEDIR set "DSA_GAMEDIR=%%L"
)
exit /b 0
