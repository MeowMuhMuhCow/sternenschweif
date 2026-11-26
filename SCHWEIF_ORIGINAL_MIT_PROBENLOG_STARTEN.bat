@echo off
rem ============================================================
rem  SCHWEIF_ORIGINAL_MIT_PROBENLOG_STARTEN.bat - startet das
rem  Original ohne jede Fehlerbehebung (Build\SCHWEIF_ORIGINAL_PROBENLOG.EXE,
rem  FIX_BUGS 0), nur mit dem Probenlog: jede Probe erscheint live in
rem  einem zweiten Fenster und wird in
rem  Build\Testspielstaende_Original_Probenlog\PROBEN.CSV mitgeschrieben.
rem  Wuerfelgleich mit SCHWEIF.EXE: dieselben Zufallswerte in derselben
rem  Reihenfolge. Die CD-Pruefung bleibt wie im Original.
rem  Gebaut wird sie mit Bauen\BAUEN.bat probenlog (oder PATCHEN.bat).
rem  Hinterher ansehen: Probenlog\Probenlog_Sternenschweif.html
rem  Den Spielordner findet SPIELPFAD_SUCHEN.cmd (oder fragt nach).
rem ============================================================

set "HIER=%~dp0"
set "BUILD=%HIER%Build"
set "SAVES=%BUILD%\Testspielstaende_Original_Probenlog"
set "FIXEXE=%BUILD%\SCHWEIF_ORIGINAL_PROBENLOG.EXE"
set "RUN=%BUILD%\Original_Probenlog"
set "CONF=%TEMP%\dosbox_schweif_original_probenlog.conf"

if not exist "%FIXEXE%" goto keinexe
call "%HIER%SPIELPFAD_SUCHEN.cmd"
if not defined SPIEL goto keinspiel
set "GOG=%SPIEL%"
if not exist "%SAVES%" mkdir "%SAVES%"
rem --- Spielordner in den Spielstand-Ordner spiegeln: nur fehlende Dateien, vorhandene (Spielstaende) bleiben.
rem     Der Bugfix-Ordner selbst und Build sind ausgeschlossen, falls das Paket im Spielordner liegt
rem     (sonst wuerde es sich selbst immer tiefer hineinkopieren).
rem     Die DOSBox 0.74 von GOG kennt keine Overlay-Laufwerke, deshalb ist dieser Ordner das ganze Laufwerk C:.
rem     Alte Spielstaende des Originals (Windows-VirtualStore) kommen mit.
robocopy "%GOG%" "%SAVES%" /E /XC /XN /XO /R:1 /W:1 /XD "%HIER:~0,-1%" "%BUILD%" Build DOSBOX MUSIC __support cloud_saves /XF *.gog *.ins *.inst *.pdf *.jpg *.zip *.lnk *.ico *.dll *.ini *.info *.hashdb *.script unins000.* goggame* >nul
set "VSTORE=%LOCALAPPDATA%\VirtualStore%GOG:~2%"
if exist "%VSTORE%" robocopy "%VSTORE%" "%SAVES%" /E /XC /XN /XO /R:1 /W:1 /XD Build >nul
if not exist "%RUN%" mkdir "%RUN%"
copy /y "%FIXEXE%" "%RUN%\SCHWEIF.EXE" >nul

rem --- Probenlog: immer an. Das Spiel schreibt nur, wenn PROBEN.CSV da ist. Ein altes Log wandert nach Build\Probenlogs.
if exist "%SAVES%\PROBEN.CSV" for %%A in ("%SAVES%\PROBEN.CSV") do if %%~zA==0 del "%SAVES%\PROBEN.CSV"
if exist "%SAVES%\PROBEN.CSV" (
  if not exist "%BUILD%\Probenlogs" mkdir "%BUILD%\Probenlogs"
  for /f %%t in ('powershell -NoProfile -Command "Get-Date -Format yyyy-MM-dd_HH-mm-ss"') do move /y "%SAVES%\PROBEN.CSV" "%BUILD%\Probenlogs\PROBEN_Original_%%t.CSV" >nul
)
type nul > "%SAVES%\PROBEN.CSV"
start "Probenlog" powershell -NoProfile -ExecutionPolicy Bypass -File "%HIER%Probenlog\probenlog_fenster.ps1" -Datei "%SAVES%\PROBEN.CSV"

rem --- DOSBox-Startbefehle in eine kleine Zusatz-Konfiguration schreiben
rem     C: = Spielstand-Ordner (Kopie des Spielordners, dort landen auch die Spielstaende)
rem     D: = CD-Abbild von GOG (fuer die CD-Pruefung cd_check)
rem     E: = Build\Original_Probenlog (die EXE als SCHWEIF.EXE)
> "%CONF%" echo [sdl]
>>"%CONF%" echo fullscreen=false
>>"%CONF%" echo [autoexec]
>>"%CONF%" echo @echo off
>>"%CONF%" echo mount C "%SAVES%"
if exist "%GOG%\gamea.ins" >>"%CONF%" echo imgmount D "%GOG%\gamea.ins" "%GOG%\gameb.ins" -t iso
>>"%CONF%" echo mount E "%RUN%"
>>"%CONF%" echo C:
>>"%CONF%" echo cls
>>"%CONF%" echo E:\SCHWEIF.EXE
>>"%CONF%" echo echo.
>>"%CONF%" echo echo SCHWEIF_ORIGINAL_PROBENLOG.EXE (Original mit Probenlog) wurde beendet. Taste druecken zum Schliessen.
>>"%CONF%" echo pause
>>"%CONF%" echo exit

rem --- Einstellungen des Spielordners (Grafik, Sound), falls vorhanden, plus unsere Startbefehle
cd /d "%GOG%\DOSBOX"
if exist "..\dosbox_realms2.conf" start "" DOSBox.exe -conf "..\dosbox_realms2.conf" -conf "%CONF%" -noconsole
if not exist "..\dosbox_realms2.conf" start "" DOSBox.exe -conf "%CONF%" -noconsole
exit /b

:keinspiel
echo Ohne den Spielordner von Sternenschweif (GOG) kann das Spiel nicht starten.
pause
exit /b

:keinexe
echo Build\SCHWEIF_ORIGINAL_PROBENLOG.EXE fehlt. Erst PATCHEN.bat ausfuehren (ohne Compiler) oder Bauen\BAUEN.bat probenlog.
pause
exit /b
