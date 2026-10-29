/* OVL183 - Overlay O183 (Segment 0x22ee in SCHWEIF.EXE): Ereignisobjekt einer Stadt, class Town (s_class.h; 0x12
 * Bytes: Texte txt_buf/txt, Stadt id +8, Vorgabe desc +0a = g_town_descs[id] mit dem Kartenereignis-Rueckruf), hier
 * mit den Methoden der Hafenstadt. Konstruktor (new(0x12), txt_load_list(g_font, a, b, &txt_buf, &txt)), Destruktor
 * (beide Puffer frei). enter(loctype, ev): Ort betreten - g_exit_dir = Gegenrichtung von cell_dir(ev) (ausser
 * Stadt 3), dann group.loctype = loctype (set_loctype: alte nach old_loctype, wenn Bit 7 frei). cell_dir(ev):
 * Drehung der Zelle g_cells[ev->x][ev->y] - 1. harbor_event(ev): Rueckruf wie O186 (pos = (Spalte - g_map_x0) << 8
 * | (Zeile - g_map_y0)): MAP_EVENT_ENTER an zehn Hafenfeldern (ohne g_voyage_done) -> ship_voyage,
 * enter(LOC_DATA | 3), Zeit timewarp_until(g_date, 108000), Musik aus, group_set_screen_pos(11, 9); 0x1011 ->
 * enter(LOC_DATA | 9); Feldtyp 3 (get_cell_a) -> swim_check. Aktion 1: switch pos -> enter(LOC_DATA | 1..9),
 * sonst enter(LOC_HOUSE). Aktion 100: Marktstand 1..3 nach pos, loc_load_data(7), market_stall(k),
 * enter(LOC_MERCHANT). market_stall(k): Laden 0x18 einrichten - g_market_buy/sell/lvl/female aus
 * g_markets[g_loc_id], Art k, g_loc_id = 0x18 (O226 liest die Ladendaten ab g_market_buy). ship_voyage:
 * Schiffsreise-Bildfolge (Texte 0x219 neu laden, NVF-Bilder 0x2e5 per Nvf::load in Bmp-Objekte, Bildfolgen O041
 * (zwei lokale Objekte Outro1/Outro2), Merker g_voyage_done, g_vindaria_state .. = 1). show_anim3: Bildfolge O040 + Tagebuch
 * txt[19], g_topic_sternenschweif = 1. swim_check: schlechtester Schwimmer (count_failed_skill/get_worst_hero_skill, TA_SCHWIMMEN):
 * wenn Bit see_und_fluss fehlt, Schaden random_schweif(20) mit Meldung g_ttx[443] bzw. [442] (Pronomen grammar_pronoun).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL183.CPP   -> 2533/2533 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++: Konstruktor/Destruktor, Methoden (cell_dir/ship_voyage/market_stall/swim_check unten -> far call), lokale
 *   Objekte
 *   Outro1/Outro2/Anim3 mitten in der Funktion deklariert -> Konstruktor an der Deklarationsstelle, Destruktor
 *   (&o, 2) am Funktionsende in umgekehrter Reihenfolge; new Bmp(0x40, 0x40) / delete b wie O184.
 * - inline group::set_loctype(v): "if (!(loctype & 0x80)) old_loctype = loctype; else ; return loctype = v;" - KEIN
 *   Local: -Og legt loctype als Zwischenwert nach [bp-3] (mov [bp-3],al / cbw / test ax,80h) und laedt ihn VOR dem
 *   les (mov al,[bp-3] / les bx / mov es:[bx+1f],al). Mit eigenem Local c kommt les zuerst. Parameterkopie
 *   v = [bp-2] (in market_stall mit Konstante 0 keine Kopie); das Ergebnis (cbw) wird verworfen.
 * - Konstruktor: "desc = g_town_descs[n]" -> SCOPY@ (DS -> far, cx = 8).
 * - harbor_event: pos in zwei Anweisungen (wie O186); drei switch mit Faellen in Quellreihenfolge, default am Ende;
 *   im dritten switch kein default (k bleibt ggf. uninitialisiert - Originalfehler).
 * - market_stall: register-Parameter k in CX (keine Aufrufe), -Og-Offset g_loc_id * 11 in SI;
 *   "g_market_buy = g_market_sell = ..." speichert sell zuerst. Die fuenf Felder haben eigene Publics (eigene
 *   Symbole statt einer Struktur).
 */
#include <stdio.h>
#include "schweif.h"

#define NVF_HEADS ((Nvf far *)g_nvf_heads)	/* NVF-Dateiobjekt (O098) */

Town::Town(register signed short n, signed short txt_file, signed short txt_n)
{
	id = n;
	txt_load_list(g_font, txt_file, txt_n, &txt_buf, &txt);
	desc = g_town_area_descs[n - 1];
}

Town::~Town()
{
	mem_free(txt_buf);
	mem_free(txt);
}

void Town::enter(signed short loctype, struct map_event far *ev)
{
	if (g_group->town != 3)
		g_exit_dir = (cell_dir(ev) + 2) & 3;
	g_group->set_loctype(loctype);
}

signed short Town::cell_dir(struct map_event far *ev)
{
	struct map_cell far *c;

	c = &g_cells[ev->x][ev->y];	/* map_event: +2 = Zeile, +3 = Spalte (s_struct.h) */
	return c->rot - 1;
}

signed short Town::harbor_event(struct map_event far *ev)
{
	signed short loc;
	signed short r;
	signed short k;
	register signed short type;
	register signed short xy;

	type = ev->type;
	xy = (ev->y - g_map_x0) << 8;
	xy |= ev->x - g_map_y0;
	r = 0;
	if (type == MAP_EVENT_ENTER) {
		if ((xy == 0xe07 || xy == 0xf07 || xy == 0xf08 || xy == 0x1007 || xy == 0xe08 ||
			xy == 0xe09 || xy == 0xf09 || xy == 0xd08 || xy == 0xf06 || xy == 0xf0a) && !g_voyage_done) {
			ship_voyage();
			g_kvirasim_arrived = 1;
			enter(LOC_DATA | 3, ev);
			screen_redraw(g_screen);
			group_clr_hunger_thirst(g_group);
			timewarp_until(g_date, 108000L);
			snd_obj_stop(g_snd_music);
			group_clr_hunger_thirst(g_group);
			g_exit_dir = 3;
			group_set_screen_pos(0x0b, 9);
			r = 1;
		}
		if (xy == 0x1011) {
			enter(LOC_DATA | 9, ev);
			r = 1;
		}
		if (get_cell_a(ev->x, ev->y) == 3)
			swim_check();
	} else if (type == 1) {
		loc = 0;
		switch (xy) {
		case 0x1008:
			loc = LOC_DATA | 1;
			break;
		case 0xe10:
			loc = LOC_DATA | 2;
			break;
		case 0xa09:
			loc = LOC_DATA | 3;
			break;
		case 0x1009:
			loc = LOC_DATA | 4;
			break;
		case 0x140b:
			loc = LOC_DATA | 5;
			break;
		case 0xc0d:
			loc = LOC_DATA | 6;
			break;
		case 0x50d:
			loc = LOC_DATA | 7;
			break;
		case 0x1011:
			loc = LOC_DATA | 9;
			break;
		default:
			enter(LOC_HOUSE, ev);
			r = 1;
			break;
		}
		if (loc) {
			enter(loc, ev);
			r = 1;
		}
	} else if (type == 100) {
		switch (xy) {
		case 0xc07:
			k = 1;
			break;
		case 0xe07:
			k = 2;
			break;
#if FIX_0011
		/* Fix #11: k fuer unbekannte Felder vorbelegen */
		case 0xd09:
			k = 3;
			break;
		default:
			k = 1;
			break;
		}
		loc_load_data(7);
#else
		case 0xd09:
			k = 3;
			break;
		}
		loc_load_data(7);
#endif
		market_stall(k);
		enter(LOC_MERCHANT, ev);
	}
	return r;
}

void Town::market_stall(register signed short k)
{
	g_group->set_loctype(LOC_NONE);
	g_market_buy = g_market_sell = g_markets[g_loc_id].markup;
	g_market_lvl = g_markets[g_loc_id].lvl;
	g_market_type = k;
	if (k == 1)
		g_market_female = g_markets[g_loc_id].female[0];
	else if (k == 2)
		g_market_female = g_markets[g_loc_id].female[1];
	else
		g_market_female = g_markets[g_loc_id].female[2];
	g_loc_id = 0x18;
}

void Town::ship_voyage()
{
	Bmp far *b;
	signed short old;
	signed short w;
	void far *t_buf;
	char far * far *t;

	old = dlg_set16(g_dialog, 6);
	get_palette(g_pal_buf, 0, 0x100);
	txt_load_list(g_font, 0x219, 0x0a, &t_buf, &t);
	b = new Bmp(0x40, 0x40);
	NVF_HEADS->load(0x2e5, b->data(), 0, 0, 2, 0, &w, &w, 0x140);
	dlg_box_bmp(g_dialog, b, t[0], t[1], 0);
	dlg_box_bmp(g_dialog, b, t[0], t[2], 0);
	mem_free(t_buf);
	mem_free(t);
	delete b;
	pal_fade_out();
	swap_out_screen();
	txt_load_list(g_font, 0x219, 0x0a, &t_buf, &t);
	snd_obj_stop(g_snd_music);
	Outro1 a1;
	snd_play(g_snd_music, 0x27);
	mouse_hide(0);
	a1.run();
	mouse_show(0);
	pal_fade_in(g_pal_buf);
	Sound_volume2(g_snd_music, g_vol_music, 0);
	b = new Bmp(0x40, 0x40);
	NVF_HEADS->load(0x2e5, b->data(), 0, 0, 2, 0, &w, &w, 0x140);
	dlg_box_bmp(g_dialog, b, t[0], t[3], 0);
	NVF_HEADS->load(0x2e5, b->data(), 0, 0, 3, 0, &w, &w, 0x140);
	dlg_box_bmp(g_dialog, b, (char far *)0, t[6], 0);
	delete b;
	pal_fade_out();
	Outro2 a2;
	mouse_hide(0);
	a2.run();
	mouse_show(0);
	pal_fade_in(g_pal_buf);
	Sound_volume2(g_snd_music, g_vol_music, 0);
	b = new Bmp(0x40, 0x40);
	NVF_HEADS->load(0x2e5, b->data(), 0, 0, 3, 0, &w, &w, 0x140);
	dlg_box_bmp(g_dialog, b, t[5], t[7], 0);
	diary_add(g_diary, t[4], 0);
	diary_add(g_diary, t[8], 0);
	g_voyage_done = g_topic_ingramosch = g_topic_salamander_stone = g_topic_binge = g_elsurion_met = g_topic_egelbronn = g_vindaria_state = 1;
	mem_free(t_buf);
	mem_free(t);
	delete b;
	dlg_set16(g_dialog, old);
}

void Town::show_anim3()
{
	pal_fade_out();
	anim_free_all(g_screen_pic);
	Anim3 a;
	a.run();
	diary_add(g_diary, txt[19], 0);
	g_topic_sternenschweif = 1;
}

void Town::swim_check()
{
	signed short le;
	struct hero far *h;
	register signed short dmg;

	if (count_failed_skill(g_group, TA_SCHWIMMEN, 0)) {
		h = get_worst_hero_skill(g_group, TA_SCHWIMMEN);
		if (!h->see_und_fluss) {
			dmg = random_schweif(20);
			le = h->le;
			if (le - dmg > 0)
				sprintf(g_text_output_buf, g_ttx[443], h->name);
			else
				sprintf(g_text_output_buf, g_ttx[442], h->name, grammar_pronoun(h->sex, 1, 1),
					grammar_pronoun(h->sex, 2, 0), grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			g_in_fight = 1;
			sub_hero_le(h, dmg);
			g_in_fight = 0;
			g_hero_died = 1;
		}
	}
}
