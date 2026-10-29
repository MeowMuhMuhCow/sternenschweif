"""prjlib.py - BC 3.1 Projektdatei (.PRJ) lesen/schreiben: Kopf 0x24 Bytes, dann Records id(2) len(2) daten."""
import struct
def load(path):
    d=open(path,'rb').read(); hdr=d[:0x24]; j=0x24; recs=[]
    while j+4<=len(d):
        t,l=struct.unpack_from('<HH',d,j); recs.append([t,bytearray(d[j+4:j+4+l])]); j+=4+l
    return hdr,recs,d[j:]
def save(path,hdr,recs,tail=b''):
    out=bytearray(hdr)
    for t,b in recs: out+=struct.pack('<HH',t,len(b))+bytes(b)
    open(path,'wb').write(bytes(out)+tail)

def node(name, overlay=False, refs=(0,0,0), flags0=0):
    """108-Byte-Knoten einer Projektdatei (Record 0x35)"""
    b=bytearray(108)
    b[0]=flags0|(0x08 if overlay else 0)
    b[8:16]=b'\xff'*8
    nm=name.encode(); b[20:20+len(nm)]=nm
    for k,v in enumerate(refs): b[98+2*k:100+2*k]=v.to_bytes(2,'little')
    return b

def setopt(recs, rid, val):
    for r in recs:
        if r[0]==rid:
            if isinstance(val,(bytes,bytearray)): r[1][:len(val)]=val
            else: r[1][0]=val
            return
    raise KeyError(hex(rid))

def setstr(recs, rid, s):
    for r in recs:
        if r[0]==rid:
            b=s.encode(); r[1][:]=b+bytes(len(r[1])-len(b)); return
    raise KeyError(hex(rid))
