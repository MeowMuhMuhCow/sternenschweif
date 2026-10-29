import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket
"""items.py - liest dsg/DS00..05 + DSBSS: Labels mit Adresse, Typ, Laenge, Bytes, Zeigerziele."""
import re, struct
R=_SW + ''
def exe_ds():
    e=open(R+'/orig/SCHWEIF.EXE','rb').read()
    hdr=struct.unpack_from('<H',e,8)[0]*16
    return e[hdr+0x24EB0:hdr+0x24EB0+0x8b4e]
def parse_db(s):
    out=[]; s=s.strip()
    toks=[]; i=0; cur=''
    while i<len(s):
        c=s[i]
        if c=="'":
            j=s.index("'",i+1)
            while j+1<len(s) and s[j+1]=="'": j=s.index("'",j+2)
            cur+=s[i:j+1]; i=j+1
        elif c==',': toks.append(cur.strip()); cur=''; i+=1
        else: cur+=c; i+=1
    if cur.strip(): toks.append(cur.strip())
    for t in toks:
        if t.startswith("'"): out+=list(t[1:-1].replace("''","'").encode('latin1'))
        elif re.match(r'^\d+ dup \(\?\)$',t): out+=[None]*int(t.split()[0])
        elif t.lower().endswith('h'): out.append(int(t[:-1],16))
        else: out.append(int(t))
    return out
def load(bss=True):
    files=[f'{R}/dsg/DS0{n}.ASM' for n in range(6)]+([f'{R}/dsg/DSBSS.ASM'] if bss else [])
    items=[]
    for fn in files:
        kind='BSS' if 'BSS' in fn else 'DATA'
        ins=False
        for l in open(fn,encoding='latin1'):
            l=l.rstrip('\r\n')
            if l.startswith('_DATA\tsegment') or l.startswith('_BSS\tsegment'):
                ins=(kind=='DATA')==l.startswith('_DATA'); continue
            if not ins: continue
            if l.startswith('_DATA\tends') or l.startswith('_BSS\tends'): ins=False; continue
            m=re.match(r'(\S+)\tlabel\t(\S+)\t; 0x([0-9a-f]+) (\S+) (\d+)$',l) or re.match(r'(\S+)\tlabel\t(\S+)\t; 0x([0-9a-f]+) ()()',l)
            if m:
                items.append(dict(name=m.group(1),addr=int(m.group(3),16),typ=m.group(4) or m.group(2),size=int(m.group(5) or 0),
                                  kind=kind,data=[],dptr=[],cptr=[])); continue
            if l.startswith('\tpublic'): continue
            it=items[-1]
            m=re.match(r'\tdb\t(.*)',l)
            if m: it['data']+=parse_db(m.group(1)); continue
            m=re.match(r'\tdw\t0([0-9a-f]+)h,DGROUP',l)
            if m:
                o=int(m.group(1),16); it['dptr'].append((it['addr']+len(it['data']),o)); it['data']+=[o&255,o>>8,0xeb,0x24]; continue
            m=re.match(r'\tdd\t(\S+)',l)
            if m: it['cptr'].append((it['addr']+len(it['data']),m.group(1))); it['data']+=[None]*4; continue
            raise SystemExit('?? '+l)
    return items
if __name__=='__main__':
    it=load(); ds=exe_ds(); bad=0
    for x in it:
        if x['kind']!='DATA': continue
        for k,v in enumerate(x['data']):
            if v is not None and k<len(x['data']) and x['addr']+k<len(ds) and ds[x['addr']+k]!=v and not (k>=2 and (x['addr']+k-2) in [p for p,_ in x['dptr']]): bad+=1
    d=[x for x in it if x['kind']=='DATA']
    gaps=[(a['addr']+len(a['data']),b['addr']) for a,b in zip(d,d[1:]) if a['addr']+len(a['data'])!=b['addr']]
    print(len(it),'items, bytefehler',bad,'luecken',gaps[:5], 'ende', hex(d[-1]['addr']+len(d[-1]['data'])))
    b=[x for x in it if x['kind']=='BSS']; print('BSS',hex(b[0]['addr']),hex(b[-1]['addr']+len(b[-1]['data'])))
