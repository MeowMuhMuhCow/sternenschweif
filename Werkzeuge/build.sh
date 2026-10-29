#!/bin/bash
# build.sh - Komplett-Link SCHWEIF.EXE aus den erzeugten Modulen
# Datensegment (Phase A, dsana/PLAN.md): je Originalmodul ein Datenstueck lnk/DM<modul>.ASM, gelinkt direkt
# hinter seinem Modul in der verzahnten Original-Linkreihenfolge (tools/dmgen.py -> FULL.RES).
# Phase B: Module mit Spalte D in c_module.tsv definieren ihre Daten selbst (C-OBJ behaelt _DATA/_BSS,
# tools/cobjfix_d.py --eigen + tools/dscheck.py), ihr Stueck entfaellt im Stufe-2-Link (FULL2.RES).
ROOT=/root/sternenschweif; cd $ROOT; L=$ROOT/lnk
# 1. Overlays: nur Einsprünge exportieren (Original-Reihenfolge), _main
python3 -I tools/ovlpub.py o*/OVL*.ASM
python3 -I tools/aliases.py
# 2. Verweise auf main umbenennen
grep -l 'F_O032_0000' l*/SEG0*.ASM o*/OVL*.ASM dsg/DS*.ASM 2>/dev/null | xargs -r sed -i 's/\bF_O032_0000\b/_main/g'
# 3. alles nach lnk kopieren und dort assemblieren
for n in $(seq -w 1 46); do cp l$n/SEG0$n.ASM $L/; done
for n in $(seq -w 0 270); do cp o$n/OVL$n.ASM $L/ 2>/dev/null; done
python3 -I tools/dmgen.py > $L/dmgen.txt || { cat $L/dmgen.txt; echo "dmgen fehlgeschlagen - Abbruch"; exit 1; }
tail -1 $L/dmgen.txt
{ for f in $(cd $L; ls SEG0*.ASM OVL*.ASM); do echo "..\\bc\\BIN\\TASM.EXE /ml $f > NUL"; done
  for f in $(cat $L/dm_liste.txt); do echo "..\\bc\\BIN\\TASM.EXE /ml /m2 $f.ASM > NUL"; done
  echo "..\\bc\\BIN\\TASM.EXE /ml INIT.ASM > NUL"; echo "..\\bc\\BIN\\TASM.EXE /ml FARDATA.ASM > NUL"; echo exit; } | sed 's/$/\r/' > $L/ASMALL.BAT
rm -f $L/*.OBJ
DOSTIMEOUT=3000 bash tools/dosrun.sh lnk ASMALL.BAT > /dev/null
echo "OBJ: $(ls $L/*.OBJ | wc -l)"
for f in $(cat $L/dm_liste.txt); do [ -f $L/$f.OBJ ] || { echo "$f.ASM nicht assembliert - Abbruch"; exit 1; }; done
python3 -I tools/ovlobjfix.py
bash tools/ovlchunk.sh
for f in $(cat $L/dm_liste.txt); do python3 -I tools/objorder.py $L/$f.OBJ $L/dm.json $f; done | grep -v Bloecke
for n in $(seq -w 1 46); do python3 -I tools/objorder.py $L/SEG0$n.OBJ - R$n; done | grep -v Bloecke
# 4. Antwortdatei: FULL.RES (alle Stuecke, Stufe 1) und FULL2.RES (ohne Stuecke der D-Module) kommen aus dmgen.py
# 5. Linken wie Attic: mit der IDE (BC.EXE /b), die den EXE-Namen gross schreibt.
#    BUILD_TLINK=1 linkt stattdessen mit TLINK 5.1 (identisch bis auf den kleingeschriebenen Namen im _EXEINFO_)
rm -f $L/SCHW_CD.EXE
if [ -n "$BUILD_TLINK" ]; then
  printf '%s\r\n' '..\bc\BIN\TLINK.EXE /c /s /m @FULL.RES' 'exit' > $L/LINK.BAT
  BE_DATE=1995-02-15 DOSTIMEOUT=1200 bash tools/dosrun.sh lnk LINK.BAT | tr -d '\r' | grep -v '^$' > $L/full_out.txt
else
  python3 -I tools/mkprj.py FULL.RES > /dev/null
  printf '%s\r\n' '..\bc\BIN\BC.EXE /b SCHW_CD.PRJ' 'exit' > $L/LINK.BAT
  BE_DATE=1995-02-15 DOSTIMEOUT=1200 bash tools/dosrun.sh lnk LINK.BAT | tr -d '\r' | grep -v '^$' | grep -v '^Linking' > $L/full_out.txt
fi
echo "Linker-Meldungen: $(grep -c . $L/full_out.txt)"; head -20 $L/full_out.txt
cmp -s $L/SCHW_CD.EXE orig/SCHWEIF.EXE && echo "Stufe 1 (alles Assembler): BYTEGLEICH" || echo "Stufe 1 (alles Assembler): ABWEICHUNG"
# ASM-Fassung der OBJ sichern (Vergleichsbasis fuer cobjfix/dscheck, auch fuer Einzeltests mit tools/dsb.sh)
mkdir -p $L/asm1; cp $L/*.OBJ $L/SCHW_CD.MAP $L/asm1/
# 6. Stufe 2: fertige C-Module (c_module.tsv, Quellen in csrc/) statt der Assembler-Fassung
if [ -s $ROOT/c_module.tsv ] && [ -z "$NUR_ASM" ]; then
  C=$ROOT/cbau; rm -rf $C; mkdir -p $C; cp $ROOT/csrc/*.C* $C/
  nm() { case $1 in O*) echo OVL${1#O};; *) echo SEG$1;; esac; }
  grep -v '^#' $ROOT/c_module.tsv | while IFS=$'\t' read n o d; do
    m=$(nm $n); f=$m.C; [ -f $C/$m.CPP ] && f=$m.CPP
    printf '..\\bc\\BIN\\BCC.EXE -mlarge %s -c -Y -I..\\names\\inc -E..\\bc\\BIN\\TASM.EXE %s > %s.TXT\r\n' "$o" $f $m; done > $C/C.BAT
  # in Bloecken zu 40 Modulen je DOSBox-Lauf: in einem langen Lauf aendert BCC nach ~150 Uebersetzungen
  # sein Ergebnis (OVL199 nach OVL198 2 Bytes laenger, Speicher-/Ressourcenverlust ueber die Laeufe)
  # Module aus tools/frisch.txt in eigenem DOSBox-Lauf (frischer Speicher): BCC liest bei der switch-Registerwahl
  # offenbar nicht initialisierten Speicher, OVL199 wird direkt nach OVL198 sonst 2 Bytes laenger (mov ax,di/mov bx,ax)
  # Zeile "MODUL [VORLAUF...]": die Vorlauf-Module werden im selben frischen Lauf direkt davor uebersetzt (ihr
  # Speicherrest bestimmt die switch-Registerwahl: OVL030 brauchte OVL198 davor fuer mov ax,si/dec ax/mov bx,ax).
  # Die Vorlauf-Module bleiben zusaetzlich in den normalen Bloecken (deren OBJ wird danach ueberschrieben).
  # Vorlauf "@KKKK:ooo=ww,..." statt eines Moduls: Speicherbild laden (tools/heapbild.py, tools/fill/FILL.ASM) -
  # unabhaengig von den Kopfdateien, die ein Vorlauf-Modul einliest (OVL030: "@0115:2e8=8b,2f8=17").
  if grep -q ' @' $ROOT/tools/frisch.txt 2>/dev/null; then cp tools/fill/FILL.ASM $C/
    printf '..\\bc\\BIN\\TASM.EXE FILL.ASM > FILL.TXT\r\n..\\bc\\BIN\\TLINK.EXE /t FILL.OBJ > FILLL.TXT\r\nexit\r\n' > $C/FILL.BAT
    DOSTIMEOUT=120 bash tools/dosrun.sh cbau FILL.BAT > /dev/null; [ -s $C/FILL.COM ] || { echo "FILL.COM fehlt - Abbruch"; exit 1; }; fi
  while read fm vor; do [ -z "$fm" ] && continue
    { for v in $vor; do case $v in @*) python3 -I tools/heapbild.py $v $C/LOAD.BIN;; *) grep -i " $v\.C" $C/C.BAT;; esac; done
      grep -i " $fm\.C" $C/C.BAT; printf 'exit\r\n'; } > $C/F_$fm.BAT
    grep -v -i " $fm\.C" $C/C.BAT > $C/C.TMP; mv $C/C.TMP $C/C.BAT; DOSTIMEOUT=300 bash tools/dosrun.sh cbau F_$fm.BAT > /dev/null; done < <(cat $ROOT/tools/frisch.txt 2>/dev/null | tr -d '\r')
  split -l 40 -d $C/C.BAT $C/CB; rm -f $C/C.BAT
  for b in $C/CB[0-9][0-9]; do bn=$(basename $b); printf 'exit\r\n' >> $b; mv $b $C/$bn.BAT
    DOSTIMEOUT=600 bash tools/dosrun.sh cbau $bn.BAT > /dev/null; done
  grep -h -i '^error\|fatal' $C/*.TXT
  grep -v '^#' $ROOT/c_module.tsv | while IFS=$'\t' read n o d; do m=$(nm $n)
    if [ "${d#D}" != "$d" ]; then   # D oder DI
      python3 -I tools/cobjfix_d.py --eigen $C/$m.OBJ $L/$m.OBJ $L/SCHW_CD.MAP $C/$m.FIX > $C/$m.D.TXT || { cat $C/$m.D.TXT; echo "cobjfix --eigen $m fehlgeschlagen - Abbruch"; exit 1; }
      python3 -I tools/dscheck.py $C/$m.FIX $L/SCHW_CD.MAP >> $C/$m.D.TXT || { cat $C/$m.D.TXT; echo "dscheck $m fehlgeschlagen - Abbruch"; exit 1; }
    else
      python3 -I tools/cobjfix_d.py $C/$m.OBJ $L/$m.OBJ $L/SCHW_CD.MAP $C/$m.FIX > /dev/null || { echo "cobjfix $m fehlgeschlagen - Abbruch"; exit 1; }
    fi
  done || exit 1
  CDIR=$C python3 -I tools/cvsasm.py $(grep -v '^#' $ROOT/c_module.tsv | cut -f1 | tr '\n' ' ') | sed 's|^|  |' > $C/vergleich.txt
  grep -v GLEICH $C/vergleich.txt
  cat $C/*.D.TXT 2>/dev/null | grep '^dscheck' > $C/dscheck.txt; [ -s $C/dscheck.txt ] && echo "Phase B: $(grep -c . $C/dscheck.txt) Module mit eigenen Daten (dscheck GLEICH)"
  for n in $(grep -v '^#' $ROOT/c_module.tsv | cut -f1); do m=$(nm $n); cp $C/$m.FIX $L/$m.OBJ; done
  python3 -I tools/mkprj.py FULL2.RES > /dev/null
  python3 -I tools/pubdup.py $L/FULL2.RES || { echo "doppelte Publics (BC.EXE /b linkt dann stumm nicht) - Abbruch"; exit 1; }
  rm -f $L/SCHW_CD.EXE
  BE_DATE=1995-02-15 DOSTIMEOUT=1200 bash tools/dosrun.sh lnk LINK.BAT | tr -d '\r' | grep -v '^$' | grep -v '^Linking' > $L/full_out2.txt
  echo "Stufe 2: $(grep -vc '^#' $ROOT/c_module.tsv) Module aus C, Linker-Meldungen: $(grep -c . $L/full_out2.txt)"; head -20 $L/full_out2.txt
fi
ls -la $L/SCHW_CD.EXE
md5sum $L/SCHW_CD.EXE orig/SCHWEIF.EXE
cmp $L/SCHW_CD.EXE orig/SCHWEIF.EXE && echo "BYTEGLEICH mit dem Original"
# 7. Fix-Modus (FIX=1, FIXFLAGS): Fehlerbehebungen per Schalter - siehe tools/build_fix.sh
[ -n "$FIX" ] && exec bash tools/build_fix.sh
exit 0
