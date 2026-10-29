#!/bin/bash
# c.sh SRC "opts"  -> compiliert SRC in $WOV (Umgebungsvariable), zeigt Fehler, ccmp
cd /root/sternenschweif/${WOV:?WOV=Arbeitsordner setzen}
S=$1; O=$2; B=${S%.*}
rm -f $B.OBJ
printf '..\\bc\\BIN\\BCC.EXE -mlarge %s -c -Y -E..\\bc\\BIN\\TASM.EXE %s > O.TXT\r\nexit\r\n' "$O" "$S" > C.BAT
bash ../tools/dosrun.sh $WOV C.BAT >/dev/null
grep -i -E "error|warn" O.TXT | tr -d '\r'
cd ..; python3 -I tools/ccmp.py ref_obj/$B.OBJ $WOV/$B.OBJ | tail -${N:-5}
