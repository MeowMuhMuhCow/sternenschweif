/* OVL054 - Overlay O054 (Segment 0x2046 in SCHWEIF.EXE): Hintergrundanimationen eines Bildes - Abspielen fuer das
 * Animationsobjekt Ani (struct ani, Objekt g_screen_pic; Laden ani_load, Anhalten ani_halt in O055). Grundbild
 * (w x h, pic) mit count Teilanimationen areas[] (struct ani_area: Lage, Groesse, Flags je Variante pingpong/masked/
 * off/order, Einzelbilder pics bzw. aus der Datei, Phasen changes[] mit je 10 Varianten {Bild, Marke, Dauer}).
 * select_variant(how, var): Variante var aktivieren (Flags und Phasen nach [0] kopieren), reset.
 * reset(how): Zaehler zuruecksetzen (ausser Art 2/5), Grundbild zeichnen (Bmp::put auf g_vga_buf, Maus aus/an, bei
 *   how < 3 Palette schwarz und danach setzen), Zeichenreihenfolge eintragen, Start (ausser Art 1/4).
 * anim_sound(id, code): leer (Klangausloeser fuer Bild 0xaf/0xb0 entfernt).
 * load_pic(i, n, dst): Einzelbild n des Teils i aus der Datei laden (Archiv g_file_3, Kopf mit Versaetzen +0x0c,
 *   gepackt unpack_pp20 bzw. direkt nach dst->data()).
 * tick(): Animationsschritt (Restdauer -5, ausser nach g_bg_restored; Phase weiter/pingpong, Marke stop_at), bei
 *   multi alle Teile in Reihenfolge neu auf das Grundbild, sonst je Teil; Maus nur aus, wenn g_mouse_posx/posy im
 *   Rechteck +-16 liegt; zwischendurch snd_speech_service.
 * load_reset/load_variant (anim_load/anim_load_variant): laden + reset bzw. select_variant. free_all: alles
 *   freigeben, init: Grundeinstellung.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL054.CPP   -> 4316/4316 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - C++: lokales Bmp-Objekt (gb_new / Destruktor (&b, 2)), new/delete Bmp (gb_new_alloc(0, ..) / gb_delete(p, 3)).
 * - -Oi: memcpy(dst->data(), buf + off2, row) -> rep movsw / adc cx,cx / rep movsb; -Og: Offsets i * 0x92 in DI
 *   (select_variant, free_all), (how & 0x7f) in [bp-0ch].
 * - "row = s->w * s->h" (short * uchar) ergibt push ax / mov ax,[w] / pop dx / imul dx (h zuerst geladen!);
 *   "s->h * s->w" -> imul word [mem]. "off2 = row * n" (long * int) -> movsx eax,n / mov edx,row / imul edx,eax;
 *   "n * row" -> imul eax,[row].
 * - data (+0x45) ist ein huge-Zeiger (Nulltest mov ax / or ax statt cmp dword); pal (+0x1a, in O055 huge) hier far
 *   getestet (Cast, cmp dword).
 * - Einzelbilder 1-basiert: pics[pic - 1] (struct ani_area, pics ab +0x16) ergibt dieselbe Adressrechnung wie die
 *   alte Sicht mit pics ab +0x12.
 * - Methoden der Klasse Ani statt freier Funktionen (this an derselben Stelle); Parameter how statt mode (Feld mode).
 * - tick: Rechteck-Variablen (x2, y2, y1, register x1 = SI) und "Bmp far *t" in GESCHWISTER-Bloecken (gleiche
 *   Stapelplaetze [bp-18h]/[bp-1ah] bzw. [bp-0ch]/[bp-0eh]); "continue" fuer den Zaehler-Abbruch, "break" bei
 *   stop_at -1. anim_sound: register-Parameter in AX/DL (keine Aufrufe).
 * Auffaellig/Originalfehler: tick zeichnet im Reihenfolge-Zweig Teil k = order[i] mit dem Bildobjekt bmp[i]
 *   (Index der Zeichenposition, nicht des Teils). load_pic addiert den Bildversatz als far-Arithmetik (nur Offset,
 *   ohne Normalisierung) und erst danach huge (+0x0c). anim_sound ist leer, wird aber in jedem Schritt gerufen.
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 */
#include <stdio.h>
#include <mem.h>
#include "schweif.h"

void Ani::select_variant(signed short how, signed char var)
{
	signed short i;
	register signed short j;

	multi = 0;
	for (i = 0; i < count; i++) {
		areas[i].fl[0] = areas[i].fl[var];
		for (j = 0; j < nr_changes[i]; j++)
			areas[i].changes[j].v[0] = areas[i].changes[j].v[var];
		if (areas[i].fl[0].masked)
			multi = 1;
	}
	reset(how);
}

void Ani::reset(register signed short how)
{
	Bmp b(w, h, pic);
	register signed short i;

	if (id != -1) {
		if ((how & 0x7f) != 2 && (how & 0x7f) != 5) {
			for (i = 0; i < 10; i++) {
				timer[i] = 0;
				cur[i] = -1;
				step[i] = 1;
				order[i] = -1;
			}
			if (how < 3)
				pal_black();
			busy = 1;
			mouse_hide(0);
			b.put(g_vga_buf, x, y, mode);
			if (how < 3)
				set_pal(pal);
			mouse_show(0);
			busy = 0;
		}
		if (multi) {
			signed short n = count;

			for (i = 0; i < n; i++)
				order[areas[i].fl[0].order] = i;
		}
		if ((how & 0x7f) != 1 && (how & 0x7f) != 4)
			run = 1;
	}
}

void anim_sound(register signed short id, register signed char code)
{
	if (code != 0) {
		if (id == 0xaf || id == 0xb0) {
			if (code == 1)
				;
		}
	}
}

void Ani::load_pic(signed short i, signed short n, Bmp far *dst)
{
	signed long len;
	signed long off;
	signed long off2;
	signed long len2;
	signed long size;
	signed long row;
	char far *hdr;
	char far *pk_buf;
	char far *buf;
	char huge *hp;
	struct ani_area far *s;

	arc_open(g_file_3, id);
	arc_seek(g_file_3, pos, SEEK_SET);
	arc_read(g_file_3, &len, 4L);
	arc_seek(g_file_3, pos, SEEK_SET);
	hdr = (char far *)mem_alloc(len);
	arc_read(g_file_3, hdr, len);
	s = areas + i;
	off = *(signed long far *)(hdr + i * 4 + 0x0c);
#if FIX_0092
	/* Fix #92: Bildversatz als huge-Arithmetik (normalisiert) */
	hp = (char huge *)hdr + off;
#else
	hp = hdr + off;
#endif
	off2 = *(signed long huge *)(hp + 0x0c);
	if (packed) {
		arc_seek(g_file_3, pos + off2, SEEK_SET);
		arc_read(g_file_3, &len2, 4L);
		pk_buf = (char far *)mem_alloc(len2);
		arc_seek(g_file_3, -4L, SEEK_CUR);
		arc_read(g_file_3, pk_buf, len2);
		row = s->w * s->h;
		size = row * s->nr_pics;
		buf = (char far *)mem_alloc(size);
		unpack_pp20(buf, pk_buf, len2);
		off2 = row * n;
		memcpy(dst->data(), buf + off2, row);
		mem_free(pk_buf);
		mem_free(buf);
	} else {
		row = s->w * s->h;
		off2 += row * n;
		arc_seek(g_file_3, pos + off2, SEEK_SET);
		arc_read(g_file_3, dst->data(), row);
	}
	mem_free(hdr);
	arc_close(g_file_3);
}

void Ani::tick()
{
	signed short n;
	signed char hit;
	struct ani_area far *s;
	signed short changed;
	register signed short i;

	if (run == 1) {
		n = count;
		if (n == 0 && stop_at != 0) {
			run = 0;
			stop_at = 0;
		}
		if (multi != 0) {
			changed = 0;
			for (i = 0; i < n; i++) {
				snd_speech_service(g_snd_speech);
				s = areas + i;
				if (s->nr_changes[0] != 0 && !s->fl[0].off) {
					if (g_bg_restored == 0)
						timer[i] -= 5;
					if (g_bg_restored == 0 && timer[i] > 0)
						continue;
					changed = 1;
					if (stop_at > 0 && s->changes[cur[i]].v[0].code == stop_at) {
						run = 0;
						stop_at = 0;
					}
					cur[i] += step[i];
					if (cur[i] == s->nr_changes[0]) {
						if (s->fl[0].pingpong) {
							step[i] = -1;
							cur[i]--;
						} else
							cur[i] = 0;
						if (stop_at == -1) {
							run = 0;
							stop_at = 0;
							break;
						}
					}
					if (cur[i] == 0 && s->fl[0].pingpong)
						step[i] = 1;
					timer[i] = s->changes[cur[i]].v[0].dur * 2;
					anim_sound(id, s->changes[cur[i]].v[0].code);
				}
			}
			if (changed != 0) {
				signed short k;
				Bmp b(w, h, pic);

				b.put(g_vga_buf, x, y, mode);
				for (i = 0; i < n; i++) {
					snd_speech_service(g_snd_speech);
					k = order[i];
					s = areas + k;
					if (s->nr_changes[0] != 0 && !s->fl[0].off) {
						hit = 0;
						{
							signed short x2;
							signed short y2;
							signed short y1;
							register signed short x1;

							x1 = x + s->x - 16;
							x2 = x1 + s->w + 32;
							y1 = y + s->y - 16;
							y2 = y1 + s->h + 32;
							if (g_mouse_posx >= x1 && g_mouse_posx <= x2 && g_mouse_posy >= y1 && g_mouse_posy <= y2) {
								hit = 1;
								mouse_hide(0);
							}
						}
						if (in_mem == 0) {
							Bmp far *t = new Bmp(s->w, s->h);

							load_pic(k, s->changes[cur[k]].v[0].pic - 1, t);
							t->put(g_vga_buf, x + s->x, y + s->y, s->fl[0].masked ? 4 : mode);
							delete t;
						} else {
#if FIX_0092
							/* Fix #92: Teil k mit seinem eigenen Bildobjekt bmp[k] zeichnen */
							bmp[k]->set_base(s->pics[s->changes[cur[k]].v[0].pic - 1]);
							bmp[k]->put(g_vga_buf, x + s->x, y + s->y, s->fl[0].masked ? 4 : mode);
#else
							bmp[i]->set_base(s->pics[s->changes[cur[k]].v[0].pic - 1]);
							bmp[i]->put(g_vga_buf, x + s->x, y + s->y, s->fl[0].masked ? 4 : mode);
#endif
						}
						if (hit == 1) {
							mouse_show(0);
							hit = 0;
						}
					}
				}
				changed = 0;
			}
		} else {
			for (i = 0, s = areas; i < n; s = &areas[++i]) {
				snd_speech_service(g_snd_speech);
				if (s->nr_changes[0] != 0 && !s->fl[0].off) {
					if (g_bg_restored == 0)
						timer[i] -= 5;
					if (g_bg_restored == 0 && timer[i] > 0)
						continue;
					if (stop_at > 0 && s->changes[cur[i]].v[0].code == stop_at) {
						run = 0;
						stop_at = 0;
					}
					if (timer[i] <= 0)
						cur[i] += step[i];
					if (cur[i] == s->nr_changes[0]) {
						if (s->fl[0].pingpong) {
							step[i] = -1;
							cur[i]--;
						} else
							cur[i] = 0;
						if (stop_at == -1) {
							run = 0;
							stop_at = 0;
							break;
						}
					}
					if (cur[i] == 0 && s->fl[0].pingpong)
						step[i] = 1;
					if (timer[i] <= 0)
						timer[i] = s->changes[cur[i]].v[0].dur * 2;
					anim_sound(id, s->changes[cur[i]].v[0].code);
					hit = 0;
					{
						signed short x2;
						signed short y2;
						signed short y1;
						register signed short x1;

						x1 = x + s->x - 16;
						x2 = x1 + s->w + 32;
						y1 = y + s->y - 16;
						y2 = y1 + s->h + 32;
						if (g_mouse_posx >= x1 && g_mouse_posx <= x2 && g_mouse_posy >= y1 && g_mouse_posy <= y2) {
							hit = 1;
							mouse_hide(0);
						}
					}
					if (in_mem == 0) {
						Bmp far *t = new Bmp(s->w, s->h);

						load_pic(i, s->changes[cur[i]].v[0].pic - 1, t);
						t->put(g_vga_buf, x + s->x, y + s->y, s->fl[0].masked ? 4 : mode);
						delete t;
					} else {
						bmp[i]->set_base(s->pics[s->changes[cur[i]].v[0].pic - 1]);
						bmp[i]->put(g_vga_buf, x + s->x, y + s->y, mode);
					}
					if (hit == 1) {
						mouse_show(0);
						hit = 0;
					}
				}
			}
		}
	}
	g_bg_restored = 0;
}

void Ani::load_reset(signed short id_, signed short how)
{
	load(id_);
	reset(how);
}

void Ani::load_variant(signed short id_, signed short how, signed char var)
{
	load(id_);
	select_variant(how, var);
}

void Ani::free_all()
{
	register signed short i;

	halt();
	for (i = 0; i < count; i++) {
		if (bmp[i]) {
			delete bmp[i];
			bmp[i] = 0;
		}
		if (areas != 0 && areas[i].changes)
			areas[i].changes = (struct ani_changes far *)mem_free(areas[i].changes);
	}
	if (areas != 0)
		areas = (struct ani_area far *)mem_free(areas);
	if (data) {
		data = (char huge *)mem_free(data);
		if (in_mem != 0)
			pal = 0;
		else
			pal = (char huge *)mem_free(pal);
	} else if ((char far *)pal != 0)		/* far-Nulltest (cmp dword) */
		pal = (char huge *)mem_free(pal);
	init();
}

void Ani::init()
{
	signed short i;

	x = 16;
	y = 2;
	busy = 0;
	stop_at = 0;
	run = 0;
	in_mem = 1;
	id = -1;
	for (i = 0; i < 10; i++)
		bmp[i] = 0;
}
