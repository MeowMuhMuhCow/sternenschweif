import sys, os, tempfile, collections, csv
sys.path.insert(0,'/root/sternenschweif/lib'); sys.path.insert(0,'/root/sternenschweif/tools'); sys.path.insert(0,'/root/sternenschweif/q004/zerl')
import libsplit, omfcmp, schweif
segs=schweif.load(sys.argv[2]); S={s['id']:s for s in segs}
hits=[]; nf=[]; tot=0
for name,blob in libsplit.split(sys.argv[1]):
    f=tempfile.NamedTemporaryFile(delete=False,suffix='.obj'); f.write(blob); f.close()
    try: parts=omfcmp.parse_obj(f.name)
    finally: os.unlink(f.name)
    for sn,cls,data,mask in parts:
        if cls!='CODE' or len(data)<4: continue
        tot+=1; found=None
        for s in segs:
            if s['kind']!='R': continue
            c=s['code']; n=len(data)
            first=next((i for i in range(n) if mask[i]),None)
            if first is None: continue
            p=c.find(bytes([data[first]]))
            while p!=-1:
                b=p-first
                if b>=0 and b+n<=len(c) and all(not mask[i] or c[b+i]==data[i] or (data[i]==0x9A and c[b+i]==0x90) for i in range(n)):
                    found=(s['id'],b); break
                p=c.find(bytes([data[first]]),p+1)
            if found: break
        (hits if found else nf).append((name,sn,len(data),found))
cov=collections.defaultdict(int)
for name,sn,n,(sid,b) in hits: cov[sid]+=n
print(f'Code-Teile in CL.LIB: {tot}, in SCHWEIF gefunden: {len(hits)}')
for sid in sorted(cov): print(f'  {sid}: {cov[sid]} von {len(S[sid]["code"])} Bytes durch Bibliotheksmodule belegt')
with open(sys.argv[3],'w') as f:
    for name,sn,n,(sid,b) in sorted(hits,key=lambda x:(x[3][0],x[3][1])): f.write(f'{sid}\t{b:#06x}\t{n}\t{name}\t{sn}\n')
