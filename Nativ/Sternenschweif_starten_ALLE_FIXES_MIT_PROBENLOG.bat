@echo off
setlocal
REM ===========================================================================
REM Startet den nativen Port (ALLE FIXES, FIX_BUGS=2) MIT Live-Probenlog-Fenster.
REM Jede Eigenschafts-, Talent-, Zauber-, Fernkampf- und AT/PA-Probe erscheint
REM sofort im zweiten Fenster und wird in PROBEN.CSV mitgeschrieben.
REM ===========================================================================
set "PORT_EXE_NAME=SCHWEIF_PORT.EXE"
set "PROBENLOG=1"
if exist "%~dp0Sternenschweif_starten_ALLE_FIXES.bat" (
    call "%~dp0Sternenschweif_starten_ALLE_FIXES.bat" %*
) else (
    call "%~dp0start_sternenschweif.bat" %*
)
