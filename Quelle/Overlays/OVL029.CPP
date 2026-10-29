/* OVL029 - Overlay O029 (Segment 0x1f96 in SCHWEIF.EXE): Heldendateien (Saetze zu 0x5ef Bytes) ueber das
 * Arbeitsdatei-Objekt g_ofile (class TempFile, O243) und DOS-Dateien mit Namen sprintf(g_path_fmts[0]/[1],
 * file_getname(g_file_main, id)) (..\TEMP\ bzw. ..\GAMES\). Dateien: FILE_HEROES (0xfb, Helden des Spielstands)
 * und FILE_HEROES_ALL (0xfa, alle erschaffenen Helden, mit Kopie in GAMES).
 * herofile_copy(src, dst): Datei in 20000-Byte-Bloecken kopieren. savegame_read_hero(slot, id, buf): Satz slot
 * lesen (FILE_HEROES -> Name nach *g_names_star_idx, Datei-id 0), Ergebnis Laenge oder 0. herofile_remove(id,
 * slot): Satz slot aus der Datei entfernen (Kopie nach "t.$$$", Neuaufbau, Kopie loeschen, bei FILE_HEROES_ALL
 * auch die Kopie in GAMES). hero_delete_chr(name): Helden name in beiden Dateien loeschen. herofile_find(name,
 * &slot, id): Helden per strcmp (file_name) suchen -> 1, sonst slot = -1 und 0. herofile_write(slot, buf, id)/
 * herofile_append(buf, id): Satz ueberschreiben bzw. anhaengen (Datei ggf. anlegen, bei id != FILE_HEROES auch in
 * die Kopie in GAMES). herofile_save(h, id): speichern (vorhanden -> herofile_write, sonst herofile_append).
 * hero_load_chr(id, slot, i, group): Satz als Held g_heroes[i] laden, group_no = group, startup_equipment,
 * 1. hero_save_file(h): herofile_save(h, FILE_HEROES).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL029.CPP   -> 1980/1980 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - C++ wegen der Methoden von TempFile (g_ofile, freie Namen ofile_* -> push ds / push 0b635h) und des
 *   inline-Setters hero::set_group_no(signed short) (Parameterkopie [bp-6], dann mov al).
 * - RTL: _open/_creat/_read/_write (-> __open usw.) neben open/creat/write/close/lseek/unlink.
 * - register-Parameter id bekommt SI nur, wenn die lokale Variable n in savegame_read_hero KEIN register traegt
 *   (wie in O087); in herofile_append dagegen register-Local fd = SI, Parameter id = DI.
 * - Formatzeiger ds:3587/ds:358b als Feld g_path_fmts[0]/[1] (ds:358b hat kein eigenes Public).
 * - -Og noetig: Epilogkopien (return 1 / else return 0 in herofile_find, if/else in herofile_write/_append/_save).
 * - (signed long)slot * 0x5ef -> movsx eax / imul eax,eax,5efh.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:186c..187e, dsana) - initialisierte Daten in Adressreihenfolge */
/* Die drei Texte "t.$$$" (ds:186c/1872/1878) sind Literale in herofile_remove (Literalpool in Reihenfolge des
 * Auftretens). ds:187a (Mitte des dritten Textes) ist zugleich die gefaltete Basis g_187a der Ortsdaten-Tabelle
 * (long je Stadt ab ds:187e), die O030 als g_187a[town] liest. */

void herofile_copy(char far *src, char far *dst)
{
	signed short fin;
	void far *buf;
	register signed short fout;
	register signed short n;

	buf = mem_alloc(20000L);
	if ((fin = _open(src, O_RDONLY)) != -1) {
		fout = _creat(dst, 0);
		while ((n = _read(fin, buf, 20000)) != 0 && n != -1)
			write(fout, buf, n);
		close(fin);
		close(fout);
	}
	mem_free(buf);
}

signed short savegame_read_hero(signed short slot, register signed short id, struct hero far *buf)
{
	signed short n;

	if (id == FILE_HEROES) {
		*(char far * far *)g_names_star_idx = file_getname(g_file_main, FILE_HEROES);
		id = 0;
	}
	if (ofile_open(g_ofile, id) != -1) {
		ofile_seek(g_ofile, (signed long)slot * sizeof(struct hero), SEEK_SET);
		n = ofile_read(g_ofile, buf, (signed long)sizeof(struct hero));
		ofile_close(g_ofile);
	} else
		n = 0;
	return n;
}

void herofile_remove(signed short id, signed short slot)
{
	char far *name;
	char far *tmp;
	signed short fout;
	signed short fin;
	void far *buf;
	signed short n;
	register signed short i;
	register signed short done;

	name = (char far *)mem_alloc(100L);
	tmp = (char far *)mem_alloc(100L);
	buf = mem_alloc((signed long)sizeof(struct hero));
	sprintf(name, g_path_fmts[0], file_getname(g_file_main, id));
	strcpy(tmp, "t.$$$");
	herofile_copy(name, tmp);
	unlink(name);
	fout = creat(name, S_IREAD | S_IWRITE);
	fin = _open("t.$$$", O_RDWR);
	done = 0;
	i = 0;
	do {
		n = _read(fin, buf, sizeof(struct hero));
		if (n != sizeof(struct hero))
			done = 1;
		else if (slot != i)
			_write(fout, buf, sizeof(struct hero));
		i++;
	} while (!done);
	close(fin);
	close(fout);
	unlink("t.$$$");
	if (id == FILE_HEROES_ALL) {
		sprintf(name, g_path_fmts[0], file_getname(g_file_main, id));
		sprintf(tmp, g_path_fmts[1], file_getname(g_file_main, id));
		herofile_copy(name, tmp);
	}
	mem_free(buf);
	mem_free(name);
	mem_free(tmp);
}

void hero_delete_chr(char far *name)
{
	signed short slot;

	if (herofile_find(name, &slot, FILE_HEROES))
		herofile_remove(FILE_HEROES, slot);
	if (herofile_find(name, &slot, FILE_HEROES_ALL))
		herofile_remove(FILE_HEROES_ALL, slot);
}

signed short herofile_find(char far *name, signed short far *slot, signed short id)
{
	struct hero far *buf;
	register signed short done;
	register signed short n;

	done = 0;
	buf = (struct hero far *)mem_alloc((signed long)sizeof(struct hero));
	*slot = 0;
	while (!done) {
		n = savegame_read_hero(*slot, id, buf);
		if (n != sizeof(struct hero)) {
			done = 1;
			*slot = -1;
		}
		if (!done) {
			if (!strcmp(name, buf->file_name))
				done = 1;
			else
				(*slot)++;
		}
	}
	mem_free(buf);
	if (*slot != -1)
		return 1;
	else
		return 0;
}

void herofile_write(register signed short slot, void far *buf, register signed short id)
{
	signed short fd;
	char name[40];

	if (id == FILE_HEROES) {
		if (ofile_create(g_ofile, id, O_RDWR | O_BINARY) != -1) {
			ofile_seek(g_ofile, (signed long)slot * sizeof(struct hero), SEEK_SET);
			ofile_write(g_ofile, buf, (signed long)sizeof(struct hero));
			ofile_close(g_ofile);
		}
	} else {
		if (ofile_create(g_ofile, id, O_RDWR | O_BINARY) != -1) {
			ofile_seek(g_ofile, (signed long)slot * sizeof(struct hero), SEEK_SET);
			ofile_write(g_ofile, buf, (signed long)sizeof(struct hero));
			ofile_close(g_ofile);
		}
		sprintf(name, g_path_fmts[1], file_getname(g_file_main, FILE_HEROES_ALL));
		fd = _open(name, O_RDWR);
		lseek(fd, (signed long)slot * sizeof(struct hero), SEEK_SET);
		_write(fd, buf, sizeof(struct hero));
		close(fd);
	}
}

void herofile_append(void far *buf, register signed short id)
{
	char name[40];
	register signed short fd;

	if (id == FILE_HEROES) {
		sprintf(name, g_path_fmts[0], file_getname(g_file_main, id));
		fd = open(name, O_BINARY | O_CREAT | O_RDWR);
		close(fd);
		ofile_create(g_ofile, id, O_RDWR | O_BINARY);
		ofile_seek(g_ofile, 0L, SEEK_END);
		ofile_write(g_ofile, buf, (signed long)sizeof(struct hero));
		ofile_close(g_ofile);
	} else {
		if (ofile_open(g_ofile, id) == -1) {
			sprintf(name, g_path_fmts[0], file_getname(g_file_main, id));
			fd = creat(name, S_IREAD | S_IWRITE);
			close(fd);
			sprintf(name, g_path_fmts[1], file_getname(g_file_main, id));
			fd = creat(name, S_IREAD | S_IWRITE);
			close(fd);
		} else
			ofile_close(g_ofile);
		ofile_create(g_ofile, id, O_RDWR | O_BINARY);
		ofile_seek(g_ofile, 0L, SEEK_END);
		ofile_write(g_ofile, buf, (signed long)sizeof(struct hero));
		ofile_close(g_ofile);
		sprintf(name, g_path_fmts[1], file_getname(g_file_main, id));
		fd = _open(name, O_RDWR);
		lseek(fd, 0L, SEEK_END);
		_write(fd, buf, sizeof(struct hero));
		close(fd);
	}
}

void herofile_save(struct hero far *h, register signed short id)
{
	signed short slot;

	herofile_find(h->file_name, &slot, id);
	if (slot != -1)
		herofile_write(slot, h, id);
	else
		herofile_append(h, id);
}

signed short hero_load_chr(signed short id, signed short slot, register signed short i, signed short group)
{
	struct hero far *h;

	h = &g_heroes[i];
	savegame_read_hero(slot, id, h);
	h->set_group_no(group);
	startup_equipment(h, i);
	return 1;
}

void hero_save_file(struct hero far *h)
{
	herofile_save(h, FILE_HEROES);
}
