#!/bin/bash
cd /root/sternenschweif
for n in $(seq -w 0 270); do python3 -I tools/objchunk.py lnk/OVL$n.OBJ O$n 2>&1 | grep -v Bloecke; done
echo fertig
