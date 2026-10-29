#!/usr/bin/env python3
"""aehnlichkeit.py - wie aehnlich sind die Funktionen aus OBJ-Dateien den Funktionen einer EXE?
Befehlsgeruest (Zahlen -> N, far/near-Aufruf gleichgesetzt), 4er-Befehlsfolgen, Jaccard.
Aufruf: aehnlichkeit.py ZIEL.EXE OBJ-Verzeichnis ergebnis.tsv
"""
import sys, re, subprocess, collections, glob, os, tempfile, struct
sys.path.insert(0,'/root/sternenschweif/tools')
import omfcmp
exec(open('/root/sternenschweif/tools/funcmatch.py').read().split('\nexe = open')[0].split('import omfcmp\n',1)[1])
def norm(t):
    t=re.sub(r'0x[0-9a-f]+|\b\d+\b','N',t)
    t=t.replace('call N:N','CALL').replace('pop cx','add sp,byte +N') if False else t
    return t
def canon(v):
    out=[];i=0
    while i<len(v):
        if v[i]=='nop' and i+2<len(v) and v[i+1]=='push cs' and v[i+2].startswith('call'):
            out.append('call N:N'); i+=3; continue
        if v[i]=='push cs' and i+1<len(v) and v[i+1]=='call N':
            out.append('call N:N'); i+=2; continue
        out.append(v[i]); i+=1
    return out
def dis(b):
    f=tempfile.NamedTemporaryFile(delete=False); f.write(b); f.close()
    out=subprocess.run(['ndisasm','-b16',f.name],capture_output=True,text=True).stdout; os.unlink(f.name)
    return canon([norm(m[3]) for m in (re.match(r'([0-9A-F]{8})\s+([0-9A-F]+)\s+(.*)',l) for l in out.splitlines()) if m])
def exe_funcs(path):
    d=open(path,'rb').read();hdr=int.from_bytes(d[8:10],'little')*16
    starts=[];p=hdr
    while True:
        p=d.find(b'\x55\x8b\xec',p)
        if p<0: break
        starts.append(p);p+=1
    args=['ndisasm','-b16']
    for s in starts: args+=['-s',str(s)]
    out=subprocess.run(args+[path],capture_output=True,text=True).stdout
    si=set(starts);res={};cur=None
    for l in out.splitlines():
        m=re.match(r'([0-9A-F]{8})\s+([0-9A-F]+)\s+(.*)',l)
        if not m: continue
        off=int(m[1],16);txt=m[3]
        if off in si: cur=off;res[cur]=[]
        if cur is not None:
            res[cur].append(norm(txt))
            if txt.startswith(('retf','ret')) and len(res[cur])>3: cur=None
    return {k:canon(v) for k,v in res.items() if len(v)>=6}
K=4
sh=lambda v:set(tuple(v[i:i+K]) for i in range(len(v)-K+1))
B=exe_funcs(sys.argv[1]); SB={k:sh(v) for k,v in B.items()}
inv=collections.defaultdict(list)
for k,s in SB.items():
    for g in s: inv[g].append(k)
bins=collections.Counter(); wins=collections.Counter(); rows=[]
for obj in sorted(glob.glob(sys.argv[2]+'/SEG*.OBJ')):
    code=[c for c in omfcmp.parse_obj(obj) if c[1]=='CODE']
    if not code: continue
    data=code[0][2]; pubs=publics(obj)
    for i,(off,name) in enumerate(pubs):
        end=pubs[i+1][0] if i+1<len(pubs) else len(data)
        v=dis(bytes(data[off:end]))
        if len(v)<6: continue
        s=sh(v); cnt=collections.Counter()
        for g in s:
            lst=inv.get(g,())
            if len(lst)>200: continue
            for b in lst: cnt[b]+=1
        best=0;bb=None
        for b,c in cnt.most_common(5):
            j=c/len(s|SB[b])
            if j>best: best,bb=j,b
        bk=int(best*10)/10; bins[bk]+=1; wins[bk]+=len(v)
        rows.append((os.path.basename(obj),name,len(v),round(best,2),hex(bb) if bb else '-'))
tot=sum(wins.values())
for b in sorted(bins,reverse=True):
    print(f'Aehnlichkeit >= {b:.1f}: {bins[b]:5d} Funktionen, {100*wins[b]/tot:5.1f} % der Befehle')
with open(sys.argv[3],'w') as f:
    for r in rows: f.write('\t'.join(map(str,r))+'\n')
