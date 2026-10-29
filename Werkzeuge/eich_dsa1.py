import sys, re, collections; sys.path.insert(0,'/root/sternenschweif/q004/zerl')
import schweif
def truth(segs, mapf):
    M=open(mapf,encoding='latin1').read().splitlines()
    ovy=[]; stubn=[]; segpara=collections.Counter()
    for l in M:
        m=re.match(r'\s*([0-9A-F]{5})H ([0-9A-F]{5})H ([0-9A-F]{5})H (\S+)\s+(\S+)',l)
        if m and m[5]==':OVY': ovy.append((int(m[1],16),m[4]))
        if m and m[5]=='STUBSEG' and int(m[3],16)>0: stubn.append(m[4])
    O=[s for s in segs if s['kind']=='O']; R=[s for s in segs if s['kind']=='R']
    bystub={o['para']:o for o in O}
    i=[k for k,l in enumerate(M) if 'Publics by Value' in l][0]
    pubs=collections.defaultdict(set); un=collections.Counter()
    for l in M[i+1:]:
        m=re.match(r'\s*([0-9A-F]{4}):([0-9A-F]{4})\s+(?:(idle|Abs)\s+)?(\S+)\s*$',l)
        if not m or m[3]=='Abs': continue
        a=int(m[1],16)*16+int(m[2],16); s=None
        o=bystub.get(int(m[1],16))
        if o is not None and int(m[2],16)<len(o['code']): s=o; off=int(m[2],16)
        if not s:
            for r in R:
                if r['para']*16<=a<r['para']*16+len(r['code']): s=r; off=a-r['para']*16; break
        if s: pubs[s['id']].add((off,m[4]))
        else: un[m[1]]+=1
    return pubs, un
if __name__=='__main__':
    segs=schweif.load('/root/sternenschweif/orig/SCHICKM.EXE'); F=schweif.functions(segs)
    pubs,un=truth(segs,'/root/sternenschweif/dsa1/BLADEM.MAP')
    tot=hit=0; miss=collections.Counter(); ex=[]; extra=0
    for sid,P in pubs.items():
        st={o for o,q in F.get(sid,[])}
        for o,n in P:
            tot+=1
            if o in st: hit+=1
            else: miss[sid[0]]+=1; ex.append((sid,hex(o),n))
        extra+=len(st-{o for o,n in P})
    print('Publics in Code',tot,'gefunden',hit,f'({100*hit/tot:.1f} %)','nicht gefunden je Art',dict(miss),' eigene Anfaenge ohne Public',extra,' Publics ausserhalb Code',sum(un.values()))
    print('Beispiele:',ex[:15])
