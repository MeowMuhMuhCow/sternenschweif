#!/bin/bash
# link_einrichten.sh - Komplettlink in einer neuen Sitzung einrichten und SCHWEIF.EXE bytegleich bauen.
#
# Aufruf: link_einrichten.sh <Q004-Kopie in der Cloud>   (z.B. "/mnt/user-data/uploads/Q 004 DSA Sternenschweif")
# Voraussetzungen: Werkbank steht (setup_werkbank.sh), Compiler unter ~/sternenschweif/bc
#   inkl. BC.EXE (IDE-Linker) und PRJ2MAK.EXE, Werkzeuge unter ~/sternenschweif/tools.
# Benoetigt aus Q004: Quelle/Link/link_quellen.tar.gz (oder die entpackten Ordner), Quelle/SEG*.C,
#   Analyse/Zerlegung/ (funktionen.tsv ...), Werkzeuge/schweif.py, Original/SCHWEIF.EXE
set -e
P=${1:?Q004-Kopie angeben}
R=$HOME/sternenschweif
mkdir -p $R/orig $R/q004/zerl $R/lnk $R/dsg
cp "$P/Original/SCHWEIF.EXE" $R/orig/
cp -r "$P/Analyse/Zerlegung/." $R/q004/zerl/
cp "$P/Werkzeuge/schweif.py" "$P/Werkzeuge/ovlstruct.py" "$P/Werkzeuge/namen.py" $R/q004/zerl/
T=$(mktemp -d)
if [ -f "$P/Quelle/Link/link_quellen.tar.gz" ]; then
  tar -xzf "$P/Quelle/Link/link_quellen.tar.gz" -C $T
else
  cp -r "$P/Quelle/Link/." $T/
fi
for f in $T/Resident/SEG0*.ASM; do n=$(basename $f .ASM); n=${n#SEG0}; mkdir -p $R/l$n; cp $f $R/l$n/; done
for f in $T/Overlays/OVL*.ASM; do n=$(basename $f .ASM); n=${n#OVL}; mkdir -p $R/o$n; cp $f $R/o$n/; done
cp $T/Daten/DS0*.ASM $T/Daten/DSBSS.ASM $R/dsg/
cp $T/Daten/INIT.ASM $T/Daten/FARDATA.ASM $R/lnk/
cp $T/Tabellen/* $R/
mkdir -p $R/lib; mv $R/lib_syms.tsv $R/lib/   # nur fuer asm2tasm (Neuerzeugung)
rm -rf $T
# fertige C-Module (Stufe 2 in build.sh)
mkdir -p $R/csrc
# gemeinsame Header (Quelle/inc: schweif.h, s_*.h)
mkdir -p $R/names/inc; [ -d "$P/Quelle/inc" ] && cp "$P/Quelle/inc/"*.h $R/names/inc/
for n in $(grep -v '^#' $R/c_module.tsv | cut -f1); do
  case $n in O*) m=OVL${n#O}; d="$P/Quelle/Overlays";; *) m=SEG$n; d="$P/Quelle";; esac
  if [ -f "$d/$m.CPP" ]; then cp "$d/$m.CPP" $R/csrc/; else cp "$d/$m.C" $R/csrc/; fi
done
echo "eingerichtet: $(ls -d $R/l?? | wc -l) residente, $(ls -d $R/o??? | wc -l) Overlay-Module"
bash $R/tools/build.sh
