/* OVL196 - Overlay O196 (Segment 0x2346 in SCHWEIF.EXE): NSC-Begleiter entlassen und aufnehmen (BE seg095
 * npc_farewell/load_npc/add_npc; O196_0000 heisst wegen O005 weiter groups_check_npc).
 * groups_check_npc: fuer alle sechs Gruppen (g_groups, voruebergehend als g_group) den NSC holen (group_get_npc); lebt
 * er (check_hero) oder ist g_npc_months >= 99, je nach NSC-Nummer (npc_id, 1..8) und g_npc_months entlassen
 * (group_remove_npc) mit Meldung (Texte g_ttx), NSC 3 fragt bei 99 ggf. nach (dlg_yes_no) und zieht dem Helden mit
 * der hoechsten LE alle LE bis auf 6 ab (get_hero_max_le, sub_hero_le, Bildeffekt 20 x nvfile_load/gb_blit auf ein
 * 32x32-Bild), NSC 8 setzt g_npc8_days = 0 / g_dragan_task = 1; danach Abschiedstext ins Tagebuch (diary_add(g_diary, ..)).
 * load_npc(h, n): Heldendatei g_npc_file_ids[n] oeffnen (Dateiobjekt g_ofile, O243), 0x5ef Bytes lesen,
 * h->group_no = g_group_no. add_npc(n): g_npc_months = 99, groups_check_npc (alte NSC entlassen), freien Platz
 * (group_free_slot), NSC n als Held 6 laden und aufnehmen (group_add_hero), Begruessung je NSC-Nummer, g_refresh = 1.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL196.C   -> 2631/2631 Bytes GLEICH (18.10.2026; -Ov statt -Og ebenso),
 * cobjfix laeuft (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Reines C. -Og nur fuer load_npc ("if (..) { ..; return 1; } else return 0;" -> zwei tote Epiloge).
 * - strcpy(name, h->name) als -Oi-Intrinsic (repne scasb / rep movsw / adc cx,cx / rep movsb).
 * - Drei switch: Sprungtabellen hinter der Funktion, die des ZWEITEN switch steht zuerst.
 *   Fehlender case 2 -> Tabelleneintrag = default; "case 4: case 6: case 7: case 8:" teilen einen Rumpf.
 * - Erster switch: Rumpf von case 7 steht VOR case 6 (gleicher Inhalt, aber Sprungtabelle verraet die Reihenfolge).
 * - "if (!!g_group->loctype && ...)" fuer mov al / cbw / or ax,ax; "g_group->loctype", "!= 0" und "(signed short)"
 *   ergeben alle cmp byte [..],0.
 * - Schleife "for (i = 0, g = g_groups; i < NR_GROUPS; i++, g++)" -> mov [bp-0ah],ds / mov [bp-0ch],0c30fh,
 *   add word [bp-0ch],49h; Gruppenzeiger sichern/zurueck ueber mov eax.
 * - victim->le - 6 -> add ax,0fffah (kein sub).
 * - load_npc: Laenge als unsigned short (movzx eax fuer ofile_read), "group_no = g_group_no; h->group_no = group_no;"
 *   mit int-Local -> mov al,[bp-4].
 * - g_portrait_x[6] (ds:349d hat kein eigenes Public, cobjfix: g_3491+0xc).
 * Auffaellig (kein sicherer Fehler): nvfile_load bekommt zweimal &x (vermutlich &x, &y gemeint); "drained"
 *   wird bei NSC 3 auch gesetzt, wenn kein Held LE verloren hat (Abschiedstext 0x17d trotzdem).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

void groups_check_npc(void)
{
	struct hero far *h;
	struct hero far *victim;
	struct group far *g;
	struct group far *saved;
	void far *bmp;
	signed short i;
	signed short answer;
	signed short x;
	signed char k;
	signed short left;
	char far *name;
	signed short drained;
	signed short quiet;
	signed char npc;

	left = 0;
	name = (char far *)mem_alloc_w(0x28);
	drained = 0;
	quiet = 0;
	npc = 0;
	saved = g_group;
	for (i = 0, g = g_groups; i < NR_GROUPS; i++, g++) {
		g_group = g;
		if ((h = group_get_npc(g_group)) != 0) {
			npc = h->npc_id;
			strcpy(name, h->name);
			if (check_hero(h) || g_npc_months >= 99) {
				switch (npc) {
				case 1:
					if (g_npc_months >= 2) {
						left = 1;
						group_remove_npc(g_group, h);
						if (check_hero3(h)) dlg_message(g_dialog, g_ttx[0x1b5]);
					}
					break;
				case 2:
					if (g_npc_months >= 2) {
						left = 1;
						group_remove_npc(g_group, h);
						if (check_hero3(h)) {
							sprintf(g_text_output_buf, g_ttx[0x1b7], name);
							dlg_message(g_dialog, g_text_output_buf);
						}
					}
					break;
				case 3:
					if (g_npc_months >= 60) {
						if (!!g_group->loctype && g_npc_months >= 99 && check_hero3(h))
							answer = dlg_yes_no(g_dialog, g_ttx[0x1c4]);
						else if (g_npc_months >= 99)
							answer = 1;
						else
							answer = 0;
						if (answer == 1) {
							/* NSC 3 nimmt beim Abschied einem Helden LE */
							drained = 1;
							victim = get_hero_max_le(g_group);
							if (victim && victim->le > 6 && check_hero3(h)) {
								if (g_npc_months == 0x7f)
									sprintf(g_text_output_buf, g_ttx[0x183], victim->name,
										grammar_pronoun(victim->sex, 1, 1));
								else
									sprintf(g_text_output_buf, g_ttx[0x1c5], victim->name,
										grammar_pronoun(victim->sex, 1, 1));
								dlg_message(g_dialog, g_text_output_buf);
								sub_hero_le(victim, victim->le - 6);
								screen_portraits(g_screen);
								sprintf(g_text_output_buf, g_ttx[0x1c6], hero_name_gen(victim, 1));
								dlg_message(g_dialog, g_text_output_buf);
								bmp = gb_new_alloc(0, 0x20, 0x20);
								for (k = 0; k < 20; k++) {
									nvfile_load(g_nvf_heads, 10, gb_data(bmp), 0, random_schweif(200), 0,
										&x, &x, 0x140);
									gb_blit(bmp, g_vga_buf, g_portrait_x[6], 0x9d);
									wait_ticks_key(5);
								}
								gb_clear(bmp);
								gb_blit(bmp, g_vga_buf, g_portrait_x[6], 0x9d);
								gb_delete(bmp, 3);
								wait_ticks_key(0x24);
							}
							left = 1;
							group_remove_npc(g_group, h);
							screen_portraits(g_screen);
						}
					}
					break;
				case 4:
					if (g_npc_months >= 2) {
						left = 1;
						group_remove_npc(g_group, h);
						if (check_hero3(h)) {
							sprintf(g_text_output_buf, g_ttx[0x1b7], name);
							dlg_message(g_dialog, g_text_output_buf);
						}
					}
					break;
				case 5:
					if (g_npc_months == 0x7f) quiet = 1;
					if (g_npc_months >= 2) {
						left = 1;
						group_remove_npc(g_group, h);
						if (check_hero3(h)) {
							sprintf(g_text_output_buf, g_ttx[0x1b7], name);
							dlg_message(g_dialog, g_text_output_buf);
						}
					}
					break;
				case 7:
					if (g_npc_months >= 2) {
						left = 1;
						group_remove_npc(g_group, h);
						if (check_hero3(h)) {
							sprintf(g_text_output_buf, g_ttx[0x1b7], name);
							dlg_message(g_dialog, g_text_output_buf);
						}
					}
					break;
				case 6:
					if (g_npc_months >= 2) {
						left = 1;
						group_remove_npc(g_group, h);
						if (check_hero3(h)) {
							sprintf(g_text_output_buf, g_ttx[0x1b7], name);
							dlg_message(g_dialog, g_text_output_buf);
						}
					}
					break;
				case 8:
					if (g_npc_months >= 2) {
						left = 1;
						group_remove_npc(g_group, h);
						g_npc8_days = 0;
						g_dragan_task = 1;
						if (check_hero3(h)) dlg_message(g_dialog, g_ttx[0x1b6]);
					}
					break;
				default:
					left = 1;
					group_remove_npc(g_group, h);
					if (check_hero3(h)) {
						sprintf(g_text_output_buf, g_ttx[0x1b7], name);
						dlg_message(g_dialog, g_text_output_buf);
					}
					break;
				}
			}
		}
		if (left) {
			/* Abschied ins Tagebuch */
			switch (npc) {
			case 1:
				diary_add(g_diary, g_ttx[0x17c], 0);
				break;
			case 3:
				if (drained) {
					diary_add(g_diary, g_ttx[0x17d], 0);
				} else {
					sprintf(g_text_output_buf, g_ttx[0x17b], name);
					diary_add(g_diary, g_text_output_buf, 0);
				}
				break;
			case 5:
				if (!quiet) {
					sprintf(g_text_output_buf, g_ttx[0x17b], name);
					diary_add(g_diary, g_text_output_buf, 0);
				}
				break;
			case 4:
			case 6:
			case 7:
			case 8:
				sprintf(g_text_output_buf, g_ttx[0x17b], name);
				diary_add(g_diary, g_text_output_buf, 0);
				break;
			default:
				break;
			}
			left = 0;
		}
	}
	g_group = saved;
	mem_free_v(name);
}

signed short load_npc(struct hero far *h, signed short n)
{
	unsigned short size;
	signed short group_no;

	size = sizeof(struct hero);
	if (ofile_open(g_ofile, g_npc_file_ids[n]) != -1) {
		ofile_read(g_ofile, h, size);
		ofile_close(g_ofile);
		group_no = g_group_no;
		h->group_no = group_no;
		return 1;
	} else {
		return 0;
	}
}

void add_npc(signed short n)
{
	signed short slot;
	signed short idx;
	struct hero far *h;

	g_npc_months = 99;
	groups_check_npc();
	if ((slot = group_free_slot(g_group)) != -1) {
		idx = 6;
		h = g_heroes + idx;
		if (load_npc(h, n)) {
			g_npc_months = 0;
			group_add_hero(g_group, h, slot);
			/* Begruessung im Tagebuch */
			switch (h->npc_id) {
			case 1:
				diary_add(g_diary, g_ttx[0xf8], 0);
				break;
			case 2:
				diary_add(g_diary, g_ttx[0xf9], 0);
				break;
			case 3:
				diary_add(g_diary, g_ttx[0xfa], 0);
				break;
			case 4:
				diary_add(g_diary, g_ttx[0xfb], 0);
				break;
			case 5:
				diary_add(g_diary, g_ttx[0xfc], 0);
				break;
			case 6:
				diary_add(g_diary, g_ttx[0x161], 0);
				break;
			case 7:
				diary_add(g_diary, g_ttx[0x186], 0);
				break;
			case 8:
				diary_add(g_diary, g_ttx[0x187], 0);
				break;
			default:
				break;
			}
			g_refresh = 1;
		}
	}
}
