#!/usr/bin/env python3
"""optsuch.py DIR DATEI.C SEGID BASIS [FUNKTIONSWEISE] - probiert Compiler-Optionen durch (ein DOSBox-Lauf mit BAT)
und meldet die Varianten mit den wenigsten Abweichungen gegen das Original-Segment."""
import sys, itertools, subprocess, os, re
d,src,sid,base=sys.argv[1:5]
ROOT='/root/sternenschweif'
flags=os.environ.get('FLAGS','-O -Oe -Ot -Ol -Og -Oi -Z -k-').split()
combos=[]
for cpu in os.environ.get('CPUS','-1 -3').split():
    for k in range(0,len(flags)+1):
        for c in itertools.combinations(flags,k): combos.append((cpu,)+c)
lines=[]
for i,c in enumerate(combos):
    lines.append(f'..\\bc\\BIN\\BCC.EXE -mlarge {os.environ.get("BASE","")} {" ".join(c)} -c -Y -I..\\bc\\INCLUDE -E..\\bc\\BIN\\TASM.EXE -oV{i:03d}.OBJ {src} > NUL')
open(f'{ROOT}/{d}/VARS.BAT','w',newline='\r\n').write('\n'.join(lines)+'\nexit\n')
subprocess.run(['bash',f'{ROOT}/tools/dosrun.sh',d,'VARS.BAT'],capture_output=True,timeout=3000)
res=[]
for i,c in enumerate(combos):
    o=f'{ROOT}/{d}/V{i:03d}.OBJ'
    if not os.path.exists(o): continue
    out=subprocess.run(['python3','-I',f'{ROOT}/tools/segvgl.py',o,sid,base],capture_output=True,text=True).stdout
    m=re.search(r'neu (\d+) Bytes, Original ab \S+ noch (\d+) Bytes, Abweichungen (\d+)',out)
    if m: res.append((int(m.group(3))+abs(int(m.group(1))-int(m.group(2)))*4,int(m.group(1)),int(m.group(2)),int(m.group(3)),' '.join(c)))
    os.remove(o)
res.sort()
for r in res[:12]: print(f'Abw {r[3]:4}  neu {r[1]} / orig {r[2]}  {r[4]}')
