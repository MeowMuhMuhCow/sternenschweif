#!/bin/bash
# vartest.sh DIR SEGID ORIGSTART OPTS VARFILE...  - jede Datei enthaelt eine Funktion; kompiliert alle, vergleicht
d=$1; sid=$2; st=$3; opts=$4; shift 4
: > ~/sternenschweif/$d/V.BAT; i=0
for f in "$@"; do printf '..\\bc\\BIN\\BCC.EXE -mlarge -3 %s -c -Y -oW%02d.OBJ %s > W%02d.TXT\r\n' "$opts" $i "$f" $i >> ~/sternenschweif/$d/V.BAT; i=$((i+1)); done
printf "exit\r\n" >> ~/sternenschweif/$d/V.BAT
timeout 600 bash ~/sternenschweif/tools/dosrun.sh $d V.BAT >/dev/null
i=0; for f in "$@"; do o=$(printf '%s/W%02d.OBJ' ~/sternenschweif/$d $i); grep -i error $(printf '%s/W%02d.TXT' ~/sternenschweif/$d $i)
 echo "== $f: $(NEARSKIP=1 python3 -I ~/sternenschweif/tools/fcmp.py $o $sid $st | head -1)"; i=$((i+1)); done
