/* OVL257 - Overlay O257 (Segment 0x2490 in SCHWEIF.EXE): Dialog-Objekt (ds:b86e, 0x53a Bytes; Gespraechsfuehrung in
 * O049, Szenen in O259). Felder wie OVL049 struct dlg, dazu +356 Menuezeilen (5), +36a Zeilenzahl, +370 Themen-
 * merker (100 Bytes), +3d4 "eben freigeschaltet" (100), +438 Themenliste (100 Worte), +500 Zaehlertabelle (10 x 3
 * Bytes {id, Stufe}), +51e Skript (O260), +522 Bmp, +52c Merker, +532/+536 Textbloecke.
 * Dlg()/~Dlg(): Themen 0..6/97 frei, Satztabelle Datei ds:35fe (0x1f2, R06) nach +352 laden / freigeben.
 * f_0164/f_020a: Abschluss-/Begruessungstext (+346/+34a oder Zufallstext O049_10EF 0x62/0x61; id 0xa3 -> fc = 0x62;
 *   f_020a nur ohne O259_0931). f_0281(v): Thema einsortieren (ab 7). f_031d: erledigte Themen (+370[t] == 1)
 *   entfernen. f_03a2: Skriptthemen 7..41 freischalten, Texte 0x61..0x63 aus den Skriptabschnitten nach +34a/+346/
 *   +34e. f_059e: Freischaltungen zuruecknehmen. f_05c8: Themen je Ort (ds:c4c5 +20, Faelle 1..19, Merker
 *   ds:256b..2578, 25a6..2746). f_0bb3/f_0c1a: Spielstand schreiben/lesen ("XDIA", 0x82, +370, +500; ds:c68d).
 * f_0c8b/f_0cc5/f_0d4e/f_0d89/f_0dd0: Zaehlertabelle lesen/setzen/verringern/nullen/aufraeumen (memmove).
 * f_0e49(k, v): Merker setzen (2 -> einsortieren), alter Wert. f_0e93: Merker = 1. f_0ea4(k): Thema freischalten,
 *   bei ds:b666 == 0x4f einsortieren und Texte nach Satz +352[k] (9 Bytes, SCOPY@) per O248_02E3, O049_0FEF.
 * f_102b: Liste neu aufbauen, Texte laden. f_110c: Abschied (Bmp 32 x 32 auf ds:c4fb, O190_037F). f_11db(n, ...):
 *   Menuezeilen. f_1261(n): Menue (ds:8bed/8bad/8bb5/8bc3, Notizfrage O190_01C6 -> f_164d), Rueckgabe Zeile + 1.
 * f_1538: Menue zeichnen (Fenster ds:3303 kurz 0xe1). f_164d: Notiz ins Tagebuch (sprintf/strcat, O103_04E1).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL257.CPP   -> 6102/6102 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   Ohne -Og 4203 Abweichungen; -Oi fuer memset/strcat (memmove bleibt Aufruf).
 * - Zusammengesetzt aus OVL257.HDR + gen/sw257.txt per gen/mk257.py; der switch in f_05c8 ist mit gen/g257.py aus
 *   Sprungtabelle und ASM erzeugt (Rumpfreihenfolge 1-4, 7, 5, 6, 8/9, 10 -> faellt in 11/12, 13..19, default).
 * - f_0dd0: "(10 - i - 1) * sizeof(struct t3)" (unsigned) behaelt mov ax,10 / sub / dec ax; mit "* 3" faltet BCC
 *   das -1 zu add dx,-3.
 * - Ketten: "f370[0] = .. = f370[6] = f370[0x61] = 1" (mov al,1, +3d1 zuerst), "f3d4[i] = f370[i] = 1",
 *   "f34a = f346 = f34e = 0" (xor eax,eax / mov dword), "g_8bed = g_8bb5 = 0".
 * - f_0c8b: Parameter und Zaehler mit register (CX/DX, keine Aufrufe); f_059e: Zaehler in AX.
 * - f_0ea4: j im Block hinter dem -Og-Zwischenwert this + k ([bp-2]), t/a/b im inneren Block.
 * - f_1261: Fensterwerte/col/r als Block-Locals hinter dem -Og-Zwischenwert (int)f36a ([bp-6]); "f36a <= sel".
 * - Member-far-Zeiger = 0: einfache Mitglieder (scene_buf/scene_txt) mov dword, Feldelemente f356[i] zwei Wort-Stores.
 * Originalfehler: f_031d setzt nach dem Verschieben "list[100] = 0" (+0x500) - hinter der Themenliste; das loescht
 *   die id des ersten Eintrags der Zaehlertabelle.
 * (Namensfassung mit schweif.h, class Conv/Script in s_class.h: 19.10.2026 GLEICH, LINKGLEICH) Methoden: f_0164 greet,
 *   f_020a bye, f_0281 insert_topic, f_031d remove_temp, f_03a2 script_topics, f_059e relock, f_05c8 place_topics,
 *   f_0bb3 save, f_0c1a load, f_0c8b get_counter, f_0cc5 set_counter, f_0d4e dec_counter, f_0d89 clr_counter,
 *   f_0dd0 compact_counters, f_0e49 set_topic, f_0e93 topic_on, f_0ea4 unlock_topic, f_102b build_list, f_110c
 *   refuse, f_11db set_menu, f_1261 menu, f_1538 draw_menu, f_164d note. Die Rohfassung aus tools/gen/o257
 *   (mk257.py: OVL257.HDR + sw257.txt) ist veraltet - massgeblich ist diese Datei.
 */
#include <io.h>
#include <mem.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:7784..778e, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

#define FILE_TOPIC_TXT	0x1f2		/* struct conv_topic_txt je Thema (g_file_3) */
#define TOPIC_BYE	0x61		/* Sonderthemen: Abschied, Begruessung, ohne Antwort */
#define TOPIC_GREET	0x62
#define TOPIC_NO_ANSWER	0x63
#define TOPIC_REFUSE	0x66		/* Zufallstext "Gespraech schon beendet" */
#define TOPIC_SCRIPT_FIRST	7	/* Skriptthemen 7..41 */
#define TOPIC_SCRIPT_END	0x2a
#define TOPIC_57	0x39		/* weiteres Thema mit eigener Textdatei */
#define CONV_ID_A3	0xa3		/* Gespraech mit Begruessung als Antwort (topic = TOPIC_GREET) */
#define PIC_CONV	0x4f		/* Vollbild des Gespraechs (g_pp20_index) */
#define CONV_SAVE_VERSION	0x82
#define MENU_X		0xe1		/* Themen-/Menuespalte */
#define COL_MENU	0xaa
#define COL_MENU_SEL	0xf9

Conv::Conv()
{
	register signed short i;

	topic_state[0] = topic_state[1] = topic_state[2] = topic_state[3] = topic_state[4] = topic_state[5] = topic_state[6] = topic_state[TOPIC_BYE] = 1;
	arc_open(g_file_3, FILE_TOPIC_TXT);
	topic_txt = (struct conv_topic_txt far *)mem_alloc(arc_size(g_file_3));
	arc_read_all(g_file_3, topic_txt);
	arc_close(g_file_3);
	for (i = 0; i < 5; i++)
		menu_lines[i] = 0;
	scene_buf = 0;
	scene_txt = 0;
}

Conv::~Conv()
{
	mem_free(topic_txt);
}

void Conv::greet()
{
	font_set_colors(g_font, 0xff, -1);
	font_set_y(g_font, 0x2e);
	if (greeting != 0) {
		if (id == CONV_ID_A3) {
			print(greeting, 0);
			topic = TOPIC_GREET;
		} else {
			print(greeting, 1);
		}
	} else {
		print(random_text(TOPIC_GREET, 0), 1);
	}
}

void Conv::bye()
{
	if (conv_bye_scene(this) == 0) {
		font_set_colors(g_font, 0xff, -1);
		if (farewell != 0)
			print(farewell, 1);
		else
			print(random_text(TOPIC_BYE, 0), 1);
	}
}

void Conv::insert_topic(signed short v)
{
	register signed short i;
	register signed short j;

	i = 7;
	if (refresh != 0)
		remove_temp();
	if (level >= script->tolerance)
		level--;
	while (list[i] < v)
		i++;
#if FIX_0174
	/* Fix #174: beim Einfuegen ebenso: nicht nach list[100] schreiben */
	for (j = 98; j >= i; j--)
		list[j + 1] = list[j];
#else
	for (j = 99; j >= i; j--)
		list[j + 1] = list[j];
#endif
	list[i] = v;
	refresh = 2;
}

void Conv::remove_temp()
{
	register signed short i;
	register signed short j;

	i = 7;
	do {
		while (list[i] != 0 && topic_state[list[i]] == 1)
			i++;
#if FIX_0174
		/* Fix #174: nur innerhalb der 100 Eintraege schieben (list[100] ist counters[0].id) */
		for (j = i; j < 99; j++)
			list[j] = list[j + 1];
		list[99] = 0;
#else
		for (j = i; j < 100; j++)
			list[j] = list[j + 1];
		list[100] = 0;
#endif
	} while (list[i] != 0 && i < 100);
}

void Conv::script_topics()
{
	struct script_topic far *s;
	struct script_ent far *e;
	register signed short i;

	farewell = greeting = no_answer = 0;
	for (i = TOPIC_SCRIPT_FIRST; i < TOPIC_SCRIPT_END; i++) {
		if (script->is_used(i) && !topic_state[i])
			unlocked[i] = topic_state[i] = 1;
	}
	if (script->is_used(TOPIC_BYE)) {
		s = &script->topics[TOPIC_BYE];
		e = *s->list;
		farewell = script->texts[e->answers[e->cur]];
	}
	if (script->is_used(TOPIC_GREET)) {
		s = &script->topics[TOPIC_GREET];
		e = *s->list;
		greeting = script->texts[e->answers[e->cur]];
	}
	if (script->is_used(TOPIC_NO_ANSWER)) {
		s = &script->topics[TOPIC_NO_ANSWER];
		e = *s->list;
		no_answer = script->texts[e->answers[e->cur]];
	}
}

void Conv::relock()
{
	register signed short i;

	for (i = TOPIC_SCRIPT_FIRST; i < TOPIC_SCRIPT_END; i++)
		if (unlocked[i] != 0)
			topic_state[i] = 0;
}

/* Themen je Ort (group.town 1..19): Orts-/Gebietsthemen 7..0x14 (TALKS_00..13) werden angeboten und als bekannt
 * gemerkt (g_topic_kvirasim .. g_topic_rorwhed); ausserhalb der Orte (default) kommen nur die Gebietsthemen 0x0c..0x14
 * zurueck. Dazu die Geschichtsthemen 0x2a.. nach ihren Merkern (g_topic_binge, g_topic_ingramosch ...). */
void Conv::place_topics()
{
	memset(&topic_state[7], 0, 14);
	memset(&topic_state[TOPIC_BINGE], 0, 30);
	switch (g_group->town) {
	case 1:
		topic_state[TOPIC_KVIRASIM] = topic_state[TOPIC_SALAMANDERSTEINE] = 1;
		g_topic_kvirasim = g_topic_salamandersteine = 1;
		break;
	case 2:
		topic_state[TOPIC_GASHOK] = topic_state[TOPIC_LOWANGEN] = topic_state[TOPIC_FINSTERKAMM] = topic_state[TOPIC_SALAMANDERSTEINE] = 1;
		g_topic_gashok = g_topic_lowangen = g_topic_finsterkamm = g_topic_salamandersteine = 1;
		if (g_sites_hint_done != 0 && !g_sites_vision_ok && !g_sites_vision_failed)
			topic_state[TOPIC_ASCHEHAUFEN] = 1;
		if (g_sites_explored != 0 && !g_topic_artherion)
			topic_state[TOPIC_MUEHLE] = 1;
		if (g_sites_night_met != 0 && !g_sites_challenges)
			topic_state[TOPIC_ZUGEZOGENE] = 1;
		if (g_sites_vision_ok != 0)
			topic_state[TOPIC_BANNSTRAHLER] = 1;
		else if (g_sites_vision_failed != 0)
			topic_state[TOPIC_VERMUMMTE] = 1;
		if (g_sites_shock_done != 0)
			topic_state[TOPIC_ARMBRUSTBOLZEN] = 1;
		if (g_sites_challenges != 0)
			topic_state[TOPIC_KUSLIK] = 1;
		if (g_topic_artherion != 0)
			topic_state[TOPIC_ARTHERION] = 1;
		break;
	case 3:
		topic_state[TOPIC_FINSTERKAMM] = topic_state[TOPIC_FINSTERKOPPEN] = 1;
		g_topic_finsterkoppen = g_topic_finsterkamm = 1;
		if (g_topic_binge != 0)
			topic_state[TOPIC_BINGE] = 1;
		break;
	case 4:
		topic_state[TOPIC_LOWANGEN] = topic_state[TOPIC_SVELLTTAL] = topic_state[TOPIC_SVELLTSUEMPFE] = topic_state[TOPIC_THASCH] = 1;
		g_topic_lowangen = g_topic_svellttal = g_topic_svelltsuempfe = g_topic_thasch = 1;
		if (g_topic_ansvell != 0)
			topic_state[TOPIC_ANSVELL] = 1;
		if (g_topic_siebenquell != 0)
			topic_state[TOPIC_SIEBENQUELL] = 1;
		if (g_topic_egelbronn != 0)
			topic_state[TOPIC_EGELBRONN] = 1;
		if (g_topic_gavron != 0)
			topic_state[TOPIC_GAVRON] = 1;
		if (g_topic_ingramosch != 0)
			topic_state[TOPIC_INGRAMOSCH] = 1;
		break;
	case 7:
		topic_state[TOPIC_ANSVELL] = topic_state[TOPIC_SVELLTTAL] = topic_state[TOPIC_SVELLTSUEMPFE] = 1;
		g_topic_ansvell = g_topic_svellttal = g_topic_svelltsuempfe = 1;
		break;
	case 5:
		topic_state[TOPIC_TIEFHUSEN] = topic_state[TOPIC_RORWHED] = topic_state[TOPIC_BLUTZINNEN] = topic_state[TOPIC_SVELLTTAL] = 1;
		g_topic_tiefhusen = g_topic_rorwhed = g_topic_blutzinnen = g_topic_svellttal = 1;
		if (g_port_can_haggle != 0)
			topic_state[TOPIC_HENSGER] = 1;
		break;
	case 6:
		topic_state[TOPIC_TJOLMAR] = topic_state[TOPIC_SVELLTTAL] = 1;
		g_topic_tjolmar = g_topic_svellttal = 1;
		if (g_topic_ingramosch != 0)
			topic_state[TOPIC_INGRAMOSCH] = 1;
		break;
	case 8:
	case 9:
		topic_state[TOPIC_SALAMANDERSTEINE] = 1;
		g_topic_salamandersteine = 1;
		if (g_topic_binge != 0)
			topic_state[TOPIC_BINGE] = 1;
		if (g_topic_ingramosch != 0)
			topic_state[TOPIC_INGRAMOSCH] = 1;
		break;
	case 10:
		topic_state[TOPIC_LOWANGEN] = 1;
		g_topic_lowangen = 1;
		if (g_topic_svellttal != 0)
			topic_state[TOPIC_SVELLTTAL] = 1;
		if (g_topic_svelltsuempfe != 0)
			topic_state[TOPIC_SVELLTSUEMPFE] = 1;
	case 11:
	case 12:
		topic_state[TOPIC_FINSTERKAMM] = 1;
		topic_state[TOPIC_FINSTERKOPPEN] = 1;
		g_topic_finsterkoppen = g_topic_finsterkamm = 1;
		if (g_topic_binge != 0)
			topic_state[TOPIC_BINGE] = 1;
		if (g_topic_ingramosch != 0)
			topic_state[TOPIC_INGRAMOSCH] = 1;
		break;
	case 13:
		topic_state[TOPIC_LOWANGEN] = topic_state[TOPIC_SVELLTTAL] = topic_state[TOPIC_THASCH] = 1;
		g_topic_lowangen = g_topic_svellttal = g_topic_thasch = 1;
		if (g_topic_binge != 0)
			topic_state[TOPIC_BINGE] = 1;
		if (g_topic_ingramosch != 0)
			topic_state[TOPIC_INGRAMOSCH] = 1;
		if (g_blackmage_done != 0)
			topic_state[TOPIC_REISENDE] = 1;
		break;
	case 14:
		topic_state[TOPIC_SVELLTTAL] = topic_state[TOPIC_SVELLTSUEMPFE] = topic_state[TOPIC_BLUTZINNEN] = topic_state[TOPIC_THASCH] = 1;
		g_topic_svellttal = g_topic_svelltsuempfe = g_topic_blutzinnen = g_topic_thasch = 1;
		if (g_topic_ansvell != 0)
			topic_state[TOPIC_ANSVELL] = 1;
		if (g_topic_binge != 0)
			topic_state[TOPIC_BINGE] = 1;
		if (g_topic_ingramosch != 0)
			topic_state[TOPIC_INGRAMOSCH] = 1;
		if (g_blackmage_done != 0)
			topic_state[TOPIC_REISENDE] = 1;
		break;
	case 15:
		topic_state[TOPIC_SVELLTTAL] = topic_state[TOPIC_SVELLTSUEMPFE] = topic_state[TOPIC_THASCH] = 1;
		g_topic_svellttal = g_topic_svelltsuempfe = g_topic_thasch = 1;
		if (g_topic_ansvell != 0)
			topic_state[TOPIC_ANSVELL] = 1;
		if (g_topic_ingramosch != 0)
			topic_state[TOPIC_INGRAMOSCH] = 1;
		if (g_blackmage_done != 0)
			topic_state[TOPIC_REISENDE] = 1;
		break;
	case 16:
		topic_state[TOPIC_BLUTZINNEN] = 1;
		g_topic_blutzinnen = 1;
		if (g_topic_ingramosch != 0)
			topic_state[TOPIC_INGRAMOSCH] = 1;
		break;
	case 17:
		topic_state[TOPIC_SVELLTTAL] = topic_state[TOPIC_RORWHED] = 1;
		g_topic_svellttal = g_topic_rorwhed = 1;
		if (g_topic_ingramosch != 0)
			topic_state[TOPIC_INGRAMOSCH] = 1;
		break;
	case 18:
		topic_state[TOPIC_SVELLTTAL] = 1;
		g_topic_svellttal = 1;
		if (g_topic_ingramosch != 0)
			topic_state[TOPIC_INGRAMOSCH] = 1;
		break;
	case 19:
		topic_state[TOPIC_LOWANGEN] = 1;
		topic_state[TOPIC_FINSTERKAMM] = 1;
		topic_state[TOPIC_FINSTERKOPPEN] = 1;
		g_topic_lowangen = g_topic_finsterkoppen = g_topic_finsterkamm = 1;
		if (g_topic_binge != 0)
			topic_state[TOPIC_BINGE] = 1;
		if (g_topic_ingramosch != 0)
			topic_state[TOPIC_INGRAMOSCH] = 1;
		break;
	default:
		if (g_topic_binge != 0)
			topic_state[TOPIC_BINGE] = 1;
		if (g_topic_finsterkoppen != 0)
			topic_state[TOPIC_FINSTERKOPPEN] = 1;
		if (g_topic_ansvell != 0)
			topic_state[TOPIC_ANSVELL] = 1;
		if (g_topic_salamandersteine != 0)
			topic_state[TOPIC_SALAMANDERSTEINE] = 1;
		if (g_topic_finsterkamm != 0)
			topic_state[TOPIC_FINSTERKAMM] = 1;
		if (g_topic_svellttal != 0)
			topic_state[TOPIC_SVELLTTAL] = 1;
		if (g_topic_svelltsuempfe != 0)
			topic_state[TOPIC_SVELLTSUEMPFE] = 1;
		if (g_topic_thasch != 0)
			topic_state[TOPIC_THASCH] = 1;
		if (g_topic_blutzinnen != 0)
			topic_state[TOPIC_BLUTZINNEN] = 1;
		if (g_topic_rorwhed != 0)
			topic_state[TOPIC_RORWHED] = 1;
		if (g_blackmage_done != 0)
			topic_state[TOPIC_REISENDE] = 1;
		break;
	}
	if (g_topic_salamander_stone != 0)
		topic_state[TOPIC_SALAMANDERSTEIN] = 1;
	if (g_topic_sternenschweif != 0)
		topic_state[TOPIC_STERNENSCHWEIF] = 1;
}

signed short Conv::save()
{
	signed short v = CONV_SAVE_VERSION;

	_write(g_save_fd, "XDIA", 4);
	_write(g_save_fd, &v, 2);
	_write(g_save_fd, topic_state, 100);
	_write(g_save_fd, counters, 30);
	return 1;
}

signed short Conv::load()
{
	signed long magic;
	signed short v;

	_read(g_save_fd, &magic, 4);
	if (magic == 0x41494458L /* "XDIA" */) {
		_read(g_save_fd, &v, 2);
		_read(g_save_fd, topic_state, 100);
		_read(g_save_fd, counters, 30);
		return 1;
	}
	return 0;
}

signed char Conv::get_counter(register signed short id)
{
	register signed short i;

	for (i = 0; i < 10; i++)
		if (counters[i].id == id)
			return counters[i].level;
	return 0;
}

void Conv::set_counter(register signed short id)
{
	signed short found;
	register signed short i;

	found = 0;
	for (i = 0; i < 10; i++) {
		if (counters[i].id == id) {
			counters[i].level = level;
			found = 1;
		}
	}
	if (found == 0) {
		memmove(&counters[1], &counters[0], 27);
		counters[0].id = id;
		counters[0].level = level;
	}
}

void Conv::dec_counter(register signed short k)
{
	if (counters[k].level > 0)
		counters[k].level--;
	compact_counters();
}

void Conv::clr_counter(register signed short id)
{
	register signed short i;

	for (i = 0; i < 10; i++)
		if (counters[i].id == id)
			counters[i].level = 0;
	compact_counters();
}

void Conv::compact_counters()
{
	register signed short i;

	for (i = 0; i < 10; i++) {
		if (!counters[i].level) {
			if (i < 9)
				memmove(&counters[i], &counters[i + 1], (10 - i - 1) * sizeof(struct conv_counter));
			counters[9].id = 0;
			counters[9].level = 0;
		}
	}
}

signed char Conv::set_topic(register signed short k, signed short v)
{
	signed char old;

	old = topic_state[k];
	if (v != -1) {
		topic_state[k] = v;
		if (v == 2)
			insert_topic(k);
	}
	return old;
}

void Conv::topic_on(signed short k)
{
	topic_state[k] = 1;
}

void Conv::unlock_topic(register signed short k)
{
	register signed short i;

	if (script->is_used(k)) {
		if (!topic_state[k]) {
			unlocked[k] = topic_state[k] = 1;
			if (g_pp20_index == PIC_CONV) {
				signed short j;

				for (j = 0; list[j] < k; j++)
					;
				for (i = 99; i >= j; i--)
					list[i + 1] = list[i];
				list[j] = k;
				if (((k >= TOPIC_SCRIPT_FIRST && k < TOPIC_SCRIPT_END) || k == TOPIC_57) && topic_buf[k] == 0 && topic_txts[k] == 0) {
					struct conv_topic_txt t;
					signed short a;
					signed short b;

					t = topic_txt[k];
					a = g_conv_topic_tab[k - TOPIC_SCRIPT_FIRST];
					b = t.q_n + t.a_n + t.alt_n;
					txt_load_list(g_font, a, b, &topic_buf[k], &topic_txts[k]);
				}
				draw_topics(1);
			}
		}
	}
}

void Conv::build_list()
{
	register signed short i;
	register signed short n;
	signed short a;
	signed short b;

	for (i = n = 0; i < 100; i++) {
		list[i] = 0;
		if (topic_state[i] != 0) {
			list[n++] = i;
			if ((i >= TOPIC_SCRIPT_FIRST && i < TOPIC_SCRIPT_END) || i == TOPIC_57) {
				a = g_conv_topic_tab[i - TOPIC_SCRIPT_FIRST];
				b = topic_txt[i].q_n + topic_txt[i].a_n + topic_txt[i].alt_n;
				txt_load_list(g_font, a, b, &topic_buf[i], &topic_txts[i]);
			}
		}
	}
}

void Conv::refuse()
{
	char far *s;
	Bmp b(0x20, 0x20, g_portrait_buf);

	load_portrait_buf(script->pic - 1);
	if (script->tolerance == 0) {
		if (farewell != 0)
			s = farewell;
		else
			s = random_text(TOPIC_BYE, 0);
	} else {
		s = random_text(TOPIC_REFUSE, 0);
	}
	dlg_box_bmp(g_dialog, &b, script->name, s, 0);
}

void Conv::set_menu(signed short n, ...)
{
	va_list ap;
	register signed short i;

	for (i = 0; i < 5; i++)
		menu_lines[i] = 0;
	if (n > 5)
		n_menu = 5;
	else
		n_menu = n;
	va_start(ap, n);
	for (i = 0; i < n_menu; i++)
		menu_lines[i] = va_arg(ap, char far *);
}

signed short Conv::menu(signed short n)
{
	signed short done = 0;
	signed short last;
	register signed short old = -1;
	register signed short sel = 0;

	last = g_mouse_posy;
	draw_menu();
	mouse_on();
	g_menu_line = 0;
	mouse_clear_left();
	mouse_clear_right();
	do {
		poll_input();
		if (g_mouse_posy != last) {
			last = g_mouse_posy;
			sel = g_menu_line - 1;
		}
		if (g_action == KEY_ESC) {
			done = 1;
			sel = n - 1;
		} else if (g_action == KEY_UP) {
			sel--;
		} else if (g_action == KEY_DOWN) {
			sel++;
		} else if (g_mouse_rclick != 0 || g_action == KEY_PGUP) {
			signed short x, y, w, h;
			signed short col;

			font_get_box(g_font, &x, &y, &w, &h);
			col = font_get_y(g_font);
			if (topic != -1 && *answer != 0) {
				signed short r;

				r = dlg_choose(g_dialog, menu_txt[0], 2, menu_txt[1], menu_txt[3]);
				if (r == 1)
					note();
			}
			font_set_box(g_font, x, y, w, h);
			font_set_y(g_font, col);
			g_action = g_mouse_rclick = 0;
			mouse_on();
		} else if ((g_mouse_lclick != 0 && g_action == 0x81) || g_action == KEY_INS || g_action == KEY_F5 || g_action == KEY_CTRL_J) {
			note();
			g_action = g_mouse_lclick = 0;
		} else if ((g_mouse_lclick != 0 && g_action != 0) || g_action == KEY_RETURN) {
			done = 1;
			g_action = g_mouse_lclick = 0;
		}
		if (sel < 0)
			sel = 0;
		if (n_menu <= sel)
			sel = n_menu - 1;
		if (n_menu > 1 && sel != old) {
			if (old != -1) {
				font_set_colors(g_font, COL_MENU, -1);
				font_print(g_font, menu_lines[old], MENU_X, old * 8 + 3);
				font_restore_colors(g_font);
			}
			old = sel;
			font_set_colors(g_font, COL_MENU_SEL, -1);
			font_print(g_font, menu_lines[sel], MENU_X, sel * 8 + 3);
			font_restore_colors(g_font);
		}
	} while (done == 0);
	g_mouse_handler_on = 0;
	mouse_off();
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	return sel + 1;
}

void Conv::draw_menu()
{
	struct rect save;
	register signed short i;

	mouse_hide(0);
	save = g_clip_rect;
	g_clip_rect.x0 = MENU_X;
	bmp.put(g_vga_buf, 0, 0);
	if (n_menu > 1) {
		font_set_colors(g_font, COL_MENU, -1);
		for (i = 0; i < n_menu; i++)
			font_print(g_font, menu_lines[i], MENU_X, i * 8 + 3);
		font_restore_colors(g_font);
	} else {
		font_set_colors(g_font, COL_MENU_SEL, -1);
		font_print(g_font, menu_lines[0], MENU_X, 3);
		font_restore_colors(g_font);
	}
	g_clip_rect = save;
	mouse_show(0);
}

void Conv::note()
{
	signed short col;

	if (topic != -1 && *answer != 0) {
		col = font_get_y(g_font);
		mouse_hide(0);
		do_border(g_vga_buf, 0xab, 7, 0x27, 0x1a, 0xfd);
		sprintf(g_text_buf2, menu_txt[5], script->name);
		sprintf(g_text_output_buf, menu_txt[6], script->titles[topic]);
		strcat(g_text_buf2, g_text_output_buf);
		sprintf(g_text_output_buf, "%s%s", g_text_buf2, answer);
		diary_add(g_diary, g_text_output_buf, 0);
		memset(answer, 0, 1000);
		do_border(g_vga_buf, 0xab, 7, 0x27, 0x1a, 0x0c);
		font_set_y(g_font, col);
		mouse_show(0);
	}
}
