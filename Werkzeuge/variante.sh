#!/bin/bash
# variante.sh NAME "BCC-Optionen"  - alle DSA-1-C-Segmente mit anderen Optionen uebersetzen
# und die Funktionen in SCHWEIF.EXE suchen (funcmatch). Ergebnis: opt/NAME.tsv + NAME.sum
# NAME hoechstens 6 Zeichen (Ordner v_NAME muss ein DOS-8.3-Name sein!), -mlarge steht schon in BCR.BAT.
# SEGS="SEG002|SEG056" schraenkt auf einzelne Module ein (Regex).
set -e
R=$HOME/sternenschweif; N=$1; OPT=$2
W=$R/v_$N; rm -rf $W; mkdir -p $W
cp -r $R/dsa1/. $W/; cd $W; rm -f *.OBJ *.obj
grep "BCC.EXE" BCR.BAT | grep -E "${SEGS:-.}" | sed "s/-O- -c -1/$OPT -c/" > V.BAT
DOSTIMEOUT=2400 bash $R/tools/dosrun.sh v_$N 'CALL V.BAT' | grep -iE '^(error|fatal)' | head -3 || true
mkdir -p o && mv SEG*.OBJ o/ 2>/dev/null || true
python3 -I $R/tools/funcmatch.py $R/orig/SCHWEIF.EXE o > $R/q004/opt/$N.tsv 2> $R/q004/opt/$N.sum
echo "$N [$OPT] OBJ=$(ls o | wc -l)"; cat $R/q004/opt/$N.sum
