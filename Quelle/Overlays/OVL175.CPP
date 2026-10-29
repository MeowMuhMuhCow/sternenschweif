/* OVL175 - Overlay O175: Meldungsliste des Kampfes FightMsgs (struct fight_msgs, 0x14e Bytes; einziges Objekt
 * g_fight_list) mit bis zu 15 Meldungen struct fight_msg (14 Bytes ab +8: Ereignis, Wert, zwei Objekte mit Art).
 * Konstruktor (Texte 0x9a ueber txt_load_list(g_font, 0x9a, 0x17, ...), dann init()), Destruktor (Texte freigeben),
 * set_screen(p) -> +0xde, init() (Meldungen loeschen, Anzeigedauer +4/+6 aus g_fight_speed bzw. g_text_speed),
 * add/add2/add0 (Meldung anhaengen, Zaehler bis 14), run() (Klang nachladen, fight_draw bis != 1, dann die
 * Meldungsleiste fight_add_rect(0, 140, 90, 199) und ein letztes fight_draw). Freie Namen Lst_* in s_proto.h.
 *
 * BCC -mlarge -3 -Ot -Oi -c -Y OVL175.CPP   -> 946/946 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * Schluessel: [bp-2] in init() ist eine echte Variable speed ("if ((speed = g_fight_speed) == 0) ... else
 *   delay = speed * 4 + 1"). obj1/obj2 im Eintrag sind far-Zeiger (Kopie ueber ax/dx), +0xde wird als long
 *   uebergeben (Kopie ueber eax; set_screen nimmt signed long).
 *   memset(e, 0, sizeof(e)) mit -Oi -> rep stosw; das "mov cl,[bp+0eh]" in add2 macht BCC von selbst.
 *   -Og schadet hier (init wird anders).
 */
#include <mem.h>
#include "schweif.h"

#define FIGHT_MSG_MAX	14		/* letzter Index (15 Plaetze) */

FightMsgs::FightMsgs()
{
	txt_load_list(g_font, 0x9a, 0x17, &txt_buf, &txt);
	init();
}

FightMsgs::~FightMsgs()
{
	mem_free(txt_buf);
	mem_free(txt);
}

void FightMsgs::set_screen(signed long p)
{
	screen = (void far *)p;
}

void FightMsgs::init()
{
	signed short speed;

	memset(e, 0, sizeof(e));
	count = 0;
	if (all_heroes_down(g_group)) {
		if ((speed = g_fight_speed) == 0) delay = 5;
		else delay = speed * 4 + 1;
	} else {
		delay = g_text_speed * 4 + 2;
	}
	delay_init = delay;
	snd_done = 0;
	active = 0;
}

void FightMsgs::add(signed short id, signed short value, signed char type1, void far *obj1, signed char type2,
	void far *obj2)
{
	e[count].id = id;
	e[count].value = value;
	e[count].type1 = type1;
	e[count].obj1 = obj1;
	e[count].type2 = type2;
	e[count].obj2 = obj2;
	if (count < FIGHT_MSG_MAX) count++;
}

/* Meldung mit nur einem Objekt (als beide Objekte eingetragen) */
void FightMsgs::add2(signed short id, signed short value, signed char type, void far *obj)
{
	e[count].id = id;
	e[count].value = value;
	e[count].type1 = type;
	e[count].obj1 = obj;
	e[count].type2 = type;
	e[count].obj2 = obj;
	if (count < FIGHT_MSG_MAX) count++;
}

/* Meldung ohne Objekt */
void FightMsgs::add0(signed short id, signed short value)
{
	e[count].id = id;
	e[count].value = value;
	e[count].type1 = 0;
	e[count].obj1 = 0;
	e[count].type2 = 0;
	e[count].obj2 = 0;
	if (count < FIGHT_MSG_MAX) count++;
}

/* Meldungen abspielen: Kampfbild zeichnen, bis fight_draw nicht mehr 1 (Meldungen laufen) liefert */
void FightMsgs::run()
{
	register signed short busy;

	active = 1;
	do {
		snd_service(g_snd_music);
		snd_speech_service(g_snd_speech);
		snd_service(g_snd_fx);
		busy = fight_draw((Fight far *)g_fight_obj);
	} while (busy == 1);
	fight_add_rect(0, 140, 90, 199);
	fight_draw((Fight far *)g_fight_obj);
}
