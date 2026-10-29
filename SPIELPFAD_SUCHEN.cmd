@echo off
rem ============================================================
rem  SPIELPFAD_SUCHEN.cmd - findet den Spielordner von Sternenschweif
rem  (GOG: der Ordner mit SCHWEIF.EXE und DOSBOX\DOSBox.exe).
rem  Ergebnis in der Variablen SPIEL. Wird von SCHWEIF_STARTEN.bat
rem  und Bauen\BAUEN.bat aufgerufen und merkt sich die Wahl in
rem  SPIELPFAD.txt (neben dieser Datei). Zum Neu-Waehlen
rem  SPIELPFAD.txt einfach loeschen.
rem ============================================================
set "SPIEL="
set "MERK=%~dp0SPIELPFAD.txt"

rem --- 1. gemerkter Pfad
if exist "%MERK%" set /p SPIEL=<"%MERK%"
if defined SPIEL if exist "%SPIEL%\DOSBOX\DOSBox.exe" goto :eof
set "SPIEL="

rem --- 2. uebliche GOG-Orte
for %%P in ("%ProgramFiles(x86)%\GOG Galaxy\Games\Realms of Arkania 2" "%ProgramFiles%\GOG Galaxy\Games\Realms of Arkania 2" "C:\GOG Games\Realms of Arkania 2" "D:\GOG Games\Realms of Arkania 2" "C:\GOG Games\Realms of Arkania - Star Trail" "D:\GOG Games\Realms of Arkania - Star Trail") do if not defined SPIEL if exist "%%~P\DOSBOX\DOSBox.exe" set "SPIEL=%%~P"
if defined SPIEL goto merken

rem --- 3. fragen: Ordner-Auswahlfenster
echo.
echo Sternenschweif wurde nicht automatisch gefunden.
echo Bitte im folgenden Fenster den Spielordner waehlen:
echo den Ordner, in dem SCHWEIF.EXE und der Unterordner DOSBOX liegen.
echo.
for /f "usebackq delims=" %%F in (`powershell -NoProfile -STA -Command "Add-Type -AssemblyName System.Windows.Forms; $f = New-Object System.Windows.Forms.FolderBrowserDialog; $f.Description = 'Sternenschweif-Spielordner waehlen (mit SCHWEIF.EXE und DOSBOX)'; if ($f.ShowDialog() -eq 'OK') { $f.SelectedPath }"`) do set "SPIEL=%%F"
if not defined SPIEL goto keiner
if exist "%SPIEL%\DOSBOX\DOSBox.exe" goto merken
echo In diesem Ordner fehlt DOSBOX\DOSBox.exe - das ist nicht der Sternenschweif-Ordner von GOG.
set "SPIEL="
goto :eof

:keiner
echo Kein Ordner gewaehlt.
goto :eof

:merken
> "%MERK%" echo %SPIEL%
echo Spielordner: %SPIEL%
goto :eof
