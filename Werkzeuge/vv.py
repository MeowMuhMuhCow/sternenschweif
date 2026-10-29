# vv.py BASE "opts" VARIANTS.json [desc.json] - Varianten in Stapeln zu 32 (je Unterordner, gleicher Dateiname), ccmp
import sys,subprocess,os,json,shutil
R='/root/sternenschweif'; import os; D=R+'/'+os.environ['WOV']
src,opts,vf=sys.argv[1:4]; V=json.load(open(vf)); desc=json.load(open(sys.argv[4])) if len(sys.argv)>4 else None
B,ext=src.split('.'); s=open(D+'/'+src).read()
for b in range(0,len(V),32):
    shutil.rmtree(D+'/v',ignore_errors=True); os.makedirs(D+'/v')
    L=[]
    for i in range(b,min(b+32,len(V))):
        t=s
        for o,n in V[i]:
            assert o in t, o
            t=t.replace(o,n)
        k=f'X{i-b:02d}'; os.makedirs(D+'/v/'+k)
        open(D+f'/v/{k}/{src}','w').write(t)
        L+= [f'cd {k}', f'..\\..\\..\\bc\\BIN\\BCC.EXE -mlarge {opts} -c -Y -E..\\..\\..\\bc\\BIN\\TASM.EXE {src} > O.TXT','cd ..']
    open(D+'/v/V.BAT','w',newline='\r\n').write('\n'.join(L)+'\nexit\n')
    subprocess.run(['bash',R+'/tools/dosrun.sh',os.environ['WOV']+'\\v','V.BAT'],capture_output=True)
    for i in range(b,min(b+32,len(V))):
        o=D+f'/v/X{i-b:02d}/{B}.OBJ'
        if not os.path.exists(o): print(i,'FEHLER', desc[i] if desc else ''); continue
        r=subprocess.run(['python3','-I',R+'/tools/ccmp.py',R+f'/ref_obj/{B}.OBJ',o],capture_output=True,text=True).stdout.strip().splitlines()
        print(i,r[-1][:100] if r else '?', desc[i] if desc else '',flush=True)
