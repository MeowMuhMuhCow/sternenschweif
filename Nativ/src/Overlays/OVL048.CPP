/* OVL048 - Overlay O048 (Segment 0x2019 in SCHWEIF.EXE): weitere Methoden der Klasse ItemUse (Gegenstand benutzen,
 * O047; Texte txt, Benutzer user, Plaetze/Gegenstaende slot/id/slot2/id2), aufgerufen ueber g_item_use_funcs.
 * use_keyring(): Schluesselbund 0x145 (in der Hand oder per get_item_pos gesucht) mit 27 Faechern (g_keyring_keys,
 *   Belegung box_has/box_set): allein benutzt -> Auswahl (dlg_choose mit bis zu 16 Namen ueber grammar_name) und
 *   Schluessel abnehmen (group_give_item), sonst Schluessel aus der anderen Hand bzw. per Auswahl einsortieren und aus dem
 *   Inventar nehmen (drop_item).
 * use_quest_items(): Gegenstand 0x146 bei der Gruppe mit 0x14e -> quest_anim, sonst Meldungen.
 * maintain_weapons(): fuer alle 31 Inventarplaetze mit Waffen (item_desc.weapon, nicht stackable, Platz nicht
 *   broken, bf > g_weapons_bf[..] + 1) den Bruchfaktor um 1 senken (inline dec_user_inv_bf, -99 = unzerbrechlich),
 *   Klang 0x74, Meldung.
 * combo_index(a, b): ungeordnetes Paar in g_item_combos {item1, item2, func} -> func oder -1.
 * in_recipe_book/on_keyring(item): steht item schon im Rezeptbuch 0xbf (g_recipe_items, 14) bzw. im Schluesselbund 0x145
 *   (g_keyring_keys, 26)? -> Meldung, 1.
 * item_anim_cb(frame, frames): Rueckruf der Animation (Strg-Q bricht ab, bei Bild 0x21 einmal Sprachausgabe 0x421).
 * quest_anim(): Texte, Animation 0x0c (fli_play_frames/fli_play) mit Palettenblende (pal_blend_*, Farbe
 *   g_item_anim_color by value), NSC 5 (add_npc), group_add_ap(0x15e), g_quest_anim_seen = 1.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL048.CPP   -> 2582/2582 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH; die freien Funktionen mit
 *   Kontextzeiger sind jetzt Methoden von ItemUse - gleicher Code)
 * - inline-Elementfunktion ItemUse::dec_user_inv_bf(i): Parameterkopie [bp-2], -Og-Zwischenwert [bp-0ah]
 *   (i * 27), Ausdruck "(x == -99 ? 0 : 1) && x > 0" -> mov al,0 / mov al,1 / or al,al, abschliessender
 *   jmp short ans Inline-Ende. this wird NICHT kopiert.
 * - Bitfelder hier "unsigned short b:1" wie im Header: shr ax / and ax,1 / or ax,ax ohne mov ah,0.
 * - Locals heissen pos (nicht slot: das waere das Element ItemUse::slot).
 * - Grundbruchfaktor ueber das eigene Public g_weapons_bf[table_index * 8] (nicht g_weapons_table[..].bf).
 * - Lokale Felder: idx[17] (char) bei [bp-16h], names[17][50]; dlg_choose bekommt nur names[0..15].
 * - combo_index: i und r OHNE register (i -> DI, r -> CX, Zeilenoffset i*6 als -Og-Wert in SI);
 *   "for (i = 0, r = -1; ...)" fuer die Reihenfolge xor di / mov cx.
 * - Rueckruf item_anim_cb als far-Funktionszeiger (Original: Stub-Einsprung 0x43, von cobjfix angepasst).
 * - Farbe als 3-Byte-struct by value (SPUSH@), wie OVL028.
 * - g_item_anim_snd (ds:5032) hat kein eigenes Public (cobjfix: g_4ff0+0x42).
 * Originalfehler: use_keyring sammelt bis zu 27 Namen/Indizes in names[17]/idx[17] (Stapelueberlauf ab 18
 *   Treffern), die Auswahl zeigt nur 16. on_keyring sucht nur i < 26 (Tabelle hat 27), in_recipe_book i <= 13: ein
 *   nicht gefundenes item prueft dann den fremden Index 26 bzw. den Index 14 ausserhalb der Tabelle. "sel = -2" im
 *   Leer-Zweig ist ueberfluessig.
 * - Phase B (19.10.2026): eigene Daten im Quelltext; g_item_use_funcs[20] als echte Methodenzeiger auf ItemUse
 *   (O047, O252 per EXTDEF, cobjfix_d benennt sie auf die Overlay-Einspruenge um; eigene O048-Methoden
 *   segmentrelativ).
 */
#include <stdio.h>
#include <string.h>
#include <bios.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:4d38..5038, dsana) - initialisierte Daten in Adressreihenfolge */
ITEMUSE_PMF g_item_use_funcs[20] = {	/* Methoden je usable_desc.func bzw. combo_index (0 = keine) */
	0,
	&ItemUse::use_arcano,		/* 1  O047 */
	&ItemUse::read_text,		/* 2 */
	&ItemUse::read_special,		/* 3 */
	&ItemUse::use_armatrutz,	/* 4 */
	&ItemUse::use_flim_flam,	/* 5 */
	&ItemUse::use_with_item,	/* 6 */
	&ItemUse::use_ignifaxius,	/* 7 */
	&ItemUse::poison_weapon,	/* 8 */
	&ItemUse::use_attack_item,	/* 9  O252 */
	&ItemUse::play_instrument,	/* 10 */
	&ItemUse::read_heptagon_book,	/* 11 */
	&ItemUse::use_light,		/* 12 */
	&ItemUse::show_hidden_enemies,	/* 13 */
	&ItemUse::write_recipe,		/* 14 */
	&ItemUse::read_recipe_book,	/* 15 */
	&ItemUse::show_note,		/* 16 */
	&ItemUse::use_keyring,	/* 17 O048 */
	&ItemUse::use_quest_items,	/* 18 */
	&ItemUse::maintain_weapons	/* 19 */
};
struct item_combo g_item_combos[80] = {
	{ 249, 41, 12 },
	{ 25, 41, 12 },
	{ 85, 41, 12 },
	{ 85, 65, 12 },
	{ 85, 25, 12 },
	{ 167, 36, 14 },
	{ 169, 36, 14 },
	{ 202, 36, 14 },
	{ 203, 36, 14 },
	{ 204, 36, 14 },
	{ 205, 36, 14 },
	{ 206, 36, 14 },
	{ 186, 36, 14 },
	{ 240, 36, 14 },
	{ 242, 36, 14 },
	{ 243, 36, 14 },
	{ 244, 36, 14 },
	{ 241, 36, 14 },
	{ 338, 36, 14 },
	{ 167, 191, 14 },
	{ 169, 191, 14 },
	{ 202, 191, 14 },
	{ 203, 191, 14 },
	{ 204, 191, 14 },
	{ 205, 191, 14 },
	{ 206, 191, 14 },
	{ 186, 191, 14 },
	{ 240, 191, 14 },
	{ 242, 191, 14 },
	{ 243, 191, 14 },
	{ 244, 191, 14 },
	{ 241, 191, 14 },
	{ 338, 191, 14 },
	{ 167, 29, 14 },
	{ 169, 29, 14 },
	{ 202, 29, 14 },
	{ 203, 29, 14 },
	{ 204, 29, 14 },
	{ 205, 29, 14 },
	{ 206, 29, 14 },
	{ 186, 29, 14 },
	{ 240, 29, 14 },
	{ 242, 29, 14 },
	{ 243, 29, 14 },
	{ 244, 29, 14 },
	{ 241, 29, 14 },
	{ 338, 29, 14 },
	{ 36, 191, 14 },
	{ 36, 29, 14 },
	{ 22, 318, 6 },
	{ 22, 239, 6 },
	{ 187, 325, 17 },
	{ 194, 325, 17 },
	{ 195, 325, 17 },
	{ 201, 325, 17 },
	{ 208, 325, 17 },
	{ 209, 325, 17 },
	{ 210, 325, 17 },
	{ 211, 325, 17 },
	{ 212, 325, 17 },
	{ 219, 325, 17 },
	{ 222, 325, 17 },
	{ 254, 325, 17 },
	{ 259, 325, 17 },
	{ 266, 325, 17 },
	{ 273, 325, 17 },
	{ 281, 325, 17 },
	{ 300, 325, 17 },
	{ 301, 325, 17 },
	{ 302, 325, 17 },
	{ 303, 325, 17 },
	{ 304, 325, 17 },
	{ 305, 325, 17 },
	{ 306, 325, 17 },
	{ 307, 325, 17 },
	{ 308, 325, 17 },
	{ 347, 325, 17 },
	{ 334, 326, 18 },
	{ 333, 326, 18 },
	{ -1, -1, -1 }
};
signed short g_recipe_items[14] = { 167, 169, 202, 203, 204, 205, 206, 186, 240, 242, 243, 244, 241, 338 };
signed short g_recipe_outcomes[14] = { 166, 168, 180, 239, 153, 147, 154, 145, 146, 237, 155, 238, 236, 339 };
signed short g_keyring_keys[26] = {
	187, 194, 195, 201, 208, 209, 210, 211, 212, 219, 222, 254, 259, 266, 273, 281,
	300, 301, 302, 303, 304, 305, 306, 307, 308, 347
};
struct item_list7 g_weapons_no_poison = { { 9, 317, 256, 19, 12, 257, -1 } };
signed short g_item_anim_snd = 0;
struct srgb g_item_anim_color = { 63, 0, 0 };

/* Schluesselbund ITEM_KEYRING: allein benutzt -> Schluessel auswaehlen und abnehmen, sonst einen Schluessel (aus der anderen
 * Hand bzw. per Auswahl) einsortieren und aus dem Inventar entfernen */
void ItemUse::use_keyring()
{
	signed short i;
	signed short pos;
	signed char idx[17];
	char names[17][50];
	register signed short n = 0;
	register signed short sel = -2;

	if (id == ITEM_KEYRING) pos = slot;
	else if (id2 == ITEM_KEYRING) pos = slot2;
	else pos = get_item_pos(user, ITEM_KEYRING);

	if (pos == -1) {
		sprintf(g_text_output_buf, txt[0x65], user->name);
		dlg_message(g_dialog, g_text_output_buf);
		return;
	}
	if (id == ITEM_KEYRING && id2 == -1) {
		for (i = 0; i <= 26; i++) {
#if FIX_0014
			/* Fix #14: hoechstens 16 Namen sammeln (Feld names[17], Auswahl zeigt 16) */
			if (n < 16 && box_has(user, pos, i)) {
				strcpy(names[n], grammar_name(g_item_names[g_keyring_keys[i]]));
#else
			if (box_has(user, pos, i)) {
				strcpy(names[n], grammar_name(g_item_names[g_keyring_keys[i]]));
#endif
				idx[n] = i;
				n++;
			}
		}
		if (n == 1) sel = 0;
		else if (n > 1)
			sel = dlg_choose(g_dialog, txt[0x6e], n, names[0], names[1], names[2], names[3], names[4],
				names[5], names[6], names[7], names[8], names[9], names[10], names[11], names[12],
				names[13], names[14], names[15]) - 1;
		if (sel != -2) {
#if FIX_0362
			/* Fix #362: Schluessel nur vom Bund nehmen, wenn er wirklich ins Gepaeck kam */
			if (group_give_item(g_group, g_keyring_keys[idx[sel]], 1) > 0)
				box_set(user, pos, idx[sel], 0);
#else
			group_give_item(g_group, g_keyring_keys[idx[sel]], 1);
			box_set(user, pos, idx[sel], 0);
#endif
		}
	} else {
		if (id != ITEM_KEYRING && !on_keyring(id)) {
			for (i = 0; i <= 26; i++) {
				if (g_keyring_keys[i] == id) {
					sel = 0;
					idx[0] = i;
				}
			}
		} else if (!on_keyring(id2)) {
			for (i = 0; i <= 26; i++) {
				if (g_keyring_keys[i] == id2) {
					sel = 0;
					idx[0] = i;
				}
			}
		} else {
			for (i = 0; i <= 26; i++) {
#if FIX_0014
				/* Fix #14: hoechstens 16 Namen sammeln (Feld names[17], Auswahl zeigt 16) */
				if (n < 16 && get_item_pos(user, g_keyring_keys[i]) != -1 && !box_has(user, pos, i)) {
#else
				if (get_item_pos(user, g_keyring_keys[i]) != -1 && !box_has(user, pos, i)) {
#endif
					strcpy(names[n], grammar_name(g_item_names[g_keyring_keys[i]]));
					idx[n] = i;
					n++;
				}
			}
			if (n == 1) sel = 0;
			else if (n > 1)
				sel = dlg_choose(g_dialog, txt[0x66], n, names[0], names[1], names[2], names[3], names[4],
					names[5], names[6], names[7], names[8], names[9], names[10], names[11], names[12],
					names[13], names[14], names[15]) - 1;
			else {
				sprintf(g_text_output_buf, txt[0x67], user->name);
				dlg_message(g_dialog, g_text_output_buf);
				sel = -2;
			}
		}
		if (sel != -2 && pos != -1) {
			box_set(user, pos, idx[sel], 1);
			drop_item(user, get_item_pos(user, g_keyring_keys[idx[sel]]), 1);
		}
	}
}

/* Gegenstand 0x146 (bei der Gruppe) mit 0x14e: nach Text 98 (g_heather_recipe_read) Zwischensequenz quest_anim, 0x146/0x14e/0x150
 * ablegen (vermutet: Abschluss einer Questkette) */
void ItemUse::use_quest_items()
{
	if (group_find_item(g_group, 0x146) != -1) {
		if (!g_heather_recipe_read) {
			sprintf(g_text_output_buf, txt[0x72], user->name, grammar_pronoun(user->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
		} else if (id == 0x14e) {
			quest_anim();
			group_drop_item(g_group, 0x146);
			group_drop_item(g_group, 0x14e);
			group_drop_item(g_group, 0x150);
		} else {
			dlg_message(g_dialog, txt[0x7c]);
		}
	} else {
		dlg_message(g_dialog, txt[0x78]);
	}
}

/* Bruchfaktor aller (nicht stapelbaren, heilen) Waffen des Benutzers um 1 senken, bis Grundwert + 1 (vermutet: Waffen
 * pflegen/schaerfen) */
void ItemUse::maintain_weapons()
{
	register signed short i;
	register signed short item;

	for (i = 0; i < 31; i++) {
		item = user->inv[i].item_id;
		if (item != 0) {
			if (g_items[item].weapon && !g_items[item].stackable && !user->inv[i].broken &&
				user->inv[i].bf > g_weapons_bf[g_items[item].table_index * 8] + 1)
				dec_user_inv_bf(i);
		}
	}
	snd_play(g_snd_fx, 0x74);
	sprintf(g_text_output_buf, txt[0x6f], user->name, grammar_pronoun(user->sex, 1, 1), grammar_pronoun(user->sex, 1, 1));
	dlg_message(g_dialog, g_text_output_buf);
}

/* ungeordnetes Paar (a, b) in g_item_combos -> Index in g_item_use_funcs oder -1 */
signed short ItemUse::combo_index(signed short a, signed short b)
{
	signed short i;
	signed short r;

	for (i = 0, r = -1; g_item_combos[i].item1 != -1 && r == -1; i++) {
		if (g_item_combos[i].item1 == a) {
			if (g_item_combos[i].item2 == b) r = g_item_combos[i].func;
		} else if (g_item_combos[i].item2 == a && g_item_combos[i].item1 == b) {
			r = g_item_combos[i].func;
		}
	}
	return r;
}

/* steht das Rezept item schon im Rezeptbuch 0xbf (14 Faecher, g_recipe_items)? -> Meldung, 1 */
signed short ItemUse::in_recipe_book(register signed short item)
{
	signed short pos;
	signed short ret;
	register signed short i;

	ret = 0;
	pos = get_item_pos(user, ITEM_RECIPE_BOOK);
	if (pos != -1) {
#if FIX_0015
		/* Fix #15: nicht gefundenes Rezept nicht als Fach 14 pruefen */
		for (i = 0; i <= 13; i++) {
			if (g_recipe_items[i] == item) break;
		}
		if (i <= 13 && box_has(user, pos, i)) {
#else
		for (i = 0; i <= 13; i++) {
			if (g_recipe_items[i] == item) break;
		}
		if (box_has(user, pos, i)) {
#endif
			dlg_message(g_dialog, txt[0x58]);
			ret = 1;
		}
	}
	return ret;
}

/* steckt id schon im Schluesselbund ITEM_KEYRING (g_keyring_keys)? -> Meldung, 1 */
signed short ItemUse::on_keyring(register signed short item)
{
	signed short pos;
	signed short ret;
	register signed short i;

	ret = 0;
	pos = get_item_pos(user, ITEM_KEYRING);
	if (pos != -1) {
#if FIX_0015
		/* Fix #15: alle 27 Faecher durchsuchen, nicht gefundenes Kraut nicht pruefen */
		for (i = 0; i <= 26; i++) {
			if (g_keyring_keys[i] == item) break;
		}
		if (i <= 26 && box_has(user, pos, i)) {
#else
		for (i = 0; i < 26; i++) {
			if (g_keyring_keys[i] == item) break;
		}
		if (box_has(user, pos, i)) {
#endif
			dlg_message(g_dialog, txt[0x68]);
			ret = 1;
		}
	}
	return ret;
}

/* Rueckruf der Animation (quest_anim): Strg-Q bricht ab, bei Bild 0x21 einmal Sprachausgabe 0x421 */
signed short item_anim_cb(register unsigned short frame, unsigned short frames)
{
	register signed short ret = 0;

	if (bioskey(1) && bioskey(0) == 0x1011) {
		ret = 1;
		g_item_anim_snd = 0;
	}
	if (g_item_anim_snd == 0 && frame == 0x21) {
		g_item_anim_snd = 1;
		snd_speech(g_snd_speech, 0x421);
	}
	if (frame >= frames) {
		ret = 1;
		g_item_anim_snd = 0;
	}
	return ret;
}

/* Zwischensequenz: Animation 0x0c mit Palettenblende, danach NSC 5, AP 0x15e fuer alle, g_quest_anim_seen = 1 */
void ItemUse::quest_anim()
{
	struct srgb col;
	unsigned short music;

	dlg_message(g_dialog, txt[0x63]);
	dlg_message(g_dialog, txt[0x64]);
	if (g_speech_ext_avail) {
		col = g_item_anim_color;
		music = g_music_id_req & 0x3fff;
		g_pp20_index = -1;
		mouse_hide(0);
		if (g_cd_music) snd_obj_stop(g_snd_music);
		pal_blend_to_color((struct srgb far *)0, col, 0x19);
		fli_play_frames(0x0c, 1, 0);
		pal_blend_from_color(col, g_fli_pal, 0x19);
		fli_play(0x0c, -1, item_anim_cb, 1);
		pal_blend(g_fli_pal, 0x19);
		snd_play(g_snd_music, music);
		mouse_show(0);
		g_refresh = 1;
	}
	add_npc(5);
	group_add_ap(g_group, 0x15e);
	g_quest_anim_seen = 1;
}
