"""namen_gesamt.py - fuehrt die Namensquellen fuer DGROUP-Marken zusammen (Vorrang in dieser Reihenfolge):
 1 eigene Rekonstruktion (variablen.tsv, nur sprechende Namen), 2 Bright Eyes DSA 2, 3 DSA 1 ueber gleichen Inhalt,
 4 DSA 1 ueber ausgerichteten Code (>=6 Stimmen, eindeutig)."""
import csv, re
X='/root/sternenschweif/ds/'
G=lambda a: 0x94<=a<0x86b4 or 0x8b4e<=a<0xcb76
names={}
def put(a,n,src,note=''):
    if G(a) and a not in names and n not in {v[0] for v in names.values()}: names[a]=(n,src,note)
for r in csv.DictReader(open(X+'variablen.tsv'),delimiter='\t'):
    n=r['name']
    if not re.fullmatch(r'g_(str_)?[0-9a-f]{4}',n): put(int(r['adresse'],16),n,'eigene Rekonstruktion',r['typ'])
put(0xb128,'g_renderbuf_ptr','Bright Eyes DSA 2','Zeiger auf den 3D-Bildpuffer (sceneptr)')
put(0x1af2,'g_skill_attribs','Bright Eyes DSA 2','je Talent 3 Eigenschaften (test_skill)')
put(0x98fe,'g_seg013_pairs','Bright Eyes DSA 2','4 Paare x/y, seg013_91d')
for r in csv.DictReader(open(X+'namen_inhalt.tsv'),delimiter='\t'):
    n=r['name']
    if 'unkn' in n or 'empty' in n: continue
    if r['art']=='dat' and int(r['schluessel'])<6: continue
    put(int(r['adresse_dsa2'],16),n,'DSA 1, gleicher Inhalt',f"DSA1 {r['adresse_dsa1']}, {r['uebereinstimmung']}")
for r in csv.DictReader(open(X+'namen_code.tsv'),delimiter='\t'):
    n=r['name_dsa1']
    if n=='None' or '+' in n or 'unkn' in n: continue
    a2,a1=int(r['adresse_dsa2'],16),int(r['adresse_dsa1'],16)
    if (a2>=0x8b4e)!=(a1>=0xbc38): continue   # DSA1-BSS/DATA muss zur Lage in DSA2 passen
    if int(r['stimmen'])>=6 and r['stimmen']==r['stimmen_gesamt']:
        put(int(r['adresse_dsa2'],16),n,'DSA 1, gleicher Code',f"DSA1 {r['adresse_dsa1']}, {r['stimmen']} Stimmen")
with open(X+'namen_gesamt.tsv','w') as f:
    f.write('adresse\tname\tquelle\tbemerkung\n')
    for a in sorted(names): f.write(f'{a:#06x}\t'+'\t'.join(names[a])+'\n')
import collections
print(len(names),collections.Counter(v[1] for v in names.values()))
for a in sorted(names): print(hex(a),names[a][0],names[a][1])
