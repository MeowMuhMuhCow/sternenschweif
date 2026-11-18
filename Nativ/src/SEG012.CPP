/* SEG012 - 3D-Ansicht: Tastaturpuffer, Bild-Ablauf, Bewegungs-/Dreh-Eingabe, Objekt zeichnen (drehbare
 * Flaechen, Klickflaechen-Tabelle), HUD-Rahmen, Klick-Auswahl, Sternenhimmel, Mauszeiger, Nebeltabelle
 * (seg012 in SCHWEIF.EXE)
 * view_frame: je Bild Tastaturpuffer, Kamera in Zelle umrechnen (view_draw), Gebietsereignisse
 * (g_pending_msg .. g_dng16_double_key_state -> area_event_*, call_handler mit struct map_event), Gruppenposition aus der Kamera
 * (group::set_pos_x/set_pos_y), Klick-Auswahl (pick_mouse/pick_center -> g_pick_event), Feldereignis beim
 * Betreten (cell_event -> g_cell_event, Typ -2). move_input: Bewegung frei (g_free_move: Geschwindigkeit
 * g_cam_speed, Neigung g_cam_roll, Blickhoehe g_cam_pitch) oder schrittweise (g_step_active: Ziel 0x100-Winkel
 * g_step_target, Drehschritt g_step_turn) aus Richtungsbits g_key_flag bzw. Mausbereich g_cursor_nr; cam_set_speed/
 * cam_move (R13). draw_object: drehbare Flaechen (FACE_ROTATE) um g_cam_angle - Objektwinkel drehen,
 * obj_project, dann je sichtbarer Flaeche (Liste g_face_list/g_face_next) Klickflaeche (FACE_CLICK: Rechteck,
 * Tiefe und Ereignis in g_click_* , hoechstens 8) oder Polygon (poly_setup[_clip], Textur FACE_TEXTURED aus
 * EMS bzw. g_tex_base, Schattierung FACE_SHADED ueber g_shade_table, poly_tex.. bzw. poly_flat). draw_hud: vier
 * Rahmenteile g_hud_* und Kompass. pick_rect: vorderste Klickflaeche im Rechteck -> g_pick_event (Sperre
 * g_pick_delay 25 Bilder). draw_stars: 100 Sterne g_stars. set_cursor/install_cursor: Mauszeiger je
 * Mausbereich (Rueckruf g_mouse_handler). fog_table/fade_table: Helligkeitstabelle g_fog_table[32].
 *
 * C++! BCC -mlarge -3 -Ot -c -Y SEG012.CPP   -> 5280/5280 Bytes GLEICH (18.10.2026)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 *   ab draw_object gilt #pragma option -Og (move_input geht mit -Og nicht, alle Funktionen danach
 *   sind mit und ohne -Og gleich). -Oc schadet nicht.
 * Schluessel:
 * - abs() ist in C++ die inline-Funktion aus STDLIB.H: ihre Parameterkopie erzeugt die Locals
 *   [bp-2] (pick_center), [bp-0Ch]/[bp-0Eh]/[bp-10h] (move_input) und [bp-38h] (draw_object).
 * - view_frame: die Gruppenposition wird ueber inline-Memberfunktionen group::set_pos_x/set_pos_y gesetzt
 *   (Parameterkopien [bp-2..-8]). Inline-Parameterkopien liegen auf Funktionsebene VOR den Locals
 *   innerer Bloecke; das Original (Rahmen 8, Ereigniskopie bei [bp-4]) geht nur, wenn die beiden
 *   Aufrufe in einem eigenen Block MIT einer Deklaration stehen (irgendeine; hier extern) und die
 *   Ereigniskopie t in den if-Bloecken liegt: dann teilen sich die Geschwisterbloecke die Plaetze.
 * - draw_object: Drehung wie ROT in seg013, aber ohne X/Y/Z-Variablen: [bp-3Ch]/[bp-40h] sind
 *   -Og-Zwischenwerte (long)p.x/(long)p.y, [bp-42h] der -Og-Zwischenwert &g_face_list[k-1] (nur Offset).
 *   Winkel als struct vtx (4 Worte, w unbenutzt). Klickflaechen-Block: sum/x register (SI/DI),
 *   m register (CX), zweite Schleifenvariable m2 nicht-register in eigenem Block (DX).
 *   f->nverts--; nv = f->nverts; bzw. nv = f->nverts; f->nverts++; als getrennte Anweisungen.
 * - pick_rect: EIN register-Hilfswert i fuer Tausch und Schleife (DX); Fehler im Original: der
 *   y-Vergleich tauscht x1/x2.
 * - draw_stars: Bildschirmposition als zweite struct vtx q (Strukturen landen nicht in Registern),
 *   off = q.x + q.y * 320, Schleife i <= 99.
 * - move_input: if (!g_step_active) erzeugt mov al/cbw/or ax,ax; frueheres return erzeugt den Epilog im Zweig.
 * - Objektparameter wie in s_proto.h als void far * (draw_object, call_handler), Zugriff ueber Cast-Makros.
 * - Phase B (19.10.2026): eigene Daten im Quelltext. g_area_descs hat 17 Eintraege (Dungeon 0..16; Dungeon 16 lag
 *   frueher scheinbar in g_town_descs), die Stadttabelle heisst g_town_area_descs[6] (Index town - 1, MAP-Name
 *   g_town_descs = 8 Bytes davor). Rueckrufe aus Overlays per EXTDEF, cobjfix_d prueft jeden gegen das Original-DS.
 */
#include <bios.h>
#include <stdlib.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5d46..5e78, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct map_event g_dng16_key_event1 = { 1, 0x00, 0x00 };
struct map_event g_dng16_key_event2 = { 2, 0x00, 0x00 };
signed short g_pick_delay = 0;
signed short g_last_cam_cell_y = -1;
signed short g_last_cam_cell_x = -1;
struct cursor_areas g_view_cursor_areas = { {	/* Mausbereiche der 3D-Ansicht (x0, y0, x1, y1, Aktion) */
	{ 64, 0, 173, 33, 0x80 },
	{ 64, 103, 173, 135, 0x81 },
	{ 15, 33, 64, 103, 0x82 },
	{ 173, 33, 222, 103, 0x83 },
	{ 15, 0, 88, 52, 0x84 },
	{ 147, 0, 222, 52, 0x85 },
	{ 0, 0, 319, 199, 0x86 }
}, -1 };
/* Kartenereignis-Rueckrufe (struct area_desc.handler); die Funktionen haben verschiedene Objektzeiger-Typen */
#define AREA_HANDLER(f)	{ (signed short (far *)(void far *, void far *))(f) }
struct area_desc g_area_descs[17] = {	/* je Dungeon (Index group.dungeon), [0] = kein Dungeon */
	{ 0 },
	AREA_HANDLER(dng01_handler), AREA_HANDLER(dng02_handler), AREA_HANDLER(dng03_handler),
	AREA_HANDLER(dng04_handler), AREA_HANDLER(dng05_handler), AREA_HANDLER(dng06_handler),
	AREA_HANDLER(dng07_handler), AREA_HANDLER(dng08_handler), AREA_HANDLER(dng09_handler),
	AREA_HANDLER(dng10_handler), AREA_HANDLER(dng11_handler), AREA_HANDLER(dng12_handler),
	AREA_HANDLER(dng13_handler), AREA_HANDLER(dng14_handler), AREA_HANDLER(dng14_handler),	/* 14, 15 */
	AREA_HANDLER(dng16_handler)
};
struct area_desc g_town_area_descs[6] = {	/* je Stadt 1..6 (Index town - 1) */
	AREA_HANDLER(town_harbor_event), AREA_HANDLER(town_sites_event), AREA_HANDLER(town03_handler),
	AREA_HANDLER(town_lowangen_event), AREA_HANDLER(port_event), AREA_HANDLER(town_tjolmar_event)
};

#define OBJ	((struct obj3d far *)obj)
#define EVOBJ	((struct area_events far *)evobj)

void far kbd_poll(void)
{
	register signed short k;

	while (bioskey(1)) {
		k = bioskey(0);
		if (k != KEY_UP && k != KEY_DOWN && k != KEY_LEFT && k != KEY_RIGHT && g_key_buf == 0)
			g_key_buf = k;
	}
}

void far view_frame(void)
{
	kbd_poll();
	view_draw();
	if (g_pending_msg == 1) {
		snd_play(g_snd_fx, 0xa5);
		dlg_message(g_dialog, g_text_buf2);
		g_pending_msg = 0;
	}
	if (g_dng07_prison_event == 1)
		dng07_prison_day(g_area_events);
	if (g_dng03_thief_pending == 1)
		dng03_stone_thief(g_area_events);
	if (g_tjolmar_vault == 1)
		dng01_entry_fight(g_area_events);
	if (g_dng05_poltergeist == 1 || g_dng05_poltergeist == 3)
		dng05_poltergeist(g_area_events);
	if (g_dng16_double_key_state == 1 && !(get_cell_b(g_map_x0 + 2, g_map_y0 + 13) & 0x1000)) {
		struct map_event t;

		g_dng16_key_event1.x = g_map_x0 + 3;
		g_dng16_key_event1.y = g_map_y0 + 13;
		t = g_dng16_key_event1;
		g_dng16_double_key_state = 2;
		call_handler(g_area_events, &t);
	}
	if (g_dng16_double_key_state == 2 && !(get_cell_b(g_map_x0 + 3, g_map_y0 + 13) & 0x1000)) {
		struct map_event t;

		g_dng16_key_event2.x = g_map_x0 - 1;
		g_dng16_key_event2.y = g_map_y0 - 1;
		t = g_dng16_key_event2;
		g_dng16_double_key_state = 3;
		call_handler(g_area_events, &t);
	}
	g_view_pos_y = g_cam_x - g_map_y0 * 168;
	g_view_pos_x = g_cam_z - g_map_x0 * 168;
	{
		extern struct group far *g_group;
		g_group->set_pos_x(g_cam_cell_x - g_map_x0, g_view_pos_x);
		g_group->set_pos_y(g_cam_cell_y - g_map_y0, g_view_pos_y);
	}
	pick_mouse();
	if (g_move_flags || g_action == KEY_SPACE || g_action == KEY_INS) {
		if (g_move_flags == 1 || (g_move_flags & 8) == 8 || g_action == KEY_SPACE || g_action == KEY_INS)
			pick_center();
		if (g_move_flags)
			g_move_flags = 0;
	}
	cell_event();
}

void far move_input(void)
{
	register signed short up, down;
	signed short left, right, f10, f20;
	signed short spd;

	if (g_mine_cart_pos > 0) {
		if (g_mine_cart_pos == 15) {
			g_cam_speed = 4;
			g_cam_pitch += 10;
			if (g_cam_pitch > 128)
				g_cam_pitch = 128;
		} else {
			g_cam_speed = g_mine_cart_pos / 2 + 1;
		}
		cam_set_speed(g_cam_speed >> 1);
		cam_move();
		return;
	}
	if (!g_silent_mode2 && !g_silent_mode && g_pp20_index == 4 && !g_view_input_blocked) {
		if (g_pick_delay)
			g_pick_delay--;
		if (g_mouse_ldown == 0) {
			up = g_key_flag & 1;
			down = g_key_flag & 2;
			left = g_key_flag & 4;
			right = g_key_flag & 8;
			f10 = g_key_flag & 0x10;
			f20 = g_key_flag & 0x20;
		} else {
			up = (g_cursor_nr == 9 || g_cursor_nr == 10 || g_cursor_nr == 11);
			down = g_cursor_nr == 8;
			left = (g_cursor_nr == 6 || g_cursor_nr == 10);
			right = (g_cursor_nr == 7 || g_cursor_nr == 11);
			f10 = g_key_flag & 0x10;
			f20 = g_key_flag & 0x20;
		}
		g_move_flags = 0;
		spd = 0;
		if (up) {
			g_move_flags |= 1;
			spd = 8;
		}
		if (down) {
			g_move_flags |= 2;
			spd = 16;
		}
		if (left || right) {
			g_move_flags |= 4;
			spd = 16;
		}
		if (!g_step_active)
			g_step_flags = spd;
		if (g_free_move) {
			if (up && g_cam_speed < 8)
				g_cam_speed++;
			if (down && g_cam_speed > -5)
				g_cam_speed--;
			if (!up && !down) {
				if (g_cam_speed < 0 && ++g_cam_speed > 0)
					g_cam_speed = 0;
				if (g_cam_speed > 0 && --g_cam_speed < 0)
					g_cam_speed = 0;
			}
			if (left) {
				g_cam_angle -= 3;
				g_cam_roll -= 2;
				if (g_cam_roll > 0)
					g_cam_roll -= 3;
				if (abs(g_cam_speed) * -25 > g_cam_roll)
					g_cam_roll += 4;
			} else if (right) {
				g_cam_angle += 3;
				g_cam_roll += 2;
				if (g_cam_roll < 0)
					g_cam_roll += 3;
				if (abs(g_cam_speed) * 25 < g_cam_roll)
					g_cam_roll -= 4;
			} else {
				if (abs(g_cam_roll) < 4)
					g_cam_roll = 0;
				else if (g_cam_roll < 0)
					g_cam_roll += 4;
				else
					g_cam_roll -= 4;
			}
			if (f10) {
				if (++g_cam_pitch > 128)
					g_cam_pitch = 128;
			}
			if (f20) {
				if (--g_cam_pitch < -128)
					g_cam_pitch = -128;
			}
			if (g_action == KEY_DEL || g_key_pad5 || (g_action & 0xff00) == KEY_PAD5)
				g_cam_pitch = 0;
			cam_set_speed(g_cam_speed >> 1);
			cam_move();
			return;
		}
		if (!g_step_active) {
			if (up) {
				g_cam_speed = 8;
				g_step_turn = 0;
				g_step_active = 1;
				g_step_pending = 0;
			} else if (down) {
				g_cam_speed = -5;
				g_step_turn = 0;
				g_step_active = 1;
				g_step_pending = 0;
			} else if (left) {
				g_step_target = (g_cam_angle - 0x100) & 0x300;
				g_step_turn = -8;
				g_step_active = 1;
				g_step_pending = 0;
			} else if (right) {
				g_step_target = (g_cam_angle + 0x100) & 0x300;
				g_step_turn = 8;
				g_step_active = 1;
				g_step_pending = 0;
			} else if (f10) {
				if (++g_cam_pitch > 128)
					g_cam_pitch = 128;
			} else if (f20) {
				if (--g_cam_pitch < -128)
					g_cam_pitch = -128;
			} else if (g_action == KEY_DEL || (g_action & 0xff00) == KEY_PAD5)
				g_cam_pitch = 0;
		} else {
			g_move_flags |= g_step_flags;
			if (g_cam_angle == g_step_target) {
				g_step_turn = 0;
				if (g_cam_speed == 0)
					g_step_active = 0;
			}
			g_cam_angle += g_step_turn;
			g_cam_angle &= 0x3ff;
		}
		if (!cam_move()) {
			g_cam_speed = 0;
			g_step_flags = 0;
			if (g_step_turn == 0)
				g_step_active = 0;
		}
		cam_set_speed(g_cam_speed >> 1);
	}
}

#pragma option -Og
void far draw_object(void far *obj)
{
	signed short i, j, n, k;
	unsigned short u, zmin, z;
	unsigned short flags;
	struct face3d far *f;
	signed short persp;
	struct vtx p, base;
	struct vtx ang;
	signed short vpos;
	signed short nv;
	struct face3d far * far *pp;

	n = OBJ->nfaces;
	vpos = 0x80;
	ang.x = 0;
	ang.y = 0;
	ang.z = g_cam_angle - OBJ->rot.az;
	pp = OBJ->faces;
	for (i = 0; i < n; i++) {
		if ((*pp++)->flags & FACE_ROTATE) {
			f = pp[-1];
			f->nverts--;
			nv = f->nverts;
			base = OBJ->vl->v[f->idx[nv]];
			for (j = 0; j < nv; j++) {
				p = g_vtx_buf[vpos--] = OBJ->vl->v[k = f->idx[j]];
				p.x = p.x - base.x;
				p.y = p.y - base.y;
				p.z = p.z - base.z;
				{
					long s0, s1, s2, c0, c1, c2, t1, t2, t3;
					s0 = g_sin_table[ang.x & 0x3ff];
					s1 = g_sin_table[ang.y & 0x3ff];
					s2 = g_sin_table[ang.z & 0x3ff];
					c0 = g_sin_table[(ang.x + 0x100) & 0x3ff];
					c1 = g_sin_table[(ang.y + 0x100) & 0x3ff];
					c2 = g_sin_table[(ang.z + 0x100) & 0x3ff];
					t1 = ((long)p.x * c0 - (long)p.y * s0) >> 15;
					t2 = ((long)p.y * c0 + (long)p.x * s0) >> 15;
					p.y = (t2 * c1 - p.z * s1) >> 15;
					t3 = (p.z * c1 + t2 * s1) >> 15;
					p.z = (t3 * c2 - t1 * s2) >> 15;
					p.x = (t1 * c2 + t3 * s2) >> 15;
				}
				p.x = p.x + base.x;
				p.y = p.y + base.y;
				p.z = p.z + base.z;
				OBJ->vl->v[k] = p;
			}
		}
	}
	g_nr_vtx = g_nr_vis_faces = g_nr_clip_faces = 0;
	if (!obj_project(obj))
		guru_meditation("CalcBuffer full.", 0);
	if (g_nr_vtx > vpos)
		guru_meditation("Too many vectors.", 0);
	k = *g_face_next;
	for (i = 0; i < g_nr_vis_faces; i++) {
		f = g_face_list[k - 1].face;
		n = f->nverts;
		flags = f->flags;
		if (flags & FACE_CLICK) {
			signed short xmin, ymin, xmax, ymax, y, zz;
			register signed short sum, x;
			register signed short m;

			xmin = ymin = 0x7fff;
			xmax = ymax = -0x7fff;
			sum = 0;
			for (m = 0; m < n; m++) {
				x = g_vtx_buf[f->idx[m]].x;
				y = g_vtx_buf[f->idx[m]].y;
				zz = g_vtx_buf[f->idx[m]].z;
				if (zz > 400)
					goto next;
				sum += zz;
				if (xmin > x)
					xmin = x;
				if (xmax < x)
					xmax = x;
				if (ymin > y)
					ymin = y;
				if (ymax < y)
					ymax = y;
			}
			if (n)
				sum /= n;
			if (g_nr_click == 8) {
				signed short m2;

				for (m2 = 0; m2 < 8; m2++) {
					if (g_click_depth[m2] > sum) {
						g_click_rect[m2].y0 = ymin;
						g_click_rect[m2].y1 = ymax;
						g_click_rect[m2].x0 = xmin;
						g_click_rect[m2].x1 = xmax;
						g_click_event[m2] = g_face_list[k - 1].event;
						g_click_angle[m2] = g_face_list[k - 1].angle;
						g_click_depth[m2] = sum;
						break;
					}
				}
			} else {
				g_click_rect[g_nr_click].y0 = ymin;
				g_click_rect[g_nr_click].y1 = ymax;
				g_click_rect[g_nr_click].x0 = xmin;
				g_click_rect[g_nr_click].x1 = xmax;
				g_click_event[g_nr_click] = g_face_list[k - 1].event;
				g_click_angle[g_nr_click] = g_face_list[k - 1].angle;
				g_click_depth[g_nr_click] = sum;
				if (g_nr_click++ > 8)
					g_nr_click = 8;
			}
		} else {
			persp = (flags & (FACE_TEXTURED | FACE_NEAR)) == (FACE_TEXTURED | FACE_NEAR);
			fill_smth();
			zmin = 0x7fff;
			j = 0;
			for (u = 0; u < n; u++) {
				if ((z = g_vtx_buf[f->idx[u]].z) < zmin)
					zmin = z;
				if (z > j)
					j = z;
			}
			if (g_area_type == AREA_TOWN) {
				if (zmin < 100 && j < 400 && g_area_type == AREA_TOWN && (flags & FACE_NEAR))
					poly_setup_clip(n, f->idx, g_vtx_buf, f->uv, persp);
				else
					poly_setup(n, f->idx, g_vtx_buf, f->uv, persp);
			} else {
				if (zmin < 100 && j < 400)
					poly_setup_clip(n, f->idx, g_vtx_buf, f->uv, persp);
				else
					poly_setup(n, f->idx, g_vtx_buf, f->uv, persp);
			}
			if (flags & FACE_SHADED) {
				u = abs(g_face_list[k - 1].angle);
				if (u > 0xff)
					u = 0xff;
				u = g_shade_table[u];
				if (u < 0x60)
					u = 0x60;
				u = 7 - (u >> 5);
			} else
				u = 0;
			if (flags & FACE_TEXTURED) {
				g_cur_tex = f->tex->ref;
				g_cur_tex_w = f->tex->w;
				if (g_cur_tex.seg == 0) {
					if (g_ems_avail == 0)
						g_cur_tex.off += g_tex_base;
					else {
						EMS_map4(0, g_cur_tex.off >> 14, g_swap_handle);
						g_cur_tex.seg = g_ems_frame;
						g_cur_tex.off &= 0x3fff;
					}
				}
				if (f->flags & FACE_MASKED) {
					if (persp)
						poly_tex_mp(u);
					else
						poly_tex_m(u);
				} else {
					if (persp)
						poly_tex_p(u);
					else
						poly_tex(u);
				}
			} else
				poly_flat(u, ((unsigned char far *)f)[0x17]);
		}
next:
		k = g_face_next[k];
	}
	vpos = 0x80;
	n = OBJ->nfaces;
	pp = OBJ->faces;
	for (i = 0; i < n; i++) {
		if ((*pp++)->flags & FACE_ROTATE) {
			f = pp[-1];
			nv = f->nverts;
			f->nverts++;
			for (j = 0; j < nv; j++)
				OBJ->vl->v[f->idx[j]] = g_vtx_buf[vpos--];
		}
	}
}


void far draw_hud(void)
{
	register signed short m = 0;

	if (g_area_type == AREA_TOWN) {
		gb_blit_m(g_hud_top_left, g_renderbuf_ptr, 0, 0, 4);
		gb_blit_m(g_hud_top_right, g_renderbuf_ptr, 0xb7, 0, 4);
		gb_blit_m(g_hud_bottom_left, g_renderbuf_ptr, 0, 0x84, 4);
		gb_blit_m(g_hud_bottom_right, g_renderbuf_ptr, 0xba, 0x6b, 4);
	} else {
		gb_blit_m(g_hud_top_left, g_renderbuf_ptr, 0, 0, 4);
		gb_blit_m(g_hud_top_right, g_renderbuf_ptr, 0xa3, 0, 4);
		gb_blit_m(g_hud_bottom_left, g_renderbuf_ptr, 0, 0x52, 4);
		gb_blit_m(g_hud_bottom_right, g_renderbuf_ptr, 0xc5, 0x70, 4);
	}
	compass_draw(g_compass, g_cam_angle);
	if (mouse_overlaps_rect(0x10, 2, 0xe0, 0x89)) {
		m = 1;
		mouse_hide(0);
	}
	copy_3d_to_screen();
	if (m)
		mouse_show(0);
}

void far pick_rect(signed short x1, signed short y1, signed short x2, signed short y2)
{
	register signed short i;

	if (x1 > x2) {
		i = x1;
		x1 = x2;
		x2 = i;
	}
	if (y1 > y2) {
		i = x1;
		x1 = x2;
		x2 = i;
	}
	g_pick_event = -1L;
	g_pick_depth = 10000;
	for (i = g_nr_click - 1; i >= 0; i--) {
		if (g_click_rect[i].x0 <= x2 && g_click_rect[i].x1 >= x1 && g_click_rect[i].y0 <= y2 && g_click_rect[i].y1 >= y1 &&
			g_click_depth[i] < g_pick_depth) {
			if (g_pick_delay)
				return;
			g_pick_event = g_click_event[i];
			g_pick_delay = 25;
			g_pick_depth = g_click_depth[i];
			g_pick_angle = g_click_angle[i];
			return;
		}
	}
}

void far pick_point(signed short x, signed short y)
{
	pick_rect(x, y, x, y);
}

void far pick_mouse(void)
{
	g_pick_event = -1L;
	if (g_mouse_lclick) {
		if (g_mouse_clickx < 0xd0 && g_mouse_clicky < 0x88 && g_cursor_nr == 0)
			pick_point(g_mouse_clickx, g_mouse_clicky);
		g_mouse_lclick = g_mouse_lpending = 0;
	}
}

void far pick_center(void)
{
	pick_rect(0x64, 0x31, 0x8c, 0x59);
	if (g_action != KEY_SPACE) {
		if (!g_free_move && (g_cam_angle & 0xff)) {
			g_pick_event = -1L;
			return;
		}
		if (abs(g_pick_angle) < 80 || g_pick_depth > 100)
			g_pick_event = -1L;
	}
}

void far cell_event(void)
{
	*(signed long *)&g_cell_event = -1L;
	if (g_last_cam_cell_y != g_cam_cell_y || g_last_cam_cell_x != g_cam_cell_x) {
		if (g_skip_cell_event == 0) {
			g_cell_event.type = MAP_EVENT_ENTER;
			g_cell_event.x = g_cam_cell_y;
			g_cell_event.y = g_cam_cell_x;
		}
		g_last_cam_cell_y = g_cam_cell_y;
		g_last_cam_cell_x = g_cam_cell_x;
		g_skip_cell_event = 0;
	}
}

void far draw_stars(void)
{
	unsigned char far *scr = (unsigned char far *)g_renderbuf_ptr;
	unsigned char col;
	struct vtx q, p;
	register signed short off, i;

	for (i = 0; i <= 99; i++) {
		p = g_stars[i];
		p.x = ((p.x + 0x80 - g_cam_ang_z) & 0xff) - 0x80;
		p.y = ((p.y + g_cam_ang_y + 0x80) & 0xff) - 0x80;
		if (p.z > 0) {
			col = g_star_colors[7 - (p.z >> 6)];
			q.x = p.x + 0x68;
			q.y = p.y + 0x44;
			if (q.x >= 0 && q.x < 0xce && q.y >= 0 && q.y < 0x86) {
				off = q.x + q.y * 320;
				if (p.z > 150)
					scr[off] = col;
				else if (p.z > 50) {
					scr[off] = col;
					scr[off + 1] = col;
				} else {
					scr[off] = col;
					scr[off + 1] = col;
					scr[off + 320] = col;
					scr[off + 321] = col;
				}
			}
		}
	}
}

void far set_cursor(signed short key)
{
	register signed short c;

	if (key) {
		switch (key) {
		case 0x80: c = 9; break;
		case 0x81: c = 8; break;
		case 0x82: c = 6; break;
		case 0x83: c = 7; break;
		case 0x84: c = 10; break;
		case 0x85: c = 11; break;
		default: c = 0; break;
		}
		if (c != g_cursor_no) {
			mouse_set_cursor(c, 0);
			g_cursor_no = c;
		}
	}
}

void far install_cursor(void)
{
	g_mouse_handler_areas = &g_view_cursor_areas;
	g_mouse_handler = set_cursor;
	g_mouse_handler_on = 1;
}

void far fog_table(signed short n, signed short base)
{
	signed short range = 32 - base;
	register signed short i;
	signed short v;

	if (!n)
		n = 1;
	for (i = 0; i < 32; i++) {
		v = base + range * i / n;
		g_fog_table[i] = v < 32 ? v : 31;
	}
}

void far fade_table(signed short a, signed short b)
{
	register signed short i;

	for (i = 0; i < 32; i++)
		g_fog_table[i] = a + ((b - a) * i >> 5);
}

signed short far call_handler(void far *evobj, void far *pos)
{
	signed short r;

	g_mouse_lclick = g_mouse_lpending = 0;
	g_mouse_event = 0;
	r = EVOBJ->desc.handler(evobj, pos);
	*(signed long far *)pos = -1L;
	return r;
}

signed short far call_handler2(void far *evobj, void far *pos)
{
	signed short r;

	g_mouse_lclick = g_mouse_lpending = 0;
	g_mouse_event = 0;
	r = EVOBJ->desc.handler(evobj, pos);
	*(signed long far *)pos = -1L;
	return r;
}
