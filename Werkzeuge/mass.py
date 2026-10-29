# mass.py SRC STARTS [cpus]  - Massentest ueber Optionen, Stapel je 32
import itertools,subprocess,os,sys
R='/root/sternenschweif'; D=R+'/'+os.environ['WOV']
src=sys.argv[1]; st=sys.argv[2]; cpus=sys.argv[3].split(',') if len(sys.argv)>3 else ['-3']
B=src.split('.')[0]
fl='-O -Oe -Ol -Oc -Og -Ov -Z -Oi'.split()
combos=[(cpu,)+c for cpu in cpus for k in range(len(fl)+1) for c in itertools.combinations(fl,k)]
for f in os.listdir(D+'/m'): os.remove(D+'/m/'+f)
for b in range(0,len(combos),32):
    L=[f'..\\bc\\BIN\\BCC.EXE -mlarge {c[0]} -Ot {os.environ.get('EXTRA','')} {" ".join(c[1:])} -c -Y -E..\\bc\\BIN\\TASM.EXE -om\\V{i:03d}.OBJ {src} > NUL' for i,c in list(enumerate(combos))[b:b+32]]
    open(D+'/MASS.BAT','w',newline='\r\n').write('\n'.join(L)+'\nexit\n')
    subprocess.run(['bash',R+'/tools/dosrun.sh',os.environ['WOV'],'MASS.BAT'],capture_output=True,timeout=3000)
objs=[D+f'/m/V{i:03d}.OBJ' for i in range(len(combos))]
miss=[o for o in objs if not os.path.exists(o)]
print('fehlend',len(miss))
out=subprocess.run(['python3','-I',R+'/tools/funcdiff.py',R+f'/ref_obj/{B}.OBJ',st]+[o for o in objs if os.path.exists(o)],capture_output=True,text=True,env=dict(os.environ,TOP=os.environ.get('TOP','10'))).stdout
for l in out.splitlines():
    i=int(l.split()[1][1:4]); print(l, ' '.join(combos[i]))
