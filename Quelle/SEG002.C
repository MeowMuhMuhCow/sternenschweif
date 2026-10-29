/*
 *	Rekonstruktion DSA 2 Sternenschweif C1.02 de, Segment seg002 (R02)
 *	Eingabe-Warteschleifen, Palette, Bildschirmschoner, Heldenpruefungen, zeitlich begrenzte Wirkungen
 *	(hero.effects[20], Arten EFFECT_*), Portraetleiste (LE/AE-Balken im Bars-Objekt g_bars), EMS,
 *	Eingabeverteiler (Funktionstasten, Klick auf Portraets) und Optionen-Taste.
 *	Compiler: Borland C++ 3.1
 *	  BCC -mlarge -3 -Ot -Og -c -Y -E..\bc\BIN\TASM.EXE SEG002.C
 *	  (OHNE -O, -Oe, -Ol, -Oi; MIT -Og; EMS_map in Inline-Assembler braucht TASM)
 *	  memcpy und memset werden aufgerufen, nicht eingebaut
 *
 *	Stand 17.10.2026: KOMPLETT BYTEGLEICH, 4.769/4.769 Bytes ab R02+0x05,
 *	alle 34 Funktionen, 4 Stellen far->near durch TLINK.
 *	(Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 *	- Objektparameter (Bildschirmschoner, Bars, EMS-Block) wie in s_proto.h als void far *, Zugriff ueber
 *	  Cast-Makros (gleicher Code). screensaver_reset/handle_input haben den (unbenutzten) Objektparameter,
 *	  den die Aufrufer uebergeben.
 *	- Bitfelder der Heldenstruktur (unsigned short) in Wahrheitstests mit (signed char)-Cast (frueher
 *	  unsigned-char-Bitfelder).
 */
#include <dos.h>
#include <mem.h>
#include "schweif.h"

#define SAVER	((struct saver far *)saver)
#define BARS	((struct bars far *)bars)
#define REQ	((struct mem_move far *)req)
#define PAL	((unsigned char far *)pal)
#define RGB	((unsigned char far *)rgb)

signed short wait_ticks_key(signed short n)
{
	signed short i;
	signed short hit;
	signed short retval;

	hit = 0;
	retval = 1;

	for (i = 0; i < n; i++) {
		g_in_wait = 1;
		poll_input();
		g_in_wait = 0;

		if (g_action || g_mouse_rclick || g_mouse_lclick) {
			hit = 1;
		}

		if (hit == 1) {
			if (g_key == KEY_ESC || g_mouse_rclick) {
				retval = 0;
			}
			mouse_clear_left();
			mouse_clear_right();
			g_mouse_event = g_mouse_ldclick = 0;
			g_action = KEY_RETURN;
			break;
		}

		wait_for_vsync();
	}

	return retval;
}

void wait_ticks(signed short n)
{
	signed short i;

	for (i = 0; i < n; i++) {
		g_in_wait = 1;
		poll_input();
		g_in_wait = 0;
		wait_for_vsync();
	}
}

char far *str_pad(char far *s, signed short n, char c)
{
	signed short i;

	for (i = 0; i < n; i++) {
		if (!s[i]) {
			s[i] = c;
			s[i + 1] = 0;
		}
	}

	return s;
}

void set_palette(void far *pal, signed short first, signed short count)
{
	signed short i;

	disable();
	outportb(0x3c8, first);

	for (i = 0; i < count; i++) {
		outportb(0x3c9, PAL[i * 3] &= 0x3f);
		outportb(0x3c9, (PAL + i * 3)[1] &= 0x3f);
		outportb(0x3c9, (PAL + i * 3)[2] &= 0x3f);
	}

	if (g_palette != pal) {
		memcpy(g_palette + first * 3, pal, count * 3);
	}

	enable();
}

void set_color(void far *rgb, signed short nr)
{
	disable();
	outportb(0x3c8, nr);
	outportb(0x3c9, RGB[0] &= 0x3f);
	outportb(0x3c9, RGB[1] &= 0x3f);
	outportb(0x3c9, RGB[2] &= 0x3f);
	enable();

	*(struct rgb far *)(g_palette + nr * 3) = *(struct rgb far *)rgb;
}

void check_palette(void)
{
#if FIX_0369
	/* Platz fuer Fix #369 in diesem residenten Segment (es darf nicht laenger werden): check_palette wird in
	 * C1.02 nirgends aufgerufen (toter Code) - der Rumpf entfaellt, die Funktion bleibt als leerer Einsprung */
#else
	unsigned char col[3];

	wait_for_vsync();
	get_color(col, 0xfd);

	if ((g_palette[0xfd * 3] != col[0]) || (g_palette[0xfd * 3 + 1] != col[1]) || (g_palette[0xfd * 3 + 2] != col[2])) {
		set_palette(g_palette, 0, 0x100);
	}
#endif
}

void set_mouse_handler2(signed short v)
{
	if (v && v != g_select_shown) {
		g_select_next = v;
	}
}

void set_menu_action(signed short v)
{
	if (v) {
		g_menu_line = v - 0x81;
	}
}

void screensaver_reset(void far *saver)
{
	g_idle_ticks = 0;
	g_saver_due = 0;
}

void screensaver_run(void far *saver)
{
	signed short pp20_bak;

	if (g_saver_due && g_saver_level && SAVER->busy == 0) {
		pp20_bak = g_pp20_index;
		SAVER->busy = 1;
		g_pp20_index = -1;
		g_dialog_depth++;
		mouse_hide(0);
		save_screen();
		saver_anim(saver);
		restore_screen();
		mouse_show(0);
		g_dialog_depth--;
		g_pp20_index = pp20_bak;
		SAVER->busy = 0;
		clear_kbd();
		clear_input();
		mouse_clear_left();
		mouse_clear_right();
	}
}

signed long clamp_long(signed long v, signed long lo, signed long hi)
{
	if (v < lo) {
		v = lo;
	}
	if (v > hi) {
		v = hi;
	}
	return v;
}

signed short check_hero(struct hero far *h)
{
	if (!h->type || (signed char)h->asleep || (signed char)h->dead || (signed char)h->petrified ||
		(signed char)h->unconscious || (signed char)h->renegade || (signed char)h->scared) {
		return 0;
	}
	return 1;
}

signed short check_hero2(struct hero far *h)
{
	if (!h->type || (signed char)h->dead || (signed char)h->petrified ||
		(signed char)h->unconscious || (signed char)h->renegade) {
		return 0;
	}
	return 1;
}

signed short check_hero3(struct hero far *h)
{
	if (!h->type || (signed char)h->dead || (signed char)h->petrified || (signed char)h->unconscious) {
		return 0;
	}
	return 1;
}

signed short get_item_pos(struct hero far *h, signed short item_id)
{
	signed short i;

	for (i = 0; i < NR_INV_SLOTS; i++) {
		if (h->inv[i].item_id == item_id) {
			return i;
		}
	}

	return -1;
}

signed short hero_is_diseased(struct hero far *h)
{
	signed short i;

	for (i = 1; i <= 7; i++) {
		if ((unsigned char)h->illness[i].state == (unsigned char)ILL_STATE_SICK) {
			return i;
		}
	}

	return 0;
}

signed short hero_is_poisoned(struct hero far *h)
{
	signed short i;

	for (i = 1; i <= 9; i++) {
		if ((unsigned char)h->poison[i].state == (unsigned char)ILL_STATE_SICK) {
			return i;
		}
	}

	return 0;
}

signed short hero_is_diseased1(struct hero far *h)
{
	signed short i;

	for (i = 1; i <= 7; i++) {
		if (h->illness[i].state == ILL_STATE_RECOVER) {
			return i;
		}
	}

	return 0;
}

signed short hero_is_poisoned1(struct hero far *h)
{
	signed short i;

	for (i = 1; i <= 9; i++) {
		if (h->poison[i].state == ILL_STATE_RECOVER) {
			return i;
		}
	}

	return 0;
}

void update_hero_bars(struct hero far *h, signed short slot)
{
	signed short ae;

	if ((signed char)h->dead) {
		ae = 0;
	} else {
		ae = h->ae;
	}

	if (h->saved_le != h->le || h->saved_le_max != h->le_max) {
		Bars_draw(g_bars, 0, slot, h->le, h->le_max, 0, 0);
		h->saved_le_max = h->le_max;
		h->saved_le = h->le;
	}

	if (h->saved_ae != ae || h->saved_ae_max != h->ae_max) {
		Bars_draw(g_bars, 1, slot, ae, h->ae_max, 0, 0);
		h->saved_ae_max = h->ae_max;
		h->saved_ae = ae;
	}
}

signed short get_free_effect(struct hero far *h)
{
	signed short i;

	for (i = 0; i < NR_HERO_EFFECTS; i++) {
		if (h->effects[i].time == 0) {
			break;
		}
	}

	if (i == NR_HERO_EFFECTS) {
#if FIX_0369
		/* Fix #369: Wirkung von Platz 0 auch im Heldenbogen (g_dialog_depth != 0) wirklich zuruecknehmen */
		h->effects[0].time = 1;
		i = g_dialog_depth;
		g_dialog_depth = 0;
		effects_tick(h, 1L);
		g_dialog_depth = i;
#else
		h->effects[0].time = 1;
		effects_tick(h, 1L);
#endif
		return 0;
	}

	return i;
}

void effects_tick(struct hero far *h, signed long t)
{
	signed short i;
	struct hero_effect far *e;

	if (g_dialog_depth == 0) {
		for (i = 0, e = h->effects; i < NR_HERO_EFFECTS; i++, e++) {
			if (e->time != 0) {
				if ((e->time -= t) <= 0) {
					e->time = 0;

					switch (e->type) {
					case EFFECT_ATTRIB: change_attrib(h, e->index, -e->value); break;
					case EFFECT_SKILL: add_skill_value(h, e->index, -e->value); break;
					case EFFECT_AT: h->at[e->index] -= e->value; break;
					case EFFECT_PA: h->pa[e->index] -= e->value; break;
					case EFFECT_MR: add_mr_mod(h, -e->value); break;
					case EFFECT_SPELL: h->spells[e->index] -= e->value; break;
					case EFFECT_HUNGER_TIMER: h->hunger_timer -= e->value; break;
					case EFFECT_AXXELERATUS: h->axxeleratus -= e->value; break;
					case EFFECT_SAFTKRAFT: h->saftkraft -= e->value; break;
					case EFFECT_RS_BONUS: h->rs_bonus -= e->value; break;
					case EFFECT_FIREBAN: h->fireban -= e->value; break;
					case EFFECT_INVISIBLE: h->invisible -= e->value; break;
					case EFFECT_UEBER_EIS: h->ueber_eis = 0; break;
					case EFFECT_HARMLOS: h->harmless = 0; break;
					case EFFECT_SAFTKRAFT_ON: h->saftkraft_on = 0; break;
					case EFFECT_SCHARFES_AUGE: h->scharfes_auge = 0; break;
					case EFFECT_SEE_UND_FLUSS: h->see_und_fluss = 0; break;
					case EFFECT_ADLER_WOLF: h->adler_wolf = 0; break;
					case EFFECT_DUNKELHEIT: h->dunkelheit = 0; break;
					case EFFECT_FLIM_FLAM: h->flim_flam = 0; break;
					}

					if (g_pp20_index == 0xe || g_pp20_index == 0xf) {
						g_refresh = 1;
					}
				}
			}
		}
	}
}

void add_effect(struct hero far *h, signed short slot, signed long time, signed char index, signed char value, signed char type)
{
	struct hero_effect far *e;

	e = &h->effects[slot];
	e->value = value;
	e->time = time;
	e->type = type;
	e->index = index;

	switch (type) {
	case EFFECT_ATTRIB: change_attrib(h, e->index, e->value); break;
	case EFFECT_SKILL: add_skill_value(h, index, value); break;
	case EFFECT_AT: h->at[index] += value; break;
	case EFFECT_PA: h->pa[index] += value; break;
	case EFFECT_MR: add_mr_mod(h, e->value); break;
	case EFFECT_SPELL: h->spells[index] += value; break;
	case EFFECT_HUNGER_TIMER: h->hunger_timer += value; break;
	case EFFECT_AXXELERATUS: h->axxeleratus += value; break;
	case EFFECT_SAFTKRAFT: h->saftkraft += value; break;
	case EFFECT_RS_BONUS: h->rs_bonus += value; break;
	case EFFECT_FIREBAN: h->fireban += value; break;
	case EFFECT_INVISIBLE: h->invisible += value; break;
	case EFFECT_UEBER_EIS: h->ueber_eis = 1; break;
	case EFFECT_HARMLOS: h->harmless = 1; break;
	case EFFECT_ADLER_WOLF: h->adler_wolf = 1; break;
	case EFFECT_SAFTKRAFT_ON: h->saftkraft_on = 1; break;
	case EFFECT_SCHARFES_AUGE: h->scharfes_auge = 1; break;
	case EFFECT_SEE_UND_FLUSS: h->see_und_fluss = 1; break;
	case EFFECT_DUNKELHEIT: h->dunkelheit = 1; break;
	case EFFECT_FLIM_FLAM: h->flim_flam = 1; break;
	}
}

void remove_effect(struct hero far *h, signed char type)
{
	signed short i;
	struct hero_effect far *e;

	for (i = 0, e = h->effects; i < NR_HERO_EFFECTS; i++, e++) {
		if (e->time != 0) {
			if (e->type == type) {
				e->time = 0;

				switch (e->type) {
				case EFFECT_ATTRIB: change_attrib(h, e->index, -e->value); break;
				case EFFECT_SKILL: add_skill_value(h, e->index, -e->value); break;
				case EFFECT_AT: h->at[e->index] -= e->value; break;
				case EFFECT_PA: h->pa[e->index] -= e->value; break;
				case EFFECT_MR: add_mr_mod(h, -e->value); break;
				case EFFECT_SPELL: h->spells[e->index] -= e->value; break;
				case EFFECT_HUNGER_TIMER: h->hunger_timer -= e->value; break;
				case EFFECT_AXXELERATUS: h->axxeleratus -= e->value; break;
				case EFFECT_SAFTKRAFT: h->saftkraft -= e->value; break;
				case EFFECT_RS_BONUS: h->rs_bonus -= e->value; break;
				case EFFECT_FIREBAN: h->fireban -= e->value; break;
				case EFFECT_INVISIBLE: h->invisible -= e->value; break;
				case EFFECT_UEBER_EIS: h->ueber_eis = 0; break;
				case EFFECT_HARMLOS: h->harmless = 0; break;
				case EFFECT_ADLER_WOLF: h->adler_wolf = 0; break;
				case EFFECT_SAFTKRAFT_ON: h->saftkraft_on = 0; break;
				case EFFECT_SCHARFES_AUGE: h->scharfes_auge = 0; break;
				case EFFECT_SEE_UND_FLUSS: h->see_und_fluss = 0; break;
				case EFFECT_DUNKELHEIT: h->dunkelheit = 0; break;
				case EFFECT_FLIM_FLAM: h->flim_flam = 0; break;
				}

				if (g_pp20_index == 0xe || g_pp20_index == 0xf) {
					g_refresh = 1;
				}
			}
		}
	}
}

unsigned char get_hero_index(struct hero far *h)
{
	unsigned char i;
	struct hero far *p;

	i = 0;
	p = g_heroes;

	while (h != p) {
		p = g_heroes + ++i;
	}

	return i;
}

signed short group_find_item(struct group far *grp, signed short item_id)
{
	signed short i;
	struct hero far *h;

	for (i = 0, h = grp->heroes[0]; i <= 6; h = grp->heroes[++i]) {
		if (h && get_item_pos(h, item_id) >= 0) {
			return i;
		}
	}

	return -1;
}

void bars_refresh(void far *bars)
{
	if (g_silent_mode == 0 && BARS->active != 0) {
		if (g_pp20_index == 0xe) {
			Bars_tick(bars);
			return;
		}
		if (g_pp20_index == 4) {
			bars_update_all(bars);
		}
	}
}

void bars_update_all(void far *bars)
{
	struct hero far *h;
	signed short empty[7];		/* Portraetplatz ohne Helden */
	signed short i;

	memset(empty, 1, 7);

	for (i = 0, h = g_heroes; i <= 6; i++, h++) {
		if (h->type != HERO_TYPE_NONE) {
			update_hero_bars(h, h->slot_pos);
			empty[h->slot_pos] = 0;
		}
	}

	for (i = 0; i <= 6; i++) {
		if (empty[i]) {
			bars_clear_slot(g_bars, i);
		}
	}
}

void bars_clear_slot(void far *bars, signed short slot)
{
	signed short j;
	signed short x;

	if (!BARS->drawn[slot]) {
		x = g_portrait_x[slot];
		mouse_hide(0);
		for (j = 0; j < 3; j++) {
			do_v_line(g_vga_buf, x + j + 0x22, 0x9e, 0x1e, 0);
			do_v_line(g_vga_buf, x + j + 0x26, 0x9e, 0x1e, 0);
		}
		mouse_show(0);
		BARS->drawn[slot] = 1;
	}
}

signed short EMS_map(unsigned char phys, signed short log, signed short handle)
{
	asm mov ah, 44h
	asm mov al, phys
	asm mov bx, log
	asm mov dx, handle
	asm int 67h
	asm mov al, ah
	asm mov ah, 0
	return _AX;
}

signed short EMS_map4(unsigned char phys, signed short log, signed short handle)
{
	if (g_ems_cur_page == log) {
		return 0;
	} else {
		g_ems_cur_page = log;
		EMS_map(phys, log, handle);
		EMS_map(phys + 1, log + 1, handle);
		EMS_map(phys + 2, log + 2, handle);
		return EMS_map(phys + 3, log + 3, handle);
	}
}

unsigned long EMS_copy(void far *req)
{
	signed short dir;		/* 0: Speicher -> EMS, 1: EMS -> Speicher */
	signed short handle;
	unsigned long ems;
	unsigned long remaining;
	unsigned long chunk;
	unsigned char huge *mem;

	dir = (REQ->src_handle == 0) ? 0 : 1;

	if (!dir) {
		handle = REQ->dst_handle;
		ems = REQ->dst;
		mem = (unsigned char huge *)REQ->src;
	} else {
		handle = REQ->src_handle;
		ems = REQ->src;
		mem = (unsigned char huge *)REQ->dst;
	}

	remaining = REQ->len;

	do {
		chunk = (remaining >= 0xc000UL) ? 0xc000UL : remaining;

		EMS_map4(0, ems >> 14, handle);

		if (!dir) {
			memcpy(MK_FP(g_ems_frame, ems & 0x3fff), (void far *)mem, (unsigned)chunk);
		} else {
			memcpy((void far *)mem, MK_FP(g_ems_frame, ems & 0x3fff), (unsigned)chunk);
		}

		ems += chunk;
		remaining -= chunk;
		mem += chunk;

	} while (remaining > 0);

	return REQ->len;
}

void handle_input(void far *screen)
{
	struct hero far *h;
	void far *inv;
	signed short slot;
	signed short count;

	poll_input();
	handle_options_key();

	if (g_key == KEY_F2 || g_key == KEY_CTRL_O) {
		swap_heroes_dialog();
		g_action = 0;
	} else if (g_key == KEY_F1 || g_key == KEY_CTRL_E) {
		open_hero_sheet();
		g_action = 0;
	} else if (g_key == KEY_F4) {
		g_action = ACTION_DATE;
	} else if (g_key == KEY_F5 || g_key == KEY_CTRL_J) {
		g_action = ACTION_DIARY;
	} else if (g_key == KEY_F8) {
		g_action = ACTION_MENU_SCREEN;
	}

	if (g_action == ACTION_MENU || g_key == KEY_F9 || g_action == ACTION_DIARY || g_action == ACTION_MENU_SCREEN) {
		game_menu();
	}

	if (g_action >= ACTION_PORTRAIT && g_action <= ACTION_PORTRAIT + 6) {

		h = 0L;
		count = g_group->nr_heroes;
		if (group_get_npc(g_group)) {
			count--;
		}

		slot = g_action - ACTION_PORTRAIT;

		if (slot == 6) {
			h = group_get_npc(g_group);
		} else if (slot < count) {
			h = g_group->heroes[slot];
		} else {
		}

		if (g_mouse_ldclick != 0) {
			if (h != 0L) {
				snd_obj_stop(g_snd_music);
				ani_stop(g_screen_pic);
				swap_out_screen();
				inv = Inv_ctor(0, 0);
				g_mouse_ldclick = 0;
				g_action = 0;
				hero_sheet_loop(inv, h);
				Inv_dtor(inv, 3);
			}
		} else if (g_portrait_drag != 0 && h != 0L) {
			drag_hero_portrait(g_action - ACTION_PORTRAIT);
			g_action = 0;
		}
	}

	if (g_action == ACTION_DATE) {
		Date_print(g_date);
		dlg_message(g_dialog, g_text_output_buf);
	}
}

void handle_options_key(void)
{
	signed short swapped;
	void far *opt;
	signed short music;

	g_options_shown = 0;

	if (g_paused == 0) {
		if (((g_key == KEY_F3 || g_key == KEY_CTRL_S) && g_options_open == 0) || g_action == ACTION_OPTIONS) {
			music = g_music_id_req & 0x3fff;
			snd_obj_stop(g_snd_music);
			ani_stop(g_screen_pic);
			swapped = swap_out_screen();
			opt = Opt_ctor_o199(0, 0);
			Opt_run_o199(opt);
			Opt_dtor_o199(opt, 3);
			g_options_shown = 1;
			if (swapped) {
				swap_in_screen();
			}
			if (music != -1) {
				snd_play(g_snd_music, music);
			}
			clear_input();
		}
	}
}
