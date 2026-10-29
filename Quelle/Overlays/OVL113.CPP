/* OVL113 - Overlay O113 (Segment 0x2188 in SCHWEIF.EXE): Reiseereignisse der Wege 0..8 (tevent_run O211 ruft je Weg
 * mit dem Reiseobjekt o; Ort o->from, Texte g_tev_loc_txt = SECTION.LTX und g_tev_txt = ALL_SECT.LTX, Merker
 * route.event der Wegtabelle g_routes, Bilder per nvfile_load aus g_nvf_heads in ein Bmp 64x64), Geschwister
 *   O114/O115.
 * tevent_000 (ab Ort 1): bei travel_stage == 1 einmalig (g_rondra_priestess_met) und hoechstens 100 AP je Held: Kampfeslaerm, Bild
 *   0x1c - Rondra-Geweihte gegen fuenf Orks; helfen -> Kampf 0x83 (fight_result), gewonnen: Bild 0x1e, Texte 0x13e
 *   und Ortstexte neu laden, Dank der Geweihten txt[7..9], group_add_ap_each(300), g_scene31_due = 1, Tagebuch
 *   ALL_SECT[0x75], Texte frei, check_level_up. Danach bei route.event == 0x32 und travel_stage == 2: Bild 0x196 -
 *   Kriegerhuene, ansprechen -> Gespraech 0x82 (conv_run), txt[13] bei g_tev000_warrior_cursing.
 * tevent_001 (Ort 0x5c) / tevent_002 (Ort 0x21): erster Besuch (g_tev_peddler_trail == 0) Bild 0x2f - Haendler mit
 *   Kiepe (Gespraech 0x83), Kupferkessel fuer 120 Heller; Merker route.event herunterzaehlen; zweiter Besuch Bild
 *   0x30
 *   - Blutspur folgen (1 Stunde): Leiche begraben (2 Stunden, g_god_favor_boron += 40) oder liegen lassen
 *   (g_god_favor_boron -= 100, g_god_favor_rondra -= 40).
 * tevent_003: Pflanzenkunde-Probe (route.event * 3 + 5) -> W6 Wirselkraut, Zaehler bis 40.
 * tevent_004 (Ort 0x22) / tevent_005 (0x24): Zaehler route.event [4]/[5]/[30] erhoehen, Kampfstaerke
 *   g_fight_max_enemies = 12 bzw. n * 2 - 100, Kampf 2 bzw. 1. tevent_006: Bild 0x29, Halmar-Antilope - Loewen
 *   (Kampf 0x2d/0x2c, Staerke 2/3 nach Gruppengroesse), gewonnen -> 8 x (travel_redraw, 15 Minuten), 15
 *   Proviantpakete; [6]--. tevent_007 (Ort 0x25): zwei Elfen mit Langboegen (Bild 0x2a, zwei Menues, switch-
 *   Rumpfreihenfolge 1, 3, 2 bzw. 1, 2, 3), ggf. Kampf 0x2e; [7]--. tevent_008: guide_offer(o, 4/3) je nach Ort
 *   0x26/2, bei g_guide_won route.event[8] = 0.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL113.CPP -> 4789/4789 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. -Oi wirkungslos (auch -3 -Ot -Og GLEICH), ohne -Og 2192 Abweichungen (Epilogkopien in
 *   tevent_001/tevent_002, toter Epilog).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Vorlage OVL115: "r = dlg_box_bmp(..); if (r == 1)", "r = -1; while (r == -1)".
 * - Lokale Bmp-Objekte in Bloecken (tevent_000 im inneren if, tevent_001/tevent_002 mitten im then-Block bzw. im
 *   else-if-
 *   Block, beide auf [bp-16h]) -> Destruktor Bmp_dtor(&b, 2) am Blockende; new/delete -> Bmp-Konstruktor (0, ..)/(p,
 *   3).
 * - "(signed short)g_group->nr_heroes <= 3" -> mov al / cbw / cmp ax,3 (ohne Cast cmp byte); dagegen
 *   "group_sum_ap(g) /
 *   g->nr_heroes" ergibt ohne Cast cbw / movsx ebx,ax / idiv ebx.
 * - "g_routes[1].event -= 2" -> mov al / add al,0feh / mov; "-= 1" -> dec al; "x = x + 1" -> inc al; "x++" -> inc
 *   byte.
 * - "g_fight_max_enemies = c >= 0x38 ? 12 : c * 2 - 100" -> gemeinsamer Store, add ax,0ff9ch.
 * Auffaelligkeiten (Original):
 * - tevent_000 laedt die Texte (g_tev_buf/g_tev_txt/g_tev_loc_buf/g_tev_loc_txt) neu, ueberschreibt dabei die von
 *   O211 geladenen Zeiger
 *   (Leck) und gibt sie selbst frei, nullt aber nur g_tev_buf; kehrt check_level_up zurueck, gibt O211
 *   g_tev_txt/g_tev_loc_buf/
 *   g_tev_loc_txt ein zweites Mal frei.
 * - tevent_000: dlg_choose bekommt die Antworten in der Reihenfolge txt[2], txt[1].
 */
#include <stdio.h>

#include "schweif.h"

void tevent_000(Travel far *o)
{
	signed short dummy;
	signed char r;
	signed char x;
	signed char flag = 0;
	signed short w;
	signed short h;
	Bmp far *p;
	Bmp far *p2;

	if (o->from == 1) {
		if (travel_stage(o) == 1 && g_rondra_priestess_met == 0) {
			if (group_sum_ap(g_group) / g_group->nr_heroes <= 100)
				flag = 1;
			g_rondra_priestess_met = 1;
			if (flag == 1) {
				/* SECTION.LTX[0]: "Ihr habt die Stadt gerade erst verlassen, als Ihr links vom Weg ploetzlich Kampfeslaerm
				 * hoert. Wollt Ihr" */
				/* SECTION.LTX[2]: "Nachsehen, was da los ist" */
				/* SECTION.LTX[1]: "Einfach weitergehen" */
				r = dlg_choose(g_dialog, g_tev_loc_txt[0], 2, g_tev_loc_txt[2], g_tev_loc_txt[1]);
				if (r == 1) {
					p = new Bmp(0x40, 0x40);
					nvfile_load(g_nvf_heads, 0x197, p->data(), 0, 0x1c, 0, &dummy, &dummy, 0x140);
					/* SECTION.LTX[3]: "Am Rand einer Lichtung steht eine Frau in den Gewaendern der Rondra an einen grossen Baum
					 * gelehnt, waehrend fuenf Orks heftig auf sie eindringen. Obwohl die Priesterin Euch gesehen hat, bittet sie
					 * doch mit keinem Wort um Hilfe, aber Ihr koennt unschwer erkennen, dass sie bereits einige Treffer hat
					 * hinnehmen muessen. Wollt Ihr" */
					/* SECTION.LTX[4]: "der Frau helfen oder" */
					/* SECTION.LTX[5]: "sie Ihrem Stolz und Ihrem Schicksal ueberlassen und Eurer Wege gehen?" */
					r = dlg_box_bmp(g_dialog, p, (char far *)0, g_tev_loc_txt[3], 2, g_tev_loc_txt[4], g_tev_loc_txt[5]);
					delete p;
					if (r == 1) {
						travel_leave(o);
						x = start_fight(0x83, 0);
						x = fight_result(o, x);
						if (!x) {
							Bmp b(0x40, 0x40);

							nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x1e, 0, &dummy, &dummy, 0x140);
							txt_load_list(g_font, 0x13e, 0x8c, &g_tev_buf, &g_tev_txt);
							txt_load_list(g_font, o->evtxt_file, o->evtxt_n, &g_tev_loc_buf, &g_tev_loc_txt);
							/* SECTION.LTX[7]: "Gerade, als Ihr Euch um die Geweihte kuemmern wollt, kommt sie wieder zu sich. <Ihr habt
							 * mir das Leben gerettet> stellt sie kuehl fest, <und dafuer danke ich Euch, obwohl ich Euch nicht darum
							 * gebeten habe.>@Dann jedoch bekommen Ihre Augen einen eigenartigen Ausdruck, und mit einer leicht
							 * veraenderten Stimme, die Euch die Haare zu Berge stehen laesst, redet sie weiter." */
							dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[7], 0);
							/* SECTION.LTX[8]: "<Ihr habt etwas gefaehrliches vor, meine Kleinen. Ihr werdet einer gewaltigen Uebermacht
							 * gegenuebertreten; das ehrt uns und Euch. An Mut mangelt es Euch nicht, das sehen wir wohl, aber an
							 * Erfahrung. Ihr wuerdet in den sicheren Tod gehen; aber das koennen wir nicht zulassen bei solch tapferen
							 * Recken.@Ich will Euch darum etwas unserer Erfahrung, sagen wir, leihen. Aber nutzt sie wohl und macht uns
							 * keine Schande.>" */
							dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[8], 0);
							/* SECTION.LTX[9]: "Die Frau zuckt einmal kurz und steht dann auf. <Habt nochmal meinen Dank, und gehabt
							 * Euch wohl> sagt sie, jedoch nun ohne diesen Ausdruck in  ihrer Stimme, und verschwindet im Wald." */
							dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[9], 0);
							group_add_ap_each(g_group, 300);
							g_scene31_due = 1;
							/* ALL_SECT.LTX[117]: "Kurz nach Kvirasim mussten wir beobachten, wie eine Horde Orks ueber eine Geweihte
							 * herfiel. Als Dank fuer unseren Beistand <lieh> uns die Goettin Rondra, die durch diese Geweihte zu uns
							 * sprach, etwas von ihrer Erfahrung. Wir fuehlen uns nicht anders als vorher, aber wir werden wohl bald
							 * merken, was davon zu halten ist." */
							diary_add(g_diary, g_tev_txt[0x75], 0);
#if FIX_0112
							/* Fix #112: alle vier Zeiger nach dem Freigeben nullen */
							g_tev_buf = mem_free(g_tev_buf);
							g_tev_txt = (char far * far *)mem_free(g_tev_txt);
							g_tev_loc_buf = mem_free(g_tev_loc_buf);
							g_tev_loc_txt = (char far * far *)mem_free(g_tev_loc_txt);
#else
							g_tev_buf = mem_free(g_tev_buf);
							mem_free(g_tev_txt);
							mem_free(g_tev_loc_buf);
							mem_free(g_tev_loc_txt);
#endif
							check_level_up();
						}
						travel_enter(o);
					}
				}
			}
		}
		if (g_routes[0].event == 0x32 && travel_stage(o) == 2) {
			g_routes[0].event = 0;
			nvfile_info(g_nvf_heads, 0x196, 0, &w, &h);
			p2 = new Bmp(w, h);
			nvfile_load(g_nvf_heads, 0x196, p2->data(), 0, 0, 0, &dummy, &dummy, 0x140);
			g_dlg_box_flag = 0;
			/* SECTION.LTX[10]: "Ein breitschultriger Mann mit Unmengen von martialischem Schmuck wie Riesenohrringen,
			 * Oberarmreifen, Ketten aus Zaehnen usw., beladen mit einem Rucksack, zwei Schwertern, einem Langbogen, einer
			 * Armbrust, einer grossen Axt und einem halben Dutzend Wurfdolchen... kommt Euch auf der Strasse entgegen.
			 * Wollt Ihr" */
			/* SECTION.LTX[11]: "den Mann ansprechen oder" */
			/* SECTION.LTX[12]: "staunend an ihm vorbeigehen?" */
			r = dlg_box_bmp(g_dialog, p2, (char far *)0, g_tev_loc_txt[10], 2, g_tev_loc_txt[11], g_tev_loc_txt[12]);
			delete p2;
			if (r == 1) {
				travel_leave(o);
				if (conv_run(g_dialog2, 0x82))
					timewarp_minutes(g_date, 30L);
				mouse_hide(0);
				travel_enter(o);
				mouse_show(0);
				travelmap_draw(o);
				travel_redraw(o);
			}
			if (g_tev000_warrior_cursing == 1)	/* nie gesetzt - Originalfehler Nr. 233 */
				/* SECTION.LTX[13]: "Waehrend der Mann hinter der naechsten Biegung verschwindet, hoert Ihr sein Geschimpfe
				 * langsam leiser werden und schliesslich verstummen." */
				dlg_message(g_dialog, g_tev_loc_txt[13]);
		}
	}
}

void tevent_001(Travel far *o)
{
	signed short dummy;
	struct hero far *h;
	signed char r;
	Bmp far *p;

	if (o->from == 0x5c && g_tev_peddler_trail == 0) {
		p = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, p->data(), 0, 0x2f, 0, &dummy, &dummy, 0x140);
		/* SECTION.LTX[14]: "Euch kommt auf der Strasse ein Mann entgegen, der auf seinem Ruecken eine grosse Kiepe
		 * traegt. Bei jedem Schritt klappert und klirrt es maechtig, und sein gebeugter Ruecken zeigt deutlich, dass er
		 * ein schweres Gewicht mit sich herumtraegt. <Phex und alle anderen mit Euch!> ruft er Euch bereits auf eine
		 * grosse Entfernung lauthals entgegen." */
		dlg_box_bmp(g_dialog, p, (char far *)0, g_tev_loc_txt[14], 0);
		g_tev_peddler_trail = 1;
		delete p;
		travel_leave(o);
		if (conv_run(g_dialog2, 0x83))
			timewarp_minutes(g_date, 30L);
		mouse_hide(0);
		travel_enter(o);
		mouse_show(0);
		travelmap_draw(o);
		travel_redraw(o);
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x2f, 0, &dummy, &dummy, 0x140);
		/* SECTION.LTX[15]: "<Wollt Ihr vielleicht einen Kupferkessel kaufen?@Er soll nur 12 Silbertaler kosten!>" */
		/* ALL_SECT.LTX[1]: "Ja" */
		/* ALL_SECT.LTX[2]: "Nein" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[15], 2, g_tev_txt[1], g_tev_txt[2]);
		if (r == 1 && g_group->money >= 120)  {
			group_give_item(g_group, ITEM_COPPER_KETTLE, 1);
			group_sub_money(g_group, 120L);
		}
		if (r == 1 && g_group->money < 120)
			/* SECTION.LTX[16]: "Ihr habt leider nicht die noetigen 12 Silbertaler." */
			dlg_message(g_dialog, g_tev_loc_txt[16]);
		g_routes[1].event -= 2;
		g_routes[2].event -= 1;
	} else if (g_tev_peddler_trail == 1) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x30, 0, &dummy, &dummy, 0x140);
		h = get_first_active_hero(g_group);
		/* SECTION.LTX[17]: "%s entdeckt am Strassenrand einige Blutflecken und eine Schleifspur! Wollt Ihr ihr in das
		 * Unterholz hinein folgen?" */
		sprintf(g_text_output_buf, g_tev_loc_txt[17], h->name);
		r = dlg_choose(g_dialog, g_text_output_buf, 2, g_tev_txt[1], g_tev_txt[2]);
		if (r == 1) {
			g_tev_peddler_trail = 0;
			g_routes[1].event -= 1;
			timewarp_hours(g_date, 1L);
			/* SECTION.LTX[18]: "Etwa einhundert Schritt in den Wald hinein entdeckt Ihr die Leiche eines alten Mannes.
			 * Neben Ihr liegt ein zerbrochener, grosser Hammer, so einer, wie man ihn zum Kaltschmieden von weichen
			 * Metallen benoetigt...@Wollt Ihr" */
			/* SECTION.LTX[51]: "den armen Kerl begraben" */
			/* SECTION.LTX[52]: "ihn seinem Schicksal ueberlassen" */
			r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[18], 2, g_tev_loc_txt[0x33], g_tev_loc_txt[0x34]);
			if (r == 1) {
				/* SECTION.LTX[53]: "Ihr vergrabt den Alten und macht Euch guten Gewissens wieder auf den Weg." */
				dlg_message(g_dialog, g_tev_loc_txt[0x35]);
				timewarp_hours(g_date, 2L);
				g_god_favor_boron += 40;
			} else {
				/* SECTION.LTX[54]: "Ihr lasst den Alten liegen; die Woelfe werden schon den Rest erledigen." */
				dlg_message(g_dialog, g_tev_loc_txt[0x36]);
				g_god_favor_boron -= 100;
				g_god_favor_rondra -= 40;
			}
		}
	}
}

void tevent_002(Travel far *o)
{
	signed short dummy;
	struct hero far *h;
	signed char r;
	Bmp far *p;

	if (o->from == 0x21 && g_tev_peddler_trail == 0) {
		p = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, p->data(), 0, 0x2f, 0, &dummy, &dummy, 0x140);
		/* SECTION.LTX[14]: "Euch kommt auf der Strasse ein Mann entgegen, der auf seinem Ruecken eine grosse Kiepe
		 * traegt. Bei jedem Schritt klappert und klirrt es maechtig, und sein gebeugter Ruecken zeigt deutlich, dass er
		 * ein schweres Gewicht mit sich herumtraegt. <Phex und alle anderen mit Euch!> ruft er Euch bereits auf eine
		 * grosse Entfernung lauthals entgegen." */
		dlg_box_bmp(g_dialog, p, (char far *)0, g_tev_loc_txt[14], 0);
		g_tev_peddler_trail = 1;
		delete p;
		travel_leave(o);
		if (conv_run(g_dialog2, 0x83))
			timewarp_minutes(g_date, 30L);
		mouse_hide(0);
		travel_enter(o);
		mouse_show(0);
		travelmap_draw(o);
		travel_redraw(o);
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x2f, 0, &dummy, &dummy, 0x140);
		/* SECTION.LTX[15]: "<Wollt Ihr vielleicht einen Kupferkessel kaufen?@Er soll nur 12 Silbertaler kosten!>" */
		/* ALL_SECT.LTX[1]: "Ja" */
		/* ALL_SECT.LTX[2]: "Nein" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[15], 2, g_tev_txt[1], g_tev_txt[2]);
		if (r == 1 && g_group->money >= 120)  {
			group_give_item(g_group, ITEM_COPPER_KETTLE, 1);
			group_sub_money(g_group, 120L);
		}
		if (r == 1 && g_group->money < 120)
			/* SECTION.LTX[16]: "Ihr habt leider nicht die noetigen 12 Silbertaler." */
			dlg_message(g_dialog, g_tev_loc_txt[16]);
		g_routes[2].event -= 2;
		g_routes[1].event -= 1;
	} else if (g_tev_peddler_trail == 1) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x30, 0, &dummy, &dummy, 0x140);
		h = get_first_active_hero(g_group);
		/* SECTION.LTX[17]: "%s entdeckt am Strassenrand einige Blutflecken und eine Schleifspur! Wollt Ihr ihr in das
		 * Unterholz hinein folgen?" */
		sprintf(g_text_output_buf, g_tev_loc_txt[17], h->name);
		r = dlg_choose(g_dialog, g_text_output_buf, 2, g_tev_txt[1], g_tev_txt[2]);
		if (r == 1) {
			g_tev_peddler_trail = 0;
			g_routes[2].event -= 1;
			timewarp_hours(g_date, 1L);
			/* SECTION.LTX[18]: "Etwa einhundert Schritt in den Wald hinein entdeckt Ihr die Leiche eines alten Mannes.
			 * Neben Ihr liegt ein zerbrochener, grosser Hammer, so einer, wie man ihn zum Kaltschmieden von weichen
			 * Metallen benoetigt...@Wollt Ihr" */
			/* SECTION.LTX[51]: "den armen Kerl begraben" */
			/* SECTION.LTX[52]: "ihn seinem Schicksal ueberlassen" */
			r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[18], 2, g_tev_loc_txt[0x33], g_tev_loc_txt[0x34]);
			if (r == 1) {
				/* SECTION.LTX[53]: "Ihr vergrabt den Alten und macht Euch guten Gewissens wieder auf den Weg." */
				dlg_message(g_dialog, g_tev_loc_txt[0x35]);
				timewarp_hours(g_date, 2L);
				g_god_favor_boron += 40;
			} else {
				/* SECTION.LTX[54]: "Ihr lasst den Alten liegen; die Woelfe werden schon den Rest erledigen." */
				dlg_message(g_dialog, g_tev_loc_txt[0x36]);
				g_god_favor_boron -= 100;
				g_god_favor_rondra -= 40;
			}
		}
	}
}

void tevent_003(Travel far *o)
{
	struct hero far *h;
	signed short res;
	signed char n;

	h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
	res = test_skill(h, TA_PFLANZENKUNDE, g_routes[3].event * 3 + 5);
	if (res > 0) {
		n = random_schweif(6);
		/* ALL_SECT.LTX[3]: "%s entdeckt am Wegesrand %d Portion%s Wirselkraut!" */
		/* ALL_SECT.LTX[5]: "en" */
		/* ALL_SECT.LTX[4]: "" */
		sprintf(g_text_output_buf, g_tev_txt[3], h->name, n, n > 1 ? g_tev_txt[5] : g_tev_txt[4]);
		dlg_message(g_dialog, g_text_output_buf);
		group_give_item(g_group, ITEM_WHIRLWEED, n);
	}
	if (g_routes[3].event < 40)
		g_routes[3].event++;
}

void tevent_004(Travel far *o)
{
	signed char x;

	if (o->from == 0x22 && travel_stage(o) == 2) {
		g_routes[4].event = g_routes[4].event + 1;
		g_routes[5].event = g_routes[5].event + 1;
		g_routes[30].event = g_routes[30].event + 1;
		g_fight_max_enemies = g_routes[4].event >= 0x38 ? 12 : g_routes[4].event * 2 - 100;
		travel_leave(o);
		g_fight_flag = 1;
		x = start_fight(2, 0);
		fight_result(o, x);
		travel_enter(o);
	}
}

void tevent_005(Travel far *o)
{
	signed char x;

	if (o->from == 0x24 && travel_stage(o) == 2) {
		g_routes[4].event = g_routes[4].event + 1;
		g_routes[5].event = g_routes[5].event + 1;
		g_routes[30].event = g_routes[30].event + 1;
		g_fight_max_enemies = g_routes[5].event >= 0x38 ? 12 : g_routes[5].event * 2 - 100;
		travel_leave(o);
		g_fight_flag = 1;
		x = start_fight(1, 0);
		fight_result(o, x);
		travel_enter(o);
	}
}

void tevent_006(Travel far *o)
{
	signed char r;
	signed char x;
	signed short dummy;
	Bmp far *p;
	signed char i;

	p = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x197, p->data(), 0, 0x29, 0, &dummy, &dummy, 0x140);
	/* SECTION.LTX[19]: "Als Ihr auf eine kleine Huegelkuppe kommt, seht Ihr etwas unter Euch, wie eine
	 * Halmar-Antilope gerade mit einem riesigen Satz aus dem Gebuesch bricht. Ohne von Euch Notiz zu nehmen, rennt
	 * sie auf Euch zu, um direkt vor Euren Fuessen zusammenzubrechen. Jetzt koennt Ihr auch erkennen, dass sie aus
	 * einer klaffenden Halswunde blutet. Sie roechelt noch einmal, dann ist es wieder still. Weit und breit seht Ihr
	 * nichts und niemanden. Wollt Ihr das Tier ausweiden und mitnehmen?" */
	/* ALL_SECT.LTX[1]: "Ja" */
	/* ALL_SECT.LTX[2]: "Nein" */
	r = dlg_box_bmp(g_dialog, p, (char far *)0, g_tev_loc_txt[19], 2, g_tev_txt[1], g_tev_txt[2]);
	delete p;
	if (r == 1) {
		if ((signed short)g_group->nr_heroes <= 3)
			g_fight_max_enemies = 2;
		else
			g_fight_max_enemies = 3;
		travel_leave(o);
		g_fight_flag = 1;
		if (o->from == 0x24)
			x = start_fight(0x2d, 0);
		else
			x = start_fight(0x2c, 0);
		x = fight_result(o, x);
		travel_enter(o);
		if (x == 0) {
			for (i = 0; i < 8; i++) {
				travel_redraw(o);
				timewarp_minutes(g_date, 15L);
			}
			group_give_item(g_group, ITEM_FOOD_PACKAGE, 15);
		}
	}
	g_routes[6].event -= 1;
}

void tevent_007(Travel far *o)
{
	signed char x;
	signed char flag = 0;
	signed char r = -1;
	struct hero far *h1;
	struct hero far *h2;
	signed short dummy;
	Bmp far *p;

	if (o->from == 0x25) {
		p = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, p->data(), 0, 0x2a, 0, &dummy, &dummy, 0x140);
		h1 = get_best_hero_skill(g_group, TA_SINNESSCHAERFE);
		/* SECTION.LTX[20]: "Ploetzlich bemerkt %s zwei Elfen, die nahezu reglos an einen Baum gelehnt stehen und Euch
		 * mustern. Sie tragen Langboegen in den Haenden, auf denen locker ein Pfeil liegt und mit denen sie Euren
		 * Bewegungen folgen. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[20], h1->name);
		while (r == -1)
			/* SECTION.LTX[21]: "Grusslos weitergehen?" */
			/* SECTION.LTX[22]: "Freundlich gruessen?" */
			/* SECTION.LTX[23]: "oder nach Euren Waffen greifen?" */
			r = dlg_box_bmp(g_dialog, p, (char far *)0, g_text_output_buf, 3, g_tev_loc_txt[21], g_tev_loc_txt[22], g_tev_loc_txt[23]);
		switch (r) {
		case 1:
			break;
		case 3:
			flag = 1;
			break;
		case 2:
			r = -1;
			while (r == -1)
				/* SECTION.LTX[24]: "Sie reagieren in keiner Weise auf Euren Gruss; mustern Euch nur weiterhin sehr
				 * eindringlich. Wollt Ihr nun" */
				/* SECTION.LTX[25]: "Achselzuckend weitergehen?" */
				/* SECTION.LTX[26]: "Etwas nachdruecklicher gruessen?" */
				/* SECTION.LTX[27]: "oder die arroganten Kerle Mores lehren?" */
				r = dlg_box_bmp(g_dialog, p, (char far *)0, g_tev_loc_txt[24], 3, g_tev_loc_txt[25], g_tev_loc_txt[26], g_tev_loc_txt[27]);
			switch (r) {
			case 1:
				break;
			case 2:
				h2 = get_first_active_hero(g_group);
				/* SECTION.LTX[28]: "Der Blick der beiden Elfen wird noch intensiver, und %s hat ploetzlich das Gefuehl
				 * voelliger Offenheit den beiden gegenueber. Dann verschwinden die Elfen mit einem leisen Rascheln im
				 * Unterholz." */
				sprintf(g_text_output_buf, g_tev_loc_txt[28], h2->name);
				dlg_box_bmp(g_dialog, p, (char far *)0, g_text_output_buf, 0);
				break;
			case 3:
				flag = 1;
				break;
			}
			break;
		}
		delete p;
		if (flag == 1) {
			travel_leave(o);
			g_fight_flag = 1;
			x = start_fight(0x2e, 0);
			fight_result(o, x);
			travel_enter(o);
		}
		g_routes[7].event -= 1;
	}
}

void tevent_008(Travel far *o)
{
	if (o->from == 0x26)
		guide_offer(o, 4);
	if (o->from == 2)
		guide_offer(o, 3);
	if (g_guide_won != 0)
		g_routes[8].event = 0;
}
