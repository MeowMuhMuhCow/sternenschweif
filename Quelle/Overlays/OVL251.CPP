/* OVL251 - Overlay O251 (Segment 0x247b in SCHWEIF.EXE): Kartenereignis-Rueckruf der Stadt Tjolmar (Stadt 6,
 * g_town_descs[6].handler; Ereignisobjekt o = Town aus O183, Texte TJOLMAR.LTX = Datei 0x21e).
 * town_tjolmar_event(o, q): pos = (y - g_map_x0) << 8 | (x - g_map_y0) wie O186. Art -2 (Feld betreten): Felder
 *   0xd15/0x1204 -> Orte LOC_DATA | 0x12/0x13, Wasserfeld (Feldtyp 3) -> town_swim_check. Art 1 (Tuer): Feld -> Ort
 *   LOC_DATA | 1 .. 0x14 (town_enter), Haus des Zwergs 0x50d/0x50a -> tjolmar_dwarf_door, sonst Wohnhaus LOC_HOUSE.
 * tjolmar_dwarf_door(o): Haus des Zwergs Ingramosch (Portraet NVF 10 Nr. 0x63, Bmp 32 x 32):
 *   - ohne Salamanderstein und ohne g_tjolmar_vault: TJOLMAR.LTX[14] Tuer aufbrechen? -> magisch gesichert [15], ohne
 *     Magier [16], sonst [17] mit Menue [18] Analues (10 AE, [24]: Claudibus Clavistibor u. a.) / [19] Foramen (die
 *     ganze AE wird entzogen, [25]) / [20]; zu wenig AE [23].
 *   - mit dem Salamanderstein: einmal (g_ingramosch_talked) das Gespraech an der Tuer [0..10] - die Aura des Steins
 *     entriegelt die Tuer, der Zwerg weist die Gruppe ab ([8], [9], [10]); danach [11] Tuer aufbrechen ([12]) oder
 *     gehen ([13]). Aufbrechen: ein NSC (nicht NSC 4) verabschiedet sich [27] (g_npc_months 0x7f bzw. 0x63), die
 *     Gruppe dringt in das Gewoelbe unter Tjolmar ein: Stadt 0, Dungeon 1 bei (0x1c, 1), Tagebuch GLOBLTXT.LTX[385]
 *     "Wir sind heute in ein Gewoelbe unter Tjolmar geraten.", Neuaufbau, g_tjolmar_vault = 1 (R12 -> Ereignis O132).
 *   - mit g_tjolmar_vault nur der Wechsel in das Gewoelbe.
 *
 * BCC -mlarge -3 -Ot -Oi -c -Y OVL251.CPP   -> 2570/2570 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (mit zusaetzlich -Og ebenfalls GLEICH; Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Oi: strcpy/strcat inline (repne scasb / rep movsw / rep movsb), daher SI/DI gesichert.
 * - C++ wegen lokalem Bmp-Objekt (Konstruktor gb_new_alloc(&b, 0x20, 0x20) mitten in der Funktion nach
 *   "ask_door = admitted = 0", Destruktor gb_delete(&b, 2) am Ende) und der inline-Memberfunktion group::set_town,
 *   aufgerufen verschachtelt "g_group->set_town(g_group->set_town(0))": Ergebnis des inneren Aufrufs =
 *   Parameterkopie des aeusseren ([bp-19h] bzw. im zweiten Zweig eigener Local [bp-1ah]; ein Block-Local wuerde den
 *   Platz teilen).
 * - "ae = h->ae; if (ae < 10)" als eigener Local [bp-4] (danach wird +76 erneut gelesen).
 * - Gruppenzugriffe ueber den Zeiger g_group (les je Anweisung); town_enter/town_swim_check als freie Funktionen.
 */
#include <string.h>
#include <stdio.h>
#include "schweif.h"

void tjolmar_dwarf_door(Town far *o);

signed short town_tjolmar_event(Town far *o, struct map_event far *q)
{
	signed short v;
	signed short r;
	register signed short ev;
	register signed short xy;

	ev = q->type;
	xy = (q->y - g_map_x0) << 8;
	xy |= q->x - g_map_y0;
	r = 0;
	if (ev == -2) {
		if (xy == 0xd15) {
			town_enter(o, LOC_DATA | 0x12, q);
			r = 1;
		}
		if (xy == 0x1204) {
			town_enter(o, LOC_DATA | 0x13, q);
			r = 1;
		}
		if (get_cell_a(q->x, q->y) == 3)
			town_swim_check(o);
	} else if (ev == 1) {
		v = 0;
		switch (xy) {
		case 0x412:
			v = LOC_DATA | 0x01;
			break;
		case 0xd0f:
			v = LOC_DATA | 0x02;
			break;
		case 0x604:
			v = LOC_DATA | 0x03;
			break;
		case 0x140a:
			v = LOC_DATA | 0x04;
			break;
		case 0xc0c:
			v = LOC_DATA | 0x05;
			break;
		case 0x1205:
			v = LOC_DATA | 0x06;
			break;
		case 0x1510:
			v = LOC_DATA | 0x07;
			break;
		case 0x809:
			v = LOC_DATA | 0x08;
			break;
		case 0x1415:
			v = LOC_DATA | 0x09;
			break;
		case 0x1907:
			v = LOC_DATA | 0x0a;
			break;
		case 0x1011:
			v = LOC_DATA | 0x0b;
			break;
		case 0xd07:
			v = LOC_DATA | 0x0c;
			break;
		case 0x120c:
			v = LOC_DATA | 0x0d;
			break;
		case 0xc09:
			v = LOC_DATA | 0x0e;
			break;
		case 0x180d:
			v = LOC_DATA | 0x0f;
			break;
		case 0x50d:
			tjolmar_dwarf_door(o);
			break;
		case 0xd15:
			v = LOC_DATA | 0x12;
			break;
		case 0x1204:
			v = LOC_DATA | 0x13;
			break;
		case 0x100d:
			v = LOC_DATA | 0x14;
			break;
		default:
			town_enter(o, LOC_HOUSE, q);
			r = 1;
			break;
		}
		if (v) {
			town_enter(o, v, q);
			r = 1;
		} else if (xy == 0x50a) {
			tjolmar_dwarf_door(o);
			r = 1;
		}
	}
	return r;
}

void tjolmar_dwarf_door(Town far *o)
{
	signed short w;
	signed short ae;
	signed short old;
	signed char ans = -1;
	signed char ask_door;
	signed char admitted;
	struct hero far *h;

	ask_door = admitted = 0;
	Bmp b(0x20, 0x20);
	nvfile_load(g_nvf_heads, 0x0a, b.data(), (void far *)0, 0x63, 0, &w, &w, 0x140);
	old = dlg_set16(g_dialog, 5);
	if (group_find_item(g_group, ITEM_SALAMANDER_STONE) == -1 && !g_tjolmar_vault) {
		/* TJOLMAR.LTX[14]: "Auf Euer Klopfen ruehrt sich nichts. Wollt Ihr die Tuere aufbrechen?" */
		ans = dlg_yes_no(g_dialog, o->txt[14]);
		if (ans == 1) {
			/* TJOLMAR.LTX[15]: "Die Tuere scheint magisch gesichert zu sein." */
			strcpy(g_text_output_buf, o->txt[15]);
			if (get_mage_hero(g_group)) {
				/* TJOLMAR.LTX[17]: "@Wollt Ihr" */
				strcat(g_text_output_buf, o->txt[17]);
				do {
					/* TJOLMAR.LTX[18]: "die Tuere durch einen <Analues>-Zauber auf die Art des Verschlusszaubers untersuchen" */
					/* TJOLMAR.LTX[19]: "durch einen <Foramen>-Zauber versuchen die Tuere zu oeffnen" */
					/* TJOLMAR.LTX[20]: "Eure Zeit nicht mit Tueren verschwenden" */
					ans = dlg_choose(g_dialog, g_text_output_buf, 3, o->txt[18], o->txt[19],
						o->txt[20]);
					if (ans == 1) {
						/* TJOLMAR.LTX[21]: "Wer soll den Zauber versuchen?" */
						h = get_active_mage(g_group, o->txt[21]);
						if (h) {
							ae = h->ae;
							if (ae < 10) {
								/* TJOLMAR.LTX[23]: "Leider verfuegt %s nicht ueber die noetige Astralenergie." */
								sprintf(g_text_output_buf, o->txt[23], h->name);
								dlg_message(g_dialog, g_text_output_buf);
							} else {
								sub_ae_splash(h, 10);
								/* TJOLMAR.LTX[24]: "Sofort erkennt %s einen sehr starken <Claudibus Clavistibor>, der fuer den Verschluss
								 * der Tuere verantwortlich ist, aber neben diesem Zauber kann %s auch noch mindestens zwei schwaechere
								 * Zauber ausmachen, wovon einer ein Druidenzauber aus dem Bereich Beherrschung ist." */
								sprintf(g_text_output_buf, o->txt[24], h->name,
									grammar_pronoun(h->sex, 0, 0));
								dlg_message(g_dialog, g_text_output_buf);
								screen_portraits(g_screen);
							}
						}
					} else if (ans == 2) {
						h = get_active_mage(g_group, o->txt[21]);
						if (h) {
							ae = h->ae;
							if (ae < 2) {
								sprintf(g_text_output_buf, o->txt[23], h->name);
								dlg_message(g_dialog, g_text_output_buf);
							} else {
								sub_ae_splash(h, h->ae);
								/* TJOLMAR.LTX[25]: "Der <Foramen> kann den Verschlusszauber nicht aufheben und %s spuert, wie %s gesamte
								 * Astralenergie entzogen wird.@Auf der Tuere muss noch der Zauber <Magischer Raub> liegen, mit dem sich
								 * der Verschlusszauber staendig wieder erneuert." */
								sprintf(g_text_output_buf, o->txt[25], h->name,
									grammar_pronoun(h->sex, 1, 1));
								dlg_message(g_dialog, g_text_output_buf);
								screen_portraits(g_screen);
							}
						}
					}
				} while (ans != 3 && ans != -1);
			} else {
				/* TJOLMAR.LTX[16]: "@Ihr habt keine Moeglichkeit sie zu oeffnen." */
				strcat(g_text_output_buf, o->txt[16]);
				dlg_message(g_dialog, g_text_output_buf);
			}
		}
	} else if (!g_tjolmar_vault) {
		if (g_ingramosch_talked != 0)
			ask_door = 1;
		else {
			g_ingramosch_talked = 1;
			while (ans == -1)
				/* TJOLMAR.LTX[0]: "Gerade als Ihr an die Tuere klopfen wollt, bemerkt Ihr, dass der Salamanderstein eine
				 * ungeheure Energie ausstrahlt, welche die Tuere mit einer magischen Aura umhuellt und die Tuere
				 * entriegelt.@Danach erscheint ein Zwerg in der Tuere und mustert Euch kuehl. <Was wollt Ihr?> fragt er mit
				 * einer Stimme, die Euch irgendwie zu raten scheint, einen wirklich guten Grund fuer die Stoerung zu haben." */
				/* TJOLMAR.LTX[1]: "<Wir suchen Ingramosch.>" */
				/* TJOLMAR.LTX[2]: "<Warum so feindselig?>" */
				/* TJOLMAR.LTX[3]: "<Die Wahrheit.>" */
				ans = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[0], 3, o->txt[1], o->txt[2],
					o->txt[3]);
			switch (ans) {
			case 1:
				admitted = 1;
				break;
			case 2:
				ans = -1;
				while (ans == -1)
					/* TJOLMAR.LTX[4]: "<Ich habe zu tun. Also, was wollt Ihr?>" */
					/* TJOLMAR.LTX[5]: "<Wo ist Ingramosch?>" */
					/* TJOLMAR.LTX[6]: "<Dann entschuldigt uns.>" */
					/* TJOLMAR.LTX[7]: "<Seid gefaelligst ein wenig hoeflicher!>" */
					ans = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[4], 3, o->txt[5],
						o->txt[6], o->txt[7]);
				switch (ans) {
				case 1:
					admitted = 1;
					break;
				case 2:
					break;
				case 3:
					/* TJOLMAR.LTX[8]: "<Verzieht Euch.> sagt der Zwerg und schlaegt die Tuere zu." */
					dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[8], 0);
					ask_door = 1;
					break;
				}
				break;
			case 3:
				/* TJOLMAR.LTX[9]: "<Dann geht in einen Tempel; die haben so etwas.> Mit diesen Worten schlaegt er Euch die
				 * Tuere vor der Nase zu." */
				dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[9], 0);
				ask_door = 1;
				break;
			}
		}
		if (admitted == 1) {
			/* TJOLMAR.LTX[10]: "<Der wohnt nicht mehr hier.> Mit diesen Worten schliesst er die Tuere." */
			dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[10], 0);
			ask_door = 1;
		}
		if (ask_door == 1) {
			/* TJOLMAR.LTX[11]: "Ihr steht vor dieser Tuere und starrt sie an. Wollt Ihr" */
			/* TJOLMAR.LTX[12]: "sie aufbrechen und den Zwergen zur Antwort zwingen oder" */
			/* TJOLMAR.LTX[13]: "lieber gehen?" */
			ans = dlg_choose(g_dialog, o->txt[11], 2, o->txt[12], o->txt[13]);
			if (ans == 1) {
				h = group_get_npc(g_group);
				if (h && check_hero(h) && h->npc_id != 4) {
					/* TJOLMAR.LTX[27]: "Mit den Worten@<Ich will noch nicht so jung sterben!>@verabschiedet sich %s von Euch." */
					sprintf(g_text_output_buf, o->txt[27], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					if (h->npc_id == 3)
						g_npc_months = 0x7f;
					else
						g_npc_months = 0x63;
					groups_check_npc();
				}
				g_group->set_town(g_group->set_town(0));
				g_group->old_dungeon = g_group->dungeon;
				g_group->dungeon = 1;
				group_set_screen_pos(0x1c, 1);
				g_group->dir = 0;
				/* GLOBLTXT.LTX[385]: "Wir sind heute in ein Gewoelbe unter Tjolmar geraten." */
				diary_add(g_diary, g_ttx[385], 0);
				g_pp20_index = -1;
				g_refresh = 1;
				g_loaded_town = -1;
				g_tjolmar_vault = 1;
			}
		}
	} else {
		g_group->set_town(g_group->set_town(0));
		g_group->old_dungeon = g_group->dungeon;
		g_group->dungeon = 1;
		group_set_screen_pos(0x1c, 1);
		g_group->dir = 0;
		diary_add(g_diary, g_ttx[385], 0);
		g_pp20_index = -1;
		g_refresh = 1;
		g_loaded_town = -1;
	}
	dlg_set16(g_dialog, old);
}
