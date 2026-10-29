#!/bin/bash
# regen.sh LISTE W N - erzeugt die Segmente aus LISTE neu (Arbeiter W von N)
ROOT=/root/sternenschweif; cd $ROOT; LOG=$ROOT/regen_$W$3.tsv
W=$2; N=$3; LOG=$ROOT/regen_${W}.tsv; : > $LOG; i=0
for s in $(cat $1); do
  i=$((i+1)); [ $((i % N)) -ne $W ] && continue
  if [ ${s:0:1} = R ]; then env="LNK=1"; else env=""; fi
  r=$(env $env timeout 2400 python3 -I tools/asmauto.py $s 0 200 2>&1 | grep -oE "Runde [0-9]+: GLEICH, db-Befehle: [0-9]+|nicht fertig nach [0-9]+ Runden|haengt bei [0-9a-f]+" | head -1)
  echo -e "$s\t${r:-FEHLER}" >> $LOG
done
echo ENDE >> $LOG
