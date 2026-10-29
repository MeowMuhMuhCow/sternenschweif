"""selfoff.py - prueft Verweise auf das eigene Codesegment (Sprungtabellen, Funktionszeiger-Offsets).
Fixup typ 1 (Offset) mit Ziel = eigenes Segment: Endwert = Inline-Wert + d (d = Startversatz im Paragraph,
fuer alle gleich). In der ASM-Fassung stehen diese Werte als feste Zahlen. Liefert Liste abweichender Stellen."""
import struct, objbytes, objfix
def check(asm_path, c_path):
    sb=objbytes.seg_bytes(c_path); seg=[s for s in sb if s.endswith('_TEXT')][0]
    c=sb[seg]; a=objbytes.seg_bytes(asm_path)[seg]
    afix={f['loc'] for f in objfix.parse(asm_path)['fix'] if f['seg']==seg}
    pts=[f for f in objfix.parse(c_path)['fix'] if f['seg']==seg and f['typ'] in (1,5) and f['tgt']==('SEG',seg) and f['loc'] not in afix and f['loc']+2<=min(len(a),len(c))]
    if not pts: return []
    ds={(struct.unpack_from('<H',a,f['loc'])[0]-struct.unpack_from('<H',c,f['loc'])[0]-f['disp'])&0xffff for f in pts if f['loc']+2<=len(a)}
    # haeufigster Versatz gilt
    vals=[(struct.unpack_from('<H',a,f['loc'])[0]-struct.unpack_from('<H',c,f['loc'])[0]-f['disp'])&0xffff for f in pts if f['loc']+2<=len(a)]
    d=max(set(vals),key=vals.count)
    # Funktionszeiger ins eigene Overlay: ASM-Wert ist ein Stub-Thunk (0x20+5n), C-Inline-Wert ein Funktionsanfang
    # (Public der C-OBJ) -> der Linker setzt den Thunk ein; die echte Pruefung macht der Komplettlink.
    cpub={o for s_,o,n in objfix.parse(c_path)['pubs']}
    def thunk(f):
        av=struct.unpack_from('<H',a,f['loc'])[0]; cv=struct.unpack_from('<H',c,f['loc'])[0]
        return av>=0x20 and (av-0x20)%5==0 and cv in cpub
    return [f['loc'] for f,v in zip(pts,vals) if v!=d and not thunk(f)]
