/* OVL254 - Overlay O254 (Segment 0x2486 in SCHWEIF.EXE): Wirkungen verzehrter Kraeuter/Traenke - Methoden von ItemUse
 * (Aufruf aus consume O253; Texte txt, Meldungsflag verbose, Benutzer user, Ziel target, Gegenstand id).
 * item_poison_type(item): Giftart POISON_* zu Shurinknollengift/Araxgift/Angstgift/Schlafgift/Kukris/
 *   Kroetenschemelgift (sonst POISON_SHURINKNOLLE) fuer hero_get_poison.
 * herb_effect(): Gulmondblatt KK + 2 (add_effect 0xfd20 Takte, LE + 2), Einbeere/Wirselkraut/0x10b/0x135 LE-Heilung
 *   (W6, 10, W(10..18), 2W6, begrenzt auf LE-Max - LE, add_hero_le), Belmart/Menchal heilen Gift bis Stufe 5/10
 *   (hero_cure_poison) + MR + 1/2 fuer 0x1fa40 Takte, Atmonbluete Talente 9..18 + 2 (0x6978 Takte), 0x136 Schaden 2W6
 *   (sub_hero_le am Ende), Tarnele ruhe_koerper = 1, Thonnysbluete Text; Meldung nur mit verbose und nichtleerem
 *   Puffer.
 * potion_effect(): Heiltraenke (10, W20 + 10, 0x128: 40; Einheit txt[42/43] per strcpy/strcat nach g_text_buf2),
 *   Zaubertraenke (nur Zauberkundige: 10 bzw. 30 AE), Wunderkur (LE-Grundwert zurueck, Krankheit 2/3 und Gift < 7
 *   heilen), Schlaftrunk ruhe_koerper = 3, Gegengift (Gift < 5), Antikrankheitselixier (alle Krankheiten), 0x10c
 *   Schaden 30, 0x155 Bit bd2, Goldleim/Lotusgift (Gift + Schaden), Bannstaub (AE-Verlust), Giftgegenstaende ->
 *   hero_get_poison(item_poison_type(id)); Flasche zurueck per give_hero_new_item(target, Glas- bzw. Bronzeflasche, 2,
 *   1); Meldung in Fensterbreite 5.
 *
 * BCC -mlarge -3 -Ot -Oi -c -Y OVL254.CPP   -> 5224/5224 Bytes GLEICH (18.10.2026; mit -Og ebenso), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH; Methoden von ItemUse)
 * - item_poison_type: gemeinsamer Epilog mit "mov ax,k / jmp short" NUR als verschachtelter ?: ("return item == .. ?
 *   1 : .."); if-return-Ketten erzeugen in dieser Funktion ohne Locals Epilogkopien (pop bp/retf) auch OHNE -Og, mit
 *   else zusaetzlich je einen toten Epilog.
 * - herb_effect: "dmg = val = dice_roll(..)" speichert dmg aus AX; das Original hat "val = ..; dmg = val;"
 *   (mov [bp-0ah],si).
 * - "val == 1 ? (char far *)g_str_empty_7718 : txt[51]" - fuenf benachbarte ""-Literale ds:7718..771c (cobjfix ordnet
 *   sie _s_7713+5..9 zu). C++ fuer das inline-Praedikat is_mage() (mov ax,1 / xor ax,ax / or ax,ax).
 * - add_effect mit Wort-Argumenten fuer Betrag/Art (gecasteter Funktionszeiger add_effect_cw).
 * - -Oi: strcpy/strcat(g_text_buf2, txt[..]) als Intrinsics (push si/di in potion_effect); Ziel-Bitfeld +bd per or byte.
 * Originalfehler: herb_effect Fall Gulmondblatt: "sprintf(txt[74], user->name)" - der Zielpuffer g_text_output_buf
 *   fehlt, der Heldenname ueberschreibt Text 74, angezeigt wird der alte Pufferinhalt. potion_effect Fall starker
 *   Zaubertrank nennt die Einheit txt[42] (Lebenspunkte) statt txt[52] wie der Zaubertrank. Lotusgift ruft als
 *   einziger Schadensfall kein give_hero_new_item (keine leere Flasche).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:7718..7784, dsana) - initialisierte Daten in Adressreihenfolge */
char g_str_empty_7718[1] = "";
char g_str_empty_7719[1] = "";
char g_str_empty_771a[1] = "";
char g_str_empty_771b[1] = "";
char g_str_empty_771c[1] = "";
char g_unused_771d[1] = { 0 };	/* ds:771d, unbenutzt (vermutet Fuellbyte vor der Tabelle) */
/* ds:771e: Textdatei je Gespraechsthema 7..0x39 (Conv::unlock_topic/build_list in O257 lesen g_conv_topic_tab[t - 7];
 * BCC faltet "- 7" in die Adresse ds:7710 = MAP-Name g_7710). Themen 7..0x29 TALKS_00..TALKS_34.LTX (0x1a4..0x1c6:
 * Kvirasim, Gashok, Lowangen, Tiefhusen, Tjolmar, Finsterkoppen, Ansvell, Salamandersteine, Finsterkamm, Svellttal,
 * Svelltsuempfe, Thasch, Blutzinnen, Rorwhed, Handel, Kraeuter, Waffen, Krankheiten, Gifte, Schmiedekunst, Jagd und
 * die Goetter Praios .. Ifirn), 0x2a..0x38 ohne eigene Datei (Fragen aus STDTALKS.LTX), 0x39 TALKS_35.LTX
 * "Wisst Ihr vielleicht, was es mit der abgebrannten Muehle auf sich hat?" */
signed short g_conv_topic_tab[51] = {
	0x1a4, 0x1a5, 0x1a6, 0x1a7, 0x1a8, 0x1a9, 0x1aa, 0x1ab,
	0x1ac, 0x1ad, 0x1ae, 0x1af, 0x1b0, 0x1b1, 0x1b2, 0x1b3,
	0x1b4, 0x1b5, 0x1b6, 0x1b7, 0x1b8, 0x1b9, 0x1ba, 0x1bb,
	0x1bc, 0x1bd, 0x1be, 0x1bf, 0x1c0, 0x1c1, 0x1c2, 0x1c3,
	0x1c4, 0x1c5, 0x1c6, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0x412
};

#define add_effect_cw ((void (*)(struct hero far *, signed short, signed long, signed char, signed short, signed short))add_effect)

/* Giftart POISON_* eines Giftgegenstands (sonst Shurinknolle) */
signed short item_poison_type(register signed short item)
{
	return item == ITEM_SHURIN_POISON ? POISON_SHURINKNOLLE : item == ITEM_ARAX_POISON ? POISON_ARAX :
		item == ITEM_ANGST_POISON ? POISON_ANGSTGIFT : item == ITEM_SLEEP_POISON ? POISON_SCHLAFGIFT :
		item == ITEM_KUKRIS ? POISON_KUKRIS : item == ITEM_KROETEN_POISON ? POISON_KROETENSCHEMEL : POISON_SHURINKNOLLE;
}

/* Wirkung eines Krauts (Meldung nur mit verbose und nichtleerem Puffer) */
void ItemUse::herb_effect()
{
	signed short i;
	signed short pois;
	struct hero_attrib far *kk;
	signed short dmg = 0;
	register signed short val;
	register signed short max;

	switch (id) {
	case ITEM_GULMOND_LEAF:
		kk = &user->attrib[ATTRIB_KK];
		if (kk->cur > kk->normal)
#if FIX_0130
			/* Fix #130: in den Ausgabepuffer schreiben statt in Text 74 */
			sprintf(g_text_output_buf, txt[74], user->name);
#else
			sprintf(txt[74], user->name);
#endif
		else {
			sprintf(g_text_output_buf, txt[35], user->name);
			dlg_message(g_dialog, g_text_output_buf);
			*g_text_output_buf = 0;
			val = get_free_effect(user);
			add_effect_cw(user, val, 0xfd20L, ATTRIB_KK, 2, EFFECT_ATTRIB);
			add_hero_le(user, 2);
		}
		break;
	case ITEM_EINBEERE:
		val = random_schweif(6);
		max = user->le_max - user->le;
		if (val > max)
			val = max;
		add_hero_le(user, val);
		sprintf(g_text_output_buf, txt[36], val, txt[42], val == 1 ? (char far *)g_str_empty_7718 : txt[51]);
		break;
	case ITEM_BELMART:
		pois = hero_is_poisoned(user);
		if (pois != 0 && pois <= 5)
			hero_cure_poison(user);
		val = get_free_effect(user);
		add_effect_cw(user, val, 0x1fa40L, 0, 1, EFFECT_MR);
		sprintf(g_text_output_buf, txt[37], user->name);
		break;
	case ITEM_MENCHAL:
		pois = hero_is_poisoned(user);
		if (pois != 0 && pois <= 10)
			hero_cure_poison(user);
		val = get_free_effect(user);
		add_effect_cw(user, val, 0x1fa40L, 0, 2, EFFECT_MR);
		sprintf(g_text_output_buf, txt[37], user->name);
		break;
	case ITEM_ATMONBLUETE:
		for (i = 9; i < 0x13; i++) {
			val = get_free_effect(user);
			add_effect_cw(user, val, 0x6978L, i, 2, EFFECT_SKILL);
		}
		sprintf(g_text_output_buf, txt[38], user->name);
		break;
	case ITEM_WHIRLWEED:
		val = 10;
		max = user->le_max - user->le;
		if (max < val)
			val = max;
		add_hero_le(user, val);
		sprintf(g_text_output_buf, txt[39], val, txt[42], val == 1 ? (char far *)g_str_empty_7719 : txt[51]);
		break;
	case 0x10b:
		val = random_interval(10, 0x12);
		max = user->le_max - user->le;
		if (max < val)
			val = max;
		add_hero_le(user, val);
		sprintf(g_text_output_buf, txt[39], val, txt[42], val == 1 ? (char far *)g_str_empty_771a : txt[51]);
		break;
	case 0x135:
		val = dice_roll(2, 6, 0);
		max = user->le_max - user->le;
		if (max < val)
			val = max;
		add_hero_le(user, val);
		sprintf(g_text_output_buf, txt[36], val, txt[42], val == 1 ? (char far *)g_str_empty_771b : txt[51]);
		break;
	case 0x136:
		val = dice_roll(2, 6, 0);
		dmg = val;
		sprintf(g_text_output_buf, txt[68], val, txt[42], val == 1 ? (char far *)g_str_empty_771c : txt[92]);
		break;
	case ITEM_TARNELE:
		user->ruhe_koerper = 1;
		sprintf(g_text_output_buf, txt[86], user->name);
		break;
	case ITEM_THONNYS:
		sprintf(g_text_output_buf, txt[87], user->name);
		break;
	}
	if (verbose != 0 && *g_text_output_buf != 0)
		dlg_message(g_dialog, g_text_output_buf);
	if (dmg != 0)
		sub_hero_le(user, dmg);
}

/* Wirkung eines Tranks bzw. Gifts, leere Flasche zurueck */
void ItemUse::potion_effect()
{
	signed short n;
	signed short old;
	signed short r1;
	signed short r2;
	signed short dmg = 0;

	switch (id) {
	case ITEM_LE_POTION:
		n = user->le_max - user->le;
		if (n > 10)
			n = 10;
		strcpy(g_text_buf2, txt[42]);
		if (n != 1)
			strcat(g_text_buf2, txt[43]);
		add_hero_le(user, n);
		give_hero_new_item(target, ITEM_FLASK_GLASS, 2, 1);
		sprintf(g_text_output_buf, txt[44], user->name, n, g_text_buf2);
		break;
	case ITEM_STRONG_LE_POTION:
		n = dice_roll(1, 0x14, 10);
		if (user->le_max - user->le < n)
			n = user->le_max - user->le;
		add_hero_le(user, n);
		give_hero_new_item(target, ITEM_FLASK_BRONZE, 2, 1);
		strcpy(g_text_buf2, txt[42]);
		if (n != 1)
			strcat(g_text_buf2, txt[43]);
		sprintf(g_text_output_buf, txt[44], user->name, n, g_text_buf2);
		break;
	case 0x128:
		n = 0x28;
		if (user->le_max - user->le < n)
			n = user->le_max - user->le;
		add_hero_le(user, n);
		give_hero_new_item(target, ITEM_FLASK_BRONZE, 2, 1);
		strcpy(g_text_buf2, txt[42]);
		if (n != 1)
			strcat(g_text_buf2, txt[43]);
		sprintf(g_text_output_buf, txt[44], user->name, n, g_text_buf2);
		break;
	case 0x10c:
		dmg = 0x1e;
		give_hero_new_item(target, ITEM_FLASK_BRONZE, 2, 1);
		sprintf(g_text_output_buf, txt[67], hero_name_gen(user, 1));
		break;
	case ITEM_WUNDERKUR:
		user->le_max += user->le_mod;
		user->le_mod = 0;
		add_hero_le(user, user->le_max);
		r1 = hero_is_diseased(user);
		if (r1 == 2 || r1 == 3)
			hero_cure_disease(user);
		r2 = hero_is_poisoned(user);
		if (r2 != 0 && r2 < 7)
			hero_cure_poison(user);
		give_hero_new_item(target, ITEM_FLASK_GLASS, 2, 1);
		if (!hero_is_diseased(user) && !hero_is_poisoned(user))
			sprintf(g_text_output_buf, txt[45], user->name);
		else
			sprintf(g_text_output_buf, txt[122], hero_name_gen(user, 1));
		break;
	case ITEM_SLEEPING_POTION:
		user->ruhe_koerper = 3;
		give_hero_new_item(target, ITEM_FLASK_GLASS, 2, 1);
		sprintf(g_text_output_buf, txt[46], user->name);
		break;
	case ITEM_AE_POTION:
		if (user->is_mage()) {
			n = user->ae_max - user->ae;
			if (n > 10)
				n = 10;
			add_hero_ae(user, n);
			give_hero_new_item(target, ITEM_FLASK_GLASS, 2, 1);
			strcpy(g_text_buf2, txt[52]);
			if (n != 1)
				strcat(g_text_buf2, txt[43]);
			sprintf(g_text_output_buf, txt[47], user->name, n, g_text_buf2);
		} else {
			strcpy(g_text_output_buf, txt[48]);
			give_hero_new_item(target, ITEM_FLASK_GLASS, 2, 1);
		}
		break;
	case ITEM_STRONG_AE_POTION:
		if (user->is_mage()) {
			n = user->ae_max - user->ae;
			if (n > 30)
				n = 30;
			add_hero_ae(user, n);
			give_hero_new_item(target, ITEM_FLASK_GLASS, 2, 1);
			if (user->ae >= user->ae_max)
				sprintf(g_text_output_buf, txt[49], user->name);
			else {
#if FIX_0131
				/* Fix #131: Einheit Astralpunkte wie beim Zaubertrank */
				strcpy(g_text_buf2, txt[52]);
				if (n != 1)
#else
				strcpy(g_text_buf2, txt[42]);
				if (n != 1)
#endif
					strcat(g_text_buf2, txt[43]);
				sprintf(g_text_output_buf, txt[47], user->name, n, g_text_buf2);
			}
		} else {
			strcpy(g_text_output_buf, txt[48]);
			give_hero_new_item(target, ITEM_FLASK_GLASS, 2, 1);
		}
		break;
	case ITEM_ANTIVENOM:
		r2 = hero_is_poisoned(user);
		if (r2 != 0) {
			if (r2 < 5) {
				hero_cure_poison(user);
				give_hero_new_item(target, ITEM_FLASK_GLASS, 2, 1);
				sprintf(g_text_output_buf, txt[50], user->name);
			} else
				sprintf(g_text_output_buf, txt[89], user->name);
		} else
			sprintf(g_text_output_buf, txt[91], user->name);
		break;
	case 0x155:
		user->bd2 = 1;
		sprintf(g_text_output_buf, txt[112], user->name, grammar_pronoun(user->sex, 0, 0));
		break;
	case ITEM_ANTI_SICKNESS_ELIXIR:
		while ((r1 = hero_is_diseased(user)) != 0)
			hero_cure_disease(user);
		give_hero_new_item(target, ITEM_FLASK_GLASS, 2, 1);
		sprintf(g_text_output_buf, txt[50], user->name);
		break;
	case ITEM_GOLDLEIM:
		hero_get_poison(user, POISON_GOLDLEIM);
		dmg = random_interval(1, 0x14);
		give_hero_new_item(target, ITEM_FLASK_GLASS, 2, 1);
		sprintf(g_text_output_buf, txt[76], user->name, grammar_pronoun(user->sex, 0, 0));
		break;
	case ITEM_LOTUS_POISON:
		hero_get_poison(user, POISON_LOTUSGIFT);
#if FIX_0131
		/* Fix #131: leere Flasche zurueck wie bei den anderen Giften */
		dmg = random_interval(10, 0x28);
		give_hero_new_item(target, ITEM_FLASK_GLASS, 2, 1);
		sprintf(g_text_output_buf, txt[77], user->name);
#else
		dmg = random_interval(10, 0x28);
		sprintf(g_text_output_buf, txt[77], user->name);
#endif
		break;
	case ITEM_BANNSTAUB:
		if (user->is_mage() && user->ae > 0) {
			sub_ae_splash(user, random_interval(10, 0x14));
			sprintf(g_text_output_buf, txt[78], user->name, grammar_pronoun(user->sex, 1, 1));
		} else
			sprintf(g_text_output_buf, txt[79], user->name);
		break;
	case ITEM_SHURIN_POISON:
	case ITEM_ARAX_POISON:
	case ITEM_ANGST_POISON:
	case ITEM_SLEEP_POISON:
	case ITEM_KUKRIS:
	case ITEM_KROETEN_POISON:
		hero_get_poison(user, item_poison_type(id));
		give_hero_new_item(target, ITEM_FLASK_GLASS, 2, 1);
		sprintf(g_text_output_buf, txt[83], user->name);
		break;
	}
	old = dlg_set16(g_dialog, 5);
	if (verbose != 0)
		dlg_message(g_dialog, g_text_output_buf);
	dlg_set16(g_dialog, old);
	if (dmg != 0)
		sub_hero_le(user, dmg);
}
