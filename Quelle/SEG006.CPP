/* SEG006 - Archiv- und Dateizugriff: Archivobjekt (0x5c Bytes) fuer das Datenarchiv mit Verzeichnis
 * (Eintraege zu 20 Bytes: Flag, Name[14], Offset), Dateinamentabelle bzw. Namensdatei (14 Bytes je Name),
 * Bitfeld "Datei liegt einzeln vor", Ausweichpfade (sprintf-Formate g_fmt_cd_path "X:\\DATA\\%s" / g_fmt_temp_path "..\\CTEMP\\%s"), XMS-Zwischenspeicher
 * (128-KB-Bloecke, Inhaltsverzeichnis Offset/Laenge je Datei, Bit 31 = temporaer seit arc_mark).
 * Handle: 0x8000|id = Datei im XMS, 0x4000|fd = Datei im Archiv (Start/Ende/Rest), sonst DOS-Handle.
 * (seg006 in SCHWEIF.EXE, 6873 Bytes, 27 Funktionen)
 *
 * C++! BCC -mlarge -3 -Ot -Og -c -Y SEG006.CPP   -> 6873/6873 Bytes GLEICH (18.10.2026)
 * (gleichwertig: zusaetzlich -Oc; alle anderen der 256 Kombinationen aus -O -Oe -Ol -Oc -Og -Ov -Z -Oi
 *  nicht gleich; -1/-2 scheiden wegen 386-Befehlen aus)
 *
 * Schluessel-Erkenntnisse:
 * - handle ist unsigned: nur dann rechnet BCC bei &a->tab[a->handle & 0x7fff] erst den Index und laedt
 *   danach den Tabellenzeiger (mit signed umgekehrt).
 * - arc_open: [bp-10h]/[bp-14h] sind die Parameterkopien der inline-Memberfunktion arc_entry::set(off, len)
 *   (speichert len vor off); "len = size = arc_size(a)" (size zuerst gespeichert);
 *   Handle-Wert "(signed short)0x8000 | id" (nur mit vorzeichenbehafteter Konstante wird der Wert vor dem
 *   les berechnet).
 * - arc_release: "mov ax,1 / xor ax,ax / or ax,ax" kommt von der inline-Memberfunktion arc_entry::is_temp()
 *   (ein ?: direkt im if ergibt die Byte-Form mov al,1); [bp-2] ist der -Og-Zwischenwert a->save_blk.
 * - arc_delete: flags als "register"-Local (Kopie des Parameters), i ausdruecklich NICHT register -> SI/DI
 *   wie im Original (ein echter C++-Destruktor mit register i vertauscht SI/DI ebenso).
 * - arc_info: i/n landen per -Og in DX/CX; "i = n = 0".
 * - arc_read_raw: Locals des XMS-Zweigs auf Funktionsebene, die des Archivzweigs (nr, want, got, dst2) im
 *   inneren Block hinter dem -Og-Zwischenwert (long)nr [bp-26h]; Zieladresse des 2-Byte-Puffers als
 *   (unsigned long)(void far *)tmp (shl/shrd-Form).
 * - Zuweisung und Vergleich getrennt ("fd = open(); if (fd < 0)") wo das Original das Register testet
 *   (or di,di), in einem Ausdruck wo es ax testet (cmp ax,-1).
 * - Tote Epiloge/Spruenge = if/else mit return in beiden Zweigen; arc_open_raw/arc_open_mode enden mit
 *   unerreichbarem "return -1".
 * - Aufrufe weiter unten liegender Funktionen als far call: Reihenfolge der Funktionen wie im Original.
 * - Objektparameter wie in s_proto.h als void far * (Aufrufer uebergeben g_file_3 = Archivobjekt), Zugriff
 *   ueber den Cast-Makro A (kein eigener Zeiger - gleicher Code); struct arc/arc_entry/arc_stats in s_struct.h.
 */
#include <io.h>
#include <mem.h>
#include <alloc.h>
#include <string.h>
#include <stdio.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5038..5048, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

#define A	((struct arc far *)arc)
#define OUT	((struct arc_stats far *)out)

/* R06_0009 */
void arc_init(void far *arc, char far * far *names, signed short nfiles, signed short nblocks)
{
	A->buf = 0;
	A->nblk = 0;
	A->xpos = 0;
	A->handle = 0;
	A->names = names;
	A->nfiles = nfiles;
	A->nblocks = nblocks;
	A->xms = 0;
	A->mark = 0;
	if (xms_present() && xms_free_kb() >= 0x80)
		A->xms = 1;
	if (A->xms) {
		A->tab = (struct arc_entry far *)mem_alloc_w(A->nfiles << 3);
		memset(A->tab, 0, A->nfiles << 3);
		A->blocks = (signed short far *)mem_alloc_w(A->nblocks << 1);
		memset(A->blocks, 0, A->nblocks << 1);
		A->blocks[0] = xms_alloc_kb(0x80);
		A->buf = farmalloc(0x1000);
	}
}

/* R06_0137 */
void far *arc_new0(void far *arc, char far * far *names, signed short nfiles, signed short nblocks)
{
	if (arc || (arc = mem_alloc_w(sizeof(struct arc))))
		arc_init(arc, names, nfiles, nblocks);
	return arc;
}

/* R06_0178 */
void far *arc_new(void far *arc, void far *arcname, void far *bitmap, void far *names,
			void far *dir, signed short nfiles, signed short nblocks)
{
	char path[80];
	register signed short fd;

	if (arc || (arc = mem_alloc_w(sizeof(struct arc)))) {
		A->arcname = (char far *)arcname;
		A->bitmap = (char far *)bitmap;
		A->dir = (char far *)dir;
		A->have_arc = 1;
		if ((fd = open(A->arcname, 0x8001)) != -1) {
			close(fd);
		} else {
			sprintf(path, g_fmt_cd_path, A->arcname);
			if ((fd = open(path, 0x8001)) != -1)
				A->have_arc = 1;
			else
				A->have_arc = 0;
		}
		arc_init(arc, (char far * far *)names, nfiles, nblocks);
	}
	return arc;
}

/* R06_0262 */
void arc_delete(void far *arc, signed short flags_arg)
{
	register signed short flags = flags_arg;
	signed short i;

	if (arc) {
		if (A->xms) {
			for (i = 0; i < A->nblocks; i++)
				if (A->blocks[i])
					xms_free_block(A->blocks[i]);
			mem_free_v(A->tab);
			mem_free_v(A->blocks);
			if (A->xms)
				farfree(A->buf);
		}
		if (flags & 1)
			mem_free_v(arc);
	}
}

/* R06_0312 */
void arc_info(void far *arc, void far *out)
{
	signed short i, n;

	if (A->xms) {
		i = n = 0;
		for (; i < A->nblocks; i++)
			if (A->blocks[i])
				n++;
		OUT->total = (signed long)A->nblocks << 17;
		OUT->used = ((signed long)(n - 1) << 17) + A->xpos;
	} else {
		OUT->total = OUT->used = 0;
	}
	OUT->xms = A->xms;
}

/* R06_0398 */
signed short arc_open(void far *arc, register signed short id)
{
	signed long size;
	signed long len;
	struct arc_entry far *e;

	if (A->handle == 0) {
		if (A->xms) {
			A->pos = 0;
			e = &A->tab[id];
			if (A->buf && (e->off | e->len)) {
				return A->handle = (signed short)0x8000 | id;
			} else {
				if ((A->handle = arc_open_raw(arc, id)) != -1) {
					len = size = arc_size(arc);
					if (xms_alloc(arc, size)) {
						xms_store(arc, size);
						if (A->mark)
							len |= 0x80000000;
						e->set(A->xpos, len);
						if (size & 1)
							size++;
						A->xpos += size;
						close(A->handle & 0xbfff);
						return A->handle = (signed short)0x8000 | id;
					} else {
						return A->handle;
					}
				} else {
					A->handle = 0;
					return -1;
				}
			}
		} else {
			signed short fd;
			if ((fd = arc_open_raw(arc, id)) == -1) {
				A->handle = 0;
				return -1;
			} else {
				return A->handle = fd;
			}
		}
	} else {
		return -1;
	}
}

/* R06_054f */
signed long arc_load(void far *arc, signed short id, void far *buf)
{
	signed long n;

	if ((A->handle = arc_open(arc, id)) != -1) {
		n = arc_read_all(arc, buf);
		arc_close(arc);
		return n;
	} else {
		A->handle = 0;
		return 0;
	}
}

/* R06_05c2 */
signed short xms_alloc(void far *arc, signed long size)
{
	signed long h;
	signed short start;
	signed long rest;
	register signed short blk, n;

	if (A->xms) {
		if (size & 1)
			size++;
		blk = start = A->nblk;
		rest = 0x20000L - (A->xpos & 0x1ffffL);
		if (rest)
			size -= rest;
		if (size > 0) {
			n = size / 0x20000L + 1;
			if (xms_free_kb() > (n << 7)) {
				while (n-- && (h = xms_alloc_kb(0x80)) >= 0)
					A->blocks[++blk] = h;
				if (n == -1 && h >= 0) {
					return 1;
				} else {
					while (start < blk)
						if (A->blocks[blk] >= 0)
							xms_free_block(A->blocks[blk--]);
					return 0;
				}
			} else {
				return 0;
			}
		} else {
			return 1;
		}
	} else {
		return 0;
	}
}

/* R06_06f0 */
void xms_store(void far *arc, signed long size)
{
	signed long off;
	signed long rest;
	register signed short h;

	off = A->xpos & 0x1ffff;
	rest = 0x20000 - off;
	while (size > 0) {
		h = A->blocks[A->nblk];
		size -= xms_write(arc, A->buf, rest, h, off);
		if (size > 0) {
			off = 0;
			A->nblk++;
			rest = 0x20000;
		}
	}
}

/* R06_0789 */
signed long arc_read_all(void far *arc, void far *buf)
{
	signed long len;
	struct arc_entry far *e;

	if (A->handle & 0x8000) {
		e = &A->tab[A->handle & 0x7fff];
		len = e->len & 0x7fffffff;
	} else {
		len = arc_size(arc);
	}
	return arc_read_raw(arc, buf, len - A->pos);
}

/* R06_080f */
signed long arc_read(void far *arc, void far *buf, signed long n)
{
	signed long len;
	struct arc_entry far *e;
	signed long rest;

	if (A->handle & 0x8000) {
		e = &A->tab[A->handle & 0x7fff];
		len = e->len & 0x7fffffff;
	} else {
		len = arc_size(arc);
	}
	if ((rest = len - A->pos) < n)
		n = rest;
	return arc_read_raw(arc, buf, n);
}

/* R06_08a9 */
signed long arc_read_raw(void far *arc, void far *buf, signed long n)
{
	struct arc_entry far *e;
	char huge *dst;
	signed short h;
	signed long off;
	signed long rest;
	signed long pos;
	signed long cnt;
	signed long total;
	char tmp[2];
	signed short odd;
	register signed short blk;

	if (A->handle & 0x8000) {
		e = &A->tab[A->handle & 0x7fff];
		pos = e->off;
		pos += A->pos;
		dst = (char huge *)buf;
		total = n;
		blk = pos / 0x20000;
		off = pos & 0x1ffff;
		rest = 0x20000 - off;
		while (n > 0) {
			h = A->blocks[blk];
			if (off & 1) {
				g_xms_move.len = 2;
				g_xms_move.src_handle = h;
				g_xms_move.src = off - 1;
				g_xms_move.dst_handle = 0;
				g_xms_move.dst = (unsigned long)(void far *)tmp;
				xms_move_any(&g_xms_move);
				*dst = tmp[1];
				dst++;
				off++;
				rest--;
				n--;
				A->pos++;
				if (rest == 0) {
					blk++;
					off = 0;
					rest = 0x20000;
					h = A->blocks[blk];
				}
			}
			cnt = n > rest ? rest : n;
			odd = 0;
			if (cnt & 1) {
				odd = 1;
				cnt--;
			}
			g_xms_move.len = cnt;
			g_xms_move.src_handle = h;
			g_xms_move.src = off;
			g_xms_move.dst_handle = 0;
			g_xms_move.dst = (signed long)dst;
			xms_move_any(&g_xms_move);
			dst += cnt;
			if (odd == 1) {
				g_xms_move.len = 2;
				g_xms_move.src_handle = h;
				g_xms_move.src = off + cnt;
				g_xms_move.dst_handle = 0;
				g_xms_move.dst = (unsigned long)(void far *)tmp;
				xms_move_any(&g_xms_move);
				*dst = tmp[0];
				dst++;
				cnt++;
			}
			n -= cnt;
			A->pos += cnt;
			blk++;
			off = 0;
			rest = 0x20000;
		}
		return total;
	} else if (A->handle & 0x4000) {
		register unsigned short nr;
		signed long want;
		signed long got;
		char huge *dst2;

		got = 0;
		dst2 = (char huge *)buf;
		want = n;
		if (A->remain) {
			if (A->remain < want)
				want = A->remain;
		} else {
			want = 0;
		}
		if (want) {
			do {
				nr = want > 0xfa00 ? 0xfa00 : want;
				nr = _read(A->handle & 0xbfff, dst2, nr);
				A->remain -= nr;
				dst2 += nr;
				got += nr;
				want -= nr;
			} while (nr && nr != -1);
		}
		return got;
	} else {
		return read_big(A->handle, buf, n);
	}
}

/* R06_0c16 */
signed short arc_close(void far *arc)
{
	register signed short h = A->handle;

	A->handle = 0;
	if (h & 0x8000)
		return 1;
	else
		return close(h & 0xbfff);
}

/* R06_0c4d */
signed long arc_size(void far *arc)
{
	struct arc_entry far *e;

	if (A->handle & 0x8000) {
		e = &A->tab[A->handle & 0x7fff];
		return e->len & 0x7fffffff;
	} else if (A->handle == -1) {
		return 0;
	} else if (A->handle & 0x4000) {
		return A->len;
	} else {
		return filelength(A->handle);
	}
}

/* R06_0cda */
signed long arc_file_size(void far *arc, signed short id)
{
	signed long n;

	arc_open(arc, id);
	n = arc_size(arc);
	arc_close(arc);
	return n;
}

/* R06_0d24 */
signed long arc_seek(void far *arc, signed long off, register signed short whence)
{
	struct arc_entry far *e;
	signed long len;
	signed long pos;

	if (A->handle & 0x8000) {
		if (whence == 0) {
			A->pos = off;
		} else if (whence == 1) {
			A->pos += off;
		} else {
			e = &A->tab[A->handle & 0x7fff];
			len = e->len & 0x7fffffff;
			A->pos = len + off;
		}
		return A->pos;
	} else if (A->handle & 0x4000) {
		if (whence == 2) {
			A->remain = -off;
			pos = A->end + off;
			return lseek(A->handle & 0xbfff, pos, 0) - A->start;
		} else if (whence == 1) {
			A->remain -= off;
			pos = A->end - A->remain;
			return lseek(A->handle & 0xbfff, pos, 0) - A->start;
		} else {
			A->remain = A->len - off;
			pos = A->start + off;
			return lseek(A->handle & 0xbfff, pos, 0) - A->start;
		}
	} else {
		return lseek(A->handle, off, whence);
	}
}

/* R06_0ee6 */
signed long xms_write(void far *arc, void far *buf, signed long n, signed short h, signed long off)
{
	register unsigned short got = 0, want = 0;
	signed long total = 0;

	while (n > 0 && want == got) {
		want = n > 0x1000 ? 0x1000 : n;
		got = arc_read_raw(arc, buf, (unsigned long)want);
		g_xms_move.len = (got & 1) ? got + 1 : got;
		g_xms_move.src_handle = 0;
		g_xms_move.src = (signed long)buf;
		g_xms_move.dst_handle = h;
		g_xms_move.dst = off;
		xms_move_any(&g_xms_move);
		n -= got;
		total += got;
		off += got;
	}
	return total;
}

/* R06_0fa4 */
void arc_mark(void far *arc)
{
	if (A->handle)
		arc_close(arc);
	A->mark = 1;
	if (A->xms) {
		A->save_blk = A->nblk;
		A->save_pos = A->pos;
		A->save_xpos = A->xpos;
	}
}

/* R06_1000 */
void arc_release(void far *arc)
{
	register signed short i, j;

	if (A->handle)
		arc_close(arc);
	if (A->xms) {
		for (i = 0; i < A->nfiles; i++)
			if (A->tab[i].is_temp()) {
				A->tab[i].len = 0;
				A->tab[i].off = 0;
			}
		for (j = A->nblk; A->save_blk < j; )
			xms_free_block(A->blocks[j--]);
		A->nblk = A->save_blk;
		A->pos = A->save_pos;
		A->xpos = A->save_xpos;
	}
	A->mark = 0;
}

/* R06_10f1 */
char far *arc_name(void far *arc, register signed short id)
{
	register signed short fd;
	signed long pos;

	if (id == 0 || A->have_arc == 0) {
		return A->names[id];
	} else {
		fd = open(A->dir, 0x8001);
		if (fd == -1) {
			return 0;
		} else {
			pos = (signed long)id * 14;
			lseek(fd, pos, 0);
			_read(fd, A->name, 14);
			close(fd);
			return A->name;
		}
	}
}

/* R06_1199 */
signed short arc_find(void far *arc, char far *name)
{
	register signed short i, found = 0;
	signed short idx;
	signed short fd;
	signed long len;
	char far *buf;
	char far *p;

	if (A->have_arc == 0) {
		for (i = 0; i < A->nfiles; i++)
			if (!strcmp(A->names[i], name)) {
				found = 1;
				idx = i;
				break;
			}
	} else {
		fd = open(A->dir, 0x8001);
		if (fd == -1) {
			return -1;
		} else {
			len = filelength(fd);
			if ((buf = (char far *)farcalloc(len, 1)) == 0) {
				for (i = 0; i < A->nfiles; i++) {
					_read(fd, A->name, 14);
					if (!strcmp(A->name, name)) {
						found = 1;
						idx = i;
						break;
					}
				}
			} else {
				p = buf;
				_read(fd, buf, len);
				for (i = 0; i < A->nfiles; i++) {
					if (!strcmp(p, name)) {
						found = 1;
						idx = i;
						break;
					}
					p += 14;
				}
				farfree(buf);
			}
			close(fd);
		}
	}
	if (found)
		return idx;
	else
		return -1;
}

/* R06_1312 */
signed short arc_is_set(void far *arc, register signed short id)
{
	signed short byte;
	signed short bit;
	signed short fd;
	signed char c;

	if (id) {
		if ((fd = _open("STAR.DAT", 0x8001)) == -1)
			return 0;
		else
			close(fd);
		byte = id / 8;
		bit = id % 8;
		c = A->bitmap[byte];
		return c & (1 << bit);
	} else {
		return 1;
	}
}

/* R06_138c */
signed short arc_open_raw(void far *arc, signed short id)
{
	char path[80];
	char name[14];
	register signed short fd, fd2;

	strcpy(name, arc_name(arc, id));
	if (arc_is_set(arc, id)) {
		fd = open(name, 0x8001);
		return fd;
	}
	sprintf(path, g_fmt_temp_path, name);
	if ((fd = open(path, 0x8001)) != -1)
		return fd;
	fd2 = open(A->arcname, 0x8001);
	if (fd2 < 0) {
		if ((fd = open(name, 0x8001)) != -1) {
			return fd;
		} else {
			sprintf(path, g_fmt_cd_path, A->arcname);
			fd2 = open(path, 0x8001);
			if (fd2 < 0) {
				sprintf(path, g_fmt_cd_path, name);
				fd = open(path, 0x8001);
				return fd;
			} else {
				close(fd2);
				return arc_open_dir(arc, id);
			}
		}
	} else {
		close(fd2);
		return arc_open_dir(arc, id);
	}
	return -1;
}

/* R06_1508 */
signed short arc_open_mode(void far *arc, register signed short id, register signed short mode)
{
	if ((mode & 2) || (mode & 4) || (mode & 0x100))
		return open(arc_name(arc, id), mode, 0x180);
	else
		return arc_open_raw(arc, id);
	return -1;
}

/* R06_1564 */
signed short arc_exists(void far *arc, signed short id)
{
	signed short cnt;
	signed long pos;
	struct arc_entry far *e;
	char path[80];
	char name[14];
	register signed short fd, found = 0;

	strcpy(name, arc_name(arc, id));
	if (A->nfiles < (id & 0x7fff))
		return 0;
	if (strstr(name, "DUMMY"))
		return 0;
	if (A->xms) {
		e = &A->tab[id];
		if (A->buf && (e->off | e->len))
			return 1;
	}
	if ((fd = open(name, 0x8001)) != -1) {
		found = 1;
		if (filelength(fd) == 1)
			found = 0;
		close(fd);
	} else {
		sprintf(path, g_fmt_temp_path, name);
		if ((fd = open(path, 0x8001)) != -1) {
			found = 1;
			close(fd);
		}
	}
	if (!found && A->have_arc) {
		fd = open(A->arcname, 0x8001);
		if (fd != -1) {
			pos = 2;
			pos += (signed long)id * 20;
			lseek(fd, pos, 0);
			_read(fd, &cnt, 2);
			if (cnt)
				found = 1;
			else
				found = 0;
			close(fd);
		}
		if (!found) {
			sprintf(path, g_fmt_cd_path, A->arcname);
			fd = open(path, 0x8001);
			if (fd != -1) {
				pos = 2;
				pos += (signed long)id * 20;
				lseek(fd, pos, 0);
				_read(fd, &cnt, 2);
				if (cnt)
					found = 1;
				else
					found = 0;
				close(fd);
			} else {
				found = 0;
			}
		}
	}
	return found;
}

/* R06_1790 */
signed short arc_open_dir(void far *arc, register signed short id)
{
	signed short n;
	signed short flag;
	signed short dummy;
	signed long off;
	signed long pos;
	signed long base;
	signed long end;
	char path[80];
	register signed short fd;

	fd = open(A->arcname, 0x8001);
	if (fd < 0) {
		sprintf(path, g_fmt_cd_path, A->arcname);
		fd = open(path, 0x8001);
	}
	if (fd != -1) {
		_read(fd, &n, 2);
		base = 2;
		base += (signed long)n * 20;
		pos = 2;
		pos += (signed long)id * 20;
		lseek(fd, pos, 0);
		_read(fd, &flag, 2);
		_read(fd, A->name, 14);
		_read(fd, &off, 4);
		if (n - 1 != id) {
			_read(fd, &dummy, 2);
			_read(fd, A->name, 14);
			_read(fd, &end, 4);
			end += base;
		} else {
			end = filelength(fd);
		}
		off += base;
		lseek(fd, off, 0);
		A->start = off;
		A->end = end;
		A->len = A->remain = end - off;
		if (flag == 0) {
			close(fd);
			sprintf(path, g_fmt_cd_path, arc_name(arc, id));
			fd = open(path, 0x8001);
			if (fd < 0) {
				sprintf(path, g_fmt_cd_path, A->arcname);
				fd = open(path, 0x8001);
				_read(fd, &n, 2);
				base = 2;
				base += (signed long)n * 20;
				pos = 2;
				pos += (signed long)id * 20;
				lseek(fd, pos, 0);
				_read(fd, &flag, 2);
				_read(fd, A->name, 14);
				_read(fd, &off, 4);
				if (n - 1 != id) {
					_read(fd, &dummy, 2);
					_read(fd, A->name, 14);
					_read(fd, &end, 4);
					end += base;
				} else {
					end = filelength(fd);
				}
				off += base;
				lseek(fd, off, 0);
				A->start = off;
				A->end = end;
				A->len = A->remain = end - off;
				if (flag == 0) {
					close(fd);
					fd = -1;
				} else {
					fd |= 0x4000;
				}
			}
		} else {
			fd |= 0x4000;
		}
	}
	return fd;
}

/* R06_1add */
void arc_dummy(void)
{
}
