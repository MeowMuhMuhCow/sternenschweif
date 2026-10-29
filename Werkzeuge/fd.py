#!/usr/bin/env python3
"""fd.py OBJ SEGID START1,START2,... NAME - fdis fuer eine Funktion per Name"""
import sys, subprocess
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); sys.path.insert(0,'/root/sternenschweif/tools')
import schweif, omfcmp, struct, collections
exec(open('/root/sternenschweif/tools/funcmatch.py').read().split('\nexe = open')[0].split('import omfcmp\n',1)[1])
obj,sid,starts,name=sys.argv[1],sys.argv[2],[int(x,16) for x in sys.argv[3].split(',')],sys.argv[4]
P=publics(obj)
_,_,data,mask=[c for c in omfcmp.parse_obj(obj) if c[1]=='CODE'][0]
for i,(off,n) in enumerate(P):
    if n.lstrip('_')==name.lstrip('_'):
        end=P[i+1][0] if i+1<len(P) else len(data)
        ln=max(end-off, (starts[i+1]-starts[i]) if i+1<len(starts) else end-off)+2
        subprocess.run(['python3','-I','/root/sternenschweif/tools/fdis.py',obj,'%x'%off,sid,'%x'%starts[i],'%x'%ln])
