/* OVL112 - Overlay O112 (SCHWEIF.EXE): Animationsobjekte der Kampffiguren FigAnim (struct fig_anim: Kopf 0xa2 Bytes =
 * Ablaufbeschreibung struct ani_seq wie in O135/O136, dahinter der Zustand bis +0xdf), freie Funktionen zu den
 * Methoden in O016.
 * fig_anim_reset(o): Zustand zuruecksetzen (Einzelbildkopf frame_hdr per mem_free_v freigeben, Zeit last_tick =
 *   g_anim_ticks, running = 1). seq_set(o, a): Ablaufbeschreibung a (0xa2 Bytes, SCOPY@) uebernehmen und
 *   zuruecksetzen. seq_done(o): fertig (running == 0 oder laufender Schritt der Art 4).
 * fig_anim_step(o): Takt - Phase umschalten, bei faelliger Zeit (delay Takte seit last_tick) bzw. sync == 1
 *   weiterschalten (ticked = 1) und die verkettete Liste next mitnehmen (gleiche Phase -> ticked, sonst sync).
 * fig_anim_draw(o, img, x, y, dir, &x1, &y1, &x2, &y2, &frame, half): Figur zeichnen - load_seq/load_frame beim Anlauf,
 *   Bild aus dem XMS-Platz (g_xms_move, Handle g_fight_xms, Versatz slot * 0x550 + 0x1f400, EMS_copy bzw. xms_move_any)
 *   oder aus bmp, Pfad seq.path (Punkt path_pos), Clip-Fenster g_clip_rect per fig_anim_clip, Maus aus (mouse_overlaps_
 *   rect/mouse_hide/mouse_show) bei Bildschirm g_vga_buf, Duplicatus des Helden (duplicatus == 2: drei Bilder um 4
 *   versetzt, != 0: zwei), Rueckgabe Ereignis (frame_hdr->event bzw. next_frame: 0x0c..0x1e -> fig_anim_sound,
 *   0x1f -> Partner seq.linked starten, 0x20 -> Ende, ANI_EV_SFIGHT = Sonderfigur im Kampf 0x493 in den Folgen
 *   4..8/0x0f..0x17/0x1a..0x4d, ANI_EV_PATH = Pfad laeuft), g_fig_anim_removed = 1 in der Mitte der Folge.
 * fig_anim_clip(o, dir, x, y, half): Clip-Fenster auf das Figurrechteck (clamp_long auf 0..319 bzw. 0..199), je
 *   Richtung 1..4 an der Feldmitte g_fig_origin_x/y + Feldlage (fig_list_find_anim) halbiert.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL112.CPP   -> 3202/3202 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++ wegen des lokalen Bmp-Objekts in fig_anim_draw (Konstruktor gb_new0 nach "ev = 0; draw = 1;", Destruktor
 *   gb_delete(&b, 2) nach dem Zwischenwert [bp-2ch] des return); b.init = gb_init, b.put = gb_blit_m, b.free_lines.
 *   Die Methoden aus O016 werden als o->load_seq() usw. gerufen (gleicher far call wie frei mit o).
 * - fig_anim_draw: rw/rh muessen volatile sein. Sonst legt -Og "px + rw"/"py + rh" (Speicher *x2/*y2 und Argumente von
 *   mouse_overlaps_rect) als Zwischenwert ab; das Original rechnet beide Male neu. "py - 4" ist dagegen echter -Og-
 *   Zwischenwert [bp-30h], ebenso o->path_pos * 4 [bp-2eh]. Vertauschte Operanden (rw + px) verhindern das Teilen
 *   auch, aendern aber den Code.
 * - "!(signed char)g_mouse_locked" (Header unsigned char; Original cbw).
 * - fig_anim_step: "t = o->last_tick; q->last_tick = t;" als eigener Local [bp-10h] (direkte Zuweisung ohne
 *   Zwischenspeicher); p/q als struct fig_anim far * (Kette next).
 * - fig_anim_clip: letzter case faellt in "default: break;" bzw. "default: return;" - nur so springt default auf den
 *   Abschluss-jmp (b55) bzw. die Epilogkopie (c6a) statt auf das Funktionsende; Sprungtabelle des 2. switch zuerst.
 * - XMS-Auftrag g_xms_move (struct mem_move; cobjfix verlangt eine Adresse je Name: Feldreihenfolge len/src_handle/
 *   src/dst_handle/dst); Clip-Rechteck ueber die eigenen Publics g_clip_y0/g_clip_x1/g_clip_y1.
 * - Auffaellig (Original): fig_anim_clip begrenzt waagerecht mit cx -+ 5, senkrecht aber mit cy bzw. cy - 5 (nie + 5);
 *   case 1 wiederholt die schon gesetzte linke Kante.
 */
#include "schweif.h"

/* Zustand zuruecksetzen (nach seq_set) */
void fig_anim_reset(FigAnim far *o)
{
	if (o->frame_hdr) {
		mem_free_v(o->frame_hdr);
		o->frame_hdr = 0;
	}
	o->fb2 = 0;
	o->fb4 = 0;
	o->ofs_x = 0;
	o->ofs_y = 0;
	o->frame = 0;
	o->step = 0;
	o->delay_fix = -1;
	o->frame_loaded = 0xff;
	o->delay = 0;
	o->last_tick = g_anim_ticks;
	o->nr_frames = 0;
	o->loops = 0;
	o->seq_pos = 0;
	o->skip = 0;
	o->loaded = 0;
	o->fbc = 0;
	o->ticked = 0;
	o->running = 1;
	o->next = 0;
	o->phase = 0;
	o->path_pos = 0;
}

/* Ablaufbeschreibung uebernehmen und neu starten (O135/O136 ...) */
void seq_set(FigAnim far *o, struct ani_seq far *a)
{
	o->seq = *a;
	fig_anim_reset(o);
}

/* Folge fertig: beendet oder laufender Schritt der Art 4 */
signed short seq_done(FigAnim far *o)
{
	if (o->running == 0 || o->seq.step[o->step].type == 4)
		return 1;
	else
		return 0;
}

/* Takt: Phase umschalten, bei faelliger Zeit weiterschalten (Kette next mitnehmen); 1 = weitergeschaltet */
signed short fig_anim_step(FigAnim far *o)
{
	struct fig_anim far *p;
	struct fig_anim far *q;
	unsigned long now;
	unsigned long t;

	now = g_anim_ticks;
	o->ticked = 0;
	o->phase ^= 1;
	if (o->started && o->running && (o->sync == 1 || o->delay <= now - o->last_tick)) {
		o->ticked = 1;
		if (o->sync == 0) {
			o->last_tick = now;
			for (p = o; p->next; p = q) {
				q = p->next;
				t = o->last_tick;
				q->last_tick = t;
				if (q->phase == p->phase)
					q->ticked = 1;
				else
					q->sync = 1;
			}
		}
	}
	o->sync = 0;
	return o->ticked;
}

/* Figur nach img zeichnen (Bild aus dem XMS-Platz bzw. bmp, Pfad), gezeichnetes Rechteck und Einzelbild zurueck;
 * Ergebnis Ereignis (Klaenge/Partnerstart/Ende schon erledigt, sonst ANI_EV_* bzw. 0) */
signed short fig_anim_draw(FigAnim far *o, void far *img, signed short x, signed short y, signed char dir,
	signed short far *x1, signed short far *y1, signed short far *x2, signed short far *y2,
	signed short far *pframe, signed short half)
{
	signed short sy2;
	signed short sx2;
	signed short sy1;
	signed short sx1;
	signed short rx;
	signed short ry;
	signed short mouse;
	volatile signed short rw;
	volatile signed short rh;
	signed short ev;
	unsigned char step_ev;
	struct fig_node far *node;
	struct hero far *h;
	signed short draw;
	register signed short px;
	register signed short py;

	ev = 0;
	draw = 1;
	Bmp b;
	node = fig_list_find_anim(g_fig_list, o);
	*pframe = o->frame;
	*x1 = *x2 = x;
	*y1 = *y2 = y;
	if (o->started) {
		if (!o->skip) {
			if (o->loaded == 0) {
				o->load_seq();
				draw = 0;
			} else if ((o->running || o->seq.figure == FIG_FIGURE_SFIGHT) && (o->frame != o->frame_loaded || o->seq.figure == FIG_FIGURE_SFIGHT)) {
				o->load_frame();
				draw = 0;
			}
		}
		rx = o->frame_hdr->x;
		ry = o->frame_hdr->y;
		rw = o->frame_hdr->w;
		rh = o->frame_hdr->h;
#if FIX_0073
		/* Fix #73: Figur ohne XMS-Platz: Bild neu entpacken statt fremden Platz lesen */
		if (draw && o->in_xms) {
			if (o->slot < 0)
				o->load_frame();
			else {
				g_xms_move.len = ANIM_SMALL_MAX;
				g_xms_move.dst_handle = 0;
				g_xms_move.dst = (signed long)g_anim_small_buf;
				g_xms_move.src_handle = (signed short)g_fight_xms;
				g_xms_move.src = (signed long)o->slot * ANIM_SMALL_MAX + ANIM_XMS_BASE;
				if (g_fight_use_ems == 1)
					EMS_copy(&g_xms_move);
				else
					xms_move_any(&g_xms_move);
			}
#else
		if (draw && o->in_xms) {
			g_xms_move.len = ANIM_SMALL_MAX;
			g_xms_move.dst_handle = 0;
			g_xms_move.dst = (signed long)g_anim_small_buf;
			g_xms_move.src_handle = (signed short)g_fight_xms;
			g_xms_move.src = (signed long)o->slot * ANIM_SMALL_MAX + ANIM_XMS_BASE;
			if (g_fight_use_ems == 1)
				EMS_copy(&g_xms_move);
			else
				xms_move_any(&g_xms_move);
#endif
			b.init(rw, rh, g_anim_small_buf);
		} else {
			b.init(rw, rh, o->bmp->data());
		}
		if (o->seq.path) {
			x = o->seq.path[o->path_pos].x;
			y = o->seq.path[o->path_pos].y;
			if (o->ticked)
				o->path_pos++;
		}
		x += o->ofs_x;
		y += o->ofs_y;
		px = x + rx;
		py = y + ry;
		sx1 = g_clip_rect.x0;
		sy1 = g_clip_y0;
		sx2 = g_clip_x1;
		sy2 = g_clip_y1;
		fig_anim_clip(o, dir, px, py, half);
		*x1 = px;
		*x2 = px + rw;
		*y1 = py;
		*y2 = py + rh;
		mouse = 0;
		if (img == g_vga_buf && !(signed char)g_mouse_locked && mouse_overlaps_rect(px, py, px + rw, py + rh)) {
			mouse = 1;
			mouse_hide(0);
		}
		h = fig_get_hero(g_group, node->id);
		if (h) {
			if (h->duplicatus == 2) {
				*x1 = px - 4;
				*x2 = px + rw + 4;
				*y1 = py - 4;
				*y2 = py + rh + 4;
				g_clip_rect.x0 -= 4;
				g_clip_x1 += 4;
				g_clip_y0 -= 4;
				g_clip_y1 += 4;
				b.put(img, px + 4, py - 4, BLIT_MASKED);
				b.put(img, px, py, BLIT_MASKED);
				b.put(img, px - 4, py + 4, BLIT_MASKED);
			} else if (h->duplicatus != 0) {
				*x1 = px - 4;
				*x2 = px + rw;
				*y1 = py;
				*y2 = py + rh + 4;
				g_clip_rect.x0 -= 4;
				g_clip_y1 += 4;
				b.put(img, px, py, BLIT_MASKED);
				b.put(img, px - 4, py + 4, BLIT_MASKED);
			} else {
				b.put(img, px, py, BLIT_MASKED);
			}
		} else {
			b.put(img, px, py, BLIT_MASKED);
		}
		if (mouse)
			mouse_show(0);
		b.free_lines();
		g_clip_rect.x0 = sx1;
		g_clip_y0 = sy1;
		g_clip_x1 = sx2;
		g_clip_y1 = sy2;
		if (dir) {	/* (jede Richtung != 0: nur jedes zweite Zeichnen schaltet weiter) */
			o->skip++;
			if (o->skip == 2)
				o->skip = 0;
		}
		if (!o->skip) {
			if (o->ticked)
				ev = o->frame_hdr->event;
			else
				ev = 0;
			if (o->ticked) {
				step_ev = o->next_frame();
				if (!g_sfight_seq_on) {
					if (step_ev) {
						if (step_ev >= ANI_EV_SOUND && step_ev <= ANI_EV_SOUND_LAST) {
							fig_anim_sound(o, step_ev);
							step_ev = 0;
						} else if (step_ev == ANI_EV_START_LINKED) {
							o->seq.linked->started = 1;
							step_ev = 0;
						} else if (step_ev == ANI_EV_END) {
							o->running = 0;
						}
						if (step_ev)
							ev = step_ev;
					}
					if (g_fight_id == FIGHT_ID_SPECIAL && o->seq.figure == FIG_FIGURE_SFIGHT &&
						((o->seq_id >= 4 && o->seq_id <= 8) || (o->seq_id >= 0x0f && o->seq_id <= 0x17) ||
						(o->seq_id >= 0x1a && o->seq_id <= 0x4d)))
						ev = ANI_EV_SFIGHT;
					if (ev != ANI_EV_SFIGHT && o->nr_frames != 1 && (o->seq_id < 0x77 || o->seq_id > 0x9e) &&
						o->frame == o->nr_frames / 2 + 1)
						g_fig_anim_removed = 1;
				}
			}
			if (!g_sfight_seq_on) {
				if (ev >= ANI_EV_SOUND && ev <= ANI_EV_SOUND_LAST) {
					fig_anim_sound(o, ev);
					ev = 0;
				} else if (ev == ANI_EV_START_LINKED) {
					o->seq.linked->started = 1;
					ev = 0;
				} else if (ev == ANI_EV_END) {
					o->running = 0;
				}
			}
		}
		if (!o->skip)
			o->ticked = 0;
		if (ev == 0 && o->seq.path)
			ev = ANI_EV_PATH;
	} else {
		ev = 0;
	}
	return ev;
}

/* Clip-Fenster auf das Figurrechteck, je nach Richtung dir 1..4 an der Feldmitte halbiert (half: die andere Haelfte) */
void fig_anim_clip(FigAnim far *o, signed char dir, register signed short x, register signed short y, signed short half)
{
	signed short rx;
	signed short ry;
	signed short rw;
	signed short rh;
	signed short a;
	signed short b;
	signed short cx;
	signed short cy;
	struct fig_node far *node;

	node = fig_list_find_anim(g_fig_list, o);
	a = node->cbx;
	b = node->cby;
	cx = g_fig_origin_x + (a + b) * 10;
	cy = g_fig_origin_y + (a - b) * 5;
	rx = o->frame_hdr->x;
	ry = o->frame_hdr->y;
	rw = o->frame_hdr->w;
	rh = o->frame_hdr->h;
	g_clip_rect.x0 = clamp_long(x, 0, 0x13f);
	g_clip_x1 = clamp_long(x + rw, 0, 0x13f);
	g_clip_y0 = clamp_long(y, 0, 0xc7);
	g_clip_y1 = clamp_long(y + rh, 0, 0xc7);
	if (half) {
		switch (dir) {
		case 1:
			g_clip_rect.x0 = clamp_long(x, 0, 0x13f);
			g_clip_x1 = clamp_long(cx - 5, 0, 0x13f);
			break;
		case 2:
			g_clip_rect.x0 = clamp_long(cx + 5, 0, 0x13f);
			g_clip_x1 = clamp_long(x + rw, 0, 0x13f);
			break;
		case 3:
#if FIX_0224
			/* Fix #224: senkrecht symmetrisch: cy + 5 wie waagerecht cx + 5 */
			g_clip_y0 = clamp_long(cy + 5, 0, 0xc7);
			g_clip_y1 = clamp_long(y + rh, 0, 0xc7);
			break;
#else
			g_clip_y0 = clamp_long(cy, 0, 0xc7);
			g_clip_y1 = clamp_long(y + rh, 0, 0xc7);
			break;
#endif
		case 4:
			g_clip_y0 = clamp_long(y, 0, 0xc7);
			g_clip_y1 = clamp_long(cy - 5, 0, 0xc7);
		default:
			break;
		}
	} else {
		switch (dir) {
		case 1:
			g_clip_rect.x0 = clamp_long(x, 0, 0x13f);
			g_clip_x1 = clamp_long(cx + 5, 0, 0x13f);
			return;
		case 2:
			g_clip_rect.x0 = clamp_long(cx - 5, 0, 0x13f);
			g_clip_x1 = clamp_long(x + rw, 0, 0x13f);
			return;
		case 3:
			g_clip_y0 = clamp_long(cy - 5, 0, 0xc7);
			g_clip_y1 = clamp_long(y + rh, 0, 0xc7);
			return;
		case 4:
#if FIX_0224
			/* Fix #224: senkrecht symmetrisch: cy + 5 wie waagerecht cx + 5 */
			g_clip_y0 = clamp_long(y, 0, 0xc7);
			g_clip_y1 = clamp_long(cy + 5, 0, 0xc7);
			default:
			return;
#else
			g_clip_y0 = clamp_long(y, 0, 0xc7);
			g_clip_y1 = clamp_long(cy, 0, 0xc7);
		default:
			return;
#endif
		}
	}
}
