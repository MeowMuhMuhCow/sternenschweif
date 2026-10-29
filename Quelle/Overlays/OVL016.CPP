/* OVL016 - Overlay O016 (Segment 0x1f58 in SCHWEIF.EXE): Animationsobjekt einer Kampffigur FigAnim (struct fig_anim,
 * 0xe0 Bytes), erste 0xa2 Bytes die Ablaufbeschreibung struct ani_seq (seq_set: Figur/Bilddatei +0, Pfad +2,
 * Partner +6, Schritte zu 4 Bytes ab +0x0a, Ende frame 0xff). Die Methoden zum Takten/Zeichnen stehen in O112.
 * Konstruktor FigAnim(src, started): new 0xe0, frame_hdr/bmp = 0, seq_set, freien XMS-Platz in g_anim_slots[40]
 *   belegen (slot).
 * Destruktor: Platz freigeben, Einzelbildkopf frame_hdr (delete), Bild bmp (delete Bmp -> gb_delete(p, 3)) und Puffer
 *   pixels (ausser g_anim_small_buf/g_anim_big_buf), Pfad seq.path per mem_free.
 * load_seq(): Kopf (8 Bytes) der Bilddatei seq.figure aus g_file_3 lesen, Takte je Einzelbild delay aus delay_fix
 *   bzw. g_anim_delays[g_fight_speed / g_anim_speed] (je nach all_heroes_down), Folgensatz des Schritts suchen
 *   (find_seq), erstes Einzelbild laden (load_frame).
 * load_frame(): zum Einzelbild frame vorspulen (Satz 14 Bytes + Daten, Position pos/pos_frame), neues Bmp (w, h, Puffer
 *   0xcb20 fuer Figur 0x42, g_anim_big_buf fuer die Sonderfigur, sonst 0x2ee0 bzw. g_anim_small_buf fuer kleine Bilder
 *   <= 0x550), Kopf new fig_frame (inline-Konstruktor), Daten bei zu wenig farcoreleft ueber den Kampfpuffer
 *   g_fight_buf (vorher ins XMS gerettet, g_xms_move, EMS_copy/xms_move_any je nach g_fight_use_ems) sonst ueber
 *   mem_alloc, entpacken mit unpack (decomp), kleine Bilder in den XMS-Platz (slot * 0x550 + 0x1f400).
 * find_seq(n, id): unter n Folgensaetzen (16 Bytes) den mit id suchen, Felder uebernehmen.
 * next_frame(): naechstes Einzelbild bzw. naechster Schritt (Wiederholungen loops, Art 3/4), liefert das Ereignis
 *   des beendeten Schritts.
 * fig_anim_sound(a, ev): Klang (snd_play2(g_snd_fx, ..)) zum Ereignis 0x0c..0x1e je nach Figur (Helden 0xb5..0xc8/
 *   0xfc nach Waffenkategorie, Gegner 0xc9..0xf9 nach Kampfgrafik, Sonderfigur 0x9f, Figur 0x42, sonst snd_busy-Test).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL016.CPP   -> 3942/3942 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++: echter Konstruktor/Destruktor (this == 0 -> new, toter Epilog; Destruktor if (this), flags in SI);
 *   "delete bmp" (Bmp, nicht-inline Destruktor) -> gb_delete(p, 3) ohne Nulltest; "new Bmp(w, h, p)" ->
 *   gb_new(0, ..); "delete frame_hdr" (ohne Destruktor) -> mem_free_v direkt.
 * - register-Schluesselwoerter wirkungslos (BCC vergibt SI/DI selbst, Quelle ohne register); im Konstruktor
 *   bleibt der Schleifenzaehler trotzdem auf [bp-2].
 * - new fig_frame(..) mit inline-Konstruktor (s_struct.h): Parameterkopien [bp-0eh]..[bp-1ah] in Parameterreihenfolge,
 *   Speichern im Rumpf in Rumpfreihenfolge (event vor pack), Ergebnis in beiden Zweigen mov dx/ax.
 * - XMS-Auftrag g_xms_move (struct mem_move, signed long): Zeiger per (signed long)p -> mov eax.
 * - Lokale Tabelle "struct anim_delays d = g_anim_delays;" (8 Bytes, wortweise kopiert) oben auf [bp-8].
 * - decomp-Parameter short (mov ah,0 nach dem unsigned-char-Feld pack).
 * - fig_anim_sound: Sprungtabellen am Modulende in umgekehrter switch-Reihenfolge; Fall-Reihenfolge der Quelle
 *   12/14/13/20 bzw. 21 (leer)/23/12/14; switch mit 3 Faellen -> Vergleichskette; verschachtelte ?: als Argument.
 * Originalfehler/Auffaelligkeiten: Konstruktor ohne freien Platz in g_anim_slots laesst slot uninitialisiert
 *   (Destruktor schreibt dann an zufaellige Stelle); fig_anim_sound waehlt in Fall 20 (beide Zweige) und Gegnerfall
 *   12 (size == 4) je zweimal dasselbe Geraeusch (0xab bzw. 0x4d) - Abfrage wirkungslos.
 */
#include <stdio.h>
#include <alloc.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:0f22..0f2a, dsana) - initialisierte Daten in Adressreihenfolge */
struct anim_delays g_anim_delays = { { 0, 3, 7, 15 } };


/* Kopf der Bilddatei einer Figur (8 Bytes) */
struct anim_file_hdr {
	signed long f0;
	signed short version;		/* muss 1 sein */
	unsigned char nr_seqs;		/* Zahl der Folgensaetze */
	signed char _r7;
};

/* Kopf eines Einzelbilds in der Datei (14 Bytes), danach size Bytes Daten */
struct anim_file_frame {
	signed long size;
	signed short x;
	signed short y;
	signed short w;
	signed short h;
	unsigned char pack;
	unsigned char event;
};

/* Folgensatz (16 Bytes) */
struct anim_file_seq {
	signed long pos;		/* Dateiposition der Einzelbilder */
	signed short id;		/* Folge */
	signed short f6;
	signed char f8;
	signed char _r9;
	signed short ofs_x;
	signed short ofs_y;
	signed char nr_frames;
	signed char _rf;
};

FigAnim::FigAnim(struct ani_seq far *src, signed short started_)
{
	signed short i;

	frame_hdr = 0;
	bmp = 0;
	seq_set(this, src);
#if FIX_0073
	/* Fix #73: ohne freien XMS-Platz slot = -1 statt Muell */
	started = started_;
	slot = -1;
	for (i = 0; i < ANIM_SLOTS; i++) {
#else
	started = started_;
	for (i = 0; i < ANIM_SLOTS; i++) {
#endif
		if (!g_anim_slots[i]) {
			slot = i;
			g_anim_slots[slot] = 1;
			break;
		}
	}
}

FigAnim::~FigAnim()
{
#if FIX_0073
	/* Fix #73: nur einen eigenen Platz freigeben */
	if (slot >= 0)
		g_anim_slots[slot] = 0;
	if (frame_hdr != 0) {
#else
	g_anim_slots[slot] = 0;
	if (frame_hdr != 0) {
#endif
		delete frame_hdr;
		frame_hdr = 0;
	}
	if (bmp != 0) {
		delete bmp;
		if (pixels != g_anim_small_buf && pixels != g_anim_big_buf)
			mem_free(pixels);
		bmp = 0;
	}
	if (seq.path != 0) {
		mem_free(seq.path);
		seq.path = 0;
	}
}

/* Folgenkopf des laufenden Schritts laden, erstes Einzelbild laden */
void FigAnim::load_seq()
{
	struct anim_delays d = g_anim_delays;
	struct anim_file_hdr hdr;

	if (running != 0) {
		if (arc_open(g_file_3, seq.figure) != -1) {
			arc_read(g_file_3, &hdr, 8L);
			if (hdr.version == 1) {
				if (delay_fix != -1)
					delay = delay_fix;
				else if (all_heroes_down(g_group))
					delay = d.d[g_fight_speed];
				else
					delay = d.d[g_anim_speed];
				find_seq(hdr.nr_seqs, seq.step[step].frame);
				arc_close(g_file_3);
				frame_loaded = 0xff;
				loaded = 1;
				load_frame();
				return;
			}
			arc_close(g_file_3);
		}
	}
}

/* Einzelbild frame laden und entpacken */
void FigAnim::load_frame()
{
	void far *buf;
	signed long need;
	signed long avail;
	signed short via_fight_buf = 0;
	signed short i;
	struct anim_file_frame fr;

	if (arc_open(g_file_3, seq.figure) != -1) {
		if (frame == pos_frame + 1) {
			/* das naechste: ab der gemerkten Position */
			arc_seek(g_file_3, pos, SEEK_SET);
			arc_read(g_file_3, &fr, 14L);
			pos += 14;
			arc_seek(g_file_3, fr.size, SEEK_CUR);
			pos += fr.size;
		} else {
			arc_seek(g_file_3, seq_pos, SEEK_SET);
			pos = seq_pos;
			for (i = 0; i < frame; i++) {
				arc_read(g_file_3, &fr, 14L);
				pos += 14;
				arc_seek(g_file_3, fr.size, SEEK_CUR);
				pos += fr.size;
			}
		}
		pos_frame = frame;
		arc_read(g_file_3, &fr, 14L);
		if (bmp != 0) {
			delete bmp;
			if (pixels != g_anim_small_buf && pixels != g_anim_big_buf)
				mem_free(pixels);
			bmp = 0;
		}
		need = (signed long)fr.w * fr.h;
		if (need > ANIM_SMALL_MAX || seq.figure == FIG_FIGURE_SFIGHT || seq.figure == FIG_FIGURE_SFIGHT2) {
			if (seq.figure == FIG_FIGURE_SFIGHT2)
				pixels = mem_alloc(0xcb20L);
			else if (seq.figure == FIG_FIGURE_SFIGHT)
				pixels = g_anim_big_buf;
			else
				pixels = mem_alloc(0x2ee0L);
			in_xms = 0;
		} else {
			pixels = g_anim_small_buf;
			in_xms = 1;
		}
		bmp = new Bmp(fr.w, fr.h, pixels);
		if (frame_hdr != 0) {
			delete frame_hdr;
			frame_hdr = 0;
		}
		frame_hdr = new fig_frame(fr.x, fr.y, fr.w, fr.h, fr.size, fr.event, fr.pack);
		via_fight_buf = 0;
		need += fr.size;
		avail = farcoreleft();
		if (avail <= need) {
			/* zu wenig Speicher: gepackte Daten im Kampfpuffer, der dafuer ins XMS ausweicht */
			via_fight_buf = 1;
			g_xms_move.len = 0xfa00L;
			g_xms_move.src_handle = 0;
			g_xms_move.src = (signed long)g_fight_buf;
			g_xms_move.dst_handle = (signed short)g_fight_xms;
			g_xms_move.dst = 0xfa00L;
			if (g_fight_use_ems == 1)
				EMS_copy(&g_xms_move);
			else
				xms_move_any(&g_xms_move);
			arc_read(g_file_3, g_fight_buf, fr.size);
			arc_close(g_file_3);
		} else {
			buf = mem_alloc(fr.size);
			arc_read(g_file_3, buf, fr.size);
			arc_close(g_file_3);
		}
		if (via_fight_buf) {
			unpack(bmp, g_fight_buf);
			g_xms_move.len = 0xfa00L;
			g_xms_move.dst_handle = 0;
			g_xms_move.dst = (signed long)g_fight_buf;
			g_xms_move.src_handle = (signed short)g_fight_xms;
			g_xms_move.src = 0xfa00L;
			if (g_fight_use_ems == 1)
				EMS_copy(&g_xms_move);
			else
				xms_move_any(&g_xms_move);
		} else {
			unpack(bmp, buf);
			buf = mem_free(buf);
		}
#if FIX_0073
		/* Fix #73: ohne Platz nicht ins XMS schreiben */
		if (in_xms != 0 && slot >= 0) {
			g_xms_move.len = ANIM_SMALL_MAX;
#else
		if (in_xms != 0) {
			g_xms_move.len = ANIM_SMALL_MAX;
#endif
			g_xms_move.src_handle = 0;
			g_xms_move.src = (signed long)g_anim_small_buf;
			g_xms_move.dst_handle = (signed short)g_fight_xms;
			g_xms_move.dst = (signed long)slot * ANIM_SMALL_MAX + ANIM_XMS_BASE;
			if (g_fight_use_ems == 1)
				EMS_copy(&g_xms_move);
			else
				xms_move_any(&g_xms_move);
		}
	}
	frame_loaded = frame;
}

/* unter n Folgensaetzen den mit id suchen */
void FigAnim::find_seq(signed short n, signed short id)
{
	signed short i;
	struct anim_file_seq s;

	for (i = 0; i < n; i++) {
		arc_read(g_file_3, &s, 16L);
		if (s.id == id) {
			seq_pos = s.pos;
			nr_frames = s.nr_frames;
			ofs_x = s.ofs_x;
			ofs_y = s.ofs_y;
			fb2 = s.f6;
			fb4 = s.f8;
			seq_id = id;
			return;
		}
	}
}

/* weiterschalten: naechstes Einzelbild, Wiederholung oder naechster Schritt; Ergebnis Ereignis des Schritts */
signed char FigAnim::next_frame()
{
	signed char ev = 0;

	loaded = 1;
	if (running != 0 || seq.figure == FIG_FIGURE_SFIGHT) {
		if (frame < nr_frames - 1) {
			frame++;
		} else if (seq.step[step].type == 3 || seq.step[step].type == 4) {
			if (seq.step[step].count == 0xff && nr_frames == 1) {
				loops++;
				running = 0;
			} else if (seq.step[step].count > loops) {
				loops++;
				frame = 0;
				loaded = 0;
			} else if (seq.step[step].count == 0xff) {
				frame = 0;
				loaded = 0;
			} else {
				ev = seq.step[step].event;
				if (seq.step[step + 1].frame != ANI_FRAME_END) {
					step++;
					frame = 0;
					loaded = 0;
				} else {
					running = 0;
				}
			}
		} else {
			ev = seq.step[step].event;
			if (seq.step[step + 1].frame != ANI_FRAME_END) {
				step++;
				frame = 0;
				loaded = 0;
			} else {
				running = 0;
			}
		}
	}
	return ev;
}

void FigAnim::unpack(Bmp far *b, void far *src)
{
	decomp(g_decomp_work, frame_hdr->pack, frame_hdr->size, frame_hdr->w, frame_hdr->h, b->data(), src);
}

/* Klang zum Animationsereignis ev (0x0c..0x1e) */
void fig_anim_sound(FigAnim far *a, unsigned char ev)
{
	struct fig_node far *node;
	struct hero far *h;
	struct enemy far *e;
	signed char animal;
	signed short wclass;

	node = fig_list_find_anim(g_fig_list, a);
	if ((a->seq.figure >= FIG_FIGURE_HERO && a->seq.figure <= FIG_FIGURE_HERO_LAST) ||
	    a->seq.figure == FIG_FIGURE_HERO_X) {
		h = fig_get_hero(g_group, node->id);
		wclass = hand_weapon_class_melee(h);
		switch (ev) {
		case 13:
		case 14:
			snd_play2(g_snd_fx, wclass == 1 ? 0x59 : wclass == -1 ? 0x5a : wclass == 0 ? 0x57 :
				wclass == 3 ? 0x58 : 0x79);
			break;
		case 17:
			snd_play2(g_snd_fx, 0x5d);
			break;
		case 20:
			if (hand_weapon_class(h) == 4)
				snd_play2(g_snd_fx, 0xab);
			else
				snd_play2(g_snd_fx, 0xab);
			break;
		}
	} else if (a->seq.figure >= FIG_FIGURE_ENEMY && a->seq.figure <= FIG_FIGURE_ENEMY_LAST) {
		e = fig_get_enemy(node->id);
		animal = e->is_animal;
		switch (ev) {
		case 12:
			snd_play2(g_snd_fx, animal ? 0x4d : enemy_is_gfx_17_29(e) ? 0x4f : enemy_is_gfx_1c(e) ? 0x76 :
				enemy_is_humanoid(e) ? 0x4c : e->size == 4 ? 0x4d : 0x4d);
			break;
		case 14:
			snd_play2(g_snd_fx, enemy_is_gfx_24_18(e) ? 0x66 : enemy_is_gfx_2a(e) ? 0x68 :
				e->gfx_id == 0x29 ? 0x89 : enemy_is_gfx_17_29(e) ? 0x67 : enemy_is_fire(e) ? 0x85 :
				enemy_is_gfx_1c(e) ? 0x7d : enemy_is_gfx_42(e) ? 0x7e : enemy_is_gfx_25_35_36_23(e) ? 0x7f :
				animal ? 0x78 : 0x7a);
			break;
		case 13:
			snd_play2(g_snd_fx, enemy_is_gfx_24_18(e) ? 0x66 : enemy_is_gfx_2a(e) ? 0x68 :
				e->gfx_id == 0x29 ? 0x8b : enemy_is_gfx_17_29(e) ? 0x67 : enemy_is_fire(e) ? 0x85 :
				enemy_is_gfx_1c(e) ? 0x7d : enemy_is_gfx_42(e) ? 0x8a : enemy_is_gfx_25_35_36_23(e) ? 0x7f :
				animal ? 0x78 : 0x7a);
			break;
		case 20:
			if (e->shots > 0)
				snd_play2(g_snd_fx, 0xab);
			else
				snd_play2(g_snd_fx, 0xab);
			break;
		}
	} else if (a->seq.figure == FIG_FIGURE_SFIGHT) {
		switch (ev) {
		case 16:
			snd_play2(g_snd_fx, 0x88);
			break;
		case 17:
			snd_play2(g_snd_fx, 0x86);
			break;
		case 19:
			snd_play2(g_snd_fx, 0x5f);
			break;
		case 21:
			snd_play2(g_snd_fx, 0x6a);
			break;
		case 27:
			snd_play2(g_snd_fx, 0x87);
			break;
		}
	} else if (a->seq.figure == FIG_FIGURE_SFIGHT2) {
		switch (ev) {
		case 21:
			break;
		case 23:
			snd_play2(g_snd_fx, 0x81);
			break;
		case 12:
			snd_play2(g_snd_fx, 0x4c);
			break;
		case 14:
			snd_play2(g_snd_fx, 0x6a);
			break;
		}
	} else {
		switch (ev) {
		case 0x16:
			if (!snd_busy(g_snd_fx, 0))
				snd_play2(g_snd_fx, 0x64);
			break;
		case 0x17:
			if (!snd_busy(g_snd_fx, 0))
				snd_play2(g_snd_fx, 0x5c);
			break;
		case 0x1a:
			if (!snd_busy(g_snd_fx, 0))
				snd_play2(g_snd_fx, 0x81);
			break;
		}
	}
}
