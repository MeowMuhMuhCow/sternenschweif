# genblk.py - erzeugt den Byteblock fuer F_R05_0012 aus der Referenz-OBJ (Bytes unveraendert,
# Fernaufrufe als 'asm call far ptr NAME', damit die Fixups entstehen)
import sys, subprocess
sys.path.insert(0, '../tools')
from objbytes import seg_bytes
b = list(seg_bytes('ref/SEG005.OBJ').values())[0]
fx = {}
for l in subprocess.run(['python3', '-I', '../tools/fixdump.py', 'ref/SEG005.OBJ'], capture_output=True, text=True).stdout.split('\n'):
    if 'EXT' in l:
        p = l.split(); fx[int(p[0][1:], 16)] = p[-2]
names = {'_open': 'open', '_read': 'read', '_close': 'close', '_printf': 'printf', '_puts': 'puts',
         '_bioskey': 'bioskey', '_strcmp': 'strcmp', 'F_O004_0103': 'O004_0103', 'F_O004_004F': 'O004_004F'}
start, end = 0x1e - 8, 0x1a1 - 8	# ab "mov [bp-4],ds" bis vor den toten Epilog
out = []; i = start; row = []
def flush():
    global row
    if row: out.append('\tasm db ' + ', '.join('0%02xh' % x for x in row)); row = []
while i < end:
    if b[i] == 0x9a and (i + 1) in fx:
        flush(); out.append('\tasm call far ptr ' + names[fx[i + 1]]); i += 5; continue
    row.append(b[i]); i += 1
    if len(row) == 12: flush()
flush()
open('blk0012.inc', 'w').write('\n'.join(out) + '\n')
print(len(out))
