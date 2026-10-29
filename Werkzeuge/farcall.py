#!/usr/bin/env python3
"""farcall.py - stellt in allen Overlay-ASMs die BCC-Fernaufrufe wieder her: an jeder Stelle mit 90 0E E8 (von TLINK
umgewandelter Fernaufruf) wird 'db 9Ah / dd Ziel' ausgegeben. TLINK erzeugt daraus dieselben Bytes und reserviert
wie im Original 2 Bytes Relocation-Platz je Aufruf."""
import sys,re,struct
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); import schweif
S={s['id']:s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE')}
def H(v): return f'0{v:x}h' if v>9 else str(v)
def db(c,a,b): return '\tdb\t'+','.join(H(x) for x in c[a:b])
tot=0; probs=[]
import json
_d=open('/root/sternenschweif/orig/SCHWEIF.EXE','rb').read(); _h=8192
_rg=json.load(open('/root/sternenschweif/lnk_ranges.json'))['mods']
args=sys.argv[1:] or [f'O{n:03d}' for n in range(271)]
for sid in args:
    n=int(sid[1:])
    if sid[0]=='O':
        code=S[sid]['code']; f=f'/root/sternenschweif/o{n:03d}/OVL{n:03d}.ASM'; base=0
    else:
        base,end=_rg[sid]; p0=S[sid]['para']*16; code=_d[_h+p0:_h+p0+end]; f=f'/root/sternenschweif/l{n:02d}/SEG{n:03d}.ASM'
    L=open(f,newline='').read().split('\r\n')
    ins=[(o+base,ln,t) for o,ln,t in schweif.disasm(code[base:])]
    sites=[ins[j][0] for j in range(len(ins)-2) if ins[j][2]=='nop' and ins[j+1][2]=='push cs' and ins[j+2][2].startswith('call ')]
    # Zeilengruppen nach @-Offset
    body_start=next(i for i,l in enumerate(L) if '_TEXT\tsegment' in l)+2
    groups=[]  # [start, [line indices]]
    labels={}  # offset -> label line index
    pending=[]
    for i in range(body_start,len(L)):
        l=L[i]
        if l.endswith('_TEXT\tends') : break
        m=re.search(r';@([0-9a-f]+)$',l)
        if m:
            o=int(m.group(1),16)
            if groups and groups[-1][0]==o: groups[-1][1].append(i)
            else: groups.append([o,[i]])
        elif re.match(r'(L_[0-9a-f]+):$',l) or '\tlabel\tfar' in l:
            pending.append(i)
    starts=[g[0] for g in groups]
    labline={}
    for i in range(body_start,len(L)):
        mm=re.match(r'(L_([0-9a-f]+)):$',L[i])
        if mm: labline[int(mm.group(2),16)]=mm.group(1)
    repl={}  # erster Zeilenindex -> neue Zeilen; zu loeschende Zeilen
    dele=set(); newlabels={}
    for p in sites:
        tgt=(p+5+struct.unpack_from('<h',code,p+3)[0])&0xffff
        # Gruppen, die [p,p+5) ueberdecken
        gi=[k for k,g in enumerate(groups) if g[0]<p+5 and (groups[k+1][0] if k+1<len(groups) else len(code))>p]
        if not gi: probs.append((sid,p,'keine Gruppe')); continue
        a=groups[gi[0]][0]; b=groups[gi[-1]+1][0] if gi[-1]+1<len(groups) else len(code)
        inner=[o for o in labline if a<o<b and o!=p]
        if inner: probs.append((sid,p,'Marke innen',inner)); continue
        if tgt not in labline:
            if tgt in starts: newlabels[tgt]=f'L_{tgt:04x}'; labline[tgt]=f'L_{tgt:04x}'
            else: probs.append((sid,p,'Ziel ohne Grenze',hex(tgt))); continue
        out=[]
        if a<p:
            if a==p-2 and code[a]==0xEB:
                t2=(a+2+struct.unpack_from('<b',code,a+1)[0])&0xffff
                out.append((f'\tjmp\tshort {labline[t2]}' if t2 in labline else db(code,a,p))+f'\t;@{a:04x}')
            else: out.append(db(code,a,p)+f'\t;@{a:04x}')
        lab_p = labline.get(p) if p>a else None
        if lab_p and a<p: out.append(f'{lab_p}:')
        out.append(f'\tdb\t09ah\t; call far {labline[tgt]} (TLINK: nop/push cs/call near)\t;@{p:04x}')
        out.append(f'\tdd\t{labline[tgt]}\t;@{p:04x}')
        if p+5<b: out.append(db(code,p+5,b)+f'\t;@{p+5:04x}')
        idxs=[i for k in gi for i in groups[k][1]]
        repl[idxs[0]]=out; dele|=set(idxs[1:])
        if lab_p and a<p:
            for i in range(idxs[0],idxs[-1]+1):
                if L[i]==f'{lab_p}:': dele.add(i)
        tot+=1
    # neue Marken vor Gruppen einsetzen
    addlab={groups[k][1][0]:f'{newlabels[o]}:' for k,(o,_) in enumerate(groups) if o in newlabels}
    out=[]
    for i,l in enumerate(L):
        if i in addlab: out.append(addlab[i])
        if i in dele: continue
        if i in repl: out+=repl[i]; continue
        out.append(l)
    open(f,'w',newline='').write('\r\n'.join(out))
print('umgewandelt',tot,'Probleme',len(probs)); [print(' ',x) for x in probs[:20]]
