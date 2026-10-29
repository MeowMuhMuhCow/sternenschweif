#!/usr/bin/env python3
"""bauen.py - baut SCHWEIF.EXE (DSA 2 Sternenschweif, Attic 1994) aus dem rekonstruierten Quelltext.

Portierung von tools/build.sh der Cloud-Werkbank, laeuft unter Windows und Linux, nur Standardbibliothek,
Standard-DOSBox 0.74 (kein gepatchtes DOSBox noetig):
  - das feste Linkdatum (15.02.1995, steht in der EXE) setzt C:\\DATUM.COM per INT 21h/2Bh zu Beginn jedes Laufs
  - jeder DOSBox-Lauf bekommt eine erzeugte .conf und laeuft mit -noconsole -exit, Ausgabe per Umleitung in DOSOUT.TXT

Ablauf: Bauordner leeren -> baustand.tar.gz entpacken -> Werkzeuge, Compiler, Original hineinkopieren ->
C-Quellen aus Quelle\\ (Q004) uebernehmen -> Stufe 1 (alles Assembler, Link) -> Stufe 2 (C-Module, Relink) ->
Vergleich mit Original\\SCHWEIF.EXE -> Ergebnis nach Build\\SCHWEIF.EXE.

Fix-Modus (python bauen.py wichtig | alles): nach dem Standardbau zusaetzlich die Fehlerbehebungen einschalten
(names/inc/s_fix.h: FIX_BUGS=1 "wichtig" bzw. 2 "alles", Portierung von tools/build_fix.sh) -> Build\\SCHWEIF_FIX_WICHTIG.EXE
bzw. Build\\SCHWEIF_FIX_ALLES.EXE. Der Standardbau (Build\\SCHWEIF.EXE) laeuft immer mit und dient als Vergleichsfassung.

Aufruf:  python bauen.py [wichtig|alles] [--q004 ORDNER] [--dosbox DOSBox.exe] [--baudir ORDNER] [--nur-asm] [--sichtbar]
Alle Pfade auch per Umgebung: SW_Q004, SW_DOSBOX, SW_BAUDIR."""
import os, sys, re, glob, json, shutil, stat, tarfile, hashlib, subprocess, time, argparse

HIER = os.path.dirname(os.path.abspath(__file__))
WINDOWS = os.name == 'nt'
ORIGINAL_MD5 = 'da3b59ce86a82e4b8bb14d36ed37d4d0'
DATUM = (1995, 2, 15)          # Linkdatum, das BC.EXE in die EXE schreibt
# DATUM.COM: mov ah,2Bh / mov cx,1995 / mov dx,020Fh / int 21h / mov ax,4C00h / int 21h
DATUM_COM = bytes([0xB4, 0x2B, 0xB9, DATUM[0] & 255, DATUM[0] >> 8, 0xBA, DATUM[2], DATUM[1], 0xCD, 0x21,
                   0xB8, 0x00, 0x4C, 0xCD, 0x21])
GOG_DOSBOX = [r'C:\Program Files (x86)\GOG Galaxy\Games\Realms of Arkania 2\DOSBOX\DOSBox.exe',
              r'C:\Program Files\GOG Galaxy\Games\Realms of Arkania 2\DOSBOX\DOSBox.exe',
              r'C:\GOG Games\Realms of Arkania 2\DOSBOX\DOSBox.exe',
              r'C:\Program Files (x86)\DOSBox-0.74-3\DOSBox.exe', r'C:\Program Files (x86)\DOSBox-0.74-2\DOSBox.exe',
              r'C:\Program Files (x86)\DOSBox-0.74\DOSBox.exe']

LOG = None
FEHLER_GEFUNDEN = []


def say(*a):
    t = ' '.join(str(x) for x in a)
    print(t, flush=True)
    if LOG: LOG.write(t + '\n'); LOG.flush()


def abbruch(msg):
    say(''); say('ABBRUCH: ' + msg)
    raise SystemExit(2)


def md5(p):
    return hashlib.md5(open(p, 'rb').read()).hexdigest()


def crlf(lines):
    """DOS-Zeilen (BAT): jede Zeile genau mit \\r\\n"""
    return ''.join(l.replace('\r', '').replace('\n', '') + '\r\n' for l in lines)


def _rw(func, path, _):
    os.chmod(path, stat.S_IWRITE | stat.S_IREAD); func(path)


def rmtree(p):
    if os.path.isdir(p):
        if sys.version_info >= (3, 12): shutil.rmtree(p, onexc=lambda f, q, e: _rw(f, q, e))
        else: shutil.rmtree(p, onerror=_rw)


def copyfile(a, b):
    """Inhalt kopieren, Ziel immer beschreibbar (Compiler-Dateien sind schreibgeschuetzt)"""
    if os.path.exists(b): os.chmod(b, stat.S_IWRITE | stat.S_IREAD)
    shutil.copyfile(a, b)


def copytree_plain(a, b):
    for d, _, fs in os.walk(a):
        z = os.path.join(b, os.path.relpath(d, a)); os.makedirs(z, exist_ok=True)
        for f in fs: copyfile(os.path.join(d, f), os.path.join(z, f))


def find_ci(d, name):
    """Datei in d ohne Ruecksicht auf Gross/Klein (Linux-Test, Windows egal)"""
    p = os.path.join(d, name)
    if os.path.exists(p): return p
    if os.path.isdir(d):
        for f in os.listdir(d):
            if f.upper() == name.upper(): return os.path.join(d, f)
    return None


# ------------------------------------------------------------------ Umgebung finden
def finde_q004(arg):
    if arg: return os.path.abspath(arg)
    d = HIER
    for _ in range(4):
        if os.path.isdir(os.path.join(d, 'Compiler')) and os.path.isdir(os.path.join(d, 'Original')): return d
        d = os.path.dirname(d)
    abbruch('Projektordner (mit Compiler\\ und Original\\) nicht gefunden. Das Paket gehoert nach '
            'Q 004 DSA Sternenschweif\\Bauen\\ - oder --q004 ORDNER angeben.')


def finde_dosbox(arg):
    kand = [arg] if arg else []
    kand += [os.path.join(HIER, 'DOSBox', 'DOSBox.exe'), os.path.join(HIER, 'DOSBox.exe')]
    if WINDOWS: kand += GOG_DOSBOX
    else: kand += [shutil.which('dosbox') or '']
    for k in kand:
        if k and os.path.isfile(k): return os.path.abspath(k)
    abbruch('DOSBox nicht gefunden (GOG-Ordner Realms of Arkania 2\\DOSBOX\\DOSBox.exe oder DOSBox\\DOSBox.exe '
            'neben BAUEN.bat) - oder --dosbox PFAD angeben.')


def waehle_baudir(arg):
    if arg: return os.path.abspath(arg)
    import tempfile
    d = os.path.join(tempfile.gettempdir(), 'sternenschweif_bau')
    if not d.isascii():          # DOSBox 0.74 vertraegt keine Umlaute im mount-Pfad sicher
        d = os.path.join(os.environ.get('SystemDrive', 'C:') + '\\', 'sternenschweif_bau') if WINDOWS else '/tmp/sternenschweif_bau'
    return d


# ------------------------------------------------------------------ DOSBox
class Dos:
    def __init__(self, exe, root, sichtbar):
        self.exe, self.root = exe, root
        self.run_dir = os.path.join(root, '_dosbox'); os.makedirs(self.run_dir, exist_ok=True)
        self.env = dict(os.environ)
        self.dummy = not sichtbar
        self.laeufe = 0; self.zeit = 0.0

    def conf(self, wd, cmd):
        mount = self.root
        return '\n'.join([
            '[sdl]', 'output=surface', 'fullscreen=false', 'autolock=false',
            '[dosbox]', 'machine=svga_s3', 'memsize=16',
            '[cpu]', 'core=normal', 'cputype=auto', 'cycles=max',
            '[mixer]', 'nosound=true', '[speaker]', 'pcspeaker=false', 'tandy=off', 'disney=false',
            '[sblaster]', 'sbtype=none', '[gus]', 'gus=false', '[midi]', 'mpu401=none', 'mididevice=none',
            '[dos]', 'xms=true', 'ems=true', 'umb=true', 'keyboardlayout=none',
            '[autoexec]',
            f'mount c "{mount}"', 'c:', 'C:\\DATUM.COM', f'cd \\{wd}', f'{cmd} > DOSOUT.TXT', 'exit', ''])
        # Hinweis: eine BAT, aus dem autoexec ohne CALL gestartet, kehrt nicht zurueck - jede BAT endet daher mit exit

    def run(self, wd, cmd, timeout, name=None, weich=False):
        """fuehrt cmd (meist eine BAT) im DOS-Ordner wd (relativ zur Bauwurzel) aus, liefert DOSOUT.TXT"""
        cp = os.path.join(self.run_dir, 'lauf.conf')
        txt = self.conf(wd, cmd)
        open(cp, 'w', encoding='mbcs' if WINDOWS else 'utf-8', newline='\r\n' if WINDOWS else '\n').write(txt)
        out = os.path.join(self.root, wd, 'DOSOUT.TXT')
        if os.path.exists(out): os.remove(out)
        env = dict(self.env)
        if self.dummy: env['SDL_VIDEODRIVER'] = 'dummy'
        env.setdefault('SDL_AUDIODRIVER', 'dummy')
        t0 = time.time()
        try:
            subprocess.run([self.exe, '-conf', cp, '-noconsole', '-exit'], cwd=self.run_dir, env=env,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=timeout)
        except subprocess.TimeoutExpired:
            if weich: return None
            abbruch(f'DOSBox-Lauf {name or cmd} nach {timeout} s nicht fertig (abgebrochen).')
        self.laeufe += 1; self.zeit += time.time() - t0
        if not os.path.exists(out): return None
        return open(out, 'rb').read().decode('cp437', 'replace').replace('\r', '')

    def probe(self):
        """prueft, ob DOSBox laeuft (und ob ohne Fenster - SDL-Dummytreiber - moeglich ist)"""
        # direkter Befehl statt BAT: die Umleitung "X.BAT > DATEI" erfasst in DOSBox die Zeilen der BAT nicht
        if self.dummy:
            r = self.run('', 'echo DOSBOX-OK', 60, 'Probe', weich=True)
            if r and 'DOSBOX-OK' in r: return 'ohne Fenster'
            self.dummy = False
        r = self.run('', 'echo DOSBOX-OK', 60, 'Probe', weich=True)
        if r and 'DOSBOX-OK' in r: return 'mit Fenster'
        abbruch('DOSBox startet, fuehrt aber keine Befehle aus (Probe-Lauf ohne Ergebnis). DOSBox-Pfad pruefen.')


# ------------------------------------------------------------------ Werkzeuge
class Tools:
    def __init__(self, root):
        self.root = root
        self.env = dict(os.environ); self.env['SW_ROOT'] = root

    def run(self, name, *args, stdout=None, check=True, env=None):
        cmd = [sys.executable, '-I', '-X', 'utf8', os.path.join(self.root, 'tools', name + '.py')] + [str(a) for a in args]
        e = dict(self.env)
        if env: e.update(env)
        r = subprocess.run(cmd, cwd=self.root, env=e, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        txt = r.stdout.decode('utf-8', 'replace').replace('\r', '')
        if stdout: open(stdout, 'w', encoding='utf-8').write(txt)
        if check and r.returncode != 0:
            say(txt[-3000:]); abbruch(f'{name} {" ".join(str(a) for a in args)} fehlgeschlagen (Rueckgabe {r.returncode})')
        return r.returncode, txt


def nm(n):
    return 'OVL' + n[1:] if n.startswith('O') else 'SEG' + n


def c_module(root):
    mods = []
    for l in open(os.path.join(root, 'c_module.tsv'), encoding='utf-8').read().split('\n'):
        l = l.rstrip('\r')
        if not l or l.startswith('#'): continue
        f = l.split('\t')
        mods.append((f[0], f[1] if len(f) > 1 else '', f[2] if len(f) > 2 else ''))
    return mods


# ------------------------------------------------------------------ Vorbereitung
def vorbereiten(q, B, paket):
    rmtree(B)
    if os.path.exists(B): abbruch(f'Bauordner {B} laesst sich nicht leeren (Datei offen?).')
    os.makedirs(B)
    say(f'  Baustand entpacken ({os.path.basename(paket)})')
    with tarfile.open(paket, 'r:gz') as t:
        if hasattr(tarfile, 'data_filter'): t.extractall(B, filter='data')
        else: t.extractall(B)
    for d in ('tools', 'dsana', 'q004/zerl', 'lnk', 'orig', 'bc'): os.makedirs(os.path.join(B, d), exist_ok=True)
    wz = os.path.join(HIER, 'werkzeuge')
    for f in os.listdir(wz):
        if not f.endswith('.py'): continue
        ziel = {'items.py': 'dsana', 'schweif.py': 'q004/zerl'}.get(f, 'tools')
        copyfile(os.path.join(wz, f), os.path.join(B, ziel, f))
    # Compiler: BIN, INCLUDE, LIB aus Q004\Compiler, Konfiguration auf Laufwerk C: (= Bauordner) umbiegen
    comp = os.path.join(q, 'Compiler')
    for sub in ('BIN', 'INCLUDE', 'LIB'):
        s = find_ci(comp, sub)
        if not s: abbruch(f'Compiler\\{sub} fehlt in {comp}')
        copytree_plain(s, os.path.join(B, 'bc', sub))
    for exe in ('BC.EXE', 'BCC.EXE', 'TASM.EXE', 'TLINK.EXE', 'DPMI16BI.OVL', 'DPMILOAD.EXE', 'DPMIMEM.DLL'):
        if not find_ci(os.path.join(B, 'bc', 'BIN'), exe): abbruch(f'Compiler\\BIN\\{exe} fehlt')
    for cfg, txt in (('TURBOC.CFG', ['-IC:\\BC\\INCLUDE', '-LC:\\BC\\LIB']), ('TLINK.CFG', ['-LC:\\BC\\LIB'])):
        p = find_ci(os.path.join(B, 'bc', 'BIN'), cfg) or os.path.join(B, 'bc', 'BIN', cfg)
        if os.path.exists(p): os.chmod(p, stat.S_IWRITE | stat.S_IREAD)
        open(p, 'w', newline='').write(crlf(txt))
    o = find_ci(os.path.join(q, 'Original'), 'SCHWEIF.EXE')
    if not o: abbruch('Original\\SCHWEIF.EXE fehlt')
    if md5(o) != ORIGINAL_MD5: abbruch(f'Original\\SCHWEIF.EXE ist nicht die erwartete Fassung (MD5 {md5(o)})')
    copyfile(o, os.path.join(B, 'orig', 'SCHWEIF.EXE'))
    open(os.path.join(B, 'DATUM.COM'), 'wb').write(DATUM_COM)


def quelle_uebernehmen(q, B, nur_paket):
    """C-Quellen aus Q004\\Quelle (SEG*.C/CPP, Overlays\\OVL*.C/CPP, inc\\) statt des Referenzstands im Paket"""
    csrc = os.path.join(B, 'csrc'); inc = os.path.join(B, 'names', 'inc')
    qd = os.path.join(q, 'Quelle')
    if nur_paket or not os.path.isdir(qd):
        say('  C-Quellen: Referenzstand aus dem Paket' + ('' if nur_paket else ' (Quelle\\ nicht gefunden)'))
        return
    ref = {f.upper(): open(os.path.join(csrc, f), 'rb').read() for f in os.listdir(csrc)}
    neu = {}
    for d in (qd, find_ci(qd, 'Overlays') or ''):
        if not d or not os.path.isdir(d): continue
        for f in os.listdir(d):
            u = f.upper()
            if re.match(r'^(SEG|OVL)\d{3}\.(C|CPP)$', u): neu[u] = open(os.path.join(d, f), 'rb').read()
    mods = {nm(n) for n, _, _ in c_module(B)}
    hat = {k.split('.')[0] for k in neu}
    fehlt = sorted(mods - hat)
    if len(hat & mods) == 0:
        say('  C-Quellen: in Quelle\\ keine Module gefunden - Referenzstand aus dem Paket'); return
    for f in os.listdir(csrc): os.remove(os.path.join(csrc, f))
    for k, v in neu.items(): open(os.path.join(csrc, k), 'wb').write(v)
    for m in fehlt:              # Luecken mit Referenzstand fuellen
        for ext in ('.CPP', '.C'):
            if m + ext in ref: open(os.path.join(csrc, m + ext), 'wb').write(ref[m + ext]); break
    anders = sorted(k for k in neu if ref.get(k) != neu[k])
    say(f'  C-Quellen: {len(neu)} Dateien aus Quelle\\' + (f', {len(fehlt)} aus dem Paket ergaenzt ({", ".join(fehlt[:8])})' if fehlt else ''))
    if anders: say(f'  GEAENDERT gegenueber Referenzstand: {len(anders)} Datei(en): {", ".join(anders[:12])}{" ..." if len(anders) > 12 else ""}')
    qi = find_ci(qd, 'inc')
    if qi and os.path.isdir(qi):
        geaendert = []
        for f in os.listdir(qi):
            if f.endswith('.orig') or not os.path.isfile(os.path.join(qi, f)): continue
            z = find_ci(inc, f) or os.path.join(inc, f)
            if not os.path.exists(z) or open(z, 'rb').read() != open(os.path.join(qi, f), 'rb').read(): geaendert.append(f)
            copyfile(os.path.join(qi, f), z)
        say('  Kopfdateien: aus Quelle\\inc' + (f', GEAENDERT: {", ".join(geaendert)}' if geaendert else ''))


# ------------------------------------------------------------------ Bau (Portierung von tools/build.sh)
def stufe1(B, dos, T):
    L = os.path.join(B, 'lnk')
    say('[1/9] Overlays: Einspruenge exportieren, Aliase, F_O032_0000 -> _main')
    ovl = sorted(glob.glob(os.path.join(B, 'o[0-9][0-9][0-9]', 'OVL*.ASM')))
    T.run('ovlpub', *[os.path.relpath(p, B).replace('\\', '/') for p in ovl])
    T.run('aliases')
    pat = re.compile(rb'\bF_O032_0000\b')
    for p in sorted(glob.glob(os.path.join(B, 'l[0-9][0-9]', 'SEG0*.ASM')) + ovl + glob.glob(os.path.join(B, 'dsg', 'DS*.ASM'))):
        d = open(p, 'rb').read()
        if pat.search(d): open(p, 'wb').write(pat.sub(b'_main', d))
    for n in range(1, 47): copyfile(os.path.join(B, f'l{n:02d}', f'SEG{n:03d}.ASM'), os.path.join(L, f'SEG{n:03d}.ASM'))
    for p in ovl: copyfile(p, os.path.join(L, os.path.basename(p)))
    say('[2/9] Datenstuecke erzeugen (dmgen)')
    rc, txt = T.run('dmgen', stdout=os.path.join(L, 'dmgen.txt'), check=False)
    if rc != 0: say(txt[-3000:]); abbruch('dmgen fehlgeschlagen')
    say('  ' + txt.strip().split('\n')[-1][:300])
    dm = [x for x in open(os.path.join(L, 'dm_liste.txt'), encoding='utf-8').read().split() if x]
    asm = sorted(f for f in os.listdir(L) if re.match(r'^(SEG0|OVL).*\.ASM$', f))
    zeilen = [f'..\\bc\\BIN\\TASM.EXE /ml {f} > NUL' for f in asm]
    zeilen += [f'..\\bc\\BIN\\TASM.EXE /ml /m2 {f}.ASM > NUL' for f in dm]
    zeilen += ['..\\bc\\BIN\\TASM.EXE /ml INIT.ASM > NUL', '..\\bc\\BIN\\TASM.EXE /ml FARDATA.ASM > NUL', 'exit']
    open(os.path.join(L, 'ASMALL.BAT'), 'w', newline='').write(crlf(zeilen))
    for f in glob.glob(os.path.join(L, '*.OBJ')): os.remove(f)
    say(f'[3/9] DOSBox: {len(zeilen) - 1} Dateien assemblieren (TASM) - dauert einige Minuten')
    dos.run('lnk', 'ASMALL.BAT', 3000, 'ASMALL')
    objs = glob.glob(os.path.join(L, '*.OBJ'))
    say(f'  OBJ: {len(objs)}')
    for f in dm:
        if not os.path.exists(os.path.join(L, f + '.OBJ')): abbruch(f'{f}.ASM nicht assembliert')
    for f in asm:
        if not os.path.exists(os.path.join(L, f[:-4] + '.OBJ')): abbruch(f'{f} nicht assembliert')
    say('[4/9] OBJ nachbearbeiten (ovlobjfix, objchunk, objorder)')
    T.run('ovlobjfix')
    warn = []
    for n in range(0, 271):
        p = os.path.join(L, f'OVL{n:03d}.OBJ')
        if not os.path.exists(p): continue
        rc, txt = T.run('objchunk', p, f'O{n:03d}', check=False)
        if rc: warn.append(f'objchunk O{n:03d}: ' + txt.strip()[-200:])
    for f in dm:
        rc, txt = T.run('objorder', os.path.join(L, f + '.OBJ'), os.path.join(L, 'dm.json'), f, check=False)
        if rc: warn.append(f'objorder {f}: ' + txt.strip()[-200:])
    for n in range(1, 47):
        rc, txt = T.run('objorder', os.path.join(L, f'SEG{n:03d}.OBJ'), '-', f'R{n:02d}', check=False)
        if rc: warn.append(f'objorder R{n:02d}: ' + txt.strip()[-200:])
    for w in warn: say('  WARNUNG ' + w)
    say('[5/9] Stufe 1 linken (BC.EXE /b, Datum 15.02.1995)')
    T.run('mkprj', 'FULL.RES')
    open(os.path.join(L, 'LINK.BAT'), 'w', newline='').write(crlf(['..\\bc\\BIN\\BC.EXE /b SCHW_CD.PRJ', 'exit']))
    exe = os.path.join(L, 'SCHW_CD.EXE')
    if os.path.exists(exe): os.remove(exe)
    out = dos.run('lnk', 'LINK.BAT', 1200, 'LINK') or ''
    meld = [l for l in out.split('\n') if l.strip() and not l.startswith('Linking')]
    open(os.path.join(L, 'full_out.txt'), 'w', encoding='utf-8').write('\n'.join(meld) + '\n')
    if not os.path.exists(exe): abbruch('Stufe 1: SCHW_CD.EXE wurde nicht erzeugt')
    gleich = open(exe, 'rb').read() == open(os.path.join(B, 'orig', 'SCHWEIF.EXE'), 'rb').read()
    say('  Stufe 1 (alles Assembler): ' + ('BYTEGLEICH' if gleich else 'ABWEICHUNG'))
    a1 = os.path.join(L, 'asm1'); os.makedirs(a1, exist_ok=True)
    for f in glob.glob(os.path.join(L, '*.OBJ')) + [os.path.join(L, 'SCHW_CD.MAP')]: copyfile(f, os.path.join(a1, os.path.basename(f)))
    return gleich


def stufe2(B, dos, T):
    L = os.path.join(B, 'lnk'); C = os.path.join(B, 'cbau')
    mods = c_module(B)
    say(f'[6/9] Stufe 2: {len(mods)} C-Module uebersetzen (BCC 3.1)')
    rmtree(C); os.makedirs(C)
    for f in os.listdir(os.path.join(B, 'csrc')): copyfile(os.path.join(B, 'csrc', f), os.path.join(C, f))
    cbat = []
    for n, o, d in mods:
        m = nm(n)
        f = m + '.CPP' if os.path.exists(os.path.join(C, m + '.CPP')) else m + '.C'
        if not os.path.exists(os.path.join(C, f)): abbruch(f'Quelle fuer {m} fehlt (.C/.CPP)')
        cbat.append(f'..\\bc\\BIN\\BCC.EXE -mlarge {o} -c -Y -I..\\names\\inc -E..\\bc\\BIN\\TASM.EXE {f} > {m}.TXT')
    # frisch.txt: Module, die einen eigenen frischen DOSBox-Lauf brauchen (BCC liest nicht initialisierten Speicher)
    frisch = []
    fp = os.path.join(B, 'tools', 'frisch.txt')
    if os.path.exists(fp):
        for l in open(fp, encoding='utf-8').read().replace('\r', '').split('\n'):
            t = l.split()
            if t: frisch.append((t[0], t[1:]))

    def zeile(mod):
        for z in cbat:
            if re.search(r' ' + re.escape(mod) + r'\.C', z, re.I): return z
        return None
    if any(v.startswith('@') for _, vor in frisch for v in vor):
        copyfile(os.path.join(B, 'tools', 'fill', 'FILL.ASM'), os.path.join(C, 'FILL.ASM'))
        open(os.path.join(C, 'FILL.BAT'), 'w', newline='').write(crlf(
            ['..\\bc\\BIN\\TASM.EXE FILL.ASM > FILL.TXT', '..\\bc\\BIN\\TLINK.EXE /t FILL.OBJ > FILLL.TXT', 'exit']))
        dos.run('cbau', 'FILL.BAT', 120, 'FILL')
        if not os.path.exists(os.path.join(C, 'FILL.COM')) or os.path.getsize(os.path.join(C, 'FILL.COM')) == 0:
            abbruch('FILL.COM fehlt')
    for fm, vor in frisch:
        b = []
        for v in vor:
            if v.startswith('@'):
                _, txt = T.run('heapbild', v, os.path.join(C, 'LOAD.BIN'))
                b += [x for x in txt.replace('\r', '').split('\n') if x.strip()]
            else:
                z = zeile(v)
                if z: b.append(z)
        z = zeile(fm)
        if z: b.append(z)
        b.append('exit')
        open(os.path.join(C, f'F_{fm}.BAT'), 'w', newline='').write(crlf(b))
        cbat = [x for x in cbat if not re.search(r' ' + re.escape(fm) + r'\.C', x, re.I)]
        say(f'  frischer Lauf fuer {fm}' + (f' (Vorlauf {" ".join(vor)})' if vor else ''))
        dos.run('cbau', f'F_{fm}.BAT', 300, f'F_{fm}')
    bloecke = [cbat[i:i + 40] for i in range(0, len(cbat), 40)]
    for i, blk in enumerate(bloecke):
        bn = f'CB{i:02d}'
        open(os.path.join(C, bn + '.BAT'), 'w', newline='').write(crlf(blk + ['exit']))
        say(f'  Block {i + 1}/{len(bloecke)}: {len(blk)} Module')
        dos.run('cbau', bn + '.BAT', 600, bn)
    fehler = []
    for f in sorted(glob.glob(os.path.join(C, '*.TXT'))):
        for l in open(f, 'rb').read().decode('cp437', 'replace').replace('\r', '').split('\n'):
            if re.match(r'^error', l, re.I) or re.search('fatal', l, re.I): fehler.append(l)
    for l in fehler: say('  ' + l)
    say('[7/9] C-OBJ an die Original-Ordnung anpassen (cobjfix_d, dscheck)')
    mapf = os.path.join(L, 'SCHW_CD.MAP')
    for n, o, d in mods:
        m = nm(n)
        cobj, aobj, fix = os.path.join(C, m + '.OBJ'), os.path.join(L, m + '.OBJ'), os.path.join(C, m + '.FIX')
        if not os.path.exists(cobj): abbruch(f'{m}.OBJ fehlt - BCC-Fehler? Siehe {os.path.join(C, m + ".TXT")}')
        if d.startswith('D'):
            rc, txt = T.run('cobjfix_d', '--eigen', cobj, aobj, mapf, fix, check=False)
            if rc: say(txt[-2000:]); abbruch(f'cobjfix --eigen {m} fehlgeschlagen')
            rc2, txt2 = T.run('dscheck', fix, mapf, check=False)
            open(os.path.join(C, m + '.D.TXT'), 'w', encoding='utf-8').write(txt + txt2)
            if rc2: say(txt2[-2000:]); abbruch(f'dscheck {m} fehlgeschlagen')
        else:
            rc, txt = T.run('cobjfix_d', cobj, aobj, mapf, fix, check=False)
            if rc: say(txt[-2000:]); abbruch(f'cobjfix {m} fehlgeschlagen')
    _, txt = T.run('cvsasm', *[n for n, _, _ in mods], env={'CDIR': C}, check=False)
    verg = ['  ' + l for l in txt.split('\n') if l]
    open(os.path.join(C, 'vergleich.txt'), 'w', encoding='utf-8').write('\n'.join(verg) + '\n')
    nicht = [l for l in verg if 'GLEICH' not in l]
    say(f'  Vergleich C gegen ASM: {len(verg) - len(nicht)} GLEICH, {len(nicht)} andere Zeilen')
    for l in nicht[:30]: say(l)
    ds = []
    for f in glob.glob(os.path.join(C, '*.D.TXT')):
        ds += [l for l in open(f, encoding='utf-8').read().split('\n') if l.startswith('dscheck')]
    if ds: say(f'  Phase B: {len(ds)} Module mit eigenen Daten (dscheck GLEICH)')
    say('[8/9] Stufe 2 linken (Datum 15.02.1995)')
    for n, _, _ in mods: copyfile(os.path.join(C, nm(n) + '.FIX'), os.path.join(L, nm(n) + '.OBJ'))
    T.run('mkprj', 'FULL2.RES')
    rc, txt = T.run('pubdup', os.path.join(L, 'FULL2.RES'), check=False)
    if rc: say(txt); abbruch('doppelte Publics (BC.EXE /b linkt dann stumm nicht)')
    exe = os.path.join(L, 'SCHW_CD.EXE')
    if os.path.exists(exe): os.remove(exe)
    out = dos.run('lnk', 'LINK.BAT', 1200, 'LINK2') or ''
    meld = [l for l in out.split('\n') if l.strip() and not l.startswith('Linking')]
    open(os.path.join(L, 'full_out2.txt'), 'w', encoding='utf-8').write('\n'.join(meld) + '\n')
    say(f'  Stufe 2: {len(mods)} Module aus C gelinkt')
    if not os.path.exists(exe): abbruch('Stufe 2: SCHW_CD.EXE wurde nicht erzeugt')


def stufe_fix(B, dos, T, flags, fn, ausgabe):
    """Fix-Modus (Portierung von tools/build_fix.sh): nur die Quellen mit Schaltern FIX_nnnn neu uebersetzen,
    per cobjfix --ref (Namen aus der Vergleichsfassung cbau) linkfertig machen, residente Module auf Originallaenge
    auffuellen (segpad), Assembler-Module mit IF FIX_nnnn neu assemblieren, in lnkfix linken, fixcheck."""
    L = os.path.join(B, 'lnk'); C = os.path.join(B, 'cbau'); F = os.path.join(B, 'cfix'); LF = os.path.join(B, 'lnkfix')
    rmtree(F); rmtree(LF); os.makedirs(F); os.makedirs(LF)
    mods = c_module(B)
    cn = os.path.join(F, 'cnamen.tsv')
    T.run('cnamen', C, os.path.join(L, 'asm1'), stdout=cn)
    cbat = []; fm = []
    for n, o, d in mods:
        m = nm(n)
        f = m + '.CPP' if os.path.exists(os.path.join(B, 'csrc', m + '.CPP')) else m + '.C'
        if not re.search(rb'FIX_[0-9]', open(os.path.join(B, 'csrc', f), 'rb').read()): continue
        copyfile(os.path.join(B, 'csrc', f), os.path.join(F, f))
        cbat.append(f'..\\bc\\BIN\\BCC.EXE -mlarge {o} {flags} -c -Y -I..\\names\\inc -E..\\bc\\BIN\\TASM.EXE {f} > {m}.TXT')
        fm.append((n, m, d))
    if not fm:
        say('  WARNUNG: keine Quelle enthaelt Schalter FIX_nnnn (alter Stand in Quelle\\?) - die Fix-EXE gleicht dem Original')
    say(f'[10/11] Fix-Modus ({flags}): {len(fm)} C-Module mit Schaltern uebersetzen')
    bloecke = [cbat[i:i + 40] for i in range(0, len(cbat), 40)]
    for i, blk in enumerate(bloecke):
        bn = f'CB{i:02d}'
        open(os.path.join(F, bn + '.BAT'), 'w', newline='').write(crlf(blk + ['exit']))
        say(f'  Block {i + 1}/{len(bloecke)}: {len(blk)} Module')
        dos.run('cfix', bn + '.BAT', 600, 'F' + bn)
    fehler = []
    for f in sorted(glob.glob(os.path.join(F, '*.TXT'))):
        for l in open(f, 'rb').read().decode('cp437', 'replace').replace('\r', '').split('\n'):
            if re.match(r'^error', l, re.I) or re.search('fatal', l, re.I): fehler.append(l)
    for l in fehler: say('  ' + l)
    if fehler: abbruch('Fix-Modus: Uebersetzungsfehler')
    for f in os.listdir(L):
        if os.path.isfile(os.path.join(L, f)): copyfile(os.path.join(L, f), os.path.join(LF, f))
    mapf = os.path.join(L, 'asm1', 'SCHW_CD.MAP')
    env = {'CNAMEN': cn}
    nc = ng = 0
    for n, m, d in fm:
        fobj, ref, fix = os.path.join(F, m + '.OBJ'), os.path.join(C, m + '.OBJ'), os.path.join(F, m + '.FIX')
        if not os.path.exists(fobj): abbruch(f'Fix-Modus: {m}.OBJ fehlt - BCC-Fehler? Siehe {os.path.join(F, m + ".TXT")}')
        rc, _ = T.run('objsame', fobj, ref, check=False)
        if rc == 0: ng += 1; continue
        aobj = os.path.join(L, 'asm1', m + '.OBJ')
        if d.startswith('D'):
            rc, txt = T.run('cobjfix_d', '--eigen', fobj, aobj, mapf, fix, '--ref', ref, check=False, env=env)
            if rc: say(txt[-2000:]); abbruch(f'Fix-Modus: cobjfix --ref {m} fehlgeschlagen')
            rc2, txt2 = T.run('dscheck', fix, mapf, check=False)
            if rc2: say(txt2[-2000:]); abbruch(f'Fix-Modus: dscheck {m} - Datensegment geaendert')
        else:
            rc, txt = T.run('cobjfix_d', fobj, aobj, mapf, fix, '--ref', ref, check=False, env=env)
            if rc: say(txt[-2000:]); abbruch(f'Fix-Modus: cobjfix --ref {m} fehlgeschlagen')
        for l in txt.split('\n'):
            if 'WARNUNG' in l: say('  ' + m + ': ' + l.strip())
        if m.startswith('SEG'):
            rc, txt = T.run('segpad', fix, os.path.join(C, m + '.FIX'), check=False)
            if rc: say(txt); abbruch(f'Fix-Modus: {m} passt nicht in sein residentes Segment')
        copyfile(fix, os.path.join(LF, m + '.OBJ')); nc += 1
    say(f'  {nc} C-Module geaendert, {ng} ohne Wirkung der Schalter')
    am = []
    for p in sorted(glob.glob(os.path.join(B, 'l[0-9][0-9]', 'SEG0*.ASM'))):
        if re.search(rb'(?m)^IF FIX_[0-9]', open(p, 'rb').read()): am.append(os.path.basename(p)[:-4])
    af = flags.replace('-D', '/d')
    open(os.path.join(LF, 'ASMFIX.BAT'), 'w', newline='').write(crlf(
        [f'..\\bc\\BIN\\TASM.EXE /ml {af} {m}.ASM,{m}.FXO > {m}.FXT' for m in am] + ['exit']))
    dos.run('lnkfix', 'ASMFIX.BAT', 600, 'ASMFIX')
    na = 0
    for m in am:
        fxo = os.path.join(LF, m + '.FXO')
        if not os.path.exists(fxo): abbruch(f'Fix-Modus: TASM {m} fehlgeschlagen (siehe {os.path.join(LF, m + ".FXT")})')
        rc, _ = T.run('objsame', fxo, os.path.join(L, 'asm1', m + '.OBJ'), check=False)
        if rc == 0: continue
        rc, txt = T.run('segpad', fxo, os.path.join(L, 'asm1', m + '.OBJ'), check=False)
        if rc: say(txt); abbruch(f'Fix-Modus: {m} passt nicht in sein residentes Segment')
        copyfile(fxo, os.path.join(LF, m + '.OBJ')); na += 1
    say(f'  {na} Assembler-Module geaendert')
    say('[11/11] Fix-EXE linken und pruefen (fixcheck)')
    rc, txt = T.run('pubdup', os.path.join(LF, 'FULL2.RES'), check=False)
    if rc: say(txt); abbruch('Fix-Modus: doppelte Publics')
    exe = os.path.join(LF, 'SCHW_CD.EXE')
    if os.path.exists(exe): os.remove(exe)
    out = dos.run('lnkfix', 'LINK.BAT', 1200, 'LINKFIX') or ''
    meld = [l for l in out.split('\n') if l.strip() and not l.startswith('Linking')]
    for l in meld[:20]: say('  ' + l)
    if not os.path.exists(exe): abbruch('Fix-Modus: keine EXE erzeugt')
    rc, txt = T.run('fixcheck', exe, os.path.join(B, 'orig', 'SCHWEIF.EXE'), check=False)
    for l in txt.strip().split('\n')[-12:]: say('  ' + l)
    if rc: abbruch('Fix-Modus: fixcheck meldet Fehler - die Fix-EXE wird nicht ausgegeben')
    ziel = os.path.join(ausgabe, f'SCHWEIF_FIX_{fn}.EXE')
    if os.path.exists(ziel): os.chmod(ziel, stat.S_IWRITE | stat.S_IREAD)
    shutil.copyfile(exe, ziel)
    say(f'  {md5(ziel)}  Build\\SCHWEIF_FIX_{fn}.EXE ({os.path.getsize(ziel)} Bytes)')
    return ziel


def main():
    global LOG
    ap = argparse.ArgumentParser(description='Baut SCHWEIF.EXE aus dem rekonstruierten Quelltext.')
    ap.add_argument('fix', nargs='?', choices=['wichtig', 'alles'], type=str.lower,
                    help='zusaetzlich die Fix-EXE bauen: wichtig (FIX_BUGS=1) oder alles (FIX_BUGS=2)')
    ap.add_argument('--exploits-lassen', action='store_true',
                    help='Fix-Modus: Fehler, die dem Spieler nuetzen, nicht beheben (FIX_EXPLOITS=0)')
    ap.add_argument('--fixflags', default='', help='Fix-Modus: weitere Schalter fuer BCC/TASM, z. B. "-DFIX_0020=0"')
    ap.add_argument('--q004', default=os.environ.get('SW_Q004'), help='Projektordner Q 004 (mit Compiler, Original, Quelle)')
    ap.add_argument('--dosbox', default=os.environ.get('SW_DOSBOX'), help='Pfad zu DOSBox.exe (0.74)')
    ap.add_argument('--baudir', default=os.environ.get('SW_BAUDIR'), help='Arbeitsordner (wird geleert)')
    ap.add_argument('--ausgabe', default=None, help='Zielordner fuer SCHWEIF.EXE (Standard: Q004\\Build)')
    ap.add_argument('--nur-asm', action='store_true', help='nur Stufe 1 (alles Assembler)')
    ap.add_argument('--nur-paket', action='store_true', help='C-Quellen aus dem Paket statt aus Quelle\\')
    ap.add_argument('--ohne-datum', action='store_true', help=argparse.SUPPRESS)   # Gegenprobe: Datum nicht setzen
    ap.add_argument('--sichtbar', action='store_true', help='DOSBox-Fenster zeigen (sonst ohne Fenster, falls moeglich)')
    a = ap.parse_args()
    t0 = time.time()
    q = finde_q004(a.q004)
    ausgabe = os.path.abspath(a.ausgabe) if a.ausgabe else os.path.join(q, 'Build')
    os.makedirs(ausgabe, exist_ok=True)
    LOG = open(os.path.join(ausgabe, 'bau.log'), 'w', encoding='utf-8')
    say('Sternenschweif - Bau von SCHWEIF.EXE  (' + time.strftime('%d.%m.%Y %H:%M') + ')')
    say(f'  Python {sys.version.split()[0]} ({sys.executable})')
    dosbox = finde_dosbox(a.dosbox)
    B = waehle_baudir(a.baudir)
    paket = os.path.join(HIER, 'baustand.tar.gz')
    if not os.path.exists(paket): abbruch('baustand.tar.gz fehlt neben bauen.py')
    say(f'  Projekt:  {q}'); say(f'  DOSBox:   {dosbox}'); say(f'  Bauordner: {B}')
    say('[0/9] Bauordner vorbereiten')
    vorbereiten(q, B, paket)
    quelle_uebernehmen(q, B, a.nur_paket)
    if a.ohne_datum:
        open(os.path.join(B, 'DATUM.COM'), 'wb').write(bytes([0xB8, 0x00, 0x4C, 0xCD, 0x21]))
        say('  GEGENPROBE: Datum wird NICHT gesetzt')
    dos = Dos(dosbox, B, a.sichtbar)
    say('  DOSBox-Probe: laeuft ' + dos.probe())
    T = Tools(B)
    g1 = stufe1(B, dos, T)
    if not a.nur_asm: stufe2(B, dos, T)
    if a.fix and a.nur_asm: abbruch('Fix-Modus braucht Stufe 2 (nicht mit --nur-asm)')
    exe = os.path.join(B, 'lnk', 'SCHW_CD.EXE')
    say('[9/9] Vergleich mit dem Original' + (' (Standardbau, Vergleichsfassung fuer den Fix-Modus)' if a.fix else ''))
    ziel = os.path.join(ausgabe, 'SCHWEIF.EXE')
    if os.path.exists(ziel): os.chmod(ziel, stat.S_IWRITE | stat.S_IREAD)
    shutil.copyfile(exe, ziel)
    m1, m2 = md5(exe), ORIGINAL_MD5
    say(f'  {m1}  Build\\SCHWEIF.EXE ({os.path.getsize(exe)} Bytes)')
    say(f'  {m2}  Original\\SCHWEIF.EXE')
    say(f'  {dos.laeufe} DOSBox-Laeufe ({dos.zeit / 60:.1f} min), gesamt {(time.time() - t0) / 60:.1f} min')
    fixexe = None
    if a.fix:
        flags = ('-DFIX_BUGS=1' if a.fix == 'wichtig' else '-DFIX_BUGS=2') + (' -DFIX_EXPLOITS=0' if a.exploits_lassen else '')
        flags = (flags + ' ' + a.fixflags).strip()
        fixexe = stufe_fix(B, dos, T, flags, a.fix.upper(), ausgabe)
        say(f'  {dos.laeufe} DOSBox-Laeufe ({dos.zeit / 60:.1f} min), gesamt {(time.time() - t0) / 60:.1f} min')
    say('')
    if m1 == m2:
        say('BYTEGLEICH mit dem Original')
        if fixexe: say(f'Fix-EXE fertig: Build\\{os.path.basename(fixexe)}')
        return 0
    a_ = open(exe, 'rb').read(); b_ = open(os.path.join(B, 'orig', 'SCHWEIF.EXE'), 'rb').read()
    erste = next((i for i in range(min(len(a_), len(b_))) if a_[i] != b_[i]), min(len(a_), len(b_)))
    say(f'ABWEICHUNG vom Original: erste Differenz bei Byte 0x{erste:X}, Laenge {len(a_)} statt {len(b_)}'
        + ('  (Stufe 1 war bytegleich - Ursache liegt in den C-Modulen)' if g1 and not a.nur_asm else ''))
    if fixexe: say(f'Fix-EXE trotzdem gebaut: Build\\{os.path.basename(fixexe)} (Standardbau weicht ab - geaenderte Quellen?)')
    return 1


if __name__ == '__main__':
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        say('abgebrochen (Strg+C)'); sys.exit(3)
