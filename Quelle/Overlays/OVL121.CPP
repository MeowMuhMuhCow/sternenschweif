/* OVL121 - Overlay O121 (Segment 0x21bd in SCHWEIF.EXE): Reiseereignisse der Wege 140..154 (tevent_run O211;
 * Reiseobjekt o, Ort o->from; Texte g_tev_loc_txt = SECTION3.LTX, g_tev_txt = ALL_SECT.LTX; Merker route.event in
 * g_routes; Bilder per nvfile_load(g_nvf_heads, ..) in ein Bmp 64x64).
 * tevent_140/tevent_153: Pflanzenkunde-Probe (route.event[3] bzw. [153] * 3 + 5) -> 2 Einbeeren (txt[34]) bzw. 3
 *   Tarnelen (txt[56]); Zaehler bis 41. tevent_141: Zyklus 1..4, bei 1 deer_hunt.
 * tevent_142: Steinschlag am Abgrund (W10 <= 3 oder Merker == 1, mehr als ein Held; Bild 0x19): der Schlechteste in
 *   Gefahrensinn stuerzt (txt[35]); mit Seil (g_slow4_on) gebremst (txt[36], 2W6 Schaden, LE bleibt ueber 3), sonst
 *   3W6+4 Schaden und Text je nach tot/lebendig (txt[37]/[38]); Merker 2; danach immer Tagebuch ALL_SECT[123].
 * tevent_143: Luchs auf dem Baum (Bildfolge 0x1d4, txt[40..47]): fliehen, Bogen schlagen, vorsichtig weiter -> der
 *   Schlechteste in Tierkunde wird angesprungen: CH -3 fuer zwei Tage, Schaden |LE - (W10 + 8)| bzw. LE - 4, 10 AP,
 *   Tierkunde-Steigerung, Merker 0.
 * tevent_144: Kampf 0x43/0x44 je Richtung. tevent_145: Geschepper der Ruestungen (group_has_special_item, Bild 0x0e,
 *   txt[49]), Merker 0. tevent_146: Zyklus 1..3, bei 1 kreisender Adler (Bild 0x24, Klang 0xa1, txt[50]).
 * tevent_147/148/149/151: steht noch Merker [145], tevent_145; liegt die Lawine noch (g_tev147_rockslide ..
 *   g_tev151_rockslide) rockslide_dig (freiraeumen); sonst avalanche (Lawine durch das Ruestungsgeklirr, beim zweiten
 *   Mal
 *   Kampf), g_tev_avalanche_done einmalig, Merker 0 beim zweiten Mal.
 * tevent_152: Haengebruecke unter Beschuss (Bild 0x25, txt[53..55]): umkehren bzw. hinueberrennen (Kampf 0x45/0x46).
 * tevent_154: random_encounter (erstes Mal W6 + 4, sonst zu 60 % W6 + 2).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL121.CPP   -> 3114/3114 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; group_has_special_item hat keine Parameter, das
 *   Original uebergibt o - Aufruf ueber gecasteten Funktionszeiger)
 * - C++: lokale Bmp-Objekte in inneren Bloecken (Bmp(&b, 64, 64), Destruktor (&b, 2) am Blockende bzw. vor return),
 *   "new Bmp"/"delete b" in tevent_152; abs() aus stdlib.h ist in C++ eine inline-Funktion -> Parameterkopie [bp-0ch]
 *   vor cwd/xor/sub; Bitfeld h->dead ueber hero::is_dead() (unsigned char).
 * - -Og noetig: Zwischenwerte (int)le/(int)roll [bp-0ah]/[bp-0ch] in tevent_142.
 * - "le = h->get_le()" (inline-Getter auf int, Ziel signed char) -> mov ax,[bx+72h] / mov [bp-8],al; direkt
 *   "le = h->le" ergibt mov al.
 * - "h->is_dead() == 0 ? a : b" -> and ax,1 / or al,al / jnz; "!h->dead ? a : b" ergibt mov ah,0 / or ax,ax.
 * - NEU: "break;" als letzte Anweisung des letzten case am Funktionsende erzeugt einen toten Epilog (tevent_143);
 *   ohne dieses break (tevent_152) nicht. Spruenge ans Funktionsende werden durch Epilogkopien ersetzt (auch ohne
 *   return).
 * - o->from unsigned char (Vergleich mit 0x81).
 * Originalfehler: tevent_142 benutzt h (Name im Tagebuch ALL_SECT[123]) uninitialisiert, wenn der Steinschlag
 *   nicht stattfindet (W10 > 3 und Merker != 1, oder nur ein Held).
 */
#include <stdio.h>
#include <stdlib.h>

#include "schweif.h"

void tevent_140(Travel far *o)
{
	struct hero far *h;
	signed short r;

	h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
	r = test_skill(h, TA_PFLANZENKUNDE, g_routes[3].event * 3 + 5);
	if (r > 0) {
		/* SECTION3.LTX[34]: "%s entdeckt am Wegesrand %d Einbeeren!" */
		sprintf(g_text_output_buf, g_tev_loc_txt[34], h->name, 2);
		dlg_message(g_dialog, g_text_output_buf);
		group_give_item(g_group, ITEM_EINBEERE, 2);
	}
	if (g_routes[140].event <= 40)
		g_routes[140].event++;
}

void tevent_141(Travel far *o)
{
	if (g_routes[141].event == 1) {
		deer_hunt(o);
		g_routes[141].event = g_routes[141].event + 1;
		return;
	}
	if (g_routes[141].event >= 4) {
		g_routes[141].event = 1;
		return;
	}
	g_routes[141].event = g_routes[141].event + 1;
}

void tevent_142(Travel far *o)
{
	signed short dummy;
	struct hero far *h;
	signed char roll;
	signed char le;

	roll = random_schweif(10);
	if ((roll <= 3 || g_routes[142].event == 1) && group_count_check1(g_group) > 1) {
		Bmp b(0x40, 0x40);

		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x19, 0, &dummy, &dummy, 0x140);
		h = get_worst_hero_skill(g_group, TA_GEFAHRENSINN);
		/* SECTION3.LTX[35]: "Ploetzlich lockert sich ein Stein unter den Fuessen von %s, und %s stuerzt in die Tiefe!" */
		sprintf(g_text_output_buf, g_tev_loc_txt[35], h->name, grammar_pronoun(h->sex, 0, 0));
		dlg_message(g_dialog, g_text_output_buf);
		if (g_slow4_on == 1) {
			/* SECTION3.LTX[36]: "Obwohl das Seil nach kurzer Strecke %s Sturz bremst, schlaegt %s mehrfach heftig auf.
			 * Trotzdem, das haette schlimmer kommen koennen." */
			sprintf(g_text_output_buf, g_tev_loc_txt[36], grammar_pronoun(h->sex, 3, 1), h->name);
			dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
			le = h->get_le();
			roll = dice_roll(2, 6, 0);
			if (le - roll <= 3)
				sub_hero_le(h, le - 3);
			else
				sub_hero_le(h, roll);
		} else {
			sub_hero_le(h, dice_roll(3, 6, 4));
			/* SECTION3.LTX[37]: "Nichts haelt den Fall auf, und erst gut ein Dutzend Schritt unter Euch kommt %s mit einem
			 * Aufschrei und einem seltsam verdrehten Bein zur Ruhe.@%s" */
			/* SECTION3.LTX[38]: "Mit viel Muehe bugsiert Ihr das Opfer wieder auf den Pfad zurueck, aber das Bein ist
			 * gebrochen." */
			sprintf(g_text_output_buf, g_tev_loc_txt[37], h->name, h->is_dead() == 0 ? g_tev_loc_txt[38] : grammar_pronoun(h->sex, 2, 0));
			dlg_message(g_dialog, g_text_output_buf);
		}
#if FIX_0037
		/* Fix #37: Tagebucheintrag nur nach einem Absturz und nur, wenn der Held ueberlebt hat */
		g_routes[142].event = 2;
		if (h->is_dead() == 0) {
			sprintf(g_text_output_buf, g_tev_txt[123], h->name);
			diary_add(g_diary, g_text_output_buf, 0);
		}
	}
#else
		g_routes[142].event = 2;
	}
	/* ALL_SECT.LTX[123]: "%s ist bei einem Absturz nur knapp dem Tode entgangen." */
	sprintf(g_text_output_buf, g_tev_txt[123], h->name);
	diary_add(g_diary, g_text_output_buf, 0);
#endif
}

void tevent_143(Travel far *o)
{
	signed char sel = -1;
	signed char le;
	void far *win;
	struct hero far *h;

	win = Anim_ctor(0, 0x1d4, 0, 0);
	while (sel == -1)
		/* SECTION3.LTX[40]: "Ihr entdeckt einen Luchs, der in beobachtender Position auf einem Baum sitzt. Wollt Ihr?" */
		/* SECTION3.LTX[41]: "Unheldisch fliehen," */
		/* SECTION3.LTX[42]: "einen grossen Bogen um diesen Baum machen, oder" */
		/* SECTION3.LTX[43]: "vorsichtig weitergehen?" */
		sel = dlg_box_anim(g_dialog, win, (char far *)0, g_tev_loc_txt[40], 3, g_tev_loc_txt[41], g_tev_loc_txt[42], g_tev_loc_txt[43]);
	Anim_dtor(win, 3);
	switch (sel) {
	case 1:
		/* SECTION3.LTX[44]: "Die Flucht ist geglueckt, obwohl da ja auch nichts Aufregendes dabei war." */
		dlg_message(g_dialog, g_tev_loc_txt[44]);
		break;
	case 2:
		/* SECTION3.LTX[45]: "Ihr geht etwa 15 Schritt entfernt an dem Baum vorbei und beobachtet, wie der Luchs es sich
		 * auf dem Baum gemuetlich macht. Er scheint ueberhaupt kein Interesse an Euch zu haben." */
		dlg_message(g_dialog, g_tev_loc_txt[45]);
		break;
	case 3:
		g_routes[143].event = 0;
		h = get_worst_hero_skill(g_group, TA_TIERKUNDE);
		/* SECTION3.LTX[46]: "Als Ihr etwa einen Schritt von dem Baum entfernt seid, springt der Luchs auf %s und
		 * verletzt %s schwer im Gesicht. Ein zweiter Luchs springt aus dem Unterholz und greift auch %s an. Ein wilder
		 * Kampf entbrennt." */
		sprintf(g_text_output_buf, g_tev_loc_txt[46], h->name, grammar_pronoun(h->sex, 2, 0), h->name);
		dlg_message(g_dialog, g_text_output_buf);
		add_effect(h, get_free_effect(h), 2 * TICKS_PER_DAY, ATTRIB_CH, -3, EFFECT_ATTRIB);
		le = h->get_le();
		if (le > 10)
			sub_hero_le(h, abs(le - (random_schweif(10) + 8)));
		else
			sub_hero_le(h, le - 4);
		add_hero_ap(h, 10);
		hero_inc_skill(h, TA_TIERKUNDE);
		/* SECTION3.LTX[47]: "Als es Euch jedoch gelingt einen der Luchse zu toeten, flieht der Zweite." */
		dlg_message(g_dialog, g_tev_loc_txt[47]);
		break;
	}
}

void tevent_144(Travel far *o)
{
	signed char r;

	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x7b)
		r = start_fight(0x43, 0);
	else
		r = start_fight(0x44, 0);
	fight_result(o, r);
	travel_enter(o);
	if (r == 0)
		g_routes[144].event = 0;
}

void tevent_145(Travel far *o)
{
	signed short dummy;

	if (((signed char (*)(Travel far *))group_has_special_item)(o) >= 0) {
		Bmp b(0x40, 0x40);

		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x0e, 0, &dummy, &dummy, 0x140);
		/* SECTION3.LTX[49]: "Das Geschepper Eurer Ruestungen klingt an machen Stellen dieses engen, dunklen Tal als
		 * beaengstigend laut; manchmal hat es den Anschein, als waere das Echo  noch lauter als der urspruengliche Ton.
		 * Sonst jedoch ist kein Laut zu hoeren ausser dem leisen Knistern der Berge..." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[49], 0);
		g_routes[145].event = 0;
	}
}

void tevent_146(Travel far *o)
{
	signed short dummy;

	if (g_routes[146].event == 1) {
		Bmp b(0x40, 0x40);

		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x24, 0, &dummy, &dummy, 0x140);
		snd_play(g_snd_fx, 0xa1);
		/* SECTION3.LTX[50]: "Einsam kreist ueber Euch am Himmel majestaetisch ein Adler. Neidisch koennte man werden
		 * angesichts der muehseligen Kraxelei, wie leicht der Vogel an Hoehe gewinnt. Aber es beunruhigt Euch eventuell
		 * dennoch ein wenig, dass dieser Koenig der Luefte schon seit laengerer Zeit ueber Euch kreist, eigentlich schon
		 * den ganzen Tag. Aber da Ihr ohnehin nichts aendern koennt, koennt Ihr ihn auch genausogut ignorieren." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[50], 0);
		return;
	}
	if (g_routes[146].event >= 3) {
		g_routes[146].event = 1;
		return;
	}
	g_routes[146].event = g_routes[146].event + 1;
}

void tevent_147(Travel far *o)
{
	if (g_routes[145].event != 0) {
		tevent_145(o);
		return;
	}
	if (g_tev147_rockslide != 0) {
		g_tev147_rockslide = rockslide_dig(o);
		if (!g_tev147_rockslide)
			g_routes[147].event = 0;
		return;
	}
	avalanche(o);
	if (g_tev_avalanche_done == 1)
		g_routes[147].event = 0;
	else
		g_tev_avalanche_done = 1;
	g_tev147_rockslide = 1;
}

void tevent_148(Travel far *o)
{
	if (g_routes[145].event != 0) {
		tevent_145(o);
		return;
	}
	if (g_tev148_rockslide != 0) {
		g_tev148_rockslide = rockslide_dig(o);
		if (!g_tev148_rockslide)
			g_routes[148].event = 0;
		return;
	}
	avalanche(o);
	if (g_tev_avalanche_done == 1)
		g_routes[148].event = 0;
	else
		g_tev_avalanche_done = 1;
	g_tev148_rockslide = 1;
}

void tevent_149(Travel far *o)
{
	if (g_routes[145].event != 0) {
		tevent_145(o);
		return;
	}
	if (g_tev149_rockslide != 0) {
		g_tev149_rockslide = rockslide_dig(o);
		if (!g_tev149_rockslide)
			g_routes[149].event = 0;
		return;
	}
	avalanche(o);
	if (g_tev_avalanche_done == 1)
		g_routes[149].event = 0;
	else
		g_tev_avalanche_done = 1;
	g_tev149_rockslide = 1;
}

void tevent_151(Travel far *o)
{
	if (g_routes[145].event != 0) {
		tevent_145(o);
		return;
	}
	if (g_tev151_rockslide != 0) {
		g_tev151_rockslide = rockslide_dig(o);
		if (!g_tev151_rockslide)
			g_routes[151].event = 0;
		return;
	}
	avalanche(o);
	if (g_tev_avalanche_done == 1)
		g_routes[151].event = 0;
	else
		g_tev_avalanche_done = 1;
	g_tev151_rockslide = 1;
}

void tevent_152(Travel far *o)
{
	signed char sel;
	signed char r;
	signed short dummy;
	Bmp far *b;

	b = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x197, b->data(), 0, 0x25, 0, &dummy, &dummy, 0x140);
	/* SECTION3.LTX[53]: "Vor Euch spannt sich eine stolze, kuehne Haengebruecke ueber eine Schlucht, an deren Grund
	 * Ihr einen weissschaeumenden Gebirgsbach zu Tale rasen sehen koennt. Erstaunlicherweise macht die Bruecke einen
	 * sehr gepflegten und soliden Eindruck. Vielleicht haben die Orks dafuer gesorgt, die auf der anderen Seite
	 * stehen und laessig ihre Kurzboegen in der Hand halten. Wollt Ihr" */
	/* SECTION3.LTX[54]: "das Feld raeumen und es lieber andersherum versuchen oder" */
	/* SECTION3.LTX[55]: "ueber die Bruecke rennen und einige Pfeilschuesse riskieren?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[53], 2, g_tev_loc_txt[54], g_tev_loc_txt[55]);
	delete b;
	if (sel == -1)
		sel = 1;
	switch (sel) {
	case 1:
		o->action = 1;
		o->ev_half = 0;
		break;
	case 2:
		travel_leave(o);
		g_fight_flag = 1;
		if (o->from == 0x7e)
			r = start_fight(0x45, 1);
		else
			r = start_fight(0x46, 1);
		fight_result(o, r);
		travel_enter(o);
	}
}

void tevent_153(Travel far *o)
{
	struct hero far *h;
	signed short r;

	h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
	r = test_skill(h, TA_PFLANZENKUNDE, g_routes[153].event * 3 + 5);
	if (r > 0) {
		/* SECTION3.LTX[56]: "%s entdeckt am Wegesrand %d Tarnelen!" */
		sprintf(g_text_output_buf, g_tev_loc_txt[56], h->name, 3);
		dlg_message(g_dialog, g_text_output_buf);
		group_give_item(g_group, ITEM_TARNELE, 3);
	}
	if (g_routes[153].event <= 40)
		g_routes[153].event++;
}

void tevent_154(Travel far *o)
{
	signed char n = 0;

	if (g_routes[154].event == 1) {
		g_routes[154].event++;
		n = random_schweif(6) + 4;
		if (o->from == 0x81)
			random_encounter(o, n, 3);
		else
			random_encounter(o, n, 4);
		return;
	}
	n = random_schweif(10);
	if (n <= 5) {
		n = random_schweif(6) + 2;
		if (o->from == 0x2e)
			random_encounter(o, n, 3);
		else
			random_encounter(o, n, 4);
	}
}
