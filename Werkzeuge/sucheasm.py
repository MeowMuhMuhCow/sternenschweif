import re, glob, sys, csv, collections
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); sys.path.insert(0,'/root/sternenschweif/tools')
import schweif, omfcmp
segs={s['id']:s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE')}
F=collections.defaultdict(list)
for r in csv.DictReader(open('/root/sternenschweif/q004/zerl/funktionen.tsv'),delimiter='\t'): F[r['segment']].append((int(r['start'],16),int(r['groesse'])))
def ins(b): return [re.sub(r'0x[0-9a-f]+','N',t) for _,_,t in schweif.disasm(bytes(b))]
import difflib
for mod in ('SEG008','SEG009','SEG010'):
    lst=[x for x in glob.glob('/root/sternenschweif/asm/*') if x.upper().endswith(mod+'.LST')][0]
    procs=[]
    for l in open(lst,encoding='latin1'):
        m=re.match(r'\s*\d+\s+([0-9A-F]{4})\s+.*?\b(\w+)\s+proc\b',l,re.I)
        if m: procs.append((int(m[1],16),m[2]))
    data=[c for c in omfcmp.parse_obj(f'/root/sternenschweif/asm/{mod}.OBJ') if c[1]=='CODE'][0][2]
    for i,(a,n) in enumerate(procs):
        b=procs[i+1][0] if i+1<len(procs) else len(data); A=ins(data[a:b]); best=(0,None)
        for sid,L in F.items():
            for st,sz in L:
                if not (0.5*(b-a) <= sz <= 2*(b-a)+8): continue
                B=ins(segs[sid]['code'][st:st+sz]); r=difflib.SequenceMatcher(None,A,B,autojunk=False).ratio()
                if r>best[0]: best=(r,f'{sid}+{st:#06x} ({sz} B)')
        print(f'{mod} {n:26} {b-a:5} B  -> {best[1]}  {best[0]:.2f}')
