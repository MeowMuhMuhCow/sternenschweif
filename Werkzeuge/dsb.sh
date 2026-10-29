#!/bin/bash
# dsb.sh MODUL... - Phase B Einzeltest (ohne Komplettbuild): uebersetzt die Module aus csrc/ (Optionen aus c_module.tsv),
# cobjfix_d.py --eigen gegen die ASM-Fassung aus lnk/asm1 (vom letzten build.sh) und dscheck.py.
# Module ohne Spalte D werden ohne eigene Daten geprueft (ALS_D=1: alle als D-Module, z. B. vor dmark.sh).
# LINK=1: danach Stufe-2-Link mit den uebrigen OBJ aus lnk/ (Stand des letzten build.sh) und diesen Modulen als
#         D-Module (FULL2.RES ohne ihre Stuecke) - Ergebnis bytegleich?  Module in Spalte D von c_module.tsv
#         werden beim Link immer als D-Module behandelt.
ROOT=/root/sternenschweif; cd $ROOT; L=$ROOT/lnk; B=$ROOT/dsb; mkdir -p $B
nm() { case $1 in O*) echo OVL${1#O};; *) echo SEG$1;; esac; }
dn() { case $1 in O*) echo $1;; *) printf 'R%02d\n' $((10#$1));; esac; }
: > $B/C.BAT
for n in "$@"; do m=$(nm $n); o=$(grep -P "^$n\t" c_module.tsv | cut -f2)
  cp csrc/$m.C* $B/; f=$m.C; [ -f $B/$m.CPP ] && f=$m.CPP
  : > $B/$m.OBJ
  printf '..\\bc\\BIN\\BCC.EXE -mlarge %s -c -Y -I..\\names\\inc -E..\\bc\\BIN\\TASM.EXE %s > %s.TXT\r\n' "$o" $f $m >> $B/C.BAT
done
printf 'exit\r\n' >> $B/C.BAT
# Module aus tools/frisch.txt ("MODUL [VORLAUF...]") in eigenem frischen DOSBox-Lauf wie build.sh (BCC liest
# bei der switch-Registerwahl nicht initialisierten Speicher - Ergebnis haengt vom Vorlauf ab)
while read fm vor; do [ -z "$fm" ] && continue; grep -qi " $fm\.C" $B/C.BAT || continue
  { for v in $vor; do case $v in @*) [ -s $B/FILL.COM ] || { cp tools/fill/FILL.ASM $B/
          printf '..\\bc\\BIN\\TASM.EXE FILL.ASM > FILL.TXT\r\n..\\bc\\BIN\\TLINK.EXE /t FILL.OBJ > FILLL.TXT\r\nexit\r\n' > $B/FILL.BAT
          DOSTIMEOUT=120 bash tools/dosrun.sh dsb FILL.BAT > /dev/null; }
        python3 -I tools/heapbild.py $v $B/LOAD.BIN; continue;; esac   # Speicherbild (tools/heapbild.py)
      cp csrc/$v.C* $B/; fv=$v.C; [ -f $B/$v.CPP ] && fv=$v.CPP
      o=$(grep -P "^$(echo $v | sed -E 's/^OVL/O/; s/^SEG//')\t" c_module.tsv | cut -f2)
      printf '..\\bc\\BIN\\BCC.EXE -mlarge %s -c -Y -I..\\names\\inc -E..\\bc\\BIN\\TASM.EXE %s > VOR.TXT\r\n' "$o" $fv; done
    grep -i " $fm\.C" $B/C.BAT; printf 'exit\r\n'; } > $B/F_$fm.BAT
  grep -v -i " $fm\.C" $B/C.BAT > $B/C.TMP; mv $B/C.TMP $B/C.BAT
  DOSTIMEOUT=300 bash tools/dosrun.sh dsb F_$fm.BAT > /dev/null
done < <(tr -d '\r' < tools/frisch.txt 2>/dev/null)
DOSTIMEOUT=600 bash tools/dosrun.sh dsb C.BAT > /dev/null
ok=1; dl=$(grep -v '^#' c_module.tsv | awk -F'\t' '$3 ~ /^D/{print $1}' | tr '\n' ' ')
for n in "$@"; do m=$(nm $n)
  grep -h -i '^error\|fatal' $B/$m.TXT
  if echo " $dl " | grep -q " $n " || [ -n "$ALS_D" ]; then
    python3 -I tools/cobjfix_d.py --eigen $B/$m.OBJ $L/asm1/$m.OBJ $L/asm1/SCHW_CD.MAP $B/$m.FIX || { ok=0; continue; }
    python3 -I tools/dscheck.py $B/$m.FIX $L/asm1/SCHW_CD.MAP || ok=0
    dl="$dl $n"
  else   # Modul ohne Spalte D (z. B. nur Namen geaendert): wie build.sh ohne eigene Daten
    python3 -I tools/cobjfix_d.py $B/$m.OBJ $L/asm1/$m.OBJ $L/asm1/SCHW_CD.MAP $B/$m.FIX > /dev/null || { ok=0; continue; }
  fi
  CDIR=$B python3 -I tools/cvsasm.py $n 2>&1 | sed 's/^/  /'
done
[ -n "$LINK" ] || exit $((1 - ok))
[ $ok = 1 ] || { echo "Link uebersprungen (Fehler)"; exit 1; }
for n in "$@"; do m=$(nm $n); cp $B/$m.FIX $L/$m.OBJ; done
# jedes D-Modul muss in lnk/ seine eigenen Daten mitbringen (sonst fehlt sein Stueck im DS)
python3 -I - $dl <<'PY' || { echo "Link uebersprungen: D-Module ohne eigene Daten in lnk/ (mit uebergeben)"; exit 1; }
import sys; sys.path.insert(0, '/root/sternenschweif/tools'); import dsinfo, cobjfix_d as CD
bad = []
for n in sys.argv[1:]:
    m = n if n[0] == 'O' else 'R%02d' % int(n); fn = 'OVL' + n[1:] if n[0] == 'O' else 'SEG%03d' % int(n)
    if any(dsinfo.own(m).values()) and not CD.own_segments('/root/sternenschweif/lnk/%s.OBJ' % fn): bad.append(n)
if bad: print('ohne eigene Daten in lnk/:', ' '.join(sorted(set(bad)))); sys.exit(1)
PY
python3 -I tools/dmgen.py --dmodule $(for n in $dl; do dn $n; done | sort -u | tr '\n' ',') | tail -1
# Reststuecke (DR*) und neue/geaenderte Datenstuecke (DM* ohne aktuelle OBJ, z. B. nach korrekturen.tsv) assemblieren
NEU=$(for f in $(cat $L/dm_liste.txt); do case $f in DR*) echo $f;; *) [ $L/$f.OBJ -nt $L/$f.ASM ] || echo $f;; esac; done)
if [ -n "$NEU" ]; then
  { for f in $NEU; do echo "..\\bc\\BIN\\TASM.EXE /ml /m2 $f.ASM > NUL"; done; echo exit; } | sed 's/$/\r/' > $L/ASMDR.BAT
  DOSTIMEOUT=300 bash tools/dosrun.sh lnk ASMDR.BAT > /dev/null
  for f in $NEU; do python3 -I tools/objorder.py $L/$f.OBJ $L/dm.json $f; done | grep -v Bloecke
fi
python3 -I tools/mkprj.py FULL2.RES > /dev/null
python3 -I tools/pubdup.py $L/FULL2.RES || { echo "doppelte Publics - Link uebersprungen"; exit 1; }
printf '%s\r\n' '..\bc\BIN\BC.EXE /b SCHW_CD.PRJ' 'exit' > $L/LINK.BAT
rm -f $L/SCHW_CD.EXE
BE_DATE=1995-02-15 DOSTIMEOUT=1200 bash tools/dosrun.sh lnk LINK.BAT | tr -d '\r' | grep -v '^$' | grep -v '^Linking' > $L/full_out2.txt
echo "Linker-Meldungen: $(grep -c . $L/full_out2.txt)"; head -10 $L/full_out2.txt
md5sum $L/SCHW_CD.EXE
cmp $L/SCHW_CD.EXE orig/SCHWEIF.EXE && echo "BYTEGLEICH mit dem Original" || { python3 -I tools/exediff.py $L/SCHW_CD.EXE orig/SCHWEIF.EXE 2>/dev/null | head -20; exit 1; }
