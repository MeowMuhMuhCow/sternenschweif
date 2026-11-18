/* OVL034 - Overlay O034 (Segment 0x1fb6 in SCHWEIF.EXE): Sammelmodul fuer Gruppen, Bilder und Paletten.
 * group_find_mergeable: Index einer anderen nichtleeren Gruppe in g_groups[6] am selben Ort (group_same_place; -1, wenn
 * keine oder g_nr_groups == 1). hero_file_active: Heldendatei schon unter den aktiven Helden g_heroes[0..5]?
 * chr_list_has: Heldendatei in der Liste (struct chr_entry: Name, Dateiname, Archiv, Satz)? chr_list_remove: Eintrag i
 * entfernen. chr_list_collect(b, typ): gespeicherte Helden aus Archiv 0xfb (temporaer) und 0xfa (Spielstand) sammeln
 * (savegame_read_hero, Filter money_share == typ oder -1, ohne aktive Helden und Doppelte), Rueckgabe Anzahl.
 * load_text_block: Textblock n (1 KB) aus Archivdatei 10 nach g_text_output_buf, letzter in g_text_block.
 * clear_screen_buf: Bildpuffer loeschen (64000 Bytes, Maus aus bei g_vga_buf). pal_fade_out/pal_fade_in: Palette
 * aus-/einblenden (32 Schritte, pal_fade/wait_for_vsync/set_palette). show_fullscreen_pic2/show_fullscreen_pic:
 * Vollbild 0x2e2/0x4d9 (Pic) bis Taste. group_imprison(v): alle Helden der Gruppe jail = v, je Gegenstand
 * (Plaetze 15..30, nicht undropable) 20 % Verlust (drop_item), dann Runden zu je timewarp_until(43200), bis eine
 * Gruppe einen Helden mit jail == 0 hat (Bild 0x1d3), zu dieser Gruppe wechseln. group_scatter(a, b): Gruppe wird
 * versprengt (make_group, Ortswechsel automap_init/automap_random_cell mit g_town_view_areas/g_dng_view_areas-Eintrag),
 * Rueckgabe 0/1/2. code_to_colsel/code_to_text: Code 0xfd/0xf9/0xfc/0xe7/0xff -> Farbsteuerzeichen bzw. Text.
 * game_menu: Spielmenue (dlg_choose) bei g_action == ACTION_MENU oder Taste F9 -> ACTION_OPTIONS..ACTION_MENU_SCREEN,
 * ACTION_MENU_SCREEN -> select_screen, ACTION_DIARY -> diary_run. txt_build_index: Puffer an Nullbytes in eine
 * Zeigerliste zerlegen. read_record: Datensatz n (0x240 Bytes) aus Archivdatei 6. save_screen/restore_screen:
 * Bildpuffer in Temp-Datei (Name g_path_fmts[0] + file_getname(0x3b2)) bzw. XMS (g_xms_move, xms_move_any) sichern
 * und zurueckholen. select_screen/select_screen_update: Auswahlbild mit 18 Feldern (Mausbereiche g_sel_areas),
 * Cursortasten, Rahmen per Bmp::put/get. clamp_min_l/clamp_max_l: max/min (long). dice_code_roll: Wuerfelcode
 * 0xNTxx (N Wuerfel, T 1=W6 2=W20 3=W3 4=W4 sonst W100, + Byte). load_portrait_buf: Bild nach g_portrait_buf.
 * str_cut_at: String an Zeichen c abschneiden. pic_to_bmp: Bild aus Pic in Bmp (+ Palette). bmp_free_data: Bmp-
 * Daten freigeben. pal_blend_from_color/pal_blend_to_color/pal_blend_from_dark/pal_blend/pal_blend2/pal_mix:
 * Palettenueberblendungen (3-Byte-Eintraege, Prozent 10000 / Schritte * i / 100). load_palette_arc3/
 * load_palette_arc3b: Palette (letzte 0x300 Bytes von Datei 3) laden. dissolve_in: Pixel-Ueberblendung (x = x * 17
 * + 0x57). show_pic_337/show_end_sequence: Bildsequenzen 0x337 bzw. 0x4b3 (mit Texten 0xa9). get_group_index: Index
 * der Gruppe g in g_groups. group_leave_place(v): Ort verlassen (route_pos = 0, set_town/set_loctype/set_dungeon
 * doppelt geschachtelt, group_set_on_route, Weg 0x67/0x67, bei v Ereignis ev_show_pic_197 mit Ev-Objekt).
 * prepare_spell_icons: Keylist 0xac, fuer alle Zauberkundigen je Zauber (spells[k] >= -5) keylist_add(basis + k - 1).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y -E..\bc\BIN\TASM.EXE OVL034.CPP -> 10123/10123 Bytes GLEICH (19.10.2026),
 * cobjfix laeuft. Ohne -Og viele Abweichungen; keine -Oi (strcmp/strcpy/memset/memcpy sind echte Aufrufe).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - chr_list_remove: nur "memmove(&b[i], &b[i + 1], ..)" ergibt den -Og-Zwischenwert i * 0x24 ([bp-2], &b[i + 1] als
 *   Zwischenwert + [bp+6] + 0x24); mit "&b[i] + 1" bzw. "b + i + 1" wird stattdessen der Offset von &b[i]
 *   zwischengespeichert.
 * - txt_build_index: "*out++ = s" -> mov ax/mov dx + zwei Wort-Stores; "*out = s; out++;" -> mov eax.
 * - pal_mix: pct/n als register-Parameter (SI/DI), Zaehler i als register-Variable in eigenem inneren Block ohne
 *   Aufrufe -> CX; auf Funktionsebene bekommt i SI und n faellt auf den Stack.
 * - group_leave_place: Ev-Zeiger im if-Block deklarieren (Block-Local hinter den Parameterkopien der geschachtelten
 *   inline-Setter und dem -Og-Byte, [bp-0ah]); als Funktions-Variable laege er vorn bei [bp-4].
 * - group_scatter: Block-Locals mit Initialisierung in Deklarationsreihenfolge (struct hp7 arr = g_hero_ptrs_null / g_hero_ptrs_null2 als
 *   rep movsw nach den Aufrufen); wegen rep movsw keine Registervariablen (i, j auf dem Stack).
 * - group_imprison: inline-Setter set_jail (Kopie [bp-0fh]), "!!g->nr_heroes" (cbw / or ax,ax); lokales Pic-Objekt.
 * - dice_code_roll: __rotl__ (stdlib.h) ohne -Oi; i dient zuerst als Wuerfeltyp, dann als Schleifenzaehler (SI).
 * - select_screen: g_key unsigned (ja im switch-Baum); "g_mouse_handler = set_mouse_handler2" -> cobjfix biegt das
 *   Segment-Symbol F_R02_0005 auf F_R02_0247. get_group_index ohne Aufrufe: r in CX, i in BX (von selbst).
 * - Paletten als struct srgb (signed char): pal_mix rechnet mit Vorzeichen (cbw).
 * - font_set_colors(g_font, x, y) ueber gecasteten Funktionszeiger (Prototyp: unsigned char).
 * Originalfehler/Auffaelliges: load_text_block prueft aussen "n > -1", damit ist der innere Zweig "n == -1 -> letzter
 *   Block g_text_block" toter Code. group_scatter (k 3..5): Schleife "for (i = k; i > 0; i--)" beginnt bei arr.h[k], das
 *   nie gefuellt wurde (nur 0..k-1) - Wert aus der Vorlage g_hero_ptrs_null2; die folgende Schleife laeuft ueber
 *   arr.h[0..cnt-1] statt ueber die entfernten Helden. code_to_colsel/code_to_text: Rueckgabe bei unbekanntem Code
 *   uninitialisiert. prepare_spell_icons: r uninitialisiert, "if (r == 0) ;" (verirrtes Semikolon). group_find_mergeable: "if (g_nr_groups == 1)
 *   r = -1;" ueberfluessig; hero_file_active setzt i zweimal auf 0.
 */
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include <io.h>
#include <stdlib.h>
#include <alloc.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:3c8c..3d8e, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_text_block = -1;
struct hero_ptrs7 g_hero_ptrs_null = { { 0, 0, 0, 0, 0, 0, 0 } };
struct hero_ptrs7 g_hero_ptrs_null2 = { { 0, 0, 0, 0, 0, 0, 0 } };
struct mouse_area g_sel_areas[20] = {
	{ 244, 2, 278, 36, 1 },
	{ 209, 37, 243, 71, 2 },
	{ 244, 44, 278, 78, 3 },
	{ 279, 32, 313, 66, 4 },
	{ 249, 80, 283, 114, 5 },
	{ 285, 75, 319, 109, 6 },
	{ 166, 92, 200, 126, 7 },
	{ 202, 102, 236, 136, 8 },
	{ 82, 118, 116, 152, 9 },
	{ 92, 153, 126, 187, 10 },
	{ 57, 166, 91, 200, 11 },
	{ 128, 166, 162, 200, 12 },
	{ 175, 139, 209, 173, 13 },
	{ 208, 165, 242, 199, 14 },
	{ 240, 124, 274, 158, 15 },
	{ 243, 159, 277, 193, 16 },
	{ 279, 142, 313, 176, 17 },
	{ 127, 131, 161, 165, 18 },
	{ 0, 0, 319, 199, 0 },
	{ -1, -1, -1, -1, -1 }
};
/* Phase B: eigenes BSS (ds:c776..c794) in Adressreihenfolge */
signed short g_select_shown;
signed short g_select_next;
BMP_PTR g_sel_frame_bmp;
BMP_PTR g_sel_text_bmp;
void far *g_sel_txt_buf;
char far * far *g_sel_txt;
void far *g_sel_saved_areas;
void (far *g_sel_saved_handler)(short);
signed short g_sel_saved_on;

#define NVF_HEADS ((Nvf far *)g_nvf_heads)	/* NVF-Dateiobjekt (O098) */
#define PIC ((Pic far *)pic)			/* pic_to_bmp: Bildobjekt */

signed short group_find_mergeable(void)
{
	signed short i;
	signed short r;
	struct group far *g;

	r = -1;
	if (g_nr_groups == 1)
		r = -1;
	else {
		for (i = 0, g = g_groups; i < 6; i++, g++) {
			if (g != g_group && !!g->nr_heroes && group_same_place(g_group, g))
				r = i;
		}
	}
	return r;
}

signed short hero_file_active(char far *s)
{
	signed short i = 0;
	signed short found = 0;
	struct hero far *h;

	for (i = 0, h = g_heroes; i < 6; i++, h++) {
		if (h->type != 0 && strcmp(h->file_name, s) == 0) {
			found = 1;
			break;
		}
	}
	return found;
}

signed short chr_list_has(char far *s, signed short n, struct chr_entry far *tab)
{
	signed short i;
	signed short found;

	found = 0;
	for (i = 0; i < n; i++) {
		if (strcmp(s, tab[i].file_name) == 0) {
			found = 1;
			break;
		}
	}
	return found;
}

void chr_list_remove(struct chr_entry far *b, signed short i, signed short n)
{
	if (n - 1 == i) {
		memset(&b[i], 0, sizeof(struct chr_entry));
		return;
	}
	memmove(&b[i], &b[i + 1], (n - i) * sizeof(struct chr_entry));
}

signed short chr_list_collect(struct chr_entry far *b, signed short t)
{
	signed short id;
	signed short n1;
	signed long size;
	struct hero far *h;
	signed short done;
	struct chr_entry far *tmp;
	signed short n2;
	signed short n2b;
	signed short cnt;
	signed short k;

	cnt = 0;
	done = 0;
	size = ofile_length_id(g_ofile, 0xfa);
	*(char far * far *)g_names_star_idx = file_getname(g_file_main, 0xfb);
	size += ofile_length_id(g_ofile, 0);
	n2 = size / 0x5ef;
	h = (struct hero far *)mem_alloc(sizeof(struct hero));
	tmp = (struct chr_entry far *)mem_alloc(n2 * sizeof(struct chr_entry));
	memset(tmp, 0, n2 * sizeof(struct chr_entry));
	n2 = 0;
	id = 0xfb;
	for (k = 0; !done; k++) {
		size = savegame_read_hero(k, id, h);
		if (size == 0x5ef) {
			if (!hero_file_active(h->file_name) && (h->money_share == t || t == -1)) {
				strcpy(b[cnt].name, h->name);
				strcpy(b[cnt].file_name, h->file_name);
				b[cnt].arc_id = id;
				b[cnt].slot = k;
				cnt++;
			} else {
				strcpy(tmp[n2].name, h->name);
				strcpy(tmp[n2].file_name, h->file_name);
				tmp[n2].arc_id = id;
				tmp[n2].slot = k;
				n2++;
			}
		} else
			done = 1;
	}
	n1 = cnt;
	n2b = n2;
	done = 0;
	id = 0xfa;
	for (k = 0; !done; k++) {
		size = savegame_read_hero(k, id, h);
		if (size == 0x5ef) {
			if (!chr_list_has(h->file_name, n1, b) && !chr_list_has(h->file_name, n2b, tmp) && !hero_file_active(h->file_name) &&
				(h->money_share == t || t == -1)) {
				strcpy(b[cnt].name, h->name);
				strcpy(b[cnt].file_name, h->file_name);
				b[cnt].arc_id = id;
				b[cnt].slot = k;
				cnt++;
			}
		} else
			done = 1;
	}
	mem_free(h);
	mem_free(tmp);
	return cnt;
}

void load_text_block(signed short n)
{
	if (n > -1) {
		if (n >= 0)
			g_text_block = n;
		else if (n == -1 && g_text_block != -1)
			n = g_text_block;
		file_open(g_file_main, 10);
		file_seek(g_file_main, (signed long)n << 10, 0);
		file_read(g_file_main, g_text_output_buf, 0x400L);
		file_close(g_file_main);
	}
}

void clear_screen_buf(void far *p)
{
	if (p == g_vga_buf)
		mouse_hide(0);
	memset(p, 0, 64000U);
	if (p == g_vga_buf)
		mouse_show(0);
}

void pal_fade_out(void)
{
	signed short i;

	for (i = 0; i < 32; i++) {
		pal_fade(g_palette, g_pal_dark, 0x100);
		pal_fade(g_palette, g_pal_dark, 0x100);
		wait_for_vsync();
		set_palette(g_palette, 0, 0x100);
	}
}

void pal_fade_in(void far *pal)
{
	signed short i;

	for (i = 0; i < 32; i++) {
		pal_fade(g_pal_dark, pal, 0x100);
		pal_fade(g_pal_dark, pal, 0x100);
		wait_for_vsync();
		set_palette(g_pal_dark, 0, 0x100);
	}
	memset(g_pal_dark, 0, 0x300);
}

void show_fullscreen_pic2(void)
{
	char far *pal;
	Pic far *o;

	o = new Pic;
	pal = new char[0x300];
	ani_halt(g_screen_pic);
	pal_fade_out();
	mouse_hide(0);
	o->load(g_pp20_index = 0x2e2, g_vga_buf, (struct rgb far *)pal);
	pal_fade_in(pal);
	do
		poll_input();
	while (g_action == 0);
	g_pic2_seen = 1;
	pal_fade_out();
	clear_screen_buf(g_vga_buf);
	mouse_show(0);
	g_refresh = 1;
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	delete pal;
	delete o;
}

void show_fullscreen_pic(void)
{
	char far *pal;
	Pic far *o;

	o = new Pic;
	pal = new char[0x300];
	ani_halt(g_screen_pic);
	pal_fade_out();
	mouse_hide(0);
	o->load(g_pp20_index = 0x4d9, g_vga_buf, (struct rgb far *)pal);
	pal_fade_in(pal);
	do
		poll_input();
	while (g_action == 0);
	pal_fade_out();
	clear_screen_buf(g_vga_buf);
	mouse_show(0);
	g_refresh = 1;
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	delete pal;
	delete o;
}

void group_imprison(signed short v)
{
	Pic o;
	struct group far *g;
	struct hero far *h;
	Bmp far *b;
	signed short id;
	signed short f;
	signed short j;

	f = 0;
	b = new Bmp(0xd0, 0x87);
	group_set_screen_pos(0xd, 0x1c);
	h = get_first_hero(g_group);
	do {
		h->set_jail(v);
		for (j = 15; j < 31; j++) {
			if ((id = h->inv[j].item_id) > 0 && !g_items[id].undropable && random_schweif(100) <= 20) {
				drop_item(h, j, 1);
				f = 1;
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (f)
		dlg_message(g_dialog, g_ttx[412]);
	f = 0;
	do {
		for (j = 0, g = g_groups; j < 6; j++, g++) {
			if (!!g->nr_heroes) {
				h = get_first_hero(g);
				do {
					if (!h->jail)
						f = 1;
					else
						h = get_next_hero(g, h);
				} while (get_first_hero(g) != h && !f);
			}
			if (f)
				break;
		}
		o.load(0x1d3, b->data());
		set_palette(o.get_palette(), 0, 0x40);
		mouse_hide(0);
		b->put(g_vga_buf, 0x10, 2, 5);
		mouse_show(0);
		if (!f)
			timewarp_until(g_date, 0xa8c0L);
		else {
			g_group = g;
			g_group_no = get_group_index(g);
		}
	} while (!f);
	if (g_jail_freed != 0)
		g_jail_freed = 0;
	else
		wait_ticks_key(0xfa);
	b->clear();
	mouse_hide(0);
	b->put(g_vga_buf, 0x10, 2, 5);
	mouse_show(0);
	delete b;
	screen_redraw(g_screen);
	cam_set_from_group();
}

signed short group_scatter(signed short a, signed short b)
{
	signed short k;
	signed short ret;
	struct view_area far *p;
	signed short i;
	signed short j;

	k = random_schweif(10);
	ret = 0;
	mouse_set_cursor(5, 1);
	diary_unlock(g_diary);
	if (g_group->town != 0)
		p = &g_town_view_areas[g_group->town - 1];
	else if (g_group->dungeon != 0)
		p = &g_dng_view_areas[g_group->dungeon - 1];
	else
		return 0;
	if (a == 0 && k > 2 && (signed short)g_group->nr_heroes == 1)
		k = 10;
	if (a == 1)
		k = 2;
	else if (a == 2)
		k = 5;
	if (a == 3) {
		automap_init(g_map, p->map);
		automap_random_cell(g_map);
		automap_free_cells(g_map);
		ret = 2;
	} else if (k <= 2) {
		signed short old = g_pp20_index;
		signed short n = group_count_check3(g_group);
		signed short cnt = g_group->nr_heroes;
		struct hero far *hx = group_get_npc(g_group);
		struct hero far *h0 = get_first_active_hero(g_group);
		struct group far *g;
		struct hero_ptrs7 arr = g_hero_ptrs_null;

		g_pp20_index = -1;
		if (hx != 0) {
			cnt--;
			n--;
		}
		arr.h[0] = h0;
		for (i = 1; i < cnt; i++)
			arr.h[i] = get_next_hero(g_group, arr.h[i - 1]);
		for (i = cnt - 1; i > 0; i--)
			if (check_hero3(arr.h[i]))
				make_group(1, arr.h[i]);
		for (i = 0; i < n; i++)
			for (j = 0, g = g_groups; j < 6; j++, g = &g_groups[j])
				if (group_has_hero(g, arr.h[i])) {
					g_group = g;
					automap_init(g_map, p->map);
					automap_random_cell(g_map);
					automap_free_cells(g_map);
				}
		for (i = 0, g = g_groups; i < 6; i++, g = &g_groups[i])
			if (group_has_hero(g, h0)) {
				g_group = g;
				g_group_no = get_group_index(g);
			}
		g_pp20_index = old;
		if (b != 0)
			dlg_message(g_dialog, g_ttx[413]);
		ret = 2;
		screen_redraw(g_screen);
		cam_set_from_group();
	} else if (k <= 5) {
		signed short cnt = 1;
		signed short old = g_pp20_index;
		struct hero far *hx = group_get_npc(g_group);
		struct hero_ptrs7 arr = g_hero_ptrs_null2;
		struct group far *save = g_group;
		struct group far *g;

		g_pp20_index = -1;
		k = random_schweif(g_group->nr_heroes - 1);
		if (hx != 0)
			k--;
		if (k == 0)
			k = 1;
		arr.h[0] = get_first_hero(g_group);
#if FIX_0207
		/* Fix #207: arr.h[1..k] fuellen, bevor sie abgetrennt werden */
		for (i = 1; i <= k; i++)
			arr.h[i] = get_next_hero(g_group, arr.h[i - 1]);
		for (i = k; i > 0; i--)
#else
		for (i = 1; i < k; i++)
			arr.h[i] = get_next_hero(g_group, arr.h[i - 1]);
		for (i = k; i > 0; i--)
#endif
			if (check_hero3(arr.h[i])) {
				make_group(1, arr.h[i]);
				cnt++;
			}
#if FIX_0207
		/* Fix #207: ueber die abgetrennten Helden arr.h[1..k] laufen */
		for (i = 1; i <= k; i++)
			for (j = 0, g = g_groups; j < 6; j++, g = &g_groups[j])
				if (group_has_hero(g, arr.h[i]) && g != g_group) {
#else
		for (i = 0; i < cnt; i++)
			for (j = 0, g = g_groups; j < 6; j++, g = &g_groups[j])
				if (group_has_hero(g, arr.h[i]) && g != g_group) {
#endif
					g_group = g;
					automap_init(g_map, p->map);
					automap_random_cell(g_map);
					automap_free_cells(g_map);
				}
		g_group = save;
		g_group_no = get_group_index(save);
		g_pp20_index = old;
		if (b != 0)
			dlg_message(g_dialog, g_ttx[414]);
		ret = 1;
		screen_redraw(g_screen);
		cam_set_from_group();
		group_imprison(2);
	} else {
		if (b != 0)
			dlg_message(g_dialog, g_ttx[415]);
		ret = 0;
	}
	diary_lock(g_diary);
	mouse_reset();
	if (ret != 0)
		screen_redraw(g_screen);
	return ret;
}

signed char code_to_colsel(unsigned char c)
{
	signed char r;

	switch (c) {
	case 0xfd:
		r = 0xf1;
		break;
	case 0xf9:
		r = 0xf2;
		break;
	case 0xfc:
		r = 0xf3;
		break;
	case 0xe7:
		r = 0xf4;
		break;
#if FIX_0207
	/* Fix #207: unbekannter Code: Grundfarbe statt Muell */
	case 0xff:
		r = 0xf0;
		break;
	default:
		r = 0xf0;
		break;
	}
#else
	case 0xff:
		r = 0xf0;
		break;
	}
#endif
	return r;
}

char far *code_to_text(unsigned char c)
{
	char far *r;

	switch (c) {
	case 0xfd:
		r = g_ttx[407];
		break;
	case 0xf9:
		r = g_ttx[408];
		break;
	case 0xfc:
		r = g_ttx[409];
		break;
	case 0xe7:
		r = g_ttx[410];
		break;
#if FIX_0207
	/* Fix #207: unbekannter Code: Text wie 0xff statt Muellzeiger */
	case 0xff:
		r = g_ttx[406];
		break;
	default:
		r = g_ttx[406];
		break;
	}
#else
	case 0xff:
		r = g_ttx[406];
		break;
	}
#endif
	return r;
}

void game_menu(void)
{
	if (g_action == 0xfd || g_key == 0x4300) {
		if (g_diary_new_items != 0) {
			g_action = dlg_choose(g_dialog, 0, 3, g_ttx[404], g_ttx[402], g_ttx[401]);
			switch (g_action) {
			case 1:
				g_action = 0xe0;
				break;
			case 2:
				g_action = 0xe1;
				break;
			case 3:
				g_action = 0xe2;
				break;
			default:
				g_action = 0;
				break;
			}
		} else {
			g_action = dlg_choose(g_dialog, 0, 2, g_ttx[404], g_ttx[401]);
			switch (g_action) {
			case 1:
				g_action = 0xe0;
				break;
			case 2:
				g_action = 0xe2;
				break;
			default:
				g_action = 0;
				break;
			}
		}
	}
	if (g_action == 0xe2) {
		ani_stop(g_screen_pic);
		swap_out_screen();
		select_screen();
		return;
	}
	if (g_action == 0xe1) {
		if (g_diary_new_items != 0) {
			ani_stop(g_screen_pic);
			swap_out_screen();
			diary_run(g_diary);
		}
		return;
	}
	handle_options_key();
}

void txt_build_index(char far * far *out, char far *s, signed long n)
{
	signed long i;

	i = 0;
	*out++ = s;
	for (; i != n; s++, i++) {
		if (!*s) {
			*out = s + 1;
			out++;
		}
	}
}

void read_record(void far *p, signed short n)
{
	arc_open(g_file_3, 6);
	arc_seek(g_file_3, (signed long)n * 0x240 + 7, 0);
	arc_read(g_file_3, p, 0x240L);
	arc_close(g_file_3);
}

void save_screen(void)
{
	char far *name;

	if (g_silent_mode2 != 0 || g_silent_mode != 0) {
		name = new char[0x28];
		sprintf(name, g_path_fmts[0], file_getname(g_file_main, 0x3b2));
		unlink(name);
		delete name;
		*(char far * far *)g_names_star_idx = file_getname(g_file_main, 0x3b2);
		ofile_open_mode(g_ofile, 0, 0x8102, 0x180);
		ofile_write(g_ofile, g_vga_buf, 0xfa00L);
		ofile_close(g_ofile);
	} else {
		g_xms_move.len = 0xfa00L;
		g_xms_move.src_handle = 0;
		g_xms_move.src = (signed long)g_vga_buf;
		g_xms_move.dst_handle = g_dialog_xms;
		g_xms_move.dst = 0;
		xms_move_any(&g_xms_move);
	}
}

void restore_screen(void)
{
	char far *buf;
	char far *dst;
	signed short i;

	if (g_silent_mode2 != 0 || g_silent_mode != 0) {
		buf = new char[0x140];
		dst = (char far *)g_vga_buf;
		*(char far * far *)g_names_star_idx = file_getname(g_file_main, 0x3b2);
		ofile_create(g_ofile, 0, 0x8001);
		for (i = 0; i < 200; i++) {
			ofile_read(g_ofile, buf, 0x140L);
			memcpy(dst, buf, 0x140);
			dst += 0x140;
		}
		ofile_close(g_ofile);
		sprintf(buf, g_path_fmts[0], file_getname(g_file_main, 0x3b2));
		unlink(buf);
		delete buf;
	} else {
		g_xms_move.len = 0xfa00L;
		g_xms_move.src_handle = g_dialog_xms;
		g_xms_move.src = 0;
		g_xms_move.dst_handle = 0;
		g_xms_move.dst = (signed long)g_vga_buf;
		xms_move_any(&g_xms_move);
	}
}

void select_screen(void)
{
	Pic far *o;
	char far *pal;
	char far *pal2;

	o = new Pic;
	pal = new char[0x300];
	pal2 = new char[0x300];
	get_palette(pal2, 0, 0x100);
	pal_fade_out();
	snd_play(g_snd_music, 0x2c);
	g_sel_frame_bmp = new Bmp(0x22, 0x22);
	g_sel_text_bmp = new Bmp(0xb4, 0x50);
	txt_load_list(g_font, 0x36a, 0x12, &g_sel_txt_buf, &g_sel_txt);
	g_sel_saved_areas = g_mouse_handler_areas;
	g_sel_saved_handler = g_mouse_handler;
	g_sel_saved_on = g_mouse_handler_on;
	mouse_set_cursor(0, 1);
	mouse_hide(0);
	o->load(g_pp20_index = 0x369, g_vga_buf, (struct rgb far *)pal);
	mouse_show(0);
	pal_fade_in(pal);
	g_mouse_handler_areas = g_sel_areas;
	g_mouse_handler = set_mouse_handler2;
	g_mouse_handler_on = 1;
	g_select_shown = -1;
	g_select_next = 1;
	do {
		select_screen_update();
		poll_input();
		if (g_key != 0) {
			switch (g_key) {
			case 0x4800:
			case 0x4b00:
				if (g_select_shown > 1)
					g_select_next = g_select_shown - 1;
				break;
			case 0x4d00:
			case 0x5000:
				if (g_select_shown < 0x12)
					g_select_next = g_select_shown + 1;
				break;
			case 0x4900:
				g_select_next = 1;
				break;
			case 0x5100:
				g_select_next = 0x12;
				break;
			}
		}
	} while (g_mouse_rclick == 0 && g_mouse_lclick == 0 && g_key != 0x11b);
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	g_mouse_handler_on = 0;
	mouse_hide(0);
	pal_fade_out();
	clear_screen_buf(g_vga_buf);
	mouse_show(0);
	g_mouse_handler_areas = g_sel_saved_areas;
	g_mouse_handler = g_sel_saved_handler;
	g_mouse_handler_on = g_sel_saved_on;
	set_palette(pal2, 0, 0x100);
	g_refresh = 1;
	mem_free(g_sel_txt_buf);
	mem_free(g_sel_txt);
	delete g_sel_text_bmp;
	delete g_sel_frame_bmp;
	snd_obj_stop(g_snd_music);
	Sound_stop_music(g_snd_music);
	delete pal2;
	delete pal;
	delete o;
}

void select_screen_update(void)
{
	if (g_select_next != g_select_shown) {
		g_mouse_handler_on = 0;
		if (g_select_shown > 0) {
			signed short k = g_select_shown - 1;

			mouse_hide(0);
			g_sel_frame_bmp->put(g_vga_buf, g_sel_areas[k].x0, g_sel_areas[k].y0);
			g_sel_text_bmp->put(g_vga_buf, 0xe, 3);
			mouse_show(0);
			g_select_shown = 0;
		}
		if (g_select_next != -1) {
			signed short x, y, a, b, c, d;
			signed short n = g_select_next - 1;

			font_get_box(g_font, &a, &b, &c, &d);
			font_get_colors(g_font, &x, &y);
			font_set_box(g_font, 0xe, 0xb4, 3, 1);
			font_set_colors(g_font, 0xfe, -1);
			mouse_hide(0);
			g_sel_frame_bmp->get(g_vga_buf, g_sel_areas[n].x0, g_sel_areas[n].y0);
			g_sel_text_bmp->get(g_vga_buf, 0xe, 3);
			do_border(g_vga_buf, g_sel_areas[n].x0, g_sel_areas[n].y0, 0x22, 0x22, 0xfd);
			font_print_wrapped(g_font, g_sel_txt[n]);
			mouse_show(0);
			((void (*)(void far *, signed short, signed short))font_set_colors)(g_font, x, y);
			font_set_box(g_font, a, b, c, d);
			g_select_shown = g_select_next;
		}
		g_mouse_handler_on = 1;
	}
}

signed long clamp_min_l(signed long v, signed long lo)
{
	if (v < lo)
		v = lo;
	return v;
}

signed long clamp_max_l(signed long v, signed long hi)
{
	if (v > hi)
		v = hi;
	return v;
}

signed short dice_code_roll(signed short d)
{
	signed short n;
	signed short sides;
	signed char add;
	signed short i;
	signed short sum;

	sum = 0;
	n = __rotl__(d & 0xf000, 4);
	i = __rotl__(d & 0x0f00, 8);
	sides = i == 1 ? 6 : i == 2 ? 20 : i == 3 ? 3 : i == 4 ? 4 : 100;
	add = d;
	for (i = 0; i < n; i++)
		sum += random_schweif(sides);
	sum += add;
	return sum;
}

void far *load_portrait_buf(signed short pic)
{
	signed short dummy;

	NVF_HEADS->load(10, g_portrait_buf, 0, pic, 0, &dummy, &dummy, 0x140);
	return g_portrait_buf;
}

char far *str_cut_at(char far *s, signed char c)
{
	while (*s != 0 && *s != c)
		s++;
	*s = 0;
	return s;
}

void pic_to_bmp(void far *pic, signed short id, Bmp far *b, struct srgb far *pal, signed short n)
{
	signed short w;
	signed short h;

	if (PIC->load(id)) {
		PIC->get_size(&w, &h);
		b->init(w, h, PIC->get_data());
		if (n != 0 && pal != 0)
			memcpy(pal, PIC->get_palette(), n * 3);
	}
}

void bmp_free_data(Bmp far *b)
{
	farfree(b->data());
	b->free_lines();
}

void pal_blend_from_color(struct srgb c, struct srgb far *p, signed short n)
{
	struct srgb far *pal;
	signed short i;

	pal = new struct srgb[0x100];
	for (i = 0; i < 0x100; i++)
		pal[i] = c;
	pal_blend2(pal, p, n, 0x100, 1);
	delete pal;
}

void pal_blend_to_color(struct srgb far *p, struct srgb c, signed short n)
{
	struct srgb far *pal;
	signed short i;

	pal = new struct srgb[0x100];
	if (p == 0) {
		get_palette(g_pal_buf, 0, 0x100);
		p = (struct srgb far *)g_pal_buf;
	}
	for (i = 0; i < 0x100; i++)
		pal[i] = c;
	pal_blend2(p, pal, n, 0x100, 1);
	delete pal;
}

void pal_blend_from_dark(struct srgb far *p, signed short n)
{
	pal_blend2((struct srgb far *)g_pal_dark, p, n, 0x100, 1);
}

void pal_blend(struct srgb far *p, signed short n)
{
	if (p == 0) {
		get_palette(g_pal_buf, 0, 0x100);
		p = (struct srgb far *)g_pal_buf;
	}
	pal_blend2(p, (struct srgb far *)g_pal_dark, n, 0x100, 1);
	clear_screen_buf(g_vga_buf);
}

void pal_blend2(struct srgb far *a, struct srgb far *b, signed short c, signed short n, signed short d)
{
	signed short pct;
	struct srgb far *tmp;
	signed short i;

	tmp = new struct srgb[n];
	if (a == 0) {
		get_palette(g_pal_buf, 0, 0x100);
		a = (struct srgb far *)g_pal_buf;
	}
	memcpy(tmp, a, n * 3);
	for (i = 0; i <= c; i++) {
		pct = 10000L / c * i / 100;
		pal_mix(a, b, tmp, pct, n);
		wait_ticks_key(d);
		set_palette(tmp, 0, n);
	}
	delete tmp;
}

void pal_mix(struct srgb far *a, struct srgb far *b, struct srgb far *dst, register signed short pct,
	register signed short n)
{
	if (pct <= 0) {
		memcpy(dst, a, n * 3);
		return;
	}
	if (pct >= 100) {
		memcpy(dst, b, n * 3);
		return;
	}
	{
	register signed short i;
	for (i = 0; i < n; i++) {
		dst[i].r = a[i].r + (b[i].r - a[i].r) * pct / 100;
		dst[i].g = a[i].g + (b[i].g - a[i].g) * pct / 100;
		dst[i].b = a[i].b + (b[i].b - a[i].b) * pct / 100;
	}
	}
}

void load_palette_arc3(void)
{
	file_open(g_file_main, 3);
	file_seek(g_file_main, -0x300L, 2);
	file_read(g_file_main, g_pal_buf, 0x300L);
	file_close(g_file_main);
	set_palette(g_pal_buf, 0, 0x100);
}

void load_palette_arc3b(void far *p)
{
	file_open(g_file_main, 3);
	file_seek(g_file_main, -0x300L, 2);
	file_read(g_file_main, p, 0x300L);
	file_close(g_file_main);
}

void dissolve_in(unsigned char far *buf)
{
	unsigned short k;
	unsigned short v;
	unsigned short x;

	x = 0;
	poll_input();
	for (k = 0; k < 64000U; k++) {
		do {
			v = (x * 0x11 + 0x57) & 0xffff;
			x = v;
		} while (v >= 64000U);
		((unsigned char far *)g_vga_buf)[v] = buf[v];
	}
}

void show_pic_337(void)
{
	Pic o;
	void far *pal;

	swap_out_screen();
	pal = mem_alloc(0x300L);
	memcpy(pal, g_palette, 0x300);
	Sound_volume3(g_snd_music, 0, 500, 0);
	pal_fade_out();
	snd_play(g_snd_music, 0x22);
	mouse_hide(0);
	o.load(g_pp20_index = 0x337, g_vga_buf, (struct rgb far *)g_pal_buf);
	pal_fade_in(g_pal_buf);
	wait_ticks_key(0x92e);
	Sound_volume3(g_snd_music, 0, g_action == 0 ? 5000 : 1000, 0);
	wait_ticks_key(g_action == 0 ? 0xaa : 0x14);
	pal_fade_out();
	clear_screen_buf(g_vga_buf);
	mouse_show(0);
	g_refresh = 1;
	set_palette(pal, 0, 0x100);
	snd_obj_stop(g_snd_music);
	mem_free(pal);
}

void show_end_sequence(void)
{
	void far *pal2;
	void far *pal;
	Pic far *o;
	void far *buf;
	Bmp far *b;
	void far *txt_buf;
	char far * far *txt;
	signed short x1, y1, x2, y2;
	signed short old;

	swap_out_screen();
	txt_load_list(g_font, 0xa9, 0x32, &txt_buf, &txt);
	mouse_hide(0);
	ani_stop(g_screen_pic);
	old = g_clock_update;
	g_clock_update = 0;
	g_dialog_depth++;
	pal = mem_alloc(0x300L);
	pal2 = mem_alloc(0x300L);
	memcpy(pal2, g_palette, 0x300);
	buf = mem_alloc(0xfa00L);
	o = new Pic;
	o->load(g_pp20_index = 0x4b3, buf);
	memcpy(pal, o->get_palette(), 0x300);
	delete o;
	Sound_volume3(g_snd_music, 0, 500, 0);
	pal_fade_out();
	snd_play(g_snd_music, 0x22);
	b = new Bmp(0x140, 0xc8, buf);
	b->put(g_vga_buf, 0, 0);
	pal_fade_in(pal);
	wait_ticks_key(0x2d);
	font_set_colors(g_font, 0x72, -1);
	font_get_box(g_font, &x1, &y1, &x2, &y2);
	font_set_box(g_font, 0xc8, 0x6e, 0x10, 0);
	if (g_game_state == 4)
		font_print_wrapped(g_font, group_find_item(g_group, 0xfc) >= 0 ? txt[45] : txt[48]);
	else
		font_print_wrapped(g_font, txt[47]);
	wait_ticks_key(400);
	b->put(g_vga_buf, 0, 0);
	font_set_box(g_font, 0xc8, 0x6e, 0x10, 0);
	font_print_wrapped(g_font, txt[46]);
	mem_free(buf);
	delete b;
	wait_ticks_key(0x4b0);
	font_restore_colors(g_font);
	font_set_box(g_font, x1, y1, x2, y2);
	Sound_volume3(g_snd_music, 0, g_action == 0 ? 5000 : 1000, 0);
	wait_ticks_key(g_action == 0 ? 0xaa : 0x14);
	pal_fade_out();
	clear_screen_buf(g_vga_buf);
	g_refresh = 1;
	set_palette(pal2, 0, 0x100);
	snd_obj_stop(g_snd_music);
	mouse_show(0);
	mem_free(pal2);
	mem_free(pal);
	mem_free(txt_buf);
	mem_free(txt);
	g_game_state = 2;
	g_dialog_depth--;
	g_clock_update = old;
}

signed short get_group_index(struct group far *g)
{
	signed short r;
	signed short i;
	struct group far *p;

	r = -1;
	for (i = 0, p = g_groups; i < 6; i++, p = &g_groups[i])
		if (p == g)
			r = i;
	return r;
}

void group_leave_place(signed short v)
{
	snd_obj_stop(g_snd_music);
	g_group->route_pos = 0;
	g_group->set_town(g_group->set_town(0));
	g_group->set_loctype(g_group->set_loctype(LOC_NONE));
	g_group->set_dungeon(g_group->set_dungeon(0));
	group_set_on_route(g_group, 1);
	g_group->route_from = 0x67;
	g_group->route_to = 0x67;
	if (v != 0) {
		Ev far *o = new Ev;
		ev_show_pic_197(o);
		delete o;
	}
	if (g_mem_saving == 1)
		g_loaded_town = -1;
}

void prepare_spell_icons(void)
{
	Keylist far *o;
	struct hero far *h;
	signed short r;
	signed short i;
	signed short k;
	signed short base;

#if FIX_0207
	/* Fix #207: r vorbelegen */
	r = 0;
	o = new Keylist(0xac);
#else
	o = new Keylist(0xac);
#endif
	if (!!g_group->nr_heroes) {
		for (i = 0; i <= 6; i++) {
			h = g_group->heroes[i];
			if (h != 0 && h->is_mage()) {
				if (h->sex == 0)
					base = 0x1f;
				else
					base = 0x74;
				for (k = 1; k < 0x56; k++)
					if (h->spells[k] >= -5)
						r = o->add(base + k - 1, 0x6000);
			}
		}
		if (r == 0)
			;
		o->load(g_cd_cache);
	}
	delete o;
}
