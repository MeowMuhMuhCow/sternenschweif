/* OVL037 - Overlay O037 (Segment 0x1fd1 in SCHWEIF.EXE): Dateiklasse File (0x38 Bytes, s_class.h; z. B. g_file_main
 * ds:35c6, ds:3796, Sound-Dateien in SEG004) fuer Spieldateien, die einzeln oder in einem Archiv liegen. Felder:
 * Namenstabelle names, Handle fd (Bit FILE_FD_ARC = Archiveintrag), Archivname arc_name, Bitfeld loose ("liegt
 * einzeln vor"), Namenspuffer name_buf, Archiv vorhanden have_arc, Groesse size, Rest left, Beginn start, Ende end,
 * Anzahl count, Namensliste name_list.
 * Archivformat: Wort n, n Eintraege zu 20 Bytes (Wort Flag, Name[14], long Versatz ab 2 + n * 20).
 * Konstruktoren (nur Namen / mit Archiv: Archiv direkt oder ueber das Pfadformat g_file_names[4] suchen),
 * Destruktor. open(id) / open(id, mode) (g_file_last_id = id), load(id, buf), read_all/read: lesen (im Archiv in
 * Bloecken zu 0xfa00 per _read auf huge-Zeiger, sonst read_big), write: schreiben (write_big, nicht im Archiv),
 * close, length()/length(id), seek (im Archiv relativ zum Eintrag), create: _creat, getname: Name zu id (Tabelle bzw.
 * Namensliste, 14 Bytes je Name), find: id zu Name (strcmp, Liste per farcalloc oder eintragsweise), is_loose: Bit
 * id im Bitfeld loose (nur wenn g_str_star_dat_o037 existiert), open_handle(id): Datei oeffnen (einzeln, Pfad
 * g_file_names[5], sonst Archiv), open_handle(id, mode): mit Modus (O_WRONLY/O_RDWR/O_CREAT -> open(name, mode,
 * S_IREAD | S_IWRITE)), exists(id): (Name enthaelt g_str_dummy_o037 -> nein, Laenge 1 -> nein, sonst Archiv-Flag),
 * open_in_arc: Archiveintrag oeffnen (Handle | FILE_FD_ARC), file_dummy: leere Funktion.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL037.CPP   -> 4312/4312 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (g_file_names[4]/[5] ohne eigenes Public -> _g_32cb+0x10/+0x14)
 *   (Namensfassung mit schweif.h, Klasse File in s_class.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++-Klasse mit zwei Konstruktoren (this == 0 -> new 0x38) und leerem Destruktor (nur delete bei flags & 1);
 *   globale RTL-Funktionen ::open/::close wegen gleichnamiger Elemente; _read/_open/_creat -> __read usw.
 * - -Og: Epilogkopien; "if (..) return a; else {..}" bzw. "if (..) {..} else return b;" (sonst fehlt die Kopie),
 *   open_in_arc endet mit "} else ;" (toter jmp short).
 * - Lesen: long-Local n ZUERST deklariert und "left -= n = chunk; p += n; total += n = chunk; rest -= n;"
 *   (zwei movzx-Speicherungen nach [bp-4]); p als char huge * -> PADA@.
 * - getname: Handle OHNE register (DI), sonst bekommt es SI vor dem register-Parameter id.
 * - Grosse Locals: path[80] vor nm[14] deklarieren (path liegt naeher an bp).
 * Originalfehler: Konstruktor 2 schliesst das Handle des zweiten open (Pfadformat) nie. Beim Lesen im Archiv
 *   wird ein _read-Fehler (0xffff) erst nach dem Verrechnen geprueft (Rest, Zeiger und Summe um 65535
 *   verfaelscht). exists prueft "count < (id & 0x7fff)" (id == count gilt als gueltig). open_in_arc prueft beim
 *   zweiten Oeffnen des Archivs (ueber g_file_names[4]) das Handle nicht. find liest die Namensliste mit
 *   _read(h, mem, (unsigned)len) (ab 64 KB abgeschnitten).
 */
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <alloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:3e70..3e86, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
unsigned long g_save_count = 1;	/* ds:3e70 je Speichern + 100 (SEG001 savemenu_save), sonst nicht gelesen */
signed short g_file_last_id = -21555;

#define FMT_CD_PATH	g_file_names[4]		/* "X:\DATA\%s" (CD-Laufwerk) */
#define FMT_CTEMP_PATH	g_file_names[5]		/* "..\CTEMP\%s" */

File::File(char far * far *nametab, signed short n)
{
	names = nametab;
	count = n;
	arc_name = 0;
	loose = 0;
	fd = 0;
	have_arc = 0;
}

File::File(char far *arc, unsigned char far *bits, char far * far *nametab, char far *list, signed short n)
{
	char path[80];
	register signed short h;

	arc_name = arc;
	loose = bits;
	names = nametab;
	count = n;
	name_list = list;
	have_arc = 1;
	if ((h = ::open(arc_name, O_RDONLY | O_BINARY)) != -1)
		::close(h);
	else {
		sprintf(path, FMT_CD_PATH, arc_name);
#if FIX_0091
		/* Fix #91: zweites Handle wieder schliessen */
		if ((h = ::open(path, O_RDONLY | O_BINARY)) != -1) {
			::close(h);
			have_arc = 1;
		} else
#else
		if ((h = ::open(path, O_RDONLY | O_BINARY)) != -1)
			have_arc = 1;
		else
#endif
			have_arc = 0;
	}
	h = 0;
}

File::~File()
{
}

signed short File::open(register signed short id)
{
	g_file_last_id = id;
	if (fd == 0) {
		if ((fd = open_handle(id)) == -1) {
			fd = 0;
			return -1;
		} else
			return fd;
	} else
		return -1;
}

signed short File::open(register signed short id, signed short mode)
{
	g_file_last_id = id;
	if (fd == 0) {
		if ((fd = open_handle(id, mode)) == -1) {
			fd = 0;
			return -1;
		} else
			return fd;
	} else
		return -1;
}

signed long File::load(register signed short id, void huge *buf)
{
	signed long n;

	g_file_last_id = id;
	if (fd == 0) {
		if ((fd = open(id)) != -1) {
			n = read_all(buf);
			close();
			return n;
		} else
			fd = 0;
	}
	return 0;
}

signed long File::read_all(void huge *buf)
{
	signed long n;
	signed long rest;
	signed long total;
	char huge *p;
	register unsigned short chunk;

	if (fd & FILE_FD_ARC) {
		total = 0;
		p = (char huge *)buf;
		rest = size;
		if (left != 0) {
			if (left < rest)
				rest = left;
		} else
			rest = 0;
		if (rest != 0)
			do {
				chunk = rest > FILE_READ_CHUNK ? FILE_READ_CHUNK : rest;
#if FIX_0091
				/* Fix #91: Lesefehler vor dem Verrechnen abfangen */
				chunk = _read(fd & ~FILE_FD_ARC, p, chunk);
				if (chunk == 0xffff)
					break;
#else
				chunk = _read(fd & ~FILE_FD_ARC, p, chunk);
#endif
				left -= n = chunk;
				p += n;
				total += n = chunk;
				rest -= n;
			} while (chunk != 0 && chunk != 0xffff);
		return total;
	} else
		return read_big(fd, buf, filelength(fd));
}

signed long File::read(void huge *buf, signed long len)
{
	signed long n;
	signed long rest;
	signed long total;
	char huge *p;
	register unsigned short chunk;

	if (fd & FILE_FD_ARC) {
		total = 0;
		p = (char huge *)buf;
		rest = len;
		if (left != 0) {
			if (left < rest)
				rest = left;
		} else
			rest = 0;
		if (rest != 0)
			do {
				chunk = rest > FILE_READ_CHUNK ? FILE_READ_CHUNK : rest;
#if FIX_0091
				/* Fix #91: Lesefehler vor dem Verrechnen abfangen */
				chunk = _read(fd & ~FILE_FD_ARC, p, chunk);
				if (chunk == 0xffff)
					break;
#else
				chunk = _read(fd & ~FILE_FD_ARC, p, chunk);
#endif
				left -= n = chunk;
				p += n;
				total += n = chunk;
				rest -= n;
			} while (chunk != 0 && chunk != 0xffff);
		return total;
	} else
		return read_big(fd, buf, len);
}

signed long File::write(void huge *buf, signed long len)
{
	if (fd & FILE_FD_ARC)
		return 0;
	else
		return write_big(fd, buf, len);
}

signed short File::close()
{
	::close(fd & ~FILE_FD_ARC);
	fd = 0;
	return fd;
}

signed long File::length()
{
	if (fd == -1)
		return 0;
	else if (fd & FILE_FD_ARC)
		return size;
	else
		return filelength(fd);
}

signed long File::length(signed short id)
{
	signed long n;

	open(id);
	n = length();
	close();
	return n;
}

signed long File::seek(signed long off, register signed short whence)
{
	signed long pos;

	if (fd & FILE_FD_ARC) {
		if (whence == SEEK_END) {
			left = -off;
			pos = end + off;
			return lseek(fd & ~FILE_FD_ARC, pos, SEEK_SET) - start;
		} else if (whence == SEEK_CUR) {
			left -= off;
			pos = end - left;
			return lseek(fd & ~FILE_FD_ARC, pos, SEEK_SET) - start;
		} else {
			left = size - off;
			pos = start + off;
			return lseek(fd & ~FILE_FD_ARC, pos, SEEK_SET) - start;
		}
	} else
		return lseek(fd, off, whence);
}

signed short File::create(signed short id, signed short attr)
{
	fd = _creat(getname(id), attr);
	return fd;
}

char far *File::getname(register signed short id)
{
	signed short n;
	signed long pos;
	signed short h;

	if (id == 0 || have_arc == 0)
		return names[id];
	else {
		h = ::open(name_list, O_RDONLY | O_BINARY);
		pos = id * (signed long)FILE_NAME_LEN;
		lseek(h, pos, SEEK_SET);
		n = _read(h, name_buf, FILE_NAME_LEN);
		::close(h);
		return name_buf;
	}
}

signed short File::find(char far *s)
{
	signed short idx;
	signed short h;
	signed long len;
	char far *mem;
	char far *p;
	register signed short i;
	register signed short found;

	found = 0;
	if (have_arc == 0) {
		for (i = 0; i < count; i++)
			if (strcmp(names[i], s) == 0) {
				found = 1;
				idx = i;
				break;
			}
	} else {
		h = ::open(name_list, O_RDONLY | O_BINARY);
		if (h == -1)
			return -1;
		else {
			len = filelength(h);
			if ((mem = (char far *)farcalloc(len, 1L)) == 0) {
				for (i = 0; i < count; i++) {
					_read(h, name_buf, FILE_NAME_LEN);
					if (strcmp(name_buf, s) == 0) {
						found = 1;
						idx = i;
						break;
					}
				}
			} else {
				p = mem;
				_read(h, mem, len);
				for (i = 0; i < count; i++) {
					if (strcmp(p, s) == 0) {
						found = 1;
						idx = i;
						break;
					}
					p += FILE_NAME_LEN;
				}
				farfree(mem);
			}
			::close(h);
		}
	}
	if (found)
		return idx;
	else
		return -1;
}

signed short File::is_loose(register signed short id)
{
	signed short q;
	signed short r;
	signed short h;
	signed char c;

	if (id != 0) {
		if ((h = _open("STAR.DAT", O_RDONLY | O_BINARY)) == -1)
			return 0;
		else
			::close(h);
		q = id / 8;
		r = id % 8;
		c = loose[q];
		return c & (1 << r);
	} else
		return 1;
}

signed short File::open_handle(signed short id)
{
	char path[80];
	char nm[FILE_NAME_LEN];
	register signed short h;
	register signed short h2;

	strcpy(nm, getname(id));
	if (is_loose(id)) {
		h = ::open(nm, O_RDONLY | O_BINARY);
		return h;
	}
	sprintf(path, FMT_CTEMP_PATH, nm);
	if ((h = ::open(path, O_RDONLY | O_BINARY)) != -1)
		return h;
	h2 = ::open(arc_name, O_RDONLY | O_BINARY);
	if (h2 < 0) {
		if ((h = ::open(nm, O_RDONLY | O_BINARY)) != -1)
			return h;
		else {
			sprintf(path, FMT_CD_PATH, arc_name);
			h2 = ::open(path, O_RDONLY | O_BINARY);
			if (h2 < 0) {
				sprintf(path, FMT_CD_PATH, nm);
				h = ::open(path, O_RDONLY | O_BINARY);
				return h;
			} else {
				::close(h2);
				return open_in_arc(id);
			}
		}
	} else {
		::close(h2);
		return open_in_arc(id);
	}
	return -1;
}

signed short File::open_handle(register signed short id, register signed short mode)
{
	if ((mode & O_WRONLY) || (mode & O_RDWR) || (mode & O_CREAT))
		return ::open(getname(id), mode, S_IREAD | S_IWRITE);
	else
		return open_handle(id);
	return -1;
}

signed short File::exists(signed short id)
{
	signed short v;
	signed long pos;
	char path[80];
	char nm[FILE_NAME_LEN];
	register signed short h;
	register signed short found;

	found = 0;
	strcpy(nm, getname(id));
#if FIX_0091
	/* Fix #91: id == count ist ungueltig */
	if (count <= (id & 0x7fff))
#else
	if (count < (id & 0x7fff))
#endif
		return 0;
	if (strstr(nm, "DUMMY") != 0)
		return 0;
	if ((h = ::open(nm, O_RDONLY | O_BINARY)) != -1) {
		found = 1;
		if (filelength(h) == 1)
			found = 0;
		::close(h);
	} else {
		sprintf(path, FMT_CTEMP_PATH, nm);
		if ((h = ::open(path, O_RDONLY | O_BINARY)) != -1) {
			found = 1;
			::close(h);
		}
	}
	if (!found && have_arc) {
		h = ::open(arc_name, O_RDONLY | O_BINARY);
		if (h != -1) {
			pos = 2;
			pos += id * (signed long)ARC_ENTRY_SIZE;
			lseek(h, pos, SEEK_SET);
			_read(h, &v, 2);
			if (v != 0)
				found = 1;
			else
				found = 0;
			::close(h);
		}
		if (!found) {
			sprintf(path, FMT_CD_PATH, arc_name);
			h = ::open(path, O_RDONLY | O_BINARY);
			if (h != -1) {
				pos = 2;
				pos += id * (signed long)ARC_ENTRY_SIZE;
				lseek(h, pos, SEEK_SET);
				_read(h, &v, 2);
				if (v != 0)
					found = 1;
				else
					found = 0;
				::close(h);
			} else
				found = 0;
		}
	}
	return found;
}

signed short File::open_in_arc(register signed short id)
{
	signed short n;
	signed short v;
	signed short v2;
	signed long a;
	signed long pos;
	signed long base;
	signed long e;
	char path[80];
	register signed short h;

	h = ::open(arc_name, O_RDONLY | O_BINARY);
	if (h < 0) {
		sprintf(path, FMT_CD_PATH, arc_name);
		h = ::open(path, O_RDONLY | O_BINARY);
	}
	if (h != -1) {
		_read(h, &n, 2);			/* Kopf: Zahl der Eintraege */
		base = 2;
		base += n * (signed long)ARC_ENTRY_SIZE;
		pos = 2;
		pos += id * (signed long)ARC_ENTRY_SIZE;
		lseek(h, pos, SEEK_SET);
		_read(h, &v, 2);			/* Eintrag id: vorhanden, Name, Versatz */
		_read(h, name_buf, FILE_NAME_LEN);
		_read(h, &a, 4);
		if (n - 1 != id) {			/* Ende = Versatz des naechsten Eintrags */
			_read(h, &v2, 2);
			_read(h, name_buf, FILE_NAME_LEN);
			_read(h, &e, 4);
			e += base;
		} else
			e = filelength(h);
		a += base;
		lseek(h, a, SEEK_SET);
		start = a;
		end = e;
		size = left = e - a;
		if (v == 0) {				/* nicht im Archiv: einzeln vom CD-Pfad */
			::close(h);
			sprintf(path, FMT_CD_PATH, getname(id));
			h = ::open(path, O_RDONLY | O_BINARY);
			if (h < 0) {			/* sonst im Archiv auf der CD */
				sprintf(path, FMT_CD_PATH, arc_name);
#if FIX_0091
				/* Fix #91: zweites Oeffnen des Archivs pruefen */
				h = ::open(path, O_RDONLY | O_BINARY);
				if (h < 0)
					return -1;
				_read(h, &n, 2);
#else
				h = ::open(path, O_RDONLY | O_BINARY);
				_read(h, &n, 2);
#endif
				base = 2;
				base += n * (signed long)ARC_ENTRY_SIZE;
				pos = 2;
				pos += id * (signed long)ARC_ENTRY_SIZE;
				lseek(h, pos, SEEK_SET);
				_read(h, &v, 2);
				_read(h, name_buf, FILE_NAME_LEN);
				_read(h, &a, 4);
				if (n - 1 != id) {
					_read(h, &v2, 2);
					_read(h, name_buf, FILE_NAME_LEN);
					_read(h, &e, 4);
					e += base;
				} else
					e = filelength(h);
				a += base;
				lseek(h, a, SEEK_SET);
				start = a;
				end = e;
				size = left = e - a;
				if (v == 0) {
					::close(h);
					h = -1;
				} else
					h |= FILE_FD_ARC;
			}
		} else
			h |= FILE_FD_ARC;
	} else
		;
	return h;
}

void file_dummy(void)
{
}
