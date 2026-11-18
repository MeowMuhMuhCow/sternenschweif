/* OVL179 - Overlay O179 (Segment 0x22db in SCHWEIF.EXE): Herberge LOC_INN, Teil 2 - Menuepunkte und Ereignisse zu
 * class Inn (O178; Texte INN.LTX = Datei 0x1a in txt, Merker done).
 * talk(): "Unterhalten" - Gespraech mit dem Wirt je Herberge g_loc_id (Gespraech 1 = KVIR02.TLK, 2..3 -> 0x0a/0x0b
 *   GASH04/05, 4..12 -> 0x25.. LOW09..LOW17, 13..14 TIEF08/09, 15..16 TJOL05/06, 17..26 FRP22B..FRP32D); wurde es
 *   gefuehrt (conv_run), vergehen 30 Minuten.
 * apply_skill(): "Talent anwenden" (nur mit gemietetem Zimmer g_inn_room): Held waehlen (INN.LTX[8]), use_skill(h, 0,
 *   -1); -1 (beim Stehlen erwischt) -> Herberge verlassen und Hausverbot g_inn_closed[g_loc_id]; 10 Minuten.
 * apply_magic(): "Magie anwenden" (nur mit Zimmer): Magier waehlen ([25]); braut er gerade einen Trank -> [45], hat er
 *   in dieser Rast schon gezaubert (rest_magic) -> [26], sonst rest_magic = use_magic(h) und 30 Minuten; kein
 *   Zauberkundiger -> [27].
 * theft_morning(): Diebstahl in einer einfachen Herberge (O178 sleep, Ort 4..12): erst ein Proviantpaket eines Helden,
 *   sonst (W90 + 10) * 10 Heller Gruppengeld, sonst ein zufaelliger Rucksackplatz 15..31 eines zufaelligen Helden
 *   (nicht undropable) - Meldung [52].
 * theft_night(): naechtlicher Dieb - bemerkt ihn ein wacher, nicht betrunkener Held (20 %), Meldung [50] und 0; sonst
 *   je Rucksackplatz 10 % Verlust, nr_heroes * 2 Proviantpakete, (W180 + 20) * 10 Heller, Ergebnis 1 (O178 zeigt [51]).
 * inn_conv_callback(a, b): Rueckruf 24 der Gespraeche (g_conv_callbacks; Wirte KVIR02, GASH04/05, LOW09..14/17,
 *   TIEF08/09, TJOL05/06, FRP24B/27B/28B/30B/31B): a == 1 nach der Antwort b - 1 Zimmer (inn_room), 2 Essen
 *   (inn_eat), 3 Ort 5: conv_cb_inn5, Ort 8..9: conv_cb_frp22b, 0x60 Thema 0x31 freischalten (g_artherion_heard = g_topic_artherion = 1,
 *   wenn g_artherion_hut_known == 0), 0x63 Gespraech beenden (conv_finish); Ergebnis -1 = Gespraech neu zeichnen.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL179.CPP   -> 1838/1838 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Og noetig: Zwischenwert [bp-10h] = Offset von h->name (zweimal im sprintf), Epilogkopien in theft_night/
 *   inn_conv_callback.
 * - Bitfeld rest_magic (+0xba.0): Schreiben "h->rest_magic = c" (signed char c) erzeugt cbw mit "unsigned short :1";
 *   Lesen ergibt "or al,al" erst mit "(unsigned char)h->rest_magic". brewing (+0xbc.3) ebenso als (unsigned char).
 * - "h->type >= 7" als inline-Funktion is_mage() -> mov ax,1 / xor ax,ax / or ax,ax; drunk per inline-Getter
 *   get_drunk() (mov al/cbw/or ax,ax); "!g_artherion_hut_known" ergibt mov al/cbw/or ax,ax, "g_artherion_hut_known == 0" dagegen cmp byte.
 * - In theft_night keine eingebettete Zuweisung (id = ...) im &&, sondern eigener Block; i im inneren Block der
 *   for-Schleife deklariert -> [bp-12h] hinter dem -Og-Zwischenwert.
 * - Die Methoden werden von O178 als freie Funktionen inn_talk(this) usw. gerufen (gleicher far call).
 * Auffaellig (Original): die zweite do-while-Schleife in theft_night laeuft weiter, solange get_first_hero() == h -
 *   also nur fuer den ersten Helden; talk() hat keinen Fall fuer Ort 0 und > 26 (kein Gespraech).
 */
#include <stdio.h>
#include "schweif.h"

void Inn::talk()
{
	register signed short id;

	id = -1;
	if (g_loc_id == 1)
		id = 1;
	else if (g_loc_id >= 2 && g_loc_id <= 3)
		id = g_loc_id + 8;
	else if (g_loc_id >= 4 && g_loc_id <= 12)
		id = g_loc_id + 0x21;
	else if (g_loc_id >= 13 && g_loc_id <= 14)
		id = g_loc_id + 0x45;
	else if (g_loc_id >= 15 && g_loc_id <= 16)
		id = g_loc_id + 0x53;
	else if (g_loc_id == 17)
		id = 0x6f;
	else if (g_loc_id == 18)
		id = 0x70;
	else if (g_loc_id == 19)
		id = 0x72;
	else if (g_loc_id == 20)
		id = 0x74;
	else if (g_loc_id == 21)
		id = 0x76;
	else if (g_loc_id == 22)
		id = 0x78;
	else if (g_loc_id == 23)
		id = 0x79;
	else if (g_loc_id == 24)
		id = 0x7b;
	else if (g_loc_id == 25)
		id = 0x7c;
	else if (g_loc_id == 26)
		id = 0x7f;
	if (id != -1 && conv_run(g_dialog2, id))
		timewarp_minutes(g_date, 30);
}

void Inn::apply_skill()
{
	struct hero far *h;

	if (g_inn_room) {
		/* INN.LTX[8]: "Welcher Held soll sein Talent anwenden?" */
		h = get_active_hero(g_group, txt[8]);
		if (h) {
			if (use_skill(h, 0, -1) == -1) {
				done = 1;
				g_inn_closed[g_loc_id] = 1;
			}
		}
		timewarp_minutes(g_date, 10);
	}
}

void Inn::apply_magic()
{
	struct hero far *h;
	signed char c;

	if (g_inn_room) {
		/* INN.LTX[25]: "Welcher Held soll Magie anwenden?" */
		h = get_active_mage(g_group, txt[25]);
		if (h) {
			if ((unsigned char)h->brewing) {
				/* INN.LTX[45]: "Waehrend eines alchimistischen Versuchs geht das nicht!" */
				dlg_message(g_dialog, txt[45]);
				return;
			}
			if (h->is_mage()) {
				if ((unsigned char)h->rest_magic)
					/* INN.LTX[26]: "Die letzte Magieanwendung hat schon mehr als genug an der Konstitution gezehrt." */
					dlg_message(g_dialog, txt[26]);
				else {
					c = use_magic(h);
					h->rest_magic = c;
					if ((unsigned char)h->rest_magic)
						timewarp_minutes(g_date, 30);
				}
				return;
			}
			/* INN.LTX[27]: "Diese Option ist ausschliesslich Magiekundigen vorbehalten." */
			dlg_message(g_dialog, txt[27]);
		}
	}
}

void Inn::theft_morning()
{
	signed long v;
	signed long m;
	struct hero far *h;
	register signed short i;
	register signed short id;

	i = group_hero_with_item(g_group, ITEM_FOOD_PACKAGE, -3);
	if (i != -1) {
		h = g_group->heroes[i];
		i = get_item_pos(h, ITEM_FOOD_PACKAGE);
		drop_item(h, i, 1);
		/* INN.LTX[52]: "Als Ihr morgens Eure Ausruestung ueberprueft, stellt Ihr fest, dass Ihr bestohlen worden seid." */
		dlg_message(g_dialog, txt[52]);
		return;
	}
	m = g_group->money;
	if (m) {
		v = (signed long)(random_schweif(90) + 10) * 10;
		group_sub_money(g_group, v);
		dlg_message(g_dialog, txt[52]);
		return;
	}
	h = get_random_hero2(g_group);
	i = random_interval(15, 31);
	id = h->inv[i].item_id;
	if (id && !g_items[id].undropable) {
		drop_item(h, i, 1);
		dlg_message(g_dialog, txt[52]);
	}
}

signed short Inn::theft_night()
{
	struct hero far *h;
	signed short noticed;
	signed long v;
	signed long m;
	register signed short k;
	register signed short id;

	noticed = 0;
	h = get_first_hero(g_group);
	do {
		if (check_hero3(h) && h->get_drunk() == 0 && random_schweif(100) <= 20) {
			noticed = 1;
			break;
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (!noticed) {
		h = get_first_hero(g_group);
		do {
			for (k = 15; k < 31; k++) {
				if (random_schweif(100) <= 10) {
					id = h->inv[k].item_id;
					if (id && !g_items[id].undropable)
						drop_item(h, k, 1);
				}
			}
			h = get_next_hero(g_group, h);
#if FIX_0004
		/* Fix #4: alle Helden durchgehen (!= wie in der ersten Schleife) */
		} while (get_first_hero(g_group) != h);
#else
		} while (get_first_hero(g_group) == h);
#endif
		for (k = 0; k < g_group->nr_heroes * 2; k++) {
			signed short i;

			i = group_hero_with_item(g_group, ITEM_FOOD_PACKAGE, -3);
			if (i != -1) {
				h = g_group->heroes[i];
				i = get_item_pos(h, ITEM_FOOD_PACKAGE);
				drop_item(h, i, 1);
			}
		}
		m = g_group->money;
		if (m) {
			v = (signed long)(random_schweif(180) + 20) * 10;
			group_sub_money(g_group, v);
		}
		return 1;
	} else {
		/* INN.LTX[50]: "%s wird nachts wach, als irgendjemand an Eurem Gepaeck herumfingert. Noch bevor %s sich
		 * aufrappeln kann, verschwindet die Person im Schutz der Dunkelheit aus dem Schlafsaal. Als Ihr die Ausruestung
		 * ueberprueft, stellt Ihr fest, dass nichts gestohlen wurde." */
		sprintf(g_text_output_buf, txt[50], h->name, h->name);
		dlg_message(g_dialog, g_text_output_buf);
		return 0;
	}
}

signed short inn_conv_callback(signed short a, signed short b)
{
	signed short r = 0;

	if (g_inn != 0 && a == 1) {
		if (b == 1) {
			inn_room(g_inn);
			return -1;
		} else if (b == 2) {
			inn_eat(g_inn);
			return -1;
		} else if (b == 3) {
			if (g_loc_id == 5)
				conv_cb_inn5(a, 1);
			if (g_loc_id >= 8 && g_loc_id <= 9)
				conv_cb_frp22b(a, 1);
		} else if (b == 0x60) {
			conv_unlock_topic(g_dialog2, TOPIC_ARTHERION);
			if (!g_artherion_hut_known)
				g_artherion_heard = g_topic_artherion = 1;
			r = -1;
		} else if (b == 0x63)
			conv_finish(g_dialog2);
	}
	return r;
}
