/* OVL242 - Overlay O242 (Segment 0x2453 in SCHWEIF.EXE): Bestellungen und Ereignis der Schenke (struct tavern far *o,
 * Klasse Tavern aus O240; Texte o->txt, price_mod/quality, female, done). tavern_order_dinner(o) (O242_0000, Schenken
 * 3..5): beim ersten Mal (g_tavern_dinner_asked) Frage mit 3, sonst gleich mit 4 Antworten (dlg_choose), Text mit
 * Gruppengroesse (group_count_check2b); Wahl 1-3: group_sub_hunger_thirst_max(Gruppe, 10/4/3, (21 - quality) * 30),
 * Meldung, 30 Minuten, tavern_add_bill(o, 2..4); Wahl 4: tavern_pay_meal. tavern_pay_meal(o): Preis =
 * group_count_alive * (6 - quality / 4) plus price_mod %, Ja/Nein (dlg_yes_no), 1 Stunde, Bezahlen
 * (group_sub_money) oder bei zu wenig Geld alles abgeben, g_tavern_banned[g_loc_id] = 1, group_sub_le_range und
 * o->done = 1. tavern_order_drink(o) (Schenken 9..18): Ja/Nein, dann group_sub_hunger_thirst_max(.., 2, ..),
 * Meldung, 30 Minuten, tavern_add_bill/tavern_drink_test(o, 1). tavern_night_event(o): Ereignis in Schenke 14 ab
 * 20 Uhr, wenn group.town == 4, g_low_stone_delivered == 1 und g_gavron_chase == 0: Portraet 0xd8 (Bmp 32x32 aus
 * g_portrait_buf, gb_new/gb_delete), Auswahl dlg_box_bmp, bei 1 o->done = 1 und tavern_pay_bill(o, 0x44) ->
 * g_gavron_chase = 1 bzw. Meldung, sonst g_gavron_chase = -2.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL242.C   -> 2424/2424 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Reines C; alle Texte als o->txt[n], Varianten als ?: (o->female ? txt[0x80] : txt[0x7f]).
 * - "cost += cost * o->price_mod / 100" (movsx/push eax/mov eax,[bp-4]/pop edx/imul); mit o->price_mod * cost wird
 *   imul eax,[bp-4] erzeugt und die folgenden les-Ladevorgaenge fallen weg.
 * - Konstante 30L/1L als Argument -> o32 push byte (66 6A xx).
 * - tavern_order_drink: "if (..) { ..; return 1; } return 0;" (mit else: jmp short + anderes Epilogbild).
 * - !g_tavern_dinner_asked (signed char) -> mov al/cbw/or ax,ax; Bmp als C-Aufrufe gb_new(0, ..)/gb_delete(bmp, 3).
 */
#include <stdio.h>
#include "schweif.h"

signed short tavern_pay_meal(struct tavern far *o);

signed short tavern_order_dinner(struct tavern far *o)
{
	signed short ret;
	register signed short sel;
	register signed short n;

	ret = 1;
	n = group_count_check2b(g_group);
	if (!g_tavern_dinner_asked) {
		g_tavern_dinner_asked = 1;
		if (n > 1)
			sprintf(g_text_output_buf, o->txt[0x96], group_count_check2b(g_group));
		else
			sprintf(g_text_output_buf, o->txt[0x3c]);
		sel = dlg_choose(g_dialog, o->txt[0x3b], 3, g_text_output_buf, o->txt[0x3d], o->txt[0x3e]);
		if (sel == 1 || sel == 2) {
			sprintf(g_text_output_buf, o->txt[0x86], o->female ? o->txt[0x80] : o->txt[0x7f]);
			dlg_message(g_dialog, g_text_output_buf);
			if (n > 1)
				sprintf(g_text_output_buf, o->txt[0x99], n);
			else
				sprintf(g_text_output_buf, o->txt[0x87]);
			sel = dlg_choose(g_dialog, o->txt[0x3b], 4, g_text_output_buf, o->txt[0x88], o->txt[0x89], o->txt[0x3e]);
		} else
			sel = 4;
	} else {
		if (n > 1)
			sprintf(g_text_output_buf, o->txt[0x99], n);
		else
			sprintf(g_text_output_buf, o->txt[0x87]);
		sel = dlg_choose(g_dialog, o->txt[0x3b], 4, g_text_output_buf, o->txt[0x88], o->txt[0x89], o->txt[0x3e]);
	}
	switch (sel) {
	case 1:
		group_sub_hunger_thirst_max(g_group, 10, (21 - o->quality) * 30);
		sprintf(g_text_output_buf, o->txt[0x8a], o->female ? o->txt[0x80] : o->txt[0x7f]);
		dlg_message(g_dialog, g_text_output_buf);
		timewarp_minutes(g_date, 30L);
		tavern_add_bill(o, 2);
		break;
	case 2:
		group_sub_hunger_thirst_max(g_group, 4, (21 - o->quality) * 30);
		sprintf(g_text_output_buf, o->txt[0x8b], o->female ? o->txt[0x82] : o->txt[0x81]);
		dlg_message(g_dialog, g_text_output_buf);
		timewarp_minutes(g_date, 30L);
		tavern_add_bill(o, 3);
		break;
	case 3:
		group_sub_hunger_thirst_max(g_group, 3, (21 - o->quality) * 30);
		sprintf(g_text_output_buf, o->txt[0x8c], o->female ? o->txt[0x82] : o->txt[0x81]);
		dlg_message(g_dialog, g_text_output_buf);
		timewarp_minutes(g_date, 30L);
		tavern_add_bill(o, 4);
		break;
	case 4:
		ret = tavern_pay_meal(o);
		break;
	default:
		ret = 0;
		break;
	}
	return ret;
}

signed short tavern_pay_meal(struct tavern far *o)
{
	signed long cost;
	register signed short d;
	register signed short ret;

	ret = 0;
	cost = group_count_alive(g_group) * (6 - (o->quality >> 2));
	cost += cost * o->price_mod / 100;
	sprintf(g_text_output_buf, o->txt[0x24], (signed short)cost, o->female ? o->txt[0x80] : o->txt[0x7f]);
	if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
		d = 21 - o->quality;
		if (d < 1)
			d = 1;
		group_sub_hunger_thirst_max(g_group, d * 20, d * 30);
		dlg_message(g_dialog, o->quality < 5 ? o->txt[0x8d] : (o->quality < 15 ? o->txt[0x26] : o->txt[0x8e]));
		timewarp_hours(g_date, 1L);
		cost *= 10;
		if (g_group->money < cost) {
			sprintf(g_text_output_buf, o->txt[0x28], o->female ? o->txt[0x80] : o->txt[0x7f],
				o->female ? o->txt[0x80] : o->txt[0x7f]);
			dlg_message(g_dialog, g_text_output_buf);
			g_tavern_banned[g_loc_id] = 1;
			group_sub_le_range(g_group, 1, 3);
			group_sub_money(g_group, g_group->money);
			o->done = 1;
		} else {
			group_sub_money(g_group, cost);
			ret = 1;
		}
	}
	return ret;
}

signed short tavern_order_drink(struct tavern far *o)
{
	if (dlg_yes_no(g_dialog, o->txt[0x7d]) == 1) {
		group_sub_hunger_thirst_max(g_group, 2, (21 - o->quality) * 30);
		sprintf(g_text_output_buf, o->txt[0x7e], o->female ? o->txt[0x82] : o->txt[0x81]);
		dlg_message(g_dialog, g_text_output_buf);
		timewarp_minutes(g_date, 30L);
		tavern_add_bill(o, 1);
		tavern_drink_test(o, 1);
		return 1;
	}
	return 0;
}

void tavern_night_event(struct tavern far *o)
{
	signed short r;
	signed short old;
	void far *bmp;

	if (o->done == 0 && g_low_stone_delivered == 1 && g_group->town == 4 && g_loc_id == 14 && g_daytime >= (20 * TICKS_PER_HOUR) &&
	    g_gavron_chase == 0) {
		old = dlg_set16(g_dialog, 6);
		bmp = gb_new((void far *)0, 0x20, 0x20, g_portrait_buf);
		load_portrait_buf(0xd8);
		r = dlg_box_bmp(g_dialog, bmp, g_str_empty_75d8, o->txt[0x90], 2, o->txt[0x91], o->txt[0x92]);
		gb_delete(bmp, 3);
		if (r == 1) {
			o->done = 1;
			if (tavern_pay_bill(o, 0x44))
				g_gavron_chase = 1;
			else
				dlg_message(g_dialog, o->txt[0x93]);
		} else
			g_gavron_chase = -2;
		dlg_set16(g_dialog, old);
	}
}
