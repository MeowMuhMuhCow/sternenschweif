@echo off
rem ============================================================
rem  SCHWEIF_FIX_WICHTIG_STARTEN.bat - startet die Fix-Fassung
rem  Build\SCHWEIF_FIX_WICHTIG.EXE (Fehlerbehebungen kritisch + mittel)
rem  mit den GOG-Spieldaten. Das Original im Spielordner wird nicht
rem  angefasst. Die EXE wird als SCHWEIF.EXE nach Build\Fix_Wichtig
rem  kopiert (DOS kennt nur 8.3-Namen). Spielstaende landen in
rem  Build\Testspielstaende_Fix_Wichtig, getrennt von den echten
rem  Spielstaenden und von den Tests der anderen Fassungen.
rem  Gebaut wird sie mit Bauen\BAUEN.bat wichtig.
rem  Den Spielordner findet SPIELPFAD_SUCHEN.cmd (oder fragt nach).
rem ============================================================

set "HIER=%~dp0"
set "BUILD=%HIER%Build"
set "SAVES=%BUILD%\Testspielstaende_Fix_Wichtig"
set "FIXEXE=%BUILD%\SCHWEIF_FIX_WICHTIG.EXE"
set "RUN=%BUILD%\Fix_Wichtig"
set "CONF=%TEMP%\dosbox_schweif_fix_wichtig.conf"

if not exist "%FIXEXE%" goto keinexe
call "%HIER%SPIELPFAD_SUCHEN.cmd"
if not defined SPIEL goto keinspiel
set "GOG=%SPIEL%"
if not exist "%SAVES%" mkdir "%SAVES%"
if not exist "%RUN%" mkdir "%RUN%"
copy /y "%FIXEXE%" "%RUN%\SCHWEIF.EXE" >nul

rem --- DOSBox-Startbefehle in eine kleine Zusatz-Konfiguration schreiben
rem     C: = Spielordner (nur lesen), Aenderungen gehen nach Testspielstaende
rem     D: = CD-Abbild von GOG (fuer die CD-Pruefung cd_check)
rem     E: = Build\Fix_Wichtig (die Fix-EXE als SCHWEIF.EXE)
> "%CONF%" echo [sdl]
>>"%CONF%" echo fullscreen=false
>>"%CONF%" echo [autoexec]
>>"%CONF%" echo @echo off
>>"%CONF%" echo mount C "%GOG%"
>>"%CONF%" echo mount C "%SAVES%" -t overlay
if exist "%GOG%\gamea.ins" >>"%CONF%" echo imgmount D "%GOG%\gamea.ins" "%GOG%\gameb.ins" -t iso
>>"%CONF%" echo mount E "%RUN%"
>>"%CONF%" echo C:
>>"%CONF%" echo cls
>>"%CONF%" echo E:\SCHWEIF.EXE
>>"%CONF%" echo echo.
>>"%CONF%" echo echo SCHWEIF_FIX_WICHTIG.EXE (Fix-Fassung) wurde beendet. Taste druecken zum Schliessen.
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
echo Build\SCHWEIF_FIX_WICHTIG.EXE fehlt. Erst Bauen\BAUEN.bat wichtig ausfuehren.
pause
exit /b
