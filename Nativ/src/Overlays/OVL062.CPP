/* OVL062 - Overlay O062 (Segment 0x206c in SCHWEIF.EXE): Ereignisse des Dungeons 11 (Fortsetzung von O061, Texte
 * BINGE3.LTX ueber ev->txt): Einsturz unter dem Hebel, Grubenrampe, Koboldwohnung mit dem Kobold Mumpitz.
 * dng11_lever_floor(ev): Hebel umgelegt - der Boden gibt nach (txt[56], g_dng11_floor_collapsed), Felder 4 x 4 ab
 *   (3, 9) und zwei Einzelfelder per event_set_cell, Automap, alle Helden W2+1 Schaden, txt[57]: fuenf kleine
 *   Gestalten greifen an (Kampf 0xa8).
 * dng11_ramp_climb(ev): nach dem Einsturz (bis zu 4 Mal, g_dng11_ramp_falls) GE-Probe des Ungeschicktesten (+2 -
 *   Zahl der Stuerze): misslungen -> txt[58] zurueck in die Grube, W2+2 Schaden, bei mehreren Helden eigene Gruppe
 *   (make_group) eine Zeile zurueck; alte Gruppe wieder aktiv.
 * dng11_plate/dng11_wardrobe: einmalige Meldungen txt[25] (Bodenplatte) bzw. txt[26] (Kleiderschrank).
 * dng11_kobold_chest: Truhe des Kobolds - Rueckfrage txt[27] -> dng11_kobold (erstes Mal), danach Ablage Stash 0x27
 *   (bis das Raetsel geloest ist) bzw. txt[28] (leer).
 * dng11_kobold(ev): Gespraech mit Mumpitz von Zappendust-Zwackenpurtz (Bild 0x4b9, Bmp 64x64, dlg_box_bmp) in drei
 *   Fragerunden; Unhoeflichkeit zaehlt Punkte cnt (+1/+2): Angreifen -> dng11_kobold_soften_weapons (Gummischwert),
 *   Geschenk -> dng11_kobold_gift; dann Raetsel dng11_kobold_riddle und Abschied dng11_kobold_send_away(ev, cnt).
 * dng11_kobold_soften_weapons(ev): fuer alle Helden und 31 Plaetze mit Waffe (nicht stapelbar) hero::inc_inv_bf(i)
 *   (Bruchfaktor + 1 bis 20, -99 = unzerbrechlich bleibt).
 * dng11_kobold_gift(ev): Held und Gegenstand waehlen (get_any_hero/select_item), Geschenk drop_item; Wert (Preis x
 *   Einheit) > 500 -> 1.
 * dng11_kobold_riddle(ev): Raetsel (Krone ohne Geweih) per dlg_message_flag, Antwort strncmp mit g_dng11_password
 *   ("ELCH", 4 Zeichen), 3 Versuche; geloest -> Silberschluessel (ITEM_SILVER_KEY), g_dng11_riddle_solved.
 * dng11_kobold_send_away(ev, n): Gruppe einschlaefern und verteilen - n == 0: ganze Gruppe nach
 *   g_dng11_kobold_dest_split[0]; sonst Heldenliste (bis 7), n <= 2: make_group(cnt/2 + 1, ..) und die Gruppen auf
 *   dem
 *   eigenen Feld (Dungeon 11) nach g_dng11_kobold_dest_split (n == 1) bzw. g_dng11_kobold_dest_far (n == 2, Dungeons
 *   10/12); n > 2: alle Helden einzeln nach g_dng11_kobold_dest_single; zuletzt aktuelle Gruppe neu suchen
 *   (g_group/g_group_no), g_pp20_index zurueck.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL062.CPP   -> 4286/4286 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen inline-Elementfunktionen: hero::inc_inv_bf(i) (Parameterkopie [bp-8], "(x == -99 ? 0 : 1) && x < 20"
 *   -> mov al,0 / mov al,1 / or al,al, toter jmp short ans Inline-Ende, Offset h+i*27 als -Og-Wert in DI) und
 *   group::set_pos_x/set_pos_y(v, sx)/set_dungeon_v(v) in dng11_kobold_send_away: je Aufruf eigene Slots fuer v und
 *   sx = v * 0xa8 + 0x54; die Kopien liegen hier in Argumentreihenfolge (v zuerst bei [bp-14h], sx bei [bp-16h]).
 * - -Oi: strncmp(.., 4) als Intrinsic (repne scasb / repe cmpsb, daher push si/di in dng11_kobold_riddle).
 * - -Og: Flagbyte von g_items[id] als Zwischenwert [bp-9] (dng11_kobold_soften_weapons), Offset von g_items[id] in
 *   [bp-8] (dng11_kobold_gift), Tabellenoffset k*6 in SI (dng11_kobold_send_away); dort Schleifenzaehler i OHNE
 *   register (DI).
 * - "(signed short)g_group->nr_heroes > 1" -> mov al / cbw / cmp ax,1 (sonst cmp byte).
 * - Wertprodukt "(signed long)it.price * (signed long)(signed short)it.price_unit" -> movsx eax,word / cbw / movsx
 *   eax,ax.
 * Originalfehler: dng11_kobold_gift verlaesst die Auswahlschleife auch mit sel < -1 (h != 0) und greift dann auf
 *   h->inv[sel] zu (nur falls select_item andere negative Werte als -1 liefert). dng11_kobold_send_away zaehlt k ohne
 *   Grenze weiter: mehr passende Gruppen als Tabelleneintraege lesen in die Nachbartabelle
 *   (g_dng11_kobold_dest_single[1] = g_dng11_kobold_dest_split[0]).
 */
#include <stdio.h>
#include <string.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:575e..579a, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct dng_target g_dng11_kobold_dest_single[1] = { { 10, 1, 11, 0 } };
struct dng_target g_dng11_kobold_dest_split[2] = { { 18, 13, 11, 0 }, { 11, 6, 11, 0 } };
struct dng_target g_dng11_kobold_dest_far[3] = { { 9, 4, 10, 0 }, { 18, 5, 12, 0 }, { 2, 1, 12, 0 } };

void dng11_lever_floor(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;
	register signed short y;
	register signed short x;

	old = dlg_set16(g_dialog, 5);
	h = get_first_hero(g_group);
	/* BINGE3.LTX[56]: "Kaum habt Ihr den Hebel umgelegt, gibt ploetzlich der Boden unter Euch nach!" */
	dlg_message(g_dialog, ev->txt[56]);
	g_dng11_floor_collapsed = 1;
	for (x = 0; x < 4; x++)
		for (y = 0; y < 4; y++)
			event_set_cell(ev, x + 3, y + 9, 0x0f, 0, -1);
	event_set_cell(ev, 7, 0x0b, 0x12, 0, -1);
	event_set_cell(ev, 6, 9, 0x11, 0, -1);
	automap_cell_write(g_map, 0x302, 9, 3, 0x29);
	for (y = 0; y < 3; y++)
		automap_cell_write(g_map, 0x302, y + 10, 3, 0x28);
	for (x = 0; x < 2; x++)
		automap_cell_write(g_map, 0x302, 9, x + 4, 0x27);
	for (x = 0; x < 3; x++)
		for (y = 0; y < 3; y++)
			automap_cell_write(g_map, 0x302, y + 10, x + 4, 0x26);
	automap_cell_write(g_map, 0x302, 9, 6, 0x2c);
	automap_cell_write(g_map, 0x302, 0x0b, 7, 0x2a);
	view_frame();
	do {
		sub_hero_le(h, random_schweif(2) + 1);
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	screen_redraw(g_screen);
	/* BINGE3.LTX[57]: "Ploetzlich seht Ihr Euch fuenf kleinen Gestalten in zerschlissenen Lederruestungen gegenueber,
	 * die Euch ohne Warnung angreifen." */
	dlg_message(g_dialog, ev->txt[57]);
	start_fight(0xa8, 0);
	dlg_set16(g_dialog, old);
}

void dng11_ramp_climb(struct area_events far *ev)
{
	signed short old;
	signed short mod;
	struct hero far *h;
	struct group far *save;
	struct group far *g;
	register signed short i;

	if (g_dng11_floor_collapsed != 0 && g_dng11_ramp_falls < 4) {
		old = dlg_set16(g_dialog, 5);
		mod = 2 - g_dng11_ramp_falls;
		h = get_hero_attrib_min(g_group, ATTRIB_GE);
		save = g_group;
		if (test_attrib(h, ATTRIB_GE, mod) < 0) {
			g_dng11_ramp_falls++;
			diary_unlock(g_diary);
			/* BINGE3.LTX[58]: "%s klettert die Rampe hoch, verliert auf halber Hoehe aber das Gleichgewicht und landet
			 * unsanft wieder in der Grube." */
			sprintf(g_text_output_buf, ev->txt[58], h->name, grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			sub_hero_le(h, random_schweif(2) + 2);
			if ((signed short)g_group->nr_heroes > 1) {
				make_group(1, h);
				for (i = 0, g = g_groups; i < 6; g = &g_groups[++i])
					if (group_has_hero(g, h)) {
						g_group = g;
						g_group_no = i;
					}
			}
			group_set_screen_pos(g_group->x, g_group->y - 1);
			g_group = save;
			g_group_no = get_group_index(save);
			cam_set_from_group();
			diary_lock(g_diary);
		}
		dlg_set16(g_dialog, old);
	}
}

void dng11_plate(struct area_events far *ev)
{
	signed short old;

	if (!g_dng11_plate_seen) {
		old = dlg_set16(g_dialog, 5);
		g_dng11_plate_seen = 1;
		/* BINGE3.LTX[25]: "Es passiert nichts, als Ihr die Platte betretet." */
		dlg_message(g_dialog, ev->txt[25]);
		dlg_set16(g_dialog, old);
	}
}

void dng11_wardrobe(struct area_events far *ev)
{
	signed short old;

	if (!g_dng11_wardrobe_seen) {
		old = dlg_set16(g_dialog, 5);
		g_dng11_wardrobe_seen = 1;
		/* BINGE3.LTX[26]: "Der Schrank ist nur eineinhalb Schritt hoch, die Tueren sind nicht verschlossen. Der Schrank
		 * hat mehrere Faecher, die mit Kleidung vollgestopft sind. An einer Stange haengt eine Ledergarnitur, ausserdem
		 * mehrere Roben und Umhaenge in den verschiedensten Farben. Mit den Kleidungsstuecken koennt Ihr nichts
		 * anfangen, denn sie sind Euch mehrere Nummern zu klein." */
		dlg_message(g_dialog, ev->txt[26]);
		dlg_set16(g_dialog, old);
	}
}

void dng11_kobold_chest(struct area_events far *ev)
{
	signed short old;
	void far *st;

	old = dlg_set16(g_dialog, 5);
	if (!g_dng11_kobold_met) {
		/* BINGE3.LTX[27]: "In der Truhe liegen mehrere Phiolen aus Glas, Schmuckstuecke, Schluessel, Muenzen aus Marmor
		 * und eine ganze Reihe von seltsam anmutenden Gegenstaenden, mit denen Ihr nichts rechtes anzufangen wisst.
		 * Wollt Ihr etwas mitnehmen?" */
		if (dlg_yes_no(g_dialog, ev->txt[27]) == 1)
			dng11_kobold(ev);
	} else if (!g_dng11_riddle_solved) {
		st = Stash_ctor(0, 0x27, 0);
		Stash_dtor(st, 3);
	} else
		/* BINGE3.LTX[28]: "Die Truhe ist leer." */
		dlg_message(g_dialog, ev->txt[28]);
	dlg_set16(g_dialog, old);
}

void dng11_kobold(struct area_events far *ev)
{
	Bmp far *b;
	signed short n;
	signed short dummy;
	signed short old;
	signed short attacked;
	register signed short cnt;
	register signed short sel;

	old = dlg_set16(g_dialog, 6);
	cnt = 0;
	attacked = 0;
	b = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x4b9, b->data(), 0, 0, 0, &dummy, &dummy, 0x140);
	/* BINGE3.LTX[29]: "<Aha, an fremdem Eigentum vergreifen, wenn der Besitzer gerade nicht zuhause ist, solche seid
	 * Ihr...>, ertoent eine piepsige Stimme hinter Euch.@Als Ihr Euch erschrocken umwendet, steht eine kleine Gestalt
	 * mit faltigem, eingekniffenem Gesicht in der Ecke des Raumes." */
	dlg_box_bmp(g_dialog, b, (char far *)0, ev->txt[29], 0);
	/* BINGE3.LTX[30]: "<Nun ja, man muss die Besucher nehmen, wie sie kommen, kann ich mir ja nicht aussuchen, in
	 * letzter Zeit ist Besuch seltener geworden.>" */
	/* BINGE3.LTX[0]: "Wollt Ihr" */
	sprintf(g_text_output_buf, "%s@%s", ev->txt[30], ev->txt[0]);
	/* BINGE3.LTX[31]: "den Kobold weitersprechen lassen?" */
	/* BINGE3.LTX[32]: "dazwischenreden?" */
	/* BINGE3.LTX[33]: "den Kobold angreifen?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 3, ev->txt[31], ev->txt[32], ev->txt[33]);
	if (sel == 2)
		/* BINGE3.LTX[34]: "<Iiiich rede, Ihr seid nur meine Gaeste!>" */
		dlg_box_bmp(g_dialog, b, (char far *)0, ev->txt[34], 0);
	else if (sel == 3) {
		/* BINGE3.LTX[35]: "Als Ihr zu Euren Waffen greifen wollt merkt Ihr, dass sie weich wie Kautschuk sind. <Hihiii,
		 * gut nicht?! Gumgumgummischwert - Dass Ihr Euch wie Ruepel benehmt, war ja zu erwarten...>" */
		dlg_box_bmp(g_dialog, b, (char far *)0, ev->txt[35], 0);
		dng11_kobold_soften_weapons(ev);
		attacked = 1;
		cnt += 2;
	}
	if (attacked == 0)
		n = 3;
	else
		n = 2;
	/* BINGE3.LTX[36]: "<Tja, ich erwarte Gastgeschenke, wie das nun mal so ueblich ist...>" */
	sprintf(g_text_output_buf, "%s@%s", ev->txt[36], ev->txt[0]);
	/* BINGE3.LTX[37]: "abwarten?" */
	/* BINGE3.LTX[38]: "ein Geschenk rausruecken?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, n, ev->txt[37], ev->txt[38], ev->txt[33]);
	if (sel == 1) {
		/* BINGE3.LTX[39]: "<Hmm, gut, dann eben nicht, hatte aber schon manierlichere Gaeste>, meint der Kobold in
		 * beleidigtem Tonfall, dabei zuckt er wild mit den Schultern." */
		dlg_box_bmp(g_dialog, b, (char far *)0, ev->txt[39], 0);
		cnt++;
	} else if (sel == 2) {
		if (dng11_kobold_gift(ev) == 0)
			cnt++;
	} else if (sel == 3) {
		dlg_box_bmp(g_dialog, b, (char far *)0, ev->txt[35], 0);
		dng11_kobold_soften_weapons(ev);
		cnt += 2;
	}
	/* BINGE3.LTX[43]: "<Also Ihr ungehobelten Gesellen, wenn Ihr Euch nicht vorstellen wollt, dann fange ich eben an,
	 * Mumpitz von Zappendust-Zwackenpurtz, meine Freunde nennen mich Mumpi - aber kommt Ihr bloss nicht auf die Idee,
	 * mich Mumpi zu nennen!>" */
	sprintf(g_text_output_buf, "%s@%s", ev->txt[43], ev->txt[0]);
	/* BINGE3.LTX[44]: "Euch vorstellen?" */
	/* BINGE3.LTX[45]: "laut loslachen?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 3, ev->txt[44], ev->txt[37], ev->txt[45]);
	if (sel == 1)
		/* BINGE3.LTX[46]: "<Komische Namen, naja, Ihr muesst ja damit zurechtkommen.>" */
		dlg_box_bmp(g_dialog, b, (char far *)0, ev->txt[46], 0);
	else if (sel == 2) {
		/* BINGE3.LTX[47]: "<Namenlose Findelkinder, tzz, tzz, tzz...>" */
		dlg_box_bmp(g_dialog, b, (char far *)0, ev->txt[47], 0);
		cnt++;
	} else if (sel == 3) {
		/* BINGE3.LTX[48]: "<Nun habe ich meine Gaeste belustigt, und den Scherz selbst nicht verstanden, komisch, wie?!>
		 * meint das hutzelige Maennchen mit so scharfem Tonfall, wie es seine piepsige Stimme gerade eben zulaesst." */
		dlg_box_bmp(g_dialog, b, (char far *)0, ev->txt[48], 0);
		cnt += 2;
	}
	/* BINGE3.LTX[49]: "<Genug jetzt mit dem Geplaudere. Zwei Wege kenne ich, der eine ist unumgaenglich, der andere
	 * nuetzlich. Der unumgaengliche fuehrt aus diesem Raum, denn ich will meine Ruhe haben. Den nuetzlichen muesst
	 * Ihr Euch erarbeiten.>@Mumpi kramt ein wenig in der Truhe und bringt einen grossen reich verzierten Schluessel
	 * zum Vorschein. <Was die Leute so alles achtlos herumliegen lassen, tzz, tzz>, murmelt der Kobold vor sich hin." */
	dlg_box_bmp(g_dialog, b, (char far *)0, ev->txt[49], 0);
	dng11_kobold_riddle(ev);
	/* BINGE3.LTX[50]: "<Zeit zur Verabschiedung> meint der Kobold, und beginnt kreisende Bewegungen mit seinen Armen.
	 * Ihr werdet muede, und Euch fallen die Augen zu." */
	dlg_box_bmp(g_dialog, b, (char far *)0, ev->txt[50], 0);
	delete b;
	dng11_kobold_send_away(ev, cnt);
	/* BINGE3.LTX[51]: "Als Ihr erwacht und die Augen oeffnet, stellt Ihr fest, dass Ihr nicht mehr im Wohnraum des
	 * Koboldes seid." */
	dlg_message(g_dialog, ev->txt[51]);
	g_dng11_kobold_met = 1;
	dlg_set16(g_dialog, old);
}

void dng11_kobold_soften_weapons(struct area_events far *ev)
{
	signed short id;
	struct hero far *h;
	register signed short i;

	h = get_first_hero(g_group);
	do {
		for (i = 0; i < 31; i++) {
			id = h->inv[i].item_id;
			if (id != 0) {
				if (g_items[id].weapon && !g_items[id].stackable)
					h->inc_inv_bf(i);
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
}

signed short dng11_kobold_gift(struct area_events far *ev)
{
	signed short id;
	struct hero far *h;
	register signed short sel;
	register signed short ret;

	ret = 0;
	sel = -1;
	do {
		/* BINGE3.LTX[40]: "Wer soll dem Kobold etwas geben?" */
		h = get_any_hero(g_group, ev->txt[40]);
		if (h != 0) {
			/* BINGE3.LTX[41]: "Was soll %s dem Kobold geben?" */
			sprintf(g_text_output_buf, ev->txt[41], h->name);
			sel = select_item(h, g_text_output_buf, 1);
		}
#if FIX_0089
		/* Fix #89: jede negative Auswahl wiederholen (kein Zugriff auf h->inv[sel < 0]) */
		if (sel < 0)
				h = 0;
		} while (sel < 0 && h == 0);
#else
		if (sel == -1)
			h = 0;
	} while (sel < 0 && h == 0);
#endif
	id = h->inv[sel].item_id;
	if ((signed long)g_items[id].price * (signed long)(signed short)g_items[id].price_unit > 500)
		ret = 1;
	/* BINGE3.LTX[42]: "Mit einem lauten Puffen verschwindet der Kobold ploetzlich aus der Ecke und steht ebenso
	 * unerwartet mit aufgehaltener Hand vor %s. Das Geschenk nimmt er und wirft es achtlos in die offenstehende
	 * Truhe:@<Danke! Wie grosszuegig!> piepst der Kobold ironisch." */
	sprintf(g_text_output_buf, ev->txt[42], h->name);
	dlg_message(g_dialog, g_text_output_buf);
	drop_item(h, sel, 1);
	return ret;
}

void dng11_kobold_riddle(struct area_events far *ev)
{
	signed short tries;
	signed short ok;

	tries = 3;
	ok = 0;
	/* BINGE3.LTX[52]: "<Raetsel, Raetsel!> springt das Hutzelmaennchen ploetzlich wild durch den Raum, und fuchtelt
	 * mit dem Schluessel vor Euren Nasen herum." */
	dlg_message(g_dialog, ev->txt[52]);
	do {
		/* BINGE3.LTX[53]: "<Sie haben kein Geweih, aber dennoch tragen sie eine Krone.@Man kann darauf reiten, aber
		 * dennoch tut es keiner.@Na?...>" */
		dlg_message_flag(g_dialog, 5, ev->txt[53]);
		if (strncmp(g_input_buf, "ELCH", 4) == 0)
			ok = 1;
		tries--;
	} while (tries != 0 && ok == 0);
	if (ok != 0) {
		/* BINGE3.LTX[54]: "<Bravo! Haette sogar ein Ork loesen koennen.>" */
		dlg_message(g_dialog, ev->txt[54]);
		group_give_item(g_group, ITEM_SILVER_KEY, 1);
		g_dng11_riddle_solved = 1;
	} else
		/* BINGE3.LTX[55]: "<Elche, Ihr Esel.>" */
		dlg_message(g_dialog, ev->txt[55]);
}

void dng11_kobold_send_away(struct area_events far *ev, signed short n)
{
	signed short save;
	signed short k;
	signed short x;
	signed short y;
	struct hero far *h0;
	struct group far *g;
	signed short cnt;
	struct hero far *list[7];
	signed short i;

	save = g_pp20_index;
	k = 0;
	x = g_group->x;
	y = g_group->y;
	h0 = get_first_active_hero(g_group);
	g_pp20_index = -1;
	diary_unlock(g_diary);
	if (n == 0) {
		group_set_screen_pos(g_dng11_kobold_dest_split[0].x, g_dng11_kobold_dest_split[0].y);
	} else {
		cnt = g_group->nr_heroes;
		list[0] = get_first_hero(g_group);
		for (i = 1; i < cnt; i++)
			list[i] = get_next_hero(g_group, list[i - 1]);
		if (n <= 2) {
			if (cnt > 1)
				make_group(cnt / 2 + 1, list[0], list[1], list[2], list[3]);
			for (i = 0, g = g_groups; i < 6; g = &g_groups[++i])
				if (g->dungeon == 11 && g->x == x && g->y == y) {
					if (n == 1) {
						g->set_pos_x(g_dng11_kobold_dest_split[k].x, g_dng11_kobold_dest_split[k].x * 0xa8 + 0x54);
						g->set_pos_y(g_dng11_kobold_dest_split[k].y, g_dng11_kobold_dest_split[k].y * 0xa8 + 0x54);
						g->set_dungeon_v(g_dng11_kobold_dest_split[k].dungeon);
					} else {
						g->set_pos_x(g_dng11_kobold_dest_far[k].x, g_dng11_kobold_dest_far[k].x * 0xa8 + 0x54);
						g->set_pos_y(g_dng11_kobold_dest_far[k].y, g_dng11_kobold_dest_far[k].y * 0xa8 + 0x54);
						g->set_dungeon_v(g_dng11_kobold_dest_far[k].dungeon);
					}
					k++;
				}
		} else {
			if (cnt != 0)
				for (i = 0; i < cnt; i++)
					make_group(1, list[i]);
			for (i = 0, g = g_groups; i < 6; g = &g_groups[++i])
				if (g->dungeon == 11 && g->x == x && g->y == y) {
					g->set_pos_x(g_dng11_kobold_dest_single[k].x, g_dng11_kobold_dest_single[k].x * 0xa8 + 0x54);
					g->set_pos_y(g_dng11_kobold_dest_single[k].y, g_dng11_kobold_dest_single[k].y * 0xa8 + 0x54);
					g->set_dungeon_v(g_dng11_kobold_dest_single[k].dungeon);
					k++;
				}
		}
	}
	for (i = 0, g = g_groups; i < 6; g = &g_groups[++i])
		if (group_has_hero(g, h0)) {
			g_group = g;
			g_group_no = i;
		}
	g_pp20_index = save;
	diary_lock(g_diary);
	screen_redraw(g_screen);
	cam_set_from_group();
}
