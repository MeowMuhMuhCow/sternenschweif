# var.py BASE.CPP "opts" STARTS 'old' 'new1' 'new2' ... - Varianten einer Zeile testen
import sys,subprocess,os
R='/root/sternenschweif'; D=R+'/'+os.environ['WOV']
src,opts,st,old=sys.argv[1:5]; news=sys.argv[5:]
B,ext=src.split('.'); s=open(D+'/'+src).read(); assert old in s, 'old fehlt'
os.makedirs(D+'/v',exist_ok=True)
for f in os.listdir(D+'/v'): os.remove(D+'/v/'+f)
L=[]
for i,n in enumerate(news):
    open(D+f'/v/{B}.{ext}'.replace(B,f'X{i:02d}'),'w').write(s.replace(old,n.replace('\\n','\n').replace('\\t','\t')))
    L.append(f'..\\..\\bc\\BIN\\BCC.EXE -mlarge {opts} -c -Y -E..\\..\\bc\\BIN\\TASM.EXE X{i:02d}.{ext} > X{i:02d}.TXT')
open(D+'/v/V.BAT','w',newline='\r\n').write('\n'.join(L)+'\nexit\n')
subprocess.run(['bash',R+'/tools/dosrun.sh',os.environ['WOV']+'\\v','V.BAT'],capture_output=True)
for i,n in enumerate(news):
    o=D+f'/v/X{i:02d}.OBJ'
    if not os.path.exists(o): print(i,'FEHLER',open(D+f'/v/X{i:02d}.TXT',errors='replace').read()[-300:].replace('\n',' '),n); continue
    r=subprocess.run(['python3','-I',R+'/tools/funcdiff.py',R+f'/ref_obj/{B}.OBJ',st,o],capture_output=True,text=True).stdout.strip()
    print(i,r.split(' ',2)[0],r.split(' ',2)[2] if r.count(' ')>1 else r,'|',n)
