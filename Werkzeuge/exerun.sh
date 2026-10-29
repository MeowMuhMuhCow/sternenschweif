#!/bin/bash
# exerun.sh EXE [sekunden] [ordner] - startet eine SCHWEIF-EXE headless in DOSBox OHNE Spieldaten (Startprobe fuer
# die Fix-EXEs). Messfuehler des gepatchten DOSBox (be-build): BE_FILELOG (Dateizugriffe, Vektoren, TERMINATE),
# BE_TRACEIP (Ablaufspur alle 200 ms, Falle fuer far-Spruenge ins Leere [FAR], Division durch 0 [DIV0]).
# Ausgabe: Log nach <ordner>/run.log (Standard: Ordner gtest/<EXE-Name>), Zusammenfassung auf stdout.
R=/root/sternenschweif; EXE=$1; T=${2:-25}; N=$(basename "$EXE" .EXE)
W=${3:-$R/gtest/$N}; rm -rf "$W"; mkdir -p "$W"; cp "$EXE" "$W/SCHWEIF.EXE"
# Attrappe: PATH.CFG zeigt auf D:, D: ist ein leeres ISO-Abbild mit dem Label der Spiel-CD (tools/mkiso.py) - cd_check
# besteht, danach fehlen die Spieldaten: der Start laeuft bis zum ersten Datei-/Ladefehler
printf 'D' > "$W/PATH.CFG"; python3 -I $R/tools/mkiso.py "$W/CD.ISO" DSA2_CD1
CONF=$(mktemp --suffix=.conf)
cat > $CONF <<C
[sdl]
output=surface
[dosbox]
machine=svga_s3
memsize=16
[cpu]
core=normal
cycles=fixed 30000
[mixer]
nosound=true
[sblaster]
sbtype=none
[dos]
xms=true
ems=true
umb=true
[autoexec]
mount c $W
imgmount d $W/CD.ISO -t iso
c:
SCHWEIF.EXE
echo EXITCODE %ERRORLEVEL% > RUNOUT.TXT
exit
C
START=$(date +%s.%N)
BE_FILELOG=1 BE_NOHOOK=1 BE_TRACEIP=200 LD_LIBRARY_PATH=$R/sdlinst/lib SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  timeout -s KILL $T $R/be-build/src/dosbox -conf $CONF -noconsole > "$W/run.log" 2>&1
RC=$?
rm -f $CONF
echo "$N: rc=$RC (137 = lief bis Timeout), $(echo "$(date +%s.%N) - $START" | bc | cut -c1-5) s"
tr -d '\r' < "$W/RUNOUT.TXT" 2>/dev/null
grep -c '^\[DOS\] OPEN' "$W/run.log" | sed 's/^/  Dateioeffnungen: /'
grep -m3 '^\[FAR\]\|^\[DIV0\]' "$W/run.log" | sed 's/^/  /'
grep -m1 'TERMINATE' "$W/run.log" | sed 's/^/  /'
grep -iE 'illegal|unhandled|exit to error' "$W/run.log" | head -3 | sed 's/^/  /'
