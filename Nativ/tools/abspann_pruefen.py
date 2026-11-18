#!/usr/bin/env python3
"""Prüft die Abspann-Bilder eines Replays: aufeinanderfolgende Aufnahmen dürfen nicht gleich sein."""
import hashlib
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
REP = os.path.join(ROOT, "Port", "build", "etappe2d_finale.rep")
LOG = os.path.join(ROOT, "Port", "build", "abspann.log")
NEED = ("Endkampf-Sieg", "Beginn Finale::run", "Ende Abspann")


def md5(path):
    h = hashlib.md5()
    with open(path, "rb") as f:
        h.update(f.read())
    return h.hexdigest()


def main():
    shots = []
    with open(REP, encoding="utf-8", errors="replace") as f:
        for line in f:
            parts = line.split()
            if len(parts) >= 4 and parts[0] == "F" and parts[2] == "B":
                shots.append(parts[3])
    if len(shots) < 2:
        print("FEHLGESCHLAGEN keine Bilder im Replay")
        return 1
    prev = None
    prev_name = None
    ok = True
    for rel in shots:
        path = rel if os.path.isabs(rel) else os.path.join(ROOT, rel.replace("/", os.sep))
        if not os.path.isfile(path):
            print(f"FEHLGESCHLAGEN fehlt {rel}")
            ok = False
            continue
        digest = md5(path)
        if digest == prev:
            print(f"FEHLGESCHLAGEN {os.path.basename(path)} gleich wie {prev_name} {digest}")
            ok = False
        else:
            print(f"OK {os.path.basename(path)} {digest}")
        prev = digest
        prev_name = os.path.basename(path)
    text = open(LOG, encoding="utf-8", errors="replace").read() if os.path.isfile(LOG) else ""
    for line in NEED:
        if line not in text:
            print(f"FEHLGESCHLAGEN Log fehlt: {line}")
            ok = False
        else:
            print(f"OK Log {line}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
