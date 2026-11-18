/* OVL055 - Overlay O055 (Segment 0x204b in SCHWEIF.EXE): Hintergrundanimationen eines Bildes - Konstruktion und
 * Laden fuer das Animationsobjekt Ani (struct ani, Objekt g_screen_pic; Abspielen in O054) aus dem Archiv g_file_3
 * (Gegenstueck zu Bright Eyes load_ani, seg027).
 * Ani()/~Ani(): mode 5, pal_size 0x40, count 0, init / free_all.
 * clear(): Teilanimationen zuruecksetzen (Lage/Groesse/Name[4], Flags je Variante, Phasentabellen, 25 Bildzeiger,
 *   Bmp-Objekte loeschen). pal_black(): Palette schwarz (new pal_size*3 Bytes, set_palette), set_pal(p): Palette p
 *   setzen, set_pos, set_pal_size, halt (mode 5, run = 0), stop (halt und freigeben), play(how)/play_until(how,
 *   code): abspielen (reset) und warten bis stop_at == 0 (game_idle).
 * calc_size(hdr_size): Kopf lesen und den Speicherbedarf berechnen (Grundbild w*h, je Teil Hoehe*Breite*Bilder,
 *   Palette 3*n mit 0xff = 256), im alten Format auch count/areas/nr_changes eintragen.
 * load(id): Datei id oeffnen; ohne "ANI"-Kennung (g_str_ani) altes Format (old_format = 1, calc_size,
 *   Phasentabellen), sonst Version 1 mit Varianten (nr_variants), f2b, Phasen je Variante. Reicht farcoreleft() fuer
 *   das Doppelte des Bedarfs, wird alles nach data geladen (in_mem = 1, Zeiger in den Puffer, PP20-Entpacken
 *   unpack_pp20 mit Verschieben des Rests ueber EMS_norm_ptr, Bmp-Objekte je Teil), sonst nur Kopf, Palette und
 *   Grundbild (Teilbilder spaeter aus der Datei, load_pic in O054). Danach multi und Palette auf 6 Bit (& 0x3f).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL055.CPP   -> 6750/6750 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ohne -Og 141 Bytes laenger, ohne -Oi 183 Bytes kuerzer: strcmp/strncpy/memcpy/memset sonst als Aufrufe).
 * - C++: Klasse Ani (struct ani) mit Konstruktor/Destruktor (this == 0 -> new, flags & 1 -> delete automatisch),
 *   new Bmp(w, h, 0) -> gb_new(0, ..), delete bmp -> gb_delete(p, 3), new char[]/delete -> mem_alloc_w/mem_free_v.
 * - struct ani_area: Name[5] +0, Flags fl[10] +0b, Bildanzahl +15, pics[25] +16, Phasenzahl nr_changes[10] +7a.
 *   Datei-Teil (struct ani_file_area) +0c Versaetze.
 * - load: "if (id_ != -1 && id != id_) { .. }" statt zweier returns (sonst Epilogkopien); die Bloecke
 *   in_mem == 0 / != 0 haben eigene Locals ([bp-6ch].. teilen sich mit den Zeichen-Locals c/c2 der Ladezweige).
 *   size -= c + 1; pos += c + 1 -> -Og-Zwischenwert (long) [bp-66h].
 * - Methoden statt freier Funktionen (this an derselben Stelle); Locals, die Feldnamen verdecken wuerden, heissen
 *   anders (bw/bh/pal_off in calc_size, pal_off in load).
 * - Phasentabelle: "*(chg + (unsigned)((j << 1) << 1))" - nur mit unsigned-Index wird der Byte-Wert VOR der
 *   Zieladresse geladen (push ax / imul / pop ax); signed Index -> Ziel zuerst, Quelle ueber push es / les si.
 * - calc_size: "(signed long)ar_h * ar_w * (signed long)ar_pics" (movzx/imul, add mem,eax); mit unsigned long -> mov edx/add/mov.
 *   "need = (signed long)w * (unsigned long)h" -> movsx/movzx. hp = buf wird nie benutzt (Original).
 * Originalfehler/Auffaelliges: pal_black loescht mit memset nur 4 Bytes des pal_size*3 Bytes grossen Puffers, der Rest
 *   der "schwarzen" Palette ist uninitialisierter Speicher. load: mem_alloc-Ergebnisse ungeprueft; im in_mem == 0
 *   -Zweig wird mem_free(tmp) nicht zugewiesen (haengender Zeiger, harmlos); calc_size legt hp an und benutzt es nie.
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 */
#include <stdio.h>
#include <alloc.h>
#include <mem.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5558..555c, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

/* Teilanimation in der Datei (Kopfbereich), danach die Phasentabelle */
struct ani_file_area {
	char name[4];			/* +0x00 */
	signed short x;			/* +0x04 */
	unsigned char y;		/* +0x06 */
	unsigned char h;		/* +0x07 */
	signed short w;			/* +0x08 */
	unsigned char cyc;		/* +0x0a Flags (ani_flags der Variante 0) */
	signed char pics;		/* +0x0b Zahl der Einzelbilder */
	signed long off[1];		/* +0x0c Versaetze der Einzelbilder, danach Zahl der Phasen (Wort) und Phasen */
};

/* Kopf der Bilddatei (Version 0 bzw. hinter dem "ANI"-Vorspann) */
struct ani_file_hdr {
	signed long main;		/* +0x00 Versatz des Grundbilds */
	signed long pal;		/* +0x04 Versatz der Palette (- 6) */
	signed short w;			/* +0x08 */
	unsigned char h;		/* +0x0a */
	unsigned char count;		/* +0x0b Zahl der Teilanimationen */
	signed long area[1];		/* +0x0c Versaetze der Teilanimationen */
};

Ani::Ani()
{
	mode = 5;
	pal_size = 0x40;
	count = 0;
	init();
}

Ani::~Ani()
{
	free_all();
}

void Ani::clear()
{
	signed short i;
	signed short k;
	signed short j;

	w = 0;
	h = 0;
	pic = 0;
	pal = 0;
	for (i = 0; count > i; i++) {
		areas[i].x = 0;
		areas[i].y = 0;
		areas[i].w = 0;
		areas[i].h = 0;
		areas[i].nr_pics = 0;
		areas[i].name[4] = 0;
		for (j = 0; j < 10; j++) {
			areas[i].fl[j].pingpong = 0;
			areas[i].fl[j].masked = 0;
			areas[i].fl[j].off = 0;
			areas[i].fl[j].b3 = 0;
			areas[i].fl[j].order = 0;
			for (k = 0; areas[i].nr_changes[j] > k; k++) {
				areas[i].changes[k].v[j].pic = 0;
				areas[i].changes[k].v[j].dur = 0;
				areas[i].changes[k].v[j].code = 0;
			}
		}
		for (j = 0; j < 25; j++)
			areas[i].pics[j] = 0;
		if (bmp[i]) {
			delete bmp[i];
			bmp[i] = 0;
		}
	}
}

void Ani::pal_black()
{
	char far *p;

	p = new char[pal_size * 3];
#if FIX_0195
	/* Fix #195: ganze Palette schwarz setzen */
	memset(p, 0, pal_size * 3);
#else
	memset(p, 0, 4);
#endif
	wait_for_vsync();
	set_palette(p, 0, pal_size);
	delete p;
}

void Ani::set_pal(void far *p)
{
	wait_for_vsync();
	set_palette(p, 0, pal_size);
}

void Ani::set_pos(signed short x_, signed short y_)
{
	x = x_;
	y = y_;
}

void Ani::set_pal_size(signed short n)
{
	pal_size = n;
}

void Ani::halt()
{
	mode = 5;
	run = 0;
}

void Ani::stop()
{
	mode = 5;
	free_all();
}

void Ani::play(signed short how)
{
	stop_at = -1;
	reset(how);
	while (stop_at != 0)
		game_idle();
}

void Ani::play_until(signed short how, signed char code)
{
	stop_at = code;
	reset(how);
	while (stop_at != 0)
		game_idle();
}

signed long Ani::calc_size(signed long hdr_size)
{
	char huge *buf;
	char huge *ar;
	char huge *hp;
	signed long total;
	signed long pal_off;
	signed long off;
	unsigned short bw;
	unsigned char bh;
	unsigned char cnt;
	unsigned char ar_h;
	unsigned char ar_pics;
	unsigned char c;
	signed short ncolors;
	signed long base;
	signed short n;
	signed short i;
	unsigned short ar_w;

	base = arc_seek(g_file_3, 0, SEEK_CUR);
	buf = (char huge *)mem_alloc(hdr_size);
	arc_read(g_file_3, buf, hdr_size);
	total = *(signed long huge *)buf;
	total += 6;
	hp = buf;
	bw = *(unsigned short huge *)(buf + 8);
	bh = *(buf + 10);
	cnt = *(buf + 11);
	if (old_format != 0) {
		n = cnt;
		if (n == 0)
			n = 1;
		count = cnt;
		areas = (struct ani_area far *)mem_alloc(n * sizeof(struct ani_area));
	}
	total += bw * bh;
	for (i = 0; cnt > i; i++) {
		off = *(signed long huge *)(buf + i * 4 + 0x0c);
		ar = buf + off;
		ar_h = *(ar + 7);
		ar_w = *(unsigned short huge *)(ar + 8);
		ar_pics = *(ar + 0x0b);
		if (old_format != 0)
			areas[i].nr_changes[0] = nr_changes[i] = *(ar + ar_pics * 4 + 0x0c);
		total += (signed long)ar_h * ar_w * (signed long)ar_pics;
	}
	pal_off = *(signed long huge *)(buf + 4) + 6;
	arc_seek(g_file_3, base + pal_off - 2, SEEK_SET);
	arc_read(g_file_3, &c, 1);
	if (c == 0xff)
		ncolors = 0x100;
	else
		ncolors = c;
	total += ncolors * 3;
	buf = (char huge *)mem_free(buf);
	return total;
}

void Ani::load(signed short id_)
{
	signed short i;
	signed short j;
	signed short k;
	signed short np;
	signed short nch;
	char far *chg;
	struct ani_file_area far *ar;
	char huge *base;
	char huge *up;
	char huge *res;
	char huge *end;
	signed long delta;
	signed long unplen;
	signed long plen;
	signed long d2;
	signed long rlen;
	signed long asize;
	signed long aoff;
	signed long doff;
	struct ani_area far *s;
	void far *tmp2;
	void far *tmp;
	char magic[4];
	unsigned char ver;
	signed long size;
	signed long need;
	struct ani_file_hdr far *hdr;

	halt();
	if (id_ != -1 && id != id_) {
	if (data)
		free_all();
	run = 0;
	id = id_;
	arc_open(g_file_3, id_);
	size = arc_size(g_file_3);
	pos = 0;
	magic[3] = 0;
	arc_read(g_file_3, magic, 3);
	if (strcmp(magic, "BOB")) {
		signed char c;

		old_format = 1;
		nr_variants = 1;
		f2b[0] = 1;
		arc_seek(g_file_3, pos, SEEK_SET);
		arc_read(g_file_3, &doff, 4);
		arc_seek(g_file_3, pos, SEEK_SET);
		need = calc_size(doff);
		if (farcoreleft() >= need * 2) {
			data = (char huge *)mem_alloc(need);
			in_mem = 1;
		} else
			in_mem = 0;
		arc_seek(g_file_3, pos, SEEK_SET);
		for (j = 0; count > j; j++) {
			c = nr_changes[j];
			if (!c)
				c = 1;
			areas[j].changes = (struct ani_changes far *)mem_alloc(c * 10 * sizeof(struct ani_change));
		}
		clear();
	} else {
		old_format = 0;
		arc_read(g_file_3, &ver, 1);
		if (ver == 1) {
			signed char c;
			register signed short n;

			arc_read(g_file_3, &nr_variants, 1);
			arc_read(g_file_3, (void far *)&count, 1);
			n = count;
			if (n == 0)
				n = 1;
			areas = (struct ani_area far *)mem_alloc(n * sizeof(struct ani_area));
			size -= 6;
			pos += 6;
			arc_read(g_file_3, &c, 1);
			arc_read(g_file_3, f2b, c);
			size -= c + 1;
			pos += c + 1;
			for (j = 0; count > j; j++) {
				signed char c2;

				arc_read(g_file_3, &nr_changes[j], 1);
				size--;
				pos++;
				c2 = nr_changes[j];
				if (!c2)
					c2 = 1;
				areas[j].changes = (struct ani_changes far *)mem_alloc(c2 * 10 * sizeof(struct ani_change));
				for (k = 0; k < 10; k++)
					areas[j].nr_changes[k] = nr_changes[j];
			}
			clear();
			for (i = 0; nr_variants > i; i++) {
				for (j = 0; count > j; j++) {
					arc_read(g_file_3, &areas[j].fl[i], 1);
					size--;
					pos++;
					arc_read(g_file_3, &areas[j].nr_changes[i], 2);
					size -= 2;
					pos += 2;
					for (k = 0; areas[j].nr_changes[i] > k; k++) {
						arc_read(g_file_3, &areas[j].changes[k].v[i], 4);
						size -= 4;
						pos += 4;
					}
				}
			}
		}
		arc_read(g_file_3, &doff, 4);
		arc_seek(g_file_3, -4, SEEK_CUR);
		need = calc_size(doff);
		if (farcoreleft() >= need * 2) {
			data = (char huge *)mem_alloc(need);
			in_mem = 1;
		} else
			in_mem = 0;
		arc_seek(g_file_3, pos, SEEK_SET);
	}
	if (in_mem == 0) {
		signed long pal_off;
		signed long first;
		signed long off0;
		signed long rd;
		struct rgb far *p;

		hdr = (struct ani_file_hdr far *)mem_alloc(doff);
		arc_read(g_file_3, hdr, doff);
		pal_off = hdr->pal + 6;
		arc_seek(g_file_3, pos + pal_off - 2, SEEK_SET);
		arc_read(g_file_3, &nr_colors, 1);
		arc_read(g_file_3, &packed, 1);
		pal = (char huge *)mem_alloc(nr_colors * 3U);
		arc_read(g_file_3, pal, nr_colors * 3U);
		w = hdr->w;
		h = hdr->h;
		count = hdr->count;
		need = (signed long)w * (unsigned long)h;
		data = (char huge *)mem_alloc(need);
		pic = data;
		first = hdr->main;
		if (count != 0) {
			off0 = hdr->area[0];
			ar = (struct ani_file_area far *)((char far *)hdr + off0);
			doff = ar->off[0];
			rd = doff - first;
		} else
			rd = need;
		arc_seek(g_file_3, pos + first, SEEK_SET);
		arc_read(g_file_3, data, rd);
		if (packed != 0) {
			plen = *(signed long far *)pic;
			tmp = mem_alloc(rd);
			memcpy(tmp, pic, rd);
			unpack_pp20(pic, tmp, plen);
			mem_free(tmp);
		}
		for (i = 0; count > i; i++) {
			s = &areas[i];
			aoff = hdr->area[i];
			ar = (struct ani_file_area far *)((char far *)hdr + aoff);
			strncpy(s->name, ar->name, 4);
			s->x = ar->x;
			s->y = ar->y;
			s->h = ar->h;
			s->w = ar->w;
			*(unsigned char far *)&s->fl[0] = ar->cyc;
			s->nr_pics = np = ar->pics;
			s->nr_changes[0] = nch = *(signed short far *)&ar->off[np];
			chg = (char far *)ar + np * 4 + 0x0e;
			for (j = 0; j < nch; j++) {
				s->changes[j].v[0].pic = *(chg + (unsigned)((j << 1) << 1));
				s->changes[j].v[0].code = *(chg + (unsigned)((j << 1) << 1) + 1);
				s->changes[j].v[0].dur = *(signed short far *)(chg + ((j << 1) << 1) + 2);
			}
		}
		multi = 0;
		for (i = 0; count > i; i++) {
			if (areas[i].fl[0].masked) {
				multi = 1;
				break;
			}
		}
		p = (struct rgb far *)pal;
		for (i = 0; nr_colors > i; i++) {
			p[i].r &= 0x3f;
			p[i].g &= 0x3f;
			p[i].b &= 0x3f;
		}
		hdr = (struct ani_file_hdr far *)mem_free(hdr);
		arc_close(g_file_3);
	} else {
		struct rgb far *p;

		arc_read_all(g_file_3, data);
		arc_close(g_file_3);
		base = data;
		pic = base + *(signed long huge *)data;
		pal = base + *(signed long huge *)(data + 4) + 6;
		f0a = *(signed short huge *)(pal - 6);
		f0c = *(signed short huge *)(pal - 4);
		packed = *(pal - 1);
		nr_colors = *(pal - 2);
		end = pal + nr_colors * 3;
		w = *(signed short huge *)(data + 8);
		h = *(data + 10);
		count = *(data + 11);
		if (packed != 0) {
			plen = *(signed long far *)pic;
			up = pic;
			up += plen - 4;
			unplen = *(signed long huge *)up;
			unplen = swap_u32(unplen) >> 8;
			tmp = mem_alloc(unplen);
			unpack_pp20(tmp, pic, plen);
			delta = unplen - plen;
			res = pic;
			res += plen;
			rlen = end - res;
			tmp2 = mem_alloc(rlen);
			memcpy(tmp2, EMS_norm_ptr(res), rlen);
			memcpy(pic, tmp, unplen);
			mem_free(tmp);
			res += delta;
			memcpy(EMS_norm_ptr(res), tmp2, rlen);
			tmp2 = mem_free(tmp2);
			pal += delta;
			end += delta;
		}
		for (i = 0; count > i; i++) {
			s = &areas[i];
			aoff = *(signed long huge *)(data + i * 4 + 0x0c);
			ar = (struct ani_file_area far *)(data + aoff);
			strncpy(s->name, ar->name, 4);
			s->x = ar->x;
			s->y = ar->y;
			s->h = ar->h;
			s->w = ar->w;
			*(unsigned char far *)&s->fl[0] = ar->cyc;
			s->nr_pics = np = ar->pics;
			if (packed != 0) {
				doff = ar->off[0];
				doff += delta;
				up = data + doff;
				plen = *(signed long huge *)up;
				up += plen - 4;
				asize = *(signed long huge *)up;
				asize = swap_u32(asize) >> 8;
				tmp = mem_alloc(asize);
				unpack_pp20(tmp, EMS_norm_ptr(data + doff), plen);
				d2 = asize - plen;
				delta += d2;
				res = data;
				res += doff;
				res += plen;
				rlen = end - res;
				tmp2 = mem_alloc(rlen);
				memcpy(tmp2, EMS_norm_ptr(res), rlen);
				memcpy(EMS_norm_ptr(data + doff), tmp, asize);
				mem_free(tmp);
				res += d2;
				memcpy(EMS_norm_ptr(res), tmp2, rlen);
				tmp2 = mem_free(tmp2);
				pal += d2;
				end += d2;
				asize = s->h * s->w;
				for (j = 0; j < np; j++)
					s->pics[j] = EMS_norm_ptr(data + doff + j * asize);
			} else {
				for (j = 0; j < np; j++) {
					doff = ar->off[j];
					s->pics[j] = EMS_norm_ptr(data + doff);
				}
			}
			s->nr_changes[0] = nch = *(signed short far *)&ar->off[np];
			chg = (char far *)ar + np * 4 + 0x0e;
			for (j = 0; j < nch; j++) {
				s->changes[j].v[0].pic = *(chg + (unsigned)((j << 1) << 1));
				s->changes[j].v[0].code = *(chg + (unsigned)((j << 1) << 1) + 1);
				s->changes[j].v[0].dur = *(signed short far *)(chg + ((j << 1) << 1) + 2);
			}
			bmp[i] = new Bmp(s->w, s->h, 0);
		}
		multi = 0;
		for (i = 0; count > i; i++) {
			if (areas[i].fl[0].masked) {
				multi = 1;
				break;
			}
		}
		p = (struct rgb far *)pal;
		for (i = 0; nr_colors > i; i++) {
			p[i].r &= 0x3f;
			p[i].g &= 0x3f;
			p[i].b &= 0x3f;
		}
	}
	}
}
