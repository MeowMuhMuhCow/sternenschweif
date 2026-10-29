#!/bin/bash
# genall.sh WORKER ANZAHL - erzeugt Link-ASMs (R01-R46 im LNK-Modus, alle Overlays), verteilt auf Arbeiter
ROOT=/root/sternenschweif; cd $ROOT
W=$1; N=$2; LOG=$ROOT/gen_log_$W.tsv; touch $LOG
i=0
for s in $(seq -f "R%02g" 1 46) $(seq -f "O%03g" 0 270); do
  i=$((i+1)); [ $((i % N)) -ne $W ] && continue
  grep -q "^$s	Runde" $LOG && continue
  if [ ${s:0:1} = R ]; then env="LNK=1"; else env=""; fi
  out=$(env $env timeout 2400 python3 -I tools/asmauto.py $s 0 200 2>&1)
  r=$(echo "$out" | grep -oE "Runde [0-9]+: GLEICH, db-Befehle: [0-9]+|nicht fertig nach [0-9]+ Runden|haengt bei [0-9a-f]+" | head -1)
  echo -e "$s\t${r:-FEHLER}" >> $LOG
done
echo ENDE >> $LOG
