#!/bin/bash
# ovlbatch.sh START END - verarbeitet Overlay-Segmente mechanisch, protokolliert nach ovl_log.tsv
ROOT=/root/sternenschweif; cd $ROOT
LOG=$ROOT/ovl_log.tsv
s=${1:-0}; e=${2:-999}
while IFS=$'\t' read id sz; do
  num=$((10#${id#O}))
  [ $num -lt $s ] && continue
  [ $num -gt $e ] && break
  grep -q "^$id	" $LOG 2>/dev/null && continue
  if [ "$sz" -lt 16 ]; then echo -e "$id\t$sz\tSKIP\twinzig (Tail/Alignment)" >> $LOG; continue; fi
  out=$(timeout 1200 python3 -I tools/asmauto.py $id 0 120 2>&1)
  r=$(echo "$out" | grep -oE "Runde [0-9]+: GLEICH, db-Befehle: [0-9]+" | head -1)
  if [ -n "$r" ]; then
    db=$(echo "$r" | grep -oE "[0-9]+$")
    cp o$(printf %03d $num)/OVL$(printf %03d $num).ASM /mnt/attach/outputs/q/ovl/ 2>/dev/null
    echo -e "$id\t$sz\tGLEICH\t$db db" >> $LOG
  else
    fail=$(echo "$out" | grep -oE "nicht fertig nach [0-9]+ Runden|haengt bei [0-9a-fx]+|Error[^ ]*" | head -1)
    echo -e "$id\t$sz\tOFFEN\t${fail:-unbekannt}" >> $LOG
  fi
done < ovl_ids.txt
echo "BATCH-ENDE $s-$e"
