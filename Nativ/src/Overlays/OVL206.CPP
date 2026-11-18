/* OVL206 - Overlay O206 (Segment 0x238a in SCHWEIF.EXE): Charakterbogen drucken (Teil 1; Teil 2 und Hilfen in O207).
 * hero_print_sheet(h): Texte 0x20d nach h->f5e7/h->txt (txt_load_list); Ausgabe in Datei (g_print_to_file: Name
 *   print_file_name -> *g_names_star_idx, file_open_mode(g_file_main, 0, ...), Fehler txt[110]) oder auf den Drucker
 *   (biosprint-Status 0/0x90, sonst txt[2] bei 0x20 bzw. txt[1]; Hinweis txt[0]/txt[109]). Zauberkundige
 *   (is_mage) waehlen Teil 1/2/beide (txt[112..115]). Teil 1: Kopf (txt[3..6], Typname g_ttx, Sternzeichen
 *   g_ttx[god + 4]), Eigenschaften (attrib[] {normal, cur, mod}, hero_at_weapon/hero_pa_weapon), Kampfwerte
 *   (avg = Summe dreier Eigenschaften / 4, get_skill_value, calc_bp), Talente get_skill_value(h, TA_*), 15
 *   Inventarzeilen print_items, Zeile fuer Platz 30 (print_get_item, memset/strncpy 14 Zeichen), Schluss
 *   g_str_space_707a/txt[111]/g_str_space_707c bzw. Seitenvorschub; Teil 2: print_spells.
 * print_line(h, s): Zeile ausgeben (Datei: s und g_str_crlf_707e per File_write, Rueckgabe 1 wenn geschrieben;
 *   Drucker: Zeichen per biosprint + game_idle, CR/LF, Status). print_get_item(h, n, id, cnt, gew, a, b, c):
 *   Inventarplatz n (g_items: armor/weapon/stackable, table_index, weight; g_armors_table/g_weapons_table), Name
 *   grammar_article(0x4000/0x4004, id, 0).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL206.CPP   -> 6108/6108 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (g_str_crlf_707e -> _s_707c+0x2). Ohne -Og 367 Abweichungen (28 Bytes zu lang).
 * - Urspruenglich zusammengesetzt aus OVL206.TPL + Rumpf per gen/mk206.py (gen/g206.py arbeitet die ASM als
 *   Stapelmaschine ab); seit der Namensfassung ist diese C-Datei massgeblich.
 * - Auswahlergebnis r im inneren Block (memset/strncpy-Intrinsics auf Funktionsebene sperren SI).
 * - Locals: 13 Skalare in Stackreihenfolge deklariert, Puffer 10/6 Bytes in Deklarationsreihenfolge, 20 ans Ende.
 * - print_get_item: "*pb = *pc = -1" (pc zuerst gespeichert); "if (..) {..; return ..;} else return 0;" (zwei tote
 *   Epiloge); Flagbyte +2 als -Og-Byte [bp-3].
 * Auffaellig: print_get_item prueft "*id != 0" ein zweites Mal, obwohl es schon feststeht (harmlos).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include <stdio.h>
#include <string.h>
#include <bios.h>
#include <fcntl.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:706a..7082, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

void hero_print_sheet(struct hero far *h)
{
	signed short st;
	signed short avg;
	signed short v6;
	signed short v8;
	signed short va;
	signed short vc;
	signed short ve;
	signed short v10;
	char far *name;
	char far *fn;
	signed short part2 = 0;
	signed short part1 = 0;
	signed short abort = 0;
	char b10[10];
	char b6[6];
	char b20[20];

	txt_load_list(g_font, 0x20d, 0x82, &h->f5e7, &h->txt);
	if (g_print_to_file != 0) {
		fn = new char[0x14];
		print_file_name(h, fn);
		*(char far * far *)g_names_star_idx = fn;
		if (file_open_mode(g_file_main, 0, O_BINARY | O_TRUNC | O_CREAT | O_WRONLY) == -1) {
			abort = 1;
			dlg_message(g_dialog, h->txt[110]);
			delete fn;
		}
	} else {
		st = biosprint(2, 0, 0);
		if (st != 0 && st != 0x90) {
			dlg_message(g_dialog, st == 0x20 ? h->txt[2] : h->txt[1]);
			abort = 1;
		} else {
			dlg_choose(g_dialog, h->txt[0], 1, h->txt[109]);
		}
	}
	if (h->is_mage()) {
		signed short r;

		r = dlg_choose(g_dialog, h->txt[112], 3, h->txt[113], h->txt[114], h->txt[115]);
		if (r == -1)
			abort = 1;
		else if (r == 1)
			part1 = 1;
		else if (r == 2)
			part2 = 1;
		else if (r == 3)
			part2 = part1 = 1;
	} else {
		part1 = 1;
	}
	if (abort == 0) {
		if (part1 != 0) {
			print_line(h, h->txt[3]);
			print_line(h, " ");
			sprintf(g_text_buf2, h->txt[4], h->name, g_ttx[(h->sex ? 0x3e : 0x32) + h->type],
				!h->sex ? h->txt[116] : h->txt[117]);
			print_line(h, g_text_buf2);
			print_line(h, " ");
			sprintf(g_text_buf2, h->txt[5], h->height, h->weight, g_ttx[h->god + 4]);
			print_line(h, g_text_buf2);
			print_line(h, " ");
			sprintf(g_text_buf2, h->txt[6], h->ap, h->level);
			print_line(h, g_text_buf2);
			print_line(h, " ");
			sprintf(g_text_buf2, h->txt[7], (signed char)(h->attrib[ATTRIB_MU].cur + h->attrib[ATTRIB_MU].mod), h->attrib[ATTRIB_MU].normal, (signed char)(h->attrib[ATTRIB_AG].cur + h->attrib[ATTRIB_AG].mod), h->attrib[ATTRIB_AG].normal, h->atpa_base);
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[8], (signed char)(h->attrib[ATTRIB_KL].cur + h->attrib[ATTRIB_KL].mod), h->attrib[ATTRIB_KL].normal, (signed char)(h->attrib[ATTRIB_HA].cur + h->attrib[ATTRIB_HA].mod), h->attrib[ATTRIB_HA].normal);
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[9], (signed char)(h->attrib[ATTRIB_CH].cur + h->attrib[ATTRIB_CH].mod), h->attrib[ATTRIB_CH].normal, (signed char)(h->attrib[ATTRIB_RA].cur + h->attrib[ATTRIB_RA].mod), h->attrib[ATTRIB_RA].normal, hero_at_weapon(h, 0), hero_pa_weapon(h, 0));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[10], (signed char)(h->attrib[ATTRIB_FF].cur + h->attrib[ATTRIB_FF].mod), h->attrib[ATTRIB_FF].normal, (signed char)(h->attrib[ATTRIB_GG].cur + h->attrib[ATTRIB_GG].mod), h->attrib[ATTRIB_GG].normal, hero_at_weapon(h, 1), hero_pa_weapon(h, 1));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[11], (signed char)(h->attrib[ATTRIB_GE].cur + h->attrib[ATTRIB_GE].mod), h->attrib[ATTRIB_GE].normal, (signed char)(h->attrib[ATTRIB_TA].cur + h->attrib[ATTRIB_TA].mod), h->attrib[ATTRIB_TA].normal, hero_at_weapon(h, 2), hero_pa_weapon(h, 2));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[12], (signed char)(h->attrib[ATTRIB_IN].cur + h->attrib[ATTRIB_IN].mod), h->attrib[ATTRIB_IN].normal, (signed char)(h->attrib[ATTRIB_NG].cur + h->attrib[ATTRIB_NG].mod), h->attrib[ATTRIB_NG].normal, hero_at_weapon(h, 3), hero_pa_weapon(h, 3));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[13], (signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod), h->attrib[ATTRIB_KK].normal, (signed char)(h->attrib[ATTRIB_JZ].cur + h->attrib[ATTRIB_JZ].mod), h->attrib[ATTRIB_JZ].normal, hero_at_weapon(h, 4), hero_pa_weapon(h, 4));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[14], hero_at_weapon(h, 5), hero_pa_weapon(h, 5));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[15], h->le_max, h->ae_max, hero_at_weapon(h, 6), hero_pa_weapon(h, 6));
			print_line(h, g_text_buf2);
			print_line(h, " ");
			avg = ((signed char)(h->attrib[ATTRIB_KL].cur + h->attrib[ATTRIB_KL].mod) + (signed char)(h->attrib[ATTRIB_GE].cur + h->attrib[ATTRIB_GE].mod) + (signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod)) / 4;
			sprintf(g_text_buf2, h->txt[16], (signed char)(h->mr + h->mr_mod), h->rs_bonus, get_skill_value(h, TA_SCHUSSWAFFEN) + avg);
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[17], h->le + (signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod), calc_bp(h), get_skill_value(h, TA_WURFWAFFEN) + avg);
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[18], (signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod) * 100, h->load);
			print_line(h, g_text_buf2);
			print_line(h, h->txt[19]);
			print_line(h, " ");
			print_line(h, h->txt[20]);
			sprintf(g_text_buf2, h->txt[21], get_skill_value(h, TA_WAFFENLOS), get_skill_value(h, TA_ABRICHTEN), get_skill_value(h, TA_ALCHIMIE));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[22], get_skill_value(h, TA_HIEBWAFFEN), get_skill_value(h, TA_FAHRZEUGE), get_skill_value(h, TA_ALTE_SPRACHEN));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[23], get_skill_value(h, TA_STICHWAFFEN), get_skill_value(h, TA_FALSCHSPIEL), get_skill_value(h, TA_GEOGRAPHIE));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[24], get_skill_value(h, TA_SCHWERTER), get_skill_value(h, TA_HEILEN_GIFT), get_skill_value(h, TA_GESCHICHTE));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[25], get_skill_value(h, TA_AEXTE), get_skill_value(h, TA_HEILEN_KRANKHEITEN), get_skill_value(h, TA_GOETTER_KULTE));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[26], get_skill_value(h, TA_SPEERE), get_skill_value(h, TA_HEILEN_WUNDEN), get_skill_value(h, TA_KRIEGSKUNST));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[27], get_skill_value(h, TA_ZWEIHAENDER), get_skill_value(h, TA_MUSIZIEREN), get_skill_value(h, TA_LESEN));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[28], get_skill_value(h, TA_SCHUSSWAFFEN), get_skill_value(h, TA_SCHLOESSER), get_skill_value(h, TA_MAGIEKUNDE));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[29], get_skill_value(h, TA_WURFWAFFEN), get_skill_value(h, TA_TASCHENDIEBSTAHL), get_skill_value(h, TA_SPRACHEN));
			print_line(h, g_text_buf2);
			print_line(h, " ");
			print_line(h, h->txt[30]);
			sprintf(g_text_buf2, h->txt[31], get_skill_value(h, TA_AKROBATIK), get_skill_value(h, TA_BEKEHREN), get_skill_value(h, TA_FAEHRTENSUCHEN));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[32], get_skill_value(h, TA_KLETTERN), get_skill_value(h, TA_BETOEREN), get_skill_value(h, TA_FESSELN));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[33], get_skill_value(h, TA_KOERPERBEHERRSCHUNG), get_skill_value(h, TA_FEILSCHEN), get_skill_value(h, TA_ORIENTIERUNG));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[34], get_skill_value(h, TA_REITEN), get_skill_value(h, TA_GASSENWISSEN), get_skill_value(h, TA_PFLANZENKUNDE));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[35], get_skill_value(h, TA_SCHLEICHEN), get_skill_value(h, TA_LUEGEN), get_skill_value(h, TA_TIERKUNDE));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[36], get_skill_value(h, TA_SCHWIMMEN), get_skill_value(h, TA_MENSCHENKENNTNIS), get_skill_value(h, TA_WILDNISLEBEN));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[37], get_skill_value(h, TA_SELBSTBEHERRSCHUNG), get_skill_value(h, TA_SCHAETZEN));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[38], get_skill_value(h, TA_VERSTECKEN), get_skill_value(h, TA_TANZEN));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[39], get_skill_value(h, TA_ZECHEN));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[40], get_skill_value(h, TA_GEFAHRENSINN));
			print_line(h, g_text_buf2);
			sprintf(g_text_buf2, h->txt[41], h->skill_incs, get_skill_value(h, TA_SINNESSCHAERFE));
			print_line(h, g_text_buf2);
			print_line(h, h->txt[42]);
			print_line(h, h->txt[43]);
			print_line(h, " ");
			print_items(h, 0, 1);
			print_items(h, 2, 3);
			print_items(h, 4, 5);
			print_items(h, 6, 7);
			print_items(h, 8, 9);
			print_items(h, 10, 11);
			print_items(h, 12, 13);
			print_items(h, 14, 15);
			print_items(h, 16, 17);
			print_items(h, 18, 19);
			print_items(h, 20, 21);
			print_items(h, 22, 23);
			print_items(h, 24, 25);
			print_items(h, 26, 27);
			print_items(h, 28, 29);
			name = print_get_item(h, 0x1e, &v8, &ve, &v10, &v6, &va, &vc);
			sprintf(b10, va != -1 ? h->txt[50] : h->txt[51], va, vc);
			sprintf(b6, v6 != -1 ? h->txt[52] : h->txt[53], v6);
			memset(b20, 0, 20);
			strncpy(b20, name, 14);
			sprintf(g_text_buf2, v8 == 0 ? h->txt[49] : h->txt[45], ve, b20, v10, b6, b10);
			print_line(h, g_text_buf2);
			if (g_print_to_file != 0) {
				print_line(h, " ");
				print_line(h, h->txt[111]);
				print_line(h, " ");
			} else {
				biosprint(0, 0x0c, 0);
			}
		}
		if (part2 != 0)
			print_spells(h);
		if (g_print_to_file != 0) {
			file_close(g_file_main);
			delete fn;
		}
	}
	mem_free(h->f5e7);
	mem_free(h->txt);
}

signed short print_line(struct hero far *h, char far *s)
{
	signed short r;
	char far *nl;

	r = 0;
	if (g_print_to_file != 0) {
		nl = "\r\n";
		if (File_write(g_file_main, s, strlen(s))) {
			r = 1;
			File_write(g_file_main, nl, strlen(nl));
		}
	} else {
		while (*s != 0) {
			biosprint(0, *s++, 0);
			game_idle();
		}
		biosprint(0, 0x0d, 0);
		biosprint(0, 0x0a, 0);
		r = biosprint(2, 0, 0);
	}
	return r;
}

char far *print_get_item(struct hero far *h, signed short n, signed short far *id, signed short far *cnt,
	signed short far *w, signed short far *pa, signed short far *pb, signed short far *pc)
{
	signed short t;

	if ((*id = h->inv[n].item_id) != 0) {
		if (((*cnt = h->inv[n].quantity) == 0 && *id != 0) || !g_items[*id].stackable)
			*cnt = 1;
		*w = g_items[*id].weight * *cnt;
		if (g_items[*id].armor)
			*pa = g_armors_table[g_items[*id].table_index].rs;
		else
			*pa = -1;
		if (g_items[*id].weapon && !g_items[*id].stackable) {
			*pb = g_weapons_table[g_items[*id].table_index].dmg_d6;
			*pc = g_weapons_table[g_items[*id].table_index].dmg_add;
		} else {
			*pb = *pc = -1;
		}
		t = *cnt == 1 ? 0x4000 : 0x4004;
		return grammar_article(t, *id, 0);
	} else
		return 0;
}
