/* OVL096 - Overlay O096 (Segment 0x212f in SCHWEIF.EXE): Puffer und Tabellen der 3D-Ansicht anlegen/freigeben.
 * view_alloc_lists: Flaechenblock 0x61c Bytes (mem_alloc, sonst guru_meditation), Segment + 1 nach g_face_seg,
 *   darin drei Listen im Abstand 0x102 (g_face_next, g_face_depth, g_vtx_buf). view_free_lists: diesen Block freigeben.
 * view_alloc: Puffer anlegen (Bild-Cache g_textures/g_tex_names je 0x2dc, 32 Objektzeiger g_cell_objs = 0,
 *   g_shade_table 0x200, Zellentabelle g_cells 0x4000), Zaehler loeschen, view_alloc_lists; g_view_active = 1.
 * view_leave: Ansicht verlassen - Zustand zuruecksetzen, Rahmenbilder (gb_blit_m, Lage je nach g_area_type) in den
 *   Hintergrund, Bild/Objekte zerstoeren (Destruktoren mit Flag 3), alle Objekte (obj3d_free) und Puffer freigeben.
 * view_alloc_face_list: g_face_list 0x912 Bytes. obj3d_alloc(o, n, m): Punktliste mit n * 8 + 2 Bytes und m
 *   Flaechen zu je 0x48 Bytes anlegen (0 bei Speichermangel). obj3d_free(o): freigeben.
 * view_load_vis_order: Sichtliste (Datei 0x405 VISIBLE.TAB) in einen 0x1000-Puffer laden (file_load). view_init_stars: 100
 *   Zufallssterne (g_stars) und acht Grautoene (g_star_colors). view_load_palette: Schattierungswerte aus Datei
 *   0x13d (CYBER.DAT, g_file_main) nach g_shade_table, Schattierungstabellen 32 x 256 Bytes an Paragraphengrenze (g_shade_segs),
 *   je nach Lage aus der Datei gelesen oder berechnet: Stufe 0..15 heller, 16..31 dunkler, jeweils naechste
 *   Palettenfarbe ueber pal_find_nearest. pal_set_rgb(pal, i, r, g, b): Palettenfarbe setzen.
 * pal_find_nearest(rgb, start): naechste Farbe (Summe der Betragsdifferenzen) in g_view_pal -> Index | Abstand << 8.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL096.C   -> 2630/2630 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - pal_find_nearest ist Inline-Assembler im Original: die Registerformen "add si,di", "sub ah,al", "mov dl,ah" usw. sind
 *   mit Richtungsbit 0 kodiert (01/28/88 statt 03/2A/8A) = BCCs EINGEBAUTER Assembler (ohne -B, ohne
 *   #pragma inline); TASM wuerde die andere Kodierung waehlen. Lokale Variable als "word ptr best"/"byte ptr
 *   best+1", Tabelle als "offset g_view_pal", Rueckgabe in C (return best -> toter Epilog mit -Og).
 * - view_alloc_face_list: "if (..) return 0; return 1;" (mit else zwei tote Epiloge statt einem).
 * - view_load_palette: "unsigned char huge *p; p += 0x10 - FP_OFF(p)" -> PADA@; Zugriff ueber (unsigned char far *)p;
 *   "(p = g_shade_buf = mem_alloc(..)) == 0" eingebettet (or ax,dx); Farbe als 3-Byte-struct (Kopie aus ds per
 *   SCOPY@); Zuschlag (15 - j) * 2 bzw. j * 2 - 32 als -Og-Byte-Zwischenwert [bp-15h]/[bp-16h]; Klemmen nach
 *   unten mit "(signed short)(signed char)c.r < 0" (cbw / or ax,ax; nur (signed char) ergibt cmp byte);
 *   "idx = (unsigned char)r" (mov al / mov ah,0; "r & 0xff" ergibt and ax,0ffh); i signed (jl), die inneren
 *   Schleifen vergleichen gegen unsigned n (jnc); memset -> rep stosw (-Oi).
 * - view_leave: Zuweisungskette "g_view_input_on = g_step_active = g_step_turn = g_cam_speed = g_step_pending = g_view_active = 0"
 *   (xor ax / al / cbw ..).
 * - g_shade_segs, g_star_colors, g_stars, g_vtx_paths ohne eigenes Public (cobjfix ordnet zu).
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * Originalfehler: pal_find_nearest addiert start viermal statt dreimal (Eintraege sind 3 Bytes; alle Aufrufe mit 0)
 *   und vergleicht "cmp di,g_view_pal_n" direkt gefolgt von "cmp di,0a0h" -> sucht immer 160 Farben statt
 *   g_view_pal_n. view_leave setzt g_area_type = 0 und prueft danach == 1 bzw. == 2 -> die beiden
 *   "+= 2"-Korrekturen der Objekte 27 und 19 werden nie ausgefuehrt.
 */
#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5e78..5eb2, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

void view_alloc_lists(void)
{
	void far *p;

	if ((p = mem_alloc(0x61c)) == 0) guru_meditation("OUT OF MEMORY", 0);
	g_face_seg = FP_SEG(p) + 1;
	g_face_block = p;
	g_face_next = MK_FP(g_face_seg, 0);
	g_face_depth = MK_FP(g_face_seg, FP_OFF(g_face_next) + 0x102);
	g_vtx_buf = MK_FP(g_face_seg, FP_OFF(g_face_depth) + 0x102);
}

void view_free_lists(void)
{
	if (g_face_block) g_face_block = mem_free(g_face_block);
}

void view_alloc(void)
{
	register signed short i;

	r39_015e();
	g_view_active = 1;
	g_textures = mem_alloc(0x2dc);
	g_tex_names = mem_alloc(0x2dc);
	g_cell_objs = mem_alloc(0x80);
	g_shade_table = mem_alloc(0x200);
	g_cells = mem_alloc(0x4000);
	for (i = 0; i <= 31; i++) g_cell_objs[i] = 0;
	g_tex_base = g_swap_pos = 0;
	g_swap_used_kb = 0;
	view_alloc_lists();
}

#if FIX_0017
/* Fix #17: Ortsart vor dem Zuruecksetzen merken */
void view_leave(void)
{
	struct obj3d far *p;
	register signed short i;
	signed short old_area;
#else
void view_leave(void)
{
	struct obj3d far *p;
	register signed short i;
#endif

	g_view_input_on = g_step_active = g_step_turn = g_cam_speed = g_step_pending = g_view_active = 0;
	g_swap_pos = 0;
	g_loaded_town = -1;
	g_mouse_handler_on = 0;
	mouse_set_cursor(0, 0);
	if (g_ems_avail == 0) {
		xms_unlock(g_swap_handle);
		flat_leave();
	}
	view_timer_off();
	if (g_renderbuf_ptr) {
		fill_scene(0);
		if (g_hud_top_left) {
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
		}
		mouse_hide(0);
		copy_3d_to_screen();
		mouse_show(0);
		gb_delete(g_render_bmp, 3);
		g_render_bmp = 0;
		g_renderbuf_ptr = mem_free(g_renderbuf_ptr);
	}
#if FIX_0017
	/* Fix #17: Ortsart vor dem Zuruecksetzen merken */
	view_free_frame();
	old_area = g_area_type;
	g_area_type = AREA_NONE;
#else
	view_free_frame();
	g_area_type = AREA_NONE;
#endif
	if (g_area_events) {
		Event_dtor(g_area_events, 3);
		g_area_events = 0;
	} else if (g_town_events) {
		Town_dtor(g_town_events, 3);
		g_town_events = 0;
	}
	if (!g_view_swap_pos) {
		if (g_compass) {
			Compass_dtor(g_compass, 3);
			g_compass = 0;
		}
#if FIX_0017
		/* Fix #17: Pruefung auf die gemerkte Ortsart */
		if (g_phex_finale_done == 1 && !g_mem_saving && old_area == AREA_TOWN && g_group->town == 5) {
#else
		if (g_phex_finale_done == 1 && !g_mem_saving && g_area_type == AREA_TOWN && g_group->town == 5) {
#endif
			p = g_cell_objs[27];
			p->nfaces += 2;
		}
#if FIX_0017
		/* Fix #17: Pruefung auf die gemerkte Ortsart */
		if (g_dng06_dogs_state == 1 && old_area == AREA_DUNGEON && g_group->dungeon == 6) {
#else
		if (g_dng06_dogs_state == 1 && g_area_type == AREA_DUNGEON && g_group->dungeon == 6) {
#endif
			p = g_cell_objs[19];
			p->nfaces += 2;
		}
		for (i = 0; i <= 31; i++) {
			p = g_cell_objs[i];
			if (p) {
				obj3d_free(p);
				mem_free(p);
			}
		}
		mem_free(g_cells);
		mem_free(g_face_list);
		mem_free(g_shade_buf);
		mem_free(g_vis_order);
		for (i = 0; i < g_nr_vtx_paths; i++) g_vtx_paths[i] = mem_free(g_vtx_paths[i]);
		mem_free(g_shade_table);
		mem_free(g_cell_objs);
	}
	g_view_swap_pos = 0;
	mem_free(g_tex_names);
	mem_free(g_textures);
	view_free_lists();
	r39_017a();
}

signed short view_alloc_face_list(void)
{
	if ((g_face_list = mem_alloc(0x912)) == 0) return 0;
	return 1;
}

signed short obj3d_alloc(struct obj3d far *o, unsigned short nverts, signed short nfaces)
{
	register signed short i;

	if ((o->vl = mem_alloc(nverts * 8 + 2)) == 0) return 0;
	if ((o->faces = mem_alloc(nfaces * 4)) == 0) return 0;
	for (i = 0; i < nfaces; i++) {
		if ((o->faces[i] = mem_alloc(0x48)) == 0) return 0;
	}
	return 1;
}

void obj3d_free(struct obj3d far *o)
{
	register signed short i;

	for (i = 0; i < o->nfaces; i++) mem_free(o->faces[i]);
	mem_free(o->faces);
	if (o->vl) o->vl = mem_free(o->vl);
}

void view_load_vis_order(void)
{
	g_vis_order = mem_alloc(0x1000);
	file_load(g_file_main, 0x405, g_vis_order);
}

void view_init_stars(void)
{
	struct rgb c;
	register signed short i;

	for (i = 0, g_stars_reset = 0; i <= 99; i++) {
		g_stars[i].x = (((signed long)rand() << 8) / 0x8000) - 128;
		g_stars[i].y = (((signed long)rand() << 8) / 0x8000) - 128;
		g_stars[i].z = ((signed long)rand() << 9) / 0x8000;
	}
	for (i = 0; i < 8; i++) {
		c.r = c.g = c.b = i * 5 + 20;
		g_star_colors[i] = pal_find_nearest(&c, 0);
	}
}

void view_load_palette(void)
{
	unsigned short n;
	unsigned short r;
	unsigned short idx;
	signed long off;
	struct rgb c;
	signed short j;
	unsigned char huge *p;
	register signed short i;

	file_open(g_file_main, 0x13d);
	file_read(g_file_main, g_shade_table, 0x200);
	view_init_stars();
	if ((p = g_shade_buf = mem_alloc(0x2010)) == 0) guru_meditation("CAN'T INITIALIZE SHADOWING (OUT OF MEMORY)", 0);
	p += 0x10 - FP_OFF(p);
	for (i = 0; i < 32; i++) g_shade_segs[i] = FP_SEG(p) + (i << 4);
	off = -1;
	if (g_area_type == AREA_TOWN) {
		off = 0;
	} else if (g_group->dungeon == 4) {
		off = 0x2010;
	} else {
		memset(g_shade_buf, 0, 0x2010);
		n = g_view_pal_n;
		for (j = 0; j < 16; j++) {
			for (i = 0; i < n; i++) {
				c = g_view_pal[i];
				c.r += (15 - j) * 2;
				if (c.r > 63) c.r = 63;
				c.g += (15 - j) * 2;
				if (c.g > 63) c.g = 63;
				c.b += (15 - j) * 2;
				if (c.b > 63) c.b = 63;
				r = pal_find_nearest(&c, 0);
				idx = (unsigned char)r;
				r >>= 8;
				((unsigned char far *)p)[(j << 8) + i] = idx;
			}
		}
		for (j = 16; j < 32; j++) {
			for (i = 0; i < n; i++) {
				c = g_view_pal[i];
				c.r -= j * 2 - 32;
				if ((signed short)(signed char)c.r < 0) c.r = 0;
				c.g -= j * 2 - 32;
				if ((signed short)(signed char)c.g < 0) c.g = 0;
				c.b -= j * 2 - 32;
				if ((signed short)(signed char)c.b < 0) c.b = 0;
				r = pal_find_nearest(&c, 0);
				idx = (unsigned char)r;
				r >>= 8;
				((unsigned char far *)p)[(j << 8) + i] = idx;
			}
		}
	}
	if (off != -1) {
		file_seek(g_file_main, off, SEEK_CUR);
		file_read(g_file_main, g_shade_buf, 0x2010);
	}
	file_close(g_file_main);
}

void pal_set_rgb(struct palette far *p, unsigned char idx, unsigned char r, unsigned char g, unsigned char b)
{
	p->c[idx].r = r;
	p->c[idx].g = g;
	p->c[idx].b = b;
}

signed short pal_find_nearest(struct rgb far *c, signed short start)
{
	signed short best;

	asm les di, c
	asm mov al, es:[di]
	asm mov bl, es:[di+1]
	asm mov cl, es:[di+2]
	asm mov di, start
	asm mov si, offset g_view_pal
#if FIX_0016
	/* Fix #16: start dreimal addieren (Eintraege zu 3 Bytes) */
	asm add si, di
	asm add si, di
	asm add si, di
#else
	asm add si, di
	asm add si, di
	asm add si, di
	asm add si, di
#endif
	asm cld
	asm mov dh, 0ffh
	asm mov word ptr best, 0ffffh
next:
	asm mov ah, [si]
	asm mov bh, [si+1]
	asm mov ch, [si+2]
	asm sub ah, al
	asm jns d1
	asm neg ah
d1:
	asm sub bh, bl
	asm jns d2
	asm neg bh
d2:
	asm sub ch, cl
	asm jns d3
	asm neg ch
d3:
	asm mov dl, ah
	asm add dl, bh
	asm add dl, ch
	asm cmp dl, dh
	asm jnc skip
	asm mov dh, dl
	asm mov word ptr best, di
	asm mov byte ptr best+1, dl
	asm or dl, dl
	asm jz done
skip:
	asm inc di
	asm add si, 3
#if FIX_0016
	/* Fix #16: nur die g_view_pal_n belegten Farben durchsuchen (fehlender Sprung) */
	asm cmp di, g_view_pal_n
	asm jnc done
	asm cmp di, 0a0h
	asm jc next
#else
	asm cmp di, g_view_pal_n
	asm cmp di, 0a0h
	asm jc next
#endif
done:
	return best;
}
