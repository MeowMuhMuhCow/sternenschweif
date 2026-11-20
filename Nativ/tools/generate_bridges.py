#!/usr/bin/env python3
"""
generate_bridges.py - Erzeugt Port/src/port_bridges.cpp und Port/build/data_aliases.ld
"""
import os
import re
import glob
import csv
import subprocess
import sys

PORT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = os.path.dirname(PORT_DIR)
for _cand in [ROOT, os.path.dirname(ROOT)] + sorted(glob.glob(os.path.join(PORT_DIR, "..", "..", "..", "*"))):
    if os.path.exists(os.path.join(_cand, "Quelle", "inc", "schweif.h")):
        ROOT = os.path.abspath(_cand)
        break
_parts = ROOT.split(os.sep)
_win_root = os.path.join(os.sep, _parts[1], _parts[2]) if len(_parts) >= 3 and len(_parts[2]) == 1 else "/tmp"
MINGW = os.environ.get(
    "SCHWEIF_MINGW_BIN",
    os.path.join(_win_root, "Program Files (x86)", "Embarcadero", "Dev-Cpp", "TDM-GCC-64", "bin"),
)

GLOBAL_ALIASES = [
    ("_g_acrobatics_timer",     "_g_ingame_timers + 24"),
    ("_g_music_timer",          "_g_ingame_timers + 28"),
    ("_g_dance_timer",          "_g_ingame_timers + 112"),
    ("_g_bmp_mark_le",          "_g_3327"),
    ("_g_bmp_mark_ae",          "_g_3331"),
    ("_g_cam_ang_y",            "_g_cam_ang + 2"),
    ("_g_cam_ang_z",            "_g_cam_ang + 4"),
    ("_g_camp_watch_names",     "_g_camp_job_names + 48"),
    ("_g_clip_y0",              "_g_clip_rect + 2"),
    ("_g_clip_x1",              "_g_clip_rect + 4"),
    ("_g_clip_y1",              "_g_clip_rect + 6"),
    ("_g_copyprot_sum",         "_g_portrait_marks + 24"),
    ("_g_mouse_bg_cw",          "_g_mouse_bg + 4"),
    ("_g_mouse_bg_ch",          "_g_mouse_bg + 6"),
    ("_g_cursor_w",             "_g_mouse_bg + 8"),
    ("_g_cursor_h",             "_g_mouse_bg + 10"),
    ("_g_diary_ctrl_entry",     "_g_diary_ctrl + 1"),
    ("_g_diary_ctrl_own",       "_g_diary_ctrl + 2"),
    ("_g_diary_ctrl_end",       "_g_diary_ctrl + 3"),
    ("_g_diary_tag4",           "_g_diary_tag + 4"),
    ("_g_diary_tag5",           "_g_diary_tag + 5"),
    ("_g_dng06_statue_ducats",  "_g_dng05_offered_ducats"),
    ("_g_encounters",           "_g_fight_defs_ab + 3"),
    ("_g_fli_vga",              "_g_fli_screen + 10"),
    ("_g_fli_pal",              "_g_fli_screen + 14"),
    ("_g_fmt_cd_path",          "_g_file_names + 16"),
    ("_g_fmt_temp_path",        "_g_file_names + 20"),
    ("_g_groups",               "_g_c30f"),
    ("_g_gui_buttons_pos_y",    "_g_gui_buttons_pos + 2"),
    ("_g_market_rolls",         "_g_market_rolls_head"),
    ("_g_market_shop",          "_g_market_buy"),
    ("_g_mon_spell_ani",        "_g_mon_spell_descriptions + 7"),
    ("_g_mouse_min_x",          "_g_mouse_range + 0"),
    ("_g_mouse_min_y",          "_g_mouse_range + 2"),
    ("_g_mouse_max_x",          "_g_mouse_range + 4"),
    ("_g_mouse_max_y",          "_g_mouse_range + 6"),
    ("_g_playflags",            "_g_anims_on"),
    ("_g_proj_dist",            "_g_unused_8c74 + 4"),
    ("_g_proj_scale",           "_g_unused_8c74 + 6"),
    ("_g_portrait_index_m_next","_g_portrait_index_m + 1"),
    ("_g_screen_pic_run",       "_g_screen_pic_rest + 6"),
    ("_g_skill_max_incs",       "_g_skill_attribs + 3"),
    ("_g_spell_ani",            "_g_spell_descriptions + 6"),
    ("_g_spell_flags",          "_g_spell_by_item"),
    ("_g_spell_snd",            "_g_spell_descriptions + 10"),
    ("_g_tmap_mark1_x",         "_g_tmap_rest + 64"),
    ("_g_tmap_mark1_y",         "_g_tmap_rest + 66"),
    ("_g_tmap_mark2_x",         "_g_tmap_rest + 68"),
    ("_g_tmap_mark2_y",         "_g_tmap_rest + 70"),
    ("_g_town_cells_state",     "_g_flags"),
    ("_g_weapons_bf",           "_g_weapons_table + 3"),
    ("_g_weapons_atpa",         "_g_weapons_table + 5"),
    ("_g_weapons_class",        "_g_weapons_table + 7"),
]


def parse_cpp_methods_in_source_order(src_path):
    """Extracts (ClassName, method_name) definitions in source order from a .CPP file."""
    with open(src_path, "r", encoding="latin1") as f:
        text = f.read()
    # Strip comments
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    methods = []
    for line in text.splitlines():
        if not line or line[0].isspace() or line.startswith("#"):
            continue
        m = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)::(\~?[A-Za-z_][A-Za-z0-9_]*)\s*\(", line)
        if m:
            cls, meth = m.group(1), m.group(2)
            if meth == cls:
                methods.append((cls, "ctor"))
            elif meth == "~" + cls:
                methods.append((cls, "dtor"))
            else:
                methods.append((cls, meth))
    return methods


def demangle_cls_meth(sym):
    m = re.match(r"^__ZN(\d+)([A-Za-z0-9_]+)$", sym)
    if not m:
        return None
    n1 = int(m.group(1))
    rest = m.group(2)
    cls = rest[:n1]
    rest2 = rest[n1:]
    if rest2.startswith("C1E"):
        return (cls, "ctor")
    if rest2.startswith("D1E"):
        return (cls, "dtor")
    m2 = re.match(r"^(\d+)([A-Za-z0-9_]+)", rest2)
    if m2:
        n2 = int(m2.group(1))
        meth = m2.group(2)[:n2]
        return (cls, meth)
    return None


def _strip_coff_underscore(s: str, is_elf: bool) -> str:
    if is_elf and s.startswith("_"):
        return s[1:]
    return s


def main():
    is_elf = "--elf" in sys.argv[1:]
    obj_dir = os.path.join(PORT_DIR, "build", "objs")
    ld_path = os.path.join(PORT_DIR, "build", "data_aliases.ld")
    out_cpp = os.path.join(PORT_DIR, "src", "port_bridges.cpp")
    for a in sys.argv[1:]:
        if a.startswith("--obj-dir="):
            obj_dir = a.split("=", 1)[1]
        elif a.startswith("--out-ld="):
            ld_path = a.split("=", 1)[1]
        elif a.startswith("--out-cpp="):
            out_cpp = a.split("=", 1)[1]

    env = os.environ.copy()
    if not is_elf:
        env["PATH"] = MINGW + ":" + env.get("PATH", "")
        env["WSLENV"] = "PATH/l"

    # 1. Write data_aliases.ld
    os.makedirs(os.path.dirname(ld_path), exist_ok=True)
    with open(ld_path, "w", encoding="utf-8") as f:
        for alias, expr in GLOBAL_ALIASES:
            if is_elf:
                alias_out = alias[1:] if alias.startswith("_") else alias
                expr_out = re.sub(r"\b_([A-Za-z_][A-Za-z0-9_]*)", r"\1", expr)
                f.write(f"{alias_out} = {expr_out};\n")
            else:
                f.write(f"{alias} = {expr};\n")

    # 2. Inspect all object files (excluding port_bridges.o)
    os.chdir(ROOT)
    rel_obj_dir = os.path.relpath(obj_dir, ROOT).replace("\\", "/")
    objs = [p.replace("\\", "/") for p in sorted(glob.glob(os.path.join(rel_obj_dir, "*.o")))
            if not p.endswith("port_bridges.o")]
    nm_bin = "nm" if is_elf else f"{MINGW}/nm.exe"
    r = subprocess.run([nm_bin, "-n", "-A"] + objs,
                       env=env, capture_output=True, stdin=subprocess.DEVNULL, text=True, timeout=30)

    defined = {}
    undef = set()
    obj_t = {}
    for line in r.stdout.splitlines():
        if ":" not in line:
            continue
        f_part, rest = line.rsplit(":", 1)
        f = os.path.basename(f_part)
        toks = rest.split()
        if not toks:
            continue
        if toks[0] == "U" and len(toks) >= 2:
            raw_sym = toks[1]
            sym = ("_" + raw_sym) if is_elf else raw_sym
            undef.add(sym)
        elif len(toks) >= 3:
            t = toks[1]
            raw_sym = toks[2]
            if is_elf and raw_sym.startswith("__x86.get_pc_thunk"):
                continue
            sym = ("_" + raw_sym) if is_elf else raw_sym
            if t in ("T", "D", "B", "R", "C", "W", "V", "A"):
                defined[sym] = f
            if t == "T" and not sym.startswith("__GLOBAL_") and not sym.startswith("___") and sym not in ("_dsa_noop_isr", "__Z7sprintfPcPKcz"):
                if not re.match(r"^__ZN\d+[A-Za-z0-9_]+(C2|D2)E", sym):
                    obj_t.setdefault(f, []).append(sym)

    # 3. Map C names from Analyse/Namen/namen.tsv using function order in each module
    mod_funcs = {}
    with open(os.path.join(ROOT, "Analyse/Namen/namen.tsv"), "r", encoding="utf-8") as f:
        for row in csv.DictReader(f, delimiter="\t"):
            ln = row["Linkname"]
            nm = row["Name"]
            if nm.startswith("@"):
                continue
            m = re.match(r"^[FL]_(R\d+|O\d+)_([0-9A-Fa-f]{4})$", ln)
            if m:
                mod = m.group(1)
                off = int(m.group(2), 16)
                mod_funcs.setdefault(mod, []).append((off, nm))
    for mod in mod_funcs:
        mod_funcs[mod].sort()

    c_to_cpp = {}
    for f, filt in sorted(obj_t.items()):
        if not f.startswith(("SEG", "OVL")):
            continue
        mod = ("R" + f[4:6]) if f.startswith("SEG") else ("O" + f[3:6])
        n_list = mod_funcs.get(mod, [])
        if len(filt) != len(n_list) and any(s.startswith("__ZN") for s in filt):
            filt = [s for s in filt if not re.match(r"^__Z\d+", s)]
        if len(filt) == len(n_list):
            for (_, c_name), sym in zip(n_list, filt):
                if "::" not in c_name and "_" + c_name != sym:
                    c_to_cpp["_" + c_name] = sym

    # Explicit overrides for overloaded / special functions
    explicit_c_to_cpp = {
        "_enter_location":          "__Z14enter_locationv",
        "_save_import_heroes":      "__ZN4Save13import_heroesEss",
        "_save_import_heroes_v1":   "__ZN4Save16import_heroes_v1Ess",
        "_Inv_ctor":                "__ZN6InvObjC1Ev",
        "_Inv_dtor":                "__ZN6InvObjD1Ev",
        "_saver_ctor":              "__ZN5saverC1Ev",
        "_file_open":               "__ZN4File4openEPcs",
        "_file_open_num":           "__ZN4File4openEss",
        "_file_read_part":          "__ZN4File4readEPvll",
        "_file_read":               "__ZN4File4readEPvl",
        "_dlg_open":                "__ZN3Win4openEv",
        "_dlg_open_2":              "__ZN3Win4openEs",
        "_dlg_open_rect":           "__ZN3Win4openEssss",
        "_dlg_message_n":           "__ZN3Dlg7messageEPcs",
        "_dlg_message":             "__ZN3Dlg7messageEPc",
        "_dlg_2943":                "__ZN3Dlg6chooseEPcsz",
        "_dlg_choose":              "__ZN3Dlg6chooseEPcsz",
        "_dlg_input_num":           "__ZN3Dlg9input_numEPcsss",
        "_dlg_input":               "__ZN3Dlg5inputEPcs",
        "_dlg_text_box":            "__ZN3Win8text_boxEPc",
        "_dlg_text_box_2":          "__ZN3Win8text_boxEPcs",
        "_dlg_text_box_xy":         "__ZN3Win8text_boxEPcss",
        "_pic_load_2":              "__ZN3Pic4loadEPc",
        "_pic_load_3":              "__ZN3Pic4loadEPcs",
        "_pic_load_4":              "__ZN3Pic4loadEs",
        "_pic_load":                "__ZN3Pic4loadEss",
        "_Pic_ctor":                "__ZN3PicC1EPc",
        "_Pic_ctor_2":              "__ZN3PicC1EPcss",
        "_Pic_ctor_3":              "__ZN3PicC1Ess",
        "_Pic_ctor_4":              "__ZN3PicC1Ev",
        "_Pic_dtor":                "__ZN3PicD1Ev",
        "_nvf_unpack_2":            "__ZN3Nvf6unpackEPvS0_ssPsS1_s",
        "_nvf_read":                "__ZN3Nvf6unpackEPvsPsS1_",
        "_event_fight":             "__ZN11area_events5fightEssssssssssss",
        "_event_fight1":            "__ZN11area_events6fight1Esss",
        "_event_fight2":            "__ZN11area_events6fight2Essssss",
        "_hero_check_skill_3":      "__ZN4hero11check_skillEssPs",
        "_hero_check_skill":        "__ZN4hero11check_skillEss",
        "_hero_add_item_2":         "__ZN4hero8add_itemEss",
        "_hero_add_item":           "__ZN4hero8add_itemEsss",
        "_snd_play_fx":             "__ZN5Sound4playEs",
        "_snd_play_fx_vol":         "__ZN5Sound4playEss",
        "_Speech_play":             "__ZN6Speech4playEs",
        "_Speech_play_2":           "__ZN6Speech4playEss",
        "_Win_ctor":                "__ZN3WinC1Ev",
        "_Win_ctor_copy":           "__ZN3WinC1ERS_",
        "_Win_dtor":                "__ZN3WinD1Ev",
        "_Dlg_ctor":                "__ZN3DlgC1Ev",
        "_Dlg_ctor_copy":           "__ZN3DlgC1ER3Win",
        "_Dlg_dtor":                "__ZN3DlgD1Ev",
        "_Chest_ctor":              "__ZN5ChestC1EPcPFsPvES1_s",
        "_Chest_ctor_2":            "__ZN5ChestC1EPcs",
        "_Chest_dtor":              "__ZN5ChestD1Ev",
    }
    for k, v in explicit_c_to_cpp.items():
        if v in defined:
            c_to_cpp[k] = v

    # Map C031::fxxxx (in OVL031.o) by offset in mod_funcs["O031"]
    ev_map = {}
    o031_t = obj_t.get("OVL031.o", [])
    o031_n = mod_funcs.get("O031", [])
    if len(o031_t) == len(o031_n):
        off_to_sym = {off: sym for (off, _), sym in zip(o031_n, o031_t)}
        for s in undef:
            m = re.match(r"^__ZN4C0315f([0-9a-f]{4})Ev$", s)
            if m:
                off = int(m.group(1), 16)
                if off in off_to_sym:
                    ev_map[s] = off_to_sym[off]

    # 4. C++ -> C / C++ bridges
    cpp_to_c = {
        "__Z12_vector_new_Pvjmjz": "__vector_new_",
        "__ZN7CharGen3runEs": "_chargen_run",
        "__ZN7CharGen3runEv": "_chargen_run",
        "__ZN3Nvf4loadEsPvssssPsS1_s": "__wrap_Nvf_load9",
        # Bmp -> gb_*
        "__ZN3BmpC1Ev": "_gb_new0",
        "__ZN3BmpC1EssPv": "_gb_new",
        "__ZN3BmpC1Ess": "_gb_new_alloc",
        "__ZN3BmpD1Ev": "__wrap_Bmp_dtor",
        "__ZN3Bmp4initEssPv": "_gb_init",
        "__ZN3Bmp10free_linesEv": "_gb_free_lines",
        "__ZN3Bmp3putEPvss": "_gb_blit",
        "__ZN3Bmp3putEPvsss": "_gb_blit_m",
        "__ZN3Bmp8put_partEPvssss": "_gb_blit_wh",
        "__ZN3Bmp8put_partEPvsssss": "_gb_blit_whm",
        "__ZN3Bmp8put_partEPvsssssss": "_gb_blit_ab",
        "__ZN3Bmp3getEPvss": "_gb_save",
        "__ZN3Bmp8set_baseEPv": "_gb_set_base",
        "__ZN3Bmp4loadEPv": "_gb_load_mem",
        "__ZN3Bmp4dataEv": "_gb_data",
        "__ZN3Bmp5scaleEPS_": "_gb_scale",
        "__ZN3Bmp5clearEv": "_gb_clear",
        "__ZN3Bmp4fillEs": "_gb_fill",
        "__ZN3Bmp4sizeEPsS0_": "_gb_get_size",
        "__ZN3Bmp4overEPS_sss": "_gb_put",
        # NvfMem* -> nvf_*
        "__ZN6NvfMemC1Ev": "_nvf_new",
        "__ZN6NvfMemD1Ev": "__wrap_Nvf_dtor",
        "__ZN6NvfMem6unpackEPvS0_scPsS1_s": "_nvf_unpack",
        "__ZN6NvfMem6unpackEPvS0_ssPsS1_s": "_nvf_unpack",
        "__ZN7NvfMem0C1Ev": "_nvf_new",
        "__ZN7NvfMem0D1Ev": "__wrap_Nvf_dtor",
        "__ZN7NvfMem06unpackEPvS0_ssPsS1_s": "_nvf_unpack",
        "__ZN7NvfMemUC1Ev": "_nvf_new",
        "__ZN7NvfMemUD1Ev": "__wrap_Nvf_dtor",
        "__ZN7NvfMemU6unpackEPvS0_scPtS1_s": "_nvf_unpack",
        "__ZN7NvfMemSC1Ev": "_nvf_new",
        "__ZN7NvfMemSD1Ev": "__wrap_Nvf_dtor",
        "__ZN7NvfMemS6unpackEPvS0_ssPsS1_s": "_nvf_unpack",
        # OVL030 C012..C240 -> real classes
        "__ZN4C012C1Ev": "__ZN5StoreC1Ev",
        "__ZN4C012D1Ev": "__ZN5StoreD1Ev",
        "__ZN4C0125checkEv": "__ZN5Store7is_openEv",
        "__ZN4C0123runEv": "__ZN5Store3runEv",
        "__ZN4C031C1Ev": "__ZN2EvC1Ev",
        "__ZN4C031D1Ev": "__ZN2EvD1Ev",
        "__ZN4C0319f184_0479Ev": "__ZN2Ev12show_pic_197Ev",
        "__ZN4C044C1Ev": "__ZN6TempleC1Ev",
        "__ZN4C044D1Ev": "__ZN6TempleD1Ev",
        "__ZN4C0445checkEv": "_temple_is_open",
        "__ZN4C0443runEv": "_temple_run",
        "__ZN4C070C1Ev": "__ZN7LodgingC1Ev",
        "__ZN4C070D1Ev": "__ZN7LodgingD1Ev",
        "__ZN4C0705checkEv": "__ZN7Lodging7is_openEv",
        "__ZN4C0703runEv": "__ZN7Lodging3runEv",
        "__ZN4C071C1Ev": "__ZN4CampC1Ev",
        "__ZN4C071D1Ev": "__ZN4CampD1Ev",
        "__ZN4C0713runEa": "__ZN4Camp3runEa",
        "__ZN4C166C1Ev": "__ZN5SitesC1Ev",
        "__ZN4C166D1Ev": "__ZN5SitesD1Ev",
        "__ZN4C1665f0d3bEv": "__ZN5Sites9challengeEv",
        "__ZN4C1665f1313Ev": "__ZN5Sites7rematchEv",
        "__ZN4C1665f1687Ev": "__ZN5Sites13evening_fightEv",
        "__ZN4C1665f1a37Ev": "__ZN5Sites7exploreEv",
        "__ZN4C173C1Ev": "__ZN5HouseC1Ev",
        "__ZN4C173D1Ev": "__ZN5HouseD1Ev",
        "__ZN4C1733runEv": "__ZN5House3runEv",
        "__ZN4C174C1Ev": "__ZN6HealerC1Ev",
        "__ZN4C174D1Ev": "__ZN6HealerD1Ev",
        "__ZN4C1745checkEv": "__ZN6Healer7is_openEv",
        "__ZN4C1743runEv": "__ZN6Healer3runEv",
        "__ZN4C178C1Ev": "__ZN3InnC1Ev",
        "__ZN4C178D1Ev": "__ZN3InnD1Ev",
        "__ZN4C1785checkEv": "__ZN3Inn7is_openEv",
        "__ZN4C1783runEv": "__ZN3Inn3runEv",
        "__ZN4C226C1Ev": "__ZN4ShopC1Ev",
        "__ZN4C226D1Ev": "__ZN4ShopD1Ev",
        "__ZN4C2265checkEv": "__ZN4Shop7is_openEv",
        "__ZN4C2263runEv": "__ZN4Shop3runEv",
        "__ZN4C230C1Ev": "__ZN5SmithC1Ev",
        "__ZN4C230D1Ev": "__ZN5SmithD1Ev",
        "__ZN4C2305checkEv": "__ZN5Smith10check_openEv",
        "__ZN4C2303runEv": "__ZN5Smith3runEv",
        "__ZN4C240C1Ev": "__ZN6TavernC1Ev",
        "__ZN4C240D1Ev": "__ZN6TavernD1Ev",
        "__ZN4C2405checkEv": "__ZN6Tavern5admitEv",
        "__ZN4C2403runEv": "__ZN6Tavern3runEv",
    }
    cpp_to_c.update(ev_map)

    for sym in sorted(undef):
        if not sym.startswith("__Z") or sym in defined or sym in cpp_to_c:
            continue
        if sym.startswith("__ZN3Inv"):
            target = "__ZN6InvObj" + sym[len("__ZN3Inv"):]
            if target in defined:
                cpp_to_c[sym] = target
        elif sym.startswith("__ZN4Cast"):
            m = re.match(r"^__ZN4Cast(\d+)([A-Za-z0-9_]+)E", sym)
            if m:
                meth = m.group(2)[:int(m.group(1))]
                if "_" + meth in defined:
                    cpp_to_c[sym] = "_" + meth

    lines = []
    lines.append("/* Auto-generiert von Port/tools/generate_bridges.py */")
    lines.append("#include <stdlib.h>")
    lines.append("#include <string.h>")
    lines.append("#include <stdint.h>")
    lines.append("")
    lines.append('extern "C" void *mem_alloc(unsigned long size);')
    lines.append('extern "C" void *mem_free(void *p);')
    lines.append("")
    lines.append("void *operator new(size_t sz) { return mem_alloc((unsigned long)(sz ? sz : 1)); }")
    lines.append("void *operator new[](size_t sz) { return mem_alloc((unsigned long)(sz ? sz : 1)); }")
    lines.append("void operator delete(void *p) noexcept { if (p) mem_free(p); }")
    lines.append("void operator delete[](void *p) noexcept { if (p) mem_free(p); }")
    lines.append("void operator delete(void *p, size_t) noexcept { if (p) mem_free(p); }")
    lines.append("void operator delete[](void *p, size_t) noexcept { if (p) mem_free(p); }")
    lines.append("")
    lines.append('extern "C" void *_vector_new_(void *ptr, size_t elem_size, unsigned int count, unsigned int mode, void (*ctor)(void *)) {')
    lines.append("    (void)mode;")
    lines.append("    if (!ptr) ptr = mem_alloc((unsigned long)(count * elem_size));")
    lines.append("    if (ctor && ptr) {")
    lines.append("        for (unsigned int i = 0; i < count; ++i) {")
    lines.append("            ctor((char *)ptr + i * elem_size);")
    lines.append("        }")
    lines.append("    }")
    lines.append("    return ptr;")
    lines.append("}")
    lines.append("")
    lines.append('extern "C" void gb_delete(void *self, short flags);')
    lines.append('extern "C" void nvf_delete(void *self, short flags);')
    lines.append('extern "C" void _wrap_Bmp_dtor(void *self) { gb_delete(self, 2); }')
    lines.append('extern "C" void _wrap_Nvf_dtor(void *self) { nvf_delete(self, 2); }')
    lines.append('extern "C" void *dsa_mk_fp(unsigned short seg, unsigned long off);')
    nvf_load8_asm = _strip_coff_underscore("__ZN3Nvf4loadEsPvS0_saPsS1_s", is_elf)
    lines.append(f'extern "C" signed long Nvf_load8_call(void *self, short id, void *dst, void *src, short nr, signed char type, short *w, short *h, short stride) __asm__("{nvf_load8_asm}");')
    lines.append('extern "C" void _wrap_Nvf_load9(void *self, short id, void *dst, short src_lo, short src_hi, short nr, short type, short *w, short *h, short stride) {')
    lines.append('    void *src = (src_lo == 0 && src_hi == 0) ? 0 : dsa_mk_fp((unsigned short)src_hi, (unsigned short)src_lo);')
    lines.append('    Nvf_load8_call(self, id, dst, src, nr, (signed char)type, w, h, stride);')
    lines.append('}')
    lines.append("")

    lines.append("/* C++ -> C/C++ Jump-Stubs */")
    for cpp_sym, target in sorted(cpp_to_c.items()):
        cpp_out = _strip_coff_underscore(cpp_sym, is_elf)
        tgt_out = _strip_coff_underscore(target, is_elf)
        lines.append(f'__asm__(".text\\n\\t.p2align 2\\n\\t.globl {cpp_out}\\n{cpp_out}:\\n\\tjmp {tgt_out}");')
    lines.append("")

    lines.append("/* C -> C++ Konstruktor-, Destruktor- und Methoden-Bruecken */")
    for c_sym, cpp_sym in sorted(c_to_cpp.items()):
        if c_sym in defined or c_sym not in undef:
            continue
        c_name = c_sym[1:]
        c_out = _strip_coff_underscore(c_sym, is_elf)
        cpp_out = _strip_coff_underscore(cpp_sym, is_elf)
        if "_ctor" in c_name:
            lines.append(f'extern "C" void {cpp_sym}_call(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) __asm__("{cpp_out}");')
            lines.append(f'extern "C" void *{c_name}(void *self, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {{')
            lines.append(f'    if (!self) self = mem_alloc(4192);')
            lines.append(f'    {cpp_sym}_call(self, a1, a2, a3, a4, a5, a6, a7, a8);')
            lines.append(f'    return self;')
            lines.append(f'}}')
        elif "_dtor" in c_name:
            lines.append(f'extern "C" void {cpp_sym}_call(void *self) __asm__("{cpp_out}");')
            lines.append(f'extern "C" void {c_name}(void *self, short flags) {{')
            lines.append(f'    if (!self) return;')
            lines.append(f'    {cpp_sym}_call(self);')
            lines.append(f'    if (flags & 1) mem_free(self);')
            lines.append(f'}}')
        else:
            lines.append(f'__asm__(".text\\n\\t.p2align 2\\n\\t.globl {c_out}\\n{c_out}:\\n\\tjmp {cpp_out}");')

    os.makedirs(os.path.dirname(out_cpp), exist_ok=True)
    new_content = "\n".join(lines) + "\n"
    old_content = ""
    if os.path.exists(out_cpp):
        with open(out_cpp, "r", encoding="utf-8", errors="ignore") as f:
            old_content = f.read()
    if new_content != old_content:
        with open(out_cpp, "w", encoding="utf-8") as f:
            f.write(new_content)
        print("Wrote", out_cpp, "with", len(c_to_cpp), "C->CPP mappings and", len(cpp_to_c), "CPP->C mappings")
    else:
        print("Unchanged", out_cpp, "with", len(c_to_cpp), "C->CPP mappings and", len(cpp_to_c), "CPP->C mappings")


if __name__ == "__main__":
    main()
