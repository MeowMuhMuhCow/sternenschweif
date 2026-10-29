/* OVL210 - Overlay O210 (Segment 0x239c in SCHWEIF.EXE): Ortsereignisse der Reisekarte (Reiseobjekt Travel).
 * travel_stage(o): Weg route_find(o, from, to) der Wegtabelle g_routes, Laenge nach route_len, Grenzwert v = len * 1000;
 * ohne Merker resume_10/50/90 Stufe 1/0/2 aus v/8, v/3..2v/3, 7v/8 gegen route_pos, sonst route_pos neu (v/10, v/2,
 * v/10*9; route.event >= 50 -> 1) und Merker loeschen (Rueckgabe dabei ggf. uninitialisiert, so im Original).
 * place_has_event(o, ort): 1 fuer die Orte 0x2c/0x2d/0x61/0x77/0x84..0x86 (Ereignis shrine_donation O164; O038 betritt
 * dort einen Dungeon). place_event(o): Ortsereignis - Ereignistexte 0x13e und je nach Ort (from) die Texte 0x145/
 * 0x144/0xac (txt_load_list nach g_tev_txt bzw. g_tev_loc_txt), grosses switch Ort -> place_event_<Ort> (O017, O125,
 * O126, O127, O164), umrahmt von travel_ui_unload/travel_ui_load (wenn place_unloads_ui), Dialogbreite 5, danach Texte
 * frei, Rueckgabe g_on_boat == 1. place_unloads_ui(o): Ort in der Liste g_scene_places (is_in_byte_array2).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL210.CPP   -> 2163/2163 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Reiseobjekt Travel: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Og noetig: g_routes[idx] als CSE in SI (Segment spaeter neu aus ds:c7d2 geladen).
 * - C++ wegen des inline-Setters Travel::set_route_len(unsigned char) -> Parameterkopie [bp-9].
 * - place_unloads_ui steht hinter place_event -> Aufruf als far call (TLINK: nop/push cs/call near).
 * - Kette "resume_10 = resume_50 = resume_90 = 0" (rechts nach links, al wiederverwendet).
 * - switch-Faelle in Quellreihenfolge (7, 9, 0x11.., 0x27.., 0x2c/0x2d/0x61/0x77/0x84..0x86, 0x30, ...), gleiche
 *   Rumpfe (place_event_007, place_event_039, shrine_donation) bleiben getrennte Faelle; "default: break;" am Ende.
 * - shrine_donation (O164, ohne Parameter) wird mit o aufgerufen: gecasteter Funktionszeiger.
 * - Einzeiler-Pruefungen "if (...) return 1; else return 0;" -> zwei tote Epiloge; Ende von place_event als
 *   "return g_on_boat == 1 ? 1 : 0;".
 */
#include "schweif.h"

#define DLG		((Dlg far *)g_dialog)
#define TXT_TEV		0x13e		/* Reiseereignistexte (Datei, Anzahl) */
#define TXT_TEV_N	0x8c

signed char travel_stage(Travel far *o)
{
	signed char r;
	unsigned long v;
	signed short idx;

#if FIX_0005
	/* Fix #5: r vorbelegen: zwischen den Stufen kein Zufallsereignis aus Stapelmuell */
	r = -1;
	idx = route_find(o, o->from, o->to);
	o->set_route_len(g_routes[idx].len);
#else
	idx = route_find(o, o->from, o->to);
	o->set_route_len(g_routes[idx].len);
#endif
	v = (unsigned long)o->route_len * 1000;
	if (o->resume_10 == 0 && o->resume_50 == 0 && o->resume_90 == 0) {
		if (v / 8 > o->route_pos)
			r = 1;
		if (v / 3 < o->route_pos && v * 2 / 3 > o->route_pos)
			r = 0;
		if (v * 7 / 8 < o->route_pos)
			r = 2;
	} else {
		o->route_pos = 0;
		if (o->resume_10 == 1) {
			o->route_pos = v / 10;
			if (g_routes[idx].event >= 50)
				r = 1;
		}
		if (o->resume_50 == 1) {
			o->route_pos = v / 2;
			r = 1;
		}
		if (o->resume_90 == 1) {
			o->route_pos = v / 10 * 9;
			if (g_routes[idx].event >= 50)
				r = 1;
		}
		o->resume_10 = o->resume_50 = o->resume_90 = 0;
	}
	return r;
}

signed char place_has_event(Travel far *o, unsigned char c)
{
	if (c == 0x2c || c == 0x2d || c == 0x61 || c == 0x77 || c == 0x84 || c == 0x85 || c == 0x86)
		return 1;
	else
		return 0;
}

signed char place_event(Travel far *o)
{
	signed char r;
	signed char old;

	old = o->sub_scene;
	o->sub_scene = 1;
	txt_load_list(g_font, TXT_TEV, TXT_TEV_N, &g_tev_buf, &g_tev_txt);
	if (o->from <= 0x20 && o->from != 0x11 && o->from != 0x12) {
		o->evtxt_file = 0x145;
		o->evtxt_n = 0x5a;
	} else if ((o->from > 0x64 && o->from != 0xbb) || o->from == 0x11 || o->from == 0x12) {
		o->evtxt_file = 0x144;
		o->evtxt_n = 0x41;
	} else {
		o->evtxt_file = 0xac;
		o->evtxt_n = 0x41;
	}
	txt_load_list(g_font, o->evtxt_file, o->evtxt_n, &g_tev_loc_buf, &g_tev_loc_txt);
	r = DLG->set_width(5);
	if (place_unloads_ui(o))
		travel_ui_unload(o);
	switch (o->from) {
	case 0x07:
		place_event_007(o);
		break;
	case 0x09:
		place_event_007(o);
		break;
	case 0x11:
		place_event_017(o);
		break;
	case 0x12:
		place_event_018(o);
		break;
	case 0x15:
		place_event_021(o);
		break;
	case 0x16:
		place_event_022(o);
		break;
	case 0x17:
		place_event_023(o);
		break;
	case 0x18:
		place_event_024(o);
		break;
	case 0x19:
		place_event_025(o);
		break;
	case 0x1a:
		place_event_026(o);
		break;
	case 0x1b:
		place_event_027(o);
		break;
	case 0x1c:
		place_event_028(o);
		break;
	case 0x1d:
		place_event_029(o);
		break;
	case 0x1e:
		place_event_030(o);
		break;
	case 0x1f:
		place_event_031(o);
		break;
	case 0x20:
		place_event_032(o);
		break;
	case 0x27:
		place_event_039(o);
		break;
	case 0x28:
		place_event_039(o);
		break;
	case 0x29:
		place_event_041(o);
		break;
	case 0x2c:
		((void (*)(Travel far *))shrine_donation)(o);
		break;
	case 0x2d:
		((void (*)(Travel far *))shrine_donation)(o);
		break;
	case 0x61:
		((void (*)(Travel far *))shrine_donation)(o);
		break;
	case 0x77:
		((void (*)(Travel far *))shrine_donation)(o);
		break;
	case 0x84:
		((void (*)(Travel far *))shrine_donation)(o);
		break;
	case 0x85:
		((void (*)(Travel far *))shrine_donation)(o);
		break;
	case 0x86:
		((void (*)(Travel far *))shrine_donation)(o);
		break;
	case 0x30:
		place_event_048(o);
		break;
	case 0x41:
		place_event_065(o);
		break;
	case 0x42:
		place_event_066(o);
		break;
	case 0x53:
		place_event_083(o);
		break;
	case 0x57:
		place_event_087(o);
		break;
	case 0x64:
		place_event_100(o);
		break;
	case 0x67:
		place_event_103(o);
		break;
	case 0x68:
		place_event_104(o);
		break;
	case 0x6a:
		place_event_106(o);
		break;
	case 0x72:
		place_event_114(o);
		break;
	case 0x75:
		place_event_117(o);
		break;
	case 0x80:
		place_event_128(o);
		break;
	case 0x83:
		place_event_131(o);
		break;
	case 0x8e:
		place_event_142(o);
		break;
	case 0xaa:
		place_event_170(o);
		break;
	case 0xab:
		place_event_171(o);
		break;
	case 0xb7:
		place_event_183(o);
		break;
	case 0xb9:
		place_event_185(o);
		break;
	case 0xbb:
		place_event_187(o);
		break;
	case 0xbd:
		place_event_189(o);
		break;
	case 0xc4:
		place_event_196(o);
		break;
	case 0xc5:
		place_event_197(o);
		break;
	case 0xc6:
		place_event_198(o);
		break;
	default:
		break;
	}
	if (place_unloads_ui(o))
		travel_ui_load(o);
	DLG->set_width(r);
	mem_free(g_tev_loc_buf);
	mem_free(g_tev_loc_txt);
	g_tev_buf = mem_free(g_tev_buf);
	mem_free(g_tev_txt);
	o->sub_scene = old;
	return g_on_boat == 1 ? 1 : 0;
}

signed char place_unloads_ui(Travel far *o)
{
	if (is_in_byte_array2(o->from, g_scene_places) != -1)
		return 1;
	else
		return 0;
}
