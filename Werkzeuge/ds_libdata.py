import sys, tempfile, os, struct, collections
sys.path.insert(0,'/root/sternenschweif/lib'); sys.path.insert(0,'/root/sternenschweif/tools')
import libsplit, omfcmp
exe=sys.argv[1]; dgpara=int(sys.argv[2],16); dglen=int(sys.argv[3])
d=open(exe,'rb').read(); h=struct.unpack_from('<H',d,8)[0]*16
DG=d[h+dgpara*16:h+dgpara*16+dglen]
used=set(l.split('\t')[3] for l in open(sys.argv[4]))
mods=dict(libsplit.split('/root/sternenschweif/bc/LIB/CL.LIB')); mods.update(libsplit.split('/root/sternenschweif/bc/LIB/OVERLAY.LIB'))
def segdefs(blob):
    p=0; ln=['']; out=[]
    def idx(b,q):
        v=b[q]; return ((((v&0x7F)<<8)|b[q+1]),q+2) if v&0x80 else (v,q+1)
    while p<len(blob):
        t=blob[p]; n=struct.unpack_from('<H',blob,p+1)[0]; b=blob[p+3:p+3+n-1]; p+=3+n
        if t==0x96:
            q=0
            while q<len(b): k=b[q]; ln.append(b[q+1:q+1+k].decode('latin1')); q+=1+k
        elif t==0x98:
            acbp=b[0]; q=1+(3 if (acbp>>5)==0 else 0); L=struct.unpack_from('<H',b,q)[0]; q+=2; ni,q=idx(b,q); ci,q=idx(b,q); out.append((ln[ni],ln[ci],L))
    return out
res=[]
for name in sorted(used):
    blob=mods.get(name)
    if not blob: continue
    f=tempfile.NamedTemporaryFile(delete=False,suffix='.obj'); f.write(blob); f.close()
    try: parts=omfcmp.parse_obj(f.name)
    except Exception: parts=[]
    os.unlink(f.name)
    sd=segdefs(blob)
    bss=sum(L for n,c,L in sd if c=='BSS')
    for sn,cls,data,mask in parts:
        if cls!='DATA' or len(data)<2: continue
        n=len(data); hits=[]
        first=next((i for i in range(n) if mask[i]),0)
        p=DG.find(bytes([data[first]]))
        while p!=-1 and len(hits)<3:
            b=p-first
            if b>=0 and b+n<=len(DG) and all(not mask[i] or DG[b+i]==data[i] for i in range(n)): hits.append(b)
            p=DG.find(bytes([data[first]]),p+1)
        res.append((hits[0] if len(hits)==1 else None, name, sn, n, len(hits), bss))
res.sort(key=lambda r:(r[0] is None, r[0] or 0))
for r in res: print(f'{r[0]:#06x}' if r[0] is not None else '  ?   ', r[1], r[2], r[3], 'Treffer', r[4], 'BSS', r[5])
