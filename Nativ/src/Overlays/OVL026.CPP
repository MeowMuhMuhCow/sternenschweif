/* OVL026 - Overlay O026 (Segment 0x1f84 in SCHWEIF.EXE): Heldenerschaffung - Rahmen (class CharGen, 0x137 Bytes, in
 * s_class.h; Mitglieder Nvf +4e und Pic +67). CharGen(): Tabellen aus g_file_main (0x9c, 0x1f5..0x1f9), Texte
 * txt_load_list(0x1e1, 0x4b), Bmp-Objekte, Symbole 0x1e2 per Nvf::load. ~CharGen(): alles frei, g_refresh = 1.
 * chargen_draw_name(g): Name (bis Breite 90) bei (0xb4, 12). chargen_enter_name(g): Name eingeben (dlg_message_flag,
 * 15 Zeichen, Wortanfaenge gross, Leerzeichen am Ende weg) -> file_name und name. chargen_draw_portrait(g): Portraet
 * 32x32 (load_portrait) auf Seite 0 bzw. > 4. chargen_select_portrait(g): Portraet aus dem Bereich je Typ/Geschlecht
 * (g_portrait_index_m/_f, dlg_select_pic) -> g->portrait, hero.portrait. chargen_run(g): Hauptschleife (Menue auf
 * Seite 0 mit 11 Punkten, Seiten 1..10 chargen_menu_skills/chargen_change_atpa/chargen_menu_spells, Aktionen 0x60..0x62,
 * Blaettern KEY_LEFT/KEY_RIGHT, Tasten '1'..'5' -> Seite), Ergebnis g->saved. chargen_redraw(g): Seite neu zeichnen
 * (Pic::loadf Hintergrund 0x1e3 + Seite). chargen_can_move_attribs(g): 1, wenn Eigenschaften noch verschoben werden
 * koennen.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL026.CPP   -> 5359/5359 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (g_chargen_name_init -> _g_1832+0x6). Ohne -Og 1 Byte zu lang, -Oi falsch (strcpy/memcpy/memset sind Aufrufe).
 * - Ktor/Dtor als echte Klasse: Mitglieder Nvf/Pic werden automatisch konstruiert (O098_0000, O269_004E nach dem
 *   new-Test) und am Ende des Dtors zerstoert (O269_00AD(+67, 2), O098_003D(+4e, 2)); &dummy zweimal als Argument
 *   -> einmal in SI gerechnet.
 * - chargen_run: "if (sel == -1) break;" im case 0 springt direkt hinter den aeusseren switch (mit if-Block landet der
 *   Sprung auf dem break-jmp des case, 2 Bytes falsch). Leerer then-Zweig "if (g_game_mode == ..) ; else if .." ->
 *   jnz short / jmp short. inline hero::is_mage() { return type >= 7; } -> mov ax,1 / xor ax,ax / or ax,ax.
 * - chargen_draw_name: "struct str16 nm = g_chargen_name_init;" (Strukturkopie mov si / lea di / push ss / pop es /
 *   rep movsw).
 * - switch (g->page) mit gemeinsamen Rumpfen (1-3, 5-10), Tabelle hinter der des inneren switch.
 * Originalfehler: chargen_can_move_attribs rechnet den Spielraum zum Senken als "8 - v" bzw. "2 - v" statt v - 8 /
 *   v - 2 (Summen immer <= 0): "down_neg < 2" ist damit immer wahr, die Pruefung haengt nur noch an up_pos und
 *   up_neg. chargen_select_portrait kopiert die Portraetfelder g_portrait_rects (0x48 Bytes) in ein unbenutztes Local (ebenso
 *   O093 select_portrait).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Phase B (19.10.2026): eigene Daten im Quelltext. Neu benannt: Mausbereichslisten g_chargen_areas_*, Portraetfelder
 *   g_portrait_rects, Startgeldlisten g_type_money_01..12 mit der Tabelle g_type_money_ranges[12] (Index type - 1;
 *   MAP-Name g_type_money = 4 Bytes davor). g_portrait_index_m hat 12 Eintraege (g_portrait_index_m_next = [1..]).
 */
#include <string.h>
#include <ctype.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:1542..184a, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_type_le[13] = { 0, 30, 30, 30, 30, 30, 40, 25, 30, 25, 25, 30, 25 };
signed short g_type_ae[13] = { 0, 0, 0, 0, 0, 0, 0, 25, 25, 30, 25, 25, 25 };
struct height_range g_type_height[13] = {	/* Koerpergroesse in cm von, bis je Heldentyp */
	{ 0, 0 },	/* HERO_TYPE_NONE */
	{ 150, 188 },	/* HERO_TYPE_JUGGLER */
	{ 155, 170 },	/* HERO_TYPE_HUNTER */
	{ 165, 205 },	/* HERO_TYPE_WARRIOR */
	{ 156, 194 },	/* HERO_TYPE_ESTRAY */
	{ 167, 210 },	/* HERO_TYPE_THORWALIAN */
	{ 130, 140 },	/* HERO_TYPE_DWARF */
	{ 154, 188 },	/* HERO_TYPE_WITCH */
	{ 154, 188 },	/* HERO_TYPE_DRUID */
	{ 164, 197 },	/* HERO_TYPE_MAGE */
	{ 170, 204 },	/* HERO_TYPE_GREEN_ELF */
	{ 160, 194 },	/* HERO_TYPE_ICE_ELF */
	{ 170, 210 } 	/* HERO_TYPE_SYLVAN_ELF */
};
unsigned char g_type_weight[13] = { 0, 120, 110, 100, 110, 100, 90, 120, 110, 110, 120, 120, 120 };	/* Gewicht: (Groesse - Wert) * 40 Unzen */
struct mouse_area g_chargen_areas_main[8] = {	/* Seite 0 (Grundwerte) */
	{ 16, 8, 143, 191, 253 },
	{ 272, 8, 304, 41, 98 },
	{ 305, 7, 319, 21, 96 },
	{ 145, 13, 175, 21, 97 },
	{ 145, 178, 164, 192, KEY_LEFT },
	{ 284, 178, 303, 192, KEY_RIGHT },
	{ 0, 0, 319, 199, 254 },
	{ -1, -1, -1, -1, -1 }
};
struct mouse_area g_chargen_areas_skills[4] = {	/* Seiten 1..4 (Talente, AT/PA): blaettern */
	{ 145, 178, 164, 192, KEY_LEFT },
	{ 284, 178, 303, 192, KEY_RIGHT },
	{ 0, 0, 319, 199, 254 },
	{ -1, -1, -1, -1, -1 }
};
struct mouse_area g_chargen_areas_spells[4] = {	/* Seiten 5..10 (Zauber): blaettern */
	{ 16, 178, 35, 192, KEY_LEFT },
	{ 284, 178, 303, 192, KEY_RIGHT },
	{ 0, 0, 319, 199, 254 },
	{ -1, -1, -1, -1, -1 }
};
void far *g_chargen_mouse_areas[11] = {	/* Mausbereiche je Seite (CharGen.page) */
	g_chargen_areas_main,
	g_chargen_areas_skills, g_chargen_areas_skills, g_chargen_areas_skills, g_chargen_areas_skills,
	g_chargen_areas_spells, g_chargen_areas_spells, g_chargen_areas_spells, g_chargen_areas_spells,
	g_chargen_areas_spells, g_chargen_areas_spells
};
struct rect g_portrait_rects[9] = {	/* Portraetfelder der Auswahl (x0, y0, x1, y1) */
	{ 0, 12, 31, 43 }, { 34, 12, 65, 43 }, { 68, 12, 99, 43 }, { 102, 12, 133, 43 }, { 136, 12, 167, 43 },
	{ 170, 12, 201, 43 }, { 204, 12, 235, 43 }, { 238, 12, 269, 43 }, { 272, 12, 303, 43 }
};

/* Portraetbereiche je Bildtyp: maennlich m[t] .. f[t] - 1, weiblich f[t] .. m[t + 1] - 1 */
signed char g_portrait_index_m[12] = { 0, 0, 12, 24, 36, 48, 60, 72, 83, 95, 107, 123 };
signed char g_portrait_index_f[11] = { 0, 6, 18, 30, 42, 54, 68, 74, 91, 101, 115 };
/* Startgeld je Typ: Eintraege {Grenze fuer den W20, Spanne in Silber von, bis}, letzter mit Grenze 20 */
struct money_range g_type_money_01[] = {	/* HERO_TYPE_JUGGLER */
	{ 14, 5, 5 }, { 19, 100, 100 }, { 20, 300, 300 }
};
struct money_range g_type_money_02[] = { { 1, 1, 1 }, { 20, 0, 0 } };	/* HERO_TYPE_HUNTER */
struct money_range g_type_money_03[] = {	/* HERO_TYPE_WARRIOR */
	{ 2, 1, 1 }, { 4, 20, 20 }, { 7, 50, 50 }, { 10, 30, 30 }, { 18, 200, 200 }, { 20, 1000, 1000 }
};
struct money_range g_type_money_04[] = {	/* HERO_TYPE_ESTRAY */
	{ 10, 0, 0 }, { 17, 2, 2 }, { 18, 5, 5 }, { 20, 10, 10 }
};
struct money_range g_type_money_05[] = {	/* HERO_TYPE_THORWALIAN */
	{ 9, 10, 10 }, { 13, 100, 100 }, { 19, 200, 200 }, { 20, 500, 500 }
};
struct money_range g_type_money_06[] = {	/* HERO_TYPE_DWARF */
	{ 7, 100, 100 }, { 16, 200, 200 }, { 19, 300, 300 }, { 20, 1200, 1200 }
};
struct money_range g_type_money_07[] = { { 20, 1, 20 } };	/* HERO_TYPE_WITCH */
struct money_range g_type_money_08[] = {	/* HERO_TYPE_DRUID */
	{ 14, 0, 1 }, { 15, 1, 6 }, { 16, 2, 12 }, { 18, 1, 20 }, { 19, 10, 60 }, { 20, 10, 200 }
};
struct money_range g_type_money_09[] = {	/* HERO_TYPE_MAGE */
	{ 3, 1, 6 }, { 13, 10, 60 }, { 16, 10, 200 }, { 19, 20, 400 }, { 20, 5, 1000 }
};
struct money_range g_type_money_10[] = {	/* HERO_TYPE_GREEN_ELF */
	{ 4, 1, 6 }, { 6, 1, 20 }, { 13, 10, 60 }, { 19, 10, 200 }, { 20, 20, 400 }
};
struct money_range g_type_money_11[] = { { 1, 1, 6 }, { 20, 1, 20 } };	/* HERO_TYPE_ICE_ELF */
struct money_range g_type_money_12[] = { { 14, 1, 6 }, { 19, 1, 20 }, { 20, 10, 60 } };	/* HERO_TYPE_SYLVAN_ELF */
struct money_range far *g_type_money_ranges[12] = {	/* je Heldentyp 1..12 (Index type - 1) */
	g_type_money_01, g_type_money_02, g_type_money_03, g_type_money_04, g_type_money_05, g_type_money_06,
	g_type_money_07, g_type_money_08, g_type_money_09, g_type_money_10, g_type_money_11, g_type_money_12
};
signed short g_chargen_pics[22] = {
	0x21e3, 0x21e4, 0x21e5, 0x21e6, 0x21e7, 0x21e8, 0x21e9, 0x21ea, 0x21eb, 0x21ec, 0x21ed, 0x21f9, 0x21f5,
	0x21f8, 0x21f6, 0x21f7, 0x209c, 0x21ef, 0x21ee, 0x21e2, 0x21e1, -1
};
unsigned short g_chargen_pics_state[3] = { 0x0000, 0x0000, 0x0000 };
struct str16 g_chargen_name_init = { "" };
signed char g_chargen_type_shown = 1;

/* font_set_colors mit Wort-Parameter (das Original uebergibt die gesicherte Farbe als short: push word) */
#define font_set_colors_w ((void (*)(void far *, signed short, signed short))font_set_colors)

CharGen::CharGen()
{
	signed long size;

	snd_obj_stop(g_snd_music);
	ani_stop(g_screen_pic);
	load_pic_list(g_cd_cache, g_chargen_pics, g_chargen_pics_state, 1, 0);
	type_pal = (unsigned char far *)mem_alloc(0x60L);
	skill_incs = (struct inc_counter far *)mem_alloc(NR_SKILLS * sizeof(struct inc_counter));
	spell_incs = (struct inc_counter far *)mem_alloc(NR_SPELLS * sizeof(struct inc_counter));
	file_open(g_file_main, 0x9c);
	size = file_length2(g_file_main);
	auto_skills = (struct skill_tpl (far *)[25])mem_alloc(size);
	file_read(g_file_main, auto_skills, size);
	file_close(g_file_main);
	type_reqs = (struct type_req (far *)[6])mem_alloc(0x9cL);
	file_load(g_file_main, 0x1f5, type_reqs);
	type_skills = (signed char (far *)[NR_SKILLS])mem_alloc(0x2a4L);
	file_load(g_file_main, 0x1f6, type_skills);
	type_spells = (signed char (far *)[NR_SPELLS])mem_alloc(0x204L);
	file_load(g_file_main, 0x1f7, type_spells);
	schools = (struct school_bonus far *)mem_alloc(0x129L);
	file_load(g_file_main, 0x1f8, schools);
	auto_spells = (signed short (far *)[45])mem_alloc(0x21cL);
	file_load(g_file_main, 0x1f9, auto_spells);
	txt_load_list(g_font, 0x1e1, 0x4b, &txt_buf, &txt);
	hero = (struct hero far *)mem_alloc((signed long)sizeof(struct hero));
	bg = mem_alloc(0xfa00L);
	show_moved = 0;
	bmp_name = new Bmp(0x5e, 9);
	bmp_male = new Bmp(0x0f, 0x0f);
	bmp_female = new Bmp(0x0f, 0x0f);
	bmp_locked = new Bmp(0x14, 0x0f);
	bmp_type = new Bmp(0x80, 0xb8);
	bmp_bg = new Bmp(SCREEN_W, SCREEN_H, bg);
	bmp_save1 = 0;
	bmp_save2 = 0;
	bmp_save3 = 0;
	nvf.load(0x1e2, bmp_male->data(), 0, 0, 0, 0, &dummy, &dummy, SCREEN_W);
	nvf.load(0x1e2, bmp_female->data(), 0, 0, 1, 0, &dummy, &dummy, SCREEN_W);
	nvf.load(0x1e2, bmp_locked->data(), 0, 0, 2, 0, &dummy, &dummy, SCREEN_W);
	page = CHARGEN_PAGE_MAIN;
	saved = 0;
	dlg_set_pos_x(g_dialog, -100);
	clear_screen_buf(g_vga_buf);
}

CharGen::~CharGen()
{
	if (bmp_save1)
		delete bmp_save1;
	if (bmp_save2)
		delete bmp_save2;
	if (bmp_save3)
		delete bmp_save3;
	delete bmp_bg;
	delete bmp_type;
	delete bmp_locked;
	delete bmp_female;
	delete bmp_male;
	delete bmp_name;
	mem_free(hero);
	mem_free(bg);
	mem_free(txt_buf);
	mem_free(txt);
	mem_free(type_reqs);
	mem_free(type_skills);
	mem_free(type_spells);
	mem_free(schools);
	mem_free(auto_spells);
	mem_free(auto_skills);
	mem_free(skill_incs);
	mem_free(spell_incs);
	mem_free(type_pal);
	clear_screen_buf(g_vga_buf);
	g_refresh = 1;
	dlg_set_pos_x(g_dialog, 0);
}

void chargen_draw_name(CharGen far *g)
{
	signed short i;
	signed short done = 0;
	struct str16 nm = g_chargen_name_init;
	char buf[16];

	i = 0;
	strcpy(buf, g->hero->name);
	do {
		nm.s[i] = buf[i];
		if (!nm.s[i])
			done = 1;
		if (font_text_width0(g_font, nm.s) > 90) {
			nm.s[i] = 0;
			done = 1;
		}
		i++;
	} while (done == 0);
	mouse_hide(0);
	g->bmp_name->put(g_font_dst, 0xb0, 0x0c);
	font_set_colors(g_font, 0xff, -1);
	font_print(g_font, nm.s, 0xb4, 0x0c);
	mouse_show(0);
}

void chargen_enter_name(CharGen far *g)
{
	signed short i;

	if (dlg_message_flag(g_dialog, 15, g->txt[1]) != -1) {
		str_tolower(g_input_buf);
		g_input_buf[0] = char_toupper(g_input_buf[0]);
		for (i = 1; g_input_buf[i] != 0; i++) {
			if (g_input_buf[i] == ' ' && g_input_buf[i + 1] != 0)
				g_input_buf[i + 1] = toupper(g_input_buf[i + 1]);
		}
		i--;
		while (g_input_buf[i] == ' ') {
			g_input_buf[i] = 0;
			i--;
		}
		strcpy(g->hero->file_name, g_input_buf);
		strcpy(g->hero->name, g_input_buf);
		chargen_draw_name(g);
	}
}

void chargen_draw_portrait(CharGen far *g)
{
	Bmp b(0x20, 0x20, 0);

	b.set_base(load_portrait(g->hero));
	if (!g->page)
		b.put(g_vga_buf, 0x110, 8);
	else if (g->page > CHARGEN_PAGE_ATPA)
		b.put(g_vga_buf, 0x110, 4);
}

void chargen_select_portrait(CharGen far *g)
{
	signed short n;
	signed short r;
	char tab[0x48];

	if (!g->hero->type) {
		dlg_message(g_dialog, g->txt[14]);
		return;
	}
	memcpy(tab, g_portrait_rects, 0x48);
	if (g->hero->sex != 0) {
		g->portrait_first = g_portrait_index_f[g->pic_type];
		g->portrait_last = g_portrait_index_m[g->pic_type + 1] - 1;
	} else {
		g->portrait_first = g_portrait_index_m[g->pic_type];
		g->portrait_last = g_portrait_index_f[g->pic_type] - 1;
	}
	n = g->portrait_last - g->portrait_first + 1;
	r = dlg_select_pic(g_dialog, n, g->portrait_first);
	if (r != -1) {
		g->portrait = r;
		g->hero->set_portrait(g->portrait);
	}
	mouse_hide(0);
	chargen_draw_portrait(g);
	mouse_show(0);
}

signed short chargen_run(CharGen far *g)
{
	signed short sel;
	signed short fg;
	signed short bg;
	signed short done;

	done = 0;
	g_refresh = 1;
	g_dialog_depth++;
	snd_play(g_snd_music, 0x0d);
	g_mouse_rclick = 1;
	font_get_colors(g_font, &fg, &bg);
	font_set_colors(g_font, 0xff, -1);
	while (!done) {
		if (g_refresh != 0)
			chargen_redraw(g);
		g_mouse_areas2 = g_chargen_mouse_areas[g->page];
		poll_input();
		handle_options_key();
		g_mouse_areas2 = 0;
		if (g_action == KEY_ESC && dlg_yes_no(g_dialog, g->txt[53]) == 1)
			done = 1;
		if (g_mouse_rclick != 0 || g_action == KEY_PGUP) {
			switch (g->page) {
			case CHARGEN_PAGE_MAIN:
				sel = dlg_choose(g_dialog, g->txt[4], 11, g->txt[56], g->txt[11], g->txt[9], g->txt[12],
					g->txt[7], g->txt[8], g->txt[68], g->txt[70], g->txt[6], g->txt[5], g->txt[52]);
				if (sel == -1)
					break;
				/* neu wuerfeln / Typ / neuer Held / Eigenschaften verschieben verwerfen die Werte */
				if ((sel == 1 || sel == 2 || sel == 4 || sel == 10) && g->hero->attrib[ATTRIB_MU].normal != 0 &&
				    dlg_yes_no(g_dialog, g->txt[10]) != 1)
					sel = 0;
				g_action = 0;
				switch (sel) {
				case 1:
					chargen_new_hero(g);
					break;
				case 2:
					chargen_roll_attribs(g);
					break;
				case 3:
					chargen_select_type(g);
					break;
				case 4:
					chargen_move_attribs(g);
					break;
				case 5:
					chargen_enter_name(g);
					break;
				case 6:
					chargen_toggle_sex(g);
					break;
				case 7:
					chargen_select_portrait(g);
					break;
				case 8:
					if (!g->hero->type)
						dlg_message(g_dialog, g->txt[22]);
					else if (dlg_yes_no(g_dialog, g->txt[71]) == 1)
						hero_print_sheet(g->hero);
					break;
				case 9:
					if (chargen_save_hero(g)) {
						memset(g->hero, 0, sizeof(struct hero));
						chargen_reset_incs(g);
						g_mouse_rclick = g_refresh = 1;
					}
					break;
				case 10:
					memset(g->hero, 0, sizeof(struct hero));
					chargen_reset_incs(g);
					g_mouse_rclick = g_refresh = 1;
					break;
				case 11:
					if (dlg_yes_no(g_dialog, g->txt[53]) == 1)
						done = 1;
					break;
				}
				break;
			case 1:
			case 2:
			case 3:
				chargen_menu_skills(g);
				break;
			case CHARGEN_PAGE_ATPA:
				chargen_change_atpa(g);
				break;
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				chargen_menu_spells(g);
				break;
			}
		}
		if (g_action == 0x60)
			chargen_toggle_sex(g);
		else if (g_action == 0x61)
			chargen_enter_name(g);
		else if (g_action == 0x62)
			chargen_select_portrait(g);
		if (g_action == KEY_RIGHT) {
			if (g_game_mode == GAME_MODE_BEGINNER)
				;
			else if (!g->hero->type)
				dlg_message(g_dialog, g->txt[22]);
			else {
				g_refresh = 1;
				if (g->page < (g->hero->is_mage() ? CHARGEN_PAGE_LAST : CHARGEN_PAGE_ATPA))
					g->page++;
				else
					g->page = CHARGEN_PAGE_MAIN;
			}
		}
		if (g_action == KEY_LEFT) {
			if (g->page > CHARGEN_PAGE_MAIN) {
				g_refresh = 1;
				g->page--;
			} else if (g_game_mode == GAME_MODE_BEGINNER)
				;
			else if (!g->hero->type)
				dlg_message(g_dialog, g->txt[22]);
			else {
				g_refresh = 1;
				g->page = g->hero->is_mage() ? CHARGEN_PAGE_LAST : CHARGEN_PAGE_ATPA;
			}
		}
		/* Tasten '1'..'5': Seite 0, 1, 4, 5, 10 */
		if (g_action >= 0x231 && g_action <= 0x635 && g_game_mode == GAME_MODE_ADVANCED && g->hero->type != 0) {
			sel = g_action == 0x231 ? 0 : g_action == 0x332 ? 1 : g_action == 0x433 ? 4 : g_action == 0x534 ? 5 : 10;
			if (g->page != sel && (sel < CHARGEN_PAGE_SPELLS || g->hero->is_mage())) {
				g->page = sel;
				g_refresh = 1;
			}
		}
	}
	font_set_colors_w(g_font, fg, bg);
	snd_obj_stop(g_snd_music);
	g_dialog_depth--;
	return g->saved;
}

void chargen_redraw(CharGen far *g)
{
	if (g_refresh != 0) {
		font_set_screen(g_font, g->bg);
		g_pp20_index = g->page + 0x1e3;
		g->pic.loadf(g_pp20_index, g->bg);
		chargen_save_bg(g);
		if (!g->page) {
			g->bmp_name->get(g->bg, 0xb0, 0x0c);
			if (g->hero->sex != 0)
				g->bmp_female->put(g->bg, 0x131, 7);
			if (g_game_mode == GAME_MODE_BEGINNER) {
				g->bmp_locked->put(g->bg, 0x11c, 0xb2);
				g->bmp_locked->put(g->bg, 0x91, 0xb2);
			}
		}
		if (g->page < CHARGEN_PAGE_SPELLS) {
			if (g->hero->type != 0) {
				g_chargen_type_shown = 1;
				g->bmp_type->put(g->bg, 0x10, 8);
				font_set_colors(g_font, 0xff, -1);
				/* Typname: weiblich ab g_ttx[0x3e + 1], maennlich ab g_ttx[0x32 + 1] */
				if (g->hero->sex != 0)
					font_print(g_font, g_ttx[g->hero->type + 62],
						font_center_x(g_font, g_ttx[g->hero->type + 62], 0x10, 0x80), 0xb8);
				else
					font_print(g_font, g_ttx[g->hero->type + 50],
						font_center_x(g_font, g_ttx[g->hero->type + 50], 0x10, 0x80), 0xb8);
			} else {
				if (g_chargen_type_shown != 0) {
					g->bmp_type->clear();
					g->bmp_type->put(g->bg, 0x10, 8);
					g_chargen_type_shown = 0;
				}
				chargen_draw_type_pic(g, g->bg, 0);
			}
		}
		mouse_hide(0);
		chargen_draw_page(g);
		g->bmp_bg->put(g_vga_buf, 0, 0);
		if (g->hero->type != 0)
			chargen_draw_portrait(g);
		mouse_show(0);
		font_set_screen(g_font, g_vga_buf);
	} else {
		mouse_hide(0);
		chargen_draw_page(g);
		mouse_show(0);
	}
	g_refresh = 0;
}

signed short chargen_can_move_attribs(CharGen far *g)
{
	signed short up_pos = 0;
	signed short down_pos = 0;
	signed short up_neg = 0;
	signed short down_neg = 0;
	signed short i;
	signed char v;

	for (i = ATTRIB_MU; i <= ATTRIB_KK; i++) {
		v = g->hero->attrib[i].normal;
		if (g->attr_moved[i] != 1 && v > 8)
#if FIX_0145
			/* Fix #145: Spielraum zum Senken v - 8 */
			down_pos += v - 8;
#else
			down_pos += 8 - v;
#endif
		if (g->attr_moved[i] != -1 && v < 13)
			up_pos += 13 - v;
	}
	for (i = ATTRIB_AG; i <= ATTRIB_JZ; i++) {
		v = g->hero->attrib[i].normal;
		if (g->attr_moved[i] != 1 && v > 2)
#if FIX_0145
			/* Fix #145: Spielraum zum Senken v - 2 */
			down_neg += v - 2;
#else
			down_neg += 2 - v;
#endif
		if (g->attr_moved[i] != -1 && v < 8)
			up_neg += 8 - v;
	}
	if ((up_pos == 0 && down_pos == 0) || (up_pos == 0 && down_neg < 2) || (up_neg < 2 && down_pos == 0) ||
	    (down_neg < 2 && up_neg < 2))
		return 0;
	return 1;
}
