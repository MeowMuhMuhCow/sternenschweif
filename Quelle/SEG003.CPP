/* SEG003 - Handbuchabfrage (Kopierschutz) als Klasse CopyProt, Objekt g_copyprot (ds:c6ca); dazu das
 * Aufraeumen beim Spielende (cleanup_game, BE: cleanup_game) und der V86-Test cpu_v86_active.
 * (seg003 in SCHWEIF.EXE, Offsets R03_0006..R03_059B)
 *
 * Ablauf der Handbuchabfrage CopyProt::query(chance) (mit Wahrscheinlichkeit 1/chance):
 *   init (Merker vorbelegen, PROTECT.LTX laden), PASSWORD.DAT oeffnen (Wort: Satzzahl, dann Saetze zu 25 Bytes
 *   struct pw_rec), bis zu drei Versuche: zufaelliger Satz, Frage txt[0] =
 *     "Bitte schlagen Sie in Ihrem <Sternenschweif> Handbuch auf@@Seite %d@Zeile %d@Wort %d@@nach."
 *   (Seite/Zeile/Wort aus dem Satz), Eingabe (20 Zeichen, erstes gross; dlg_message_flag), verschluesseln
 *   (encrypt) und mit dem gespeicherten Code vergleichen (compare); richtig -> g_copyprot_passed = 1.
 *   Verschluesselung je Zeichen c (20 Zeichen, Rest 0): k1 ^= c; sum += k1; code = sum (Startwerte aus dem Satz).
 *   PROTECT.LTX enthaelt ausserdem Kartenfragen ("Wie heisst die Stadt %s von %s?", Himmelsrichtungen), die der
 *   Code nicht benutzt.
 *
 * In der CD-Fassung wird query NIRGENDS aufgerufen (kein Verweis im ganzen Programm). Wirksam bleibt nur
 * copyprot_check (aus game_idle, R01): beendet das Spiel (fail), wenn weder ok2 noch ok gesetzt ist. Der
 * Konstruktor setzt ok = Zufall 3..21, die Pruefung besteht also immer (CD-Fassung: Schutz ueber die CD-Abfrage
 * R05 cd_check). Die Aufrufstelle in game_idle und der Code zwischen copyprot_code_begin und copyprot_code_end
 * werden vom Kampf-Zeitgeber (R23 fight_timer_cb) per Pruefsumme bzw. Muster ueberwacht (g_tamper_count).
 * Die vier leeren Funktionen sind Adressmarken (Grenzen der Pruefsumme) bzw. Platzhalter.
 *
 * C++! BCC -mlarge -3 -Ot -Og -c -Y SEG003.CPP   (-Og noetig: compare haelt this + i in SI)
 *
 * Funktionen (Offset Name):
 *   0006 copyprot_code_begin  Marke: Beginn der Pruefsumme (O011 CHECKSUM, R23 fight_timer_cb)
 *   000B copyprot_stub        leer; wird in savegame_op (R01) markiert aufgerufen (Musterpruefung R23)
 *   0010 copyprot_check       Pruefung aus game_idle: !ok2 && !ok -> fail
 *   0041 CopyProt::query      Abfrage (unbenutzt)
 *   0128 CopyProt::encrypt    Eingabe verschluesseln, g_copyprot_passed = 1
 *   018C CopyProt::compare    Vergleich mit rec.code; Abweichung -> g_copyprot_passed = 0, ok = 0
 *   01F5 CopyProt::get_ok, 0203 CopyProt::init, 0256 CopyProt::get_ok2, 0264 CopyProt::fail
 *   0278 copyprot_code_end    Marke: Ende der Pruefsumme (exklusiv)
 *   027D exit_game            exit(0)
 *   028C cleanup_game         Aufraeumen vor dem Programmende (Maus, Kampf-Zeitgeber, Klang, CD, Video)
 *   0306 CopyProt::CopyProt   Konstruktor (CopyProt_ctor; O032 globals_ctor)
 *   0347 load_texts, 036D finish, 03D0 rnd, 03E2 rnd_interval
 *   03F7 copyprot_empty       leer
 *   03FC CopyProt::~CopyProt  Destruktor (CopyProt_dtor; O032 globals_dtor)
 *   0421 seek_record, 0465 clear_answer, 0480 ask, 04D4 take_answer, 04EF open_file, 051C read_record,
 *   0558 set_f8 (Fuellwerte ohne Wirkung)
 *   0591 cpu_v86_active       smsw: PE-Bit (CPU im V86-Modus, also EMM386 o. ae. geladen; R05: g_ems_avail)
 *
 * Auffaelligkeiten des Originals (bewusst nachgebaut):
 *   - Verschleierte Konstanten: init setzt i = rnd(1) - 1 (= 0) und leitet daraus ok2 = 16, ok = 0 ab; encrypt
 *     rechnet ok = k1 + three - 2, compare g_copyprot_passed = six - 6 und ok = rnd(1) - 1.
 *   - set_f8 schreibt zweimal dieselben drei Bytes (Schleife bis 2) in Felder, die nie gelesen werden.
 *   - cleanup_game: leere Abfrage "if (mode == 0) ;".
 */
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include <stdlib.h>
#include <conio.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:3e6e..3e70, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_copyprot_passed = 0;

#define G_COPYPROT	((CopyProt far *)g_copyprot)

/* R03_0006: Adressmarke, Beginn des pruefsummengeschuetzten Bereichs */
void copyprot_code_begin(void)
{
}

/* R03_000B: leerer Platzhalter (markierter Aufruf in savegame_op) */
void copyprot_stub(void)
{
}

/* R03_0010: Kopierschutzpruefung, aus game_idle markiert aufgerufen */
void copyprot_check(void)
{
	if (!G_COPYPROT->get_ok2() && !G_COPYPROT->get_ok())
		G_COPYPROT->fail();
}

/* R03_0041: Handbuchabfrage mit Wahrscheinlichkeit 1/chance (in der CD-Fassung unbenutzt) */
void CopyProt::query(signed short chance)
{
	if (rnd(chance) == 1) {
		init();
		set_f8((signed short)txt[0]);
		open_file();
		for (tries = 0; tries < 3; tries++) {
			seek_record();
			read_record();
			ask();
			clear_answer();
			take_answer();
			encrypt();
			compare();
			if (g_copyprot_passed)
				break;
		}
		finish();
	}
}

/* R03_0128: Eingabe verschluesseln (wie PASSWORD.DAT), Antwort vorlaeufig "richtig" */
void CopyProt::encrypt()
{
	signed short i;
	signed short three = 3;
	unsigned char far *p = answer;
	register unsigned char k1 = key1;
	unsigned char sum = key2;

	for (i = 0; i < 20; i++) {
		k1 ^= *p;
		ok = k1 + three - 2;
		sum += k1;
		*p = sum;
		p++;
	}
	g_copyprot_passed = 1;
}

/* R03_018C: verschluesselte Eingabe mit dem Code des Satzes vergleichen */
void CopyProt::compare()
{
	signed short six = 6;

	for (i = 0; i < 20; i++) {
		if (answer[i] != rec.code[i]) {
			g_copyprot_passed = six - 6;
			ok = rnd(1) - 1;
		}
	}
}

/* R03_01F5 */
signed short CopyProt::get_ok()
{
	return ok;
}

/* R03_0203: Merker vorbelegen (i = 0, ok2 = 16, ok = 0), Texte laden */
void CopyProt::init()
{
	i = rnd(1) - 1;
	ok2 = (i + 2) << 3;
	ok = i * 3;
	load_texts();
}

/* R03_0256 */
signed short CopyProt::get_ok2()
{
	return ok2;
}

/* R03_0264: Abfrage nicht bestanden -> Programmende */
void CopyProt::fail()
{
	cleanup_game(1);
	exit_game();
}

/* R03_0278: Adressmarke, Ende des pruefsummengeschuetzten Bereichs */
void copyprot_code_end(void)
{
}

/* R03_027D */
void exit_game(void)
{
	exit(0);
}

/* R03_028C: Aufraeumen vor dem Programmende (BE: cleanup_game); mode wird nicht ausgewertet */
void cleanup_game(signed short mode)
{
	mouse_hide(0);
	fight_timer_stop();
	snd_obj_stop(g_snd_music);
	Speech_stop(g_snd_speech);
	ani_halt(g_screen_pic);
	if (g_cd_music == 1) {
		g_cd_music = 0;
		delete (CD far *)g_cd;
	}
	if (mode == 0)
		;
	Date_free(g_date);
	restore_video_mode(&g_video_state);
	clrscr();
}

/* R03_0306 */
CopyProt::CopyProt()
{
	ok = rnd_interval(3, 21);
}

/* R03_0347: PROTECT.LTX (10 Texte) laden */
void CopyProt::load_texts()
{
	txt_load_list(g_font, FILE_PROTECT_LTX, 10, &txt_buf, &txt);
}

/* R03_036D: Texte freigeben, PASSWORD.DAT schliessen */
void CopyProt::finish()
{
	txt_buf = mem_free(txt_buf);
	txt = (char far * far *)mem_free(txt);
	ok2 = rnd(1) - 1;
	file_close(g_file_main);
}

/* R03_03D0 */
signed short CopyProt::rnd(signed short n)
{
	return random_schweif(n);
}

/* R03_03E2 */
signed short CopyProt::rnd_interval(signed short lo, signed short hi)
{
	return random_interval(lo, hi);
}

/* R03_03F7: leer */
void copyprot_empty(void)
{
}

/* R03_03FC */
CopyProt::~CopyProt()
{
}

/* R03_0421: zufaelligen Satz waehlen und anfahren */
void CopyProt::seek_record()
{
	rec_nr = rnd(nr_recs) - 1;
	file_seek(g_file_main, (unsigned short)(rec_nr * 25 + 2), 0);
}

/* R03_0465 */
void CopyProt::clear_answer()
{
	memset(answer, 0, 20);
}

/* R03_0480: Frage stellen, Antwort (hoechstens 20 Zeichen) landet in g_input_buf */
void CopyProt::ask()
{
	sprintf(g_text_output_buf, txt[0], rec.page, rec.line, rec.word);
	dlg_message_flag(g_dialog, 20, g_text_output_buf);
}

/* R03_04D4 */
void CopyProt::take_answer()
{
	strcpy((char far *)answer, g_input_buf);
}

/* R03_04EF: PASSWORD.DAT oeffnen, Zahl der Saetze lesen */
void CopyProt::open_file()
{
	file_open(g_file_main, FILE_PASSWORD_DAT);
	file_read(g_file_main, &nr_recs, 2);
}

/* R03_051C: Satz lesen, Schluessel uebernehmen */
void CopyProt::read_record()
{
	file_read(g_file_main, &rec, 25);
	key1 = rec.key1;
	key2 = rec.key2;
}

/* R03_0558: Fuellwerte (ohne Wirkung) */
void CopyProt::set_f8(signed short v)
{
	signed short n;

	for (n = 0; n < 2; n++) {
		f8 = v;
		f9 = answer[0] + 1;
		fa = answer[0] + 2;
	}
}

/* R03_0591: PE-Bit des Maschinenstatusworts - CPU laeuft im V86-Modus (Speichermanager wie EMM386 aktiv) */
#pragma warn -rvl
signed short cpu_v86_active(void)
{
	asm smsw ax
	asm and ax, 1
}
#pragma warn .rvl
