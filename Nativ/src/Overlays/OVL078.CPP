/* OVL078 - Overlay O078 (Segment 0x20d0 in SCHWEIF.EXE): Truhe - class Chest (s_class.h, 0x55 Bytes; +0..0x33 =
 * Datensatz nr aus Datei 0x9d: Geld money +2, Schluessel key +4, Sonderfunktionen special/fail_special +6/+7,
 * Versuche tries +8, verschlossen locked +9, Truhenart type +0a, Meldung msg +0b, 10 Gegenstaende items +0c (je
 * id/Anzahl)). Gegenstueck zu O108 (class Lock), aehnlicher Aufbau.
 * STUFE D1 (wovd1, 19.10.2026): asm-strcat durch echtes strcat(.., "..") ersetzt - Code GLEICH, das Literal
 *   im eigenen _DATA bildet wovd1/cobjfix_d.py auf ds:5a0b ab (Bytes geprueft).
 * trap_2w6 .. special_quest: Methoden fuer die Methodenzeiger special_func/fail_func (Fallen: Schaden dice_roll(2/3/5,
 * 6), Text txt[11..15]; trap_poison Gift nur fuer Magier (hero_get_poison, 5 Minuten); special_key Schluessel 0x111
 * (group_find_key) und Gegenstand 0x78 (group_find_item); special_quest nach g_eolan_state). Chest(nr, open): Datensatz
 * lesen (ofile_open/_seek/_read), bei open == 1 Texte txt_load_list(g_font, 0x9e, 0x28), init(), ab 10 Versuchen
 * Hinweis, try_open: Erfolg -> Meldung show_msg, leer (count_items) bzw. Gegenstaende nehmen take_items; sonst ggf.
 * Falle test_attrib(h, ATTRIB_JZ, -tries) mit Schaden W3 (mind. LE 1). ~Chest() schreibt den Datensatz zurueck.
 * init(): Held get_first_active_hero, Methodenzeiger aus g_chest_special_default/g_chest_fail_default bzw.
 * g_chest_special[special]/g_chest_fail[fail_special], Werte force/pick_mod/spell_mod aus g_chest_types[type].
 * try_open: nur verschlossen oder Art 15; special_func, Schluessel bei Art 11/13 (group_find_key(key)), sonst
 * Auswahl dlg_choose: force_open Aufbrechen (KK cur + mod gegen force), pick Dietrich (ITEM_PICKLOCKS, test_skill
 * TA_SCHLOESSER), foramen Zauber SP_FORAMEN (test_spell, AE >= 6); Misserfolg ruft fail_func oder zeigt Text.
 * take_items: Auswahlliste list_items (Anzahl + Name grammar_plural aus g_item_names, 40 Bytes je Zeile), Menge per
 * dlg_input, Geld ITEM_MONEY -> group_add_money, sonst group_give_item; Eintrag nachruecken oder Anzahl mindern;
 * Geld money * 10. set_item(id, n, k): Eintrag setzen.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL078.CPP   -> 4865/4865 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Ohne -Og 32 Bytes zu kurz (Epilogkopien, h->name-Zwischenwert in DI in special_key), ohne -Oi 3120
 *   Abweichungen.
 * - Methodenzeiger 8 Bytes wie OVL108 ("class Chest;" vor dem typedef CHEST_PMF); Konstruktor mit operator new
 *   (0x55) und Rueckgabe this, Destruktor mit delete bei flags & 1 - beides automatisch.
 * - special_key: "if (..) { ..; return 1; }" OHNE else vor dem zweiten Text (sonst toter jmp hinter der
 *   Epilogkopie); try_open ebenso: "if (special_func) {..return..} else { if (Schluessel) {..return 1;} Auswahl }".
 * - list_items: "(cnt > 1 ? 1 : 0) ? 4 : 0" ergibt mov al,1/mov al,0/or al,al; erst "((cnt > 1) != 0) ? 4 : 0"
 *   liefert mov ax,1 / xor ax,ax / or ax,ax. Zwischenwert i * 40 ([bp-8]) als eigene Variable off; strcat mit
 *   1-Zeichen-Literal " " (ds:5a0b; sein Nullbyte und das Fuellbyte 5a0c/5a0d gehoeren noch zu O078).
 * - Phase B (19.10.2026): eigene Daten im Quelltext, die Tabellen g_chest_special/g_chest_fail als echte
 *   Methodenzeiger (&Chest::special_key ...). BCC legt sie als segmentrelative Fernzeiger auf OVL078_TEXT an,
 *   TLINK setzt wie im Original den Einsprung des Overlay-Stubs ein (cobjfix_d prueft gegen das Original-DS).
 * - count_items ohne Aufrufe: "for (i = k = 0; ..)" -> k in CX, i in DX.
 * - hero::is_mage() inline "return type >= 7;" -> mov ax,1 / xor ax,ax / or ax,ax (Guide-Muster).
 * Originalfehler: take_items schreibt bei Geld (ITEM_MONEY) immer den ganzen Stapel gut (k * 100), mindert ihn
 *   aber nur um die gewaehlte Menge -> Geld vermehrt sich, wenn man nur einen Teil nimmt.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5990..5a0c, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct lock_type g_chest_types[17] = {	/* je Truhenart: Aufbrechen (KK), Zuschlag Schloesser, Zuschlag Foramen */
	{ 11, 0, 3 },	/* 0 */
	{ 12, 3, 5 },	/* 1 */
	{ 13, 6, 6 },	/* 2 */
	{ 14, 4, 8 },	/* 3 */
	{ 15, 8, 10 },	/* 4 */
	{ 16, 12, 12 },	/* 5 */
	{ 17, 5, 10 },	/* 6 */
	{ 18, 10, 15 },	/* 7 */
	{ 19, 15, 20 },	/* 8 */
	{ 20, 6, 12 },	/* 9 */
	{ 20, 10, 18 },	/* 10 */
	{ 20, 18, 27 },	/* 11 */
	{ 21, 30, 49 },	/* 12 */
	{ 99, 99, 99 },	/* 13 */
	{ 99, 99, 99 },	/* 14 */
	{ 99, 99, 99 },	/* 15 */
	{ 0, 157, 0 } 	/* 16 */
};
CHEST_PMF g_chest_special[2] = {	/* Sonderbehandlung je Chest.special */
	&Chest::special_key, &Chest::special_quest
};
CHEST_PMF g_chest_fail[5] = {		/* Fehlschlag-Folge (Falle) je Chest.fail_special */
	&Chest::trap_2w6, &Chest::trap_always, &Chest::trap_3w6, &Chest::trap_5w6, &Chest::trap_poison
};
CHEST_PMF g_chest_special_default = 0;	/* keine Sonderbehandlung */
CHEST_PMF g_chest_fail_default = 0;	/* keine Falle */

signed short Chest::trap_2w6()
{
	if (tries == 1) {
		sprintf(g_text_output_buf, txt[12], hero->name);
		dlg_message(g_dialog, g_text_output_buf);
		sub_hero_le(hero, dice_roll(2, 6, 0));
	} else
		dlg_message(g_dialog, txt[13]);
	return 0;
}

signed short Chest::trap_3w6()
{
	if (tries == 1) {
		sprintf(g_text_output_buf, txt[14], hero->name);
		dlg_message(g_dialog, g_text_output_buf);
		sub_hero_le(hero, dice_roll(3, 6, 0));
	} else
		dlg_message(g_dialog, txt[13]);
	return 0;
}

signed short Chest::trap_always()
{
	sprintf(g_text_output_buf, txt[11], hero->name);
	dlg_message(g_dialog, g_text_output_buf);
	sub_hero_le(hero, dice_roll(2, 6, 0));
	return 0;
}

signed short Chest::trap_poison()
{
	if (tries == 1 && hero->is_mage()) {
		sprintf(g_text_output_buf, txt[22], hero->name, grammar_pronoun(hero->sex, 0, 0));
		dlg_message(g_dialog, g_text_output_buf);
		hero_get_poison(hero, 9);
		timewarp_minutes(g_date, 5L);
	} else
		dlg_message(g_dialog, txt[13]);
	return 0;
}

signed short Chest::trap_5w6()
{
	if (tries <= 2) {
		sprintf(g_text_output_buf, txt[15], hero->name);
		dlg_message(g_dialog, g_text_output_buf);
		sub_hero_le(hero, dice_roll(5, 6, 0));
	} else
		dlg_message(g_dialog, txt[13]);
	return 0;
}

signed short Chest::special_key()
{
	char far *buf;
	struct hero far *h;
	register signed short i;

	if ((i = group_find_key(g_group, 0x111)) != -1) {
		h = g_group->heroes[i];
		if (!check_hero(h))
			h = get_random_hero(g_group);
		if ((i = group_find_item(g_group, 0x78)) != -1) {
			h = g_group->heroes[i];
			if (!check_hero(h))
				h = get_random_hero(g_group);
			buf = new char[300];
			sprintf(buf, txt[23], h->name, grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, buf);
			delete buf;
			return 1;
		}
		buf = new char[300];
		sprintf(buf, txt[24], h->name, h->name);
		dlg_message(g_dialog, buf);
		delete buf;
	} else
		dlg_message(g_dialog, txt[25]);
	return 0;
}

signed short Chest::special_quest()
{
	char far *buf;
	struct hero far *h;
	register signed short r = 0;

	buf = new char[300];
	h = get_first_active_hero(g_group);
	if (g_eolan_state == 2) {
		if (tries != 0) {
			sprintf(buf, txt[29], h->name);
			dlg_message(g_dialog, buf);
		}
		r = 1;
	} else {
		sprintf(buf, txt[28], h->name);
		dlg_message(g_dialog, buf);
	}
	delete buf;
	return r;
}

Chest::Chest(signed short n, signed short open)
{
	signed short r;
	unsigned long pos;

	nr = n;
	pos = (unsigned short)(n * 0x34);
	ofile_open(g_ofile, 0x9d);
	ofile_seek(g_ofile, pos, 0);
	ofile_read(g_ofile, this, 0x34);
	ofile_close(g_ofile);
	if (open == 1) {
		txt_load_list(g_font, 0x9e, 0x28, &txt_buf, &txt);
		init();
		if (locked != 0 && tries > 10) {
			sprintf(g_text_output_buf, txt[0], hero->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
		if ((r = try_open()) == 1) {
			if (msg != 0 && locked != 0)
				show_msg(msg);
			locked = 0;
			if (!count_items())
				dlg_message(g_dialog, txt[3]);
			else
				take_items();
		} else if (r != -1) {
			if (hero->le > 0) {
				if (test_attrib(hero, ATTRIB_JZ, -tries) > 0) {
					sprintf(g_text_output_buf, txt[8], hero->name);
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(hero, random_schweif(3));
					if (hero->le <= 0)
						hero->le = 1;
				}
			}
		}
	}
}

Chest::~Chest()
{
	unsigned long pos;

	pos = (unsigned short)(nr * 0x34);
	ofile_create(g_ofile, 0x9d, 0x8004);
	ofile_seek(g_ofile, pos, 0);
	ofile_write(g_ofile, this, 0x34);
	ofile_close(g_ofile);
	mem_free(txt_buf);
	mem_free(txt);
}

void Chest::init()
{
	hero = get_first_active_hero(g_group);
	special_func = g_chest_special_default;
	if (special != -1)
		special_func = g_chest_special[special];
	fail_func = g_chest_fail_default;
	if (fail_special != -1)
		fail_func = g_chest_fail[fail_special];
	force = g_chest_types[type].force;
	pick_mod = g_chest_types[type].pick_mod;
	spell_mod = g_chest_types[type].spell_mod;
}

signed short Chest::try_open()
{
	char far *buf;
	struct hero far *h;
	register signed short i;

	if (locked != 0 || type == 15) {
		if (special_func) {
			tries++;
			return (this->*special_func)();
		} else {
			if ((type == 11 || type == 13) && (i = group_find_key(g_group, key)) != -1) {
				h = g_group->heroes[i];
				if (!check_hero(h))
					h = get_random_hero(g_group);
				buf = new char[300];
				sprintf(buf, txt[16], h->name, grammar_pronoun(h->sex, 0, 0), grammar_article(0x8002, key, 0),
					grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, buf);
				delete buf;
				return 1;
			}
			if ((i = dlg_choose(g_dialog, txt[18], 3, txt[19], txt[20], txt[21])) != -1) {
				if (i == 1)
					return force_open();
				else if (i == 2)
					return pick();
				else
					return foramen();
			} else
				return -1;
		}
	} else
		return 1;
}

signed short Chest::pick()
{
	if ((hero = get_hero_with_item(g_group, ITEM_PICKLOCKS)) != 0) {
		tries++;
		if (test_skill(hero, TA_SCHLOESSER, pick_mod) >= 0)
			return 1;
		else if (fail_func)
			return (this->*fail_func)();
		else {
			sprintf(g_text_output_buf, txt[2], hero->name);
			dlg_message(g_dialog, g_text_output_buf);
			return 0;
		}
	} else {
		dlg_message(g_dialog, txt[1]);
		return -1;
	}
}

signed short Chest::count_items()
{
	signed short k;
	signed short i;

	for (i = k = 0; i < 10; i++)
		if (items[i].id != -1)
			k++;
	return k;
}

signed short Chest::foramen()
{
	if ((hero = get_active_mage(g_group, txt[7])) != 0) {
		tries++;
		if (hero->ae >= 6 && test_spell(hero, SP_FORAMEN, spell_mod) > 0) {
			sub_ae_splash(hero, 6);
			return 1;
		} else {
			sub_ae_splash(hero, 3);
			if (fail_func)
				return (this->*fail_func)();
			else {
				sprintf(g_text_output_buf, txt[5], hero->name);
				dlg_message(g_dialog, g_text_output_buf);
				return 0;
			}
		}
	} else {
		dlg_message(g_dialog, txt[4]);
		return -1;
	}
}

signed short Chest::force_open()
{
	tries++;
	if ((signed char)(hero->attrib[ATTRIB_KK].cur + hero->attrib[ATTRIB_KK].mod) >= force)
		return 1;
	else if (fail_func)
		return (this->*fail_func)();
	else {
		sprintf(g_text_output_buf, txt[6], hero->name);
		dlg_message(g_dialog, g_text_output_buf);
		return 0;
	}
}

void Chest::take_items()
{
	signed short old_mode;
	signed short take;
	char far *buf;
	struct chest_item far *it;
	signed short width;
	register signed short k;
	register signed short sel;

	old_mode = dlg_set16(g_dialog, 4);
	buf = new char[400];
	do {
		k = list_items(buf);
		if (k) {
			sel = dlg_choose(g_dialog, txt[9], k, buf, buf + 0x28, buf + 0x50, buf + 0x78, buf + 0xa0,
				buf + 0xc8, buf + 0xf0, buf + 0x118, buf + 0x140, buf + 0x168) - 1;
			if (sel != -2) {
				it = &items[sel];
				do {
					k = it->qty;
					if (k >= 100)
						width = 3;
					else if (k >= 10)
						width = 2;
					else
						width = 1;
					take = k > 1 ? dlg_input(g_dialog, width, txt[10], 0, k, k) : k;
					if (take > k)
						take = k;
				} while (take < 0);
				if (take != 0) {
					if (it->id == ITEM_MONEY)
#if FIX_0123
						/* Fix #123: nur die genommene Menge gutschreiben (Geldvermehrung) */
						group_add_money(g_group, (signed long)take * 100);
#else
						group_add_money(g_group, (signed long)k * 100);
#endif
					else
						take = group_give_item(g_group, it->id, take);
					if (take == k) {
						do {
							items[sel] = items[sel + 1];
							sel++;
						} while (items[sel - 1].id != -1);
					} else if (take != 0)
						items[sel].qty -= take;
					else
						sel--;
				}
			}
		} else
			break;
	} while (sel != -2);
	if (money != 0) {
		sprintf(g_text_output_buf, txt[26], money);
		dlg_message(g_dialog, g_text_output_buf);
		group_add_money(g_group, money * 10L);
		money = 0;
	}
	k = list_items(buf);
	if (!k)
		dlg_message(g_dialog, txt[27]);
	delete buf;
	dlg_set16(g_dialog, old_mode);
}

signed short Chest::list_items(char far *buf)
{
	signed short i = 0;
	signed short id;
	signed short cnt;
	signed short off;

	while ((id = items[i].id) != -1) {
		buf[off = i * 40] = 0;
		if ((cnt = items[i].qty) > 1) {
			itoa(cnt, buf + off, 10);
			strcat(buf + (off = i * 40), " ");
		}
		strcat(buf + i++ * 40, grammar_plural(((cnt > 1) != 0) ? 4 : 0, g_item_names[id], 0));
	}
	return i;
}

void Chest::show_msg(signed char k)
{
	char far *s = 0;

	switch (k) {
	case 1:
		s = txt[17];
		break;
	}
	if (s)
		dlg_message(g_dialog, s);
}

void Chest::set_item(signed short id, unsigned char qty, signed short k)
{
	items[k].id = id;
	items[k].qty = qty;
}
