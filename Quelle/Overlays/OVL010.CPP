/* OVL010 - Overlay O010 (Segment 0x1f2e in SCHWEIF.EXE): 3D-Objekte vorbereiten und die 3D-Ansicht in EMS/XMS
 * auslagern bzw. zurueckholen. view_init: g_face_list anlegen (view_alloc_face_list), Sinustabelle (Datei 0x206 CYBER.SIN) nach
 *   g_sin_table, Zaehler/Kamerawinkel loeschen, vier Profilplaetze g_prof_cells = -1. obj3d_prepare(obj): je Flaeche
 *   Normale aus drei Eckpunkten (Kreuzprodukt, Laenge per long_sqrt >> 8, bewegter Eckpunkt VTX_PATH = erste Lage
 *   aus g_vtx_paths), einfarbige Flaechen: 24-Bit-Farbe in die Palette g_view_pal (bis 160 Farben, sonst
 *   pal_find_nearest) -> Index im Byte +0x17; Huellquader box_min/box_max aus dem groessten Koordinatenbetrag.
 * swap_write(src, n) / swap_read(dst, n): n Bytes an Position g_swap_pos in EMS (g_ems_avail == 1, Seiten EMS_map4,
 *   Rahmen g_ems_frame, Bloecke zu 48K) bzw. XMS (g_xms_move, xms_move_any) schreiben/lesen.
 * view_swap_out: alle 32 Objekte (Punkte, Pfade, Flaechen), Puffer und die vier Rahmenbilder auslagern und
 *   freigeben (Zusatzobjekte 27/19 wie O097 korrigieren). view_swap_in: alles zurueckholen (Bmp neu anlegen,
 *   Schattierungstabellen g_shade_segs an Paragraphengrenze per huge-Zeiger), Palette, town_make_events bzw.
 *   dng_make_events, Kompass.
 * view_suspend: Ansicht auslagern und Hintergrund zeigen (Rahmenbilder per Bmp::put), Bild g_render_bmp/Puffer
 *   g_renderbuf_ptr freigeben. swap_out_screen: bei g_view_active auslagern (1) sonst 0. swap_in_screen:
 *   zurueckholen, Stadtname (g_ttx[town + 0x163]) bei g_pp20_index == 4. swap_save_file: Auslagerungsspeicher in
 *   die Arbeitsdatei 0x248 (FREEZE.CYB, g_ofile, O243) sichern und EMS/XMS freigeben; swap_alloc: EMS/XMS anlegen (g_swap_kb KB)
 *   und ggf. aus der Datei zurueckladen. view_timer_off / view_timer_on: Zeitgeber der 3D-Ansicht (AIL, Rueckruf
 *   R23:01b0, Zaehler g_ail_timer_users). tex_debug_view: Testanzeige der Texturen g_textures (Pfeile blaettern,
 *   Esc). ems_check: EMM-Treiber pruefen ("EMMXXXX0" g_str_emm an INT 67h:000a, Status 40h).
 *   ems_frame_seg/ems_free_pages/ems_alloc/ems_free: EMS 41h (Rahmensegment), 42h (freie Seiten), 43h (anlegen), 45h
 *   (freigeben).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL010.CPP   -> 6571/6571 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (g_vtx_paths -> _g_94c4+2, g_shade_segs -> _g_85e0+4). Kein -Oi (memcpy/memset/strncmp sind echte Aufrufe).
 * - EMS-Funktionen: "mov al,ah" ist 88 E0 = BCCs eingebauter Assembler (C ergaebe 8A C4) -> asm-Zeilen, Ergebnis
 *   per "return _AX;" / "return _BX;" (toter Epilog).
 * - obj3d_prepare: Flaeche +0x14 als long (FACE_COL) bzw. Byte +0x17 (FACE_IDX, ueberlappt die Farbe);
 *   Pfadzeiger je Eckpunkt in eigenem Block (gemeinsamer Platz [bp-4ch] hinter den -Og-Werten);
 *   "(signed long)a.y * b.z - a.z * b.y" (16-Bit-Subtraktion), "(signed short)FACE_COL(p) >> 8" (sar ax),
 *   abs() inline (Kopien [bp-36h..-3ah]); n (DI) dient zuerst als Flaechenzahl, dann als Betrag.
 * - swap_read: "unsigned short last" (2-Byte-XMS-Ziel), "(unsigned long)(void far *)&last" (shl/shrd edx; mit
 *   (signed long) speichert BCC das signed-long-Feld dst als Zeiger in zwei Worten).
 * - view_timer_on: Rueckruf "MK_FP(FP_SEG(r23_000f), 0x1b0)" wie OVL134.
 * - texture.w/h sind unsigned (O097): tex_debug_view rechnet signed (Casts). g_view_swap_pos als long (VIEW_SWAP_POS).
 * - EMS-Funktionen bleiben Original-Assembler (asm-Zeilen unveraendert).
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * Auffaellig (Original): swap_write (EMS) erhoeht src pro 48K-Block nicht (kopiert immer den Anfang);
 *   swap_read (EMS) kopiert in der Schleife (unsigned)n statt 0xc000 Bytes und erhoeht dst nicht; swap_read
 *   (XMS, ungerade Laenge) ueberschreibt den Auftrag fuer den Hauptteil vor dem xms_move_any-Aufruf (Hauptteil wird
 *   nie gelesen) und schreibt dst[len] = last vor dem Lesen von last; ems_check/ems_alloc setzen "mov al,0" statt
 *   "mov ah,0" (Status landet in AH der Rueckgabe).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mem.h>
#include <dos.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:050e..059c, dsana) - initialisierte Daten in Adressreihenfolge */
struct str9 g_str_emm = { "EMMXXXX0" };
char g_err_swap_write[41] = "Error moving memory into XMS (MoveToXMS)";
char g_err_swap_read[43] = "Error moving memory from XMS (MoveFromXMS)";
char g_fmt_tex_debug[49] = "Texture %d, XRes %d, YRes %d, Colors %d";
/* Phase B: eigenes BSS (ds:8b52..8b56) in Adressreihenfolge */
signed char g_saved_area_type;
signed short g_view_timer;

/* einfarbige Flaeche: 24-Bit-Farbe aus der Objektdatei in face3d.tex, Byte +0x17 danach Palettenindex */
#define FACE_COL(f) (*(signed long far *)&(f)->tex)
#define FACE_IDX(f) (((unsigned char far *)&(f)->tex)[3])
/* g_view_swap_pos als signed long (Lage der ausgelagerten Ansicht) */
#define VIEW_SWAP_POS (*(signed long *)&g_view_swap_pos)

signed short view_init(void)
{
	register signed short i;

	if (!view_alloc_face_list())
		return 0;
	file_load(g_file_main, 0x206, g_sin_table);
	g_nr_textures = 0;
	g_cam_ang[0] = 0;
	g_cam_ang_y = 0;
	g_cam_ang_z = 0;
	g_cam_eye_x = 0;
	g_cam_eye_y = 0;
	g_cam_eye_z = 0;
	g_cam_fx = g_cam_fy = g_cam_fz = 0;
	for (i = 0; i < 4; i++)
		g_prof_cells[i].x = g_prof_cells[i].y = -1;
	return 1;
}

signed short obj3d_prepare(struct obj3d far *o)
{
	signed short i;
	signed short max;
	struct face3d far *p;
	struct vtx v0;
	struct vtx v1;
	struct vtx v2;
	struct vtx a;
	struct vtx b;
	signed long len;
	register signed short n;

	n = o->nfaces;
	for (i = 0; i < n; i++) {
		p = o->faces[i];
		v0 = *(struct vtx far *)&o->vl->v[p->idx[0]];
		v1 = *(struct vtx far *)&o->vl->v[p->idx[1]];
		v2 = *(struct vtx far *)&o->vl->v[p->idx[2]];
		if (v0.x == VTX_PATH) {
			struct vtx_path far *s;

			s = g_vtx_paths[o->vl->v[p->idx[0]].y];
			v0 = s->e[0];
		}
		if (v1.x == VTX_PATH) {
			struct vtx_path far *s;

			s = g_vtx_paths[o->vl->v[p->idx[1]].y];
			v1 = s->e[0];
		}
		if (v2.x == VTX_PATH) {
			struct vtx_path far *s;

			s = g_vtx_paths[o->vl->v[p->idx[2]].y];
			v2 = s->e[0];
		}
		if (v0.x != VTX_PATH && v1.x != VTX_PATH && v2.x != VTX_PATH) {
			a.x = v1.x - v0.x;
			a.y = v1.y - v0.y;
			a.z = v1.z - v0.z;
			b.x = v2.x - v0.x;
			b.y = v2.y - v0.y;
			b.z = v2.z - v0.z;
			v0.x = (signed long)a.y * b.z - a.z * b.y;
			v0.y = (signed long)a.z * b.x - a.x * b.z;
			v0.z = (signed long)a.x * b.y - a.y * b.x;
			len = (unsigned long)long_sqrt((signed long)v0.x * v0.x + (signed long)v0.y * v0.y
				+ (signed long)v0.z * v0.z) >> 8;
			if (len == 0)
				len++;
			p->nx = (signed long)v0.x / len;
			p->ny = (signed long)v0.y / len;
			p->nz = (signed long)v0.z / len;
		}
		if (p->flags & FACE_TEXTURED)
			;
		else {
			if (g_view_pal_n < 160) {
				g_view_pal[g_view_pal_n].r = FACE_COL(p);
				g_view_pal[g_view_pal_n].g = (signed short)FACE_COL(p) >> 8;
				g_view_pal[g_view_pal_n].b = FACE_COL(p) >> 16;
				FACE_IDX(p) = g_view_pal_n;
				g_view_pal_n++;
			} else
				FACE_IDX(p) = pal_find_nearest((struct rgb far *)&FACE_COL(p), 0);
		}
	}
	max = 0;
	for (i = 0; o->vl->n > i; i++) {
		n = abs(o->vl->v[i].x);
		if (n != VTX_PATH) {
			if (max < n)
				max = n;
			n = abs(o->vl->v[i].y);
			if (max < n)
				max = n;
			n = abs(o->vl->v[i].z);
			if (max < n)
				max = n;
		}
	}
	o->box_min.x = -max;
	o->box_min.y = -max;
	o->box_min.z = max;
	o->box_max.x = max;
	o->box_max.y = max;
	o->box_max.z = max;
	o->phase = 0;
	return 1;
}

void swap_write(void far *src, unsigned long n)
{
	unsigned long len = n;

	if (g_ems_avail == 1) {
		while (n > 0xc000) {
			EMS_map4(0, g_swap_pos >> 14, g_swap_handle);
#if FIX_0198
			/* Fix #198: Quelle je 48-KB-Block weiterzaehlen */
			memcpy(MK_FP(g_ems_frame, g_swap_pos & 0x3fff), src, 0xc000);
			src = (char huge *)src + 0xc000;
			g_swap_pos += 0xc000;
#else
			memcpy(MK_FP(g_ems_frame, g_swap_pos & 0x3fff), src, 0xc000);
			g_swap_pos += 0xc000;
#endif
			n -= 0xc000;
		}
		EMS_map4(0, g_swap_pos >> 14, g_swap_handle);
		memcpy(MK_FP(g_ems_frame, g_swap_pos & 0x3fff), src, n);
	} else {
		if (len & 1)
			len++;
		g_xms_move.src_handle = 0;
		g_xms_move.src = (signed long)src;
		g_xms_move.dst_handle = g_swap_handle;
		g_xms_move.dst = g_swap_pos;
		g_xms_move.len = len;
		if (xms_move_any(&g_xms_move))
			guru_meditation(g_err_swap_write, 0);
	}
	g_swap_pos = g_swap_pos + n;
}

void swap_read(void far *dst, unsigned long n)
{
	unsigned long len = n;
	unsigned short last;

	if (g_ems_avail == 1) {
		while (n > 0xc000) {
			EMS_map4(0, g_swap_pos >> 14, g_swap_handle);
#if FIX_0198
			/* Fix #198: je Block 0xc000 Bytes lesen und Ziel weiterzaehlen */
			memcpy(dst, MK_FP(g_ems_frame, g_swap_pos & 0x3fff), 0xc000);
			dst = (char huge *)dst + 0xc000;
			g_swap_pos += 0xc000;
#else
			memcpy(dst, MK_FP(g_ems_frame, g_swap_pos & 0x3fff), n);
			g_swap_pos += 0xc000;
#endif
			n -= 0xc000;
		}
		EMS_map4(0, g_swap_pos >> 14, g_swap_handle);
		memcpy(dst, MK_FP(g_ems_frame, g_swap_pos & 0x3fff), n);
	} else {
#if FIX_0198
		/* Fix #198: ungerade Laenge: erst den Hauptteil lesen, dann die 2 Bytes, last danach eintragen */
		if (len & 1) {
			len--;
			if (len) {
				g_xms_move.dst_handle = 0;
				g_xms_move.dst = (signed long)dst;
				g_xms_move.src_handle = g_swap_handle;
				g_xms_move.src = g_swap_pos;
				g_xms_move.len = len;
				if (xms_move_any(&g_xms_move))
					guru_meditation(g_err_swap_read, 0);
			}
			g_xms_move.dst_handle = 0;
			g_xms_move.dst = (unsigned long)(void far *)&last;
			g_xms_move.src_handle = g_swap_handle;
			g_xms_move.src = g_swap_pos + len;
			g_xms_move.len = 2;
			if (xms_move_any(&g_xms_move))
				guru_meditation(g_err_swap_read, 0);
			((unsigned char far *)dst)[len] = (unsigned char)last;
			g_swap_pos = g_swap_pos + n;
			return;
		} else {
#else
		if (len & 1) {
			len--;
			g_xms_move.dst_handle = 0;
			g_xms_move.dst = (signed long)dst;
			g_xms_move.src_handle = g_swap_handle;
			g_xms_move.src = g_swap_pos;
			g_xms_move.len = len;
			g_xms_move.dst_handle = 0;
			g_xms_move.dst = (unsigned long)(void far *)&last;
			g_xms_move.src_handle = g_swap_handle;
			g_xms_move.src = g_swap_pos + len;
			g_xms_move.len = 2;
			((unsigned char far *)dst)[len] = last;
		} else {
#endif
			g_xms_move.dst_handle = 0;
			g_xms_move.dst = (signed long)dst;
			g_xms_move.src_handle = g_swap_handle;
			g_xms_move.src = g_swap_pos;
			g_xms_move.len = len;
		}
		if (xms_move_any(&g_xms_move))
			guru_meditation(g_err_swap_read, 0);
	}
	g_swap_pos = g_swap_pos + n;
}

void view_swap_out(void)
{
	signed short i;
	signed short k;
	struct obj3d far *o;
	struct vtx_path far *s;
	register signed short j;

	g_saved_area_type = g_area_type;
	VIEW_SWAP_POS = g_swap_pos;
	if (g_phex_finale_done == 1 && !g_mem_saving && g_area_type == AREA_TOWN && g_group->town == 5) {
		o = g_cell_objs[27];
		o->nfaces += 2;
	}
	if (g_dng06_dogs_state == 1 && g_area_type == AREA_DUNGEON && g_group->dungeon == 6) {
		o = g_cell_objs[19];
		o->nfaces += 2;
	}
	for (g_swap_nr_objs = i = 0; i <= 31; i++) {
		if ((o = g_cell_objs[i]) != 0) {
			swap_write(o, 0x2cL);
			swap_write(o->vl, o->vl->n * 8U + 2);
			for (j = 0; o->vl->n > j; j++) {
				if (o->vl->v[j].x == VTX_PATH) {
					s = g_vtx_paths[o->vl->v[j].y];
					swap_write(s, 2L);
					swap_write(s, s->n * 8U + 4);
					mem_free(s);
				}
			}
			mem_free(o->vl);
			for (k = 0; o->nfaces > k; k++) {
				swap_write(o->faces[k], 0x48L);
				mem_free(o->faces[k]);
			}
			mem_free(o->faces);
			g_cell_objs[i] = (struct obj3d far *)mem_free(o);
			g_swap_nr_objs++;
		}
	}
	g_cell_objs = (struct obj3d far * far *)mem_free(g_cell_objs);
	swap_write(g_shade_table, 0x200L);
	g_shade_table = (unsigned short far *)mem_free(g_shade_table);
	if (g_area_type == AREA_TOWN) {
		Town_dtor(g_town_events, 3);
		g_town_events = 0;
	} else if (g_area_type == AREA_DUNGEON) {
		Event_dtor(g_area_events, 3);
		g_area_events = 0;
	}
	Compass_dtor(g_compass, 3);
	g_compass = 0;
	swap_write(g_shade_buf, 0x2010L);
	mem_free(g_shade_buf);
	swap_write(g_cells, 0x4000L);
	mem_free(g_cells);
	mem_free(g_face_list);
	mem_free(g_vis_order);
	if (g_area_type == AREA_TOWN) {
		swap_write(g_hud_top_left->data(), 0x41eL);
		swap_write(g_hud_top_right->data(), 0x30cL);
		swap_write(g_hud_bottom_left->data(), 9L);
		swap_write(g_hud_bottom_right->data(), 0x284L);
	} else {
		swap_write(g_hud_top_left->data(), 0x138L);
		swap_write(g_hud_top_right->data(), 0xa8cL);
		swap_write(g_hud_bottom_left->data(), 0x8f0L);
		swap_write(g_hud_bottom_right->data(), 0xf2L);
	}
	g_view_swap_end = g_swap_pos;
}

void view_swap_in(void)
{
	signed short m;
	signed short i;
	signed short k;
	signed short n;
	struct obj3d far *o;
	struct vtx_path far *s;
	unsigned char huge *p;
	register signed short j;

	g_area_type = g_saved_area_type;
	if (g_renderbuf_ptr == 0)
		g_renderbuf_ptr = mem_alloc(0xaa00L);
	if (g_render_bmp == 0)
		g_render_bmp = new Bmp(0x140, 0x87, g_renderbuf_ptr);
	g_cell_objs = (struct obj3d far * far *)mem_alloc(0x80L);
	g_swap_pos = VIEW_SWAP_POS;
	for (i = 0; i < g_swap_nr_objs; i++) {
		o = g_cell_objs[i] = (struct obj3d far *)mem_alloc(0x2cL);
		swap_read(o, 0x2cL);
		swap_read(&n, 2L);
		o->vl = (struct vtx_list far *)mem_alloc(n * 8U + 2);
		o->vl->n = n;
		swap_read(&o->vl->v[0].x, n * 8U);
		for (j = 0; o->vl->n > j; j++) {
			if (o->vl->v[j].x == VTX_PATH) {
				swap_read(&m, 2L);
				s = g_vtx_paths[o->vl->v[j].y] = (struct vtx_path far *)mem_alloc(m * 8U + 4);
				swap_read(s, m * 8U + 4);
			}
		}
		o->faces = (struct face3d far * far *)mem_alloc(o->nfaces * 4);
		for (k = 0; o->nfaces > k; k++) {
			o->faces[k] = (struct face3d far *)mem_alloc(0x48L);
			swap_read(o->faces[k], 0x48L);
		}
	}
	for (i = g_swap_nr_objs; i <= 31; i++)
		g_cell_objs[i] = 0;
	if (g_phex_finale_done == 1 && !g_mem_saving && g_area_type == AREA_TOWN && g_group->town == 5) {
		o = g_cell_objs[27];
		o->nfaces -= 2;
	}
	if (g_dng06_dogs_state == 1 && g_area_type == AREA_DUNGEON && g_group->dungeon == 6) {
		o = g_cell_objs[19];
		o->nfaces -= 2;
	}
	g_shade_table = (unsigned short far *)mem_alloc(0x200L);
	swap_read(g_shade_table, 0x200L);
	g_shade_buf = mem_alloc(0x2010L);
	swap_read(g_shade_buf, 0x2010L);
	p = (unsigned char huge *)g_shade_buf;
	p += 0x10 - FP_OFF(p);
	for (i = 0; i < 32; i++)
		g_shade_segs[i] = FP_SEG(p) + (i << 4);
	g_cells = (struct map_cell (far *)[MAP_SIZE])mem_alloc(0x4000L);
	swap_read(g_cells, 0x4000L);
	if (g_area_type == AREA_TOWN) {
		if (g_hud_top_left == 0) {
			g_hud_top_left = new Bmp(0x22, 0x1f);
			g_hud_top_right = new Bmp(0x1a, 0x1e);
			g_hud_bottom_left = new Bmp(3, 3);
			g_hud_bottom_right = new Bmp(0x17, 0x1c);
		}
		swap_read(g_hud_top_left->data(), 0x41eL);
		swap_read(g_hud_top_right->data(), 0x30cL);
		swap_read(g_hud_bottom_left->data(), 9L);
		swap_read(g_hud_bottom_right->data(), 0x284L);
	} else {
		if (g_hud_top_left == 0) {
			g_hud_top_left = new Bmp(0x18, 0x0d);
			g_hud_top_right = new Bmp(0x2d, 0x3c);
			g_hud_bottom_left = new Bmp(0x2c, 0x34);
			g_hud_bottom_right = new Bmp(0x0b, 0x16);
		}
		swap_read(g_hud_top_left->data(), 0x138L);
		swap_read(g_hud_top_right->data(), 0xa8cL);
		swap_read(g_hud_bottom_left->data(), 0x8f0L);
		swap_read(g_hud_bottom_right->data(), 0xf2L);
	}
	view_alloc_face_list();
	g_swap_pos = VIEW_SWAP_POS;
	VIEW_SWAP_POS = 0;
	view_load_vis_order();
	set_palette(g_view_pal, 0, 0x100);
	if (g_area_type == AREA_TOWN) {
		town_make_events();
		g_compass = Compass_ctor(0, 1);
	} else if (g_area_type == AREA_DUNGEON) {
		dng_make_events();
		g_compass = Compass_ctor(0, 2);
	}
}

void view_suspend(void)
{
	g_view_input_on = 0;
	g_mouse_handler_on = 0;
	mouse_set_cursor(0, 0);
	view_swap_out();
	if (g_pp20_index == 4 && g_renderbuf_ptr != 0) {
		fill_scene(0);
		if (g_area_type == AREA_TOWN) {
			g_hud_top_left->put(g_renderbuf_ptr, 0, 0, 4);
			g_hud_top_right->put(g_renderbuf_ptr, 0xb7, 0, 4);
			g_hud_bottom_left->put(g_renderbuf_ptr, 0, 0x84, 4);
			g_hud_bottom_right->put(g_renderbuf_ptr, 0xba, 0x6b, 4);
			view_free_frame();
		} else {
			g_hud_top_left->put(g_renderbuf_ptr, 0, 0, 4);
			g_hud_top_right->put(g_renderbuf_ptr, 0xa3, 0, 4);
			g_hud_bottom_left->put(g_renderbuf_ptr, 0, 0x52, 4);
			g_hud_bottom_right->put(g_renderbuf_ptr, 0xc5, 0x70, 4);
			view_free_frame();
		}
		mouse_hide(0);
		copy_3d_to_screen();
		mouse_show(0);
	} else
		view_free_frame();
	view_free_lists();
	if (g_render_bmp != 0) {
		delete g_render_bmp;
		g_render_bmp = 0;
	}
	if (g_renderbuf_ptr != 0)
		g_renderbuf_ptr = mem_free(g_renderbuf_ptr);
}

signed short swap_out_screen(void)
{
	g_c6c2 = 0;
	snd_obj_stop(g_snd_fx);
	Speech_stop(g_snd_speech);
	if (g_view_active != 0 && VIEW_SWAP_POS == 0) {
		view_suspend();
		return 1;
	}
	return 0;
}

void swap_in_screen(void)
{
	view_alloc_lists();
	view_swap_in();
	if (g_area_type == AREA_DUNGEON)
		;
	else if (g_pp20_index == 4)
		screen_textbar(g_screen, g_ttx[g_group->town + 0x163], 0xff);
	g_view_input_on = 1;
	install_cursor();
}

void swap_save_file(void)
{
	signed long left;
	signed long chunk;
	signed long off;
	void far *buf;

	if (VIEW_SWAP_POS != 0) {
		*(char far * far *)g_names_star_idx = file_getname(g_file_main, 0x248);
		ofile_create(g_ofile, 0, 0x8102);
		left = (signed long)g_swap_kb << 10;
		off = 0;
		if (g_ems_avail == 1) {
			while (left != 0) {
				chunk = left > 0x10000L ? 0x10000L : left;
				EMS_map4(0, off >> 14, g_swap_handle);
				ofile_write(g_ofile, MK_FP(g_ems_frame, 0), chunk);
				left -= chunk;
				off += chunk;
			}
			ems_free(g_swap_handle);
		} else {
			buf = mem_alloc(0x4e20L);
			while (left != 0) {
				chunk = left > 0x4e20 ? 0x4e20L : left;
				g_xms_move.len = chunk;
				g_xms_move.src_handle = g_swap_handle;
				g_xms_move.src = off;
				g_xms_move.dst_handle = 0;
				g_xms_move.dst = (signed long)buf;
				xms_move_any(&g_xms_move);
				ofile_write(g_ofile, buf, chunk);
				left -= chunk;
				off += chunk;
			}
			mem_free(buf);
			xms_unlock(g_swap_handle);
			flat_leave();
			xms_free_block(g_swap_handle);
		}
		ofile_close(g_ofile);
	} else if (g_ems_avail == 1)
		ems_free(g_swap_handle);
	else {
		xms_unlock(g_swap_handle);
		flat_leave();
		xms_free_block(g_swap_handle);
	}
}

void swap_alloc(void)
{
	signed long left;
	signed long chunk;
	signed long off;
	void far *buf;

	if (g_ems_avail == 1) {
		ems_alloc((((signed long)g_swap_kb << 10) + 0x3fff) >> 14, &g_swap_handle);
		g_ems_frame = ems_frame_seg();
	} else
		g_swap_handle = xms_alloc_kb(g_swap_kb);
	g_swap_pos = 0;
	if (VIEW_SWAP_POS != 0) {
		if (g_mem_saving == 1)
			g_swap_handle2 = g_swap_handle;
		*(char far * far *)g_names_star_idx = file_getname(g_file_main, 0x248);
		ofile_open(g_ofile, 0);
		left = (signed long)g_swap_kb << 10;
		off = 0;
		if (g_ems_avail == 1) {
			while (left != 0) {
				chunk = left > 0x10000L ? 0x10000L : left;
				EMS_map4(0, off >> 14, g_swap_handle);
				ofile_read(g_ofile, MK_FP(g_ems_frame, 0), chunk);
				left -= chunk;
				off += chunk;
			}
		} else {
			buf = mem_alloc(0x4e20L);
			while (left != 0) {
				chunk = left > 0x4e20 ? 0x4e20L : left;
				ofile_read(g_ofile, buf, chunk);
				g_xms_move.len = chunk;
				g_xms_move.dst_handle = g_swap_handle;
				g_xms_move.dst = off;
				g_xms_move.src_handle = 0;
				g_xms_move.src = (signed long)buf;
				xms_move_any(&g_xms_move);
				left -= chunk;
				off += chunk;
			}
			mem_free(buf);
			g_tex_base = (signed long)xms_lock(g_swap_handle);
			flat_enter();
		}
		ofile_close(g_ofile);
	}
}

void view_timer_off(void)
{
	AIL_release_timer_handle(g_view_timer);
	if (!(g_ail_timer_users -= 1))
		AIL_shutdown(0, 0);
}

void view_timer_on(void)
{
	if (!g_ail_timer_users++)
		AIL_startup();
	g_view_timer = AIL_register_timer(MK_FP(FP_SEG(r23_000f), 0x1b0));
	AIL_set_timer_frequency(g_view_timer, 100L);
	AIL_start_timer(g_view_timer);
}

void tex_debug_view(void)
{
	struct texture far *im;
	register signed short y;
	register signed short i;

	i = 0;
	do {
		memset(MK_FP(0xa000, 0), 0, 0xfa00);
		im = &g_textures[i];
		if (g_ems_avail == 0) {
			for (y = 0; (signed short)im->h > y; y++)
				if (im->ref.seg == 0)
					tex_copy_flat(0, g_tex_base + im->ref.off + y * (signed short)im->w, 0xa000, y * 320, (signed short)im->w);
		} else {
			EMS_map4(0, im->ref.off >> 14, g_swap_handle);
			for (y = 0; (signed short)im->h > y; y++)
				memcpy(MK_FP(0xa000, y * 320), MK_FP(g_ems_frame, (im->ref.off & 0x3fff) + y * (signed short)im->w), (signed short)im->w);
		}
		sprintf(g_text_buf2, g_fmt_tex_debug, i, im->w, im->h, im->nr_colors);
		screen_textbar(g_screen, g_text_buf2, 0xff);
		do
			poll_input();
		while (g_action == 0);
		if (g_action == KEY_UP) {
			if (g_nr_textures - 1 > i)
				i++;
		} else if (g_action == KEY_DOWN) {
			if (i)
				i--;
		}
	} while (g_action != KEY_ESC);
}

signed short ems_check(void)
{
	char far *p;
	struct str9 name = g_str_emm;

	p = (char far *)getvect(0x67);
	((unsigned short *)&p)[0] = 10;
	if (strncmp(p, name.s, 8))
		return 1;
	asm mov ah, 40h
	asm int 67h
	asm mov al, ah
	asm mov al, 0
	return _AX;
}

signed short ems_frame_seg(void)
{
	asm mov ah, 41h
	asm int 67h
	return _BX;
}

signed short ems_free_pages(void)
{
	asm mov ah, 42h
	asm int 67h
	return _BX;
}

signed short ems_alloc(signed short n, void far *h)
{
	asm mov ah, 43h
	asm mov bx, n
	asm int 67h
	asm mov al, ah
	asm mov al, 0
	*(signed short far *)h = _DX;
	return _AX;
}

signed short ems_free(signed short h)
{
	asm mov ah, 45h
	asm mov dx, h
	asm int 67h
	asm mov al, ah
	asm mov ah, 0
	return _AX;
}
