#!/usr/bin/env python3
"""mk_baupaket.py [ZIEL] - erzeugt das Windows-Bau-Paket (Standard: <Baum>/baupaket) aus dem Werkbank-Baum.

Inhalt:
  werkzeuge/       die vom Bau benutzten Python-Werkzeuge, umgeschrieben auf SW_ROOT (statt /root/sternenschweif),
                   Unterprozesse mit sys.executable -X utf8 (statt 'python3'), os.replace statt shutil.move
  baustand.tar.gz  ASM-Fassungen (l*/o*), Datensegment (dsg, dsana-Tabellen), lnk-Vorlagen (INIT/FARDATA),
                   lnk_ranges.json, aliases.tsv, c_module.tsv, frisch.txt, FILL.ASM, VORLAGE.PRJ,
                   Referenzstand der C-Quellen (csrc) und Kopfdateien (names/inc)
  BAUEN.bat        Startdatei des Baus (Text unten in BAUEN_BAT): sucht Python und DOSBox - DOSBox ueber
                   ..\\SPIELPFAD_SUCHEN.cmd im GOG-Spielordner; "BAUEN.bat wichtig|alles" baut zusaetzlich die Fix-EXE
  SCHWEIF_FIX_WICHTIG_STARTEN.bat, SCHWEIF_FIX_ALLES_STARTEN.bat
                   Startdateien der Fix-EXEs, abgeleitet aus SCHWEIF_STARTEN.bat (eigene Testspielstand-Ordner)
bauen.py, BAUEN.md, SCHWEIF_STARTEN.bat und SPIELPFAD_SUCHEN.cmd werden nicht erzeugt (liegen fertig im Paketordner,
werden nur geprueft).
Baumwurzel = Elternordner von tools/ (dieses Skript)."""
import os, re, sys, io, tarfile, glob, time

R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
Z = sys.argv[1] if len(sys.argv) > 1 else os.path.join(R, 'baupaket')

WERKZEUGE = ['ovlpub', 'aliases', 'dmgen', 'ovlobjfix', 'objpub', 'objchunk', 'objorder', 'mkprj', 'prjlib',
             'cobjfix_d', 'cobjfix', 'objfix', 'objbytes', 'dsinfo', 'items_ds', 'dscheck', 'cvsasm', 'selfoff',
             'pubdup', 'heapbild', 'cnamen', 'objsame', 'segpad', 'fixcheck']
# Ziel im Bauordner: tools/ (Standard), dsana/items.py, q004/zerl/schweif.py
EXTRA = {'dsana/items.py': 'items.py', 'q004/zerl/schweif.py': 'schweif.py'}


def umschreiben(src, name):
    s = open(src, encoding='utf-8').read()
    n0 = s.count('/root/sternenschweif')
    s = re.sub(r"""(f?)(['"])/root/sternenschweif""", r'_SW + \1\2', s)
    s = s.replace("['python3','-I',", "[sys.executable,'-I','-X','utf8',")
    s = s.replace("[sys.executable, '-I',", "[sys.executable, '-I', '-X', 'utf8',")
    s = s.replace('shutil.move(tmp, out)', 'os.replace(tmp, out)')
    if '/root/sternenschweif' in s or 'python3\'' in s:
        raise SystemExit(f'{name}: Pfad/Aufruf nicht vollstaendig umgeschrieben')
    kopf = "import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket\n"
    zeilen = s.split('\n')
    i = 1 if zeilen and zeilen[0].startswith('#!') else 0
    zeilen.insert(i, kopf.rstrip('\n'))
    return '\n'.join(zeilen), n0


def main():
    wz = os.path.join(Z, 'werkzeuge'); os.makedirs(wz, exist_ok=True)
    for f in glob.glob(os.path.join(wz, '*.py')): os.remove(f)
    gesamt = 0
    for w in WERKZEUGE:
        t, n = umschreiben(os.path.join(R, 'tools', w + '.py'), w)
        open(os.path.join(wz, w + '.py'), 'w', encoding='utf-8', newline='\n').write(t); gesamt += n
    for src, dst in EXTRA.items():
        t, n = umschreiben(os.path.join(R, src), dst)
        open(os.path.join(wz, dst), 'w', encoding='utf-8', newline='\n').write(t); gesamt += n
    print(f'werkzeuge: {len(WERKZEUGE) + len(EXTRA)} Dateien, {gesamt} Pfade umgeschrieben')

    dateien = []
    for n in range(1, 47): dateien.append(f'l{n:02d}/SEG{n:03d}.ASM')
    for n in range(0, 271):
        p = f'o{n:03d}/OVL{n:03d}.ASM'
        if os.path.exists(os.path.join(R, p)): dateien.append(p)
    dateien += [f'dsg/DS0{n}.ASM' for n in range(6)] + ['dsg/DSBSS.ASM']
    dateien += [f'dsana/{f}' for f in ('dm_rest.tsv', 'ds_linkorder.tsv', 'ds_module.tsv', 'ds_owner.tsv',
                                       'korrekturen.tsv', 'zuordnung.json')]
    dateien += ['lnk/INIT.ASM', 'lnk/FARDATA.ASM', 'lnk_ranges.json', 'aliases.tsv', 'c_module.tsv',
                'tools/frisch.txt', 'tools/fill/FILL.ASM', 'tools/VORLAGE.PRJ']
    dateien += sorted('csrc/' + f for f in os.listdir(os.path.join(R, 'csrc')) if f.upper().endswith(('.C', '.CPP')))
    dateien += sorted('names/inc/' + f for f in os.listdir(os.path.join(R, 'names/inc')) if not f.endswith('.orig'))
    fehlt = [d for d in dateien if not os.path.isfile(os.path.join(R, d))]
    if fehlt: raise SystemExit('fehlt: ' + ', '.join(fehlt))
    ziel = os.path.join(Z, 'baustand.tar.gz')
    with tarfile.open(ziel, 'w:gz', compresslevel=9) as t:
        for d in dateien:
            ti = t.gettarinfo(os.path.join(R, d), arcname=d)
            ti.uid = ti.gid = 0; ti.uname = ti.gname = ''; ti.mode = 0o644
            with open(os.path.join(R, d), 'rb') as f: t.addfile(ti, f)
    print(f'baustand.tar.gz: {len(dateien)} Dateien, {os.path.getsize(ziel) // 1024} KB')


BAUEN_BAT = r"""@echo off
rem BAUEN.bat - baut SCHWEIF.EXE aus dem rekonstruierten Quelltext (Projekt Sternenschweif)
rem Sucht Python und DOSBox und startet bauen.py. Ergebnis: ..\Build\SCHWEIF.EXE und ..\Build\bau.log
rem   BAUEN.bat wichtig   zusaetzlich ..\Build\SCHWEIF_FIX_WICHTIG.EXE (Fehlerbehebungen kritisch + mittel)
rem   BAUEN.bat alles     zusaetzlich ..\Build\SCHWEIF_FIX_ALLES.EXE   (alle Fehlerbehebungen)
setlocal
cd /d "%~dp0"
title Sternenschweif - Bau von SCHWEIF.EXE %1

rem ---- Python: python\python.exe neben dieser Datei, sonst py -3, sonst python
set PY=
if exist "%~dp0python\python.exe" set PY="%~dp0python\python.exe"
if defined PY goto py_ok
py -3 -c "import sys; sys.exit(0 if sys.version_info >= (3, 8) else 1)" >nul 2>&1
if not errorlevel 1 set PY=py -3
if defined PY goto py_ok
python -c "import sys; sys.exit(0 if sys.version_info >= (3, 8) else 1)" >nul 2>&1
if not errorlevel 1 set PY=python
if defined PY goto py_ok
echo.
echo Python 3 wurde nicht gefunden.
echo Bitte Python fuer Windows von https://www.python.org/downloads/ installieren
echo (beim Installieren "Add python.exe to PATH" ankreuzen) und BAUEN.bat erneut starten.
echo.
pause
exit /b 1
:py_ok

rem ---- DOSBox 0.74: DOSBox\DOSBox.exe neben dieser Datei, sonst die aus dem Sternenschweif-Spielordner
set DB=
if exist "%~dp0DOSBox\DOSBox.exe" set DB="%~dp0DOSBox\DOSBox.exe"
if defined DB goto db_ok
rem ---- sonst den Spielordner suchen bzw. erfragen (gemerkt in ..\SPIELPFAD.txt)
call "%~dp0..\SPIELPFAD_SUCHEN.cmd"
if defined SPIEL set DB="%SPIEL%\DOSBOX\DOSBox.exe"
:db_ok

echo Python: %PY%
if defined DB echo DOSBox: %DB%
echo.
if defined DB %PY% -X utf8 "%~dp0bauen.py" --dosbox %DB% %*
if not defined DB %PY% -X utf8 "%~dp0bauen.py" %*
set RC=%errorlevel%
echo.
if "%RC%"=="0" echo Fertig. Ergebnis: ..\Build\SCHWEIF.EXE
if "%RC%"=="0" if /i "%~1"=="wichtig" echo Fix-EXE: ..\Build\SCHWEIF_FIX_WICHTIG.EXE - starten mit SCHWEIF_FIX_WICHTIG_STARTEN.bat
if "%RC%"=="0" if /i "%~1"=="alles" echo Fix-EXE: ..\Build\SCHWEIF_FIX_ALLES.EXE - starten mit SCHWEIF_FIX_ALLES_STARTEN.bat
if not "%RC%"=="0" echo Nicht bytegleich oder abgebrochen - Protokoll: ..\Build\bau.log
echo.
pause
exit /b %RC%
"""


def fix_starter(src, stufe, beschr):
    """SCHWEIF_FIX_<STUFE>_STARTEN.bat aus SCHWEIF_STARTEN.bat: Fix-EXE als SCHWEIF.EXE nach Build\\Fix_<Stufe>
    kopieren (DOS: 8.3-Namen) und dort starten, Spielstaende in Build\\Testspielstaende_Fix_<Stufe>"""
    s = src.replace('\r\n', '\n'); St = stufe.capitalize()
    rep = [('rem  SCHWEIF_STARTEN.bat - startet das aus dem Quellcode gebaute\n'
            'rem  Sternenschweif (Build\\SCHWEIF.EXE) mit den GOG-Spieldaten.\n'
            'rem  Das Original im Spielordner wird nicht angefasst.\n'
            'rem  Spielstaende landen in Build\\Testspielstaende, nicht bei\n'
            'rem  deinen echten Spielstaenden.',
            f'rem  SCHWEIF_FIX_{stufe}_STARTEN.bat - startet die Fix-Fassung\n'
            f'rem  Build\\SCHWEIF_FIX_{stufe}.EXE ({beschr})\n'
            'rem  mit den GOG-Spieldaten. Das Original im Spielordner wird nicht\n'
            f'rem  angefasst. Die EXE wird als SCHWEIF.EXE nach Build\\Fix_{St}\n'
            'rem  kopiert (DOS kennt nur 8.3-Namen). Spielstaende landen in\n'
            f'rem  Build\\Testspielstaende_Fix_{St}, getrennt von den echten\n'
            'rem  Spielstaenden und von den Tests der anderen Fassungen.\n'
            f'rem  Gebaut wird sie mit Bauen\\BAUEN.bat {stufe.lower()}.'),
           ('set "SAVES=%BUILD%\\Testspielstaende"', f'set "SAVES=%BUILD%\\Testspielstaende_Fix_{St}"\n'
            f'set "FIXEXE=%BUILD%\\SCHWEIF_FIX_{stufe}.EXE"\nset "RUN=%BUILD%\\Fix_{St}"'),
           ('set "CONF=%TEMP%\\dosbox_schweif_build.conf"', f'set "CONF=%TEMP%\\dosbox_schweif_fix_{stufe.lower()}.conf"'),
           ('if not exist "%BUILD%\\SCHWEIF.EXE" goto keinexe', 'if not exist "%FIXEXE%" goto keinexe'),
           ('if not exist "%SAVES%" mkdir "%SAVES%"', 'if not exist "%SAVES%" mkdir "%SAVES%"\n'
            'if not exist "%RUN%" mkdir "%RUN%"\ncopy /y "%FIXEXE%" "%RUN%\\SCHWEIF.EXE" >nul'),
           ('rem     E: = unser Build', f'rem     E: = Build\\Fix_{St} (die Fix-EXE als SCHWEIF.EXE)'),
           ('>>"%CONF%" echo mount E "%BUILD%"', '>>"%CONF%" echo mount E "%RUN%"'),
           ('>>"%CONF%" echo echo SCHWEIF.EXE (aus dem Quellcode gebaut) wurde beendet. Taste druecken zum Schliessen.',
            f'>>"%CONF%" echo echo SCHWEIF_FIX_{stufe}.EXE (Fix-Fassung) wurde beendet. Taste druecken zum Schliessen.'),
           ('echo Build\\SCHWEIF.EXE fehlt neben dieser Datei. Erst Bauen\\BAUEN.bat ausfuehren.',
            f'echo Build\\SCHWEIF_FIX_{stufe}.EXE fehlt. Erst Bauen\\BAUEN.bat {stufe.lower()} ausfuehren.')]
    for a, b in rep:
        if a not in s: raise SystemExit(f'SCHWEIF_STARTEN.bat: Stelle fehlt: {a[:60]!r}')
        s = s.replace(a, b)
    return s.replace('\n', '\r\n')


def startdateien():
    for f in ('bauen.py', 'BAUEN.md', 'SCHWEIF_STARTEN.bat', 'SPIELPFAD_SUCHEN.cmd'):
        if not os.path.isfile(os.path.join(Z, f)): raise SystemExit(f'{f} fehlt im Paketordner {Z}')
    if 'SPIELPFAD_SUCHEN.cmd' not in BAUEN_BAT: raise SystemExit('BAUEN_BAT ohne SPIELPFAD_SUCHEN.cmd')
    open(os.path.join(Z, 'BAUEN.bat'), 'w', encoding='latin1', newline='').write(BAUEN_BAT.replace('\n', '\r\n'))
    src = open(os.path.join(Z, 'SCHWEIF_STARTEN.bat'), encoding='latin1', newline='').read()
    for st, b in (('WICHTIG', 'Fehlerbehebungen kritisch + mittel'), ('ALLES', 'alle Fehlerbehebungen')):
        open(os.path.join(Z, f'SCHWEIF_FIX_{st}_STARTEN.bat'), 'w', encoding='latin1', newline='').write(fix_starter(src, st, b))
    print('BAUEN.bat, SCHWEIF_FIX_WICHTIG_STARTEN.bat, SCHWEIF_FIX_ALLES_STARTEN.bat geschrieben')


main()
startdateien()
