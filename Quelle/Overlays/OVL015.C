/* OVL015 - Overlay O015 (Segment 0x1f54 in SCHWEIF.EXE): Heldenbogen-Animationen (nur bei g_pp20_index ==
 * PP20_SHEET (14)): Ein-/Auszoomen einer Anlege-Animation (Animationsobjekt Anim, lokal) und Zauber-Animation.
 * sheet_equip_anim_id(sex, slot, id): Animation zum Anlegen von id an Platz slot (zwei switch-Tabellen je nach
 *   Geschlecht, Sonderfaelle id 0xab/0xd9/0xf5/0x109 am Kopf), nur wenn id nicht in g_zoom_skip_items; sonst -1.
 *   Dateien (spieltexte/dateinummern.tsv): 0x30..0x36 M_HELMET/CHAIN/RING/BANGLE/BOOTS/ARMPAD/LEGPAD.ACE, 0x37..0x3d
 *   dieselben mit F_, 0x1a0 BELT.ACE, 0x1a1/0x1a2 M_/F_FRONTL.ACE; Platz 0 Kopf, 8 Hals, 1 Arme, 2/11 Handgelenke,
 *   4/13 Finger, 6 Beine, 14 Fuesse, 7 Guertel.
 * sheet_slot_changed(sex, slot, id): Animation laden, Palette setzen, Farbe 0x21 (Rahmen) = g_zoom_frame_color; mit
 *   Option g_anims_on 17 Stufen Aufzoomen von g_zoom_slot_xy[slot] auf 0xd4/0x7e (Bmp zoom/frame_bmp, Hintergrund
 *   per XMS zurueckholen, Rahmen do_border, auf den Zeitgeber g_anim_timer warten, memcpy 64000 nach g_vga_buf),
 *   sonst nur Rahmen; Animation abspielen (start/wait_cue), Pause wait_ticks(10), dann Abzoomen; Objekt freigeben.
 * sheet_cast_anim(): Animation 0x367 (ENTZAUB.ACE) in einem neuen Anim-Objekt bei 0x58/0x39 zeigen und Hintergrund
 *   wiederherstellen (O233, Zauber im Heldenbogen).
 * xms_save_screen/xms_restore_screen(p): 64000 Bytes per g_xms_move (Handle g_dialog_xms, xms_move_any) vom
 *   Bildschirmpuffer p ins XMS bzw. zurueck.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL015.C   -> 1844/1844 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - Reines C, beim ersten Versuch gleich (mit und ohne -Og).
 * - switch mit Sprungtabellen; Faelle in Quellreihenfolge 0, 8, 1, 2/11, 4/13, 6, 14, 7, "default: break;"
 *   (eigener jmp short); "if (!sex)" mit signed-char-Parameter -> mov al/cbw/or ax,ax.
 * - Eine register-Variable k (DI) erst fuer das Ergebnis von sheet_equip_anim_id, spaeter k = slot als Index.
 * - Farbe als 3-Byte-Strukturkopie aus g_zoom_frame_color (mov ax / mov al), lokale Objekte Bmp (10 Bytes) und
 *   Anim (0xaf Bytes, im Original 0xb0 belegt - gleiche Rahmengroesse) mit expliziten Konstruktor-/
 *   Destruktoraufrufen; leere while-Schleife auf g_anim_timer.
 * - g_xms_move (struct mem_move, signed long): Zeiger per Teilsicht xms_move_ptr (void far *) zugewiesen;
 *   xms_save_screen/xms_restore_screen stehen unten -> far call.
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 */
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:0ede..0f22, dsana) - initialisierte Daten in Adressreihenfolge */
struct point g_zoom_slot_xy[15] = {
	{ 99, 57 },
	{ 87, 76 },
	{ 90, 106 },
	{ 97, 125 },
	{ 97, 144 },
	{ 92, 163 },
	{ 92, 182 },
	{ 122, 104 },
	{ 143, 55 },
	{ 162, 71 },
	{ 162, 90 },
	{ 155, 109 },
	{ 149, 128 },
	{ 149, 147 },
	{ 151, 179 }
};
signed short g_zoom_skip_items[2] = { 49, -1 };
struct srgb g_zoom_frame_color = { 40, 20, 0 };

/* g_xms_move mit Zeigern statt signed long (Zuweisung als zwei Worte) */
struct xms_move_ptr {
	signed long len;		/* +0x00 */
	signed short src_handle;	/* +0x04 */
	void far *src;			/* +0x06 */
	signed short dst_handle;	/* +0x0a */
	void far *dst;			/* +0x0c */
};
#define XMS_MOVE (*(struct xms_move_ptr *)&g_xms_move)

void xms_save_screen(void far *p);
void xms_restore_screen(void far *p);

/* Animation zum Anlegen von Gegenstand id an Platz slot, -1 = keine */
signed short sheet_equip_anim_id(signed char sex, signed short slot, signed short id)
{
	register signed short r;

	r = -1;
	if (g_pp20_index == PP20_SHEET && is_in_word_array(id, g_zoom_skip_items) == -1) {
		if (!sex) {
			switch (slot) {
			case 0:
				r = 0x30;
				if (id == 0xab || id == 0xd9 || id == 0xf5 || id == 0x109)
					r = 0x1a1;
				break;
			case 8:
				r = 0x31;
				break;
			case 1:
				r = 0x35;
				break;
			case 2:
			case 11:
				r = 0x33;
				break;
			case 4:
			case 13:
				r = 0x32;
				break;
			case 6:
				r = 0x36;
				break;
			case 14:
				r = 0x34;
				break;
			case 7:
				r = 0x1a0;
				break;
			default:
				break;
			}
		} else {
			switch (slot) {
			case 0:
				r = 0x37;
				if (id == 0xab || id == 0xd9 || id == 0xf5 || id == 0x109)
					r = 0x1a2;
				break;
			case 8:
				r = 0x38;
				break;
			case 1:
				r = 0x3c;
				break;
			case 2:
			case 11:
				r = 0x3a;
				break;
			case 4:
			case 13:
				r = 0x39;
				break;
			case 6:
				r = 0x3d;
				break;
			case 14:
				r = 0x3b;
				break;
			case 7:
				r = 0x1a0;
				break;
			default:
				break;
			}
		}
	}
	return r;
}

void sheet_slot_changed(signed char sex, signed short slot, signed short id)
{
	signed short x;
	signed short y;
	signed short w;
	signed short h;
	struct srgb col;
	void far *buf;
	struct Bmp zoom;
	struct Bmp frame_bmp;
	struct Anim ani;
	register signed short i;
	register signed short k;

	k = sheet_equip_anim_id(sex, slot, id);
	col = g_zoom_frame_color;
	buf = ((struct inv_sheet far *)g_inv)->buf;
	if (k != -1) {
		mouse_hide(0);
		xms_save_screen(g_vga_buf);
		Anim_ctor(&ani, k, 0, 0);
		Anim_pal(&ani, 0, 0x21);
		set_color(&col, 0x21);
		k = slot;
		if (g_anims_on) {
			gb_new_alloc(&zoom, 0x67, 0x43);
			gb_clear(&zoom);
			Anim_rect(&ani, &x, &y, &w, &h);
			gb_new_alloc(&frame_bmp, w, h);
			Anim_unpack(&ani, &frame_bmp);
			gb_put(&zoom, &frame_bmp, x, y, 0);
			gb_delete(&frame_bmp, 2);
			g_anim_timer = 1;
			for (i = 0; i <= 16; i++) {
				x = g_zoom_slot_xy[k].x + (0xd4 - g_zoom_slot_xy[k].x) * i / 16 - 0x56;
				y = g_zoom_slot_xy[k].y + (0x7e - g_zoom_slot_xy[k].y) * i / 16 - 0x37;
				w = i * 0x59 / 16 + 14;
				h = i * 0x35 / 16 + 14;
				gb_new_alloc(&frame_bmp, w, h);
				gb_scale(&zoom, &frame_bmp);
				xms_restore_screen(buf);
				gb_blit(&frame_bmp, buf, x + 0x57, y + 0x38);
				do_border(buf, x + 0x56, y + 0x37, w + 2, h + 2, 0x21);
				while (g_anim_timer == 0)
					;
				memcpy(g_vga_buf, buf, 64000U);
				g_anim_timer = 0;
				gb_delete(&frame_bmp, 2);
			}
			gb_delete(&zoom, 2);
		} else
			do_border(g_vga_buf, 0xd4, 0x7e, 0x69, 0x45, 0x21);
		Anim_start(&ani, 0xd5, 0x7f, 0);
		Anim_step(&ani);
		wait_ticks(10);
		if (g_anims_on) {
			gb_new_alloc(&zoom, 0x67, 0x43);
			gb_clear(&zoom);
			Anim_rect(&ani, &x, &y, &w, &h);
			gb_new_alloc(&frame_bmp, w, h);
			Anim_unpack(&ani, &frame_bmp);
			gb_put(&zoom, &frame_bmp, x, y, 0);
			gb_delete(&frame_bmp, 2);
			for (i = 16; i >= 0; i--) {
				w = i * 0x65 / 16 + 2;
				h = i * 0x41 / 16 + 2;
				x = (0x67 - w) / 2 + 0x7e;
				y = (0x43 - h) / 2 + 0x47;
				gb_new_alloc(&frame_bmp, w, h);
				gb_scale(&zoom, &frame_bmp);
				xms_restore_screen(buf);
				gb_blit(&frame_bmp, buf, x + 0x57, y + 0x38);
				do_border(buf, x + 0x56, y + 0x37, w + 2, h + 2, 0x21);
				while (g_anim_timer == 0)
					;
				memcpy(g_vga_buf, buf, 64000U);
				g_anim_timer = 0;
				gb_delete(&frame_bmp, 2);
			}
			while (g_anim_timer == 0)
				;
			gb_delete(&zoom, 2);
		}
		xms_restore_screen(buf);
		memcpy(g_vga_buf, buf, 64000U);
		mouse_show(0);
		Anim_dtor(&ani, 2);
	}
}

/* Zauber-Animation 0x367 im Heldenbogen (O233) */
void sheet_cast_anim(void)
{
	struct Bmp bg;
	void far *ani;

	if (g_pp20_index == PP20_SHEET && g_anims_on) {
		gb_new_alloc(&bg, 0x90, 0x56);
		ani = Anim_ctor(0, 0x367, 0, 0);
		mouse_hide(0);
		gb_save(&bg, g_vga_buf, 0x58, 0x39);
		Anim_setbg(ani, &bg);
		Anim_start(ani, 0x58, 0x39, 0);
		Anim_step(ani);
		gb_blit(&bg, g_vga_buf, 0x58, 0x39);
		mouse_show(0);
		Anim_dtor(ani, 3);
		gb_delete(&bg, 2);
	}
}

/* Bildschirmpuffer p (64000 Bytes) ins XMS (g_dialog_xms) */
void xms_save_screen(void far *p)
{
	XMS_MOVE.len = 64000L;
	XMS_MOVE.dst_handle = g_dialog_xms;
	XMS_MOVE.dst = 0;
	XMS_MOVE.src_handle = 0;
	XMS_MOVE.src = p;
	xms_move_any(&g_xms_move);
}

/* Bildschirmpuffer p aus dem XMS zurueckholen */
void xms_restore_screen(void far *p)
{
	XMS_MOVE.len = 64000L;
	XMS_MOVE.dst_handle = 0;
	XMS_MOVE.dst = p;
	XMS_MOVE.src_handle = g_dialog_xms;
	XMS_MOVE.src = 0;
	xms_move_any(&g_xms_move);
}
