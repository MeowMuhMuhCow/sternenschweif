#!/bin/bash
# dsprep.sh MODUL... - uebersetzt Module aus csrc/ nach dsc/ und macht sie wie build.sh linkfertig (cobjfix_d, ohne
# eigene Daten) -> dsc/<m>.OBJ + .FIX; danach  OWNDIR=/root/sternenschweif/dsc python3 tools/dsconv.py MODUL
ROOT=/root/sternenschweif; cd $ROOT; L=$ROOT/lnk; B=$ROOT/dsc; mkdir -p $B
nm() { case $1 in O*) echo OVL${1#O};; *) echo SEG$1;; esac; }
: > $B/C.BAT
for n in "$@"; do m=$(nm $n); o=$(grep -P "^$n\t" c_module.tsv | cut -f2)
  cp csrc/$m.C* $B/; f=$m.C; [ -f $B/$m.CPP ] && f=$m.CPP; : > $B/$m.OBJ
  printf '..\\bc\\BIN\\BCC.EXE -mlarge %s -c -Y -I..\\names\\inc -E..\\bc\\BIN\\TASM.EXE %s > %s.TXT\r\n' "$o" $f $m >> $B/C.BAT
done
printf 'exit\r\n' >> $B/C.BAT
# Module aus tools/frisch.txt ("MODUL [VORLAUF...]") in eigenem frischen DOSBox-Lauf wie build.sh (BCC liest
# bei der switch-Registerwahl nicht initialisierten Speicher - Ergebnis haengt vom Vorlauf ab)
while read fm vor; do [ -z "$fm" ] && continue; grep -qi " $fm\.C" $B/C.BAT || continue
  { for v in $vor; do cp csrc/$v.C* $B/; fv=$v.C; [ -f $B/$v.CPP ] && fv=$v.CPP
      o=$(grep -P "^$(echo $v | sed -E 's/^OVL/O/; s/^SEG//')\t" c_module.tsv | cut -f2)
      printf '..\\bc\\BIN\\BCC.EXE -mlarge %s -c -Y -I..\\names\\inc -E..\\bc\\BIN\\TASM.EXE %s > VOR.TXT\r\n' "$o" $fv; done
    grep -i " $fm\.C" $B/C.BAT; printf 'exit\r\n'; } > $B/F_$fm.BAT
  grep -v -i " $fm\.C" $B/C.BAT > $B/C.TMP; mv $B/C.TMP $B/C.BAT
  DOSTIMEOUT=300 bash tools/dosrun.sh dsc F_$fm.BAT > /dev/null
done < <(tr -d '\r' < tools/frisch.txt 2>/dev/null)
DOSTIMEOUT=600 bash tools/dosrun.sh dsc C.BAT > /dev/null
for n in "$@"; do m=$(nm $n); grep -h -i '^error\|fatal' $B/$m.TXT
  python3 -I tools/cobjfix_d.py $B/$m.OBJ $L/asm1/$m.OBJ $L/asm1/SCHW_CD.MAP $B/$m.FIX | grep -i 'fehl\|NICHT' ; done
