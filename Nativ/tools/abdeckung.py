#!/usr/bin/env python3
"""
abdeckung.py (W3) - Wertet Port/build/abdeckung.tsv gegen die Symboltabelle von
SCHWEIF_PORT.EXE und Port/build/objs/*.o aus.

Zeigt je Modul (SEG*.C/.CPP, OVL*.C/.CPP), wie viele Funktionen erreicht wurden
und welche Funktionen nie liefen. Erzeugt auf Wunsch Port/ABDECKUNG.md.
"""
import os
import re
import sys
import glob
import subprocess

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
BUILD_DIR = os.path.join(ROOT, "Port", "build")
EXE_PATH = os.path.join(BUILD_DIR, "SCHWEIF_PORT.EXE")
TSV_PATH = os.path.join(BUILD_DIR, "abdeckung.tsv")
MD_PATH  = os.path.join(ROOT, "Port", "ABDECKUNG.md")


def load_module_symbols():
    """Returns dict: sym_name -> mod_name (e.g. 'SEG001', 'OVL032')."""
    sym_to_mod = {}
    objs = sorted(glob.glob(os.path.join(BUILD_DIR, "objs", "*.o")))
    if not objs:
        return sym_to_mod
    out = subprocess.check_output(["nm", "-A"] + objs, text=True, stdin=subprocess.DEVNULL)
    for line in out.splitlines():
        if ":" not in line:
            continue
        f_part, rest = line.rsplit(":", 1)
        base = os.path.basename(f_part)
        if not base.startswith(("SEG", "OVL")):
            continue
        mod = os.path.splitext(base)[0]
        parts = rest.split()
        if len(parts) >= 3 and parts[1] == "T":
            sym = parts[2]
            if sym.startswith("__GLOBAL_") or sym.startswith("___"):
                continue
            if re.match(r"^__ZN\d+[A-Za-z0-9_]+(C2|D2)E", sym):
                continue
            sym_to_mod[sym] = mod
    return sym_to_mod


def load_exe_addresses(sym_to_mod):
    """Returns list of (addr, sym, demangled, mod) for all game functions in SCHWEIF_PORT.EXE."""
    out = subprocess.check_output(["nm", "-n", EXE_PATH], text=True, stdin=subprocess.DEVNULL)
    entries = []
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[1] == "T":
            addr = int(parts[0], 16)
            sym = parts[2]
            if sym in sym_to_mod:
                entries.append((addr, sym, sym_to_mod[sym]))

    # Demangle
    if entries:
        c_names = [s[1:] if s.startswith("_") else s for _, s, _ in entries]
        demangled = subprocess.check_output(["c++filt"] + c_names, text=True, stdin=subprocess.DEVNULL).splitlines()
    else:
        demangled = []

    result = []
    for (addr, sym, mod), dem in zip(entries, demangled):
        result.append((addr, sym, dem, mod))
    return result


def load_hit_addresses(tsv_path, sym_to_addr=None):
    """Returns dict: addr -> (order, frame)."""
    hits = {}
    if not os.path.exists(tsv_path):
        return hits
    with open(tsv_path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("Adresse") or line.startswith("Modul"):
                continue
            parts = line.split("\t")
            order = int(parts[-2]) if len(parts) >= 2 and parts[-2].isdigit() else 0
            frame = int(parts[-1]) if len(parts) >= 3 and parts[-1].isdigit() else 0
            if sym_to_addr and len(parts) >= 4 and parts[2] in sym_to_addr:
                addr = sym_to_addr[parts[2]]
                if addr not in hits:
                    hits[addr] = (order, frame)
                continue
            for p in parts:
                if p.startswith("0x") or p.startswith("0X"):
                    try:
                        addr = int(p, 16)
                        if addr not in hits:
                            hits[addr] = (order, frame)
                    except ValueError:
                        pass
                    break
    return hits


def main():
    sym_to_mod = load_module_symbols()
    exe_funcs = load_exe_addresses(sym_to_mod)
    sym_to_addr = {sym: addr for addr, sym, _, _ in exe_funcs}
    hits = load_hit_addresses(TSV_PATH, sym_to_addr)

    # Map each hit address to the enclosing function if exact start address or within +32 bytes
    addr_to_func = {}
    for i, (addr, sym, dem, mod) in enumerate(exe_funcs):
        next_addr = exe_funcs[i + 1][0] if i + 1 < len(exe_funcs) else addr + 0x1000
        addr_to_func[addr] = (addr, sym, dem, mod, next_addr)

    func_hits = {}
    for h_addr, (order, frame) in hits.items():
        if h_addr in addr_to_func:
            func_hits[h_addr] = (order, frame)
        else:
            # Find nearest function start <= h_addr
            for addr, sym, dem, mod in reversed(exe_funcs):
                if addr <= h_addr < addr + 64:
                    if addr not in func_hits:
                        func_hits[addr] = (order, frame)
                    break

    # Rewrite enriched abdeckung.tsv
    if func_hits:
        enriched = []
        for addr, sym, dem, mod in exe_funcs:
            if addr in func_hits:
                order, frame = func_hits[addr]
                enriched.append((order, frame, mod, dem, sym, addr))
        enriched.sort()
        with open(TSV_PATH, "w", encoding="utf-8") as f:
            f.write("Modul\tFunktion\tSymbol\tAdresse\tReihenfolge\tFrame\n")
            for order, frame, mod, dem, sym, addr in enriched:
                f.write(f"{mod}\t{dem}\t{sym}\t0x{addr:08X}\t{order}\t{frame}\n")

    # Group by module
    mods = {}
    for addr, sym, dem, mod in exe_funcs:
        mods.setdefault(mod, []).append((addr, sym, dem, addr in func_hits))

    total_funcs = len(exe_funcs)
    hit_funcs = len(func_hits)
    total_mods = len(mods)
    hit_mods = sum(1 for m, flist in mods.items() if any(h for _, _, _, h in flist))

    pct_f = (100.0 * hit_funcs / total_funcs) if total_funcs else 0.0
    pct_m = (100.0 * hit_mods / total_mods) if total_mods else 0.0
    print(f"=== W3 Abdeckungsbericht ===")
    print(f"Module erreicht:     {hit_mods:3d} / {total_mods:3d} ({pct_m:.1f} %)")
    print(f"Funktionen erreicht: {hit_funcs:4d} / {total_funcs:4d} ({pct_f:.1f} %)")

    if "--md" in sys.argv or "--all" in sys.argv:
        exc_mods = {"OVL011", "SEG014", "OVL221", "OVL244", "OVL245", "OVL246", "OVL247"}
        active_mods = sum(1 for m in mods if m not in exc_mods)
        active_hit = sum(1 for m, flist in mods.items() if m not in exc_mods and any(h for _, _, _, h in flist))
        pct_act = (100.0 * active_hit / active_mods) if active_mods else 0.0
        lines = [
            "# Abdeckungsbericht (W3) — DSA 2: Sternenschweif Port",
            "",
            f"- **Erreichte Module (gesamt):** {hit_mods} / {total_mods} ({pct_m:.1f} %)",
            f"- **Erreichte aktive Spielmodule:** {active_hit} / {active_mods} ({pct_act:.1f} %)",
            f"- **Erreichte Funktionen:** {hit_funcs} / {total_funcs} ({pct_f:.1f} %)",
            "",
            "## Begründete Ausnahmemodule (7 Module ohne regulären Spielaufruf)",
            "",
            "| Modul | Funktionen | Begründung (Quelltext-Nachweis) |",
            "|---|---:|---|",
            "| `OVL011` | 8 | Interner Entwickler-Infobildschirm (`dev_info_screen`, `heap_free_total`, `debug_log`), im regulären Spielablauf abgeschaltet (`Port/src/OVL011.CPP`). |",
            "| `SEG014` | 2 | Fataler Absturz-Bildschirm (`guru_meditation`, `push_fc`), wird nur bei Speicher-/Dateifehlern angesprungen (`Port/src/SEG014.C`). |",
            "| `OVL221` | 17 | Reine Leer-Platzhalter (`o221_stub_0` .. `o221_stub_16`) ohne Aufrufer im Original-Kompilat (`Port/src/OVL221.C`). |",
            "| `OVL244` | 1 | Reiner Leer-Platzhalter (`o244_stub_0`) ohne Aufrufer im Original-Kompilat (`Port/src/OVL244.C`). |",
            "| `OVL245` | 3 | Reine Leer-Platzhalter (`o245_stub_0` .. `o245_stub_2`) ohne Aufrufer im Original-Kompilat (`Port/src/OVL245.C`). |",
            "| `OVL246` | 5 | Reine Leer-Platzhalter (`o246_stub_0` .. `o246_stub_4`) ohne Aufrufer im Original-Kompilat (`Port/src/OVL246.C`). |",
            "| `OVL247` | 2 | Reine Leer-Platzhalter (`o247_stub_0` .. `o247_stub_1`) ohne Aufrufer im Original-Kompilat (`Port/src/OVL247.C`). |",
            "",
            "## Übersicht je Modul",
            "",
            "| Modul | Erreicht | Gesamt | Quote | Nicht erreichte Funktionen |",
            "|---|---:|---:|---:|---|",
        ]
        for mod in sorted(mods.keys()):
            flist = mods[mod]
            h_cnt = sum(1 for _, _, _, h in flist if h)
            t_cnt = len(flist)
            q = 100.0 * h_cnt / t_cnt if t_cnt else 0.0
            missed = [dem for _, _, dem, h in flist if not h]
            missed_str = ", ".join(f"`{m}`" for m in missed[:6])
            if len(missed) > 6:
                missed_str += f" (+{len(missed) - 6} weitere)"
            lines.append(f"| `{mod}` | {h_cnt} | {t_cnt} | {q:.0f} % | {missed_str} |")
        with open(MD_PATH, "w", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")
        print(f"Geschrieben: {MD_PATH}")


if __name__ == "__main__":
    main()
