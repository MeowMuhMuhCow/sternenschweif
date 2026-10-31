@echo off
rem ============================================================
rem  SCHWEIF_FIX_ALLES_MIT_PROBENLOG_STARTEN.bat - wie SCHWEIF_FIX_ALLES_STARTEN.bat,
rem  dazu das Probenlog: jede Probe (Eigenschaft, Talent, Zauber, Attacke,
rem  Parade) erscheint live in einem zweiten Fenster und wird in
rem  Build\Testspielstaende_Fix_Alles\PROBEN.CSV mitgeschrieben.
rem  Hinterher ansehen: Probenlog\Probenlog_Sternenschweif.html
rem ============================================================
set "PROBENLOG=1"
call "%~dp0SCHWEIF_FIX_ALLES_STARTEN.bat"
