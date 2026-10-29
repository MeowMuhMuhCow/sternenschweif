/* OVL258 - Overlay O258 (Segment 0x249a in SCHWEIF.EXE): 44 Rueckrufe der Gespraeche (Tabelle g_conv_callbacks ds:778e,
 * Index = Satz der Datei XDIALOG2.DAT je Gespraech: Skript-TLK und Rueckruf; Gegenstueck inn_conv_callback in O179,
 * tavern_talk_event2/O241, O230). Aufruf f(a, b): a == 1 nach der Antwort b (Aktion), a == 0 Abfrage b (Bedingung,
 * Ergebnis int; -1 = Gespraech neu zeichnen). Typische Aktionen: Themenzustand conv_set_topic(g_dialog2, Thema, 2),
 * Skriptthema freischalten conv_unlock_topic(g_dialog2, Thema) mit Merkerbyte g_topic_finsterkoppen..g_topic_gavron, Zimmer/Essen der Herberge
 * (g_inn), Szenen aus O259 (conv_scene_*, Texte XDS.LTX), conv_finish. Die Namen folgen den Skriptdateien (TLK) der
 * Gespraeche, deren Texte nicht im Klartext vorliegen; die Bedeutung der meisten Themen und Merker ist offen.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL258.CPP   -> 4027/4027 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit -Og ebenso)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ nur fuer das lokale Sites-Objekt (O166) in conv_cb_tavgas.
 * - Register: KEIN register-Schluesselwort noetig - BCC verteilt SI/DI selbst (oefter benutzte Parameter und
 *   Ergebnisvariablen; Ergebnis nur initialisiert und zurueckgegeben bleibt auf [bp-2]); conv_cb_swamp2 ohne
 *   Aufrufe: b in DX, Ergebnis in BX.
 * - switch mit Sprungtabelle: der LETZTE case hat kein break und faellt in "default: break;" (ja-Sprung auf den
 *   jmp short davor); mit break + default zwei jmp, ohne default zielt ja aufs Ende. Ausnahme conv_cb_low62_dragan
 *   (case 6 mit break, kein default). switch mit 1..3 Faellen -> Vergleichskette; conv_cb_low65: "case 2: ..; break;
 *   default: break;" mit eigenem jmp.
 * - "r = g_tavgas_visits = -1" -> mov al,0ffh / Store / cbw; "g_artherion_heard = g_topic_artherion = 1" -> mov al,1 / +25ae zuerst.
 * - "b - 2" als Argument -> add ax,0fffeh; Argumente des inneren Aufrufs get_first_active_hero nach den aeusseren
 *   gepusht.
 * Originalfehler/Auffaelligkeiten: conv_cb_low42 faellt von case 1 in case 2 durch (g_low_ailian_state wird auch bei
 *   Antwort 1 gesetzt - evtl. gewollt); conv_cb_low35 ist eine exakte Kopie von conv_cb_frp25b; conv_cb_low02 =
 *   conv_cb_kvir01; conv_cb_gash/conv_cb_gasinfo2 liefern immer 0 (Ergebnisvariable wird nie gesetzt);
 *   conv_cb_inn5 steht nicht in der Tabelle. conv_cb_unused35 dagegen ist Eintrag 35 (Name irrefuehrend; Phase B:
 *   die Tabelle zeigt es, cobjfix_d prueft jeden Eintrag gegen das Original-DS).
 * - Phase B (19.10.2026): eigene Daten im Quelltext - g_conv_callbacks[47] als echte Funktionszeigertabelle (eigene
 *   Rueckrufe segmentrelativ, die aus O179/O230/O241 per EXTDEF).
 */
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:778e..784a, dsana) - initialisierte Daten in Adressreihenfolge */
CONV_CB g_conv_callbacks[47] = {	/* Rueckruf je Gespraech (Conv.callback, Satz in XDIALOG2.DAT), 0 = keiner */
	0,
	conv_cb_kvir01,	/*  1 */
	conv_cb_low04,	/*  2 */
	conv_cb_fins01,	/*  3 */
	conv_cb_low02,	/*  4 */
	conv_cb_low06,	/*  5 */
	conv_cb_low07,	/*  6 */
	conv_cb_low08,	/*  7 */
	conv_cb_low15,	/*  8 */
	conv_cb_low16,	/*  9 */
	conv_cb_low22,	/* 10 */
	conv_cb_low24,	/* 11 */
	conv_cb_low25,	/* 12 */
	conv_cb_low26,	/* 13 */
	conv_cb_low27,	/* 14 */
	conv_cb_low31,	/* 15 */
	conv_cb_low32_lore,	/* 16 */
	conv_cb_low33,	/* 17 */
	conv_cb_low34,	/* 18 */
	conv_cb_19,	/* 19 */
	conv_cb_20,	/* 20 */
	conv_cb_low42,	/* 21 */
	conv_cb_low43,	/* 22 */
	smith_talk_event,	/* 23 O230 */
	inn_conv_callback,	/* 24 O179 */
	tavern_talk_event,	/* 25 O241 */
	tavern_talk_event2,	/* 26 O241 */
	conv_cb_market,	/* 27 */
	conv_cb_tjol09_brothel,	/* 28 */
	conv_cb_low37,	/* 29 */
	conv_cb_frp183b_meal,	/* 30 */
	conv_cb_fst000_iwain,	/* 31 */
	conv_cb_fst167_music,	/* 32 */
	conv_cb_frp25b,	/* 33 */
	conv_cb_frp23b,	/* 34 */
	conv_cb_unused35,	/* 35 */
	conv_cb_frp32b,	/* 36 */
	conv_cb_low36,	/* 37 */
	conv_cb_low35,	/* 38 */
	conv_cb_low62_dragan,	/* 39 */
	conv_cb_low63_shed,	/* 40 */
	conv_cb_low65,	/* 41 */
	conv_cb_gash,	/* 42 */
	conv_cb_tavgas,	/* 43 */
	conv_cb_gasinfo2,	/* 44 */
	conv_cb_tief12b,	/* 45 */
	conv_cb_swamp2	/* 46 */
};

/* LOW04.TLK: Goldschluessel zur Binge - Zustand setzen; Abfrage 0x2a: Zustand, bei 2 den Schluessel geben (Rueckruf 4) */
signed short conv_cb_low04(signed short a, signed short b)
{
	if (a == 1) {
		g_binge_key_state = b;
	} else if (a == 0 && b == 0x2a) {
		if (g_binge_key_state == 2)
			group_give_item(g_group, 0xc3, 1);
		return g_binge_key_state;
	}
	return 0;
}

/* KVIR01.TLK: Antwort 1 beendet das Gespraech und entweiht den Tempel (g_temple) (Rueckruf 1) */
signed short conv_cb_kvir01(signed short a, signed short b)
{
	if (a == 1 && b == 1) {
		conv_finish(g_dialog2);
		if (g_temple != 0)
			temple_desecrate(g_temple);
	}
	return 0;
}

/* FINS01.TLK: Themen 0x48..0x4a; Antwort 4 zeigt das Bild LOWANGEN.AIF (o259_0048) (Rueckruf 2) */
signed short conv_cb_fins01(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			conv_set_topic(g_dialog2, TOPIC_ROGOLAN, 2);
			break;
		case 2:
			conv_set_topic(g_dialog2, TOPIC_FLUECHTLINGE, 2);
			break;
		case 3:
			conv_set_topic(g_dialog2, TOPIC_ECHSENMENSCHEN, 2);
			break;
		case 4:
			conv_scene_picture(g_dialog2);
			r = -1;
		default:
			break;
		}
	}
	return r;
}

/* LOW02.TLK: wie conv_cb_kvir01 (Rueckruf 3) */
signed short conv_cb_low02(signed short a, signed short b)
{
	if (a == 1 && b == 1) {
		conv_finish(g_dialog2);
		if (g_temple != 0)
			temple_desecrate(g_temple);
	}
	return 0;
}

/* LOW06.TLK: Themen 0x4b..0x4e (Rueckruf 5) */
signed short conv_cb_low06(signed short a, signed short b)
{
	if (a == 1) {
		switch (b) {
		case 1:
			conv_set_topic(g_dialog2, TOPIC_STADTMAUER, 2);
			break;
		case 2:
			conv_set_topic(g_dialog2, TOPIC_ARSINGEN, 2);
			break;
		case 3:
			conv_set_topic(g_dialog2, TOPIC_FLUCHTTUNNEL, 2);
			break;
		case 4:
			conv_set_topic(g_dialog2, TOPIC_SIBRA, 2);
		default:
			break;
		}
	}
	return 0;
}

/* LOW07.TLK: Thema 0x4f (Rueckruf 6) */
signed short conv_cb_low07(signed short a, signed short b)
{
	if (a == 1 && b == 1)
		conv_set_topic(g_dialog2, TOPIC_TRAVIATEMPEL, 2);
	return 0;
}

/* LOW08.TLK: Themen 0x50/0x51, Antwort 2 schaltet Thema 0x13 frei (g_topic_blutzinnen) (Rueckruf 7) */
signed short conv_cb_low08(signed short a, signed short b)
{
	if (a == 1) {
		switch (b) {
		case 1:
			conv_set_topic(g_dialog2, TOPIC_ROGLIMA, 2);
			break;
		case 2:
			g_topic_blutzinnen = 1;
			conv_unlock_topic(g_dialog2, TOPIC_BLUTZINNEN);
			break;
		case 3:
			conv_set_topic(g_dialog2, TOPIC_BERMAN, 2);
		default:
			break;
		}
	}
	return 0;
}

/* LOW15.TLK: Zimmer/Essen (inn_conv_callback), Thema 0x52, Antwort 4 Thema 0x2e (g_topic_siebenquell, g_low_ailian_state) (Rueckruf 8) */
signed short conv_cb_low15(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
		case 2:
			r = inn_conv_callback(a, b);
			break;
		case 3:
			conv_set_topic(g_dialog2, TOPIC_VISTELLA, 2);
			break;
		case 4:
			if (g_topic_siebenquell != -1) {
				g_topic_siebenquell = 1;
				conv_unlock_topic(g_dialog2, TOPIC_SIEBENQUELL);
				if (!g_low_ailian_state)
					g_low_ailian_state = 1;
			}
		default:
			break;
		}
	}
	return r;
}

/* LOW16.TLK: Zimmer/Essen, Themen 0x53/0x54 (Rueckruf 9) */
signed short conv_cb_low16(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
		case 2:
			r = inn_conv_callback(a, b);
			break;
		case 3:
			conv_set_topic(g_dialog2, TOPIC_ORKENTOD, 2);
			break;
		case 4:
			conv_set_topic(g_dialog2, TOPIC_VORMTANN, 2);
		default:
			break;
		}
	}
	return r;
}

/* LOW22.TLK: Schenke (tavern_talk_event2), Thema 0x2e, Thema 0x55 (Rueckruf 10) */
signed short conv_cb_low22(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			r = tavern_talk_event2(a, b);
			break;
		case 2:
			if (g_topic_siebenquell != -1) {
				g_topic_siebenquell = 1;
				conv_unlock_topic(g_dialog2, TOPIC_SIEBENQUELL);
				if (!g_low_ailian_state)
					g_low_ailian_state = 1;
			}
			break;
		case 3:
			conv_set_topic(g_dialog2, TOPIC_FARMION, 2);
		default:
			break;
		}
	}
	return r;
}

/* LOW24.TLK: Schenke, Themen 0x56/0x53 (Rueckruf 11) */
signed short conv_cb_low24(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			r = tavern_talk_event2(a, b);
			break;
		case 2:
			conv_set_topic(g_dialog2, TOPIC_DESDIRA, 2);
			break;
		case 3:
			conv_set_topic(g_dialog2, TOPIC_ORKENTOD, 2);
		default:
			break;
		}
	}
	return r;
}

/* LOW25.TLK: Schenke, Themen 0x57..0x59 (g_low25_topic59), Antwort 5 Thema 0x2c (g_topic_ingramosch); Abfrage 0x2e: g_low25_topic59 (Rueckruf 12) */
signed short conv_cb_low25(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			r = tavern_talk_event2(a, b);
			break;
		case 2:
			conv_set_topic(g_dialog2, TOPIC_SVELLTJEPALAST, 2);
			break;
		case 3:
			conv_set_topic(g_dialog2, TOPIC_THORESCHA, 2);
			break;
		case 4:
			conv_set_topic(g_dialog2, TOPIC_PAKT, 2);
			g_low25_topic59 = 1;
			break;
		case 5:
			g_topic_ingramosch = 1;
			conv_unlock_topic(g_dialog2, TOPIC_INGRAMOSCH);
			if (!g_low_ailian_state)
				g_low_ailian_state = 1;
		default:
			break;
		}
	} else if (a == 0 && b == 0x2e) {
		r = g_low25_topic59;
	}
	return r;
}

/* LOW26.TLK: Schenke, Thema 0x5a (Rueckruf 13) */
signed short conv_cb_low26(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			r = tavern_talk_event2(a, b);
			break;
		case 2:
			conv_set_topic(g_dialog2, TOPIC_VINSALTER, 2);
		default:
			break;
		}
	}
	return r;
}

/* LOW27.TLK: Schenke, Thema 0x5b (Rueckruf 14) */
signed short conv_cb_low27(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			r = tavern_talk_event2(a, b);
			break;
		case 2:
			conv_set_topic(g_dialog2, TOPIC_AKADEMIEN, 2);
		default:
			break;
		}
	}
	return r;
}

/* LOW31.TLK: Themen 0x4f/0x50 (Rueckruf 15) */
signed short conv_cb_low31(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			conv_set_topic(g_dialog2, TOPIC_TRAVIATEMPEL, 2);
			break;
		case 2:
			conv_set_topic(g_dialog2, TOPIC_ROGLIMA, 2);
		default:
			break;
		}
	}
	return r;
}

/* LOW32.TLK: Erzaehlungen XDS.LTX[0..8] (Zwerge, Sternenschweif, Tordol; conv_scene_lore), Thema 0x5c; Abfrage 1: erzaehlt (Rueckruf 16) */
signed short conv_cb_low32_lore(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		if (b >= 1 && b <= 3) {
			if (!g_lore_told) {
				conv_scene_lore(g_dialog2, b);
				r = -1;
			}
		} else if (b == 4) {
			if (!g_lore_told)
				conv_set_topic(g_dialog2, TOPIC_MEISTERSCHMIEDE, 2);
		}
	} else if (a == 0 && b == 1 && g_lore_told != 0) {
		r = 1;
	}
	return r;
}

/* LOW33.TLK: Themen 0x50/0x53 (Rueckruf 17) */
signed short conv_cb_low33(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			conv_set_topic(g_dialog2, TOPIC_ROGLIMA, 2);
			break;
		case 2:
			conv_set_topic(g_dialog2, TOPIC_ORKENTOD, 2);
		default:
			break;
		}
	}
	return r;
}

/* LOW34.TLK: Thema 0x5d einmal (g_low_kysira_ref); Abfrage 4: CH-Probe +4 (g_low34_ch_passed) (Rueckruf 18) */
signed short conv_cb_low34(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		if (b == 1 && !g_low_kysira_ref) {
			conv_set_topic(g_dialog2, TOPIC_ZASKIA, 2);
			g_low_kysira_ref = 1;
		}
	} else if (a == 0 && b == 4) {
		if (!g_low_kysira_ref)
			r = g_low34_ch_passed;
		else
			r = 1;
		if (!g_low34_ch_passed && test_attrib(get_first_active_hero(g_group), ATTRIB_CH, 4) > 0)
			g_low34_ch_passed = 1;
	}
	return r;
}

/* Rueckruf 19 (kein Gespraech; aus conv_cb_low37 bei Ort 8): Antwort 3 Thema 0x5e (Rueckruf 19) */
signed short conv_cb_19(signed short a, signed short b)
{
	if (a == 1) {
		switch (b) {
		case 3:
			conv_set_topic(g_dialog2, TOPIC_KYSIRA, 2);
		default:
			break;
		}
	}
	return 0;
}

/* Rueckruf 20 (kein Gespraech; aus conv_cb_low37 bei Ort 11): Antwort 3 Thema 0x5f (Rueckruf 20) */
signed short conv_cb_20(signed short a, signed short b)
{
	if (a == 1) {
		switch (b) {
		case 3:
			conv_set_topic(g_dialog2, TOPIC_SUMPFRANTZEN, 2);
		default:
			break;
		}
	}
	return 0;
}

/* LOW42.TLK: Thema 0x60, g_low_ailian_state = 1 (Rueckruf 21) */
signed short conv_cb_low42(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			conv_set_topic(g_dialog2, TOPIC_GEHEIMAUSGANG, 2);
		case 2:
			if (!g_low_ailian_state)
				g_low_ailian_state = 1;
		default:
			break;
		}
	}
	return r;
}

/* LOW43.TLK: Thema 0x5f (Rueckruf 22) */
signed short conv_cb_low43(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			conv_set_topic(g_dialog2, TOPIC_SUMPFRANTZEN, 2);
		default:
			break;
		}
	}
	return r;
}

/* MARKET*.TLK (Marktleute): Abfragen 0..2 zu 50 %, 7/8/10/11 je nach Stadt 1/2/5/6 (Rueckruf 27) */
signed short conv_cb_market(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 0) {
		switch (b) {
		case 0:
		case 1:
		case 2:
			if (random_schweif(100) <= 50)
				r = 1;
			break;
		case 7:
			if (g_group->town == 1)
				r = 1;
			break;
		case 8:
			if (g_group->town == 2)
				r = 1;
			break;
		case 10:
			if (g_group->town == 5)
				r = 1;
			break;
		case 11:
			if (g_group->town == 6)
				r = 1;
		default:
			break;
		}
	}
	return r;
}

/* TJOL09.TLK: Bestellen in der Schenke bzw. Bordell (conv_scene_brothel) (Rueckruf 28) */
signed short conv_cb_tjol09_brothel(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			if (g_tavern != 0)
				tavern_order(g_tavern);
			break;
		case 2:
		case 3:
			conv_scene_brothel(g_dialog2, b - 2);
		default:
			break;
		}
		r = -1;
	}
	return r;
}

/* LOW37..LOW41.TLK (Laeden): je Ort 8/10/11 Themen 0x5e/0x2d/0x5f; Abfrage 5: Kraemer ohne Nahrung (Rueckruf 29) */
signed short conv_cb_low37(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		if (g_loc_id == 8)
			r = conv_cb_19(a, b);
		if (g_loc_id == 10) {
			g_topic_sternenschweif = 1;
			conv_unlock_topic(g_dialog2, TOPIC_STERNENSCHWEIF);
		} else if (g_loc_id == 11) {
			r = conv_cb_20(a, b);
		}
	} else if (a == 0 && b == 5) {
		if (g_shop_sells_food[g_loc_id])
			r = 0;
		else
			r = 1;
	}
	return r;
}

/* FRP183B.TLK: Mahlzeit (conv_scene_meal) (Rueckruf 30) */
signed short conv_cb_frp183b_meal(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1 && b == 1)
		r = conv_scene_meal(g_dialog2);
	return r;
}

/* FST000.TLK: Wutrede Iwain Basiliskentoeters (conv_scene_iwain), Gespraech beenden (Rueckruf 31) */
signed short conv_cb_fst000_iwain(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1 && b == 1) {
		conv_scene_iwain(g_dialog2);
		conv_finish(g_dialog2);
		r = -1;
	}
	return r;
}

/* FST167.TLK: Bitte um Musik (conv_scene_music) (Rueckruf 32) */
signed short conv_cb_fst167_music(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1 && b == 1)
		r = conv_scene_music(g_dialog2);
	return r;
}

/* FRP22B.TLK: Thema 0x0d (g_topic_ansvell) (Rueckruf 36) */
signed short conv_cb_frp22b(signed short a, signed short b)
{
	if (a == 1 && b == 1) {
		g_topic_ansvell = 1;
		conv_unlock_topic(g_dialog2, TOPIC_ANSVELL);
	}
	return 0;
}

/* FRP23B/26B/32D.TLK: Thema 0x0c (g_topic_finsterkoppen) (Rueckruf 38) */
signed short conv_cb_frp23b(signed short a, signed short b)
{
	if (a == 1 && b == 1) {
		g_topic_finsterkoppen = 1;
		conv_unlock_topic(g_dialog2, TOPIC_FINSTERKOPPEN);
	}
	return 0;
}

/* FRP25B.TLK: Zimmer/Essen der Herberge, Thema 0x0c (Rueckruf 33) */
signed short conv_cb_frp25b(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		if (b == 1 && g_inn != 0) {
			r = -1;
			inn_room(g_inn);
		}
		if (b == 2 && g_inn != 0) {
			r = -1;
			inn_eat(g_inn);
		}
		if (b == 3) {
			g_topic_finsterkoppen = 1;
			conv_unlock_topic(g_dialog2, TOPIC_FINSTERKOPPEN);
		}
	}
	return r;
}

/* Gespraech mit Rueckruf 35 (Skript unbekannt): Thema 0x0e (g_topic_salamandersteine), Antwort 2 g_low_ailian_state */
signed short conv_cb_unused35(signed short a, signed short b)
{
	if (a == 1) {
		if (b == 1) {
			g_topic_salamandersteine = 1;
			conv_unlock_topic(g_dialog2, TOPIC_SALAMANDERSTEINE);
		} else if (b == 2) {
			if (!g_low_ailian_state)
				g_low_ailian_state = 1;
		}
	}
	return 0;
}

/* FRP32B.TLK: Zimmer/Essen, Thema 0x0e (g_topic_salamandersteine) (Rueckruf 34) */
signed short conv_cb_frp32b(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		if (b == 1 && g_inn != 0) {
			r = -1;
			inn_room(g_inn);
		}
		if (b == 2 && g_inn != 0) {
			r = -1;
			inn_eat(g_inn);
		}
		if (b == 3) {
			g_topic_salamandersteine = 1;
			conv_unlock_topic(g_dialog2, TOPIC_SALAMANDERSTEINE);
		}
	}
	return r;
}

/* LOW35/FRP21B/FRP32C.TLK: wie conv_cb_frp25b (Rueckruf 35) */
signed short conv_cb_low35(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		if (b == 1 && g_inn != 0) {
			r = -1;
			inn_room(g_inn);
		}
		if (b == 2 && g_inn != 0) {
			r = -1;
			inn_eat(g_inn);
		}
		if (b == 3) {
			g_topic_finsterkoppen = 1;
			conv_unlock_topic(g_dialog2, TOPIC_FINSTERKOPPEN);
		}
	}
	return r;
}

/* LOW36.TLK: Thema 0x2d (g_topic_sternenschweif) (Rueckruf 37) */
signed short conv_cb_low36(signed short a, signed short b)
{
	if (a == 1 && b == 1) {
		g_topic_sternenschweif = 1;
		conv_unlock_topic(g_dialog2, TOPIC_STERNENSCHWEIF);
	}
	return 0;
}

/* aus inn_conv_callback bei Herberge 5: Thema 0x2e (g_topic_siebenquell, g_low_ailian_state) (Rueckruf -) */
signed short conv_cb_inn5(signed short a, signed short b)
{
	if (a == 1 && b == 1 && g_topic_siebenquell != -1) {
		g_topic_siebenquell = 1;
		conv_unlock_topic(g_dialog2, TOPIC_SIEBENQUELL);
		if (!g_low_ailian_state)
			g_low_ailian_state = 1;
	}
	return 0;
}

/* LOW62.TLK: Dragan - Auftraege (conv_scene_dragan_task), Tipps ins Tagebuch (Brin, Raisha, Jandora, Falkert) (Rueckruf 39) */
signed short conv_cb_low62_dragan(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			r = conv_scene_dragan_task(g_dialog2);
			break;
		case 2:
			g_dragan_tip_brin = 1;
			g_low_dragan_news = 0;
			conv_scene_note(g_dialog2, 0);
			break;
		case 3:
		case 4:
		case 5:
			if (b == 3)
				g_dragan_tip_raisha |= 1;
			else if (b == 4)
				g_dragan_tip_jandora = 1;
			else if (b == 5)
				g_low_falkert_known = 1;
			conv_scene_note(g_dialog2, b - 2);
			g_low_dragan_news = 0;
			break;
		case 6:
			g_low_dragan_news = 0;
			g_topic_siebenquell = 1;
			conv_unlock_topic(g_dialog2, TOPIC_SIEBENQUELL);
			if (g_low_ailian_state != -1 && g_low_ailian_state < 3) {
				if (g_low_stone_delivered == 1)
					g_low_ailian_state = 3;
				else
					g_low_ailian_state = 1;
			}
			break;
		}
	} else if (a == 0) {
		if (g_dragan_task == 3) {
			g_dragan_task = 4;
			g_low_dragan_news = 1;
		}
		if (b == 4 || b == 5 || b == 6 || b == 0x2b || b == 0x17) {
			if (g_low_dragan_news != 0)
				r = 1;
		} else if (b == 0x30) {
			if (!g_topic_gavron)
				r = 2;
			else if (g_low_dragan_news != 0)
				r = 1;
		} else if (b == 0x2f) {
			if (g_topic_gavron != 0)
				r = 2;
		}
	}
	return r;
}

/* LOW63.TLK: Schuppen fuer 5 Silberlinge je Kopf (conv_scene_shed) (Rueckruf 40) */
signed short conv_cb_low63_shed(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1 && b == 1) {
		conv_scene_shed(g_dialog2);
		r = -1;
	}
	return r;
}

/* LOW65.TLK: Unterkunft auf Empfehlung Dragans (conv_scene_no_room); Abfrage 4: Tipp g_dragan_tip_brin (Rueckruf 41) */
signed short conv_cb_low65(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		switch (b) {
		case 1:
			conv_scene_no_room(g_dialog2);
			break;
		case 2:
			g_dragan_tip_raisha |= 2;
			break;
		default:
			break;
		}
	} else if (a == 0 && b == 4 && g_dragan_tip_brin != 0) {
		r = 1;
	}
	return r;
}

/* GASH01..03/09..13.TLK (Gashok): Thema 0x31, 0x63 Gespraech beenden (Rueckruf 42) */
signed short conv_cb_gash(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		if (b == 0x60) {
			conv_unlock_topic(g_dialog2, TOPIC_ARTHERION);
			if (!g_artherion_hut_known)
				g_artherion_heard = g_topic_artherion = 1;
		} else if (b == 0x63) {
			conv_finish(g_dialog2);
		}
	}
	return r;
}

/* TAVGAS01..03.TLK (Schenken Gashok): beim dritten Mal Sites::talk, sonst Zufallsantwort (Rueckruf 43) */
signed short conv_cb_tavgas(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		if (b == 0x63) {
			conv_finish(g_dialog2);
		} else if (b == 0x62) {
			if (g_tavgas_visits != -1)
				g_tavgas_visits++;
			if (g_tavgas_visits == 3) {
				Sites o;

				o.talk();
				r = g_tavgas_visits = -1;
			} else {
				conv_scene_no_idea(g_dialog2);
				conv_finish(g_dialog2);
			}
		}
	} else if (a == 0 && b == 0x38 && g_tavern != 0 && g_tavern->rounds >= 2) {
		r = 1;
	}
	return r;
}

/* GASINFO2.TLK: Thema 0x38 (g_sites_challenges), g_artherion_hut_known, Thema 0x31 (Rueckruf 44) */
signed short conv_cb_gasinfo2(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 1) {
		if (b == 1) {
			conv_unlock_topic(g_dialog2, TOPIC_KUSLIK);
			if (!g_sites_challenges)
				g_sites_challenges = 1;
		} else if (b == 2) {
			g_artherion_hut_known = 1;
		} else if (b == 0x60) {
			conv_unlock_topic(g_dialog2, TOPIC_ARTHERION);
			if (!g_artherion_hut_known)
				g_artherion_heard = g_topic_artherion = 1;
		}
	}
	return r;
}

/* TIEF12B.TLK: Gespraech am Hafen (port_talk), Gespraech beenden (Rueckruf 45) */
signed short conv_cb_tief12b(signed short a, signed short b)
{
	signed short r = 0;
	void far *p;

	if (a == 1 && b == 1) {
		p = PortTxt_ctor((void far *)0);
		port_talk((TOWN_PTR)p);
		PortTxt_dtor(p, 3);
		conv_finish(g_dialog2);
		r = -1;
	}
	return r;
}

/* SWAMP2.TLK: Abfrage 6/0x11 bei g_eolan_state == 2 (Rueckruf 46) */
signed short conv_cb_swamp2(signed short a, signed short b)
{
	signed short r = 0;

	if (a == 0 && (b == 6 || b == 0x11) && g_eolan_state == 2)
		r = 1;
	return r;
}
