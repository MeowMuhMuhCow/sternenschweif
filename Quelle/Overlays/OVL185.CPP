/* OVL185 - Overlay O185 (Segment 0x22f8 in SCHWEIF.EXE): Ereignisse in der belagerten Stadt Lowangen (Stadt 4; Aufrufe
 * aus dem Stadtkarten-Handler O186 und O046): Ankunft mit Antharon und dem Salamanderstein, Strassenbanden und
 * Stadtgarde (Gespraech L_STREET.TLK = Datei 0x2e4), Verfolgung Gavrons.
 * lowangen_arrival(a): nur wenn g_low_arrival_due (nach dem Orklager O163 bzw. O031): Texte NAKED.LTX (Datei 0x44, 45
 *   Eintraege per txt_load_list), Bilder Gavron (NVF 10 Nr. 0xd8) und Antharon (NVF 5 Nr. 0x81) je 32x32. a == 0 am
 *   Stadttor [5] (Neugierige umringen die Gruppe), a == 1 [37]. Mit dem Salamanderstein (Gegenstand 0x117, noch nicht
 *   abgegeben g_low_stone_delivered): Steine 0x117/0xfc abgeben, Merker g_low_stone_delivered/g_topic_gavron/g_low_vindaria_known/g_low_stone_days = 1; ist Antharon
 *   (NSC 6) dabei: lebend Abschied [6..10] und [11] "Und mit ihm der Salamanderstein!", tot Gavron [13..17], dann
 *   verlaesst der NSC die Gruppe (g_npc_months = 99, groups_check_npc). Ohne NSC je nach Antharons Zustand g_npc6_met
 *   (-1, 1, -2, 0) Gavron mit Fragen nach seinem Bruder [18..33]. Ohne Stein nur der Abschied Antharons [6, 12, 9]
 *   bzw. Gavron mit dem toten Bruder [13..17].
 * lowangen_food_thugs(): um 14 Uhr auf der Strasse (Stadt 4, loctype 0): viele Proviantpakete im Gepaeck (5/4/3/2 je
 *   Held, sicher bzw. 75/50/25 %) -> W15 + 5 Verfolger g_street_thugs, Gespraech L_STREET 0 ("Los, her mit dem
 *   Futter!"); 1 = Ereignis bzw. nicht auf der Strasse, 0 = keins.
 * lowangen_weapon_check(): sonst viele Waffen im Gepaeck (25/20/15/10 Stueck, sicher bzw. 75/50/25 %) -> Patrouille
 *   von W4 + 3 Gardisten g_street_guards, Gespraech L_STREET 1 ("Zeigt doch mal, was Ihr so an Waffen dabei habt.").
 * lowangen_drop_food(): alle Proviantpakete (Rucksackplaetze) ablegen (L_STREET: Lebensmittel herausruecken).
 * Ev::gavron_chase() (aus O046, Texte LOWANGEN.LTX[89..110] des Ev-Objekts): Verfolgung Gavrons ueber mehrere Felder,
 *   Zustand g_gavron_chase 1..8 - einholen oder vorsichtig folgen, ihn stellen und befragen (g_vindaria_state = 1: "Den
 *   Salamanderstein habe ich bei Vindaria abgeliefert"), Gassenwissen-Probe +5 des Hintersten bzw. Schleichen +2 aller
 *   Helden; wer den Anschluss verliert, bildet eine eigene Gruppe (make_group); am Ende verschwindet Gavron in einem
 *   Haus (Zustand -4, g_low_ailian_state = 3).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL185.CPP   -> 5382/5382 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - erster Wurf passte: -Oi fuer strcpy/strcat-Intrinsics (Quelle von strcpy zweimal geladen, entsteht von selbst).
 * - lowangen_arrival: "} else ;" hinter dem inneren if im else-Zweig noetig (toter jmp short), sonst 46 Bytes anders.
 *   switch g_npc6_met: case 1 faellt in case -2 durch (Sprungtabelle -2/-1/0/1 -> 06bd/046c/071b/04d8);
 *   Unter-switch "case -1: case 1:" teilen einen Rumpf, Wert 0 laeuft ins Ende.
 * - lowangen_food_thugs: Gruppengroesse als -Og-Zwischenwert [bp-0ah], nach jedem random_schweif neu geladen
 *   (gemeinsame Teilausdruecke werden durch Aufrufe ungueltig) - einfach viermal g_group->nr_heroes schreiben.
 * - gavron_chase: eine register-Variable (SI) fuer zwei Zwecke (Richtung in case 2..6, Zaehler in case 7).
 * Originalfehler: lowangen_arrival - NAKED.LTX hat nur 35 Texte, txt[37] (a == 1) liegt hinter dem Ende; case 1 -
 *   strcpy(txt[13]) wird sofort von strcpy(txt[18]) ueberschrieben (gemeint war wohl strcat); gavron_chase - die
 *   Richtung k (5/3/1/-1) in case 2/3/4/6 wird gesetzt, aber nie benutzt; r ist uninitialisiert, falls
 *   g_gavron_chase weder 1 noch 5 ist (durch den case unmoeglich).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

#define FILE_NAKED	0x44		/* NAKED.LTX (Ankunft in Lowangen) */
#define TALK_L_STREET	0x2e4		/* L_STREET.TLK: Strassenbande (0) und Stadtgarde (1) */
#define NPC_ANTHARON	6		/* NSC 6: Antharon, Bruder Gavrons */

void lowangen_arrival(signed short a)
{
	void far *txt_buf;
	char far * far *txt;
	Bmp far *gavron;
	Bmp far *antharon;
	struct hero far *npc;
	signed short r;
	signed short dummy;
	struct hero far *h2;

	npc = group_get_npc(g_group);
	if (g_low_arrival_due != 0) {
		gavron = new Bmp(0x20, 0x20);
		nvfile_load(g_nvf_heads, 0x0a, gavron->data(), (void far *)0, 0xd8, 0, &dummy, &dummy, 0x140);
		antharon = new Bmp(0x20, 0x20);
		nvfile_load(g_nvf_heads, 0x05, antharon->data(), (void far *)0, 0x81, 0, &dummy, &dummy, 0x140);
		g_low_arrival_due = 0;
		txt_load_list(g_font, FILE_NAKED, 0x2d, &txt_buf, &txt);
		if (a == 0)
			/* NAKED.LTX[5]: "Kaum habt Ihr die Stadt erreicht, da seid Ihr schon von Neugierigen umringt.@Offenbar sind
			 * schon laengere Zeit keine Reisenden mehr in Lowangen angekommen." */
			dlg_message(g_dialog, txt[5]);
		else
			/* NAKED.LTX[37]: gibt es nicht (Datei mit 35 Texten) - Originalfehler */
#if FIX_0216
			/* Fix #216: NAKED.LTX hat 35 Texte: der sonst unbenutzte Text 34 ("Wie aus dem Boden gewachsen steht ploetzlich eine Gestalt vor Euch.") passt zum Gavron-Bild (vermutlich 0x22 statt 0x25) */
			dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[0x22], 0);
#else
			dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[0x25], 0);
#endif

		if (group_find_item(g_group, ITEM_SALAMANDER_STONE) != -1 && !g_low_stone_delivered) {
			g_low_stone_delivered = g_topic_gavron = g_low_vindaria_known = 1;
			g_low_stone_days = 1;
			group_remove_item(g_group, ITEM_SALAMANDER_STONE);
			group_remove_item(g_group, ITEM_SALAMANDER_STONE_FAKE);
			if (npc != 0 && npc->npc_id == NPC_ANTHARON) {
				if (check_hero(npc)) {
					/* NAKED.LTX[6]: "<Habt Dank Freunde, fuer die Begleitung durch unsicheres Gebiet" */
					strcpy(g_text_output_buf, txt[6]);
					/* GLOBLTXT.LTX[449]: ";" */
					strcat(g_text_output_buf, g_ttx[0x1c1]);
					/* NAKED.LTX[7]: "dort hinten kommt gerade mein Bruder Gavron." */
					strcat(g_text_output_buf, txt[7]);
					/* NAKED.LTX[8]: "Die Zeit zur Verabschiedung ist gekommen." */
					strcat(g_text_output_buf, txt[8]);
					/* GLOBLTXT.LTX[423]: "." */
					strcat(g_text_output_buf, g_ttx[0x1a7]);
					/* NAKED.LTX[9]: "@Wenn Ihr einmal Hilfe braucht, koennt Ihr Euch an Ihn wenden, der kennt sich sehr gut aus
					 * in Lowangen. Ich selbst werde die Stadt schon bald wieder verlassen.>" */
					strcat(g_text_output_buf, txt[9]);
					dlg_box_bmp(g_dialog, antharon, (char far *)0, g_text_output_buf, 0);
					/* NAKED.LTX[10]: "Da tritt auch schon ein junger Mann heran, der Antharon zum Verwechseln aehnlich sieht und
					 * Ihn umarmt.@Schliesslich wendet er sich Euch zu:@<Habt Dank Reisende fuer das sichere Geleit, das Ihr
					 * meinem Bruder gewaehrt habt.@Wir stehen tief in Eurer Schuld und werden Euch gerne auch einmal zu Diensten
					 * sein.@Um seinen Dank auszudruecken umarmt er Euch alle einmal und verschwindet dann mit Antharon im
					 * Getuemmel der Schaulustigen." */
					dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[10], 0);
					/* NAKED.LTX[11]: "Und mit ihm der Salamanderstein!" */
					dlg_message(g_dialog, txt[11]);
				} else {
					/* NAKED.LTX[13]: "Da schaelt sich ein schwarzhaariger Kerl aus der Menge und kommt auf Euch zugelaufen." */
					strcpy(g_text_output_buf, txt[13]);
					/* NAKED.LTX[14]: "@<Mein Gott!>@ruft er schon von Weitem.@<Was ist mit meinem Bruder Antharon passiert!>" */
					strcat(g_text_output_buf, txt[14]);
					/* NAKED.LTX[16]: "<Er war fuer uns ein Klotz am Bein.>" */
					/* NAKED.LTX[17]: "<Wir konnten ihm nicht helfen.>" */
					dlg_box_bmp(g_dialog, gavron, (char far *)0, g_text_output_buf, 2, txt[16],
						txt[17]);
					/* NAKED.LTX[15]: "<Ihr seid fuer seinen Tod verantwortlich und ich, Gavron, werde das nie vergessen.>@Er
					 * nimmt seinen Bruder auf die Schultern und laeuft mit Ihm davon; schon bald ist er in dem Getuemmel
					 * verschwunden." */
					dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[15], 0);
					dlg_message(g_dialog, txt[11]);
					g_low_stone_delivered = -2;
					g_npc6_met = -1;
				}
				g_npc_months = 0x63;
				groups_check_npc();
			} else {
				switch (g_npc6_met) {
				case -1:
					/* NAKED.LTX[26]: "<Ihr muesst die Helden sein, die Antharon hierhergeleitet haben;@ich bin Gavron sein
					 * Bruder.>" */
					dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[0x1a], 0);
					/* NAKED.LTX[27]: "<Ihr habt Antharon nicht zufaellig noch einmal getroffen? Er ist schon wieder
					 * verschwunden.>" */
					dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[0x1b], 0);
					/* NAKED.LTX[28]: "Nachdem Ihr wahrheitsgemaess verneint habt, verschwindet Gavron durch Eure Mitte in der
					 * Menge." */
					dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[0x1c], 0);
					break;
				case 1:
					strcpy(g_text_output_buf, txt[13]);
					/* NAKED.LTX[18]: "@<Ich bin Gavron, der Bruder von Antharon.@Mir wurde mitgeteilt, dass Ihr ihn getroffen
					 * habt.@Koennt Ihr mir Neuigkeiten erzaehlen?>" */
#if FIX_0149
					/* Fix #149: txt[18] (beginnt mit @) an txt[13] anhaengen statt ueberschreiben */
					strcat(g_text_output_buf, txt[18]);
#else
					strcpy(g_text_output_buf, txt[18]);
#endif
					/* NAKED.LTX[19]: "<Wir haben ihn nur einmal gesehen.>" */
					/* NAKED.LTX[20]: "<Er muss wohl gestorben sein.>" */
					/* NAKED.LTX[21]: "<Er muesste auch bald in Lowangen eintreffen.>" */
					r = dlg_box_bmp(g_dialog, gavron, (char far *)0, g_text_output_buf, 3,
						txt[19], txt[20], txt[21]);
					switch (r) {
					case -1:
					case 1:
						/* NAKED.LTX[23]: "<Schade, ich hoffte endlich wieder etwas von ihm zu hoeren.@Ich wuensche Euch weiterhin
						 * Glueck auf Euren Wegen.>" */
						strcpy(g_text_output_buf, txt[23]);
						break;
					case 2:
						/* NAKED.LTX[24]: "<Oh nein, das kann doch nicht sein.>@Er ist den Traenen nahe." */
						strcpy(g_text_output_buf, txt[24]);
						break;
					case 3:
						/* NAKED.LTX[25]: "<Endlich wieder gute Nachrichten und neue Hoffnung, dass er noch am Leben ist.@Habt
						 * Dank!>" */
						strcpy(g_text_output_buf, txt[25]);
						break;
					}
					/* NAKED.LTX[22]: "@Gavron bahnt sich den Weg zwischen Euch hindurch und verschwindet in der Menge." */
					strcat(g_text_output_buf, txt[22]);
					dlg_box_bmp(g_dialog, gavron, (char far *)0, g_text_output_buf, 0);
				case -2:
					h2 = get_random_hero(g_group);
					/* NAKED.LTX[29]: "Ihr zwaengt Euch durch das Getuemmel der Neugierigen.@Da bemerkt %s, wie Gavron unter den
					 * Umstehenden verschwindet." */
					sprintf(g_text_output_buf, txt[0x1d], h2->name);
					dlg_box_bmp(g_dialog, gavron, (char far *)0, g_text_output_buf, 0);
					break;
				case 0:
					dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[13], 0);
					/* NAKED.LTX[30]: "<Halt Fremde, einen Augenblick bitte. Mein Name ist Gavron.@Sagt habt Ihr waehrend Eurer
					 * Reise vielleicht meinen Bruder Antharon gesehen? Er sieht mir sehr aehnlich!>" */
					/* GLOBLTXT.LTX[446]: "Ja" */
					/* GLOBLTXT.LTX[447]: "Nein" */
					r = dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[0x1e], 2, g_ttx[0x1be],
						g_ttx[0x1bf]);
					if (r == 1) {
						/* NAKED.LTX[31]: "<Oh Neuigkeiten; lasst hoeren!>" */
						r = dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[0x1f], 3,
							txt[19], txt[20], txt[21]);
						switch (r) {
						case -1:
						case 1:
							strcpy(g_text_output_buf, txt[23]);
							break;
						case 2:
							strcpy(g_text_output_buf, txt[24]);
							break;
						case 3:
							strcpy(g_text_output_buf, txt[25]);
							break;
						}
						strcat(g_text_output_buf, txt[22]);
						dlg_box_bmp(g_dialog, gavron, (char far *)0, g_text_output_buf, 0);
					} else {
						/* NAKED.LTX[32]: "<Schade, ich habe gehofft endlich wieder von ihm zu hoeren.>" */
						dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[0x20], 0);
						/* NAKED.LTX[33]: "Nach diesen Worten verschwindet Gavron durch Eure Mitte in der Menge." */
						dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[0x21], 0);
					}
					break;
				}
				dlg_message(g_dialog, txt[11]);
			}
		} else {
			if (npc != 0 && npc->npc_id == NPC_ANTHARON) {
				if (check_hero(npc)) {
					g_npc_months = 0x63;
					groups_check_npc();
					strcpy(g_text_output_buf, txt[6]);
					strcat(g_text_output_buf, g_ttx[0x1c1]);
					/* NAKED.LTX[12]: "ich werde jetzt meinen Bruder besuchen." */
					strcat(g_text_output_buf, txt[12]);
					strcat(g_text_output_buf, g_ttx[0x1a7]);
					strcat(g_text_output_buf, txt[9]);
					dlg_box_bmp(g_dialog, antharon, (char far *)0, g_text_output_buf, 0);
					g_npc6_met = -1;
				} else {
					g_npc_months = 0x63;
					groups_check_npc();
					strcpy(g_text_output_buf, txt[13]);
					strcat(g_text_output_buf, txt[14]);
					dlg_box_bmp(g_dialog, gavron, (char far *)0, g_text_output_buf, 2, txt[16],
						txt[17]);
					dlg_box_bmp(g_dialog, gavron, (char far *)0, txt[15], 0);
					g_npc6_met = -2;
				}
			} else ;
		}
		mem_free(txt_buf);
		mem_free(txt);
		delete antharon;
		delete gavron;
	}
}

signed short lowangen_food_thugs(void)
{
	signed short sum = 0;
	struct hero far *h;
	signed short res;
	register signed short i;

	h = get_first_hero(g_group);
	res = 0;
	if (g_group->town == TOWN_LOWANGEN && !g_group->loctype) {
		do {
			for (i = 15; i < 31; i++) {
				if (h->inv[i].item_id == ITEM_FOOD_PACKAGE)
					sum += h->inv[i].quantity;
			}
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
		if (g_group->nr_heroes * 5 <= sum ||
		    (g_group->nr_heroes * 4 == sum && random_schweif(100) <= 75) ||
		    (g_group->nr_heroes * 3 == sum && random_schweif(100) <= 50) ||
		    (g_group->nr_heroes * 2 == sum && random_schweif(100) <= 25)) {
			g_street_thugs = random_schweif(15) + 5;
			talk(&g_talk, TALK_L_STREET, 0, 1);
			res = 1;
		}
	} else {
		res = 1;
	}
	return res;
}

void lowangen_weapon_check(void)
{
	struct hero far *h;
	signed short cnt;
	register signed short i;
	register signed short id;

	h = get_first_hero(g_group);
	cnt = 0;
	if (g_group->town == TOWN_LOWANGEN && !g_group->loctype) {
		do {
			for (i = 15; i < 31; i++) {
				id = h->inv[i].item_id;
				if (id != 0 && g_items[id].weapon)
					cnt++;
			}
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
		if (cnt >= 25 ||
		    (cnt >= 20 && random_schweif(100) <= 75) ||
		    (cnt >= 15 && random_schweif(100) <= 50) ||
		    (cnt >= 10 && random_schweif(100) <= 25)) {
			g_street_flag1 = g_street_flag2 = 0;
			g_street_guards = random_schweif(4) + 3;
			talk(&g_talk, TALK_L_STREET, 1, 1);
		}
	}
}

void lowangen_drop_food(void)
{
	struct hero far *h;
	register signed short i;

	h = get_first_hero(g_group);
	do {
		for (i = 15; i < 31; i++) {
			if (h->inv[i].item_id == ITEM_FOOD_PACKAGE)
				drop_item(h, i, h->inv[i].quantity);
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
}

void Ev::gavron_chase()
{
	signed short old;
	signed short r;
	signed short n;
	struct hero far *npc;
	struct hero far *m;
	register signed short k;
	register signed short j;

	old = dlg_set16(g_dialog, 5);
	switch (g_gavron_chase) {
	case 1:
	case 5:
		if (g_gavron_chase == 1) {
			group_set_screen_pos(8, 0x19);
			g_gavron_chase = 2;
			/* LOWANGEN.LTX[89]: "Ihr seht Gavron gerade noch um die Ecke verschwinden.@Wollt Ihr" */
			/* LOWANGEN.LTX[90]: "versuchen Gavron einzuholen?" */
			/* LOWANGEN.LTX[91]: "oder wollt Ihr ihn vorsichtig verfolgen?" */
			r = dlg_choose(g_dialog, txt[0x59], 2, txt[0x5a], txt[0x5b]);
		} else if (g_gavron_chase == 5) {
			group_set_screen_pos(0x0d, 6);
			g_gavron_chase = 6;
			/* LOWANGEN.LTX[94]: "Ploetzlich legt Gavron schnellere Schritte ein. Wollt Ihr" */
			r = dlg_choose(g_dialog, txt[0x5e], 2, txt[0x5a], txt[0x5b]);
		}
		if (r == 1) {
			/* LOWANGEN.LTX[92]: "Gavron ist flink wie ein Wiesel, aber Ihr seid schnell genug, um nachzusetzen. Ein paar
			 * Gassen weiter stellt Ihr den Burschen." */
			dlg_message(g_dialog, txt[0x5c]);
			/* LOWANGEN.LTX[96]: "<Heh! Was soll das? Was wollt Ihr von mir?>" */
			/* LOWANGEN.LTX[97]: "<Das weisst du ganz genau!>" */
			/* LOWANGEN.LTX[98]: "<Wo ist der Salamanderstein?>" */
			r = dlg_choose(g_dialog, txt[0x60], 2, txt[0x61], txt[0x62]);
			if (r == 2)
				/* LOWANGEN.LTX[107]: "<Ich weiss nicht, wovon Ihr redet. Welcher Salamanderstein?>" */
				/* LOWANGEN.LTX[100]: "Ihr helft Gavrons Gedaechtnis durch ein paar Hiebe auf die Spruenge." */
				/* LOWANGEN.LTX[101]: "Ihr lasst den Burschen laufen." */
				r = dlg_choose(g_dialog, txt[0x6b], 2, txt[0x64], txt[0x65]);
			else
				/* LOWANGEN.LTX[99]: "<Ich weiss wirklich nicht, was das soll!>" */
				r = dlg_choose(g_dialog, txt[0x63], 2, txt[0x64], txt[0x65]);
			if (r == 1) {
				/* LOWANGEN.LTX[102]: "<Au, hoert auf!! Ich sag Euch ja schon, was Ihr wissen wollt.>" */
				dlg_message(g_dialog, txt[0x66]);
				/* LOWANGEN.LTX[103]: "<Den Salamanderstein habe ich bei Vindaria abgeliefert, mehr kann ich nicht sagen.>" */
				/* LOWANGEN.LTX[104]: "Ihr hakt noch ein bisschen nach." */
				r = dlg_choose(g_dialog, txt[0x67], 2, txt[0x68], txt[0x65]);
				if (r == 1) {
					/* LOWANGEN.LTX[105]: "<Ich weiss wirklich nichts, mit der ganzen Bande habe ich nichts zu tun. Die haben mir
					 * 100 Goldstuecke geboten, wenn ich das Steinchen besorge - das hab ich mir nicht zweimal sagen lassen.>" */
					dlg_message(g_dialog, txt[0x69]);
					/* LOWANGEN.LTX[106]: "Ihr seid nur ein Momentchen unachtsam, da ist Euch Gavron auch schon entwischt." */
					dlg_message(g_dialog, txt[0x6a]);
				}
				g_vindaria_state = 1;
			}
			g_gavron_chase = -1;
		}
		break;
	case 2:
	case 3:
	case 4:
	case 6:
		diary_unlock(g_diary);
		if (g_gavron_chase == 2) {
			group_set_screen_pos(0x0b, 0x14);
			k = 5;
			g_gavron_chase = 3;
		} else if (g_gavron_chase == 3) {
			group_set_screen_pos(8, 0x10);
			k = 3;
			g_gavron_chase = 4;
		} else if (g_gavron_chase == 4) {
			group_set_screen_pos(0x0c, 0x0c);
			k = 1;
			g_gavron_chase = 5;
		} else if (g_gavron_chase == 6) {
			group_set_screen_pos(0x11, 9);
			k = -1;
			g_gavron_chase = 7;
		}
		n = group_count_check3(g_group);
		npc = group_get_npc(g_group);
		if (npc != 0)
			n--;
		j = 1;
		do {
			m = g_group->heroes[n - j];
			j++;
		} while (!check_hero3(m));
		if (test_skill(m, TA_GASSENWISSEN, 5) <= 0) {
			if (n > 2) {
				if (npc != 0) {
					make_group(2, m, npc);
					/* LOWANGEN.LTX[110]: "%s und %s haben den Anschluss verloren und bleiben zurueck." */
					sprintf(g_text_output_buf, txt[0x6e], m->name, npc->name);
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					make_group(1, m);
					/* LOWANGEN.LTX[109]: "%s hat den Anschluss verloren und bleibt zurueck." */
					sprintf(g_text_output_buf, txt[0x6d], m->name);
					dlg_message(g_dialog, g_text_output_buf);
				}
			} else {
				/* LOWANGEN.LTX[93]: "Der Bursche ist wieselflink - im Nu ist er Euch entwischt." */
				dlg_message(g_dialog, txt[0x5d]);
				g_gavron_chase = 0;
			}
		} else {
			/* LOWANGEN.LTX[108]: "Gavron ist flink wie ein Wiesel. Ihr habt Schwierigkeiten, ihn nicht zu verlieren." */
			dlg_message(g_dialog, txt[0x6c]);
		}
		diary_lock(g_diary);
		break;
	case 7:
		group_set_screen_pos(0x17, 0x0d);
		g_gavron_chase = 8;
		k = 0;
		m = get_first_hero(g_group);
		do {
			if (check_hero3(m) && test_skill(m, TA_SCHLEICHEN, 2) <= 0)
				k++;
			m = get_next_hero(g_group, m);
		} while (get_first_hero(g_group) != m);
		if (g_group->nr_heroes == k || k > 2) {
			dlg_message(g_dialog, txt[0x5d]);
			g_gavron_chase = 0;
		} else {
			dlg_message(g_dialog, txt[0x6c]);
		}
		break;
	case 8:
		group_set_screen_pos(0x1a, 0x0b);
		g_gavron_chase = -4;
		g_low_ailian_state = 3;
		/* LOWANGEN.LTX[95]: "Nachdem er sich mehrfach umgeschaut hat, ohne Euch zu entdecken, verschwindet Gavron in
		 * einem Haus." */
		dlg_message(g_dialog, txt[0x5f]);
		break;
	}
	dlg_set16(g_dialog, old);
	cam_set_from_group();
}
