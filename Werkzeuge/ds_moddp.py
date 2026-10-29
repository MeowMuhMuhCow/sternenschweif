"""moddp.py - teilt das Spiel-_DATA (0x94..0x86b4) in Modulbloecke auf.
Annahme: jedes Modul hat einen zusammenhaengenden _DATA-Block, Bloecke liegen in Link-Reihenfolge,
residente (R) und Overlay-Module (O) jeweils aufsteigend, beliebig verzahnt. Dynamische Programmierung."""
import numpy as np, collections, sys
LO,HI=int(sys.argv[1],16),int(sys.argv[2],16); PEN=float(sys.argv[3]) if len(sys.argv)>3 else 1.0
refs=collections.defaultdict(set)
for l in open('/root/sternenschweif/ds/zugriffe.tsv'):
    a,k,w,s,o,t=l.rstrip('\n').split('\t',5); a=int(a,16)
    if LO<=a<HI: refs[a].add(s)
A=sorted(refs)
Rs=sorted({s for a in A for s in refs[a] if s[0]=='R'}); Os=sorted({s for a in A for s in refs[a] if s[0]=='O'})
nr,no=len(Rs)+1,len(Os)+1; ri={s:i+1 for i,s in enumerate(Rs)}; oi={s:i+1 for i,s in enumerate(Os)}
NEG=-1e9
V=np.full((2,nr,no),NEG,np.float32); V[:,0,0]=0
src=np.zeros((len(A),2,nr,no),np.int16)
for t,a in enumerate(A):
    S=refs[a]; w=1.0/len(S)
    M=np.maximum(V[0],V[1]); arg=(V[1]>V[0]).astype(np.int16)
    # prefix max (exklusiv) entlang r bzw. o
    def pref(M,arg,axis):
        n=M.shape[axis]; P=np.full_like(M,NEG); I=np.zeros(M.shape,np.int16)
        best=np.full(M.shape[1-axis],NEG,np.float32); bi=np.zeros(M.shape[1-axis],np.int16)
        for k in range(n):
            if axis==0: P[k]=best; I[k]=bi; cur=M[k]; ca=arg[k]
            else: P[:,k]=best; I[:,k]=bi; cur=M[:,k]; ca=arg[:,k]
            upd=cur>best; best=np.where(upd,cur,best); bi=np.where(upd,k*2+ca,bi)
        return P,I
    P0,I0=pref(M,arg,0); P1,I1=pref(M,arg,1)
    N=np.empty_like(V)
    sw0=(P0-PEN)>V[0]; N[0]=np.where(sw0,P0-PEN,V[0]); src[t,0]=np.where(sw0,I0+1,0)
    sw1=(P1-PEN)>V[1]; N[1]=np.where(sw1,P1-PEN,V[1]); src[t,1]=np.where(sw1,I1+1,0)
    sc0=np.zeros(nr,np.float32); sc1=np.zeros(no,np.float32)
    for s in S:
        if s in ri: sc0[ri[s]]=w
        else: sc1[oi[s]]=w
    N[0]+=sc0[:,None]; N[1]+=sc1[None,:]
    N[0,0,:]=NEG; N[1,:,0]=NEG
    V=N
f,r,o=np.unravel_index(np.argmax(V),V.shape)
print('Punkte',V[f,r,o],'von',sum(1 for a in A))
path=[]
for t in range(len(A)-1,-1,-1):
    path.append((A[t], Rs[r-1] if f==0 else Os[o-1]))
    c=src[t,f,r,o]
    if c:
        c-=1; k,ff=c//2,c%2
        if f==0: r=k
        else: o=k
        f=ff
path.reverse()
blocks=[]
for a,m in path:
    if blocks and blocks[-1][0]==m: blocks[-1][2]=a; blocks[-1][3]+=1; blocks[-1][4]+= (m in refs[a])
    else: blocks.append([m,a,a,1,int(m in refs[a])])
with open('/root/sternenschweif/ds/moduldp.tsv','w') as fo:
    for m,a,b,n,h in blocks: fo.write(f'{m}\t{a:#06x}\t{b:#06x}\t{n}\t{h}\n')
print('Bloecke',len(blocks))
print(' '.join(f'{m}:{a:x}-{b:x}({h}/{n})' for m,a,b,n,h in blocks))
