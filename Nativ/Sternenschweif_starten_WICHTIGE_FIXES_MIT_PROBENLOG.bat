@echo off
setlocal
REM ===========================================================================
REM Startet den nativen Port (WICHTIGE FIXES, FIX_BUGS=1) MIT Live-Probenlog-Fenster.
REM Jede Eigenschafts-, Talent-, Zauber-, Fernkampf- und AT/PA-Probe erscheint
REM sofort im zweiten Fenster und wird in PROBEN.CSV mitgeschrieben.
REM ===========================================================================
set "PORT_EXE_NAME=SCHWEIF_PORT_WICHTIG.EXE"
set "PROBENLOG=1"
if exist "%~dp0Sternenschweif_starten_WICHTIGE_FIXES.bat" (
    call "%~dp0Sternenschweif_starten_WICHTIGE_FIXES.bat" %*
) else (
    call "%~dp0start_sternenschweif.bat" %*
)
