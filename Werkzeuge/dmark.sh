#!/bin/bash
# dmark.sh MODUL... - setzt Spalte D in c_module.tsv (Phase B: Modul definiert seine Daten selbst)
cd /root/sternenschweif
for n in "$@"; do sed -i -E "s/^($n)\t([^\t]*)\$/\1\t\2\tD/" c_module.tsv; done
echo "D-Module: $(grep -cP '\tD$' c_module.tsv)"
