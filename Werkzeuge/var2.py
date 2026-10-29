# var2.py BASE "opts" STARTS VARIANTS.json  - jede Variante = Liste von [alt,neu]-Paaren
import sys,subprocess,os,json
R='/root/sternenschweif'; D=R+'/'+os.environ['WOV']
src,opts,st,vf=sys.argv[1:5]; V=json.load(open(vf))
B,ext=src.split('.'); s=open(D+'/'+src).read()
os.makedirs(D+'/v',exist_ok=True)
for f in os.listdir(D+'/v'): os.remove(D+'/v/'+f)
L=[]
for i,pairs in enumerate(V):
    t=s
    for o,n in pairs:
        assert o in t, o
        t=t.replace(o,n)
    open(D+f'/v/X{i:02d}.{ext}','w').write(t)
    L.append(f'..\\..\\bc\\BIN\\BCC.EXE -mlarge {opts} -c -Y -E..\\..\\bc\\BIN\\TASM.EXE X{i:02d}.{ext} > X{i:02d}.TXT')
open(D+'/v/V.BAT','w',newline='\r\n').write('\n'.join(L)+'\nexit\n')
subprocess.run(['bash',R+'/tools/dosrun.sh',os.environ['WOV']+'\\v','V.BAT'],capture_output=True)
for i,pairs in enumerate(V):
    o=D+f'/v/X{i:02d}.OBJ'
    if not os.path.exists(o): print(i,'FEHLER',open(D+f'/v/X{i:02d}.TXT',errors='replace').read()[-300:].replace('\n',' ')); continue
    r=subprocess.run(['python3','-I',R+'/tools/funcdiff.py',R+f'/ref_obj/{B}.OBJ',st,o],capture_output=True,text=True).stdout.strip()
    print(i,r.split(' ',2)[0],r.split(' ',2)[2] if r.count(' ')>1 else r,'|',[n for o,n in pairs])
