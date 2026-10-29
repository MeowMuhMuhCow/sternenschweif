#!/bin/bash
# gamerun2.sh <Befehl> [sek] [cue]  - wie GOG: SB16, 12000 Zyklen, CD
R=$HOME/sternenschweif; EXE=$1; T=${2:-30}; CUE=${3:-data.cue}
CONF=$(mktemp --suffix=.conf)
cat > $CONF <<C
[sdl]
output=surface
[dosbox]
machine=svga_s3
captures=/tmp/claude-0/shots
memsize=16
[cpu]
core=normal
cycles=fixed 12000
[mixer]
nosound=false
rate=44100
[sblaster]
sbtype=sb16
sbbase=220
irq=7
dma=1
hdma=5
[dos]
xms=true
ems=true
umb=true
[autoexec]
mount c $R/g1
imgmount d $R/g1/$CUE -t iso > C:\\MNT.TXT
c:
$EXE
echo ENDE
pause
exit
C
BE_WATCH=${WATCH} BE_INT21LOG=${I21:-} BE_TRACEIP=${TRACEIP:-0} BE_SHOTS=${SHOTS:-0} BE_AUTOKEY=${AUTOKEY:-0} BE_FILELOG=1 BE_NOHOOK=1 LD_LIBRARY_PATH=$R/sdlinst/lib SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout -s KILL $T $R/be-build/src/dosbox -conf $CONF -noconsole > /tmp/claude-0/game.log 2>&1
echo "rc=$? (137 = lief bis Timeout)"
echo "-- imgmount:"; tr -d '\r' < $R/g1/MNT.TXT; echo "-- Programm:"; tr -d '\r' < $R/g1/RUNOUT.TXT 2>/dev/null
rm -f $R/g1/MNT.TXT $R/g1/RUNOUT.TXT $CONF
