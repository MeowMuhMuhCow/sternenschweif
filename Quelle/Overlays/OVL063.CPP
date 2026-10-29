/* OVL063 - Overlay O063 (Segment 0x2072 in SCHWEIF.EXE): Ereignisse des Dungeons 12, Stollen der Binge mit Lorenbahn
 * (Texte BINGE4.LTX = Datei 0x457; Fortsetzung OVL064). Ereignisobjekt ev (struct area_events).
 * dng12_handler(ev, mev): Kartenereignis-Rueckruf (g_area_descs[12]); laeuft die Lorenfahrt (g_mine_cart_pos > 0),
 *   zuerst dng12_cart_ride. MAP_EVENT_ENTER: 0x707 dng12_latrine, 0x1211 dng12_tool_wall, 0xc0e/0xd0e bzw.
 *   0x90e/0xa0e -> g_dng12_pit_side = 12/10, 0xb07 dng12_rotten_beams, 0x107 einmalig Ablage 0x28; Typ 1 Tueren
 *   dng12_door; 0x64/0x65 Schutt/Einsturz (O064), 0x66/0x67 Ebenenwechsel nach 11/13, 0x68 binge_shaft (Kaminschacht),
 *   0x69 dng12_skeleton_fight, 0x6a Lorenbahn-Enden, 0x6b dng12_mine_cart, 0x6c dng12_forge, 0x6d Truhen 0x28..0x2a,
 *   0x6e..0x74 dng12_lever, dng12_pit, dng12_dwarf_grave, dng12_ore_heap, dng12_ore_chute, dng12_altar_inscription.
 * dng12_door: Schloss (Lock 0x90..0x9a je Position) mit Wandbild 0x3a/0x3b bei g_lock_cell_bits != -1.
 * dng12_latrine: Zwerg und Nichtzwerg in der Gruppe -> Bemerkung. dng12_skeleton_fight/dng12_skeleton: einmaliger
 *   Kampf 0x74 (event_fight1), danach Skelett mit Pergament (Tagebuch). dng12_mine_cart: Lore mit Wagenschmiere
 *   (Held g_dng12_cart_hero) schmieren, fahren -> dng12_cart_ride: Gruppe aufteilen (make_group, g_groups),
 *   Schrittschleife mit g_mine_cart_pos (Absprung bei 6/9/13: GE-Probe, Schaden, AP), Aufprall bei 15/16.
 * dng12_ore_chute: der Neugierigste (ATTRIB_NG) rutscht die Erzrutsche hinunter (eigene Gruppe).
 * dng12_ore_heap/dng12_tool_wall: Menues. dng12_forge: zufaellig Zangen-Musik (bester Musiker) bzw. im Abzug
 *   stecken bleiben (Neugierigster).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL063.CPP   -> 5118/5118 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (auch ohne -Og und mit -Oi gleich).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - dng12_door: automap_cell_write ueber AUTOMAP_CELL_WRITE_W (letzter Parameter short), sonst 15 Bytes mehr.
 * - C++ wegen Lock (O108) als Block-Local in dng12_door ([bp-2ch], Dtor Lock_dtor(&lk, 2) am Blockende) und new/
 *   delete der Klassen Chest (O078) / Stash (O238) (Ktor mit this = 0, Dtor (p, 3)).
 * - dng12_handler: "register" auf type UND pos noetig (type -> DI, pos -> SI); ohne register bekommt das zuerst
 *   deklarierte initialisierte ret DI.
 * - Gruppenschleife "for (i = 0, g = g_groups; i < 6; i++, g = &g_groups[i])" (far-Zeiger mit mov [bp-6],ds).
 * - "while (g_mine_cart_pos > (d = g_cam_cell_x - g_949a))" eingebettet (mov di,dx / cmp ax,dx); "get_first_hero(g)
 *   != h" ->
 *   cmp dx,[seg] / cmp ax,[off].
 * Originalfehler: dng12_cart_ride setzt g_mine_cart_pos auf 0 (alter Wert in t) und ruft danach test_attrib(..,
 *   g_mine_cart_pos / 3),
 *   sub_hero_le(.., g_mine_cart_pos / 3) und add_hero_ap(.., g_mine_cart_pos) mit der genullten Variable statt t auf
 *   (Probe ohne
 *   Zuschlag, kein Schaden, 0 AP). dng12_mine_cart liest g_group->heroes[i] schon vor der Pruefung i != -1 (bei -1
 *   das Wort
 *   vor dem Feld) nach g_dng12_cart_hero.
 */
#include <stdio.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:579a..579e, dsana) - initialisierte Daten in Adressreihenfolge */
struct hero far *g_dng12_cart_hero = 0;

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert; mit signed char legt BCC
 * die Register anders) */
#define AUTOMAP_CELL_WRITE_W ((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

signed short dng12_door(struct area_events far *ev, signed short pos, signed short x, signed short y);
void dng12_latrine(struct area_events far *ev);
void dng12_skeleton_fight(struct area_events far *ev);
void dng12_skeleton(struct area_events far *ev);
void dng12_buffer_stop(struct area_events far *ev);
void dng12_track_end(struct area_events far *ev);
void dng12_mine_cart(struct area_events far *ev);
void dng12_cart_ride(struct area_events far *ev);
void dng12_ore_chute(struct area_events far *ev);
void dng12_ore_heap(struct area_events far *ev);
void dng12_tool_wall(struct area_events far *ev);
void dng12_forge(struct area_events far *ev);

signed short dng12_handler(struct area_events far *ev, struct map_event far *mev)
{
	register signed short type;
	register signed short pos;
	signed short ret = 0;
	Chest far *o;
	Stash far *q;

	type = mev->type;
	pos = (mev->y - g_map_x0) << 8;
	pos |= mev->x - g_map_y0;
	if (g_mine_cart_pos > 0)
		dng12_cart_ride(ev);
	switch (type) {
	case MAP_EVENT_ENTER:
		switch (pos) {
		case 0x0707:
			dng12_latrine(ev);
			break;
		case 0x1211:
			dng12_tool_wall(ev);
			break;
		case 0x0c0e:
		case 0x0d0e:
			g_dng12_pit_side = 12;
			break;
		case 0x090e:
		case 0x0a0e:
			g_dng12_pit_side = 10;
			break;
		case 0x0b07:
			dng12_rotten_beams(ev);
			break;
		case 0x0107:
			if (!g_dng12_stash28_empty) {
				q = new Stash(0x28, 0);
				if (!q->has_items())
					g_dng12_stash28_empty = 1;
				delete q;
			}
			break;
		}
		break;
	case 1:
		ret = dng12_door(ev, pos, mev->x, mev->y);
		break;
	case 0x64:
		dng12_dig_rubble(ev, mev->y - g_map_x0, mev->x - g_map_y0);
		break;
	case 0x65:
		if (pos == 0x304 || pos == 0x306)
			dng12_clear_debris(ev);
		else
			dng12_dig_collapse(ev, mev->y - g_map_x0, mev->x - g_map_y0);
		break;
	case 0x66:
		group_set_screen_pos(0x0b, 0x0c);
		g_group->old_dungeon = g_group->dungeon;
		g_group->dungeon = 0x0b;
		g_group->dir = 1;
		break;
	case 0x67:
		group_set_screen_pos(1, 0x12);
		g_group->old_dungeon = g_group->dungeon;
		g_group->dungeon = 0x0d;
		g_group->dir = 3;
		break;
	case 0x68:
		binge_shaft(ev);
		break;
	case 0x69:
		dng12_skeleton_fight(ev);
		break;
	case 0x6a:
		if (pos == 0x10a)
			dng12_buffer_stop(ev);
		else if (pos == 0x100a)
			dng12_track_end(ev);
		break;
	case 0x6b:
		dng12_mine_cart(ev);
		break;
	case 0x6c:
		dng12_forge(ev);
		break;
	case 0x6d:
		switch (pos) {
		case 0x0e12:
			o = new Chest(0x28, 1);
			delete o;
			break;
		case 0x0c10:
			o = new Chest(0x29, 1);
			delete o;
			break;
		case 0x0c12:
			o = new Chest(0x2a, 1);
			delete o;
			break;
		}
		break;
	case 0x6e:
		dng12_lever(ev);
		break;
	case 0x6f:
		dng12_pit(ev);
		break;
	case 0x70:
		dng12_dwarf_grave(ev, mev->y - g_map_x0, mev->x - g_map_y0, 1);
		break;
	case 0x71:
		dng12_ore_heap(ev);
		break;
	case 0x72:
		dng12_ore_chute(ev);
		break;
	case 0x73:
		dng12_dwarf_grave(ev, mev->y - g_map_x0, mev->x - g_map_y0, 0);
		break;
	case 0x74:
		dng12_altar_inscription(ev);
		break;
	}
	return ret;
}

signed short dng12_door(struct area_events far *ev, signed short pos, signed short x, signed short y)
{
	signed short ret = 0;
	signed short nr = -1;
	signed short pic = 0x3a;

	switch (pos) {
	case 0x120a:
		nr = 0x90;
		pic = 0x3b;
		break;
	case 0x100f:
		nr = 0x91;
		pic = 0x3b;
		break;
	case 0x0e0f:
		nr = 0x92;
		pic = 0x3b;
		break;
	case 0x0d0e:
		nr = 0x93;
		break;
	case 0x090e:
		nr = 0x94;
		break;
	case 0x0911:
		nr = 0x95;
		break;
	case 0x080d:
		nr = 0x96;
		pic = 0x3b;
		break;
	case 0x0710:
		nr = 0x97;
		break;
	case 0x0712:
		nr = 0x98;
		break;
	case 0x030d:
		nr = 0x99;
		break;
	case 0x020f:
		nr = 0x9a;
		pic = 0x3b;
		break;
	default:
		break;
	}
	if (nr != -1) {
		Lock lk(nr);
		ret = 1;
		if (g_lock_cell_bits != -1) {
			event_set_cell_bits(ev, x, y, g_lock_cell_bits);
			snd_play(g_snd_fx, 0x61);
			AUTOMAP_CELL_WRITE_W(g_map, 0x306, y - g_map_x0, x - g_map_y0, pic);
		}
	}
	return ret;
}

void dng12_latrine(struct area_events far *ev)
{
	signed short old;
	struct hero far *a;
	struct hero far *b;
	struct hero far *h;

	old = dlg_set16(g_dialog, 5);
	/* BINGE4.LTX[3]: "Dieser Raum diente offensichtlich als Latrine, was einige Loecher im Boden bezeugen. Es liegt
	 * ein unangenehmer Geruch in der Luft, obwohl die Oertlichkeit sicher schon jahrzehntelang nicht mehr benutzt
	 * wurde." */
	dlg_message(g_dialog, ev->txt[3]);
	if (group_has_dwarf(g_group) == 1) {
		a = 0;
		b = 0;
		h = get_first_hero(g_group);
		do {
			if (a == 0 && h->type == HERO_TYPE_DWARF && check_hero3(h))
				a = h;
			if (b == 0 && h->type != HERO_TYPE_DWARF && check_hero3(h))
				b = h;
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
		if (a != 0 && b != 0) {
			/* BINGE4.LTX[4]: "%s erntet einen wuetenden Blick von %s, als %s eine launige Bemerkung ueber Zwerge und deren
			 * Duefte macht." */
			sprintf(g_text_output_buf, ev->txt[4], b->name, a->name, grammar_pronoun(b->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
		}
	}
	dlg_set16(g_dialog, old);
}

void dng12_skeleton_fight(struct area_events far *ev)
{
	if (!g_dng12_skeleton_found && !event_fight1(ev, 0x74, 1, 0)) {
		dng12_skeleton((struct area_events far *)g_area_events);
		g_dng12_skeleton_found = 1;
	}
}

void dng12_skeleton(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;

	old = dlg_set16(g_dialog, 5);
	h = get_first_active_hero(g_group);
	/* BINGE4.LTX[35]: "Bei der Untersuchung des Skeletts findet %s ein kleines Lederbeutelchen, darin stecken
	 * vergilbte und vermoderte Pergamente, die frueher vielleicht zu einem Buch gehoert haben moegen. Viel ist nicht
	 * mehr zu erkennen. Ein paar Fetzen sind geblieben, zudem ist das, was noch lesbar ist, in altaventurisch
	 * verfasst." */
	sprintf(g_text_output_buf, ev->txt[35], h->name, grammar_pronoun(h->sex, 0, 0));
	dlg_message(g_dialog, g_text_output_buf);
	/* BINGE4.LTX[36]: "<... steinerne Muenze haben, stehen wir vor einem weiteren Raetsel...@Eingang zur Schmiede des
	 * Ingerimm...@gefangen... wie sollen wir das Portal oeffnen...@an allem ist Madeviks Frevel Schuld...@ist Madevik
	 * wohl zur Strafe unter dem Schutt begraben worden...>" */
	dlg_message(g_dialog, ev->txt[36]);
	/* BINGE4.LTX[37]: "Nachdem %s diese Worte muehevoll entziffert hat, zerbroeseln auch noch die letzten
	 * Pergamentfetzen." */
	sprintf(g_text_output_buf, ev->txt[37], h->name, grammar_pronoun(h->sex, 0, 0));
	dlg_message(g_dialog, g_text_output_buf);
	/* BINGE4.LTX[38]: "Wir fanden in der Binge ein Skelett, das einige Pergamente bei sich hatte. Der Text war leider
	 * nur noch teilweise erhalten:@%s.@Es scheint nicht ratsam zu sein, den Schmiedegott in seinem eigenen Reich zu
	 * erzuernen!" */
	sprintf(g_text_output_buf, ev->txt[38], ev->txt[36]);
	diary_add(g_diary, g_text_output_buf, 0);
	dlg_set16(g_dialog, old);
}

void dng12_buffer_stop(struct area_events far *ev)
{
	signed short old;

	old = dlg_set16(g_dialog, 5);
	/* BINGE4.LTX[5]: "Eine verrottete Holzkonstruktion ist am Ende der Lorenbahn auf die Schienen montiert. Der Form
	 * nach koennte es mal ein Prellbock gewesen sein." */
	dlg_message(g_dialog, ev->txt[5]);
	dlg_set16(g_dialog, old);
}

void dng12_track_end(struct area_events far *ev)
{
	signed short old;

	old = dlg_set16(g_dialog, 5);
	if (g_mine_cart_pos != 0)
		/* BINGE4.LTX[6]: "Wo die Lorenbahn vor der Wand endet, liegen etliche verrottete Holztruemmer verstreut auf
		 * Schienen und Boden." */
		dlg_message(g_dialog, ev->txt[6]);
	else
		/* BINGE4.LTX[7]: "Eine verrottete Holzkonstruktion ist am Ende der Lorenbahn auf die Schienen montiert. Vor ein
		 * paar tausend Jahren haette man damit wahrscheinlich noch einen Wagen abbremsen koennen." */
		dlg_message(g_dialog, ev->txt[7]);
	dlg_set16(g_dialog, old);
}

void dng12_mine_cart(struct area_events far *ev)
{
	signed short old;
	signed short i;

	old = dlg_set16(g_dialog, 5);
	i = group_find_item(g_group, ITEM_AXLE_GREASE);
#if FIX_0115
	/* Fix #115: nicht vor das Heldenfeld lesen */
	g_dng12_cart_hero = i != -1 ? g_group->heroes[i] : (struct hero far *)0;
#else
	g_dng12_cart_hero = g_group->heroes[i];
#endif
	if (!g_dng12_cart_seen) {
		g_dng12_cart_seen = 1;
		/* BINGE4.LTX[8]: "Die Lore ist zur Haelfte mit Holzkohle gefuellt. Zwar ist der Aufbau stark verrottet und die
		 * mechanischen Teile der Lore sind ziemlich verrostet, dennoch mit ein wenig Schmiere koennte man das Gefaehrt
		 * betriebsbereit machen." */
		dlg_message(g_dialog, ev->txt[8]);
	}
	/* BINGE4.LTX[9]: "Wollt Ihr die Lore schmieren?" */
	if (i != -1 && dlg_yes_no(g_dialog, ev->txt[9]) == 1) {
		if (!check_hero3(g_dng12_cart_hero))
			g_dng12_cart_hero = get_first_active_hero(g_group);
		/* BINGE4.LTX[10]: "%s schmiert die Lore und tatsaechlich laesst sie sich mit ein wenig Anstrengung auf den
		 * Schienen hin- und herbewegen, bei jedem Versuch geht es ein wenig besser." */
		sprintf(g_text_output_buf, ev->txt[10], g_dng12_cart_hero->name);
		dlg_message(g_dialog, g_text_output_buf);
		/* BINGE4.LTX[11]: "Wollt Ihr mit der Lore fahren?" */
		if (dlg_yes_no(g_dialog, ev->txt[11]) == 1) {
			group_drop_item(g_group, ITEM_AXLE_GREASE);
			/* BINGE4.LTX[12]: "%s steigt in die Lore, die uebrigen schieben das Gefaehrt an. Die Raeder der Lore knirschen
			 * und kratzen auf den Schienen. Zunaechst ist der Wagen noch stoerrisch, doch dann geht der Beilunker Reiter
			 * ab..." */
			sprintf(g_text_output_buf, ev->txt[12], g_dng12_cart_hero->name);
			dlg_message(g_dialog, g_text_output_buf);
			dng12_cart_ride(ev);
		}
	}
	dlg_set16(g_dialog, old);
}

void dng12_cart_ride(struct area_events far *ev)
{
	signed short save = g_free_move;
	signed short flag = 0;
	struct group far *g;
	signed short t;
	signed short i;
	signed short d;

	if ((signed short)g_group->nr_heroes > 1) {
		diary_unlock(g_diary);
		make_group(1, g_dng12_cart_hero);
		for (i = 0, g = g_groups; i < 6; i++, g = &g_groups[i]) {
			if (group_has_hero(g, g_dng12_cart_hero)) {
				g_group = g;
				g_group_no = i;
			}
		}
		diary_lock(g_diary);
		screen_redraw(g_screen);
	}
	g_mine_cart_pos = 2;
	event_set_cell(ev, 10, 2, 0x0e, 0, -1);
	automap_cell_write(g_map, 0x306, 2, 10, 0x2c);
	group_set_screen_pos(2, 10);
	g_group->dir = 1;
	g_cam_angle = 0x400;
	cam_set_from_group();
	view_draw();
	g_view_input_on = 0;
	g_free_move = 1;
	g_view_input_on = 1;
	do {
		if (g_mine_cart_pos == 6 || g_mine_cart_pos == 9 || g_mine_cart_pos == 13) {
			t = g_mine_cart_pos;
			g_mine_cart_pos = 0;
			/* BINGE4.LTX[13]: "Die Lore wird immer schneller. Soll %s abspringen?" */
			sprintf(g_text_output_buf, ev->txt[13], g_dng12_cart_hero->name);
			if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
#if FIX_0114
				/* Fix #114: Probe mit der gesicherten Fahrtstrecke t */
				if (test_attrib(g_dng12_cart_hero, ATTRIB_GE, t / 3) > 0)
#else
				if (test_attrib(g_dng12_cart_hero, ATTRIB_GE, g_mine_cart_pos / 3) > 0)
#endif
					/* BINGE4.LTX[14]: "Ein eleganter Sprung, der nicht alle Tage gelingt!" */
					dlg_message(g_dialog, ev->txt[14]);
				else {
					/* BINGE4.LTX[15]: "Das war nichts! Aus dem Sprung wird eine Serie von schmerzhaften Purzelbaeumen." */
					dlg_message(g_dialog, ev->txt[15]);
#if FIX_0114
					/* Fix #114: Schaden aus der gesicherten Fahrtstrecke t */
					sub_hero_le(g_dng12_cart_hero, t / 3);
#else
					sub_hero_le(g_dng12_cart_hero, g_mine_cart_pos / 3);
#endif
				}
#if FIX_0114
				/* Fix #114: AP aus der gesicherten Fahrtstrecke t */
				add_hero_ap(g_dng12_cart_hero, t);
#else
				add_hero_ap(g_dng12_cart_hero, g_mine_cart_pos);
#endif
				flag = 1;
			}
			g_mine_cart_pos = t;
		}
		g_mine_cart_pos++;
		if (flag == 0) {
			do {
				poll_input();
				view_draw();
			} while (g_mine_cart_pos > (d = g_cam_cell_x - g_map_x0));
			group_set_screen_pos(d, g_group->y);
		}
		if (g_mine_cart_pos == 15) {
			g_cam_pitch = 30;
			view_draw();
			event_set_cell(ev, 10, 0x10, 0x0e, 0, -1);
		} else if (flag) {
			g_mine_cart_pos = -1;
			g_step_active = 0;
			g_step_turn = 0;
			g_step_pending = 0;
			g_cam_speed = 0;
			/* BINGE4.LTX[16]: "Mit lautem Krachen zerschellt die morsche Lore an einer Holzkonstruktion vor der Wand." */
			dlg_message(g_dialog, ev->txt[16]);
			group_set_screen_pos(g_group->x, 9);
			cam_set_from_group();
		} else if (g_mine_cart_pos == 16) {
			t = g_dng12_cart_hero->le - 4;
			g_mine_cart_pos = -1;
			dlg_message(g_dialog, ev->txt[16]);
			group_set_screen_pos(0x10, 9);
			/* BINGE4.LTX[17]: "%s geht es nur unwesentlich besser. Die Bekanntschaft mit der Wand faellt recht heftig aus.
			 * Schliesslich landet %s in irgendeiner staubigen Ecke, wo %s liegen bleibt." */
			sprintf(g_text_output_buf, ev->txt[17], g_dng12_cart_hero->name, grammar_pronoun(g_dng12_cart_hero->sex, 0, 0),
				grammar_pronoun(g_dng12_cart_hero->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			if (t > 0) {
				sub_hero_le(g_dng12_cart_hero, t);
				add_hero_ap(g_dng12_cart_hero, 30);
			}
			g_cam_pitch = 0;
			cam_set_from_group();
		}
	} while (g_mine_cart_pos != -1);
	g_view_input_on = 0;
	g_free_move = save;
	g_view_input_on = 1;
	event_set_cell(ev, 10, 0x10, 0x0c, 0, -1);
}

void dng12_ore_chute(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;
	struct group far *sg;
	struct group far *g;
	signed short i;
	signed short n;

	n = group_count_check3(g_group);
	if (group_get_npc(g_group))
		n--;
	if (n > 1) {
		old = dlg_set16(g_dialog, 5);
		h = get_hero_attrib_max(g_group, ATTRIB_NG);
		sg = g_group;
		/* BINGE4.LTX[18]: "Wollt Ihr die Erzrutsche genauer untersuchen?" */
		if (dlg_yes_no(g_dialog, ev->txt[18]) == 1) {
			/* BINGE4.LTX[19]: "%s fingert ein wenig ratlos an der Konstruktion herum, dann grinst %s ploetzlich und
			 * klettert das Geruest hoch. Ein froehlicher Jauchzer untermalt die Rutschpartie, die mit einem dumpfen
			 * Plumpsgeraeusch endet. Aus dem Nebenraum koennt ihr noch %s Lachen hoeren." */
			sprintf(g_text_output_buf, ev->txt[19], h->name, grammar_pronoun(h->sex, 0, 0), hero_name_gen(h, 1));
			dlg_message(g_dialog, g_text_output_buf);
			diary_unlock(g_diary);
			make_group(1, h);
			for (i = 0, g = g_groups; i < 6; i++, g = &g_groups[i]) {
				if (group_has_hero(g, h)) {
					g_group = g;
					g_group_no = i;
				}
			}
			group_set_screen_pos(0x0e, 0x0d);
			g_group = sg;
			g_group_no = get_group_index(sg);
			diary_lock(g_diary);
			screen_redraw(g_screen);
		}
		dlg_set16(g_dialog, old);
	}
}

void dng12_ore_heap(struct area_events far *ev)
{
	signed short old;
	signed short r;

	old = dlg_set16(g_dialog, 5);
	do {
		/* BINGE4.LTX[0]: "Wollt Ihr" */
		/* BINGE4.LTX[20]: "einen Erzklumpen nehmen?" */
		/* BINGE4.LTX[21]: "oder den Haufen durchsuchen?" */
		r = dlg_choose(g_dialog, ev->txt[0], 2, ev->txt[20], ev->txt[21]);
		if (r == 1) {
			if (group_give_item(g_group, ITEM_ORE_LUMP, 1))
				/* BINGE4.LTX[22]: "Nun ja - fraglich, ob Ihr das Zeug jemals brauchen werdet, aber was man hat, das hat man." */
				dlg_message(g_dialog, ev->txt[22]);
		} else if (r == 2) {
			timewarp_minutes(g_date, 10L);
			/* BINGE4.LTX[23]: "Das Erz ist feucht - eine unangenehme Wuehlerei und undankbar dazu, denn Ihr findet nichts." */
			dlg_message(g_dialog, ev->txt[23]);
		}
	} while (r != -1);
	dlg_set16(g_dialog, old);
}

void dng12_tool_wall(struct area_events far *ev)
{
	signed short old;
	signed short r;

	old = dlg_set16(g_dialog, 5);
	do {
		/* BINGE4.LTX[24]: "An der Wand sind mehrere Halterungen angebracht, an einigen haengen verrostete
		 * Schmiedewerkzeuge. In der Ecke liegen verrostete Eisengeraete herum. Die Geraete sind recht eigenartig, am
		 * auffaelligsten ist eine Art ueberdimensionaler Morgenstern, den man nicht zum Kaempfen verwenden koennte: an
		 * einer gut 25^Schritt langen Kette haengt eine Kugel mit duennen Eisenborsten.@Wollt Ihr" */
		/* BINGE4.LTX[25]: "die Werkzeuge untersuchen?" */
		/* BINGE4.LTX[26]: "oder den <Morgenstern> mitnehmen?" */
		r = dlg_choose(g_dialog, ev->txt[24], 2, ev->txt[25], ev->txt[26]);
		if (r == 1)
			/* BINGE4.LTX[27]: "Es handelt sich um Zangen verschiedenster Groesse. Die Werkzeuge sind aber so stark
			 * verrostet, dass sie voellig wertlos sind." */
			dlg_message(g_dialog, ev->txt[27]);
		else if (r == 2)
			/* BINGE4.LTX[28]: "Der Kehrbesen passt nicht in Eure Rucksaecke!" */
			dlg_message(g_dialog, ev->txt[28]);
	} while (r != -1);
	dlg_set16(g_dialog, old);
}

void dng12_forge(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;
	struct hero far *h2;

	old = dlg_set16(g_dialog, 5);
	h = get_first_active_hero(g_group);
	if (random_schweif(2) == 1) {
		h2 = get_best_hero_skill(g_group, TA_MUSIZIEREN);
		/* BINGE4.LTX[29]: "%s hat eine rostige Zange entdeckt und bearbeitet damit rhythmisch den Amboss." */
		sprintf(g_text_output_buf, ev->txt[29], h2->name);
		dlg_message(g_dialog, g_text_output_buf);
		timewarp_minutes(g_date, 5L);
		if ((signed short)g_group->nr_heroes > 1) {
			if (h2 == h)
				h = get_second_active_hero(g_group);
			/* BINGE4.LTX[30]: "Schliesslich wird es %s zu viel: wuetend reisst %s %s die Zange aus der Hand und schleudert
			 * sie in die Ecke." */
			sprintf(g_text_output_buf, ev->txt[30], h->name, grammar_pronoun(h->sex, 0, 0), h2->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
	} else {
		h = get_hero_attrib_max(g_group, ATTRIB_NG);
		/* BINGE4.LTX[31]: "%s untersucht zuerst die Esse, kriecht dann ein Stueck in den Abzug hinein und bleibt prompt
		 * darin stecken!" */
		sprintf(g_text_output_buf, ev->txt[31], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		if ((signed short)g_group->nr_heroes > 1) {
			timewarp_minutes(g_date, 5L);
			/* BINGE4.LTX[32]: "Mit vereinten Kraeften gelingt es Euch schliesslich, %s freizubekommen." */
			sprintf(g_text_output_buf, ev->txt[32], grammar_pronoun(h->sex, 2, 0));
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			timewarp_minutes(g_date, 30L);
			/* BINGE4.LTX[33]: "Es dauert geraume Zeit, bis sich %s wieder befreien kann." */
			sprintf(g_text_output_buf, ev->txt[33], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
	}
	dlg_set16(g_dialog, old);
}
