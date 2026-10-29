#!/bin/bash
# gamerun.sh <EXE> [sekunden]  - startet ein Spiel-EXE in ~/sternenschweif/g1 headless, mit CD
R=$HOME/sternenschweif; EXE=$1; T=${2:-20}
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
[autoexec]
mount c $R/g1
imgmount d $R/g1/game.inst -t iso
c:
$EXE
echo EXITCODE %ERRORLEVEL% >> RUNOUT.TXT
exit
C
START=$(date +%s.%N)
BE_FILELOG=1 BE_NOHOOK=1 LD_LIBRARY_PATH=$R/sdlinst/lib SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout -s KILL $T $R/be-build/src/dosbox -conf $CONF -noconsole > /tmp/claude-0/game.log 2>&1
RC=$?
echo "Laufzeit $(echo "$(date +%s.%N) - $START" | bc) s, rc=$RC (124 = lief bis Timeout)"
tr -d '\r' < $R/g1/RUNOUT.TXT 2>/dev/null; rm -f $R/g1/RUNOUT.TXT $CONF
grep -iE 'error|exit|illegal|unhandled' /tmp/claude-0/game.log | head -5
