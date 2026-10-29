@echo off
rem ============================================================
rem  SCHWEIF_STARTEN.bat - startet das aus dem Quellcode gebaute
rem  Sternenschweif (Build\SCHWEIF.EXE) mit den GOG-Spieldaten.
rem  Das Original im Spielordner wird nicht angefasst.
rem  Spielstaende landen in Build\Testspielstaende, nicht bei
rem  deinen echten Spielstaenden.
rem  Den Spielordner findet SPIELPFAD_SUCHEN.cmd (oder fragt nach).
rem ============================================================

set "HIER=%~dp0"
set "BUILD=%HIER%Build"
set "SAVES=%BUILD%\Testspielstaende"
set "CONF=%TEMP%\dosbox_schweif_build.conf"

if not exist "%BUILD%\SCHWEIF.EXE" goto keinexe
call "%HIER%SPIELPFAD_SUCHEN.cmd"
if not defined SPIEL goto keinspiel
set "GOG=%SPIEL%"
if not exist "%SAVES%" mkdir "%SAVES%"

rem --- DOSBox-Startbefehle in eine kleine Zusatz-Konfiguration schreiben
rem     C: = Spielordner (nur lesen), Aenderungen gehen nach Testspielstaende
rem     D: = CD-Abbild von GOG (fuer die CD-Pruefung cd_check)
rem     E: = unser Build
> "%CONF%" echo [sdl]
>>"%CONF%" echo fullscreen=false
>>"%CONF%" echo [autoexec]
>>"%CONF%" echo @echo off
>>"%CONF%" echo mount C "%GOG%"
>>"%CONF%" echo mount C "%SAVES%" -t overlay
if exist "%GOG%\gamea.ins" >>"%CONF%" echo imgmount D "%GOG%\gamea.ins" "%GOG%\gameb.ins" -t iso
>>"%CONF%" echo mount E "%BUILD%"
>>"%CONF%" echo C:
>>"%CONF%" echo cls
>>"%CONF%" echo E:\SCHWEIF.EXE
>>"%CONF%" echo echo.
>>"%CONF%" echo echo SCHWEIF.EXE (aus dem Quellcode gebaut) wurde beendet. Taste druecken zum Schliessen.
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
echo Build\SCHWEIF.EXE fehlt neben dieser Datei. Erst Bauen\BAUEN.bat ausfuehren.
pause
exit /b
