@echo off
setlocal
REM ===========================================================================
REM Startskript fuer Das Schwarze Auge: Sternenschweif (Nativer Port)
REM ===========================================================================

set "SCRIPT_DIR=%~dp0"
if not defined PORT_EXE_NAME set "PORT_EXE_NAME=SCHWEIF_PORT_WICHTIG.EXE"
if not defined PROBENLOG set "PROBENLOG=0"

REM Optionaler Schalter --probenlog / log als erster Parameter
if /i "%~1"=="--probenlog" (
    set "PROBENLOG=1"
    shift
) else if /i "%~1"=="log" (
    set "PROBENLOG=1"
    shift
)

REM 1. SPIELPFAD aus erstem Parameter oder Umgebungsvariable uebernehmen
if not "%~1"=="" (
    if exist "%~1\DATA\STAR.DAT" (
        set "SPIELPFAD=%~1"
        shift
    ) else if exist "%~1\STAR.DAT" (
        set "SPIELPFAD=%~1"
        shift
    ) else if exist "%~1\SCHWEIF.EXE" (
        set "SPIELPFAD=%~1"
        shift
    )
)

REM 2. Gemerkten Pfad aus SPIELPFAD.txt pruefen
if "%SPIELPFAD%"=="" (
    if exist "%SCRIPT_DIR%SPIELPFAD.txt" (
        set /p SPIELPFAD=<"%SCRIPT_DIR%SPIELPFAD.txt"
    )
)
if not "%SPIELPFAD%"=="" (
    if not exist "%SPIELPFAD%\DATA\STAR.DAT" if not exist "%SPIELPFAD%\STAR.DAT" if not exist "%SPIELPFAD%\SCHWEIF.EXE" (
        set "SPIELPFAD="
    )
)

REM 3. Automatische Suche in typischen Ordnern
if "%SPIELPFAD%"=="" (
    for %%D in (
        "%SCRIPT_DIR%."
        "%SCRIPT_DIR%.."
        "%SCRIPT_DIR%..\Original\CD"
        "C:\GOG Games\Realms of Arkania 2"
        "D:\GOG Games\Realms of Arkania 2"
        "E:\GOG Games\Realms of Arkania 2"
        "C:\Program Files (x86)\GOG Galaxy\Games\Realms of Arkania 2"
        "C:\Program Files\GOG Galaxy\Games\Realms of Arkania 2"
        "D:\Program Files (x86)\GOG Galaxy\Games\Realms of Arkania 2"
        "C:\Program Files (x86)\Steam\steamapps\common\Realms of Arkania 2"
        "C:\Spiele\Sternenschweif"
        "D:\Spiele\Sternenschweif"
    ) do (
        if "%SPIELPFAD%"=="" (
            if exist "%%~D\DATA\STAR.DAT" (
                set "SPIELPFAD=%%~fD"
            ) else if exist "%%~D\STAR.DAT" (
                set "SPIELPFAD=%%~fD"
            ) else if exist "%%~D\SCHWEIF.EXE" (
                set "SPIELPFAD=%%~fD"
            )
        )
    )
)

REM 4. Einmalig nachfragen und in SPIELPFAD.txt merken
:FRAG_PFAD
if "%SPIELPFAD%"=="" (
    echo ============================================================================
    echo Spielordner mit DATA\STAR.DAT ^(bzw. SCHWEIF.EXE^) wurde nicht automatisch gefunden.
    echo Bitte den Pfad zu deiner Sternenschweif-Installation eingeben
    echo ^(z. B. C:\GOG Games\Realms of Arkania 2^):
    echo ============================================================================
    set /p SPIELPFAD="Spielpfad: "
    set "SPIELPFAD=%SPIELPFAD:"=%"
)

if "%SPIELPFAD%"=="" goto :EOF
if not exist "%SPIELPFAD%\DATA\STAR.DAT" if not exist "%SPIELPFAD%\STAR.DAT" if not exist "%SPIELPFAD%\SCHWEIF.EXE" (
    echo.
    echo [FEHLER] In "%SPIELPFAD%" wurde weder DATA\STAR.DAT noch SCHWEIF.EXE gefunden!
    set "SPIELPFAD="
    goto :FRAG_PFAD
)

REM Gefundenen Pfad fuer den naechsten Start speichern
> "%SCRIPT_DIR%SPIELPFAD.txt" echo %SPIELPFAD%

REM EXE suchen (entweder direkt neben dem Skript oder unter build\)
set "EXE=%SCRIPT_DIR%%PORT_EXE_NAME%"
if not exist "%EXE%" set "EXE=%SCRIPT_DIR%build\%PORT_EXE_NAME%"
if not exist "%EXE%" set "EXE=%SCRIPT_DIR%SCHWEIF_PORT.EXE"
if not exist "%EXE%" set "EXE=%SCRIPT_DIR%build\SCHWEIF_PORT.EXE"
if not exist "%EXE%" (
    echo [FEHLER] %PORT_EXE_NAME% wurde nicht gefunden!
    echo Bitte zuerst bauen: python Port\bauen_port.py
    pause
    exit /b 1
)

REM Pruefen, ob der Spielordner beschreibbar ist; sonst Spielstaende\ neben der EXE nutzen
set "LOG_DIR=%SPIELPFAD%"
copy /y nul "%SPIELPFAD%\.schweif_write_test.tmp" >nul 2>&1
if errorlevel 1 (
    set "LOG_DIR=%SCRIPT_DIR%Spielstaende"
    if not exist "%SCRIPT_DIR%Spielstaende" mkdir "%SCRIPT_DIR%Spielstaende" >nul 2>&1
) else (
    del /q "%SPIELPFAD%\.schweif_write_test.tmp" >nul 2>&1
)

set "CSV_DATEI=%LOG_DIR%\PROBEN.CSV"
set "ARCHIV_DIR=%LOG_DIR%\Probenlogs"

REM Vorhandene PROBEN.CSV mit Zeitstempel nach Probenlogs\ sichern, damit ohne Probenlog nichts weitergeschrieben wird
if exist "%CSV_DATEI%" (
    for %%A in ("%CSV_DATEI%") do if %%~zA GTR 0 (
        if not exist "%ARCHIV_DIR%" mkdir "%ARCHIV_DIR%" >nul 2>&1
        for /f %%T in ('powershell -NoProfile -Command "(Get-Item -LiteralPath \"%CSV_DATEI%\").LastWriteTime.ToString(\"yyyy-MM-dd_HH-mm-ss\")"') do (
            move /y "%CSV_DATEI%" "%ARCHIV_DIR%\PROBEN_%%T.CSV" >nul 2>&1
        )
    )
    if exist "%CSV_DATEI%" del /q "%CSV_DATEI%" >nul 2>&1
)

REM Falls Probenlog gewuenscht: leere PROBEN.CSV anlegen und Live-Fenster starten
if "%PROBENLOG%"=="1" (
    type nul > "%CSV_DATEI%"
    set "PS1_SKRIPT=%SCRIPT_DIR%Probenlog\probenlog_fenster.ps1"
    if not exist "%SCRIPT_DIR%Probenlog\probenlog_fenster.ps1" (
        if exist "%SCRIPT_DIR%..\Probenlog\probenlog_fenster.ps1" (
            set "PS1_SKRIPT=%SCRIPT_DIR%..\Probenlog\probenlog_fenster.ps1"
        )
    )
    if exist "%SCRIPT_DIR%Probenlog\probenlog_fenster.ps1" (
        start "Probenlog Sternenschweif" powershell -NoExit -ExecutionPolicy Bypass -File "%SCRIPT_DIR%Probenlog\probenlog_fenster.ps1" -Datei "%CSV_DATEI%" -Spiel "Sternenschweif"
    ) else if exist "%SCRIPT_DIR%..\Probenlog\probenlog_fenster.ps1" (
        start "Probenlog Sternenschweif" powershell -NoExit -ExecutionPolicy Bypass -File "%SCRIPT_DIR%..\Probenlog\probenlog_fenster.ps1" -Datei "%CSV_DATEI%" -Spiel "Sternenschweif"
    ) else (
        echo [HINWEIS] Probenlog\probenlog_fenster.ps1 nicht gefunden - PROBEN.CSV wird trotzdem geschrieben.
    )
)

echo Starte %PORT_EXE_NAME% mit Spielordner: "%SPIELPFAD%"
"%EXE%" --gamedir "%SPIELPFAD%" %1 %2 %3 %4 %5 %6 %7 %8 %9
