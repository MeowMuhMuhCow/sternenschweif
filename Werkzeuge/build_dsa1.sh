#!/bin/bash
# build_dsa1.sh - Schicksalsklinge aus Quelle/ komplett bauen und pruefen.
#
#   1. Arbeitskopie von Quelle/ + Build-Patches (datseg_patch.py) + Korrekturen (be_fixes.py)
#   2. alle 111 Segmente mit Borland C++ 3.1 / TASM uebersetzen (bc_ready.bat)
#   3. Segmentvergleich gegen Original/SCHICKM.EXE            -> segmente.txt
#   4. Linknamen angleichen (objsym.py: Prototyp-Namen, _ds -> DATASEG@)
#   5. Linklauf 1 (liefert MAP und die Liste fehlender Relocations)
#   6. objpuborder.py: Overlay-Einsprünge in Attics Reihenfolge
#   7. objfixgen.py:   fest eingetragene Segmentwerte -> echte Verweise
#   8. Linklauf 2 -> BLADEM.EXE, Start-SP wie im Original (exemod_stack.py)
#   9. Pruefungen: Variablenadressen, Einsprünge, Relocations, Byte-Vergleich
#
# Aufruf: build_dsa1.sh <Projektordner-Kopie in der Cloud>
# Voraussetzung: Werkbank aus Q 004 (setup_werkbank.sh), Compiler unter ~/sternenschweif/bc
# Quelle/ bleibt unveraendert; alle Aenderungen nur in der Arbeitskopie ~/sternenschweif/dsa1
set -e
R=$HOME/sternenschweif
P=${1:?Projektordner angeben}
T=$(cd "$(dirname "$0")" && pwd)
O="$P/Original/SCHICKM.EXE"
W=$R/dsa1
rm -rf $W && mkdir -p $W && cp -r "$P/Quelle/." $W/
cd $W
chmod -R u+w .
echo "== Patches"
python3 -I $T/datseg_patch.py .
python3 -I $T/be_fixes.py .
sed 's/\.\.\\BIN\\/..\\bc\\BIN\\/g' bc_ready.bat | grep -v '^TLINK' > BCR.BAT
echo "== Kompilieren"
DOSTIMEOUT=1500 bash $T/dosrun.sh dsa1 'CALL BCR.BAT' | grep -iE '^(error|fatal)' || true
echo "$(ls SEG*.OBJ | wc -l) OBJ-Dateien"
echo "== Segmentvergleich (Code-Bytes, Verweise ausgeblendet)"
python3 -I $T/omfcmp.py "$O" SEG*.OBJ > segmente.txt 2>&1 || true
awk '{print $5}' segmente.txt | sort | uniq -c
echo "== Linknamen"
python3 -I $T/objsym.py autofix SEG*.OBJ | wc -l | sed 's/$/ Prototyp-Namen angeglichen/'
n=0
for o in SEG*.OBJ; do
  if python3 -I $T/objsym.py list $o > syms.tmp && grep -q '^EXT _ds$' syms.tmp; then
    python3 -I $T/objsym.py rename $o _ds DATASEG@ >/dev/null; n=$((n+1))
  fi
done
rm -f syms.tmp
echo "$n OBJ-Dateien: _ds -> DATASEG@"
echo "== Linklauf 1"
BE_DATE=1994-01-12 DOSTIMEOUT=600 bash $T/dosrun.sh dsa1 '..\bc\BIN\TLINK.EXE @TLINK.RES' | grep -iE 'error|warning' || echo "TLINK ohne Fehler"
cp BLADEM.EXE LAUF1.EXE; cp BLADEM.MAP LAUF1.MAP
echo "== Overlay-Einsprünge und Verweise"
python3 -I $T/objpuborder.py "$O" LAUF1.MAP .
python3 -I $T/objfixgen.py "$O" LAUF1.EXE LAUF1.MAP .
echo "== Linklauf 2"
BE_DATE=1994-01-12 DOSTIMEOUT=600 bash $T/dosrun.sh dsa1 '..\bc\BIN\TLINK.EXE @TLINK.RES' | grep -iE 'error|warning' || echo "TLINK ohne Fehler"
python3 -I $T/exemod_stack.py BLADEM.EXE 1000
echo "== Pruefungen"
python3 -I $T/mapcheck.py datseg.cpp BLADEM.MAP | head -3
python3 -I $T/stubcmp.py BLADEM.EXE "$O" BLADEM.MAP | tail -1
python3 -I $T/segdiff.py BLADEM.EXE "$O" segmente.txt | tail -6
python3 -I $T/exediff.py BLADEM.EXE "$O" BLADEM.MAP
python3 -I $T/relcmp.py BLADEM.EXE "$O"
md5sum BLADEM.EXE "$O"
