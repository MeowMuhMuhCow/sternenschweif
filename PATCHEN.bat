@echo off
rem ============================================================
rem  PATCHEN.bat - erzeugt die Bugfix-Fassungen OHNE Compiler aus
rem  der eigenen SCHWEIF.EXE der GOG-Fassung (Sternenschweif).
rem  Die Original-EXE wird nur gelesen und NICHT veraendert.
rem  Ergebnis: Build\SCHWEIF_FIX_WICHTIG.EXE und Build\SCHWEIF_FIX_ALLES.EXE
rem  Danach spielen mit SCHWEIF_FIX_WICHTIG_STARTEN.bat bzw. _ALLES_STARTEN.bat
rem  Den Spielordner findet SPIELPFAD_SUCHEN.cmd (oder fragt nach).
rem ============================================================
setlocal
title Sternenschweif - Bugfix-Patch
call "%~dp0SPIELPFAD_SUCHEN.cmd"
if not defined SPIEL goto keinspiel
echo Spielordner: %SPIEL%
echo.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Patch\patchen.ps1" -Spiel "%SPIEL%" -Name SCHWEIF.EXE -Ziel "%~dp0Build"
if errorlevel 1 goto fehler
echo.
echo Jetzt spielen: SCHWEIF_FIX_WICHTIG_STARTEN.bat (kritische + mittlere Fehler behoben)
echo          oder: SCHWEIF_FIX_ALLES_STARTEN.bat   (alle behebbaren Fehler)
echo Alle Behebungen sind im Spiel noch ungetestet - Rueckmeldungen willkommen.
echo.
pause
exit /b 0
:fehler
echo.
echo Patch nicht moeglich (siehe Meldung oben).
pause
exit /b 1
:keinspiel
echo Ohne den GOG-Spielordner geht es nicht.
pause
exit /b 1
