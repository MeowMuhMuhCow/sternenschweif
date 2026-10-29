/* OVL249 - Overlay O249 (Segment 0x2470 in SCHWEIF.EXE): Ereignisse einer zweiten Hafenstadt (Ereignisobjekt o vom Typ
 * Town aus O183, Texte o->txt). port_event(o, ev): Kartenereignis-Rueckruf wie O251 (pos = (Spalte - g_map_x0) << 8
 * | (Zeile - g_map_y0)); MAP_EVENT_ENTER: 0x1c07 (g_c715, ohne g_phex_finale_done) -> port_harbor_master, 0x1724/0x1725 ->
 * town_enter(LOC_DATA | 0x16) + group_set_screen_pos(group.x, 0x23) + cam_set_from_group, 0x1706/0x2107/0x906 ->
 * LOC_DATA | 0x17..0x19, 0xe09 -> txt[73], Feldtyp 3 -> town_swim_check; Aktion 1: switch (Vergleichsbaum) ->
 * LOC_DATA | 1..0x1b bzw. Sonderfaelle, sonst LOC_HOUSE; Aktion 100 txt[70], 120 port_harbor_master.
 * port_haggle: Verhandlung um die Ueberfahrt (Bild NVF 10/0x7c, dlg_box_bmp-Dialoge txt[23..52], Preis v), bei
 * Einigung g_port_price = v, port_pay(v*100); Ergebnis v*100. port_pay(o, cost): bezahlen (g_port_paid) bzw.
 * txt[53]/txt[71]. PortTxt: lokales Objekt (Puffer/Texte 0x21d/0x5a) mit Konstruktor/Destruktor. port_talk:
 * Gespraech (Bild 0xc4, je Antwort 1 Heller, g_port_talk/g_port_can_haggle). port_depart: Abfahrt (group.dungeon =
 * 5, group_set_screen_pos(0x10, 0x1b), strcpy(g_text_buf2, txt[79]) mit g_pending_msg, diary_add) oder Absage.
 * port_night_event (aus O178): Ereignis nach der Nacht (g_port_deal; mit g_trial_lost Bild 0x198/0x21, Stadt 2,
 * set_loctype(3)/loc_load_data(3)/set_loctype(0), group_sub_le_range; sonst 0x22 und alle per group_sub_le
 * verletzen). port_harbor_master: Hafenmeister (bezahlen 20-22 Uhr, abfahren 6-9 Uhr).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL249.CPP   -> 5616/5616 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   Ohne -Og 454 Abweichungen (port_night_event: set_loctype-Zwischenwert [bp-5]).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Oi fuer strcpy in port_depart (deshalb si/di gesichert); inline group::set_loctype wie O183.
 * - port_haggle: Locals in Deklarationsreihenfolge sel [bp-1], res [bp-2], w [bp-4], v [bp-6], die Startwerte als
 *   Anweisungen (v zuerst); "return (signed long)(v * 100);" -> movsx eax,ax / cwd (kein shld).
 * - Fall-Reihenfolgen aus den Sprungzielen; "case 2: v += 10; case 3:" faellt durch; letzte cases mit break.
 * - Town-Objekt als TOWN_PTR (s_class.h) in den Prototypen, Kartenereignis als struct map_event far *.
 * Originalfehler/Auffaelliges: port_haggle bei res == 1 wird die Auswahl mit "- 1" gelesen (alle anderen Menues
 *   1-basiert) - der erste Menuepunkt bewirkt nichts, die anderen verschieben sich um eins. Das lokale PortTxt
 *   (Texte 0x21d) in Fall 0x141d / Aktion 100 wird geladen und wieder freigegeben, die Meldung kommt aber aus
 *   o->txt. "if (v < 0) v = 1" laesst 0 zu.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

#define NVF_HEADS ((Nvf far *)g_nvf_heads)	/* NVF-Dateiobjekt (O098) */

class PortTxt {				/* 8 Bytes: Texte 0x21d (nur geladen und freigegeben) */
public:
	void far *txt_buf;		/* +0x00 */
	char far * far *txt;		/* +0x04 */
	PortTxt();
	~PortTxt();
};

signed short port_event(Town far *o, struct map_event far *ev)
{
	signed short loc;
	signed short r;
	register signed short type;
	register signed short xy;

	type = ev->type;
	xy = (ev->y - g_map_x0) << 8;
	xy |= ev->x - g_map_y0;
	r = 0;
	if (type == MAP_EVENT_ENTER) {
		if (g_mem_saving && xy == 0x1c07) {
			if (!g_phex_finale_done) {
				r = port_harbor_master(o);
			}
		} else if (xy == 0x1724 || xy == 0x1725) {
			town_enter(o, LOC_DATA | 0x16, ev);
			group_set_screen_pos(g_group->x, 0x23);
			cam_set_from_group();
			r = 1;
		} else if (xy == 0x1706) {
			town_enter(o, LOC_DATA | 0x17, ev);
			r = 1;
		} else if (xy == 0x2107) {
			town_enter(o, LOC_DATA | 0x18, ev);
			r = 1;
		} else if (xy == 0x906) {
			town_enter(o, LOC_DATA | 0x19, ev);
			r = 1;
		} else if (xy == 0xe09) {
			dlg_message(g_dialog, o->txt[73]);
			r = 1;
		}
		if (get_cell_a(ev->x, ev->y) == 3) {
			town_swim_check(o);
		}
	} else if (type == 1) {
		loc = 0;
		switch (xy) {
		case 0x1911:
			loc = LOC_DATA | 0x01;
			break;
		case 0x613:
			loc = LOC_DATA | 0x02;
			break;
		case 0x201b:
			loc = LOC_DATA | 0x03;
			break;
		case 0x1218:
			loc = LOC_DATA | 0x04;
			break;
		case 0x1622:
			loc = LOC_DATA | 0x05;
			break;
		case 0x1a1b:
			loc = LOC_DATA | 0x06;
			break;
		case 0x141d:
			{
				PortTxt t;
				dlg_message(g_dialog, o->txt[72]);
				r = 1;
			}
			break;
		case 0x1c0f:
			loc = LOC_DATA | 0x08;
			break;
		case 0x1916:
			loc = LOC_DATA | 0x09;
			break;
		case 0x1b1b:
			loc = LOC_DATA | 0x0a;
			break;
		case 0x1010:
			loc = LOC_DATA | 0x0b;
			break;
		case 0xc16:
			loc = LOC_DATA | 0x0c;
			break;
		case 0x101f:
			loc = LOC_DATA | 0x0d;
			break;
		case 0x1712:
			loc = LOC_DATA | 0x0e;
			break;
		case 0x1f12:
			loc = LOC_DATA | 0x0f;
			break;
		case 0x1d17:
			loc = LOC_DATA | 0x10;
			break;
		case 0x191d:
			loc = LOC_DATA | 0x11;
			break;
		case 0xf1b:
			loc = LOC_DATA | 0x12;
			break;
		case 0x1123:
			loc = LOC_DATA | 0x13;
			break;
		case 0x1724:
		case 0x1725:
			town_enter(o, LOC_DATA | 0x16, ev);
			group_set_screen_pos(g_group->x, 0x23);
			cam_set_from_group();
			r = 1;
			break;
		case 0x1706:
			loc = LOC_DATA | 0x17;
			break;
		case 0x2107:
			loc = LOC_DATA | 0x18;
			break;
		case 0x906:
			loc = LOC_DATA | 0x19;
			break;
		case 0xe09:
			dlg_message(g_dialog, o->txt[73]);
			r = 1;
			break;
		case 0x1314:
			loc = LOC_DATA | 0x1b;
			break;
		default:
			town_enter(o, LOC_HOUSE, ev);
			r = 1;
			break;
		}
		if (loc) {
			town_enter(o, loc, ev);
			r = 1;
		}
	} else if (type == 100) {
		PortTxt t;
		dlg_message(g_dialog, o->txt[70]);
		r = 1;
	} else if (type == 120) {
		r = port_harbor_master(o);
	}
	return r;
}

signed long port_haggle(Town far *o)
{
	signed char sel;
	signed char res;
	signed short w;
	signed short v;

	v = 0;
	sel = -1;
	res = 0;
	if (g_port_can_haggle) {
		Bmp b(0x20, 0x20);
		NVF_HEADS->load(0x0a, b.data(), 0, 0, 0x7c, 0, &w, &w, 0x140);
		dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[23], 0);
		while (sel == -1) {
			sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[24], 3, o->txt[25], o->txt[26], o->txt[27]);
		}
		switch (sel) {
		case 1:
			v += 20;
			sel = -1;
			while (sel == -1) {
				sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[28], 3, o->txt[29], o->txt[30], o->txt[31]);
			}
			switch (sel) {
			case 1:
				v += 30;
				res = 1;
				break;
			case 2:
				res = 2;
				break;
			case 3:
				v -= 15;
				res = 3;
				break;
			}
			break;
		case 2:
			v += 10;
			sel = -1;
			while (sel == -1) {
				sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[32], 3, o->txt[34], o->txt[35], o->txt[36]);
			}
			switch (sel) {
			case 1:
				res = 4;
				break;
			case 2:
				v += 20;
				res = 1;
				break;
			case 3:
				res = 2;
				break;
			}
			break;
		case 3:
			v += 30;
			sel = -1;
			while (sel == -1) {
				sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[33], 3, o->txt[37], o->txt[38], o->txt[39]);
			}
			switch (sel) {
			case 1:
				v += 35;
				res = 2;
				break;
			case 2:
				v -= 10;
				res = 3;
				break;
			case 3:
				v += 30;
				res = 1;
				break;
			}
			break;
		}
		sel = -1;
		switch (res) {
		case 1:
			sel = -1;
			while (sel == -1) {
				sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[40], 3, o->txt[44], o->txt[45], o->txt[46]) - 1;
			}
			break;
		case 2:
			sel = 3;
			break;
		case 3:
			sel = -1;
			while (sel == -1) {
				sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[41], 3, o->txt[45], o->txt[46], o->txt[47]);
			}
			break;
		case 4:
			sel = -1;
			while (sel == -1) {
				sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[42], 3, o->txt[45], o->txt[46], o->txt[47]);
			}
			break;
		}
		res = 0;
		switch (sel) {
		case 1:
			dlg_message(g_dialog, o->txt[48]);
			g_port_deal = 1;
			break;
		case 2:
			v += 10;
		case 3:
			sel = -1;
			while (sel == -1) {
				sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[43], 3, o->txt[49], o->txt[50], o->txt[51]);
			}
			switch (sel) {
			case 1:
				v += 10;
			case 2:
				if (v < 0) {
					v = 1;
				}
				sprintf(g_text_output_buf, o->txt[52], v);
				sel = dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 2, o->txt[68], o->txt[69]);
				if (sel == 1) {
					res = 1;
				} else {
					v += 20;
					sprintf(g_text_output_buf, o->txt[78], v);
					dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
				}
				break;
			}
		}
		if (res == 1) {
			g_port_price = v;
			port_pay(o, (signed long)(v * 100));
		}
	}
	return (signed long)(v * 100);
}

void port_pay(Town far *o, signed long cost)
{
	signed short w;
	signed long money = g_group->money;
	Bmp b(0x20, 0x20);

	NVF_HEADS->load(0x0a, b.data(), 0, 0, 0x7c, 0, &w, &w, 0x140);
	if (money >= cost) {
		group_sub_money(g_group, cost);
		dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[54], 0);
		g_port_paid = 1;
	} else if (!g_port_poor) {
		dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[53], 0);
		g_port_poor = 1;
		g_port_paid = 0;
	} else {
		dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[71], 0);
		g_port_can_haggle = 0;
	}
}

PortTxt::PortTxt()
{
	txt_load_list(g_font, 0x21d, 0x5a, &txt_buf, &txt);
}

PortTxt::~PortTxt()
{
	mem_free(txt_buf);
	mem_free(txt);
}

void port_talk(Town far *o)
{
	signed short w;
	signed short old;
	signed char sel;

	if (!g_port_talk) {
		old = dlg_set16(g_dialog, 5);
		Bmp b(0x20, 0x20);
		NVF_HEADS->load(0x0a, b.data(), 0, 0, 0xc4, 0, &w, &w, 0x140);
		sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[0], 2, o->txt[1], o->txt[2]);
		if (sel == 2 || sel == -1) {
			dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[3], 0);
		} else {
			if (g_group->money >= 1) {
				group_sub_money(g_group, 1L);
			}
			dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[4], 0);
			dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[5], 0);
			sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[6], 2, o->txt[17], o->txt[18]);
			if (sel == 1) {
				if (g_group->money >= 1) {
					group_sub_money(g_group, 1L);
				}
				dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[7], 0);
				dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[8], 0);
				sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[9], 2, o->txt[19], o->txt[20]);
				if (sel == 1) {
					if (g_group->money >= 1) {
						group_sub_money(g_group, 1L);
					}
					dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[10], 0);
					dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[11], 0);
					sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[12], 2, o->txt[21], o->txt[22]);
					if (sel == 1) {
						if (g_group->money >= 1) {
							group_sub_money(g_group, 1L);
						}
						dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[13], 0);
						dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[14], 0);
						dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[15], 0);
						dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[16], 0);
						g_port_can_haggle = 1;
						g_port_talk = 2;
					}
				}
			}
		}
		if (!g_port_can_haggle) {
			g_port_talk = 1;
		}
		dlg_set16(g_dialog, old);
	}
}

void port_depart(Town far *o)
{
	signed short w;
	signed char sel = -1;

	if (g_port_paid) {
		Bmp b(0x20, 0x20);
		NVF_HEADS->load(0x0a, b.data(), 0, 0, 0x7c, 0, &w, &w, 0x140);
		while (sel == -1) {
			sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[55], 2, o->txt[56], o->txt[57]);
		}
		if (sel == 2) {
			sel = -1;
			while (sel == -1) {
				sel = dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[59], 2, o->txt[60], o->txt[61]);
			}
		}
		if (sel == 1) {
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 5;
			group_set_screen_pos(0x10, 0x1b);
			g_group->dir = 2;
			dlg_message(g_dialog, o->txt[58]);
			strcpy(g_text_buf2, o->txt[79]);
			g_pending_msg = 1;
			diary_add(g_diary, g_ttx[384], 0);
		} else {
			g_port_can_haggle = g_port_paid = 0;
			g_port_haggled = 1;
			group_set_screen_pos(0x14, 0x1c);
			g_group->dir = 2;
			cam_set_from_group();
			g_cam_angle = (g_group->dir << 8) + 0x100;
		}
	}
}

void port_night_event(Town far *o)
{
	signed short w;
	signed short old;

	if (g_port_deal) {
		old = dlg_set16(g_dialog, 5);
		if (g_trial_lost) {
			Bmp b(0x40, 0x40);
			NVF_HEADS->load(0x198, b.data(), 0, 0, 0x21, 0, &w, &w, 0x140);
			dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[62], 0);
			dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[63], 0);
			dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[64], 0);
			dlg_message(g_dialog, o->txt[65]);
			g_gashok_arrested = 1;
			g_group->old_town = g_group->town;
			g_group->town = 2;
			group_set_screen_pos(0x0e, 0x0b);
			g_exit_dir = 0;
			g_group->set_loctype(LOC_INN);
			loc_load_data(3);
			g_group->set_loctype(LOC_NONE);
			group_sub_le_range(g_group, 2, 10);
		} else {
			Bmp b(0x40, 0x40);
			NVF_HEADS->load(0x198, b.data(), 0, 0, 0x22, 0, &w, &w, 0x140);
			dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[66], 0);
			dlg_box_bmp(g_dialog, &b, (char far *)0, o->txt[67], 0);
			while (group_count_alive(g_group)) {
				group_sub_le(g_group, 100);
			}
		}
		dlg_set16(g_dialog, old);
	}
}

signed short port_harbor_master(Town far *o)
{
	register signed short r = 0;

	if (g_port_deal) {
		dlg_message(g_dialog, o->txt[77]);
	} else if (g_port_haggled && g_port_paid == 0) {
		if (g_at_2000_2200) {
			port_pay(o, (signed long)(g_port_price * 100));
			r = 1;
		} else {
			dlg_message(g_dialog, o->txt[74]);
		}
	} else if (g_port_haggled && g_port_paid) {
		if (g_at_0600_0900) {
			port_depart(o);
			r = 1;
		} else {
			dlg_message(g_dialog, o->txt[75]);
		}
	} else if (!g_port_haggled) {
		if (g_port_can_haggle) {
			port_haggle(o);
			g_port_can_haggle = 0;
			g_port_haggled = 1;
			r = 1;
		} else {
			dlg_message(g_dialog, o->txt[76]);
		}
	} else {
		dlg_message(g_dialog, o->txt[77]);
	}
	return r;
}
