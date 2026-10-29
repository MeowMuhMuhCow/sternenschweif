import sys, tempfile, os, struct
sys.path.insert(0,'/root/sternenschweif/lib'); sys.path.insert(0,'/root/sternenschweif/tools')
import libsplit, omfcmp
d=open('/root/sternenschweif/orig/SCHWEIF.EXE','rb').read(); h=struct.unpack_from('<H',d,8)[0]*16
DG=d[h+0x24eb*16:h+0x24eb*16+52238]
LO,HI=int(sys.argv[1],16),int(sys.argv[2],16)
mods=dict(libsplit.split('/root/sternenschweif/bc/LIB/CL.LIB')); mods.update(libsplit.split('/root/sternenschweif/bc/LIB/OVERLAY.LIB'))
for name,blob in sorted(mods.items()):
    f=tempfile.NamedTemporaryFile(delete=False,suffix='.obj'); f.write(blob); f.close()
    try: parts=omfcmp.parse_obj(f.name)
    except Exception as e: parts=[]; print('ERR',name,e)
    os.unlink(f.name)
    for sn,cls,data,mask in parts:
        if cls not in ('DATA','INITDATA','EXITDATA') or len(data)<1: continue
        n=len(data); hits=[b for b in range(LO,HI-n+1) if all(not mask[i] or DG[b+i]==data[i] for i in range(n))]
        if hits and len(hits)<12: print(name,sn,cls,n,' '.join(hex(x) for x in hits))
