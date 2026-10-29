#!/bin/bash
# dsall.sh [MODUL...] - Phase B neu anwenden (z. B. nach einer Namenswelle): fuer alle Module mit Spalte D in
# c_module.tsv (oder die angegebenen) die Daten per tools/dsconv.py in den C-Quelltext schreiben.
# Voraussetzung: csrc/ enthaelt die Fassung OHNE eigene Daten (extern-Deklarationen) und cbau/ den Stand eines
# build.sh-Laufs mit genau diesen Quellen (cbau/<m>.OBJ + .FIX; dafuer vorher z. B. mit NUR_D_AUS=1 bauen, s. u.).
# Handarbeit vor dsconv (Daten, die das Werkzeug nicht selbst erkennt):
#   O199  g_opt_sub_val/g_opt_sub_img0/1 sind Felder von g_opt_entries[3] -> im Code ersetzen (sed unten)
#   O071  g_gather_herbs_table hat 13 Eintraege (Endemarke 0xff) -> names/daten/manuell_n16.tsv + s_data.h [13]
# Danach: tools/dsb.sh <Module> (Einzeltest) bzw. tools/build.sh (Komplettbuild, dscheck je D-Modul).
ROOT=/root/sternenschweif; cd $ROOT
dn() { case $1 in O*) echo $1;; *) printf 'R%02d\n' $((10#$1));; esac; }
nm() { case $1 in O*) echo OVL${1#O};; *) echo SEG$1;; esac; }
L="$*"; [ -n "$L" ] || L=$(grep -v '^#' c_module.tsv | awk -F'\t' '$3 ~ /^D/{print $1}')
ok=0; fail=""
for n in $L; do m=$(nm $n); f=$(ls csrc/$m.C* | head -1)
  grep -q 'Phase B: eigene' $f && { echo "$n: schon umgesetzt"; continue; }
  if [ $n = O199 ]; then
    sed -i -E '/^\s*\/?\*/!{s/\bg_opt_sub_val\b/g_opt_entries[3].val/g; s/\bg_opt_sub_img0\b/g_opt_entries[3].img[0]/g; s/\bg_opt_sub_img1\b/g_opt_entries[3].img[1]/g}' $f
    sed -i -E 's/^(\s*)\*g_opt_sub_val\b/\1*g_opt_entries[3].val/' $f
    bash tools/dsprep.sh O199 > /dev/null; OWN=$ROOT/dsc
  else OWN=$ROOT/cbau; fi
  if OWNDIR=$OWN python3 -I tools/dsconv.py $(dn $n) --write > dsb/conv_$n.txt 2>&1; then ok=$((ok+1)); else fail="$fail $n"; grep -A3 'NICHT\|Error\|von Hand' dsb/conv_$n.txt | head -4; fi
done
echo "umgesetzt: $ok, nicht automatisch:$fail"
