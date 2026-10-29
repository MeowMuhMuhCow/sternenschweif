#!/usr/bin/env python3
"""items_ds.py - Zugriff auf die DS-Items aus dsg (ueber dsana/items.py): Codezeiger und DGROUP-Fernzeiger je Adresse."""
import sys
sys.path.insert(0, '/root/sternenschweif/dsana')
import items as IT
_its = None
def its():
    global _its
    if _its is None: _its = IT.load()
    return _its
def cptr():
    """{ds-adresse: Name} fuer jedes 'dd Name' im Spiel-DS"""
    return {p: n for x in its() for p, n in x['cptr']}
def dptr():
    """{ds-adresse: Zieloffset} fuer jedes 'dw off,DGROUP' im Spiel-DS"""
    return {p: o for x in its() for p, o in x['dptr']}
