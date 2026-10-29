/* OVL198 - Overlay O198 (Segment 0x234f in SCHWEIF.EXE): 3D-Ansicht - ein Bild aufbauen.
 * view_draw: Kameraposition g_cam_x/g_cam_z per div(.., 0xa8) in Zelle (g_view_cell_y/x, g_cam_cell_y/g_cam_cell_x) und Rest
 *   (g_cam_rx/g_cam_rz) zerlegen, Augenhoehe aus den Wandprofilen der Zelle (cell_profile, Tabellen g_prof_zmin/
 *   zmax/ymin/ymax, Anzahl g_prof_count; lineare Interpolation) minus Wert aus der Groesse des ersten Helden ((cm -
 *   130) * 10 / 25 + 40), ausser in Dungeon 11/14/15 auf 40 begrenzt -> g_cam_eye_y; Kamera cam_set_angles(
 *   &g_cam_rot_x), Blickrichtung (g_cam_angle >> 4) & 0x3f, ab 32 gespiegelt. Texturanimation (g_tex_anim_due/
 *   frame/frames, Folgen g_tex_anim_a/b/c) bzw. Phase der Objekte g_cell_objs[..] je nach g_area_type und Dungeon
 *   bzw. Stadt. Himmel/Hintergrund: Stadt -> view_draw_sky, sonst fill_scene (-0x62 am Tag 0x93a8 < g_daytime <=
 *   0x190c8 in Dungeon 4) und draw_stars. Dann zwei Durchlaeufe ueber die Sichtliste g_vis_order[Richtung]
 *   (dx; dy, Ende 0x7f, Start g_vis_start): (1) Objektnummern get_cell_a der Nachbarzellen in die 9x9-Tabellen
 *   g_near_objs (Nr. + 1) und g_near_occl (g_cell_occludes[Nr.]), (2) nach view_cull_hidden (O255) und den
 *   Klangdiensten je Zelle das Objekt g_cell_objs[Nr.] positionieren (x/z = d * 0xa8 - Rest + 0x54),
 *   Klickflaechen (FACE_CLICK) bekommen die Zellkoordinaten, Tuerzustand in der Karte g_cells[x][y].flags (Bit
 *   12/13, Stufe 0..15, Start ueber cell_forget), Drehung get_cell_c, zeichnen draw_object. Zum Schluss
 *   g_view_frames++, draw_hud.
 * view_draw_sky: Himmel (Objekt g_cell_objs[6] bzw. [13]) nur tagsueber: Textur der ersten Flaeche nach g_cur_tex/
 *   g_cur_tex_w, ggf. aus EMS (EMS_map4, g_ems_avail == 1) bzw. Offset g_tex_base, zeilenweise mit Phase
 *   (g_cam_angle + g_view_frames / 2) % Breite und Hoehenpendel als Ringpuffer nach g_renderbuf_ptr kopieren
 *   (memcpy bzw. tex_copy_flat); in der Daemmerung (vor 8:00 / nach 18:00) Schattierung ueber die Tabelle
 *   g_shade_segs[16 + Stufe] (Stufe 15: Schachbrett schwarz), danach fade_table(16, Stufe * 2) bei Wechsel
 *   (g_sky_shade); nachts fog_table/fill_scene(0)/draw_stars.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL198.C   -> 3547/3547 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (g_prof_zmin.. -> _g_a008+.., g_prof_count -> _g_b107+7, g_tex_anim_b -> _g_b13a+0x12 usw.). -Og und -Oi beide
 *   noetig.
 * - Stack verraet die Bloecke: Funktionsebene c..saved, dann -Og-Zwischenwerte [bp-1ah] (cell * 0x180),
 *   [bp-1ch]/[bp-1eh] (g_view_cell_y + dx / g_view_cell_x + dy, als Makros X/Y ohne eigene Variable - werden nach Speicher-
 *   zugriffen neu berechnet), [bp-20h] (u * 9 + u2); erst dahinter die Block-Locals (a/b/e/z/t/v/h im Block der
 *   Augenhoehe, sel im else-Zweig, u2 bzw. u1/u2/j/q in den Schleifenrumpfen teilen [bp-22h..]).
 * - Register: i (DI) auf Funktionsebene; s (SI) im Block der Augenhoehe (in einem Block ohne Aufrufe -> CX);
 *   eine register-Variable s im Rumpf von Durchlauf 2 fuer Flaechenzahl UND Kartenwert (sonst CX bzw. Stack);
 *   k (Ort, dann Schleifenzaehler) als register im aufruffreien Zweig -> CX.
 * - Rueckgabe-struct von get_cell_c als expliziter Zielzeiger (Makro GET_CELL_ROT; bei "o->rot = get_cell_c(..)"
 *   bleibt ein toter mov ax / add ax,8 stehen). div() liefert div_t (shl eax / shrd eax,edx).
 * - "!(c = g_near_objs[..])" (unsigned char) -> mov ah,0 / or ax,ax; "== 0" ergaebe or al,al.
 * - Stufe per ?: (gemeinsamer Store), (g_cam_rx << 5) - 0xa80 (shl / add 0f580h).
 * - memcpy (-Oi) mit Quelle MK_FP(g_cur_tex.seg, g_cur_tex.off + off + ph): Offset als long in edx -> mov esi,edx.
 * - Sichtliste ueber den Bytezeiger "(signed char far *)g_vis_order + (col << 7) + i" (+ 0x40 fuer dy) wie im Original.
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * Originalfehler: view_draw, Dungeon 5: die Schleife ueber k = 0..3 setzt jedesmal faces[3] statt faces[k] (nur die
 *   vierte Flaeche wird animiert). Auffaellig: Augenhoehe "e + (e - s) * z / (b - a)" fuer e < s.
 */
#include <stdlib.h>
#include <mem.h>
#include <dos.h>
#include "schweif.h"

#define X (g_view_cell_y + dx)
#define Y (g_view_cell_x + dy)
/* get_cell_c mit explizitem Zielzeiger fuer die Rueckgabe-struct (wie im Original) */
#define GET_CELL_ROT(dst, x, y) ((void (*)(struct cell_rot far *, signed short, signed short))get_cell_c)(dst, x, y)

void view_draw(void)
{
	unsigned char c;
	signed short flip;
	signed short col;
	signed short dx;
	signed short dy;
	signed short cell;
	div_t d1;
	div_t d2;
	signed short cnt;
	signed short saved;
	register signed short i;

	g_nr_click = 0;
	d1 = div(g_cam_x, 0xa8);
	d2 = div(g_cam_z, 0xa8);
	g_cam_cell_y = g_view_cell_y = d1.quot;
	g_cam_cell_x = g_view_cell_x = d2.quot;
	g_cam_rx = d1.rem;
	g_cam_ry = 0;
	g_cam_rz = d2.rem;
	g_cam_eye_x = 0;
	g_cam_eye_y = 0;
	g_cam_eye_z = 0;
	cell = cell_profile(g_cam_cell_y, g_cam_cell_x);
	col = ((g_cam_rx << 5) - 0xa80) / 0xa8 + 16;
	{
	signed short a;
	signed short b;
	signed short e;
	signed short z;
	signed short t;
	signed short v;
	struct hero far *h;
	register signed short s;

	z = g_cam_rz - 0x54;
	dy = 10;
	for (i = 0; i < g_prof_count[cell]; i++) {
		a = g_prof_zmin[cell][i][col];
		b = g_prof_zmax[cell][i][col];
		s = g_prof_ymin[cell][i][col];
		e = g_prof_ymax[cell][i][col];
		if (a <= z && b >= z) {
			if (e < s)
				dy = e + (e - s) * z / (b - a);
			else
				dy = s + (e - s) * z / (b - a);
			break;
		}
	}
	h = get_first_active_hero(g_group);
	t = h->height;
	t = (t - 130) * 10 / 25 + 40;
	v = dy - t;
	t = g_group->dungeon;
	if (t == 14 || t == 15 || t == 11)
		;
	else if (v > 40)
		v = 40;
	g_cam_eye_y = v;
	}
	g_cam_rot_x = i + (g_cam_roll >> 3);
	g_cam_rot_y = g_cam_pitch;
	cam_set_angles(&g_cam_rot_x);
	if ((col = (g_cam_angle >> 4) & 0x3f) > 31) {
		col -= 32;
		flip = 1;
	} else {
		flip = 0;
	}
	if (g_area_type == AREA_DUNGEON) {
		register signed short k = g_group->dungeon;

		if (k == 4) {
			if (g_tex_anim_due != 0) {
				g_tex_anim_due = 0;
				g_tex_anim_frame++;
				if (g_tex_anim_frame >= g_tex_anim_frames)
					g_tex_anim_frame = 0;
				g_obj_cur = g_cell_objs[4];
				for (k = 0; k < 4; k++)
					g_obj_cur->faces[k]->tex = g_tex_anim_a[g_tex_anim_frame];
				g_obj_cur = g_cell_objs[7];
				for (k = 0; k < 4; k++)
					g_obj_cur->faces[k]->tex = g_tex_anim_b[g_tex_anim_frame];
				g_obj_cur = g_cell_objs[10];
				g_obj_cur->faces[0]->tex = g_tex_anim_c[g_tex_anim_frame];
			}
		} else if (k == 5) {
			if (g_tex_anim_due != 0) {
				g_tex_anim_due = 0;
				g_tex_anim_frame++;
				if (g_tex_anim_frame >= g_tex_anim_frames)
					g_tex_anim_frame = 0;
				g_obj_cur = g_cell_objs[20];
				for (k = 0; k < 4; k++)
#if FIX_0050
					/* Fix #50: alle vier Flaechen animieren (faces[k]) */
					g_obj_cur->faces[k]->tex = g_tex_anim_a[g_tex_anim_frame];
#else
					g_obj_cur->faces[3]->tex = g_tex_anim_a[g_tex_anim_frame];
#endif
			}
		} else if (k == 6) {
			if (g_tex_anim_due != 0) {
				g_tex_anim_due = 0;
				g_tex_anim_frame++;
				if (g_tex_anim_frame >= g_tex_anim_frames)
					g_tex_anim_frame = 0;
				g_obj_cur = g_cell_objs[15];
				g_obj_cur->faces[0]->tex = g_tex_anim_a[g_tex_anim_frame];
				g_obj_cur = g_cell_objs[16];
				g_obj_cur->faces[0]->tex = g_tex_anim_a[g_tex_anim_frame];
				g_obj_cur = g_cell_objs[17];
				g_obj_cur->faces[0]->tex = g_tex_anim_a[g_tex_anim_frame];
			}
		} else if (k == 1) {
			if (g_tex_anim_due != 0 && g_dng01_ice_state > 0) {
				g_tex_anim_due = 0;
				g_obj_cur = g_cell_objs[18];
				g_obj_cur->phase++;
			}
		} else if (k == 13) {
			if (g_tex_anim_due != 0) {
				g_tex_anim_due = 0;
				g_obj_cur = g_cell_objs[11];
				g_obj_cur->phase++;
			}
		} else if (k == 15) {
			if (g_tex_anim_due != 0) {
				g_tex_anim_due = 0;
				g_obj_cur = g_cell_objs[1];
				g_obj_cur->phase++;
				g_obj_cur = g_cell_objs[6];
				g_obj_cur->phase++;
			}
		}
	} else {
		register signed short k = g_group->town;
		signed short sel = g_tex_anim_due;

		if (k != 3 && sel != 0) {
			g_tex_anim_due = 0;
			g_tex_anim_frame++;
			if (g_tex_anim_frame >= g_tex_anim_frames)
				g_tex_anim_frame = 0;
			g_obj_cur = g_cell_objs[3];
			g_obj_cur->faces[0]->tex = g_tex_anim_a[g_tex_anim_frame];
		}
		if ((k == 2 || k == 3 || k == 4 || k == 6 || k == 5) && sel != 0) {
			g_tex_anim_due = 0;
			g_tex_anim_frame++;
			if (g_tex_anim_frame >= g_tex_anim_frames)
				g_tex_anim_frame = 0;
			if (k == 2)
				sel = 9;
			else if (k == 3)
				sel = 4;
			else if (k == 4)
				sel = 8;
			else if (k == 6)
				sel = 9;
			else if (k == 5)
				sel = 10;
			g_obj_cur = g_cell_objs[sel];
			for (k = 0; k < 4; k++)
				g_obj_cur->faces[k]->tex = g_tex_anim_b[g_tex_anim_frame];
		}
	}
	if (g_area_type == AREA_TOWN) {
		view_draw_sky();
	} else if (g_group->dungeon == 4) {
		if (g_daytime > 0x93a8 && g_daytime <= 0x190c8) {
			fill_scene(-0x62);
		} else {
			fill_scene(0);
			draw_stars();
		}
	} else {
		fill_scene(0);
	}
	for (i = g_vis_start; (dx = *((signed char far *)g_vis_order + (col << 7) + i)) != 0x7f; i++) {
		register unsigned short u;
		unsigned short u2;

		dy = *((signed char far *)g_vis_order + (col << 7) + i + 0x40);
		if (flip > 0) {
			dy = -dy;
			dx = -dx;
		}
		if (X < 0 || X >= 64 || Y < 0 || Y >= 64)
			c = 0xff;
		else
			c = get_cell_a(X, Y);
		u = dx + 4;
		u2 = dy + 4;
		if (u > 8 || u2 > 8)
			continue;
		g_near_objs[u * 9 + u2] = c + 1;
		g_near_occl[u * 9 + u2] = g_cell_occludes == 0 ? 0 : ((unsigned char far *)g_cell_occludes)[c];
	}
	view_cull_hidden();
	snd_service(g_snd_music);
	snd_service(g_snd_fx);
	snd_speech_service(g_snd_speech);
	cnt = 0;
	for (i = g_vis_start; (dx = *((signed char far *)g_vis_order + (col << 7) + i)) != 0x7f; i++) {
		unsigned short u1;
		unsigned short u2;
		signed short j;
		register signed short s;

		dy = *((signed char far *)g_vis_order + (col << 7) + i + 0x40);
		if (flip > 0) {
			dy = -dy;
			dx = -dx;
		}
		if (X < 0 || X >= 64 || Y < 0 || Y >= 64)
			continue;
		u1 = dx + 4;
		u2 = dy + 4;
		if (u1 < 9 && u2 < 9) {
			if (!(c = g_near_objs[u1 * 9 + u2])) {
				cnt++;
				continue;
			}
			c--;
		} else {
			c = get_cell_a(X, Y);
		}
		g_obj_cur = g_cell_objs[c];
		g_obj_cur->x = dx * 0xa8 - g_cam_rx + 0x54;
		g_obj_cur->y = 0;
		g_obj_cur->z = dy * 0xa8 - g_cam_rz + 0x54;
		s = g_obj_cur->nfaces;
		{
			for (j = 0; j < s; j++) {
				if (g_obj_cur->faces[j]->flags & FACE_CLICK) {
					struct map_event far *q = (struct map_event far *)&g_obj_cur->faces[j]->tex;

					q->x = g_view_cell_y + dx;
					q->y = g_view_cell_x + dy;
				}
			}
		}
		{
			s = g_cells[X][Y].flags;

			saved = g_obj_cur->faces[0]->flags;
			if (s & 0x1000) {
				if (s & 0x2000) {
					if ((s & 0x0f) < 15)
						s++;
					else
						s &= 0xefff;
				} else {
					if ((s & 0x0f) > 0)
						s--;
					else
						s &= 0xefff;
				}
			} else if (s & 0x2000) {
				g_obj_cur->faces[0]->flags |= FACE_SOLID;
				cell_forget(X, Y);
				s = 0x200f;
			}
			g_cells[X][Y].flags = s;
			g_obj_cur->phase = s & 0x0f;
			GET_CELL_ROT(&g_obj_cur->rot, X, Y);
			draw_object(g_obj_cur);
			g_obj_cur->faces[0]->flags = saved;
		}
	}
	g_view_frames++;
	draw_hud();
	snd_service(g_snd_music);
	snd_service(g_snd_fx);
	snd_speech_service(g_snd_speech);
}

void view_draw_sky(void)
{
	signed short h2;
	struct face3d far *f;
	signed long tm;
	signed short lvl;
	unsigned char far *p;
	unsigned char far *pal;
	unsigned short off;
	unsigned short dst;
	signed short row;
	signed short ph;
	signed short k;
	signed short rows;
	signed short hh;
	signed short shade;

	if (g_area_type == AREA_TOWN)
		g_obj_cur = g_cell_objs[6];
	else
		g_obj_cur = g_cell_objs[13];
	if (g_daytime > 0x93a8 && g_daytime <= 0x190c8) {
		f = g_obj_cur->faces[0];
		g_cur_tex = f->tex->ref;
		g_cur_tex_w = f->tex->w;
		h2 = f->tex->h;
		if (g_cur_tex.seg == 0) {
			h2 -= 4;
			if (g_ems_avail == 1) {
				EMS_map4(0, g_cur_tex.off >> 14, g_swap_handle);
				g_cur_tex.seg = g_ems_frame;
				g_cur_tex.off &= 0x3fff;
			} else {
				g_cur_tex.off += g_tex_base;
			}
			ph = (g_cam_angle + (g_view_frames >> 1)) % g_cur_tex_w;
			hh = ((g_cam_rot_y << 6) + (g_cam_rot_y << 4)) >> 7;
			k = h2 * 2 - (h2 * 2 + hh) % (h2 * 2);
			off = (k < h2 ? k : h2 * 2 - k) * g_cur_tex_w;
			dst = 0;
			rows = hh + 0x62;
			if (rows > 0x88)
				rows = 0x88;
			tm = g_daytime;
			shade = 0;
			if (tm > 0x17bb0 || tm < 0xa8c0) {
				lvl = tm > 0x17bb0 ? (tm - 0x17bb0) / 0x154 : (0xa8c0 - tm) / 0x154;
				pal = MK_FP(g_shade_segs[lvl + 16], 0);
				shade++;
			}
			for (row = 0; row < rows; row++) {
				if (g_ems_avail == 1) {
					memcpy((char far *)g_renderbuf_ptr + dst, MK_FP(g_cur_tex.seg, g_cur_tex.off + off + ph), 0xd0 - ph + 1);
					memcpy((char far *)g_renderbuf_ptr + dst + 0xd0 - ph, MK_FP(g_cur_tex.seg, g_cur_tex.off + off), ph + 1);
				} else {
					tex_copy_flat(0, g_cur_tex.off + off + ph, FP_SEG(g_renderbuf_ptr), FP_OFF(g_renderbuf_ptr) + dst, 0xd0 - ph + 1);
					tex_copy_flat(0, g_cur_tex.off + off, FP_SEG(g_renderbuf_ptr), FP_OFF(g_renderbuf_ptr) + dst + 0xd0 - ph, ph + 1);
				}
				if (shade) {
					p = (unsigned char far *)g_renderbuf_ptr + dst;
					if (lvl == 15) {
						register signed short m;

						for (m = 0; m < 0xd0; m++, p++) {
							if ((m + row + ph) & 1)
								*p = 0;
							else
								*p = pal[*p];
						}
					} else {
						register signed short m;

						for (m = 0; m < 0xd0; m++, p++)
							*p = pal[*p];
					}
				}
				k++;
				if (h2 * 2 <= k)
					k = 0;
				if (k <= h2)
					off += g_cur_tex_w;
				else
					off -= g_cur_tex_w;
				dst += 320;
			}
			if (shade) {
				if (lvl > 10)
					draw_stars();
				if (g_sky_shade != lvl) {
					fade_table(16, lvl * 2);
					g_sky_shade = lvl;
				}
			} else {
				fade_table(16, 0);
			}
		}
	} else {
		if (g_area_type == AREA_TOWN)
			fog_table(0x20, 0x14);
		fill_scene(0);
		draw_stars();
	}
}
