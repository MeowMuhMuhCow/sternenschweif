"""namen_inhalt.py - uebertraegt DSA-1-Variablennamen ueber gleichen Dateninhalt (Anfangswerte) nach SCHWEIF."""
import sys, struct, collections, math
sys.path.insert(0,'/root/sternenschweif/ds'); import dsa1_vars
def dgroup(path):
    d=open(path,'rb').read(); h=struct.unpack_from('<H',d,8)[0]*16
    p=d.find(b'Borland C++ - Copyright 1991',h); base=p-4
    assert (base-h)%16==0
    return d[base:base+0x10000]
D1=dgroup('/root/sternenschweif/orig/SCHICKM.EXE'); D2=dgroup('/root/sternenschweif/orig/SCHWEIF.EXE')
G0,G1=0x94,0x86b4
V=[v for v in dsa1_vars.load() if v['addr']<0xbc00]   # DSA1 Spiel-_DATA endet vor Bibliothek (0xB4E0)
V=[v for v in V if v['addr']<0xb4e0]
def ent(b):
    c=collections.Counter(b); n=len(b); return -sum(k/n*math.log2(k/n) for k in c.values())
res=[]
for v in V:
    a,n=v['addr'],v['size']
    if n<3: continue
    b=D1[a:a+n]
    isstr='char' in v['type'] and v['type'].strip() in ('char','signed char','unsigned char','const char') and b.find(b'\0')>0
    key=b[:b.index(b'\0')+1] if isstr else b[:32]
    if len(key)<3 or (not isstr and (len(set(key))<3 or ent(key)<1.5)): continue
    hits=[]; p=D2.find(key,G0)
    while p!=-1 and p<G1 and len(hits)<3:
        hits.append(p); p=D2.find(key,p+1)
    if len(hits)==1:
        full=D2[hits[0]:hits[0]+n]==b
        res.append((hits[0],v['name'],a,n,len(key),'voll' if full else 'Anfang', 'str' if isstr else 'dat'))
res.sort()
with open('/root/sternenschweif/ds/namen_inhalt.tsv','w') as f:
    f.write('adresse_dsa2\tname\tadresse_dsa1\tgroesse_dsa1\tschluessel\tuebereinstimmung\tart\n')
    for r in res: f.write(f'{r[0]:#06x}\t{r[1]}\t{r[2]:#06x}\t{r[3]}\t{r[4]}\t{r[5]}\t{r[6]}\n')
print('Treffer',len(res),collections.Counter((r[5],r[6]) for r in res))
for r in res[::max(1,len(res)//40)]: print(hex(r[0]),r[1],hex(r[2]),r[3],r[5],r[6])
