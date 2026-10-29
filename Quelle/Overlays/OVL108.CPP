/* OVL108 - Overlay O108 (Segment 0x216d in SCHWEIF.EXE): Schloss, Klasse Lock (0x29 Bytes, s_class.h; +0..7 =
 * Datensatz nr der Datei 0x2c8: type, key, special, fail_special, state LOCK_*, tries). Konstruktor(nr): Satz lesen
 * (ofile_*), Texte txt_load_list(g_font, 0x2c9, 0x1e), lock_init, bei LOCK_CLOSED/LOCK_SEALED: ab 10 Versuchen
 * Hinweis, lock_try_open; bei Erfolg g_lock_cell_bits = 0x2000 (zugezaubert: Werte halbieren), sonst Falle
 * (Jaehzorn-Probe test_attrib(h, ATTRIB_JZ, -tries) > 0 -> Text nach W100, Schaden W9/W6/W3, hoechstens LE - 1).
 * Destruktor schreibt den Satz zurueck. lock_init: Held get_first_active_hero, Methodenzeiger special_func/fail_func
 * aus g_lock_special_default/g_lock_fail_default bzw. g_lock_special[special]/g_lock_fail[fail_special], Werte aus
 * g_lock_types[type] (force halbiert). lock_try_open: Schluessel (lock_use_key/group_find_key), Sonderbehandlung oder
 * Auswahl dlg_choose (4 Texte): lock_force_open (KK cur + mod gegen force), lock_pick (ITEM_PICKLOCKS,
 * test_skill(h, TA_SCHLOESSER, pick_mod)), lock_foramen (SP_FORAMEN, AE >= 6), lock_claudibus (SP_CLAUDIBUS:
 * LOCK_SEALED, Werte verdoppelt); Misserfolg ruft fail_func oder zeigt Text. lock_trap: Falle (g_lock_fail[0]):
 * ohne g_lock_trap_off Schaden 10..20 fuer die Gruppe.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL108.CPP   -> 2812/2812 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; g_lock_special trifft _g_6232+0x2e)
 * - special_func/fail_func sind 8-Byte-Methodenzeiger: "class Lock;" VOR dem typedef LOCK_PMF (unvollstaendige
 *   Klasse -> allgemeinste Darstellung, 8 Bytes). Test "if (special_func)" -> or ueber 4 Worte; Aufruf
 *   (this->*special_func)() -> call dword ptr es:[bx+0dh]; Zuweisung aus festem Element -> 4 Wortkopien, aus
 *   g_lock_special[i] -> SCOPY@ mit cx = 8.
 * - -Og noetig (Epilogkopien in try_open/pick/..: "if .. return x; else if .. return y; else {..}").
 * - Konstruktor: "if ((r = try_open()) == 1)" eingebettet (cmp ax), "if (dmg >= hero->le)" mit register dmg.
 * - "force >>= 1" bei signed char -> sar byte; "<<= 1" -> shl byte; tries als unsigned char (jna, neg nach mov ah,0).
 * - Satzposition pos = (unsigned short)(n << 3) -> shl / movzx eax.
 * - Phase B (19.10.2026): eigene Daten im Quelltext; g_lock_fail[0] = &Lock::trap als echter Methodenzeiger
 *   (segmentrelativer Fernzeiger auf OVL108_TEXT, TLINK setzt den Stub-Einsprung ein - wie O078).
 */
#include <stdio.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6230..6288, dsana) - initialisierte Daten in Adressreihenfolge */
struct lock_type g_lock_types[16] = {	/* je Schlossart: Aufbrechen (KK x 2), Zuschlag Schloesser, Zuschlag Foramen */
	{ 22, 0, 3 },	/* 0 */
	{ 24, 3, 5 },	/* 1 */
	{ 26, 6, 6 },	/* 2 */
	{ 28, 4, 8 },	/* 3 */
	{ 30, 8, 10 },	/* 4 */
	{ 32, 12, 12 },	/* 5 */
	{ 34, 5, 10 },	/* 6 */
	{ 36, 10, 15 },	/* 7 */
	{ 38, 15, 20 },	/* 8 */
	{ 40, 6, 12 },	/* 9 */
	{ 40, 10, 18 },	/* 10 */
	{ 45, 18, 27 },	/* 11 */
	{ 45, 30, 49 },	/* 12 */
	{ 99, 99, 99 },	/* 13 */
	{ 99, 99, 99 },	/* 14 */
	{ 99, 99, 99 } 	/* 15 */
};
LOCK_PMF g_lock_special[1] = { 0 };	/* Sonderbehandlung je Lock.special (keine belegt) */
LOCK_PMF g_lock_fail[2] = { &Lock::trap, 0 };	/* Fehlschlag-Folge je Lock.fail_special */
LOCK_PMF g_lock_special_default = 0;	/* keine Sonderbehandlung */
LOCK_PMF g_lock_fail_default = 0;	/* keine Falle */

Lock::Lock(signed short n)
{
	signed short old_state;
	unsigned long pos;
	char far *s;
	register signed short r;
	register signed short dmg;

	g_lock_cell_bits = -1;
	nr = n;
	pos = (unsigned short)(n << 3);
	ofile_open(g_ofile, 0x2c8);
	ofile_seek(g_ofile, pos, 0);
	ofile_read(g_ofile, this, 8);
	ofile_close(g_ofile);
	txt_load_list(g_font, 0x2c9, 0x1e, &txt_buf, &txt);
	init();
	if (state == LOCK_CLOSED || state == LOCK_SEALED) {
		if (tries > 10) {
			sprintf(g_text_output_buf, state == LOCK_CLOSED ? txt[0] : txt[2], hero->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
		old_state = state;
		if ((r = try_open()) == 1) {
			if (old_state == LOCK_SEALED) {
				force >>= 1;
				pick_mod >>= 1;
				spell_mod >>= 1;
			}
			g_lock_cell_bits = 0x2000;
		} else if (r != -1) {
			/* Falle: Jaehzorn-Probe, je mehr Versuche desto leichter */
			if (test_attrib(hero, ATTRIB_JZ, -tries) > 0) {
				r = random_schweif(100);
				s = r > 80 ? txt[0x10] : (r > 60 ? txt[0x0f] : txt[0x0e]);
				sprintf(g_text_output_buf, s, hero->name);
				dlg_message(g_dialog, g_text_output_buf);
				dmg = random_schweif(r > 80 ? 9 : (r > 60 ? 6 : 3));
				if (dmg >= hero->le)
					dmg = hero->le - 1;
				sub_hero_le(hero, dmg);
			}
		}
	} else if (state == LOCK_OPENED)
		g_lock_cell_bits = 0x2000;
	else if (state == LOCK_BROKEN)
		dlg_message(g_dialog, txt[1]);
}

Lock::~Lock()
{
	unsigned long pos;

	pos = (unsigned short)(nr << 3);
	ofile_create(g_ofile, 0x2c8, 0x8004);
	ofile_seek(g_ofile, pos, 0);
	ofile_write(g_ofile, this, 8);
	ofile_close(g_ofile);
	mem_free(txt_buf);
	mem_free(txt);
}

void Lock::init()
{
	hero = get_first_active_hero(g_group);
	special_func = g_lock_special_default;
	if (special != -1)
		special_func = g_lock_special[special];
	fail_func = g_lock_fail_default;
	if (fail_special != -1)
		fail_func = g_lock_fail[fail_special];
	force = g_lock_types[type].force / 2;
	pick_mod = g_lock_types[type].pick_mod;
	spell_mod = g_lock_types[type].spell_mod;
}

signed short Lock::try_open()
{
	register signed short r;

	if (state == LOCK_CLOSED || state == LOCK_SEALED) {
		if (key != -1 && use_key() == 1)
			return 1;
		if (special_func) {
			tries++;
			return (this->*special_func)();
		} else {
			if ((r = dlg_choose(g_dialog, txt[3], 3, txt[4], txt[5], txt[6], txt[7])) != -1) {
				if (r == 1)
					return force_open();
				else if (r == 2)
					return pick();
				else if (r == 3)
					return foramen();
				else
					return claudibus();
			} else
				return -1;
		}
	} else
		return 1;
}

signed short Lock::pick()
{
	if ((hero = get_hero_with_item(g_group, ITEM_PICKLOCKS)) != 0) {
		tries++;
		if (test_skill(hero, TA_SCHLOESSER, pick_mod) >= 0) {
			state = LOCK_OPEN;
			return 1;
		} else if (fail_func)
			return (this->*fail_func)();
		else {
			sprintf(g_text_output_buf, txt[8], hero->name);
			dlg_message(g_dialog, g_text_output_buf);
			return 0;
		}
	} else {
		dlg_message(g_dialog, txt[9]);
		return -1;
	}
}

signed short Lock::claudibus()
{
	if ((hero = get_active_mage(g_group, txt[0x0a])) != 0) {
		if (hero->ae >= 6 && test_spell(hero, SP_CLAUDIBUS, spell_mod) > 0) {
			sub_ae_splash(hero, 6);
			state = LOCK_SEALED;
			force <<= 1;
			spell_mod <<= 1;
			pick_mod <<= 1;
			sprintf(g_text_output_buf, txt[0x14], hero->name);
			dlg_message(g_dialog, g_text_output_buf);
			return -1;
		} else {
			sub_ae_splash(hero, 3);
			if (fail_func)
				return (this->*fail_func)();
			else {
				sprintf(g_text_output_buf, txt[0x12], hero->name);
				dlg_message(g_dialog, g_text_output_buf);
				return 0;
			}
		}
	} else {
		dlg_message(g_dialog, txt[0x0c]);
		return -1;
	}
}

signed short Lock::foramen()
{
	if ((hero = get_active_mage(g_group, txt[0x0a])) != 0) {
		tries++;
		if (hero->ae >= 6 && test_spell(hero, SP_FORAMEN, spell_mod) > 0) {
			sub_ae_splash(hero, 6);
			state = LOCK_OPEN;
			return 1;
		} else {
			sub_ae_splash(hero, 3);
			if (fail_func)
				return (this->*fail_func)();
			else {
				sprintf(g_text_output_buf, txt[0x0b], hero->name);
				dlg_message(g_dialog, g_text_output_buf);
				return 0;
			}
		}
	} else {
		dlg_message(g_dialog, txt[0x0c]);
		return -1;
	}
}

signed short Lock::force_open()
{
	tries++;
	snd_play(g_snd_fx, 0xa2);
	if ((signed char)(hero->attrib[ATTRIB_KK].cur + hero->attrib[ATTRIB_KK].mod) >= force) {
		state = LOCK_BROKEN;
		return 1;
	} else if (fail_func)
		return (this->*fail_func)();
	else {
		sprintf(g_text_output_buf, txt[0x0d], hero->name);
		dlg_message(g_dialog, g_text_output_buf);
		return 0;
	}
}

signed short Lock::use_key()
{
	if (group_find_key(g_group, key) != -1) {
		state = LOCK_OPEN;
		return 1;
	} else
		return 0;
}

signed short Lock::trap()
{
	if (!g_lock_trap_off) {
		dlg_message(g_dialog, txt[0x13]);
		group_sub_le_range(g_group, 10, 20);
	} else
		dlg_message(g_dialog, txt[0x15]);
	return 0;
}
