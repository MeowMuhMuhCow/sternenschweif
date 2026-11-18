/* OVL050 - Overlay O050 (Segment 0x2024 in SCHWEIF.EXE): Abspieler des Animationsobjekts Anim (Klasse aus O051,
 * 0xaf Bytes; hier als Methoden, frei Anim_*).
 * tick(): Takt (g_anim_running, Guthaben g_anim_timer gegen rate; ohne g_anim_no_skip werden verpasste Bilder ohne
 *   Marke uebersprungen), Bildwechsel: Bild cur laden (load_frame), Bmp(w, h) aus rect, entpacken (unpack -> decomp),
 *   Hintergrund bg bzw. leeren, Bild einsetzen und nach dst (Maus aus/an bei Ueberlappung, is_mouse_in_rect),
 *   cue = cues[cur], advance.
 * advance(): naechstes Bild (Ende -> rounds++, ohne Wiederholung loop anhalten). running(): g_anim_running == 1.
 * load(): Datei id oeffnen (arc_* auf g_file_3 bzw. bei from_file file_* auf g_file_main), Kopf (8 Bytes, Kennung
 *   "ACE", Version 1, Zahl der Sequenzen nr_seqs, Takt rate), Sequenzsatz seq (16 Bytes) bzw. Kurzkopf (6 Bytes):
 *   w, h, nr_frames, loop; Puffer data (buf_size aus max_frame_size), erstes Bild (Kopf 14 Bytes) als new fig_frame
 *   -> frame, Marken cues[i], Palette (letzte 0x300 Bytes).
 * load_pal(first, n): Palettenteil nachladen. max_frame_size(): groesstes gepacktes Bild aller Sequenzen nach
 *   buf_size. load_frame(): Bild cur neu laden. select_seq(n): Sequenz n waehlen (Puffer loesen, wie load), start(-1,
 *   -1, 0).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL050.CPP   -> 5276/5276 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - -Oi fuer strcmp (Intrinsic repne scasb/repe cmpsb/sbb); deshalb i in load auf [bp-2].
 * - fig_frame (14 Bytes) mit inline-Konstruktor (7 Parameter, Kopien [bp-12h]..[bp-1eh] bzw. [bp-2]..[bp-0eh], je
 *   new-Ausdruck eigener Satz; Stores +0..+8, +0d, +0c); delete frame -> mem_free_v ohne Nulltest.
 * - tick: redraw OHNE register (DI) - mit register nimmt es SI und der -Og-Zwischenwert (int)rate DI;
 *   [bp-0ah] = -Og (int)y; Bmp b im inneren Block (Dtor vor dem Blockende).
 * - Vergleiche "nr_frames > i" (cmp es:[..],si / jg); "(unsigned short)(n * 3)" bzw. "(i << 4)" -> movzx eax
 *   (ohne Cast movsx); "-(0x100L - first) * 3" -> mov edx,100h / sub / neg / imul edx,edx,3.
 * - load/load_pal/load_frame: "if (!from_file) { ..; return; }" (Epilogkopie), O037-Zweig danach.
 * - Methoden statt freier Funktionen mit Anim far *a (this an derselben Stelle, gleicher Code).
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * Moeglicher Originalfehler: select_seq gibt frame frei, setzt den Zeiger aber erst beim ersten Bild neu - laesst
 *   sich die Datei nicht oeffnen (oder hat die Sequenz 0 Bilder), bleibt frame haengend (unpack/load_frame benutzen
 *   bzw. loeschen ihn erneut). Ebenso load_frame, wenn die Datei fehlt (dann bleibt das alte Bild, harmlos).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:514e..5158, dsana) - initialisierte Daten in Adressreihenfolge */
char g_str_ace[4] = "ACE";
char g_str_ace2[6] = { 'A', 'C', 'E', 0, 0xff, 0xff };	/* "ACE" (Kennung der Animationsdatei), dahinter 0xffff */

/* Kopf einer ACE-Datei (8 Bytes) */
struct ace_header {
	char magic[4];			/* +0 "ACE" */
	signed short version;		/* +4 muss 1 sein */
	unsigned char nr_seqs;		/* +6 Zahl der Sequenzen */
	unsigned char rate;		/* +7 Takte je Bild */
};

/* Kurzkopf bei nur einer Sequenz (6 Bytes, hinter dem Dateikopf) */
struct ace_seq_short {
	signed short w;			/* +0 */
	unsigned char h;		/* +2 */
	char _r3;
	unsigned char nr_frames;	/* +4 */
	unsigned char loop;		/* +5 1 = wiederholen */
};

/* Sequenzsatz (16 Bytes, hinter dem Dateikopf) */
struct ace_seq {
	signed long pos;		/* +0x00 Dateiposition der Bilder */
	signed short f04;		/* +0x04 */
	signed short w;			/* +0x06 */
	unsigned char h;		/* +0x08 */
	char _r09[5];
	unsigned char nr_frames;	/* +0x0e */
	unsigned char loop;		/* +0x0f 1 = wiederholen */
};

/* Kopf eines Bildes in der Datei (14 Bytes), danach size Bytes gepackte Daten */
struct ace_frame {
	signed long size;		/* +0x00 */
	signed short x;			/* +0x04 Versatz */
	signed short y;			/* +0x06 */
	signed short w;			/* +0x08 */
	signed short h;			/* +0x0a */
	unsigned char pack;		/* +0x0c Packart (decomp) */
	unsigned char event;		/* +0x0d Marke (cues) */
};

void Anim::tick()
{
	signed short fx;
	signed short fy;
	signed short fw;
	signed short fh;
	signed short redraw_mouse = 0;

	if (g_anim_running == 1 && rate <= g_anim_timer) {
		if (g_anim_no_skip == 0) {
			g_anim_timer -= rate;
			if (rate <= g_anim_timer) {
				while (g_anim_timer > 0) {
					g_anim_timer -= rate;
					if ((cur != 0 || rounds != 0) && !cues[cur])
						advance();
				}
				g_anim_timer = 0;
			}
		} else {
			g_anim_timer = 0;
		}
		if (g_anim_timer == 0 && g_anim_running == 1) {
			if (cur != 0 || rounds != 0)
				load_frame();
			rect(&fx, &fy, &fw, &fh);
			{
				Bmp b(fw, fh);

				unpack(&b);
				if (bg_buf != 0)
					bmp.over(&bg, 0, 0, 0);
				else
					bmp.clear();
				bmp.over(&b, fx, fy, 4);
				if (is_mouse_in_rect(x, y, x + w, y + h)) {
					redraw_mouse = 1;
					mouse_hide(1);
				}
				bmp.put(dst, x, y);
				if (redraw_mouse)
					mouse_show(1);
			}
		}
		cue = cues[cur];
		advance();
	}
}

void Anim::advance()
{
	cur++;
	if (cur >= nr_frames) {
		cur = 0;
		rounds++;
		if (loop == 0)
			g_anim_running = 0;
	}
}

void Anim::unpack(Bmp far *b)
{
	decomp(g_decomp_work, frame->pack, frame->size, frame->w, frame->h, b->data(), data);
}

unsigned char Anim::running()
{
	return g_anim_running == 1 ? 1 : 0;
}

void Anim::load()
{
	signed short i;
	struct ace_header hdr;
	struct ace_seq_short hdr2;
	struct ace_seq srec;
	struct ace_frame fr;

	if (!from_file) {
		if (arc_open(g_file_3, id) != -1) {
			arc_read(g_file_3, &hdr, 8L);
			if (!strcmp(hdr.magic, g_str_ace)) {
				rate = hdr.rate;
				if (hdr.version == 1) {
					nr_seqs = hdr.nr_seqs;
					max_frame_size();
					arc_seek(g_file_3, 8L, SEEK_SET);
					if (hdr.nr_seqs > 1) {
						for (i = 0; seq >= i; i++)
							arc_read(g_file_3, &srec, 16L);
						w = srec.w;
						h = srec.h;
						nr_frames = srec.nr_frames;
						loop = srec.loop;
						if (loop == 1)
							loop = 3;
						if (loop != 0 && loop != 3)
							loop = 0;
						if (data == 0)
							data = mem_alloc(buf_size);
						arc_seek(g_file_3, srec.pos, SEEK_SET);
					} else {
						arc_read(g_file_3, &hdr2, 6L);
						w = hdr2.w;
						h = hdr2.h;
						nr_frames = hdr2.nr_frames;
						loop = hdr2.loop;
						if (loop == 1)
							loop = 3;
						if (loop != 0 && loop != 3)
							loop = 0;
						if (data == 0)
							data = mem_alloc(buf_size);
						arc_seek(g_file_3, 14L, SEEK_SET);
					}
					for (i = 0; nr_frames > i; i++) {
						arc_read(g_file_3, &fr, 14L);
						if (i == 0) {
							arc_read(g_file_3, data, fr.size);
							frame = new fig_frame(fr.x, fr.y, fr.w, fr.h, fr.size, fr.event, fr.pack);
						} else {
							arc_seek(g_file_3, fr.size, SEEK_CUR);
						}
						cues[i] = fr.event;
					}
					arc_seek(g_file_3, -0x300L, SEEK_END);
					arc_read(g_file_3, pal, 0x300L);
				}
			}
			arc_close(g_file_3);
		}
		return;
	}
	if (file_open(g_file_main, id) != 0xff) {
		file_read(g_file_main, &hdr, 8L);
		if (!strcmp(hdr.magic, g_str_ace2)) {
			rate = hdr.rate;
			if (hdr.version == 1) {
				nr_seqs = hdr.nr_seqs;
				max_frame_size();
				file_seek(g_file_main, 8L, SEEK_SET);
				if (hdr.nr_seqs > 1) {
					for (i = 0; seq >= i; i++)
						file_read(g_file_main, &srec, 16L);
					w = srec.w;
					h = srec.h;
					nr_frames = srec.nr_frames;
					loop = srec.loop;
					if (loop == 1)
						loop = 3;
					if (loop != 0 && loop != 3)
						loop = 0;
					if (data == 0)
						data = mem_alloc(buf_size);
					file_seek(g_file_main, srec.pos, SEEK_SET);
				} else {
					file_read(g_file_main, &hdr2, 6L);
					w = hdr2.w;
					h = hdr2.h;
					nr_frames = hdr2.nr_frames;
					loop = hdr2.loop;
					if (loop == 1)
						loop = 3;
					if (loop != 0 && loop != 3)
						loop = 0;
					if (data == 0)
						data = mem_alloc(buf_size);
					file_seek(g_file_main, 14L, SEEK_SET);
				}
				for (i = 0; nr_frames > i; i++) {
					file_read(g_file_main, &fr, 14L);
					if (i == 0) {
						file_read(g_file_main, data, fr.size);
						frame = new fig_frame(fr.x, fr.y, fr.w, fr.h, fr.size, fr.event, fr.pack);
					} else {
						file_seek(g_file_main, fr.size, SEEK_CUR);
					}
					cues[i] = fr.event;
				}
				file_seek(g_file_main, -0x300L, SEEK_END);
				file_read(g_file_main, pal, 0x300L);
			}
		}
		file_close(g_file_main);
	}
}

void Anim::load_pal(register signed short first, register signed short n)
{
	if (!from_file) {
		if (arc_open(g_file_3, id) != -1) {
			arc_seek(g_file_3, -(0x100L - first) * 3, SEEK_END);
			arc_read(g_file_3, pal + first * 3, (unsigned short)(n * 3));
			arc_close(g_file_3);
		}
		return;
	}
	if (file_open(g_file_main, id) != 0xff) {
		file_seek(g_file_main, -(0x100L - first) * 3, SEEK_END);
		file_read(g_file_main, pal + first * 3, (unsigned short)(n * 3));
		file_close(g_file_main);
	}
}

signed long Anim::max_frame_size()
{
	struct ace_seq_short hdr2;
	struct ace_seq srec;
	struct ace_frame fr;
	register signed short j;
	register signed short i;

	if (!from_file) {
		for (i = 0; nr_seqs > i; i++) {
			arc_seek(g_file_3, 8L, SEEK_SET);
			if (nr_seqs > 1) {
				arc_seek(g_file_3, (unsigned short)(i << 4), SEEK_CUR);
				arc_read(g_file_3, &srec, 16L);
				arc_seek(g_file_3, srec.pos, SEEK_SET);
				nr_frames = srec.nr_frames;
			} else {
				arc_read(g_file_3, &hdr2, 6L);
				nr_frames = hdr2.nr_frames;
			}
			for (j = 0; nr_frames > j; j++) {
				arc_read(g_file_3, &fr, 14L);
				if (buf_size < fr.size)
					buf_size = fr.size;
				arc_seek(g_file_3, fr.size, SEEK_CUR);
			}
		}
	} else {
		for (i = 0; nr_seqs > i; i++) {
			file_seek(g_file_main, 8L, SEEK_SET);
			if (nr_seqs > 1) {
				file_seek(g_file_main, (unsigned short)(i << 4), SEEK_CUR);
				file_read(g_file_main, &srec, 16L);
				file_seek(g_file_main, srec.pos, SEEK_SET);
				nr_frames = srec.nr_frames;
			} else {
				file_read(g_file_main, &hdr2, 6L);
				nr_frames = hdr2.nr_frames;
			}
			for (j = 0; nr_frames > j; j++) {
				file_read(g_file_main, &fr, 14L);
				if (buf_size < fr.size)
					buf_size = fr.size;
				file_seek(g_file_main, fr.size, SEEK_CUR);
			}
		}
	}
	return buf_size;
}

void Anim::load_frame()
{
	struct ace_seq srec;
	struct ace_frame fr;
	register signed short i;

	if (!from_file) {
		if (arc_open(g_file_3, id) != -1) {
			if (nr_seqs > 1) {
				arc_seek(g_file_3, 8L, SEEK_SET);
				for (i = 0; seq >= i; i++)
					arc_read(g_file_3, &srec, 16L);
				arc_seek(g_file_3, srec.pos, SEEK_SET);
			} else {
				arc_seek(g_file_3, 14L, SEEK_SET);
			}
			for (i = 0; cur > i; i++) {
				arc_read(g_file_3, &fr, 14L);
				arc_seek(g_file_3, fr.size, SEEK_CUR);
			}
			arc_read(g_file_3, &fr, 14L);
			arc_read(g_file_3, data, fr.size);
			delete frame;
			frame = new fig_frame(fr.x, fr.y, fr.w, fr.h, fr.size, fr.event, fr.pack);
			arc_close(g_file_3);
		}
		return;
	}
	if (file_open(g_file_main, id) != 0xff) {
		if (nr_seqs > 1) {
			file_seek(g_file_main, 8L, SEEK_SET);
			for (i = 0; seq >= i; i++)
				file_read(g_file_main, &srec, 16L);
			file_seek(g_file_main, srec.pos, SEEK_SET);
		} else {
			file_seek(g_file_main, 14L, SEEK_SET);
		}
		for (i = 0; cur > i; i++) {
			file_read(g_file_main, &fr, 14L);
			file_seek(g_file_main, fr.size, SEEK_CUR);
		}
		file_read(g_file_main, &fr, 14L);
		file_read(g_file_main, data, fr.size);
		delete frame;
		frame = new fig_frame(fr.x, fr.y, fr.w, fr.h, fr.size, fr.event, fr.pack);
		file_close(g_file_main);
	}
}

void Anim::select_seq(register signed short n)
{
	struct ace_seq srec;
	struct ace_frame fr;
	register signed short i;

	if (nr_seqs > n) {
		g_anim_running = 0;
		seq = n;
		bmp.free_lines();
		if (bmp_buf != 0 && bmp_ext == 0)
			bmp_buf = mem_free(bmp_buf);
		bg.free_lines();
		if (bg_buf != 0 && bg_ext == 0)
			bg_buf = mem_free(bg_buf);
#if FIX_0141
		/* Fix #141: Zeiger nach dem Freigeben nullen (kein zweites Loeschen) */
		delete frame;
		frame = 0;
		if (!from_file) {
#else
		delete frame;
		if (!from_file) {
#endif
			if (arc_open(g_file_3, id) != -1) {
				arc_seek(g_file_3, 8L, SEEK_SET);
				for (i = 0; i <= n; i++)
					arc_read(g_file_3, &srec, 16L);
				w = srec.w;
				h = srec.h;
				nr_frames = srec.nr_frames;
				loop = srec.loop;
				if (loop == 1)
					loop = 3;
				if (loop != 0 && loop != 3)
					loop = 0;
				arc_seek(g_file_3, srec.pos, SEEK_SET);
				for (i = 0; nr_frames > i; i++) {
					arc_read(g_file_3, &fr, 14L);
					if (i == 0) {
						arc_read(g_file_3, data, fr.size);
						frame = new fig_frame(fr.x, fr.y, fr.w, fr.h, fr.size, fr.event, fr.pack);
					} else {
						arc_seek(g_file_3, fr.size, SEEK_CUR);
					}
					cues[i] = fr.event;
				}
				arc_close(g_file_3);
			}
		} else {
			if (file_open(g_file_main, id) != 0xff) {
				file_seek(g_file_main, 8L, SEEK_SET);
				for (i = 0; i <= n; i++)
					file_read(g_file_main, &srec, 16L);
				w = srec.w;
				h = srec.h;
				nr_frames = srec.nr_frames;
				loop = srec.loop;
				if (loop == 1)
					loop = 3;
				if (loop != 0 && loop != 3)
					loop = 0;
				file_seek(g_file_main, srec.pos, SEEK_SET);
				for (i = 0; nr_frames > i; i++) {
					file_read(g_file_main, &fr, 14L);
					if (i == 0) {
						file_read(g_file_main, data, fr.size);
						frame = new fig_frame(fr.x, fr.y, fr.w, fr.h, fr.size, fr.event, fr.pack);
					} else {
						file_seek(g_file_main, fr.size, SEEK_CUR);
					}
					cues[i] = fr.event;
				}
				file_close(g_file_main);
			}
		}
		start(-1, -1, 0);
	}
}
