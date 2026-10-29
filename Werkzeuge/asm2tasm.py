#!/usr/bin/env python3
"""asm2tasm.py SEGID BASIS [NAME] - erzeugt aus einem Assembler-Segment von SCHWEIF.EXE eine TASM-Quelle
mit Sprungmarken und Externen und prueft sie (TASM in DOSBox, Vergleich gegen das Original).
Befehle, die TASM anders kodiert, werden schrittweise durch db ersetzt, bis das Ergebnis bytegleich ist."""
import sys, re, struct, csv, subprocess, os
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); import schweif
ROOT='/root/sternenschweif'
sid=sys.argv[1]; base=int(sys.argv[2],16)
num=int(sid[1:]); _ovl=sid[0]=='O'; name=sys.argv[3] if len(sys.argv)>3 else (f'OVL{num:03d}' if _ovl else f'SEG{num:03d}')
_wdname=(f'o{num:03d}' if _ovl else f'a{num:02d}')
d=open(ROOT+'/orig/SCHWEIF.EXE','rb').read()
nrel,hdr=struct.unpack_from('<HH',d,6); relo=struct.unpack_from('<H',d,0x18)[0]
segs={s['id']:s for s in schweif.load(ROOT+'/orig/SCHWEIF.EXE')}
S=segs[sid]; code=S['code']; para=S['para']
_lnk=os.environ.get('LNK')
if _lnk:
    import json
    _rg=json.load(open(ROOT+'/lnk_ranges.json'))['mods'][sid]
    _hd=struct.unpack_from('<H',d,8)[0]*16
    base=_rg[0]; code=d[_hd+para*16:_hd+para*16+_rg[1]]
    _wdname=f'l{num:02d}'
P={int(r['para']):r['segment'] for r in csv.DictReader(open(ROOT+'/q004/zerl/segmente.tsv'),delimiter='\t')}
lib={}
for f in ('schweif_cl.tsv','schweif_overlay.tsv'):
    for l in open(ROOT+'/lib/'+f):
        s_,o,n,m,_=l.rstrip('\n').split('\t'); lib[(s_,int(o,16))]=m
rel=set()
for i in range(nrel):
    o,s=struct.unpack_from('<HH',d,relo+4*i); a=s*16+o-para*16
    if 0<=a<len(code): rel.add(a)
if _ovl: rel=set(S['fix'])
_e=(lambda l,p_:(p_-1)*512+l if l else p_*512)(*struct.unpack_from('<HH',d,2))
_sz,_toff,_cnt=struct.unpack_from('<III',d,_e+4)
_tab=[struct.unpack_from('<HHHH',d,_toff+8*i_) for i_ in range(_cnt)]
def segpara(v):
    if _ovl: return _tab[v//8][0] if v%8==0 and v//8<_cnt else None
    return v
_fst={}
for _r in csv.DictReader(open(ROOT+'/q004/zerl/funktionen.tsv'),delimiter='\t'):
    if _r['belege']!='A': _fst.setdefault(_r['segment'],[]).append(int(_r['start'],16))
def repsym(T):
    if T in ('R00','R47'):
        return sorted(v_ for (a_,b_),v_ in _libsym.items() if a_==T)[0]
    if T[0]=='O': return f'F_{T}_{segs[T]["entries"][0]:04X}'
    import json as _js
    _mb=_js.load(open(ROOT+'/lnk_ranges.json'))['mods'].get(T,[0])[0]
    return f'F_{T}_{min(f_ for f_ in _fst[T] if f_>=_mb):04X}'
def segref(v):
    sp=segpara(v)
    if sp is None: return None
    if sp==0x24eb: need_dgroup[0]=True; return 'DGROUP'
    T=P.get(sp)
    if T is None: return None
    if T==sid: return f'{name}_TEXT'
    sy=repsym(T); ext.add(sy); return f'seg {sy}'
_fs=set()
for _r in csv.DictReader(open(ROOT+'/q004/zerl/funktionen.tsv'),delimiter='\t'):
    if _r['segment']==sid and int(_r['start'],16)>=base: _fs.add(int(_r['start'],16))
if os.path.exists(ROOT+'/extra_pubs.tsv'):
    for _l in open(ROOT+'/extra_pubs.tsv'):
        _a,_b=_l.split()
        if _a==sid and base<=int(_b,16)<len(code): _fs.add(int(_b,16))
_cuts=sorted(x for x in _fs|{base} if base<=x<len(code))+[len(code)]
def fixt(t,a):
    m=re.match(r'((?:j\w+|loop\w*|jcxz|jecxz|call) (?:short |near )?)0x([0-9a-f]+)$',t)
    return m.group(1)+hex((int(m.group(2),16)+a)&0xffff) if m else t
ins=[]
for _a,_b in zip(_cuts,_cuts[1:]):
    for o,ln,t in schweif.disasm(code[_a:_b]): ins.append((o+_a,ln,fixt(t,_a)))
offs=set(o for o,_,_ in ins)
def H(v):
    return f'0{v:x}h' if v>9 else str(v)
def hx(s):
    return re.sub(r'0x([0-9a-f]+)',lambda m:H(int(m.group(1),16)),s)
_libsym={}
for _l in open(ROOT+'/lib/lib_syms.tsv'):
    _a,_b,_c=_l.rstrip('\n').split('\t'); _libsym.setdefault((_a,int(_b,16)),_c)
def extname(seg,off):
    sg=P.get(segpara(seg))
    if sg in ('R00','R47') and (sg,off) in _libsym: return _libsym[(sg,off)]
    if sg and sg[0]=='O':
        e=segs[sg]['entries']; i=(off-0x20)//5
        return f'F_{sg}_{e[i]:04X}' if 0<=i<len(e) else f'F_{sg}_{off:04X}'
    if sg and (sg,off) in lib: return lib[(sg,off)]+'_'+f'{off:04X}'
    return f'F_{sg}_{off:04X}'
targets={}; ext=set(); extnear=set()
for o,ln,t in ins:
    m=re.match(r'(j\w+|loop\w*|jcxz|jecxz|call) (?:short |near )?0x([0-9a-f]+)$',t)
    if m:
        tg=int(m.group(2),16)
        if base<=tg<len(code) and tg in offs: targets[tg]=f'L_{tg:04x}'
lines=[]; lineoff=[]; i=0
fstarts=set(_fs)
pubs=[]
force_db=set(int(x,16) for x in os.environ.get('FORCE_DB','').split(',') if x)
def db(o,ln): return '\tdb\t'+','.join(H(b) for b in code[o:o+ln])
cur_o=[0]; need_dgroup=[False]
def emit(x):
    if x.endswith(':') or '\tlabel\t' in x: lines.append(x)
    else: lines.append(x+f'\t;@{cur_o[0]:04x}')
while i<len(ins):
    o,ln,t=ins[i]
    cur_o[0]=o
    if o in fstarts:
        pn=f'F_{sid}_{o:04X}'; pubs.append(pn); emit(f'{pn}\tlabel\tfar')
    if o in targets: emit(f'{targets[o]}:')
    raw=code[o:o+ln]
    nxt=ins[i+1] if i+1<len(ins) else None
    relhere=[r for r in rel if o<=r<o+ln]
    if o in force_db: emit(db(o,ln)+f'\t; {t}'); i+=1; continue
    m=re.match(r'call 0x([0-9a-f]+):0x([0-9a-f]+)$',t)
    if m and raw[0]==0x9a and not any(o<r_<o+ln for r_ in rel):
        emit(db(o,ln)+f'\t; {t} (ohne Relokation)'); i+=1; continue
    if m and raw[0]==0x9a:
        e=extname(int(m.group(1),16),int(m.group(2),16)); ext.add(e)
        emit(f'\tcall\tfar ptr {e}'); i+=1; continue
    _segsub=None
    if relhere and not (raw[0]==0x9a and re.match(r'call 0x',t)):
        parts=[]; q_=o; okk=True
        for r_ in sorted(relhere):
            v_=struct.unpack_from('<H',code,r_)[0]; ts_=segref(v_)
            if not ts_: okk=False; break
            if r_>q_: parts.append(db(q_,r_-q_))
            parts.append(f'\tdw\t{ts_}'); q_=r_+2
        if okk:
            if q_<o+ln: parts.append(db(q_,o+ln-q_))
            emit(parts[0]+f'\t; {t}  (Relokation)')
            for x_ in parts[1:]: emit(x_)
            i+=1; continue
        emit(db(o,ln)+f'\t; {t}  (Relokation, TODO)'); i+=1; continue
    m=re.match(r'(j\w+|loop\w*|jcxz|jecxz|call) (?:short |near )?0x([0-9a-f]+)$',t)
    if m:
        op=m.group(1); tg=int(m.group(2),16)
        if tg not in targets:
            if op=='call' and raw[0]==0xe8:
                emit(db(o,ln)+f'\t; {t}'); i+=1; continue
            emit(db(o,ln)+f'\t; {t}'); i+=1; continue
        L=targets[tg]; fwd=tg>o
        if op=='call': emit(f'\tcall\tnear ptr {L}'); i+=1; continue
        if op=='jmp':
            if raw[0]==0xe9: emit(f'\tjmp\tnear ptr {L}'); i+=1; continue
            if fwd and nxt and code[nxt[0]]==0x90 and nxt[0] not in targets:
                emit(f'\tjmp\t{L}'); i+=2; continue
            emit(f'\tjmp\tshort {L}'); i+=1; continue
        if op.startswith('j') and raw[0]!=0x0f and fwd:
            if nxt and code[nxt[0]:nxt[0]+2]==b'\x90\x90' and nxt[0] not in targets and (nxt[0]+1) not in targets:
                emit(f'\t{op}\t{L}'); i+=3 if ins[i+1][1]==1 else 2; continue
            emit(f'\t{op}\tshort {L}'); i+=1; continue
        emit(f'\t{op}\t{L}'); i+=1; continue
    # allgemeine Uebersetzung
    tt=t
    if re.search(r'\bo32\b|\ba32\b|\bdb\b|\bo16\b|\ba16\b',tt) or tt.startswith(('fs ','gs ','es ','cs ','ss ','ds ')):
        emit(db(o,ln)+f'\t; {t}'); i+=1; continue
    tt=re.sub(r'\[(dword|word|byte) ',r'[',tt)
    tt=re.sub(r'\[(es|cs|ss|ds|fs|gs):',r'\1:[',tt)
    tt=re.sub(r'\b(byte|word|dword|qword|tword) (\w+:)?\[',lambda m:f'{m.group(1)} ptr {m.group(2) or ""}[',tt)
    tt=re.sub(r'\bfar (\w+:)?\[',lambda m:f'dword ptr {m.group(1) or ""}[',tt)
    tt=re.sub(r'(?<![:\w])\[(0x[0-9a-f]+)\]',r'ds:[\1]',tt)
    tt=re.sub(r'\b(byte|word|dword) ([+-]?0x[0-9a-f]+)$',r'\2',tt)
    tt=re.sub(r',(byte|word|dword) ([+-]?0x[0-9a-f]+)$',r',\2',tt)
    tt=re.sub(r'^push (byte|word|dword) ([+-]?0x[0-9a-f]+)$',lambda m:('push dword ' if m.group(1)=='dword' else 'push ')+m.group(2),tt)
    tt=tt.replace(',+',',').replace(' +',' ')
    tt=re.sub(r'^enter 0x([0-9a-f]+),0x([0-9a-f]+)$',lambda m:f'enter {int(m.group(1),16)},{int(m.group(2),16)}',tt)
    tt=hx(tt)
    if _segsub: tt=tt.replace('SEGPLATZ',_segsub)
    p=tt.split(' ',1)
    emit('\t'+p[0]+('\t'+p[1] if len(p)>1 else '')); i+=1
while len(lineoff)<len(lines): lineoff.append(None)
src=[f'; {name} - aus SCHWEIF.EXE {sid} (ab +{base:#x}) mit asm2tasm.py erzeugt',
     '\t.386p','\t; keine lokalen Marken']
src+=[f'\textrn\t{e}:far' for e in sorted(ext)]+[f'\textrn\t{e}:near' for e in sorted(extnear)]
src+=(['_DATA\tsegment word public \'DATA\'','_DATA\tends','DGROUP\tgroup\t_DATA'] if need_dgroup[0] else [])
_wa=set(open(ROOT+'/word_align.txt').read().split()) if os.path.exists(ROOT+'/word_align.txt') else set()
src+=[f'{name}_TEXT\tsegment {"word" if (_lnk and sid in _wa) else "byte"} public use16 \'CODE\'',f'\tassume\tcs:{name}_TEXT, ds:nothing']
src+=[f'\tpublic\t{p}' for p in pubs]+lines+[f'{name}_TEXT\tends','\tend']
wd=f'{ROOT}/{_wdname}'; os.makedirs(wd,exist_ok=True)
pre=len(src)-len(lines)-2
open(f'{wd}/{name}.ASM','w',newline='\r\n').write('\n'.join(src)+'\n')
subprocess.run(['bash',ROOT+'/tools/dosrun.sh',_wdname,f'..\\bc\\BIN\\TASM.EXE /ml {name}.ASM'],capture_output=True)
out=open(f'{wd}/DOSOUT.TXT',errors='replace').read()
errs=[l for l in out.splitlines() if '**' in l]
if errs: print('\n'.join(errs[:10])); sys.exit(1)
if _lnk:
    sys.path.insert(0,ROOT+'/tools'); import omfcmp
    _,_,_dat,_msk=[c for c in omfcmp.parse_obj(f'{wd}/{name}.OBJ') if c[1]=='CODE'][0]
    _org=code[base:]; _bad=[i for i in range(len(_dat)) if _msk[i] and (i>=len(_org) or _org[i]!=_dat[i])]
    r=f'{sid}: neu {len(_dat)} Bytes, Original ab {base:#x} noch {len(_org)} Bytes, Abweichungen {len(_bad)} '+('GLEICH' if not _bad and len(_dat)==len(_org) else '')
    if _bad: r+=f'\nerste Abweichung bei +{_bad[0]:#x} (Original +{_bad[0]+base:#x})'
    elif len(_dat)!=len(_org): r+=f'\nerste Abweichung bei +{len(_dat):#x} (Original +{min(len(_dat),len(_org))+base-1:#x})'
else:
    r=subprocess.run(['python3','-I',ROOT+'/tools/segvgl.py',f'{wd}/{name}.OBJ',sid,hex(base)],capture_output=True,text=True).stdout
print(r.strip())
m=re.search(r'erste Abweichung bei \+0x([0-9a-f]+) \(Original \+0x([0-9a-f]+)\)',r)
if m:
    a=int(m.group(2),16); cand=[o for o,ln,_ in ins if o<=a]
    print('ERSTE_ABW_BEFEHL',hex(cand[-1]) if cand else '?')
