"""dsa1_vars.py - liest Bright Eyes datseg.cpp (DSA 1, m302de): Adresse, Name, Typ, Groesse."""
import re
P='/root/sternenschweif/Bright-Eyes/src/custom/schick/rewrite_m302de/datseg.cpp'
def load():
    out=[]; start=None
    for ln in open(P,encoding='latin1'):
        if ln[:1] not in (' ','\t','}','#','/','\n','{') and not ln.startswith('//'): start=ln
        m=re.search(r'//\s*ds:0x([0-9a-fA-F]{4})',ln)
        if m and start:
            decl=start.split('//')[0]
            n=re.match(r'\s*((?:const\s+|static\s+|unsigned\s+|signed\s+)*(?:struct\s+\w+|\w+)\s*\**)\s*(\(?\s*\*?\s*\w+)\s*((?:\[[^\]]*\])*)',decl)
            g=re.search(r'\b(g_\w+)\s*((?:\[[^\]]*\])*)',decl)
            if g:
                out.append((int(m.group(1),16),g.group(1),decl[:g.start()].strip(),g.group(2)))
            elif n:
                name=n.group(2).replace('(','').replace('*','').strip()
                out.append((int(m.group(1),16),name,n.group(1).strip(),n.group(3)))
            start=None
    out.sort()
    res=[]
    for i,(a,nm,t,arr) in enumerate(out):
        sz=(out[i+1][0]-a) if i+1<len(out) else 0
        res.append(dict(addr=a,name=nm,type=t,arr=arr,size=sz))
    return res
if __name__=='__main__':
    V=load(); print(len(V)); 
    for v in V[::200]: print(hex(v['addr']),v['name'],v['type'],v['arr'],v['size'])
