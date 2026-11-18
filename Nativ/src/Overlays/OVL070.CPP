/* OVL070 - Overlay O070 (Segment 0x2098 in SCHWEIF.EXE): Rasthaus LOC_LODGING - class Lodging (s_class.h, 0x14
 * Bytes). Lodging(): Texte/Bild (txt_load_list 0x195, Bild 0x191) und den 4-Byte-Datensatz Nr. g_loc_id aus Datei
 * 0x1fe (g_file_3) nach info (regen: Erholungszuschlag %, markup: Preisaufschlag %); is_open: Hausverbot
 * (g_lodging_banned[g_loc_id]) -> Text 7, Schaden, 0; run: ask_price, dann Menueschleife (0x81 Essen eat, 0x82
 * Uebernachten sleep (bis 8 Uhr, Erholung nach Uhrzeit), 0x83/ESC Ende); ask_price: Preis = 10 + markup %,
 * Abfrage, * 10 * group_count_check2b(Gruppe); pay: bezahlen (money_to_string-Geldtext) oder Hausverbot bei zu
 * wenig Geld.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL070.CPP   -> 1588/1588 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Konstruktor/Destruktor normal (new bei this==0, delete bei flags&1 automatisch); mem_free = Freigabe.
 * - price += price * info.markup / 100; ergibt "movsx markup / push eax / mov eax,price / pop edx / imul eax,edx"
 *   (BCC tauscht die Operanden); markup * price ergibt imul eax,[mem].
 * - Antwortvariablen in run()/ask_price als register (SI); dlg_choose variadisch (Anzahl, dann Texte).
 * - Tageszeit ueber DATE->ticks (Fixup auf g_date + 1 wie im Original, nicht g_daytime).
 * - screen_textbar mit int-Farbe (push word) ueber gecasteten Funktionszeiger LOC_TEXTBAR.
 */
#include <stdio.h>
#include "schweif.h"

#define DATE ((struct game_date far *)g_date)
/* Ortsname in die Textleiste: Farbe 0xff als int wie im Original */
#define LOC_TEXTBAR(t) ((void (*)(void far *, char far *, signed short))screen_textbar)(g_screen, t, 0xff)

Lodging::Lodging()
{
	txt_load_list(g_font, 0x195, 0x10, &txt_buf, &txt);
	arc_open(g_file_3, 0x1fe);
	arc_seek(g_file_3, (signed long)g_loc_id * 4, 0);
	arc_read(g_file_3, &info, 4);
	arc_close(g_file_3);
	pic = 0x191;
	done = 0;
	g_refresh = 1;
}

Lodging::~Lodging()
{
	mem_free(txt_buf);
	mem_free(txt);
}

signed short Lodging::is_open()
{
	register signed short ok = 1;

	if (g_lodging_banned[g_loc_id]) {
		LOC_TEXTBAR(txt[g_loc_name]);
		dlg_message(g_dialog, txt[7]);
		group_sub_le_range(g_group, 1, 2);
		ok = 0;
	}
	return ok;
}

void Lodging::run()
{
	register signed short sel;

	ask_price();
	while (done == 0) {
		if (g_refresh) {
			draw_loc_icons(g_screen, 3, 0x3c, 0x11, 8);
			screen_redraw(g_screen);
			anim_load(g_screen_pic, pic, 0);
			LOC_TEXTBAR(txt[g_loc_name]);
			snd_play(g_snd_music, 0x0c);
			g_refresh = 0;
		}
		handle_input(g_screen);
		if (g_mouse_rclick != 0 || g_key == 0x4900) {
			sel = dlg_choose(g_dialog, txt[0], 3, txt[1], txt[2], txt[3]) - 1;
			if (sel != -2)
				g_action = sel + 0x81;
		}
		if (g_action == 0x81)
			eat();
		else if (g_action == 0x82)
			sleep();
		else if (g_action == 0x83 || g_key == 0x11b)
			done = 1;
		if (g_leave_location)
			done = 1;
	}
}

void Lodging::ask_price()
{
	register signed short ans;

	draw_loc_icons(g_screen, 3, 0x3c, 0x11, 8);
	screen_redraw(g_screen);
	anim_load(g_screen_pic, pic, 0);
	LOC_TEXTBAR(txt[g_loc_name]);
	snd_play(g_snd_music, 0x0c);
	g_refresh = 0;
	price = 10;
	price += price * info.markup / 100;
	sprintf(g_text_output_buf, txt[4], price);
	ans = dlg_choose(g_dialog, g_text_output_buf, 2, txt[5], txt[6]);
	if (ans == 2 || ans == -1)
		done = 1;
	price *= 10;
	price *= group_count_check2b(g_group);
}

void Lodging::eat()
{
	group_sub_hunger_thirst_max(g_group, 100, 100);
	dlg_message(g_dialog, txt[8]);
	timewarp_hours(g_date, 1);
	pay();
}

void Lodging::sleep()
{
	signed char regen;

	regen = 0;
	regen -= DATE->ticks / TICKS_PER_HOUR;
	regen += (regen > 8) ? 8 : 32;
	regen += regen * info.regen / 100;
	group_sub_hunger_thirst_max(g_group, 100, 100);
	price *= 10;
	dlg_message(g_dialog, txt[9]);
	timewarp_until(g_date, 43200);		/* 8 Uhr */
	group_sleep(g_group, regen);
	group_sub_hunger_thirst_max(g_group, 100, 100);
	pay();
}

void Lodging::pay()
{
	if (price != 0) {
		if (g_group->money < price) {
			dlg_message(g_dialog, txt[10]);
			g_lodging_banned[g_loc_id] = 1;
			group_sub_le_range(g_group, 2, 4);
		} else {
			money_to_string(g_text_buf2, price);
			sprintf(g_text_output_buf, txt[11], g_text_buf2);
			dlg_message(g_dialog, g_text_output_buf);
			group_sub_money(g_group, price);
		}
	}
	done = 1;
}
