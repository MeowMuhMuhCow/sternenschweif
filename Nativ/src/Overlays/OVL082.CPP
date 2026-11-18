/* OVL082 - Overlay O082 (Segment 0x20e5 in SCHWEIF.EXE): Tollwut-Anfall eines Helden (aus disease_effect O081 ab
 * Tag 3 der Tollwut; DSA-1-Gegenstueck rabies() in BE seg120). rabies_effect(h, txt): auf die Gruppe des Helden
 * wechseln (group_switch_next, bis g_group_no == h->group_no), Dialog dlg_box mit Portraet und drei Antworten
 * (Texte txt[23..26] der Krankheitstexte von O081, zweimal grammar_pronoun(h->sex, 2, 0) fuer Pronomen):
 * 1 = niederschlagen (sub_hero_le(h, LE / 2)), 2 = beruhigen (gelingt, wenn group_test_attrib_dmg(g_group,
 * ATTRIB_CH, 0, 0) einen Helden zaehlt), 3 = Zauber eines Zauberkundigen (get_active_hero, is_mage, AE >= 15,
 * test_spell(.., SP_SANFTMUT, 0), sub_ae_splash 15); nach Erfolg waehlt get_active_hero(txt[28]) einen Heiler
 * fuer skill_cure_disease(heiler, h, 10, 1) (g_hero_excluded = Platz des Kranken); ohne zweiten Helden
 * (group_count_check2b <= 1) Text txt[34]; ohne Erfolg txt[35] und Biss in die Gruppe group_sub_le(W6+2);
 * danach zurueck zur alten Gruppe, die zwei 100-Byte-Puffer freigeben.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL082.CPP   -> 1422/1422 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - C++ nur wegen der inline-Elementfunktion hero::is_mage() { return type >= 7; } (mov ax,1 / xor ax,ax /
 *   or ax,ax).
 * - Locals in Deklarationsreihenfolge old_width, healer, old_group, buf1, buf2 ([bp-2]..[bp-10h]); done/answer
 *   explizit register (SI/DI). Texte als txt[23]..txt[35] (far-Zeigertabelle, konstanter Index -> es:[bx+5ch]...).
 * - Zwei gleiche Aufrufe grammar_pronoun(h->sex, 2, 0) als sprintf-Argumente (rechts nach links ausgewertet).
 * Originalfehler (wie DSA 1): Antwort 2 gilt schon als gelungen, wenn group_test_attrib_dmg einen MISSERFOLG zaehlt
 *   (Rueckgabe = Zahl der gescheiterten Helden).
 */
#include <stdio.h>
#include "schweif.h"

void rabies_effect(struct hero far *h, char far * far *txt)
{
	signed short old_width;			/* gesicherter Dialog-Wert +16 */
	struct hero far *healer;
	signed short old_group;			/* Gruppe vor dem Anfall */
	char far *buf1;
	char far *buf2;
	register signed short done;
	register signed short answer;

	done = 0;
	buf1 = (char far *)mem_alloc(100L);
	buf2 = (char far *)mem_alloc(100L);
	old_group = g_group_no;
	while (h->group_no != g_group_no)
		group_switch_next(1);
	if (g_pp20_index == 4)
		screen_portraits(g_screen);
	while (!done) {
		if (group_count_check2b(g_group) > 1) {
			sprintf(g_text_output_buf, txt[23], h->name, grammar_pronoun(h->sex, 2, 0),
				grammar_pronoun(h->sex, 2, 0));
			sprintf(buf1, txt[24], h->name);
			sprintf(buf2, txt[25], h->name);
			old_width = dlg_set16(g_dialog, 6);
			answer = dlg_box(g_dialog, load_portrait(h), h->name, g_text_output_buf, 3, buf1, buf2, txt[26]);
			dlg_set16(g_dialog, old_width);
			if (answer == 1) {
				/* niederschlagen */
				sub_hero_le(h, h->le / 2);
				sprintf(g_text_output_buf, txt[27], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				g_hero_excluded = group_hero_pos(g_group, h);
				healer = get_active_hero(g_group, txt[28]);
				if (healer)
					skill_cure_disease(healer, h, 10, 1);
				done = 1;
			} else if (answer == 2) {
				/* beruhigen */
#if FIX_0215
				/* Fix #215: Rueckgabe = Zahl der gescheiterten Helden: gelungen bei 0 */
				if (group_test_attrib_dmg(g_group, ATTRIB_CH, 0, 0) == 0) {
#else
				if (group_test_attrib_dmg(g_group, ATTRIB_CH, 0, 0)) {
#endif
					sprintf(g_text_output_buf, txt[29], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					g_hero_excluded = group_hero_pos(g_group, h);
					healer = get_active_hero(g_group, txt[28]);
					if (healer)
						skill_cure_disease(healer, h, 10, 1);
					done = 1;
				} else {
					sprintf(g_text_output_buf, txt[30], h->name);
					dlg_message(g_dialog, g_text_output_buf);
				}
			} else if (answer == 3) {
				/* Zauber */
				g_hero_excluded = group_hero_pos(g_group, h);
				healer = get_active_hero(g_group, txt[31]);
				if (healer && healer->is_mage()) {
					if (healer->ae >= 15) {
						if (test_spell(healer, SP_SANFTMUT, 0)) {
							done = 1;
							sub_ae_splash(healer, 15);
							sprintf(g_text_output_buf, txt[32], h->name);
							dlg_message(g_dialog, g_text_output_buf);
							g_hero_excluded = group_hero_pos(g_group, h);
							healer = get_active_hero(g_group, txt[28]);
							if (healer)
								skill_cure_disease(healer, h, 10, 1);
						}
					} else {
						sprintf(g_text_output_buf, txt[33], healer->name);
						dlg_message(g_dialog, g_text_output_buf);
					}
				}
			}
		} else {
			sprintf(g_text_output_buf, txt[34], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			done = 1;
		}
		if (!done) {
			sprintf(g_text_output_buf, txt[35], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			group_sub_le(g_group, dice_roll(1, 6, 2));
			done = 1;
		}
	}
	while (old_group != g_group_no)
		group_switch_next(1);
	if (g_pp20_index == 4)
		screen_portraits(g_screen);
	mem_free(buf1);
	mem_free(buf2);
}
