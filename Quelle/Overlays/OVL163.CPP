/* OVL163 - Overlay O163 (Segment 0x2291 in SCHWEIF.EXE): Wegereignisse unterwegs (Reiseobjekt p = Travel mit Weg
 * p->from, Gelaende p->terrain, Abbruch p->action/ev_half/ev_start/ev_end; Texte g_tev_txt = ALL_SECT.LTX und
 * g_tev_loc_txt = Ortstexte des Weges, Bilder nvfile_load aus g_nvf_heads).
 * orc_camp_luggage (aus O017, Orklager vor Lowangen, POINT2.LTX[84..86]): die Orks nehmen alles Gepaeck ausser den
 *   magischen Gegenstaenden (group_drop_most_items); hat noch jemand einen magischen Gegenstand (item_desc.magic,
 *   group_find_item), tritt der Schamane dazu [85]; danach steht die Ankunft in Lowangen an (g_low_arrival_due = 1).
 * flag_not(p, c): c == 1 ? 0 : 1 (O215 tauscht damit Wegereignis-Merker). place_event_nop: leer (O122).
 * ford_crossing(p) (O114, ALL_SECT.LTX[23..29]): Furt durch den (Finsteren) Svellt - Weg 0x17/0x2c = Finsterer Svellt,
 *   vorher ggf. Wasserschlaeuche fuellen (group_refill_water, Zusatz [20]); durchqueren = GE-Probe aller Helden
 *   (group_test_attrib_disease, Misserfolg: unfreiwilliges Bad, Schaden, Wundfieber/Frost), sonst umkehren.
 * ferry_crossing(p, a, b) (O114, ALL_SECT.LTX[30..52], 133, 38/39): Faehre - Ort des Faehrmanns je Weg (0x27/0x28
 *   Finsterer Svellt, 0x0f hinter Tiefhusen, 0x1e hinter Hilvalla, 0xb9/0xc6 Svelltufer); Preis a (warten) bzw. b
 *   (sofort) Silbertaler je Person: sofort uebersetzen, umkehren oder warten (W6 + 4) / 2 Stunden - nur vor 18 Uhr
 *   (g_daytime + n * 5400 < 97200), sonst Nachtlager (Hunger/Durst 90 fuer alle, bis 8 Uhr), doch umkehren oder
 *   gleich bezahlen. Bei g_ferry_landed vorher der Ortstext [10] (Anlegestelle; Ortstexte des Weges, vermutet
 *   POINT1.LTX).
 * deer_hunt(p) (O116/O118/O121, ALL_SECT.LTX[80..85]): Reh im Gebuesch - Jaeger waehlen (preselect_hunter), Waffe
 *   Armbrust (+ Bolzen), Bogen (+ Pfeile) oder Speer; Schleichen +8, dann Schusswaffen bzw. Wurfwaffen +10 (Pfeil/
 *   Bolzen geht verloren), Erfolg: 2 Stunden, 20 Proviantpakete.
 * swamp_path(p, k) (ALL_SECT.LTX[53..71]): Sumpf (k = 3) bzw. Brinasker Marschen (k = 4) - Held h steckt im
 *   Morast: GE-Probe +2k, misslungen: allein -> versinkt (Gruppe aufgeloest, g_travel_state = 1), sonst Seil (oder
 *   Stabzauber 3) bzw. nach der Hand schnappen mit KK-Proben eines Helfers, Rucksack abstreifen (Plaetze 15..30
 *   ablegen), Paralyse-Probe; group_set_slot(.., -1) = Held versunken.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL163.CPP   -> 5451/5451 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - flag_not: "register signed char"-Parameter in DL, toter Epilog durch return am Ende.
 * - ford_crossing/deer_hunt: switch-Rumpfreihenfolge "case 2" vor "case 1"; am Funktionsende werden breaks zu
 *   Epilogkopien. ds:6c66 (" ") als (char far *)g_str_space_6c66 im ?: (cobjfix: 1 Inline-Wert).
 * - ferry_crossing: nr_heroes als -Og-Zwischenwert [bp-18h] fuer "a *= n; b *= n;", (int)n [bp-1ah] fuer n * 5400
 *   und sprintf; Gruppengeld ueber inline get_money() (mov eax); r dient auch als Schleifenzaehler.
 * - swamp_path: "mov al,[r] / cbw / cmp ax,k / jz / jmp near" = switch mit EINEM case + default (zweimal);
 *   "mov al,[+2e] / cbw / mov [cnt],al" = inline-Getter get_nr_heroes() auf char-Feld; Bmp b(w, h) erst nach
 *   nvfile_info deklariert (Konstruktor an der Deklarationsstelle).
 * - orc_camp_luggage/place_event_nop: Parameter p unbenutzt (Aufrufer uebergeben das Reiseobjekt).
 * Originalfehler: ferry_crossing - r bleibt uninitialisiert, wenn der Weg p->from keiner der fuenf Faelle ist (danach
 *   "if (r == 2)" auf Zufallswert); flag_not - Parameter p unbenutzt.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6c54..6c6c, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed char g_af_spell_list[16] = { 52, 12, 53, 33, 8, 27, 46, 49, 54, 56, 72, 5, 52, 53, 12, 5 };
char g_str_dot_6c64[2] = ".";   /* ds:6c64 "." - Literal von O162 (liegt im Datenbereich von O163) */

#define PIC_FERRY	0x28		/* Teilbild der Datei 0x197: Faehre */

void orc_camp_luggage(Travel far *p)
{
	register signed short i;
	register signed short n = 0;

	group_drop_most_items(g_group);
	for (i = 0; i < g_nr_items; i++) {
		if (!!g_items[i].magic) {
			n = group_find_item(g_group, i);
			if (n > 0)
				break;
		}
	}
	/* POINT2.LTX[84]: "Die Orks durchwuehlen Euer gesamtes Gepaeck und tuermen es zu einem kleinen Haufen zusammen." */
	dlg_message(g_dialog, g_tev_loc_txt[0x54]);
	if (n > 0)
		/* POINT2.LTX[85]: "Ein Schamane tritt herbei und betrachtet misstrauisch Euer Gepaeck, das gerade von einem der
		 * Orks sortiert wird. <Zauberzeuk kefeerlich!> zischt der Schamane ploetzlich, worauf der Ork erschrocken und
		 * mit spitzen Fingern einige der Dinge wieder in Eure Rucksaecke zurueckschiebt." */
		dlg_message(g_dialog, g_tev_loc_txt[0x55]);
	/* POINT2.LTX[86]: "Schliesslich duerft Ihr die Orks passieren." */
	dlg_message(g_dialog, g_tev_loc_txt[0x56]);
	g_low_arrival_due = 1;
}

signed char flag_not(Travel far *p, register signed char c)
{
	if (c == 1)
		c = 0;
	else
		c = 1;
	return c;
}

void place_event_nop(Travel far *p)
{
}

void ford_crossing(Travel far *p)
{
	signed char r;
	signed char dark;
	register signed short water = 0;

	p->terrain = 7;
	if (p->from == 0x17 || p->from == 0x2c)
		dark = 1;
	else
		dark = 0;
	water = group_refill_water(g_group);
	/* ALL_SECT.LTX[23]: "Ihr steht am Ufer des %s%s. Eine Furt fuehrt ans andere Ufer. Wollt Ihr" */
	/* ALL_SECT.LTX[24]: "Finsteren Svellt" */
	/* ALL_SECT.LTX[25]: "Svellt" */
	/* ALL_SECT.LTX[20]: " und fuellt zuerst einmal Eure Wasserschlaeuche" */
	sprintf(g_text_output_buf, g_tev_txt[0x17], dark == 1 ? g_tev_txt[0x18] : g_tev_txt[0x19],
		water ? g_tev_txt[0x14] : (char far *)" ");
	/* ALL_SECT.LTX[26]: "die Furt durchqueren oder" */
	/* ALL_SECT.LTX[27]: "umkehren und einen besseren Weg suchen?" */
	r = dlg_choose(g_dialog, g_text_output_buf, 2, g_tev_txt[0x1a], g_tev_txt[0x1b]);
	switch (r) {
	case 2:
		p->action = TRAVEL_ACT_TURN;
		p->ev_half = 0;
		break;
	case 1:
		/* ALL_SECT.LTX[28]: "%s erreicht sicher das andere Ufer" */
		/* ALL_SECT.LTX[29]: "%s nimmt ein unfreiwilliges Bad!" */
		group_test_attrib_disease(g_group, ATTRIB_GE, 0, g_tev_txt[0x1c], g_tev_txt[0x1d], 0x2d, 2, 5);
		break;
	}
}

void ferry_crossing(Travel far *p, signed short wait_price, signed short now_price)
{
	signed long money;
	signed short dummy;
	signed char r;
	signed char n;
	signed char night;
	Bmp far *pic;
	Bmp far *pic2;
	struct hero far *h;

#if FIX_0150
	/* Fix #150: r vorbelegen (Ort ohne Faehrfall) */
	r = 0;
	pic = new Bmp(0x40, 0x40);
	pic2 = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x197, pic->data(), (void far *)0, PIC_FERRY, 0, &dummy, &dummy, 0x140);
#else
	pic = new Bmp(0x40, 0x40);
	pic2 = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x197, pic->data(), (void far *)0, PIC_FERRY, 0, &dummy, &dummy, 0x140);
#endif
	nvfile_load(g_nvf_heads, 0x197, pic2->data(), (void far *)0, 0, 0, &dummy, &dummy, 0x140);
	if (g_ferry_landed == 1)
		/* POINT1.LTX[10]: "Ihr legt am linkssvelltschen Ufer an, wo an einem kleinen Platz eine Faehrstation steht." */
		dlg_box_bmp(g_dialog, pic, (char far *)0, g_tev_loc_txt[10], 0);
	wait_price *= g_group->nr_heroes;
	now_price *= g_group->nr_heroes;
	p->terrain = 7;
	money = g_group->get_money();
	night = 0;
	switch (p->from) {
	case 0x27:
	case 0x28:
		/* ALL_SECT.LTX[30]: "Ihr steht wieder am Ufer des Finsteren Svellt. Der Faehrmann sitzt bei der wenig
		 * vertrauenerweckenden Faehre. Wollt Ihr" */
		/* ALL_SECT.LTX[34]: "Euch bei dem Faehrmann nach einer Ueberfahrt erkundigen, oder" */
		/* ALL_SECT.LTX[35]: "umdrehen, und auf dieser Seite des Flusses einen Weg suchen?" */
		r = dlg_box_bmp(g_dialog, pic2, (char far *)0, g_tev_txt[0x1e], 2, g_tev_txt[0x22], g_tev_txt[0x23]);
		break;
	case 0x0f:
		/* ALL_SECT.LTX[31]: "Ihr habt gerade erst Tiefhusen verlassen, als Ihr eine Faehrstation erreicht. Das Floss
		 * steuert gerade das diesseitige Flussufer an. Wollt Ihr" */
		r = dlg_box_bmp(g_dialog, pic, (char far *)0, g_tev_txt[0x1f], 2, g_tev_txt[0x22], g_tev_txt[0x23]);
		break;
	case 0x1e:
		/* ALL_SECT.LTX[32]: "Ihr habt seit Hilvalla erst wenige 100 Schritte zurueckgelegt, als Ihr eine Faehrstation
		 * erreicht. Das Floss steuert gerade das diesseitige Flussufer an. Wollt Ihr" */
		r = dlg_box_bmp(g_dialog, pic, (char far *)0, g_tev_txt[0x20], 2, g_tev_txt[0x22], g_tev_txt[0x23]);
		break;
	case 0xb9:
	case 0xc6:
		/* ALL_SECT.LTX[33]: "Ihr erreicht das Svelltufer. Der Weg endet auf einem kleinen Platz, an dem eine
		 * Faehrstation steht. Im Fluss seht Ihr das Floss, das gerade das diesseitige Flussufer ansteuert. Wollt Ihr" */
		r = dlg_box_bmp(g_dialog, pic, (char far *)0, g_tev_txt[0x21], 2, g_tev_txt[0x22], g_tev_txt[0x23]);
		break;
	}
	delete pic2;
	if (r == 2) {
		p->action = TRAVEL_ACT_TURN;
		p->ev_half = 0;
		p->ev_start = 0;
		p->ev_end = 0;
	} else {
		if (p->from == 0x27 || p->from == 0x28) {
			delete pic;
			pic = new Bmp(0x40, 0x40);
			nvfile_load(g_nvf_heads, 0x197, pic->data(), (void far *)0, 2, 0, &dummy, &dummy, 0x140);
		}
		/* ALL_SECT.LTX[40]: "Der Faehrmann begruesst Euch muerrisch: <Wollt Ihr rueber? Macht %d Silbertaler pro Person,
		 * wenn Ihr wartet, bis ich genuegend Fahrgaeste zusammen habe, ansonsten %d Silber pro Person. Also, was wollt
		 * Ihr?>" */
		sprintf(g_text_output_buf, g_tev_txt[0x28], wait_price, now_price);
		/* ALL_SECT.LTX[41]: "<Wir haben es eilig. Hier sind die %d Silbertaler fuer Jeden. Mach hin!>" */
		sprintf(g_text_buf2, g_tev_txt[0x29], now_price);
		if (now_price * 10 <= money) {
			do {
				/* ALL_SECT.LTX[42]: "<Danke, wir suchen uns lieber einen anderen Weg.>" */
				/* ALL_SECT.LTX[43]: "<Wir warten, bis Ihr Euer Boot voll habt.>" */
				r = dlg_box_bmp(g_dialog, pic, (char far *)0, g_text_output_buf, 3, g_tev_txt[0x2a],
					g_tev_txt[0x2b],
					g_text_buf2);
			} while (r == -1);
		} else if (wait_price * 10 <= money) {
			do {
				/* ALL_SECT.LTX[44]: "<Wir koennen es uns nicht leisten sofort ueberzusetzen, also warten wir, bis Ihr Euer
				 * Boot voll habt.>" */
				r = dlg_box_bmp(g_dialog, pic, (char far *)0, g_text_output_buf, 2, g_tev_txt[0x2a],
					g_tev_txt[0x2c]);
			} while (r == -1);
		} else {
			r = 1;
			dlg_box_bmp(g_dialog, pic, (char far *)0, g_text_output_buf, 0);
			/* ALL_SECT.LTX[45]: "Da Ihr Euch die Ueberfahrt nicht leisten koennt bleibt Euch nichts anderes uebrig, als
			 * wieder umzudrehen. Vorher schnauzt Ihr aber noch den Faehrmann an:@ <Die Faehrpreise sind uns viel zu hoch,
			 * da suchen wir uns lieber einen anderen Weg.>" */
			dlg_message(g_dialog, g_tev_txt[0x2d]);
		}
		if (r == 1) {
			p->action = TRAVEL_ACT_TURN;
			p->ev_half = 0;
			p->ev_start = 0;
			p->ev_end = 0;
		}
		if (r == 2) {
			n = (random_schweif(6) + 4) / 2;
			if (g_daytime + n * 5400 < 97200L) {
				/* ALL_SECT.LTX[46]: "Ihr wartet %d Stunden, bis endlich genuegend Fahrgaeste zusammem sind. Nach kurzer
				 * Ueberfahrt erreicht Ihr das andere Ufer." */
				sprintf(g_text_output_buf, g_tev_txt[0x2e], n);
				while (n > 0) {
					timewarp_hours(g_date, 1L);
					n -= 1;
				}
				dlg_message(g_dialog, g_text_output_buf);
				group_sub_money(g_group, wait_price * 10);
			} else {
				/* ALL_SECT.LTX[47]: "<Da es schon spaet ist, werden heute wohl nicht mehr genug Fahrgaeste kommen> lautet der
				 * Kommentar des Faehrmannes und macht sich auf, um nach Hause zu gehen. Wollt Ihr" */
				/* ALL_SECT.LTX[48]: "ein Nachtlager aufschlagen" */
				/* ALL_SECT.LTX[49]: "doch einen anderen Weg waehlen" */
				/* ALL_SECT.LTX[50]: "oder den Faehrmann bremsen: <Dann setzen wir doch lieber gleich ueber>" */
				r = dlg_box_bmp(g_dialog, pic, (char far *)0, g_tev_txt[0x2f], 3, g_tev_txt[0x30],
					g_tev_txt[0x31],
					g_tev_txt[0x32]);
				switch (r) {
				case 2:
					p->action = TRAVEL_ACT_TURN;
					p->ev_half = 0;
					p->ev_start = 0;
					p->ev_end = 0;
					break;
				case 3:
					if (now_price * 10 <= money) {
						r = 3;
					} else {
						/* ALL_SECT.LTX[51]: "Ihr fragt den Faehrmann, ob er Euch auch fuer weniger Geld uebersetzt, aber der bleibt
						 * hart. Ihr muesst also bis zum naechsten Morgen warten." */
						dlg_message(g_dialog, g_tev_txt[0x33]);
						night = 1;
						r = 0;
					}
					break;
				default:
					night = 1;
					break;
				}
			}
		}
		if (r == 3) {
			timewarp_minutes(g_date, 0x1eL);
			group_sub_money(g_group, now_price * 10);
			/* ALL_SECT.LTX[52]: "Nach kurzer Ueberfahrt erreicht Ihr das andere Ufer." */
			sprintf(g_text_output_buf, g_tev_txt[0x34]);
			if (group_refill_water(g_group))
				/* ALL_SECT.LTX[38]: "@Ihr fuellt noch schnell Eure Wasserschlaeuche und macht Euch dann wieder auf den Weg." */
				strcat(g_text_output_buf, g_tev_txt[0x26]);
			dlg_message(g_dialog, g_text_output_buf);
		}
		if (night == 1) {
			for (r = 0; r < 7; r++) {
				h = g_heroes + r;
				if (h->type != 0) {
					sub_hunger(h, 0x5a);
					sub_thirst(h, 0x5a);
				}
			}
			/* ALL_SECT.LTX[133]: "Als es zu daemmern beginnt, schlagt ihr ein notduerftiges Lager auf und legt Euch total
			 * erschoepft zur Ruhe." */
			dlg_message(g_dialog, g_tev_txt[0x85]);
			timewarp_until(g_date, 0xa8c0L);	/* 8 Uhr */
			group_sub_money(g_group, wait_price * 10);
			/* ALL_SECT.LTX[39]: "Am fruehen Morgen erscheint der Faehrmann wieder und nach kurzer Ueberfahrt erreicht Ihr
			 * die andere Seite." */
			sprintf(g_text_output_buf, g_tev_txt[0x27]);
			if (group_refill_water(g_group))
				strcat(g_text_output_buf, g_tev_txt[0x26]);
			dlg_message(g_dialog, g_text_output_buf);
		}
	}
	delete pic;
}

void deer_hunt(Travel far *p)
{
	signed short dummy;
	signed char r;
	signed char weapon = 0;		/* 1 Armbrust, 2 Bogen, 3 Speer */
	struct hero far *h;
	Bmp b(0x40, 0x40);
	register signed short res;
	register signed short ammo;

	nvfile_load(g_nvf_heads, 0x197, b.data(), (void far *)0, 0x0a, 0, &dummy, &dummy, 0x140);
	/* ALL_SECT.LTX[80]: "Ihr entdeckt ein Reh im Gebuesch. Wollt Ihr versuchen, es zu schiessen?" */
	/* ALL_SECT.LTX[1]: "Ja" */
	/* ALL_SECT.LTX[2]: "Nein" */
	r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_txt[0x50], 2, g_tev_txt[1], g_tev_txt[2]);
	switch (r) {
	case 2:
		break;
	case 1:
		preselect_hunter(g_group);
		/* ALL_SECT.LTX[84]: "Wer will es versuchen?" */
		h = get_active_hero(g_group, g_tev_txt[0x54]);
		ammo = get_item_pos(h, ITEM_BOLT);
		if ((get_item_pos(h, ITEM_CROSSBOW) != -1 || get_item_pos(h,
			ITEM_HEAVY_CROSSBOW) != -1) && ammo != -1) {
			weapon = 1;
		} else {
			ammo = get_item_pos(h, ITEM_ARROW);
			if ((get_item_pos(h, ITEM_SHORTBOW) != -1 || get_item_pos(h, ITEM_LONGBOW) != -1 ||
			     get_item_pos(h, ITEM_ARTHERION_BOW) != -1) && ammo != -1)
				weapon = 2;
			else if (get_item_pos(h, ITEM_SPEAR) != -1 || get_item_pos(h, ITEM_SPEAR_D8) != -1)
				weapon = 3;
		}
		if (weapon == 0) {
			/* ALL_SECT.LTX[85]: "%s besitzt keine Waffen, mit denen man ein Reh auf diese Entfernung toeten koennte!" */
			sprintf(g_text_output_buf, g_tev_txt[0x55], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			res = test_skill(h, TA_SCHLEICHEN, 8);
			timewarp_minutes(g_date, 10L);
			if (res <= 0) {
				/* ALL_SECT.LTX[81]: "Als %s noch etwa zwanzig Schritt entfernt ist, hebt das Tier seinen Kopf und sieht %s
				 * direkt in die Augen. Dann verschwindet es in grossen Spruengen im Wald." */
				sprintf(g_text_output_buf, g_tev_txt[0x51], h->name, grammar_pronoun(h->sex, 3, 0));
				dlg_message(g_dialog, g_text_output_buf);
			} else {
				if (weapon != 3) {
					drop_item(h, ammo, 1);
					res = test_skill(h, TA_SCHUSSWAFFEN, 10);
				} else {
					res = test_skill(h, TA_WURFWAFFEN, 10);
				}
				if (res <= 0) {
					/* ALL_SECT.LTX[83]: "Der %s zischt vorbei und das Reh sucht sein Heil in der Flucht. Vernuenftig
					 * eigentlich..." */
					/* ALL_SECT.LTX[37]: "Speer" */
					/* ALL_SECT.LTX[36]: "Pfeil" */
					sprintf(g_text_output_buf, g_tev_txt[0x53],
						weapon == 3 ? g_tev_txt[0x25] : g_tev_txt[0x24]);
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					/* ALL_SECT.LTX[82]: "%s erlegt das Reh, als haette %s %s Leben lang nichts anderes gemacht." */
					sprintf(g_text_output_buf, g_tev_txt[0x52], h->name, grammar_pronoun(h->sex,
						0, 0),
						grammar_pronoun(h->sex, 1, 0));
					dlg_message(g_dialog, g_text_output_buf);
					travel_redraw(p);
					timewarp_hours(g_date, 2L);
					group_give_item(g_group, ITEM_FOOD_PACKAGE, 0x14);
				}
			}
		}
		break;
	}
}

void swamp_path(Travel far *p, signed char k)
{
	signed char r;
	signed char res;
	signed char cnt;
	signed char i;
	signed short w;
	signed short hgt;
	signed short dummy;
	struct hero far *h;
	struct hero far *helper;
	register signed short pos;

	nvfile_info(g_nvf_heads, 0x196, 1, &w, &hgt);
	Bmp b(w, hgt);
	nvfile_load(g_nvf_heads, 0x196, b.data(), (void far *)0, 1, 0, &dummy, &dummy, 0x140);
	h = get_first_active_hero(g_group);
	if (k == 3)
		/* ALL_SECT.LTX[53]: "In den letzten Minuten ist der Weg immer feuchter geworden und %s hat schon ein paarmal bis
		 * ueber die Knie im Schlamm gesteckt. Jetzt koennt Ihr sogar einige Sumpfloecher vor Euch erkennen. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_txt[0x35], h->name);
	if (k == 4)
		/* ALL_SECT.LTX[54]: "Der Weg in den Brinasker Marschen wird immer schlammiger. %s ist schon mehrfach bis zu den
		 * Hueften in dem tiefen Morast versunken. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_txt[0x36], h->name);
	cnt = g_group->get_nr_heroes();
	pos = group_hero_pos(g_group, h);
	g_dlg_box_flag = 0;
	if (cnt != 1)
		/* ALL_SECT.LTX[55]: "wieder umdrehen oder" */
		/* ALL_SECT.LTX[56]: "in die Fussstapfen Eures Anfuehrers treten?" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)" ", g_text_output_buf, 2, g_tev_txt[0x37],
			g_tev_txt[0x38]);
	else
		/* ALL_SECT.LTX[57]: "weiter in den Sumpf vordringen?" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)" ", g_text_output_buf, 2, g_tev_txt[0x37],
			g_tev_txt[0x39]);
	switch (r) {
	case 2:
		res = test_attrib(h, ATTRIB_GE, k * 2);
		if (res <= 0) {
			cnt = g_group->get_nr_heroes();
			pos = group_hero_pos(g_group, h);
			if (cnt == 1) {
				/* ALL_SECT.LTX[58]: "%s ist in ein tiefes Sumpfloch getreten und ist nicht in der Lage sich selbst zu
				 * befreien." */
				sprintf(g_text_output_buf, g_tev_txt[0x3a], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				group_set_slot(g_group, pos, -1);
				g_travel_state = 1;
				group_set_on_route(g_group, 0);
			} else {
				/* ALL_SECT.LTX[59]: "%s ist in ein tiefes Sumpfloch getreten und sinkt immer tiefer ein. Wollt Ihr" */
				sprintf(g_text_output_buf, g_tev_txt[0x3b], h->name);
				/* ALL_SECT.LTX[60]: "ein Seil benutzen, um %s gemeinsam herauszuziehen oder" */
				sprintf(g_text_buf2, g_tev_txt[0x3c], grammar_pronoun(h->sex, 2, 0));
				/* ALL_SECT.LTX[61]: "keine Zeit verlieren und nach der Hand schnappen?" */
				r = dlg_choose(g_dialog, g_text_output_buf, 2, g_text_buf2, g_tev_txt[0x3d]);
				hero_disease_test(h, ILLNESS_PARALYSE, 40 - h->attrib[ATTRIB_KK].cur);
				helper = get_next_hero(g_group, h);
				switch (r) {
				case 1:
					res = group_find_item(g_group, ITEM_ROPE);
					if (res < 0 && get_hero_staffspell(g_group, 3) == 0) {
						/* ALL_SECT.LTX[62]: "In der Zeit, in der Ihr vergeblich nach einem Seil gesucht habt, ist %s leider
						 * versunken." */
						sprintf(g_text_output_buf, g_tev_txt[0x3e], h->name);
						dlg_message(g_dialog, g_text_output_buf);
						group_set_slot(g_group, pos, -1);
					} else {
						drop_item_at(h, 3);
						drop_item_at(h, 0x0c);
						res = test_attrib(helper, ATTRIB_KK, k - (cnt * 3 - 3));
						if (cnt == 2) {
							if (res < 0) {
								/* ALL_SECT.LTX[63]: "Leider gelingt es Dir nicht %s am Versinken zu hindern." */
								sprintf(g_text_output_buf, g_tev_txt[0x3f], h->name);
								dlg_message(g_dialog, g_text_output_buf);
								group_set_slot(g_group, pos, -1);
							} else {
								/* ALL_SECT.LTX[64]: "Nach grosser Anstrengung ist es Dir gelungen %s zu befreien." */
								sprintf(g_text_output_buf, g_tev_txt[0x40], h->name);
								dlg_message(g_dialog, g_text_output_buf);
							}
						} else {
							if (res < 0) {
								/* ALL_SECT.LTX[65]: "Trotz gemeinsamer Anstrengungen gelingt es Euch nicht %s vor dem Versinken zu
								 * bewahren." */
								sprintf(g_text_output_buf, g_tev_txt[0x41], h->name);
								dlg_message(g_dialog, g_text_output_buf);
								group_set_slot(g_group, pos, -1);
							} else {
								/* ALL_SECT.LTX[66]: "Unter gemeinsamen Anstrengung gelingt es Euch %s Stueck fuer Stueck aus dem Morast zu
								 * befreien." */
								sprintf(g_text_output_buf, g_tev_txt[0x42], h->name);
								dlg_message(g_dialog, g_text_output_buf);
							}
						}
					}
					break;
				default:
					drop_item_at(h, 3);
					drop_item_at(h, 0x0c);
					res = test_attrib(helper, ATTRIB_KK, k);
					if (res < 0) {
						/* ALL_SECT.LTX[67]: "Obwohl es %s gelingt, die Hand von %s zu ergreifen, sinkt %s immer tiefer und steckt
						 * bereits bis zum Hals im Schlamm, als es %s gelingt, mit der freien Hand den Rucksack abzustreifen." */
						sprintf(g_text_output_buf, g_tev_txt[0x43], helper->name, h->name,
							grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 3,
								0));
						dlg_message(g_dialog, g_text_output_buf);
						sub_hero_le(h, random_schweif(10));
						for (i = 15; i <= 30; i++)	/* Rucksack abstreifen */
							drop_item_at(h, i);
						res = test_attrib(helper, ATTRIB_KK, k - 6);
						if (res < 0) {
							/* ALL_SECT.LTX[68]: "Trotzdem %s jetzt leichter ist, versinkt %s mit einem gurgelnden Laut im Schlamm." */
							sprintf(g_text_output_buf, g_tev_txt[0x44], h->name,
								grammar_pronoun(h->sex, 0, 0));
							dlg_message(g_dialog, g_text_output_buf);
							group_set_slot(g_group, pos, -1);
						} else {
							/* ALL_SECT.LTX[69]: "Da %s jetzt leichter ist, reicht %s Kraft aus, %s aus dem Schlamm zu befreien." */
							sprintf(g_text_output_buf, g_tev_txt[0x45], h->name,
								grammar_pronoun(helper->sex, 1, 1),
									grammar_pronoun(h->sex, 2, 0));
							dlg_message(g_dialog, g_text_output_buf);
						}
					} else {
						/* ALL_SECT.LTX[70]: "Tatsaechlich gelingt es %s die Hand von %s zu ergreifen, und %s aus dem Schlamm zu
						 * ziehen." */
						sprintf(g_text_output_buf, g_tev_txt[0x46], helper->name, h->name,
							grammar_pronoun(h->sex, 2, 0));
						dlg_message(g_dialog, g_text_output_buf);
					}
					break;
				}
			}
		} else {
			/* ALL_SECT.LTX[71]: "Mit traumwandlerischer Sicherheit fuehrt %s Eure Gruppe um die gefaehrlichen Sumpfloecher
			 * herum." */
			sprintf(g_text_output_buf, g_tev_txt[0x47], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
		break;
	default:
		p->action = TRAVEL_ACT_TURN;
		p->ev_half = 0;
		break;
	}
}
