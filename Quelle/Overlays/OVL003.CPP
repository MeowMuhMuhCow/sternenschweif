/* OVL003 - Overlay O003 (Segment 0x1efe in SCHWEIF.EXE): Festplatten-Cache fuer Dateien von der CD, Klasse Cache
 * (s_class.h, 0x16 Bytes, g_cd_cache: Eintraege tab {key, age}, 2-Bit-Zustaende states, count, max, Platzbedarf size,
 * music_off, result, esc). Schluessel = Typ (Bits 13..15: CACHE_TYPE_MAIN/FX/SPEECH/RAW/CD/SPEECH_EXT -> Archiv
 * g_file_main/g_file_fx/g_file_speech/g_file_raw/g_file_cd/g_file_speech_ext, bei CACHE_TYPE_SPEECH_EXT Nr. - 1000)
 * | Nr. (Bits 0..12). Zustaende als 2-Bit-Feld (cache_state_set/cache_state_get).
 * fatal: Spielstand DSA2_TMP.GAM schreiben (savegame_write ueber ein SaveMenu), Palette, Meldung txt[0x1db], sys_exit,
 * exit. split_key: Schluessel zerlegen. disk_full(n): reicht der freie Plattenplatz (getdfree) nicht fuer size + n?
 * find(key): Eintrag suchen. age/touch(n): Alter lesen bzw. altern lassen (Eintrag n = 0, andere -1 bis -30000).
 * victim: Eintrag zum Verdraengen (Zustand 2 sofort, sonst Zustand 0 mit kleinstem Alter). file_name: Dateiname im
 * Archiv (file_getname). copy_file: Datei aus dem Archiv in die Cache-Datei (sprintf(g_file_names[5], Name)) kopieren,
 * Puffer 0x8000 bzw. halbiert bis farcoreleft reicht (> 0x40), Esc bricht ab. on_disk: Datei schon auf der Platte
 * (Indexdatei g_file_names[6..12], Eintrag 2 + Nr. * 20 != 0, oder Datei vorhanden)? file_size: Groesse (file_length).
 * remove(i): Eintrag i loeschen (unlink, nachruecken). insert(key, st): aufnehmen (verdraengen bis Platz und Anzahl
 * reichen, einmal snd_obj_stop(g_snd_music) bei g_cd_music). fetch(key, st): holen/aufnehmen, result = 1 bei Erfolg.
 * Konstruktor (Felder max/8 aufgerundet, Verzeichnis leeren) und Destruktor. load_list(liste, zust, mouse, esc_ok)
 * (load_pic_list): Liste (Ende -1) laden, mouse == 1 mit Wartezeiger (mouse_set_cursor/mouse_reset,
 * g_cursor_nr_prev), esc_ok == 1 Esc erlaubt; fehlt danach ein Eintrag -> alles loeschen und fatal; Rueckgabe
 * 1 = abgebrochen. purge(mouse) (cache_purge): Eintraege mit Zustand 2 loeschen. clear: alle loeschen. init_dir:
 * Verzeichnis anlegen (mkdir) und alle Dateien darin ("*.*", findfirst/findnext) loeschen.
 * Klasse Keylist (0x0c Bytes): Konstruktor/Destruktor, add(nr, typ): Schluessel ohne Dubletten anfuegen (0 = voll),
 * load(cache): mit -1 abschliessen und load_list(keys, states, 1, 0), clear: leeren.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL003.CPP   -> 4235/4235 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (g_file_names[5..12] -> _g_32cb+0x14..). Ohne -Og 6 Bytes zu lang, mit -Oi zu lang (strcpy/strlen sind Aufrufe).
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * - Aufrufe nach oben im Modul push cs / call near, Konstruktor/Destruktor/load_list -> init_dir/clear weiter unten
 *   far.
 * - cache_state_set/get: Teiler als Local "d = 8" ([bp-2], im Konstruktor als register -> SI), "mask = 3;
 *   mask <<= sh; mask ^= -1;" (mov di,3 / shl di,cl / xor di,-1); der Wert ohne register landet in DI.
 * - Registerwahl wieder: register-Local schlaegt register-Parameter (fetch: i ohne register -> DI).
 * - Keylist::add: "n > i" -> cmp [bp-2],si / jg; "i < n" ergaebe cmp si,[bp-2] / jl. "(n = count) >= max"
 *   eingebettet (cmp ax). "k = nr & 0x1fff; k |= type;" -> mov di,ax / or di,[..].
 * - fetch: "if (on_disk(key)) ; else {..}" -> jz / jmp short Ende (Epilogkopie).
 * - new SaveMenu (Klasse mit Pic als erstem Member, automatischer Konstruktor) -> mem_alloc_w(0x6f7) + if (p)
 *   Pic_ctor(p); delete -> if (p) { Pic_dtor(p, 2); mem_free_v(p); } mit totem jmp short.
 * - getdfree(getdisk() + 1, ..) -> inc al (unsigned-char-Parameter); "(unsigned long)a * b * c" -> movzx/imul.
 * - disk_full: struct dfree (8 Bytes) bleibt in Deklarationsreihenfolge vor dem long ([bp-8], [bp-0ch]).
 * Originalfehler/Auffaelliges: file_name/copy_file haben kein default - bei Typ 0 oder 0xe000 bleibt der Name bzw.
 *   das Archiv uninitialisiert. copy_file prueft _creat nicht und reicht ein negatives Leseergebnis von
 *   file_read als Laenge an write() weiter.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <dir.h>
#include <alloc.h>
#include <bios.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:00bc..00ce, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

void cache_state_set(unsigned short far *states, signed short idx, register signed short val)
{
	signed short d;
	signed short w;
	signed short sh;
	signed short mask;

	d = 8;
	w = idx / d;
	sh = idx % d * 2;
	val &= 3;
	val <<= sh;
	mask = 3;
	mask <<= sh;
	mask ^= -1;
	states[w] &= mask;
	states[w] |= val;
}

signed short cache_state_get(unsigned short far *states, register signed short idx)
{
	signed short d;
	signed short w;
	signed short sh;
	register signed short v;

	d = 8;
	w = idx / d;
	sh = idx % d * 2;
	v = states[w];
	v >>= sh;
	v &= 3;
	return v;
}

void Cache::fatal()
{
	char far *s;
	SaveMenu far *sm;
	char far *pal;

	s = "DSA2_TMP.GAM";
	pal = new char[0x300];
	sm = new SaveMenu;
	savegame_write(sm, s);
	load_palette_arc3b(pal);
	set_palette(pal + 0x1e0, 0xa0, 0x60);
	dlg_set16(g_dialog, 5);
	sprintf(g_text_output_buf, g_ttx[0x1db], s);
	dlg_message(g_dialog, g_text_output_buf);
	delete sm;
	sys_exit(0);
	exit(0);
}

void Cache::split_key(unsigned short key, unsigned short far *type, unsigned short far *nr)
{
	*type = key & CACHE_TYPE_MASK;
	*nr = key & CACHE_NR_MASK;
	if (*type == CACHE_TYPE_SPEECH_EXT)
		*nr -= 1000;
}

signed short Cache::disk_full(signed long need)
{
	struct dfree df;
	unsigned long avail;

	getdfree(getdisk() + 1, &df);
	avail = (unsigned long)df.df_avail * df.df_bsec * df.df_sclus;
	return size + need >= avail ? 1 : 0;
}

signed short Cache::find(signed short key)
{
	signed short i;

	for (i = 0; i < count; i++) {
		if (tab[i].key == key)
			return i;
	}
	return -1;
}

signed short Cache::age(signed short i)
{
	signed short t;

	t = tab[i].age;
	return t;
}

void Cache::touch(signed short n)
{
	signed short i;

	for (i = 0; i < count; i++) {
		if (i == n)
			tab[i].age = 0;
		else {
			tab[i].age--;
			if (tab[i].age < -30000)
				tab[i].age = -30000;
		}
	}
}

signed short Cache::victim()
{
	signed short best;
	signed short sel;
	signed short st;
	register signed short i;
	register signed short t;

	best = 0x7fff;
	sel = -1;
	for (i = 0; i < count; i++) {
		st = cache_state_get(states, i);
		if (st == 2)
			return i;
		if (st == 0) {
			t = age(i);
			if (t < best) {
				best = t;
				sel = i;
			}
		}
	}
	return sel;
}

char far *Cache::file_name(unsigned short key)
{
	unsigned short type;
	unsigned short nr;
	char far *s;

	split_key(key, &type, &nr);
	switch (type) {
	case CACHE_TYPE_MAIN:
		s = file_getname(g_file_main, nr);
		break;
	case CACHE_TYPE_FX:
		s = file_getname(g_file_fx, nr);
		break;
	case CACHE_TYPE_SPEECH:
		s = file_getname(g_file_speech, nr);
		break;
	case CACHE_TYPE_RAW:
		s = file_getname(g_file_raw, nr);
		break;
	case CACHE_TYPE_CD:
		s = file_getname(g_file_cd, nr);
		break;
#if FIX_0086
	/* Fix #86: unbekannter Typ: Nullzeiger statt Muellzeiger */
	case CACHE_TYPE_SPEECH_EXT:
		s = file_getname(g_file_speech_ext, nr);
		break;
	default:
		s = 0;
		break;
	}
	return s;
#else
	case CACHE_TYPE_SPEECH_EXT:
		s = file_getname(g_file_speech_ext, nr);
		break;
	}
	return s;
#endif
}

void Cache::copy_file(unsigned short key)
{
	char far *arc;
	unsigned short type;
	unsigned short nr;
	char far *buf;
	char far *name;
	signed long n;
	register unsigned short sz;
	register signed short fd;

	sz = 0x8000;
	split_key(key, &type, &nr);
	name = new char[0x28];
	while (farcoreleft() < sz && sz > 0x40)
		sz = sz >> 1;
	if (sz <= 0x40) {
		delete name;
		return;
	}
	buf = new char[sz];
	sprintf(name, g_file_names[5], file_name(key));
	switch (type) {
	case CACHE_TYPE_MAIN:
		arc = g_file_main;
		break;
	case CACHE_TYPE_FX:
		arc = g_file_fx;
		break;
	case CACHE_TYPE_SPEECH:
		arc = g_file_speech;
		break;
	case CACHE_TYPE_RAW:
		arc = g_file_raw;
		break;
	case CACHE_TYPE_CD:
		arc = g_file_cd;
		break;
#if FIX_0086
	/* Fix #86: unbekannter Typ: abbrechen statt mit Muellzeiger zu lesen */
	case CACHE_TYPE_SPEECH_EXT:
		arc = g_file_speech_ext;
		break;
	default:
		delete buf;
		delete name;
		return;
	}
#else
	case CACHE_TYPE_SPEECH_EXT:
		arc = g_file_speech_ext;
		break;
	}
#endif
	unlink(name);
	if (file_open(arc, nr) > 0) {
#if FIX_0086
		/* Fix #86: _creat pruefen, negatives Leseergebnis nicht schreiben */
		fd = _creat(name, 0);
		if (fd != -1) {
			do {
				n = file_read(arc, buf, (unsigned long)sz);
				if (n > 0)
					write(fd, buf, n);
				if (esc && bioskey(1) && bioskey(0) == KEY_ESC)
					esc = -1;
			} while (n > 0 && esc != -1);
			close(fd);
		}
#else
		fd = _creat(name, 0);
		do {
			n = file_read(arc, buf, (unsigned long)sz);
			write(fd, buf, n);
			if (esc && bioskey(1) && bioskey(0) == KEY_ESC)
				esc = -1;
		} while (n > 0 && esc != -1);
		close(fd);
#endif
		file_close(arc);
	}
	delete buf;
	delete name;
}

signed short Cache::on_disk(unsigned short key)
{
	unsigned short type;
	unsigned short nr;
	char far *path;
	signed long pos;
	signed short v;
	register signed short fd;
	register signed short ok;

	ok = 0;
	path = 0;
	split_key(key, &type, &nr);
	switch (type) {
	case CACHE_TYPE_MAIN:
		path = g_file_names[6];
		break;
	case CACHE_TYPE_FX:
		path = g_file_names[8];
		break;
	case CACHE_TYPE_SPEECH:
		path = g_file_names[10];
		break;
	case CACHE_TYPE_RAW:
		path = g_file_names[12];
		break;
	}
	if (path != 0) {
		fd = open(path, O_RDONLY | O_BINARY);
		if (fd != -1) {
			pos = 2;
			pos += nr * 20L;
			lseek(fd, pos, 0);
			_read(fd, &v, 2);
			if (v != 0)
				ok = 1;
			close(fd);
		}
	}
	if (ok != 1) {
		fd = open(file_name(key), O_RDONLY | O_BINARY);
		if (fd >= 0) {
			close(fd);
			ok = 1;
		}
	}
	return ok;
}

signed long Cache::file_size(unsigned short key)
{
	signed long sz;
	unsigned short type;
	unsigned short nr;

	sz = 0;
	split_key(key, &type, &nr);
	switch (type) {
	case CACHE_TYPE_MAIN:
		sz = file_length(g_file_main, nr);
		break;
	case CACHE_TYPE_FX:
		sz = file_length(g_file_fx, nr);
		break;
	case CACHE_TYPE_SPEECH:
		sz = file_length(g_file_speech, nr);
		break;
	case CACHE_TYPE_RAW:
		sz = file_length(g_file_raw, nr);
		break;
	case CACHE_TYPE_CD:
		sz = file_length(g_file_cd, nr);
		break;
	case CACHE_TYPE_SPEECH_EXT:
		sz = file_length(g_file_speech_ext, nr);
		break;
	}
	return sz;
}

void Cache::remove(register signed short i)
{
	char far *name;
	register signed short j;

	name = new char[0x28];
	sprintf(name, g_file_names[5], file_name(tab[i].key));
	unlink(name);
	delete name;
	count--;
	for (j = i; j < count; j++) {
		tab[j] = tab[j + 1];
		cache_state_set(states, j, cache_state_get(states, j + 1));
	}
}

void Cache::insert(signed short key, signed short v)
{
	signed long need;
	register signed short k;

	if (music_off == 0) {
		if (g_cd_music)
			snd_obj_stop(g_snd_music);
		music_off = 1;
	}
	need = file_size(key);
	while (disk_full(need) || count >= max) {
		k = victim();
		if (k >= 0)
			remove(k);
		else
			break;
	}
	copy_file(key);
	tab[count].key = key;
	tab[count].age = 0;
	cache_state_set(states, count, v);
	count++;
}

void Cache::fetch(register signed short key, signed short v)
{
	signed short i;

	result = 0;
	i = find(key);
	if (i < 0) {
		if (on_disk(key))
			;
		else {
			insert(key, v);
			result = 1;
		}
	} else {
		touch(i);
		cache_state_set(states, i, v);
		result = 1;
	}
}

Cache::Cache(signed short m, signed long s)
{
	register signed short d;
	signed short n;

	max = m;
	size = s;
	d = 8;
	n = max / d + (max % d ? 1 : 0);
	tab = new struct cache_entry[max];
	states = new unsigned short[n];
	count = 0;
	init_dir();
}

Cache::~Cache()
{
	clear();
	delete states;
	delete tab;
}

signed short Cache::load_list(signed short far *list, unsigned short far *st, signed short mouse, signed short esc_ok)
{
	signed short k;
	signed short save;
	signed short far *res;
	register signed short i;
	register signed short n;

	i = 0;
	n = 0;
	if (mouse == 1) {
		save = g_cursor_nr_prev;
		mouse_set_cursor(CURSOR_WAIT, 1);
		mouse_show(0);
	}
	music_off = 0;
	if (esc_ok == 1)
		esc = 1;
	else
		esc = 0;
	while (list[n] != -1)
		n++;
	res = new signed short[n + 1];
	while (list[i] != -1 && esc != -1) {
		fetch(list[i], 0);
		res[i] = result;
		i++;
	}
	if (esc != -1) {
		for (i = 0; i < n; i++) {
			if (res[i] == 1) {
				k = find(list[i]);
				if (k < 0) {
					clear();
					fatal();
				} else
					cache_state_set(states, k, cache_state_get(st, i));
			}
		}
	}
	delete res;
	if (mouse == 1) {
		mouse_hide(0);
		mouse_reset();
		g_cursor_nr_prev = save;
	}
	if (esc == -1)
		return 1;
	else
		return 0;
}

void Cache::purge(register signed short mouse)
{
	signed short save;
	register signed short i;

	i = 0;
	if (mouse == 1) {
		save = g_cursor_nr_prev;
		mouse_set_cursor(CURSOR_WAIT, 1);
		mouse_show(0);
	}
	while (i < count) {
		if (cache_state_get(states, i) == 2)
			remove(i);
		else
			i++;
	}
	if (mouse == 1) {
		mouse_hide(0);
		mouse_reset();
		g_cursor_nr_prev = save;
	}
}

void Cache::clear()
{
	register signed short i;

	for (i = 0; i < count; )
		remove(i);
}

void Cache::init_dir()
{
	char far *p;
	struct ffblk ff;
	char path[40];

	strcpy(path, g_file_names[5]);
	path[strlen(g_file_names[5]) - 3] = 0;
	mkdir(path);
	p = path + strlen(path);
	*p = '\\';
	p++;
	*p = 0;
	strcpy(p, "*.*");
	if (findfirst(path, &ff, 0x20) != 0)
		return;
	do {
		strcpy(p, ff.ff_name);
		unlink(path);
	} while (findnext(&ff) == 0);
}

Keylist::Keylist(signed short m)
{
	register signed short d;
	signed short n;

	max = m;
	d = 8;
	n = max / d + (max % d ? 1 : 0);
	keys = new signed short[max + 1];
	states = new unsigned short[n];
	count = 0;
}

Keylist::~Keylist()
{
	delete states;
	delete keys;
}

signed short Keylist::add(signed short nr, unsigned short type)
{
	signed short n;
	register signed short i;
	register signed short k;

	if ((n = count) >= max)
		return 0;
	k = nr & CACHE_NR_MASK;
	k |= type;
	for (i = 0; n > i; i++) {
		if (keys[i] == k)
			return 1;
	}
	keys[n] = k;
	cache_state_set(states, count, 0);
	count++;
	return 1;
}

void Keylist::load(void far *cache)
{
	keys[count] = -1;
	((Cache far *)cache)->load_list(keys, states, 1, 0);
}

void Keylist::clear()
{
	count = 0;
}
