#!/usr/bin/env python3
"""asmauto.py SEGID BASIS [MAXRUNDEN] - ruft asm2tasm.py so lange auf, bis bytegleich;
abweichende Befehle werden dabei einzeln auf db umgestellt."""
import sys, subprocess, os, re
sid,base=sys.argv[1],sys.argv[2]; mx=int(sys.argv[3]) if len(sys.argv)>3 else 40
force=[]
for k in range(mx):
    env=dict(os.environ,FORCE_DB=','.join(force))
    r=subprocess.run(['python3','/root/sternenschweif/tools/asm2tasm.py',sid,base],capture_output=True,text=True,env=env)
    out=r.stdout+r.stderr
    if 'GLEICH' in out: print(f'Runde {k+1}: GLEICH, db-Befehle: {len(force)} {force}'); break
    errl=[int(x) for x in re.findall(r'\*\*Error\*\* \S+\((\d+)\)',out)]
    if errl:
        num=int(sid[1:]); _ovl=sid[0]=='O'; _ap=(f'/root/sternenschweif/o{num:03d}/OVL{num:03d}.ASM' if _ovl else (f'/root/sternenschweif/l{num:02d}/SEG{num:03d}.ASM' if os.environ.get('LNK') else f'/root/sternenschweif/a{num:02d}/SEG{num:03d}.ASM')); L=open(_ap,errors='replace').read().split('\n')
        new=[]
        for n in errl:
            mm=re.search(r';@([0-9a-f]+)',L[n-1])
            if mm and mm.group(1) not in force: new.append(mm.group(1))
        if not new: print(out); break
        force+=new; continue
    m=re.search(r'ERSTE_ABW_BEFEHL (0x[0-9a-f]+)',out)
    if not m: print(out); break
    a=m.group(1)[2:]
    if a in force: print('haengt bei',a); print(out); break
    force.append(a)
else:
    print('nicht fertig nach',mx,'Runden', force)
