#!/bin/bash
# fd.sh NNN STARTS(hex,komma) -> sd2 je Funktion
N=$1; IFS=, read -ra S <<< "$2"; S+=(ffff)
for ((k=0;k<${#S[@]}-1;k++)); do
 r=$(python3 -I /root/sternenschweif/tools/sd2.py $N ${S[k]} ${S[k+1]} ${3:-${WOV:-.}/OVL$N.OBJ} 2>&1)
 [ -n "$r" ] && { echo "=== f_${S[k]}"; echo "$r" | head -${L:-40}; }
done
