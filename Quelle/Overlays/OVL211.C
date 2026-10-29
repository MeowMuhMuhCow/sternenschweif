/* OVL211 - Overlay O211 (Segment 0x23a0 in SCHWEIF.EXE): Reiseereignisse am Weg (Gegenstueck zu place_event O210;
 * Aufrufer travel_route O215, travel_follow_route O216, O222). Eine Funktion tevent_run(p, n): hurry/speed/sub_scene
 * sichern (speed = 0, sub_scene = 1), Ereignistexte 0x13e (0x8c Stueck) nach g_tev_txt, je nach Bereich von n die
 * Textdatei evtxt_file/evtxt_n (0..0x18 -> 0xab/60, 0x19..0x31 -> 0x13f/70, 0x32..0x4d ohne 0x4c -> 0x140/68,
 * 0x4e..0x63 und 0x4c/0x88 -> 0x141/55, 0x64..0xae -> 0x142/65, ab 0xaf -> 0x143/50) nach g_tev_loc_txt, Dialogbreite 5
 * (alte merken), travel_ui_unload; switch n 0..0x103 (Nummer des Weges, route.event > 0) auf 188 Ereignisse
 * tevent_<n> in O113..O124 (BE-Benennung wie DSA 1 tevent_NNN, je Fall ein Aufruf, 72 Werte ohne Fall); danach
 * travel_ui_load, Dialogbreite zurueck, Texte frei, speed zurueck, g_guide_won = 0; ist hurry waehrend des Ereignisses
 * von != 0 auf 0 gefallen: hurry = 1 und speed neu aus travel_speed_hurry(p, speed, 0); sub_scene zurueck.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL211.C   -> 4284/4284 Bytes GLEICH (18.10.2026), cobjfix laeuft. Mit -Og bzw.
 *   -Og -Oi ebenfalls GLEICH.
 *   (Namensfassung mit schweif.h, C-Sicht struct travel: 19.10.2026 GLEICH, LINKGLEICH)
 * - switch-Faelle aufsteigend 0x00..0x103, Rumpfreihenfolge = Wertreihenfolge (aus der Sprungtabelle bei
 *   +0eb4 per Skript erzeugt); keine default-Marke, fehlende Werte zeigen auf das Ende; auch der letzte
 *   Fall hat sein break (jmp short auf das unmittelbar folgende Ende).
 * - n ist unsigned (jnc/jc), register-Parameter in SI. Drei char-Locals in Deklarationsreihenfolge
 *   [bp-1]/[bp-2]/[bp-3], danach der long [bp-8].
 * - "g_tev_buf = mem_free(g_tev_buf)" (Rueckgabe dx:ax gespeichert), die anderen drei mem_free ohne Zuweisung.
 * Auffaellig (Original): sub_scene (short) wird in einem signed char gesichert und mit cbw zurueckgeschrieben;
 *   nur g_tev_buf wird nach dem Freigeben genullt, g_tev_txt/g_tev_loc_buf/g_tev_loc_txt behalten ihre alten Zeiger.
 */
#include "schweif.h"

#define TXT_TEV		0x13e		/* Reiseereignistexte (Datei, Anzahl) */
#define TXT_TEV_N	0x8c

void tevent_run(struct travel far *p, register unsigned short n)
{
	signed char old;
#if FIX_0088
	/* Fix #88: sub_scene als short sichern */
	signed short old_scene;
#else
	signed char old_scene;
#endif
	signed char old_hurry;
	signed long old_speed;

	old_hurry = p->hurry;
	old_speed = p->speed;
	p->speed = 0;
	old_scene = p->sub_scene;
	p->sub_scene = 1;
	txt_load_list(g_font, TXT_TEV, TXT_TEV_N, &g_tev_buf, &g_tev_txt);
	if (n < 0x19) {
		p->evtxt_file = 0xab;
		p->evtxt_n = 0x3c;
	} else if (n < 0x32 && n >= 0x19) {
		p->evtxt_file = 0x13f;
		p->evtxt_n = 0x46;
	} else if (n < 0x4e && n >= 0x32 && n != 0x4c) {
		p->evtxt_file = 0x140;
		p->evtxt_n = 0x44;
	} else if (n < 0x64 && n >= 0x4e || n == 0x88 || n == 0x4c) {
		p->evtxt_file = 0x141;
		p->evtxt_n = 0x37;
	} else if (n < 0xaf && n >= 0x64 && n != 0x88) {
		p->evtxt_file = 0x142;
		p->evtxt_n = 0x41;
	} else if (n >= 0xaf) {
		p->evtxt_file = 0x143;
		p->evtxt_n = 0x32;
	}
	txt_load_list(g_font, p->evtxt_file, p->evtxt_n, &g_tev_loc_buf, &g_tev_loc_txt);
	old = dlg_set16(g_dialog, 5);
	travel_ui_unload(p);
	switch (n) {
	case 0x00:
		tevent_000(p);
		break;
	case 0x01:
		tevent_001(p);
		break;
	case 0x02:
		tevent_002(p);
		break;
	case 0x03:
		tevent_003(p);
		break;
	case 0x04:
		tevent_004(p);
		break;
	case 0x05:
		tevent_005(p);
		break;
	case 0x06:
		tevent_006(p);
		break;
	case 0x07:
		tevent_007(p);
		break;
	case 0x08:
		tevent_008(p);
		break;
	case 0x09:
		tevent_009(p);
		break;
	case 0x0a:
		tevent_010(p);
		break;
	case 0x0c:
		tevent_012(p);
		break;
	case 0x0d:
		tevent_013(p);
		break;
	case 0x0e:
		tevent_014(p);
		break;
	case 0x10:
		tevent_016(p);
		break;
	case 0x11:
		tevent_017(p);
		break;
	case 0x13:
		tevent_019(p);
		break;
	case 0x14:
		tevent_020(p);
		break;
	case 0x16:
		tevent_022(p);
		break;
	case 0x17:
		tevent_023(p);
		break;
	case 0x18:
		tevent_024(p);
		break;
	case 0x19:
		tevent_025(p);
		break;
	case 0x1e:
		tevent_030(p);
		break;
	case 0x1f:
		tevent_031(p);
		break;
	case 0x20:
		tevent_032(p);
		break;
	case 0x22:
		tevent_034(p);
		break;
	case 0x23:
		tevent_035(p);
		break;
	case 0x24:
		tevent_036(p);
		break;
	case 0x25:
		tevent_037(p);
		break;
	case 0x26:
		tevent_038(p);
		break;
	case 0x27:
		tevent_039(p);
		break;
	case 0x29:
		tevent_041(p);
		break;
	case 0x2a:
		tevent_042(p);
		break;
	case 0x2b:
		tevent_043(p);
		break;
	case 0x2c:
		tevent_044(p);
		break;
	case 0x2d:
		tevent_045(p);
		break;
	case 0x2e:
		tevent_046(p);
		break;
	case 0x30:
		tevent_048(p);
		break;
	case 0x31:
		tevent_049(p);
		break;
	case 0x32:
		tevent_050(p);
		break;
	case 0x33:
		tevent_051(p);
		break;
	case 0x34:
		tevent_052(p);
		break;
	case 0x35:
		tevent_053(p);
		break;
	case 0x36:
		tevent_054(p);
		break;
	case 0x37:
		tevent_055(p);
		break;
	case 0x3b:
		tevent_059(p);
		break;
	case 0x3d:
		tevent_061(p);
		break;
	case 0x3e:
		tevent_062(p);
		break;
	case 0x3f:
		tevent_063(p);
		break;
	case 0x40:
		tevent_064(p);
		break;
	case 0x42:
		tevent_066(p);
		break;
	case 0x43:
		tevent_067(p);
		break;
	case 0x45:
		tevent_069(p);
		break;
	case 0x46:
		tevent_070(p);
		break;
	case 0x47:
		tevent_071(p);
		break;
	case 0x48:
		tevent_072(p);
		break;
	case 0x49:
		tevent_073(p);
		break;
	case 0x4b:
		tevent_075(p);
		break;
	case 0x4c:
		tevent_076(p);
		break;
	case 0x4d:
		tevent_077(p);
		break;
	case 0x4e:
		tevent_078(p);
		break;
	case 0x51:
		tevent_081(p);
		break;
	case 0x52:
		tevent_082(p);
		break;
	case 0x53:
		tevent_083(p);
		break;
	case 0x54:
		tevent_084(p);
		break;
	case 0x55:
		tevent_085(p);
		break;
	case 0x56:
		tevent_086(p);
		break;
	case 0x57:
		tevent_087(p);
		break;
	case 0x58:
		tevent_088(p);
		break;
	case 0x59:
		tevent_089(p);
		break;
	case 0x5c:
		tevent_092(p);
		break;
	case 0x5d:
		tevent_093(p);
		break;
	case 0x5e:
		tevent_094(p);
		break;
	case 0x60:
		tevent_096(p);
		break;
	case 0x61:
		tevent_097(p);
		break;
	case 0x62:
		tevent_098(p);
		break;
	case 0x63:
		tevent_099(p);
		break;
	case 0x64:
		tevent_100(p);
		break;
	case 0x65:
		tevent_101(p);
		break;
	case 0x67:
		tevent_103(p);
		break;
	case 0x68:
		tevent_104(p);
		break;
	case 0x69:
		tevent_105(p);
		break;
	case 0x6a:
		tevent_106(p);
		break;
	case 0x6c:
		tevent_108(p);
		break;
	case 0x6d:
		tevent_109(p);
		break;
	case 0x6e:
		tevent_110(p);
		break;
	case 0x6f:
		tevent_111(p);
		break;
	case 0x70:
		tevent_112(p);
		break;
	case 0x71:
		tevent_113(p);
		break;
	case 0x72:
		tevent_114(p);
		break;
	case 0x74:
		tevent_116(p);
		break;
	case 0x75:
		tevent_117(p);
		break;
	case 0x77:
		tevent_119(p);
		break;
	case 0x79:
		tevent_121(p);
		break;
	case 0x7a:
		tevent_122(p);
		break;
	case 0x7b:
		tevent_123(p);
		break;
	case 0x7d:
		tevent_125(p);
		break;
	case 0x7e:
		tevent_126(p);
		break;
	case 0x7f:
		tevent_127(p);
		break;
	case 0x80:
		tevent_128(p);
		break;
	case 0x81:
		tevent_129(p);
		break;
	case 0x83:
		tevent_131(p);
		break;
	case 0x84:
		tevent_132(p);
		break;
	case 0x85:
		tevent_133(p);
		break;
	case 0x86:
		tevent_134(p);
		break;
	case 0x87:
		tevent_135(p);
		break;
	case 0x88:
		tevent_136(p);
		break;
	case 0x89:
		tevent_137(p);
		break;
	case 0x8a:
		tevent_138(p);
		break;
	case 0x8c:
		tevent_140(p);
		break;
	case 0x8d:
		tevent_141(p);
		break;
	case 0x8e:
		tevent_142(p);
		break;
	case 0x8f:
		tevent_143(p);
		break;
	case 0x90:
		tevent_144(p);
		break;
	case 0x91:
		tevent_145(p);
		break;
	case 0x92:
		tevent_146(p);
		break;
	case 0x93:
		tevent_147(p);
		break;
	case 0x94:
		tevent_148(p);
		break;
	case 0x95:
		tevent_149(p);
		break;
	case 0x97:
		tevent_151(p);
		break;
	case 0x98:
		tevent_152(p);
		break;
	case 0x99:
		tevent_153(p);
		break;
	case 0x9a:
		tevent_154(p);
		break;
	case 0x9d:
		tevent_157(p);
		break;
	case 0x9e:
		tevent_158(p);
		break;
	case 0x9f:
		tevent_159(p);
		break;
	case 0xa0:
		tevent_160(p);
		break;
	case 0xa1:
		tevent_161(p);
		break;
	case 0xa2:
		tevent_162(p);
		break;
	case 0xa3:
		tevent_163(p);
		break;
	case 0xa4:
		tevent_164(p);
		break;
	case 0xa5:
		tevent_165(p);
		break;
	case 0xa6:
		tevent_166(p);
		break;
	case 0xa7:
		tevent_167(p);
		break;
	case 0xa8:
		tevent_168(p);
		break;
	case 0xaa:
		tevent_170(p);
		break;
	case 0xab:
		tevent_171(p);
		break;
	case 0xac:
		tevent_172(p);
		break;
	case 0xae:
		tevent_174(p);
		break;
	case 0xaf:
		tevent_175(p);
		break;
	case 0xb1:
		tevent_177(p);
		break;
	case 0xb2:
		tevent_178(p);
		break;
	case 0xb3:
		tevent_179(p);
		break;
	case 0xb5:
		tevent_181(p);
		break;
	case 0xb6:
		tevent_182(p);
		break;
	case 0xb7:
		tevent_183(p);
		break;
	case 0xb9:
		tevent_185(p);
		break;
	case 0xba:
		tevent_186(p);
		break;
	case 0xbb:
		tevent_187(p);
		break;
	case 0xbc:
		tevent_188(p);
		break;
	case 0xbe:
		tevent_190(p);
		break;
	case 0xbf:
		tevent_191(p);
		break;
	case 0xc0:
		tevent_192(p);
		break;
	case 0xc1:
		tevent_193(p);
		break;
	case 0xc3:
		tevent_195(p);
		break;
	case 0xc6:
		tevent_198(p);
		break;
	case 0xc8:
		tevent_200(p);
		break;
	case 0xc9:
		tevent_201(p);
		break;
	case 0xca:
		tevent_202(p);
		break;
	case 0xcc:
		tevent_204(p);
		break;
	case 0xcd:
		tevent_205(p);
		break;
	case 0xd1:
		tevent_209(p);
		break;
	case 0xd3:
		tevent_211(p);
		break;
	case 0xd5:
		tevent_213(p);
		break;
	case 0xd6:
		tevent_214(p);
		break;
	case 0xd9:
		tevent_217(p);
		break;
	case 0xda:
		tevent_218(p);
		break;
	case 0xdd:
		tevent_221(p);
		break;
	case 0xe1:
		tevent_225(p);
		break;
	case 0xe4:
		tevent_228(p);
		break;
	case 0xe5:
		tevent_229(p);
		break;
	case 0xe6:
		tevent_230(p);
		break;
	case 0xea:
		tevent_234(p);
		break;
	case 0xee:
		tevent_238(p);
		break;
	case 0xef:
		tevent_239(p);
		break;
	case 0xf2:
		tevent_242(p);
		break;
	case 0xf4:
		tevent_244(p);
		break;
	case 0xf5:
		tevent_245(p);
		break;
	case 0xf6:
		tevent_246(p);
		break;
	case 0xf7:
		tevent_247(p);
		break;
	case 0xf8:
		tevent_248(p);
		break;
	case 0xfa:
		tevent_250(p);
		break;
	case 0xfb:
		tevent_251(p);
		break;
	case 0xfd:
		tevent_253(p);
		break;
	case 0xfe:
		tevent_254(p);
		break;
	case 0x101:
		tevent_257(p);
		break;
	case 0x102:
		tevent_258(p);
		break;
	case 0x103:
		tevent_259(p);
		break;
	}
	travel_ui_load(p);
	dlg_set16(g_dialog, old);
#if FIX_0088
	/* Fix #88: alle vier Zeiger nach dem Freigeben nullen */
	g_tev_loc_buf = mem_free(g_tev_loc_buf);
	g_tev_loc_txt = mem_free(g_tev_loc_txt);
	g_tev_buf = mem_free(g_tev_buf);
	g_tev_txt = mem_free(g_tev_txt);
#else
	mem_free(g_tev_loc_buf);
	mem_free(g_tev_loc_txt);
	g_tev_buf = mem_free(g_tev_buf);
	mem_free(g_tev_txt);
#endif
	p->speed = old_speed;
	g_guide_won = 0;
	if (p->hurry == 0 && p->hurry != old_hurry) {
		p->hurry = 1;
		p->speed = travel_speed_hurry(p, p->speed, 0);
	}
	p->sub_scene = old_scene;
}
