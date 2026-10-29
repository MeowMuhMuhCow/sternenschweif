#!/usr/bin/env python3
import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket
"""ovlobjfix.py - ordnet in allen Overlay-OBJs die PUBDEFs so, dass TLINK die Stub-Einsprünge in Original-Reihenfolge vergibt
(TLINK zaehlt in umgekehrter PUBDEF-Reihenfolge)"""
import sys, subprocess
sys.path.insert(0,_SW + '/q004/zerl'); import schweif
S={s['id']:s for s in schweif.load(_SW + '/orig/SCHWEIF.EXE')}
ALIAS={'F_O032_0000':'_main'}
for n in range(271):
    sid=f'O{n:03d}'
    names=[ALIAS.get(f'F_{sid}_{e:04X}',f'F_{sid}_{e:04X}') for e in S[sid]['entries']][::-1]
    subprocess.run([sys.executable,'-I','-X','utf8',_SW + '/tools/objpub.py',_SW + f'/lnk/OVL{n:03d}.OBJ',','.join(names)],capture_output=True)
print('fertig')
