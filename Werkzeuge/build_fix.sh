#!/bin/bash
# build_fix.sh - Fix-Modus des Baus: Fehlerbehebungen aus names/inc/s_fix.h einschalten (aus tools/build.sh mit FIX=1
# aufgerufen, oder allein nach einem vollstaendigen Standardbau, z. B. FIXFLAGS="-DFIX_BUGS=2" bash tools/build_fix.sh)
#    FIX=1 FIXFLAGS="-DFIX_BUGS=2" bash tools/build.sh      (Vorgabe FIXFLAGS: -DFIX_BUGS=1)
#    Der Standardbau (FIX_BUGS=0, bytegleich) liefert die Vergleichsfassung (cbau/*.OBJ, lnk/asm1). Danach werden nur
#    die Quellen mit "FIX_nnnn" neu uebersetzt (cfix/), per cobjfix --ref auf die Linknamen gebracht (Abbildung ueber
#    die Namen der Vergleichsfassung), residente Module auf ihre Originallaenge aufgefuellt (tools/segpad.py), die
#    Assembler-Module mit Schaltern (IF FIX_nnnn) mit TASM /dFIX_BUGS=.. neu uebersetzt und alles in einer Kopie von
#    lnk (lnkfix/) gelinkt.
#    Entfaellt (muss scheitern): der Bytevergleich der geaenderten Module mit der Assembler-Fassung (cvsasm).
#    Bleibt: dscheck (Datensegment unveraendert), pubdup, fixcheck (Lage der residenten Segmente, Datensegment,
#    Overlay-Einsprungpunkte). Ergebnis: fixexe/SCHWEIF_FIX_WICHTIG.EXE (FIX_BUGS=1) bzw. _ALLES (2) bzw. _<FIXNAME>.
ROOT=/root/sternenschweif; cd $ROOT; L=$ROOT/lnk
cmp -s $L/SCHW_CD.EXE orig/SCHWEIF.EXE || { echo "Fix-Modus: Standardbau (lnk/SCHW_CD.EXE) fehlt oder nicht bytegleich - Abbruch"; exit 1; }
[ -d $ROOT/cbau ] && [ -s $L/FULL2.RES ] || { echo "Fix-Modus: Standardbau (Stufe 2) fehlt - Abbruch"; exit 1; }
FIXFLAGS=${FIXFLAGS:--DFIX_BUGS=1}
case "$FIXFLAGS" in *FIX_BUGS=2*) FN=ALLES;; *FIX_BUGS=1*) FN=WICHTIG;; *) FN=EIGEN;; esac
FN=${FIXNAME:-$FN}
echo "Fix-Modus: $FIXFLAGS -> SCHWEIF_FIX_$FN.EXE"
nm() { case $1 in O*) echo OVL${1#O};; *) echo SEG$1;; esac; }
F=$ROOT/cfix; LF=$ROOT/lnkfix; rm -rf $F $LF; mkdir -p $F $LF $ROOT/fixexe
python3 -I tools/cnamen.py $ROOT/cbau $L/asm1 > $F/cnamen.tsv
# 7a. C-Module mit Schaltern neu uebersetzen (Bloecke zu 40 wie oben)
grep -v '^#' $ROOT/c_module.tsv | while IFS=$'\t' read n o d; do m=$(nm $n); f=$m.C; [ -f csrc/$m.CPP ] && f=$m.CPP
  grep -qE 'FIX_([0-9]|PROBENLOG)' csrc/$f || continue
  cp csrc/$f $F/
  printf '..\\bc\\BIN\\BCC.EXE -mlarge %s %s -c -Y -I..\\names\\inc -E..\\bc\\BIN\\TASM.EXE %s > %s.TXT\r\n' "$o" "$FIXFLAGS" $f $m
done > $F/C.BAT
echo "Fix-Modus: $(grep -c . $F/C.BAT) C-Module mit Schaltern"
split -l 40 -d $F/C.BAT $F/CB; rm -f $F/C.BAT
for b in $F/CB[0-9][0-9]; do bn=$(basename $b); printf 'exit\r\n' >> $b; mv $b $F/$bn.BAT
  DOSTIMEOUT=600 bash tools/dosrun.sh cfix $bn.BAT > /dev/null; done
grep -h -i '^error\|fatal' $F/*.TXT && { echo "Fix-Modus: Uebersetzungsfehler - Abbruch"; exit 1; }
# 7b. linkfertig machen: gleiche Bytes wie die Vergleichsfassung -> nichts tun; sonst cobjfix --ref (+ dscheck)
cp $L/* $LF/ 2>/dev/null
NG=0; NC=0
for src in $F/*.C $F/*.CPP; do [ -f "$src" ] || continue; m=$(basename ${src%.*}); n=${m#OVL}; [ "$n" = "$m" ] && n=${m#SEG} || n=O$n
  [ -f $F/$m.OBJ ] || { echo "$m.OBJ fehlt - Abbruch"; exit 1; }
  if python3 -I tools/objsame.py $F/$m.OBJ cbau/$m.OBJ; then
    NG=$((NG + 1)); continue; fi
  d=$(grep -P "^$n\t" c_module.tsv | cut -f3)
  if [ "${d#D}" != "$d" ]; then
    CNAMEN=$F/cnamen.tsv python3 -I tools/cobjfix_d.py --eigen $F/$m.OBJ $L/asm1/$m.OBJ $L/asm1/SCHW_CD.MAP $F/$m.FIX --ref cbau/$m.OBJ > $F/$m.D.TXT || { cat $F/$m.D.TXT; echo "cobjfix --ref $m - Abbruch"; exit 1; }
    python3 -I tools/dscheck.py $F/$m.FIX $L/asm1/SCHW_CD.MAP >> $F/$m.D.TXT || { cat $F/$m.D.TXT; echo "dscheck $m (Datensegment geaendert) - Abbruch"; exit 1; }
  else
    CNAMEN=$F/cnamen.tsv python3 -I tools/cobjfix_d.py $F/$m.OBJ $L/asm1/$m.OBJ $L/asm1/SCHW_CD.MAP $F/$m.FIX --ref cbau/$m.OBJ > $F/$m.D.TXT || { cat $F/$m.D.TXT; echo "cobjfix --ref $m - Abbruch"; exit 1; }
  fi
  grep -h 'WARNUNG' $F/$m.D.TXT
  case $m in SEG*) python3 -I tools/segpad.py $F/$m.FIX cbau/$m.FIX > $F/$m.PAD.TXT || { cat $F/$m.PAD.TXT; echo "segpad $m - Abbruch"; exit 1; };; esac
  cp $F/$m.FIX $LF/$m.OBJ; NC=$((NC + 1))
done
echo "Fix-Modus: $NC C-Module geaendert, $NG ohne Wirkung der Schalter (Code gleich)"
# 7c. Assembler-Module mit Schaltern (IF FIX_nnnn, names/inc/s_fix.inc): TASM /dFIX_BUGS=..
AF=$(echo "$FIXFLAGS" | sed 's/-D/\/d/g')
AM=$(grep -l '^IF FIX_[0-9]' l[0-9][0-9]/SEG0*.ASM | xargs -r -n1 basename | sed 's/\.ASM$//')
for m in $AM; do printf '..\\bc\\BIN\\TASM.EXE /ml %s %s.ASM,%s.FXO > %s.FXT\r\n' "$AF" $m $m $m; done > $LF/ASMFIX.BAT
printf 'exit\r\n' >> $LF/ASMFIX.BAT
DOSTIMEOUT=600 bash tools/dosrun.sh lnkfix ASMFIX.BAT > /dev/null
NA=0
for m in $AM; do
  [ -s $LF/$m.FXO ] || { cat $LF/$m.FXT; echo "TASM $m (Fix) - Abbruch"; exit 1; }
  if python3 -I tools/objsame.py $LF/$m.FXO $L/asm1/$m.OBJ; then
    continue; fi
  python3 -I tools/segpad.py $LF/$m.FXO $L/asm1/$m.OBJ > $LF/$m.PAD.TXT || { cat $LF/$m.PAD.TXT; echo "segpad $m - Abbruch"; exit 1; }
  cp $LF/$m.FXO $LF/$m.OBJ; NA=$((NA + 1))
done
echo "Fix-Modus: $NA Assembler-Module geaendert"
# 7d. Linken (FULL2.RES, BC.EXE /b wie oben) und pruefen
python3 -I tools/pubdup.py $LF/FULL2.RES || { echo "doppelte Publics - Abbruch"; exit 1; }
rm -f $LF/SCHW_CD.EXE
BE_DATE=1995-02-15 DOSTIMEOUT=1200 bash tools/dosrun.sh lnkfix LINK.BAT | tr -d '\r' | grep -v '^$' | grep -v '^Linking' > $LF/full_outfix.txt
echo "Fix-Modus: Linker-Meldungen: $(grep -c . $LF/full_outfix.txt)"; head -20 $LF/full_outfix.txt
[ -s $LF/SCHW_CD.EXE ] || { echo "Fix-Modus: keine EXE - Abbruch"; exit 1; }
cp $LF/SCHW_CD.EXE fixexe/SCHWEIF_FIX_$FN.EXE; cp $LF/SCHW_CD.MAP fixexe/SCHWEIF_FIX_$FN.MAP
python3 -I tools/fixcheck.py fixexe/SCHWEIF_FIX_$FN.EXE orig/SCHWEIF.EXE || { echo "Fix-Modus: fixcheck FEHLER"; exit 1; }
ls -la fixexe/SCHWEIF_FIX_$FN.EXE; md5sum fixexe/SCHWEIF_FIX_$FN.EXE
