/* OVL106 - Overlay O106 (Segment 0x215f in SCHWEIF.EXE): Spielstaende speichern/laden (class SaveMenu, 0x6f7 Bytes,
 * s_class.h: Pic-Mitglied +0, Texte txt, Kopf hdr/ver_a..ver_d, Auswahl sel/first, Handle fd, Namen names[]).
 * STUFE D1 (wovd1, 19.10.2026): asm-strcat durch echtes strcat(.., "..") ersetzt - Code GLEICH, das Literal
 *   im eigenen _DATA bildet wovd1/cobjfix_d.py auf ds:613e/6143/6180/6185 ab (Bytes geprueft). Hinweise unten zum asm sind alt.
 * savegame_save_cur: markierten Platz als <Name>.GAM speichern (savegame_write), bei Fehler Name ohne ' '/'.' +
 *   .GAM melden (txt[9]) und ersatzweise LASTGAME. savegame_write(name): Datei anlegen (_creat, Handle auch
 *   g_save_fd), Puffer so gross wie moeglich (32000, halbieren bis farcoreleft reicht), Kopf
 *   savegame_write_header, savegame_write_desc, 7 Helden hero_save, 6 Gruppen group_save, conv_save/Date_save/
 *   automap_save, Bloecke GLOB/RUNV/MONS/PLAY/LOCS (Kennung, Laenge = Abstand zweier Marken g_save_*, Daten),
 *   dann alle Dateien des Temp-Verzeichnisses ausser FILE_HEROES_ALL als TEMP-Bloecke (Laenge 2 Bytes oder -1 +
 *   4 Bytes, Name 14 Bytes, Inhalt); Fehler -> Handle -1, Datei loeschen, g_ttx[455]. savegame_seek_block(tag,
 *   n): n-ten Block mit Kennung tag ab SAVE_BLOCKS_START suchen (danach 4 zurueck). savegame_read(name):
 *   Spielstand laden (Bloecke CHAR/PART/XDIA/TIME/AMAP/GLOB/RUNV/MONS/PLAY/LOCS), Temp-Verzeichnis leeren und
 *   TEMP-Bloecke als Dateien wiederherstellen, g_group = &g_groups[g_group_no], Musik/Klang/Sprache nach
 *   g_opt_music/g_opt_sound/g_cd_speech an/aus; g_pp20_index/g_loaded_dungeon/g_loaded_town = -1,
 *   g_town_cells_state = 0. savegame_load_cur: markierten Platz laden (pruefen per savegame_check, Meldung).
 *   savegame_load(s): Spielstand s laden (Texte 0x9b/0x1e laden und freigeben), -1 wenn die Datei fehlt.
 *   savegame_write_header/savegame_read_header: Kopf "DSA VERSION" + 4 Versionsbytes schreiben/lesen.
 *   savegame_delete(i): Platz i loeschen und Beschreibungsfeld leeren (do_h_line). savegame_check(name, msg): Kopf
 *   pruefen; fremde/alte Version -> DSA-1-Import (new Save, load, delete), ggf. sofort neu speichern, Meldung
 *   txt[17]/[18]/[21], Grundzustand file_getname(0x80) laden.
 *
 * BCC -mlarge -3 -Ot -Oi -c -Y OVL106.CPP   -> 4986/4986 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit -Og
 *   ebenso GLEICH). -Oi: strcpy (far -> Stack: rep movsw/adc/rep movsb) und strcmp (repe cmpsb / sbb / sbb)
 *   als Intrinsics; strcmp(a, b): a in DS:SI, b in ES:DI (Laenge von b per repne scasb).
 * - Inline-Assembler (4 Stellen): strcat(name, ".GAM") mit 5-Byte-Literal ds:613e/6143/6180/6185 (repne scasb /
 *   dec di / mov cx,5 / rep movsb, xor ax,ax als db 33h,0c0h) - wie das bekannte strcat-Literal (OVL111/OVL238).
 * - Blocklaenge "len = g_save_glob_end - g_save_glob" (Zeigerdifferenz zweier DS-Felder): mov ax,off / xor dx,dx / sub / sbb
 *   / shl eax / shrd -> mov [len],ax. cobjfix ordnet die Endadressen Nachbar-Publics zu (_g_2a7c+0x22 usw.).
 * - "if ((g_save_fd = b->fd = _creat(path, 0)) != -1)": +d6 zuerst gespeichert, dann ds:c68d, cmp ax.
 * - chunk unsigned (flen -= chunk -> movzx eax); "chunk = flen > bufsize ? bufsize : flen".
 * - savegame_read: Klang ueber inline sound::set_enabled(v) { enabled = v; } (Parameterkopien [bp-20h]/[bp-22h]); der
 *   Kopierpuffer buf muss im TEMP-Block deklariert sein (liegt hinter den inline-Kopien bei [bp-26h]).
 * - savegame_check: "if (strcmp(..) != 0 || ver_c != g_version_c || n != 16) {lang} else {kurz}" (kurzer Zweig
 *   liegt hinten); new Save -> O176_018E(0, ..), delete -> O176_01CF(p, 3).
 * - savegame_save_cur: "return;" im if-Zweig -> Epilogkopie auch ohne -Og (mouse_reset steht zweimal im Quelltext).
 * - names[sel + first]: das alte Sv nannte +d0 "first" und +d4 "sel" - Reihenfolge der Summanden bleibt +d0, +d4.
 * Originalfehler: savegame_write benutzt path nach der Temp-Schleife als 14-Byte-Namenspuffer (strcpy(path, ff_name));
 *   im Fehlerfall vergleicht/loescht es daher den zuletzt kopierten Temp-Dateinamen statt des Spielstands.
 *   Nur das Schreiben des letzten Datenblocks (LOCS) und der TEMP-Inhalte wird geprueft. savegame_seek_block:
 *   Parameter b
 *   unbenutzt.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <dir.h>
#include <fcntl.h>
#include <alloc.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:613e..618a, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

#define SOUND(o) ((struct sound far *)(o))	/* Klangobjekt (O042) */

void savegame_save_cur(SaveMenu far *b)
{
	char name[14];

	mouse_set_cursor(5, 1);
	strcpy(name, b->names[b->sel + b->first].name);
	strcat(name, ".GAM");
	if (savegame_write(b, name) != 0) {
		str_cut_at(name, ' ');
		str_cut_at(name, '.');
		strcat(name, ".GAM");
		sprintf(g_text_output_buf, b->txt[9], name);
		dlg_message(g_dialog, g_text_output_buf);
		savegame_write(b, "LASTGAME");
		mouse_reset();
		return;
	}
	mouse_reset();
}

signed short savegame_write(SaveMenu far *b, char far *name)
{
	signed short i;
	signed short len;
	struct hero far *h;
	struct group far *g;
	signed long flen;
	unsigned short chunk;
	signed short wr;
	signed short r;
	signed short fd;
	signed short ok = 0;
	void far *buf;
	signed short good = 1;
	signed long bufsize = 32000;
	char path[40];
	char path2[40];
	struct ffblk ff;

	sprintf(path, g_path_fmts[1], name);
	if ((g_save_fd = b->fd = _creat(path, 0)) != -1) {
		ok = 1;
		while (farcoreleft() < bufsize)
			bufsize /= 2;
		buf = mem_alloc(bufsize);
		savegame_write_header(b);
		savegame_write_desc(b);
		for (i = 0, h = g_heroes; i < 7; i++, h++)
			hero_save(h);
		for (i = 0, g = g_groups; i < 6; i++, g++)
			group_save(g);
		conv_save(g_dialog2);
		Date_save(g_date);
		automap_save(g_map);
#if FIX_0129
		/* Fix #129: alle Datenbloecke auf Schreibfehler pruefen */
		len = g_save_glob_end - g_save_glob;
		if (_write(g_save_fd, "GLOB", 4) != 4) good = 0;
		if (_write(g_save_fd, &len, 2) != 2) good = 0;
		if (_write(g_save_fd, g_save_glob, len) != len) good = 0;
		len = g_save_runv_end - g_save_runv;
		if (_write(g_save_fd, "RUNV", 4) != 4) good = 0;
		if (_write(g_save_fd, &len, 2) != 2) good = 0;
		if (_write(g_save_fd, g_save_runv, len) != len) good = 0;
		len = g_save_mons_end - g_save_mons;
		if (_write(g_save_fd, "MONS", 4) != 4) good = 0;
		if (_write(g_save_fd, &len, 2) != 2) good = 0;
		if (_write(g_save_fd, g_save_mons, len) != len) good = 0;
		len = g_save_play_end - g_save_play;
		if (_write(g_save_fd, "PLAY", 4) != 4) good = 0;
		if (_write(g_save_fd, &len, 2) != 2) good = 0;
		if (_write(g_save_fd, g_save_play, len) != len) good = 0;
		len = g_save_locs_end - g_save_locs;
		if (_write(g_save_fd, "LOCS", 4) != 4) good = 0;
		if (_write(g_save_fd, &len, 2) != 2) good = 0;
		wr = _write(g_save_fd, g_save_locs, len);
		if (wr != len)
			good = 0;
#else
		len = g_save_glob_end - g_save_glob;
		_write(g_save_fd, "GLOB", 4);
		_write(g_save_fd, &len, 2);
		_write(g_save_fd, g_save_glob, len);
		len = g_save_runv_end - g_save_runv;
		_write(g_save_fd, "RUNV", 4);
		_write(g_save_fd, &len, 2);
		_write(g_save_fd, g_save_runv, len);
		len = g_save_mons_end - g_save_mons;
		_write(g_save_fd, "MONS", 4);
		_write(g_save_fd, &len, 2);
		_write(g_save_fd, g_save_mons, len);
		len = g_save_play_end - g_save_play;
		_write(g_save_fd, "PLAY", 4);
		_write(g_save_fd, &len, 2);
		_write(g_save_fd, g_save_play, len);
		len = g_save_locs_end - g_save_locs;
		_write(g_save_fd, "LOCS", 4);
		_write(g_save_fd, &len, 2);
		wr = _write(g_save_fd, g_save_locs, len);
		if (wr != len)
			good = 0;
#endif
		sprintf(g_text_output_buf, g_path_fmts[0], "*.*");
		r = findfirst(g_text_output_buf, &ff, 0);
		while (r == 0) {
			if (strcmp(ff.ff_name, file_getname(g_file_main, FILE_HEROES_ALL)) != 0) {
				_write(g_save_fd, "TEMP", 4);
				sprintf(path2, g_path_fmts[0], ff.ff_name);
				fd = _open(path2, O_BINARY | O_RDONLY);
				flen = filelength(fd);
				flen += 14;
				if (flen > 0x8000L) {
					len = -1;
					_write(g_save_fd, &len, 2);
					_write(g_save_fd, &flen, 4);
				} else {
					len = flen;
					_write(g_save_fd, &len, 2);
				}
#if FIX_0129
				/* Fix #129: Temp-Dateinamen nicht in path ablegen (path bleibt der Spielstand) */
				strcpy(path2, ff.ff_name);
				_write(g_save_fd, path2, 14);
#else
				strcpy(path, ff.ff_name);
				_write(g_save_fd, path, 14);
#endif
				flen -= 14;
				while (flen > 0) {
					chunk = flen > bufsize ? bufsize : flen;
					chunk = _read(fd, buf, chunk);
					wr = _write(g_save_fd, buf, chunk);
					if (wr != chunk)
						good = 0;
					flen -= chunk;
				}
				close(fd);
			}
			r = findnext(&ff);
		}
		close(g_save_fd);
		mem_free(buf);
	}
#if FIX_0129
	/* Fix #129: LASTGAME am Namen erkennen; geloescht wird der Spielstand (path) */
	if (good == 0 && strcmp(name, "LASTGAME") != 0) {
#else
	if (good == 0 && strcmp(path, "LASTGAME") != 0) {
#endif
		ok = 0;
		g_save_fd = b->fd = -1;
		unlink(path);
		dlg_message(g_dialog, g_ttx[455]);
	}
	return ok;
}

signed short savegame_seek_block(SaveMenu far *b, signed long tag, signed short n)
{
	signed short k = 1;
	signed long id;
	signed short len;
	signed short rd = 1;
	signed long flen;
	signed short found = 0;

	lseek(g_save_fd, SAVE_BLOCKS_START, SEEK_SET);
	while (found == 0 && rd != 0) {
		id = 0;
		rd = _read(g_save_fd, &id, 4);
		if (id == tag && k++ == n)
			found = 1;
		else {
			_read(g_save_fd, &len, 2);
			if (len == -1) {
				_read(g_save_fd, &flen, 4);
				lseek(g_save_fd, flen, SEEK_CUR);
			} else
				lseek(g_save_fd, (signed long)len, SEEK_CUR);
		}
	}
	if (found)
		lseek(g_save_fd, -4L, SEEK_CUR);
	return found;
}

signed short savegame_read(SaveMenu far *b, char far *name)
{
	signed short i;
	signed short len;
	signed long tag;
	struct hero far *h;
	struct group far *g;
	signed long flen;
	unsigned short chunk;
	signed short r;
	signed short fd;
	signed short n = 1;
	signed short ok = 0;
	char path2[40];
	char path[40];
	struct ffblk ff;

	sprintf(path, g_path_fmts[1], name);
	if ((g_save_fd = _open(path, O_BINARY | O_RDONLY)) != -1) {
		ok = 1;
		if (savegame_seek_block(b, SAVE_TAG_CHAR, 1))
			for (i = 0, h = g_heroes; i < 7; i++, h++)
				hero_load(h);
		if (savegame_seek_block(b, SAVE_TAG_PART, 1))
			for (i = 0, g = g_groups; i < 6; i++, g++)
				group_load(g);
		if (savegame_seek_block(b, SAVE_TAG_XDIA, 1))
			conv_load(g_dialog2);
		if (savegame_seek_block(b, SAVE_TAG_TIME, 1))
			Date_load(g_date);
		if (savegame_seek_block(b, SAVE_TAG_AMAP, 1))
			automap_load(g_map);
		if (savegame_seek_block(b, SAVE_TAG_GLOB, 1)) {
			_read(g_save_fd, &tag, 4);
			if (tag == SAVE_TAG_GLOB) {
				_read(g_save_fd, &len, 2);
				_read(g_save_fd, g_save_glob, len);
			}
		}
		if (savegame_seek_block(b, SAVE_TAG_RUNV, 1)) {
			_read(g_save_fd, &tag, 4);
			if (tag == SAVE_TAG_RUNV) {
				_read(g_save_fd, &len, 2);
				_read(g_save_fd, g_save_runv, len);
			}
		}
		if (savegame_seek_block(b, SAVE_TAG_MONS, 1)) {
			_read(g_save_fd, &tag, 4);
			if (tag == SAVE_TAG_MONS) {
				_read(g_save_fd, &len, 2);
				_read(g_save_fd, g_save_mons, len);
			}
		}
		if (savegame_seek_block(b, SAVE_TAG_PLAY, 1)) {
			_read(g_save_fd, &tag, 4);
			if (tag == SAVE_TAG_PLAY) {
				_read(g_save_fd, &len, 2);
				_read(g_save_fd, g_save_play, len);
			}
		}
		if (savegame_seek_block(b, SAVE_TAG_LOCS, 1)) {
			_read(g_save_fd, &tag, 4);
			if (tag == SAVE_TAG_LOCS) {
				_read(g_save_fd, &len, 2);
				_read(g_save_fd, g_save_locs, len);
			}
		}
		sprintf(path, g_path_fmts[0], "*.*");
		r = findfirst(path, &ff, 0);
		while (r == 0) {
			if (strcmp(ff.ff_name, file_getname(g_file_main, FILE_HEROES_ALL)) != 0) {
				sprintf(path2, g_path_fmts[0], ff.ff_name);
				unlink(path2);
			}
			r = findnext(&ff);
		}
		while (savegame_seek_block(b, SAVE_TAG_TEMP, n)) {
			_read(g_save_fd, &tag, 4);
			if (tag == SAVE_TAG_TEMP) {
				void far *buf;

				n++;
				_read(g_save_fd, &len, 2);
				if (len == -1)
					_read(g_save_fd, &flen, 4);
				else
					flen = len;
				flen -= _read(g_save_fd, path, 14);
				sprintf(path2, g_path_fmts[0], path);
				fd = _creat(path2, 0);
				buf = mem_alloc(30000L);
				while (flen > 0) {
					chunk = flen > 30000L ? 30000 : flen;
					chunk = _read(g_save_fd, buf, chunk);
					_write(fd, buf, chunk);
					flen -= chunk;
				}
				close(fd);
				mem_free(buf);
			}
		}
		close(g_save_fd);
		g_group = &g_groups[g_group_no];
		if (g_opt_music == 0)
			snd_obj_stop(g_snd_music);
		SOUND(g_snd_music)->set_enabled(g_opt_music);
		if (g_opt_sound == 0)
			snd_obj_stop(g_snd_fx);
		SOUND(g_snd_fx)->set_enabled(g_opt_sound);
		if (g_cd_speech == 0)
			Speech_stop(g_snd_speech);
		Speech_set(g_snd_speech, g_cd_speech);
	}
	g_loaded_town = g_loaded_dungeon = g_pp20_index = -1;
	g_town_cells_state = 0;
	return ok;
}

signed short savegame_load_cur(SaveMenu far *b)
{
	char far *msg;
	signed short ret;
	char name[14];

	prepare_dirs();
	strcpy(name, b->names[b->sel + b->first].name);
	strcat(name, ".GAM");
	msg = (char far *)mem_alloc(500L);
	mouse_set_cursor(5, 1);
	if (savegame_check(b, name, msg) != 0) {
		ret = savegame_read(b, name);
		mouse_reset();
		dlg_message(g_dialog, msg);
		mem_free(msg);
	} else {
		mouse_reset();
		dlg_message(g_dialog, msg);
		mem_free(msg);
		ret = 1;
	}
	return ret;
}

signed short savegame_load(SaveMenu far *b, char far *s)
{
	char far *msg;
	signed short ret;
	signed short fd;
	char name[20];
	char path[80];

	strcpy(name, s);
	sprintf(path, g_path_fmts[1], name);
	if ((fd = _open(path, O_BINARY | O_RDONLY)) == -1) {
		ret = -1;
	} else {
		close(fd);
		txt_load_list(g_font, 0x9b, 0x1e, &b->txt_buf, &b->txt);
		prepare_dirs();
		msg = (char far *)mem_alloc(500L);
		mouse_set_cursor(5, 1);
		if (savegame_check(b, name, msg) != 0) {
			ret = savegame_read(b, name);
			mouse_reset();
			dlg_message(g_dialog, msg);
			mem_free(msg);
		} else {
			mouse_reset();
			dlg_message(g_dialog, msg);
			mem_free(msg);
			ret = 1;
		}
		mem_free(b->txt_buf);
		mem_free(b->txt);
	}
	return ret;
}

signed short savegame_write_header(SaveMenu far *b)
{
	signed short n;

	lseek(b->fd, 0L, SEEK_SET);
	n = write(b->fd, g_dsa_version_string, 12);
	n += write(b->fd, &g_version_d, 1);
	n += write(b->fd, &g_version_c, 1);
	n += write(b->fd, &g_version_b, 1);
	n += write(b->fd, &g_version_a, 1);
	return n;
}

signed short savegame_read_header(SaveMenu far *b)
{
	signed short n;

	lseek(b->fd, 0L, SEEK_SET);
	n = read(b->fd, b->hdr, 12);
	n += read(b->fd, &b->ver_d, 1);
	n += read(b->fd, &b->ver_c, 1);
	n += read(b->fd, &b->ver_b, 1);
	n += read(b->fd, &b->ver_a, 1);
	return n;
}

void savegame_delete(SaveMenu far *b, signed short idx)
{
	signed short i;
	char name[14];
	char path[40];

	strcpy(name, b->names[idx].name);
	strcat(name, ".GAM");
	sprintf(path, g_path_fmts[1], name);
	unlink(path);
	mouse_hide(0);
	for (i = 0; i < 0x52; i++)
		do_h_line(g_vga_buf, 0x20, i + 0x3b, 0x38, 0xce);
	mouse_show(0);
}

signed short savegame_check(SaveMenu far *b, char far *name, char far *msg)
{
	signed short fd;
	signed short n;
	signed short ret = 0;
	Save far *p;

	sprintf(g_text_output_buf, g_path_fmts[1], name);
	g_save_fd = b->fd = _open(g_text_output_buf, O_BINARY | O_RDWR);
	n = savegame_read_header(b);
	lseek(b->fd, 0L, SEEK_SET);
	if (strcmp(b->hdr, g_dsa_version_string) != 0 || b->ver_c != g_version_c || n != 16) {
		fd = g_save_fd;
		if (n == 16) {
			p = new Save;
			ret = p->load(fd);
			delete p;
		} else
			ret = 0;
		close(fd);
		if (ret == 0)
			sprintf(msg, b->txt[17], name);
		else if (ret == -1) {
			sprintf(msg, b->txt[21], name);
			savegame_write(b, name);
			ret = 2;
		} else {
			savegame_write(b, name);
			sprintf(msg, b->txt[18], name);
			ret = 2;
		}
		savegame_read(b, file_getname(g_file_main, 0x80));
	} else {
		ret = 1;
		*msg = 0;
		close(g_save_fd);
	}
	return ret;
}
