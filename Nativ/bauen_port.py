#!/usr/bin/env python3
"""
bauen_port.py - Gesamtbau-Skript fuer den nativen 32-Bit-Windows-Port (Etappe 1)
von DSA 2: Sternenschweif (CD-Version C1.02 de).

Verwendung:
    python3 Port/bauen_port.py                  # baut FIX_BUGS=2 (alles) -> SCHWEIF_PORT.EXE + SCHWEIF_PORT_ALLES.EXE
    python3 Port/bauen_port.py wichtig          # baut FIX_BUGS=1         -> SCHWEIF_PORT.EXE + SCHWEIF_PORT_WICHTIG.EXE
    python3 Port/bauen_port.py original         # baut FIX_BUGS=0         -> SCHWEIF_PORT.EXE + SCHWEIF_PORT_ORIG.EXE
    python3 Port/bauen_port.py alle_stufen      # baut alle 3 Stufen (ORIG, WICHTIG, ALLES + SCHWEIF_PORT.EXE)
    python3 Port/bauen_port.py alles -DFIX_0042=0 -DFIX_PROBENLOG=1 -DPORT_SKIP_COPYPROT=0

Ablauf:
1. Fuehrt Port/tools/prepare_sources.py aus (kopiert Quelle/ nach Port/src/
   und wendet die nativen Port-Anpassungen an; Quelle/ bleibt 100 % unangetastet).
2. Kompiliert die 301 C/C++-Module des Spiels sowie die SDL2-Plattformschicht parallel mit
   32-Bit-MinGW-GCC (-m32 -mabi=sysv -fpack-struct=1 -funsigned-char).
3. Erzeugt ueber Port/tools/generate_bridges.py die C/C++-Symbolbruecken (port_bridges.cpp)
   und Daten-Aliase (data_aliases.ld) und linkt das native Windows-Executable nach Port/build/.
"""

from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor, as_completed
import glob
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

sys.dont_write_bytecode = True
os.environ["PYTHONDONTWRITEBYTECODE"] = "1"

PORT_DIR = Path(__file__).resolve().parent
ROOT_DIR = PORT_DIR.parent


def _default_win_root() -> Path:
    if os.name == "nt":
        return Path(os.environ.get("SystemDrive", "C:") + "\\")
    parts = PORT_DIR.parts
    if len(parts) >= 3 and len(parts[2]) == 1:
        return Path("/") / parts[1] / parts[2]
    return Path("/tmp")


_WIN_ROOT = _default_win_root()
MINGW_BIN_WSL = os.environ.get(
    "SCHWEIF_MINGW_BIN",
    str(_WIN_ROOT / "Program Files (x86)" / "Embarcadero" / "Dev-Cpp" / "TDM-GCC-64" / "bin"),
)

STUFEN = {
    "original": (0, "SCHWEIF_PORT_ORIG.EXE"),
    "0":        (0, "SCHWEIF_PORT_ORIG.EXE"),
    "wichtig":  (1, "SCHWEIF_PORT_WICHTIG.EXE"),
    "1":        (1, "SCHWEIF_PORT_WICHTIG.EXE"),
    "alles":    (2, "SCHWEIF_PORT_ALLES.EXE"),
    "2":        (2, "SCHWEIF_PORT_ALLES.EXE"),
}

WRAP_FLAGS = [
    "-Wl,--wrap=_open", "-Wl,--wrap=open",
    "-Wl,--wrap=_creat", "-Wl,--wrap=creat",
    "-Wl,--wrap=_read", "-Wl,--wrap=read",
    "-Wl,--wrap=_write", "-Wl,--wrap=write",
    "-Wl,--wrap=_lseek", "-Wl,--wrap=lseek",
    "-Wl,--wrap=_close", "-Wl,--wrap=close",
    "-Wl,--wrap=_filelength", "-Wl,--wrap=filelength",
    "-Wl,--wrap=_tell", "-Wl,--wrap=tell",
    "-Wl,--wrap=_unlink", "-Wl,--wrap=unlink",
    "-Wl,--wrap=_chdir", "-Wl,--wrap=chdir",
    "-Wl,--wrap=_mkdir", "-Wl,--wrap=mkdir",
]


def get_mingw_env() -> dict[str, str]:
    env = os.environ.copy()
    env["PATH"] = f"{MINGW_BIN_WSL}:{env.get('PATH', '')}"
    env["WSLENV"] = "PATH/l"
    return env


def strip_data_align(asm_path: Path) -> None:
    asm_txt = asm_path.read_text(encoding="latin-1")
    out_lines: list[str] = []
    in_data_or_bss = False
    for line in asm_txt.splitlines(True):
        s = line.strip()
        if s.startswith(".data") or s.startswith(".bss"):
            in_data_or_bss = True
        elif s.startswith(".text") or s.startswith(".section"):
            in_data_or_bss = False
        if in_data_or_bss and s.startswith(".align"):
            continue
        out_lines.append(line)
    asm_path.write_text("".join(out_lines), encoding="latin-1")


def compile_unit(cmd: list[str], env: dict[str, str], cwd: Path | None = None) -> tuple[int, str]:
    res = subprocess.run(
        cmd,
        cwd=str(cwd or ROOT_DIR),
        env=env,
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="latin-1",
        timeout=180,
    )
    return res.returncode, res.stdout


def patch_pe_rdata_writable(exe_path: Path) -> None:
    data = bytearray(exe_path.read_bytes())
    idx = data.find(b".rdata\x00\x00")
    if idx != -1:
        flags = int.from_bytes(data[idx + 36 : idx + 40], "little") | 0x80000000
        data[idx + 36 : idx + 40] = flags.to_bytes(4, "little")
        exe_path.write_bytes(data)
        time.sleep(0.3)


def build_stufe(fix_bugs: int, exe_name: str, extra_defines: list[str], copy_to_default: bool) -> Path:
    env = get_mingw_env()
    build_dir = PORT_DIR / "build"
    obj_dir = build_dir / ("objs" if fix_bugs == 2 and not extra_defines else f"objs_fix{fix_bugs}")
    obj_dir.mkdir(parents=True, exist_ok=True)

    def_flags = [f"-DFIX_BUGS={fix_bugs}", "-DPORT_SKIP_COPYPROT=1"] + extra_defines
    inc_flags = [
        "-IPort/platform/compat",
        "-IPort/platform",
        "-IPort/src/inc",
        "-IPort/src",
        "-IPort/third_party/SDL2/include",
        "-IPort/third_party/SDL2/include/SDL2",
        "-IPort/third_party/stb_vorbis",
    ]
    common_flags = [
        "-m32", "-mabi=sysv", "-O2", "-fno-toplevel-reorder",
        "-fno-zero-initialized-in-bss", "-fno-common",
        "-fno-aggressive-loop-optimizations", "-fno-strict-aliasing",
        "-fwrapv", "-fno-delete-null-pointer-checks",
        "-fpack-struct=1", "-funsigned-char", "-w",
        "-Dfar=", "-Dhuge=", "-Dnear=", "-Dinterrupt=", "-Dcdecl=",
    ]

    src_dir = PORT_DIR / "src"
    game_sources = sorted(
        list(src_dir.glob("SEG*.C"))
        + list(src_dir.glob("SEG*.CPP"))
        + list((src_dir / "Overlays").glob("OVL*.C"))
        + list((src_dir / "Overlays").glob("OVL*.CPP"))
    )

    h_mtime = max((p.stat().st_mtime for p in (src_dir / "inc").glob("*.h")), default=0.0)
    ref_obj_dir = build_dir / "objs"
    fix_a_macros: set[str] = set()
    if fix_bugs == 1 and not extra_defines and (src_dir / "inc" / "s_fix.h").exists():
        import re
        fix_h_txt = (src_dir / "inc" / "s_fix.h").read_text(encoding="latin-1", errors="ignore")
        fix_a_macros = set(re.findall(r"#define\s+(FIX_\d+)\s+FIX_STUFE_A", fix_h_txt))

    c_game: list[Path] = []
    cpp_game: list[Path] = []
    for s_path in game_sources:
        stem = s_path.stem
        obj_path = obj_dir / f"{stem}.o"
        if obj_path.exists() and obj_path.stat().st_mtime >= max(s_path.stat().st_mtime, h_mtime):
            continue
        if obj_dir != ref_obj_dir and not extra_defines:
            ref_o = ref_obj_dir / f"{stem}.o"
            if ref_o.exists() and ref_o.stat().st_mtime >= max(s_path.stat().st_mtime, h_mtime):
                src_txt = s_path.read_text(encoding="latin-1", errors="ignore")
                if ("FIX_" not in src_txt and "s_plog.h" not in src_txt) or (fix_bugs == 1 and not any(m in src_txt for m in fix_a_macros)):
                    shutil.copy2(ref_o, obj_path)
                    continue
        if s_path.suffix.upper() == ".C":
            c_game.append(s_path)
        else:
            cpp_game.append(s_path)

    plat_sources = [
        (PORT_DIR / "platform" / "schweif_asm_and_platform.cpp", obj_dir / "schweif_asm_and_platform.o", True),
        (PORT_DIR / "platform" / "dsa_platform_sdl2.cpp", obj_dir / "dsa_platform_sdl2.o", True),
        (PORT_DIR / "third_party" / "SDL2" / "sdl2_win32_provider.c", obj_dir / "sdl2_win32_provider.o", False),
        (PORT_DIR / "third_party" / "stb_vorbis" / "stb_vorbis.c", obj_dir / "stb_vorbis.o", False),
        (PORT_DIR / "platform" / "dsa_audio_cd.cpp", obj_dir / "dsa_audio_cd.o", True),
    ]
    plat_tasks: list[tuple[str, list[str]]] = []
    for p_src, p_obj, is_cpp in plat_sources:
        if p_obj.exists() and p_obj.stat().st_mtime >= p_src.stat().st_mtime:
            continue
        if obj_dir != ref_obj_dir and not extra_defines:
            ref_po = ref_obj_dir / p_obj.name
            if ref_po.exists() and ref_po.stat().st_mtime >= p_src.stat().st_mtime:
                shutil.copy2(ref_po, p_obj)
                continue
        rel_src = p_src.relative_to(ROOT_DIR).as_posix()
        rel_obj = p_obj.relative_to(ROOT_DIR).as_posix()
        # stb_vorbis und waveOut-Header duerfen nicht mit -fpack-struct=1 uebersetzt werden.
        unit_flags = [f for f in common_flags if not (p_src.name in ("stb_vorbis.c", "dsa_audio_cd.cpp") and f == "-fpack-struct=1")]
        if is_cpp:
            cmd = ["g++.exe", "-fpermissive"] + unit_flags + def_flags + inc_flags + ["-c", rel_src, "-o", rel_obj]
        else:
            cmd = ["gcc.exe", "-std=gnu99"] + unit_flags + inc_flags + ["-c", rel_src, "-o", rel_obj]
        plat_tasks.append((p_src.name, cmd))

    total_units = len(c_game) + len(cpp_game) + len(plat_tasks)
    if total_units > 0:
        print(f"[2/4] Kompiliere {total_units} Einheiten fuer FIX_BUGS={fix_bugs} ({exe_name}) im Batch-Modus...")
        inst_flags = ["-finstrument-functions", "-finstrument-functions-exclude-file-list=inc,include,compat"]
        obj_inc_flags = [
            "-I../../platform/compat",
            "-I../../platform",
            "-I../../src/inc",
            "-I../../src",
            "-I../../third_party/SDL2/include",
            "-I../../third_party/SDL2/include/SDL2",
        ]
        asm_files: list[Path] = []
        for is_cpp, file_list in ((False, c_game), (True, cpp_game)):
            for i in range(0, len(file_list), 50):
                chunk = file_list[i : i + 50]
                rel_srcs = [os.path.relpath(p, obj_dir).replace("\\", "/") for p in chunk]
                if not is_cpp:
                    cmd_s = ["gcc.exe", "-x", "c", "-std=gnu99"] + inst_flags + common_flags + def_flags + obj_inc_flags + ["-S"] + rel_srcs
                else:
                    cmd_s = ["g++.exe", "-fpermissive"] + inst_flags + common_flags + def_flags + obj_inc_flags + ["-S"] + rel_srcs
                rc, out = compile_unit(cmd_s, env, cwd=obj_dir)
                if rc != 0:
                    print(f"FEHLER beim Kompilieren:\n{out}", file=sys.stderr)
                    sys.exit(1)
                for p in chunk:
                    asm_p = obj_dir / f"{p.stem}.s"
                    strip_data_align(asm_p)
                    asm_files.append(asm_p)

        for i in range(0, len(asm_files), 50):
            chunk = asm_files[i : i + 50]
            s_names = [p.name for p in chunk]
            rc, out = compile_unit(["gcc.exe", "-m32", "-c"] + s_names, env, cwd=obj_dir)
            for p in chunk:
                try:
                    p.unlink()
                except OSError:
                    pass
            if rc != 0:
                print(f"FEHLER beim Assemblieren:\n{out}", file=sys.stderr)
                sys.exit(1)

        for name, cmd in plat_tasks:
            rc, out = compile_unit(cmd, env)
            if rc != 0:
                print(f"FEHLER in {name}:\n{out}", file=sys.stderr)
                sys.exit(1)
    else:
        print(f"[2/4] Alle Objektdateien fuer FIX_BUGS={fix_bugs} aktuell.")

    game_recompiled = bool(c_game or cpp_game)
    bridges_obj = obj_dir / "port_bridges.o"
    gen_script = PORT_DIR / "tools" / "generate_bridges.py"
    if game_recompiled or not bridges_obj.exists() or gen_script.stat().st_mtime > bridges_obj.stat().st_mtime:
        print("[3/4] Erzeuge C/C++-Bruecken (generate_bridges.py)...")
        subprocess.run(
            [sys.executable, str(PORT_DIR / "tools" / "generate_bridges.py")],
            cwd=str(ROOT_DIR),
            check=True,
            stdin=subprocess.DEVNULL,
            timeout=60,
        )
    bridges_src = PORT_DIR / "src" / "port_bridges.cpp"
    ref_bridges_obj = ref_obj_dir / "port_bridges.o"
    if not bridges_obj.exists() and obj_dir != ref_obj_dir and ref_bridges_obj.exists() and ref_bridges_obj.stat().st_mtime >= bridges_src.stat().st_mtime:
        shutil.copy2(ref_bridges_obj, bridges_obj)
    if not bridges_obj.exists() or bridges_src.stat().st_mtime > bridges_obj.stat().st_mtime:
        rc, out = compile_unit(
            ["g++.exe", "-fpermissive"]
            + common_flags
            + ["-c", "Port/src/port_bridges.cpp", "-o", bridges_obj.relative_to(ROOT_DIR).as_posix()],
            env,
        )
        if rc != 0:
            print(f"FEHLER in port_bridges.cpp:\n{out}", file=sys.stderr)
            sys.exit(1)
    else:
        print("[3/4] C/C++-Bruecken (port_bridges.o) aktuell.")

    out_exe = build_dir / exe_name
    objs = sorted(p.relative_to(ROOT_DIR).as_posix() for p in obj_dir.glob("*.o"))
    link_cmd = (
        ["g++.exe", "-m32", "-mwindows", "-O2", "-Wl,-S", "-Wl,--allow-multiple-definition"]
        + WRAP_FLAGS
        + objs
        + [
            "Port/build/data_aliases.ld",
            "-lwinmm",
            "-lgdi32",
            "-luser32",
            "-lshell32",
            "-o",
            out_exe.relative_to(ROOT_DIR).as_posix(),
        ]
    )
    print(f"[4/4] Linke {out_exe.relative_to(ROOT_DIR)}...")
    rc, out = compile_unit(link_cmd, env)
    if rc != 0:
        print(f"LINKER-FEHLER:\n{out}", file=sys.stderr)
        sys.exit(1)

    patch_pe_rdata_writable(out_exe)
    release_dirs = [PORT_DIR / "release"] + list(ROOT_DIR.parent.glob("*Schicksalsklinge*/Starter/release"))
    for rdir in release_dirs:
        if rdir.exists():
            shutil.copy2(out_exe, rdir / exe_name)
    if copy_to_default:
        default_exe = build_dir / "SCHWEIF_PORT.EXE"
        if out_exe != default_exe:
            shutil.copy2(out_exe, default_exe)
            print(f"      Kopiert nach {default_exe.relative_to(ROOT_DIR)} ({default_exe.stat().st_size:,} Bytes)")
        for rdir in release_dirs:
            if rdir.exists():
                shutil.copy2(default_exe, rdir / "SCHWEIF_PORT.EXE")
    return out_exe


def main() -> int:
    args = sys.argv[1:]
    stufe_arg = "alles"
    extra_defines: list[str] = []
    for a in args:
        if a.startswith("-D"):
            extra_defines.append(a)
        elif a != "--prepare":
            stufe_arg = a.lower()

    prep_py = PORT_DIR / "tools" / "prepare_sources.py"
    schweif_h = PORT_DIR / "src" / "inc" / "schweif.h"
    if not schweif_h.exists() or "--prepare" in args or prep_py.stat().st_mtime > schweif_h.stat().st_mtime:
        print("[1/4] Bereite Quellen in Port/src/ vor...")
        subprocess.run(
            [sys.executable, str(PORT_DIR / "tools" / "prepare_sources.py")],
            cwd=str(ROOT_DIR),
            check=True,
            stdin=subprocess.DEVNULL,
            timeout=60,
        )
    else:
        print("[1/4] Port/src/ bereits vorbereitet (nutze --prepare zum Neuaufsetzen).")

    if stufe_arg == "alle_stufen":
        for key in ("alles", "wichtig", "original"):
            fb, exe_name = STUFEN[key]
            build_stufe(fb, exe_name, extra_defines, copy_to_default=(key == "alles"))
    else:
        if stufe_arg not in STUFEN:
            print(f"Unbekannte Stufe '{stufe_arg}'. Erlaubt: original (0), wichtig (1), alles (2), alle_stufen", file=sys.stderr)
            return 1
        fb, exe_name = STUFEN[stufe_arg]
        build_stufe(fb, exe_name, extra_defines, copy_to_default=True)

    print("Bau erfolgreich abgeschlossen.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
