/* s_data.h - globale Daten im Datensegment (Teil von schweif.h)
 * ERZEUGT von names/tools/namen.py aus names/daten/manuell_data.tsv. Adressen als Kommentar; cobjfix
 * ordnet die Namen ueber die Adresse zu. Objekte (Dialog, Schrift, Klang ...) als char-Felder,
 * solange ihre Klasse nicht in s_class.h steht. */
#ifndef S_DATA_H
#define S_DATA_H

extern unsigned short far g_finale_ticks;               /* ds:0000 Takte des Spielende-Skripts (eigenes Far-Segment; O036 final */
extern signed short g_dng16_seq_pics[];                 /* ds:0094 Bildliste der Golem-Zwischensequenzen fuer load_pic_list (O */
extern unsigned short g_dng16_seq_pics_state[];         /* ds:00a2 Zustandsbits zu g_dng16_seq_pics (O002) */
extern signed short g_dng16_seq_golem_state;            /* ds:00a4 Sprachstufe der Golem-Sequenz 0x0f (O002 dng16_seq_golem_cb */
extern signed short g_dng16_seq_forge_state;            /* ds:00a6 Sprachstufe der Schmiede-Sequenz 0x10 (O002 dng16_seq_forge */
extern struct score7 g_dng16_heat_marks_init;           /* ds:00a8 7 Nullworte: Vorlage der Heldenmarken in dng16_forge_heat ( */
extern char g_fmt_sp_s_sp[];                            /* ds:00b6 " %s " (O002 dng16_forge_heat: "und" vor dem letzten Namen) */
extern char g_str_dsa2_tmp_gam[];                       /* ds:00bc "DSA2_TMP.GAM" (O003 cache_fatal: Name in der Meldung) */
extern struct cd_volctl g_cd_volctl;                    /* ds:00ce Vorlage des Lautstaerke-Steuerblocks (IOCTL 0x0c, 9 Bytes; O */
extern char g_cd_label1[];                              /* ds:00d7 Datentraegername der CD 1 (O004 CD::read_disc) */
extern struct group_ptrs6 g_group_ptrs_null;            /* ds:00e3 sechs Nullzeiger als Vorlage (O005 jail_tick; ds:00f0 = _s_0 */
extern char g_cd_label2[];                              /* ds:00e3 Datentraegername der CD 2 (O004 CD::read_disc; dasselbe Publ */
extern struct str6 g_select_skill_lvlup;                /* ds:0108 Format "%s %d" (BE SELECT_SKILL_LVLUP) */
extern struct skill_list9 g_select_skill_defaults;      /* ds:010e Talentauswahl: drei Heiltalente, Rest -1 (BE) */
extern char g_str_empty_0117[];                         /* ds:0117 "" (Einzahl in skill_heal_wounds) */
extern char g_str_empty_0118[];                         /* ds:0118 "" (Einzahl in skill_heal_wounds) */
extern signed short g_cl_blackmage[];                   /* ds:011a Dateiliste der Schwarzmagier-Szene (Ende -1; O007 load_pic_l */
extern signed short g_cl_blackmage_speech[];            /* ds:0120 Dateiliste der Schwarzmagier-Szene, Sprachausgabe (O007) */
extern unsigned char g_cl_blackmage_st[];               /* ds:0134 Zustaende zu den Dateilisten der Schwarzmagier-Szene (O007) */
extern signed short g_cl_phex_finale[];                 /* ds:0138 Dateiliste des Phex-Finales (O007) */
extern signed short g_cl_phex_finale_speech[];          /* ds:013e Dateiliste des Phex-Finales, Sprachausgabe (0xc404..0xc412;  */
extern unsigned char g_cl_phex_finale_st[];             /* ds:015e Zustaende zu den Dateilisten des Phex-Finales (O007) */
extern struct str2 g_scene_mark_on;                     /* ds:0162 Marker der gewaehlten Zeile (Zeichen 0xf1; O007 scene_choice */
extern struct str2 g_scene_mark_off;                    /* ds:0164 Marker der nicht gewaehlten Zeile (Zeichen 0xf0; O007 scene_ */
extern struct str7 g_scene_fmt;                         /* ds:0166 "%s%s%s" (O007 scene_choice) */
extern signed short g_inv_status_slots[];               /* ds:0172 Plaetze, fuer die inv_draw_slot die Statuszeile zeichnet (is */
extern struct point g_inv_slot_xy[33];                  /* ds:017c Bildschirmkoordinaten der 33 Inventarplaetze (Heldenbogen) */
extern char g_inv_mouse_knapsack[];                     /* ds:0200 Mausbereichsliste Rucksack (get_mouse_action, O008) */
extern struct mouse_area g_inv_areas_knapsack[18];      /* ds:0250 Klickfelder der Rucksackplaetze 15..30 + Restfeld + Endmarke */
extern char g_inv_mouse_all[];                          /* ds:0304 Mausbereichsliste aller Plaetze (get_mouse_action, O008) */
extern struct mouse_area g_inv_areas_all[35];           /* ds:0322 Klickfelder aller 33 Plaetze + Restfeld + Endmarke (Lage zur */
extern char g_areas_sheet[];                            /* ds:0480 Mausbereiche Heldenbogen Seiten 3-5 (O095) */
extern char g_cl_inventory[];                           /* ds:04a8 Bilderliste des Heldenbogens fuer load_pic_list (Ende -1; O */
extern char g_cl_inventory_st[];                        /* ds:04ee Zustaende zur Bilderliste g_cl_inventory (O008) */
extern char g_fmt_inv_status[];                         /* ds:04f8 Format der Statuszeile im Inventar (Name, Zusatz) */
extern char g_str_inv_usable[];                         /* ds:04ff Zusatz in der Statuszeile, wenn benutzbar (sonst txt[0]) */
extern signed short g_inv_warn_enabled;                 /* ds:0500 Warnzeichen im Heldenbogen erlaubt (inv_warn_signal nur bei */
extern signed long g_warn_blink_time;                   /* ds:0502 inv_warn_signal: Zeit des letzten Blinkens (statischer Local */
extern signed char g_warn_blink_init;                   /* ds:0506 inv_warn_signal: Waechter der statischen Zeitmarke g_warn_bl */
extern signed long g_warn_snd_time;                     /* ds:0507 inv_warn_signal: Zeit des letzten Warnklangs (statisch) */
extern signed char g_warn_snd_init;                     /* ds:050b inv_warn_signal: Waechter von g_warn_snd_time */
extern signed char g_warn_blink_on;                     /* ds:050c inv_warn_signal: Blinkzeichen gerade an */
extern struct str9 g_str_emm;                           /* ds:050e "EMMXXXX0" Kennung des EMS-Treibers (O010 ems_check) */
extern char g_err_swap_write[];                         /* ds:0517 "Error moving mem ..." (O010 swap_write, XMS) */
extern char g_err_swap_read[];                          /* ds:0540 "Error moving mem ..." (O010 swap_read, XMS) */
extern char g_fmt_tex_debug[];                          /* ds:056b "Texture %d, XRes ..." (O010 tex_debug_view) */
extern char g_str_dbg_ems[];                            /* ds:05a2 Format EMS-Menge (O011 dev_info_screen) */
extern char g_str_dbg_files[];                          /* ds:05e2 Format Zahl der Dateihandles (O011 dev_info_screen) */
extern char g_str_fmt_c[];                              /* ds:061f "%c" (O011) */
extern char g_str_fmt_ld[];                             /* ds:0622 "%ld" (O011 Pruefsumme) */
extern char g_str_depot_dat[];                          /* ds:0626 "DEPOT.DAT" (O012 Store_ctor: Name 0 der Arbeitsdatei) */
extern signed short g_cutscene_pics[];                  /* ds:0630 Bildliste der Zwischensequenz in talk_reward (0xc413, 0xc414 */
extern struct talk_tpl g_talk_tpl;                      /* ds:0636 Vorlagen: leere Bildliste (load_pic_list), 6 Namen und 6 Hel */
extern signed short g_cutscene_sub1;                    /* ds:06b0 Untertitel 1 der Zwischensequenz schon gezeigt */
extern signed short g_cutscene_sub2;                    /* ds:06b2 Untertitel 2 schon gezeigt */
extern unsigned char g_dng_occl_05[];                   /* ds:06b4 Dungeon 5: Verdeckung je Feldtyp (O014 dng_make_events -> g_ */
extern unsigned char g_dng_occl_01[];                   /* ds:06cc Dungeon 1: Verdeckung je Feldtyp (kein eigenes Public) */
extern unsigned char g_dng_occl_02[];                   /* ds:06e9 Dungeon 2: Verdeckung je Feldtyp (kein eigenes Public) */
extern unsigned char g_dng_occl_03[];                   /* ds:0701 Dungeon 3: Verdeckung je Feldtyp (kein eigenes Public) */
extern unsigned char g_dng_occl_09[];                   /* ds:0719 Dungeon 9: Verdeckung je Feldtyp (kein eigenes Public) */
extern unsigned char g_dng_occl_10[];                   /* ds:072e Dungeon 10: Verdeckung je Feldtyp (kein eigenes Public) */
extern unsigned char g_dng_occl_11[];                   /* ds:0746 Dungeon 11: Verdeckung je Feldtyp (kein eigenes Public) */
extern unsigned char g_dng_occl_12[];                   /* ds:075e Dungeon 12: Verdeckung je Feldtyp (kein eigenes Public) */
extern unsigned char g_dng_occl_13[];                   /* ds:0776 Dungeon 13: Verdeckung je Feldtyp (kein eigenes Public) */
extern unsigned char g_dng_occl_14[];                   /* ds:078e Dungeon 14/15: Verdeckung je Feldtyp (kein eigenes Public) */
extern unsigned char g_dng_occl_16[];                   /* ds:07a6 Dungeon 16: Verdeckung je Feldtyp (kein eigenes Public) */
extern struct view_obj g_dng_objs_01[];                 /* ds:07be Objektliste Dungeon 1 (struct view_obj, Ende idx -1; g_dng_v */
extern struct view_obj g_dng_objs_02[];                 /* ds:0815 Objektliste Dungeon 2 (struct view_obj, Ende idx -1; g_dng_v */
extern struct view_obj g_dng_objs_03[];                 /* ds:0854 Objektliste Dungeon 3 (struct view_obj, Ende idx -1; g_dng_v */
extern struct view_obj g_dng_objs_04[];                 /* ds:0899 Objektliste Dungeon 4 (struct view_obj, Ende idx -1; g_dng_v */
extern struct view_obj g_dng_objs_09[];                 /* ds:08ea Objektliste Dungeon 9 (struct view_obj, Ende idx -1; g_dng_v */
extern struct view_obj g_dng_objs_05[];                 /* ds:092c Objektliste Dungeon 5 (struct view_obj, Ende idx -1; g_dng_v */
extern struct view_obj g_dng_objs_06[];                 /* ds:0971 Objektliste Dungeon 6 (struct view_obj, Ende idx -1; g_dng_v */
extern struct view_obj g_dng_objs_07[];                 /* ds:09cb Objektliste Dungeon 7 (struct view_obj, Ende idx -1; g_dng_v */
extern struct view_obj g_dng_objs_08[];                 /* ds:0a13 Objektliste Dungeon 8 (struct view_obj, Ende idx -1; g_dng_v */
extern struct view_obj g_dng_objs_10[];                 /* ds:0a5e Objektliste Dungeon 10 (struct view_obj, Ende idx -1; g_dng_ */
extern struct view_obj g_dng_objs_11[];                 /* ds:0a97 Objektliste Dungeon 11 (struct view_obj, Ende idx -1; g_dng_ */
extern struct view_obj g_dng_objs_12[];                 /* ds:0ad3 Objektliste Dungeon 12 (struct view_obj, Ende idx -1; g_dng_ */
extern struct view_obj g_dng_objs_13[];                 /* ds:0b21 Objektliste Dungeon 13 (struct view_obj, Ende idx -1; g_dng_ */
extern struct view_obj g_dng_objs_14[];                 /* ds:0b63 Objektliste Dungeon 14/15 (struct view_obj, Ende idx -1; g_d */
extern struct view_obj g_dng_objs_16[];                 /* ds:0b84 Objektliste Dungeon 16 (struct view_obj, Ende idx -1; g_dng_ */
extern struct view_area g_dng_areas[];                  /* ds:0bbd = g_dng_view_areas - 1 (MAP-Public des 1-basierten Zugriffs, */
extern struct view_area g_dng_view_areas[16];           /* ds:0bcf Beschreibung je Dungeon 1..16 (Index group.dungeon - 1; O014 */
extern signed short g_dng_pics[];                       /* ds:0cef Bilderliste aller Dungeons fuer den CD-Cache (load_pic_list, */
extern char g_dng_pics_st[];                            /* ds:0d11 Zustaende zu g_dng_pics */
extern signed short g_dng_pics_01[];                    /* ds:0d15 Bilderliste Dungeon 1 (load_pic_list; O014) */
extern char g_dng_pics_01_st[];                         /* ds:0d2b Zustaende zu g_dng_pics_01 */
extern signed short g_dng_pics_02[];                    /* ds:0d2f Bilderliste Dungeon 2 */
extern char g_dng_pics_02_st[];                         /* ds:0d47 Zustaende zu g_dng_pics_02 */
extern signed short g_dng_pics_03[];                    /* ds:0d4b Bilderliste Dungeon 3 */
extern char g_dng_pics_03_st[];                         /* ds:0d6b Zustaende zu g_dng_pics_03 */
extern signed short g_dng_pics_04[];                    /* ds:0d6f Bilderliste Dungeon 4 */
extern char g_dng_pics_04_st[];                         /* ds:0d83 Zustaende zu g_dng_pics_04 */
extern signed short g_dng_pics_05[];                    /* ds:0d87 Bilderliste Dungeon 5 */
extern char g_dng_pics_05_st[];                         /* ds:0dad Zustaende zu g_dng_pics_05 */
extern signed short g_dng_pics_06[];                    /* ds:0db3 Bilderliste Dungeon 6 */
extern char g_dng_pics_06_st[];                         /* ds:0dcd Zustaende zu g_dng_pics_06 */
extern signed short g_dng_pics_07[];                    /* ds:0dd1 Bilderliste Dungeon 7 */
extern char g_dng_pics_07_st[];                         /* ds:0de7 Zustaende zu g_dng_pics_07 */
extern signed short g_dng_pics_08[];                    /* ds:0deb Bilderliste Dungeon 8 */
extern char g_dng_pics_08_st[];                         /* ds:0dfd Zustaende zu g_dng_pics_08 */
extern signed short g_dng_pics_09[];                    /* ds:0dff Bilderliste Dungeon 9 */
extern char g_dng_pics_09_st[];                         /* ds:0e11 Zustaende zu g_dng_pics_09 */
extern signed short g_dng_pics_10[];                    /* ds:0e13 Bilderliste Dungeon 10 */
extern char g_dng_pics_10_st[];                         /* ds:0e33 Zustaende zu g_dng_pics_10 */
extern signed short g_dng_pics_11[];                    /* ds:0e37 Bilderliste Dungeon 11 */
extern char g_dng_pics_11_st[];                         /* ds:0e4b Zustaende zu g_dng_pics_11 */
extern signed short g_dng_pics_12[];                    /* ds:0e4f Bilderliste Dungeon 12 */
extern char g_dng_pics_12_st[];                         /* ds:0e61 Zustaende zu g_dng_pics_12 */
extern signed short g_dng_pics_13[];                    /* ds:0e63 Bilderliste Dungeon 13 */
extern char g_dng_pics_13_st[];                         /* ds:0e75 Zustaende zu g_dng_pics_13 */
extern signed short g_dng_pics_14[];                    /* ds:0e77 Bilderliste Dungeon 14/15 */
extern char g_dng_pics_14_st[];                         /* ds:0e89 Zustaende zu g_dng_pics_14 */
extern signed short g_dng_pics_16[];                    /* ds:0e8b Bilderliste Dungeon 16 */
extern char g_dng_pics_16_st[];                         /* ds:0eab Zustaende zu g_dng_pics_16 */
extern signed char g_dng_nogroup_01[];                  /* ds:0eaf Dungeon 1/10/11/13: Feldtypen ohne Anzeige "andere Gruppe" ( */
extern signed char g_dng_nogroup_02[];                  /* ds:0eb2 Dungeon 2: Feldtypen ohne Anzeige "andere Gruppe" */
extern signed char g_dng_nogroup_03[];                  /* ds:0eb5 Dungeon 3: Feldtypen ohne Anzeige "andere Gruppe" */
extern signed char g_dng_nogroup_05[];                  /* ds:0eb9 Dungeon 5: Feldtypen ohne Anzeige "andere Gruppe" */
extern signed char g_dng_nogroup_06[];                  /* ds:0ebd Dungeon 6: Feldtypen ohne Anzeige "andere Gruppe" */
extern signed char g_dng_nogroup_07[];                  /* ds:0ec1 Dungeon 7: Feldtypen ohne Anzeige "andere Gruppe" */
extern signed char g_dng_nogroup_08[];                  /* ds:0ec5 Dungeon 8: Feldtypen ohne Anzeige "andere Gruppe" */
extern signed char g_dng_nogroup_12[];                  /* ds:0ecb Dungeon 12: Feldtypen ohne Anzeige "andere Gruppe" */
extern signed char g_dng_nogroup_16[];                  /* ds:0ece Dungeon 16: Feldtypen ohne Anzeige "andere Gruppe" */
extern signed char g_dng_camp_04[];                     /* ds:0ed3 Dungeon 4: Feldtypen, auf denen Rasten erlaubt ist (O014 dng */
extern signed char g_dng_nocamp_13[];                   /* ds:0ed7 Dungeon 13: Feldtypen ohne Rasten */
extern signed char g_dng_nocamp_15[];                   /* ds:0eda Dungeon 15: Feldtypen ohne Rasten */
extern struct point g_zoom_slot_xy[];                   /* ds:0ede Heldenbogen: Startpunkt der Anlege-Animation je Inventarplat */
extern signed short g_zoom_skip_items[];                /* ds:0f1a Gegenstaende ohne Anlege-Animation (0x31, Ende -1; O015) */
extern struct srgb g_zoom_frame_color;                  /* ds:0f1e Rahmenfarbe 0x21 der Anlege-Animation (O015) */
extern struct anim_delays g_anim_delays;                /* ds:0f22 Takte je Einzelbild der Kampfanimation je Geschwindigkeit (O */
extern signed short g_fight_use_ems;                    /* ds:0f34 Kampf-Hintergrund im EMS statt XMS (EMS_copy statt xms_move_ */
extern signed short g_file3_marked;                     /* ds:0f36 g_file_3 im Kampf mit arc_mark markiert */
extern signed short g_fight_pic_keys[];                 /* ds:0f38 Bilderliste des Kampfes (Schluessel Typ|Nr., Ende -1; load_p */
extern char g_fight_pic_states[];                       /* ds:0f94 Zustaende zu g_fight_pic_keys */
extern struct str6 g_fight_map_file;                    /* ds:0fa0 Datei fuer die gesicherte 3D-Zellentabelle (start_fight) */
extern signed short g_finster_anger_stage;              /* ds:0fa6 Zorn der Zwerge: 1 = Warnung (Zwerge versammeln sich) gezei */
extern struct sfight_frames g_sfight_frames;            /* ds:0fa8 Bildfolgen der Hauptsequenz (kein eigenes Public: ds:0fa6 +  */
extern struct frame_events8 g_sfight_intro_events;      /* ds:0fb6 Ereignisse der Einleitung (Element 7 = ds:0fc4 eigenes Publi */
extern struct frame_events6 g_sfight_end_events;        /* ds:0fc6 Ereignisse der Schlusssequenz (kein eigenes Public: ds:0fc4  */
extern struct point g_sfight_targets_b[];               /* ds:0fd4 Sonderkampf: Zielfelder (Richtung 1) */
extern struct point g_sfight_targets_c[];               /* ds:0fe0 Sonderkampf: Zielfelder (Richtung 2) */
extern struct point g_sfight_targets_d[];               /* ds:0fec Sonderkampf: Zielfelder (Richtung 0) */
extern struct point g_sfight_targets_a[];               /* ds:0ff8 Sonderkampf: Zielfelder (Richtung 3, Ende x == -1) */
extern struct point g_sfight_targets_far[];             /* ds:1050 Sonderkampf: Zielfelder fuer den Fernangriff */
extern signed char g_sfight2_nr_right;                  /* ds:118c Sonderkampf 0x495: Zahl der richtigen Woerter (O021) */
extern signed char g_sfight2_nr_wrong;                  /* ds:118d Sonderkampf 0x495: Zahl der falschen Woerter (O021) */
extern struct sfight_starts g_sfight2_starts;           /* ds:118e Sonderkampf 0x495: Startplaetze der Helden (kein eigenes Pub */
extern signed short g_sfight_pics[];                    /* ds:11aa Bilderliste des Sonderkampfs 0x493 (load_pic_list) */
extern char g_sfight_pic_states[];                      /* ds:11e4 Zustaende zu g_sfight_pics */
extern signed short g_sfight2_pics[];                   /* ds:11ec Bilderliste des Sonderkampfs 0x495 */
extern char g_sfight2_pic_states[];                     /* ds:121c Zustaende zu g_sfight2_pics */
extern unsigned char g_fli_palette[0x300];              /* ds:1222 Palettenpuffer der FLI-Wiedergabe (256 x RGB, Anfangspalette */
extern struct fli_screen g_fli_screen;                  /* ds:1522 Ausgabeziel der FLI-Animationen (320 x 200, Bild +0x0a, Pale */
extern unsigned char far *g_fli_vga;                    /* ds:152c = g_fli_screen.scr (eigenes Public): A000:0000 (O025, O028) */
extern struct srgb far *g_fli_pal;                      /* ds:1530 = g_fli_screen.pal (eigenes Public): Zeiger auf den Palette */
extern signed short g_type_le[];                        /* ds:1542 LE je Heldentyp (O167) */
extern signed short g_type_ae[];                        /* ds:155c AE je Heldentyp (O167) */
extern struct height_range g_type_height[];             /* ds:1576 Groesse je Heldentyp (O167) */
extern unsigned char g_type_weight[];                   /* ds:1590 Gewicht je Typ: (Groesse - Wert) * 40 Unzen (O167) */
extern struct mouse_area g_chargen_areas_main[8];       /* ds:159d Mausbereiche der Heldenerschaffung, Seite 0 Grundwerte (O026 */
extern struct mouse_area g_chargen_areas_skills[4];     /* ds:15ed Mausbereiche der Heldenerschaffung, Seiten 1..4 Talente/AT-P */
extern struct mouse_area g_chargen_areas_spells[4];     /* ds:1615 Mausbereiche der Heldenerschaffung, Seiten 5..10 Zauber (O02 */
extern void far *g_chargen_mouse_areas[11];             /* ds:163d Mausbereiche je Seite der Heldenerschaffung (O026) */
extern struct rect g_portrait_rects[9];                 /* ds:1669 Portraetfelder der Auswahl (9 x struct rect; O026/O093 kopi */
extern signed char g_portrait_index_m[];                /* ds:16b1 je Heldentyp erstes Portraet maennlich (bis _f[typ] - 1) */
extern signed char g_portrait_index_m_next[];           /* ds:16b2 = g_portrait_index_m + 1 (eigenes Public; O093) */
extern signed char g_portrait_index_f[];                /* ds:16bd je Heldentyp erstes Portraet weiblich (bis _m_next[typ] - 1) */
extern struct money_range g_type_money_01[];            /* ds:16c8 Startgeld Typ JUGGLER (struct money_range bis Grenze 20; O02 */
extern struct money_range g_type_money_02[];            /* ds:16da Startgeld Typ HUNTER (struct money_range bis Grenze 20; O026 */
extern struct money_range g_type_money_03[];            /* ds:16e6 Startgeld Typ WARRIOR (struct money_range bis Grenze 20; O02 */
extern struct money_range g_type_money_04[];            /* ds:170a Startgeld Typ ESTRAY (struct money_range bis Grenze 20; O026 */
extern struct money_range g_type_money_05[];            /* ds:1722 Startgeld Typ THORWALIAN (struct money_range bis Grenze 20;  */
extern struct money_range g_type_money_06[];            /* ds:173a Startgeld Typ DWARF (struct money_range bis Grenze 20; O026, */
extern struct money_range g_type_money_07[];            /* ds:1752 Startgeld Typ WITCH (struct money_range bis Grenze 20; O026, */
extern struct money_range g_type_money_08[];            /* ds:1758 Startgeld Typ DRUID (struct money_range bis Grenze 20; O026, */
extern struct money_range g_type_money_09[];            /* ds:177c Startgeld Typ MAGE (struct money_range bis Grenze 20; O026,  */
extern struct money_range g_type_money_10[];            /* ds:179a Startgeld Typ GREEN_ELF (struct money_range bis Grenze 20; O */
extern struct money_range g_type_money_11[];            /* ds:17b8 Startgeld Typ ICE_ELF (struct money_range bis Grenze 20; O02 */
extern struct money_range g_type_money_12[];            /* ds:17c4 Startgeld Typ SYLVAN_ELF (struct money_range bis Grenze 20;  */
extern struct money_range far *g_type_money[];          /* ds:17d2 = g_type_money_ranges - 1 (MAP-Public des 1-basierten Zugrif */
extern struct money_range far *g_type_money_ranges[12]; /* ds:17d6 Startgeld-Listen je Heldentyp 1..12 (Index type - 1; O167) */
extern signed short g_chargen_pics[];                   /* ds:1806 Bildliste der Heldenerschaffung fuer load_pic_list (O026, E */
extern unsigned short g_chargen_pics_state[];           /* ds:1832 Zustandsbits zu g_chargen_pics (load_pic_list, O026) */
extern struct str16 g_chargen_name_init;                /* ds:1838 16 Nullbytes: Vorbelegung des Namenspuffers (O026; kein eig */
extern signed char g_chargen_type_shown;                /* ds:1848 Typbild steht auf dem Hintergrund (O026) */
extern signed short g_cl_opening[];                     /* ds:184a Dateiliste des Vorspanns (FLI 0xa002..0xa00b, Ende -1; O027  */
extern unsigned char g_cl_opening_st[];                 /* ds:1864 Zustaende zur Dateiliste des Vorspanns (O027) */
extern struct srgb g_opening_white;                     /* ds:1868 Farbe 0x3f/0x3f/0x3f fuer die Blenden der FLI-Fassung (O028) */
extern signed short g_nr_raw_files;                     /* ds:1966 Zahl der Dateien in raw.dat (450; Bitfeld rpatch.dat mit n/8 */
extern signed short g_data_on_cd;                       /* ds:1968 Daten ueber das CD-Laufwerk aus ..\path.cfg gefunden (R05 lo */
extern struct recipe g_recipes[];                       /* ds:196a Alchimie-Rezepte (14 zu 0x1c Bytes; BE ALCHEMY_RECIPES) */
extern struct skill_attribs g_skill_attribs[NR_SKILLS]; /* ds:1af2 drei Probeneigenschaften je Talent (4 Bytes) */
extern struct skill_max_incs g_skill_max_incs[];        /* ds:1af5 = &g_skill_attribs[0].max_incs (eigenes Public): Steigerunge */
extern struct range_idx g_skills_index[];               /* ds:1bc2 Talentgruppen {erstes Talent, Anzahl} (BE SKILLS_INDEX) */
extern struct staffspell_desc g_staffspell_descriptions[];/* ds:1e80 Stabzauber je Stufe (BE STAFFSPELL_DESCRIPTIONS) */
extern struct spell_desc g_spell_descriptions[87];      /* ds:1eaa Zauberbeschreibungen je Zauber 0..86 (BE SPELL_DESCRIPTIONS) */
extern struct spell_ani g_spell_ani[];                  /* ds:1eb0 = &g_spell_descriptions[0].f6 (eigenes Public): Wirkungsbild */
extern struct spell_snd g_spell_snd[];                  /* ds:1eb4 = &g_spell_descriptions[0].snd (eigenes Public): Klang je Za */
extern struct range_idx g_spells_index[];               /* ds:22be Zauberklassen {erster Zauber, Anzahl} (BE SPELLS_INDEX) */
extern struct range_idx g_spells_index2[];              /* ds:22ce weitere Zauberklassen (4; O094 Seite 5) */
extern struct mon_spell_desc g_mon_spell_descriptions[15];/* ds:22d6 Gegnerzauber 0..14 (BE MON_SPELL_DESCRIPTIONS) */
extern struct mon_spell_ani g_mon_spell_ani[];          /* ds:22dd Byte +7 der Gegnerzauber ds:22d6 (eigenes Public): Wirkungsb */
extern signed char g_mon_spell_repertoire[];            /* ds:236c Zauber je enemy.mag_id (5, Ende -1) (BE MON_SPELL_REPERTOIRE */
extern signed short g_spells_own_test[];                /* ds:2425 Zauber, deren Routine die Probe selbst macht (Wortliste, -1  */
extern signed short g_spells_no_sfight[];               /* ds:244d Zauber ohne Wirkung im Sonderkampf 0x493 gegen Gegner/sich s */
extern char g_save_runv[];                              /* ds:246d Anfang des Spielstandblocks RUNV (O106) */
extern signed long g_ingame_timers[50];                 /* ds:246e 50 Zeitzaehler (Ticks), sub_timers senkt sie (BE INGAME_TIME */
extern signed long g_acrobatics_timer;                  /* ds:2486 Sperrzeit Akrobatik (O006, 8 h) */
extern signed long g_music_timer;                       /* ds:248a Sperrzeit Musizieren (O006, 8 h) */
extern signed long g_dance_timer;                       /* ds:24de Sperrzeit Tanzen (O006, 8 h) */
extern char g_save_runv_end[];                          /* ds:2536 Ende des Blocks RUNV (O106) */
extern char g_save_play[];                              /* ds:2537 Anfang des Spielstandblocks PLAY (O106) */
extern signed char g_anims_on;                          /* ds:2538 Option Animationen (Vorgabe 1, O032; O015 Zoom, O033 Tempel, */
extern signed char g_playflags[];                       /* ds:2538 Optionen/Spielmerker als Bytefeld (Testbefehle PVIEW/PSET, O */
extern signed char g_free_move;                         /* ds:2539 freie Bewegung statt Schritten (R12 move_input, R13 cam_move */
extern signed char g_subtitles;                         /* ds:253b Option Untertitel (O039/O040/O041 zeigen Texte nur bei 1; O */
extern signed char g_print_to_file;                     /* ds:253c Drucken in eine Datei statt auf den Drucker (Charakterbogen  */
extern signed char g_text_speed;                        /* ds:253d Textgeschwindigkeit (Wartezeit (n + 1) * 20 bzw. n * 20 + 20 */
extern signed char g_anim_speed;                        /* ds:2540 Animationsgeschwindigkeit 0..3 (Vorgabe 2; O016) (vermutet) */
extern signed char g_fight_speed;                       /* ds:2541 Kampfgeschwindigkeit (O147: Wartezeit n * 20 + 40; O134: End */
extern signed char g_opt_fight_keep_view;               /* ds:2542 Optionsschalter (O199 Zeile 8 links, Werte 1/0): 0 -> Bilds */
extern signed char g_win_centered;                      /* ds:2543 Option: Fenster fest in der Mitte - nicht mit der Mau */
extern signed char g_opt_music;                         /* ds:2544 Option Musik an (O106 setzt g_snd_music.enabled; BE MUSIC_E */
extern signed char g_opt_sound;                         /* ds:2545 Option Klangeffekte an (O106 setzt g_snd_fx.enabled; O072 s */
extern signed char g_cd_speech;                         /* ds:2546 Sprachfassung (CD): Sprachklaenge und Dateilisten mit Sprac */
extern signed char g_vol_music;                         /* ds:2547 Musiklautstaerke (R04 Sound_volume2) */
extern signed char g_vol_fx;                            /* ds:2548 Lautstaerke Klangeffekte (R04 snd_start) */
extern signed char g_vol_speech;                        /* ds:2549 Lautstaerke Sprachausgabe (R04 snd_start) */
extern signed char g_game_mode;                         /* ds:254a Spielmodus GAME_MODE_BEGINNER (0) / GAME_MODE_ADVANCED (1)  */
extern signed char g_saver_level;                       /* ds:254b Bildschirmschoner-Stufe 0..3 (O224 Tm::level: 10/5/2 x 1080  */
extern signed char g_print_width;                       /* ds:254c Druck: Zeilenbreite in Zeichen (O105) */
extern signed char g_print_page_len;                    /* ds:254d Druck: Zeilen je Seite (O105) */
extern signed char g_inv_show_body;                     /* ds:254f Inventar zeigt die Koerperplaetze (sonst nur Rucksack 15..30 */
extern signed char g_town_cells_state;                  /* ds:256a Zustand der tageszeitabhaengigen Felder 1/2 (O046 town_dayti */
extern signed char g_flags[];                           /* ds:256a Spielmerker als Bytefeld (Testbefehle VIEW/SET, O011; erstes */
extern signed char g_topic_kvirasim;                    /* ds:256b Ortsthema 7 "Kvirasim" (TALKS_00.LTX) einmal freigeschaltet */
extern signed char g_topic_gashok;                      /* ds:256c Ortsthema 8 "Gashok" (TALKS_01.LTX) einmal freigeschaltet ( */
extern signed char g_topic_lowangen;                    /* ds:256d Ortsthema 9 "Lowangen" (TALKS_02.LTX) einmal freigeschaltet */
extern signed char g_topic_tiefhusen;                   /* ds:256e Ortsthema 0x0a "Tiefhusen" (TALKS_03.LTX) einmal freigescha */
extern signed char g_topic_tjolmar;                     /* ds:256f Ortsthema 0x0b "Tjolmar" (TALKS_04.LTX) einmal freigeschalt */
extern signed char g_topic_finsterkoppen;               /* ds:2570 Gebietsthema 0x0c "Finsterkoppen" (TALKS_05.LTX) bekannt -  */
extern signed char g_topic_ansvell;                     /* ds:2571 Gebietsthema 0x0d "Ansvell" (TALKS_06.LTX) bekannt */
extern signed char g_topic_salamandersteine;            /* ds:2572 Gebietsthema 0x0e "Salamandersteine" (Gebirge, TALKS_07.LTX */
extern signed char g_topic_finsterkamm;                 /* ds:2573 Gebietsthema 0x0f "Finsterkamm" (TALKS_08.LTX) bekannt */
extern signed char g_topic_svellttal;                   /* ds:2574 Gebietsthema 0x10 "Svellttal" (TALKS_09.LTX) bekannt */
extern signed char g_topic_svelltsuempfe;               /* ds:2575 Gebietsthema 0x11 "Svelltsuempfe" (TALKS_10.LTX) bekannt */
extern signed char g_topic_thasch;                      /* ds:2576 Gebietsthema 0x12 "Thasch" (TALKS_11.LTX) bekannt */
extern signed char g_topic_blutzinnen;                  /* ds:2577 Gebietsthema 0x13 "Blutzinnen" (TALKS_12.LTX) bekannt; auch */
extern signed char g_topic_rorwhed;                     /* ds:2578 Gebietsthema 0x14 "Rorwhed" (TALKS_13.LTX) bekannt */
extern signed char g_tev_peddler_trail;                 /* ds:2579 Weg 1/2: Kesselhaendler getroffen, beim naechsten Mal die B */
extern signed char g_npc6_met;                          /* ds:257a Szene npc6_encounter gesehen (O164) */
extern signed char g_once_anim;                         /* ds:257c show_once_anim gesehen (O164) */
extern signed char g_slow5_on;                          /* ds:257d Gruppe sichert nach vorne (FEATURE.LTX[18] "nach vorne siche */
extern signed char g_slow4_on;                          /* ds:257e Gruppe ist angeseilt (FEATURE.LTX[20] "anseilen", O217 bei G */
extern signed char g_valley_asked;                      /* ds:257f Tal mit Hinterhalt: Frage gestellt (O165 valley_ambush) */
extern signed char g_mountain_asked;                    /* ds:2580 Gebirge: Frage nach dem Anseilen gestellt (O165 mountain_rop */
extern signed char g_armed_fight_lost;                  /* ds:2581 vermutet: Kampf gegen Bewaffnete (O164 armed_hero_fight) no */
extern signed char g_tev099_rockslide;                  /* ds:2582 Weg 99: Steinlawine der Wegelagerer liegt noch (o165_03d0 f */
extern signed char g_salamander_reported;               /* ds:2583 Salamanderstein-Hinweis auf Weg 123 schon erhalten (O120 te */
extern signed char g_tev_avalanche_done;                /* ds:2584 Wege 147..151: Lawine durch Ruestungsgeklirr schon einmal a */
extern signed char g_tev147_rockslide;                  /* ds:2585 Weg 147: Lawine liegt noch (o165_03d0 freiraeumen; O121 tev */
extern signed char g_tev148_rockslide;                  /* ds:2586 Weg 148: Lawine liegt noch (O121 tevent_148) */
extern signed char g_tev149_rockslide;                  /* ds:2587 Weg 149: Lawine liegt noch (O121 tevent_149) */
extern signed char g_tev151_rockslide;                  /* ds:2588 Weg 151: Lawine liegt noch (O121 tevent_151) */
extern signed char g_trial_lost;                        /* ds:258a Gerichtsverhandlung in Gashok (Objekt Intro O039): 1 = Bewe */
extern signed char g_shrine_state;                      /* ds:258b Spendenschrein: Zustand 0/1 (O164 shrine_donation) */
extern signed char g_shrine_paid;                       /* ds:258c Spendenschrein: schon gespendet (O164) */
extern signed char g_artherion_heard;                   /* ds:258d von Artherion gehoert: einmalige Auskunft XDS.LTX[42]/[43]  */
extern signed char g_artherion_hut_known;               /* ds:258e Weg zu Artherions Huette bekannt: Ort 0x56 Richtung Nordwes */
extern signed char g_artherion_hut_visited;             /* ds:258f Lichtung mit Artherions Huette besucht (O017: POINT.LTX[54. */
extern signed char g_artherion_done;                    /* ds:2590 Artherion: Begegnung abgeschlossen (geflohen bzw. Schwert u */
extern signed char g_ferry_landed;                      /* ds:2592 Gruppe an der Faehrstation angelandet: Ortstext [10] vor dem */
extern signed char g_rondra_priestess_met;              /* ds:2593 Wegereignis 0 einmal erlebt: SECTION.LTX[0] "Kampfeslaerm"  */
extern signed char g_ferry_robbed;                      /* ds:2594 Weg 10: Faehre selbst uebergesetzt und gekentert - kein Fae */
extern signed char g_tev000_warrior_cursing;            /* ds:2595 nie gesetzt (immer 0): Zusatztext SECTION.LTX[13] "hoert Ih */
extern signed char g_scene31_due;                       /* ds:2596 Szene travel_scene_31 steht noch aus (O215: bei 1/3 d */
extern signed char g_tev111_seen;                       /* ds:2597 Weg 111: Ereignis schon erlebt (O119 tevent_111) */
extern signed char g_tev066_cub_killed;                 /* ds:2599 Weg 66/69: Baerenjunges erlegt - die Baerin greift an (O117 */
extern signed char g_finsterkopp_entered;               /* ds:259a Finsterkopp-Binge betreten (O125 place_event_106 setzt 1; O */
extern signed char g_low_tunnel_known;                  /* ds:259b Geheimgang nach Lowangen bekannt: LOWANGEN.LTX[86] "Ihr oef */
extern signed char g_guest_lock[];                      /* ds:259c Schenken 9..18: Sperre der 7 Gaeste in Stunden (O241 tavern */
extern signed char g_guest_lock2[];                     /* ds:25a3 Schenken 3..5: Sperre der 3 Gaeste in Stunden (O241; O005) */
extern signed char g_topic_binge;                       /* ds:25a6 Gespraechsthema 0x2a "Zwergenbinge" frei (STDTALKS.LTX[188] */
extern signed char g_topic_salamander_stone;            /* ds:25a8 Gespraechsthema 0x2b "Salamanderstein" frei (STDTALKS.LTX[1 */
extern signed char g_topic_ingramosch;                  /* ds:25a9 Gespraechsthema 0x2c "Ingramosch" frei (STDTALKS.LTX[190] " */
extern signed char g_topic_sternenschweif;              /* ds:25aa Gespraechsthema 0x2d "Sternenschweif" frei (STDTALKS.LTX[19 */
extern signed char g_topic_siebenquell;                 /* ds:25ab Gespraechsthema 0x2e "Siebenquell" (STDTALKS.LTX[192]): 1 f */
extern signed char g_topic_egelbronn;                   /* ds:25ac Gespraechsthema 0x2f "Egelbronn" frei (STDTALKS.LTX[193] "K */
extern signed char g_topic_artherion;                   /* ds:25ae Gespraechsthema 0x31 "Artherion" frei (STDTALKS.LTX[195] "W */
extern signed char g_kvirasim_arrived;                  /* ds:25af Ankunft in Kvirasim erlebt (O183 nach der Ankunftssequenz): */
extern signed char g_rose_active;                       /* ds:25b0 Richtungswahl mit der Kompassrose laeuft (route_choose_dir) */
extern signed char g_rose_two_marks;                    /* ds:25b1 route_choose_dir: zwei verschiedene Kartenpunkte (map_pts 0/ */
extern signed char g_binge_key_state;                   /* ds:25b2 Goldschluessel zur Binge: 2 = vom Priester erhalten (O258 g */
extern signed char g_low25_topic59;                     /* ds:25b4 LOW25.TLK (Schenke): Thema 0x59 erledigt (O258 Antwort 4, A */
extern signed char g_lore_told;                         /* ds:25b5 Erzaehlung XDS.LTX[0..8] (Zwerge/Sternenschweif/Tordol) geho */
extern signed char g_low34_ch_passed;                   /* ds:25b6 LOW34.TLK: CH-Probe +4 des ersten Helden gelungen (O258 con */
extern signed char g_low_kysira_ref;                    /* ds:25b7 Lowangen: Empfehlung von Kysira fuer Zaskia (O031 zaskia; O */
extern signed char g_music_played;                      /* ds:25b8 Musik vorgespielt, Kraeutertrunk erhalten (O259 conv_scene_m */
extern signed char g_sites_shock_done;                  /* ds:25b9 Sonderort-Stadt: Zauberer-Schaden beim Betreten erledigt (O */
extern signed char g_sites_hint_done;                   /* ds:25ba Sonderort-Stadt: Hinweis txt[1] gezeigt (O166 sites_hint; O */
extern signed char g_sites_paid;                        /* ds:25bb Sonderorte: Abfindung gezahlt (O166 challenge; evening_figh */
extern signed char g_sites_evening;                     /* ds:25bc Sonderorte: Abendkampf 0 offen, 1 angeboten, -1 gewonnen, - */
extern signed char g_port_talk;                         /* ds:25bd Gespraech port_talk: 1 abgebrochen, 2 vollstaendig (O005 tim */
extern signed char g_port_can_haggle;                   /* ds:25be Ueberfahrt kann ausgehandelt werden (O249) */
extern signed char g_port_paid;                         /* ds:25bf Ueberfahrt bezahlt (O249) */
extern signed char g_port_poor;                         /* ds:25c0 einmal zu wenig Geld fuer die Ueberfahrt (O249) */
extern signed char g_port_deal;                         /* ds:25c1 besonderer Ausgang der Verhandlung (txt[48]; O249 port_night */
extern signed char g_ingramosch_talked;                 /* ds:25c2 Tjolmar: Gespraech an der Tuer des Zwergs Ingramosch gefuehr */
extern signed char g_dream_seen;                        /* ds:25c3 Traumsequenz Anim3 gesehen (O040 Anim3::run setzt 1; O178 H */
extern signed char g_low_beggars_scared;                /* ds:25c4 Lowangen: Bettler der Markthalle verscheucht (O184 tribute; */
extern signed char g_low_notices_read;                  /* ds:25c5 Lowangen: Bekanntmachungen am Magistrat gelesen (O184 story */
extern signed char g_eolan_state;                       /* ds:25c6 Zustand Eolan (Ort 7 in Lowangen, LOW48.TLK): 2 = Lowangen- */
extern signed char g_low_deringorn_state;               /* ds:25c7 Lowangen: Paeckchen fuer Meister Deringorn (1 = dabei, 2 =  */
extern signed char g_low_brooch_state;                  /* ds:25c8 Lowangen: Auftrag Brosche aus dem Schauhaus (> 0 offen, -1  */
extern signed char g_low_guard_talk;                    /* ds:25c9 Lowangen: Gespraech 0x47 im Gardehaus freigegeben (O031 gua */
extern signed char g_low_gate1_open;                    /* ds:25ca Lowangen: Gespraech LOW54 am ersten Stadttor laeuft (O031;  */
extern signed char g_low_gate2_open;                    /* ds:25cb Lowangen: Gespraech LOW54 am zweiten Stadttor laeuft (O031; */
extern signed char g_low_dragan_task1;                  /* ds:25cc Lowangen: erster Auftrag Dragans offen (> 0; O031 dragan, O */
extern signed char g_altrogolan_absent;                 /* ds:25cd Altrogolan oeffnet nicht (LOWANGEN.LTX[70] "Niemand reagier */
extern signed char g_low_ailian_state;                  /* ds:25ce Lowangen: Zustand beim Elfen Ailian Siebenquell (-1, 0..5;  */
extern signed char g_vindaria_choice;                   /* ds:25cf vermutet: gewaehlte Antwort 1/2 im Gespraech mit Vindaria ( */
extern signed char g_vindaria_state;                    /* ds:25d0 Zustand Vindaria Egelbronn (LOW60.TLK, Ev::vindaria O031):  */
extern signed char g_dragan_tip_jandora;                /* ds:25d1 Dragans Tipp: schwarze Jandora (Tagebuch XDS.LTX[49]; O258) */
extern signed char g_dragan_tip_brin;                   /* ds:25d2 Dragans Tipp: Brin Vaskendantz, Unterkunft (Tagebuch XDS.LTX */
extern signed char g_dragan_tip_raisha;                 /* ds:25d3 Bit 0: Dragans Tipp Raisha Rotenegger (XDS.LTX[48]), Bit 1:  */
extern signed char g_low_falkert_known;                 /* ds:25d4 Lowangen: Falkert Duberwin bekannt (O031 falkert; O102, O25 */
extern signed char g_dragan_task;                       /* ds:25d5 Lowangen: Auftrag Dragans 0..2 (Vinsalter, Brosche, Paeckche */
extern signed char g_low_dragan_news;                   /* ds:25d6 Lowangen: Dragan hat Neuigkeiten (Auftrag erfuellt; O031, O */
extern signed char g_low_gavron_house;                  /* ds:25d7 Lowangen: Haus, in dem Gavron verschwunden ist (0x3f..0x41  */
extern signed char g_guide_won;                         /* ds:25d8 Kampf statt Fuehrer gewonnen (O164 guide_offer) */
extern signed char g_dng02_tools_state;                 /* ds:25d9 Dungeon 2: Regal mit Gartengeraeten (FINAL.LTX[0..2]): 0 neu */
extern signed char g_dng02_jugs_state;                  /* ds:25da Dungeon 2: Kruege an 0x907 (FINAL.LTX[6]/[7]): 1 gesehen, 2  */
extern signed char g_dng02_fishhook_taken;              /* ds:25db Dungeon 2: Angelhaken unter dem Kissen genommen (O130, FINAL */
extern signed char g_dng02_otter_fur_taken;             /* ds:25dc Dungeon 2: Otterfell unter der Matratze genommen (O130, FINA */
extern signed char g_dng02_dice_money_taken;            /* ds:25dd Dungeon 2: Kleingeld auf dem Wuerfeltisch genommen (200 Hell */
extern signed char g_dng02_recipe_taken;                /* ds:25de Dungeon 2: Rezept fuer Schlaftrunk unter den Papieren genomm */
extern signed char g_dng02_secret_switch;               /* ds:25df Dungeon 2: geheimer Schalter an 0x502 betaetigt, Durchgang o */
extern signed char g_dng02_keyhole_door;                /* ds:25e0 Dungeon 2: Schluesselloch in der Wand 0x1211 mit dem Bronzes */
extern signed char g_dng02_sun_lock_open;               /* ds:25e1 Dungeon 2: Schloss 0x1c1a mit dem sonnenfoermigen Schluessel */
extern signed char g_dng03_secret_door1;                /* ds:25e2 Dungeon 3: Geheimtuer an 0x110e geoeffnet (Schloesser +12, F */
extern signed char g_dng03_secret_door2;                /* ds:25e3 Dungeon 3: Geheimtuer an 0xc0b geoeffnet (O131) */
extern signed char g_dng03_password_solved;             /* ds:25e4 Dungeon 3: Statue an 0x617 mit dem Namen ARKANDOR beantworte */
extern signed char g_dng03_voice_heard;                 /* ds:25e5 Dungeon 3: Sprachausgabe 15 an 0x131d einmal gespielt (O131) */
extern signed char g_dragon_claw_found;                 /* ds:25e6 Dungeon 3: Geheimfach in der Saeule mit Helm und Drachenklau */
extern signed char g_coward_stuck;                      /* ds:25e7 der aengstlichste Held weigert sich weiterzugehen (O129 even */
extern signed char g_dng03_combination_open;            /* ds:25e8 Dungeon 3: Zahlenschloss an 0x151d mit 1945 geoeffnet (FINAL */
extern signed char g_dng03_combination_tries;           /* ds:25e9 Dungeon 3: Fehlversuche am Zahlenschloss (ab 5 Spott mit Hin */
extern signed char g_dng03_skeleton_warriors_171d;      /* ds:25ea Dungeon 3: Kampf an 0x171d gewonnen (O131 dng03_fights) - e */
extern signed char g_dng03_skeletons_191c;              /* ds:25eb Dungeon 3: Kampf an 0x191c gewonnen (O131) - erster Gegner  */
extern signed char g_dng03_zombies_1519;                /* ds:25ec Dungeon 3: Kampf an 0x1519 gewonnen (O131) - erster Gegner  */
extern signed char g_dng03_skeleton_warriors_041d;      /* ds:25ed Dungeon 3: Kampf an 0x41d gewonnen (O131) - erster Gegner S */
extern signed char g_dng03_zombies_1b15;                /* ds:25ee Dungeon 3: Kampf an 0x1b15 gewonnen (O131) - erster Gegner  */
extern signed char g_dng03_skeleton_warriors_0b17;      /* ds:25ef Dungeon 3: Kampf an 0xb17 gewonnen (O131) - erster Gegner S */
extern signed char g_dng03_zombies_1313;                /* ds:25f0 Dungeon 3: Kampf an 0x1313 gewonnen (O131) - erster Gegner  */
extern signed char g_dng03_heshthot_130e;               /* ds:25f1 Dungeon 3: Kampf an 0x130e gewonnen (O131) - erster Gegner  */
extern signed char g_dng03_sorcerer_080c;               /* ds:25f2 Dungeon 3: Kampf an 0x80c gewonnen (O131) - erster Gegner Z */
extern signed char g_dng03_mages_0702;                  /* ds:25f3 Dungeon 3: Kampf an 0x702 gewonnen (O131) - erster Gegner M */
extern signed char g_dng03_mages_0704;                  /* ds:25f4 Dungeon 3: Kampf an 0x704 gewonnen (O131) - erster Gegner M */
extern signed char g_dng03_priests_0d01;                /* ds:25f5 Dungeon 3: Kampf an 0xd01 gewonnen (O131) - erster Gegner P */
extern signed char g_dng03_knight_0e1d;                 /* ds:25f6 Dungeon 3: Kampf an 0xe1d gewonnen (O131; mit g_fight_ab_lo */
extern signed char g_dng03_desk_opened;                 /* ds:25f7 Dungeon 3: Geheimfach im Schreibtisch an 0x105 geoeffnet (Gi */
extern signed char g_tavern_dinner_asked;               /* ds:25f8 Schenken 3..5: Eingangsfrage in tavern_order_dinner gestell */
extern signed char g_peddler_away;                      /* ds:25f9 fahrender Haendler 1/2: Tage bis er wieder anzutreffen ist ( */
extern signed char g_peddler3_away;                     /* ds:25fa fahrender Haendler 3: wie g_peddler_away */
extern signed char g_place197_done;                     /* ds:25fb Ort 0xc5 einmal erreicht: Route endet dort (O216; Ort */
extern signed char g_dng04_path_281d;                   /* ds:25fc Dungeon 4: beim Betreten von 0x281d wird Feld (0x1e, 0x29) z */
extern signed char g_dng04_path_271c;                   /* ds:25fd Dungeon 4: beim Betreten von 0x271c wird Feld (0x1d, 0x29) z */
extern signed char g_dng04_rantzen_271b;                /* ds:25fe Dungeon 4: Sumpfrantzen-Kampf an 0x271b gewonnen (O195; Anza */
extern signed char g_dng04_einbeeren_found;             /* ds:25ff Dungeon 4: 5 Einbeeren an 0x2d1d gefunden (NATURE.LTX[0]; O1 */
extern signed char g_dng04_belmart_found;               /* ds:2600 Dungeon 4: Belmart an 0x1409 gefunden (NATURE.LTX[1]; O195) */
extern signed char g_dng04_snake_0c19;                  /* ds:2601 Dungeon 4: Schlangenstelle 0x0c19 schon besucht (danach zu 1 */
extern signed char g_dng04_snake_1817;                  /* ds:2602 Dungeon 4: Schlangenstelle 0x1817 (O195) */
extern signed char g_dng04_snake_211a;                  /* ds:2603 Dungeon 4: Schlangenstelle 0x211a (O195) */
extern signed char g_dng04_snake_2419;                  /* ds:2604 Dungeon 4: Schlangenstelle 0x2419 (O195) */
extern signed char g_dng04_rantzen_1413;                /* ds:2605 Dungeon 4: Sumpfrantzen-Kampf an 0x1413 gewonnen (O195) */
extern signed char g_dng04_rantzen_2701;                /* ds:2606 Dungeon 4: Sumpfrantzen-Kampf an 0x2701 gewonnen (O195) */
extern signed char g_dng04_rantzen_left;                /* ds:2607 Dungeon 4: verbleibende Sumpfrantzen im Altsvellt (O032 init */
extern signed char g_dng04_gulmond_found;               /* ds:2608 Dungeon 4: Gulmond-Pflanze an 0x20f gefunden (NATURE.LTX[6]; */
extern signed char g_dng04_village_visited;             /* ds:2609 Dungeon 4: Echsendorf an 0xf10 betreten (NATURE.LTX[7], Gesp */
extern signed char g_pic2_seen;                         /* ds:260b Vollbild 2 (show_fullscreen_pic2, Gegenstand 0x14d) angeseh */
extern signed char g_price_level;                       /* ds:260c Preisstufe im Dialog 0x2bd (O013 talk_price: 3..5 nach Geld */
extern signed char g_dng04_hedge_cut;                   /* ds:260d Dungeon 4: Durchgang durch die Hecke an 0x116 gehauen (NATUR */
extern signed char g_dng04_rantzen_0405;                /* ds:260e Dungeon 4: Sumpfrantzen-Kampf an 0x405 gewonnen (O195) */
extern signed char g_dng04_rantzen_0e16;                /* ds:260f Dungeon 4: Sumpfrantzen-Kampf an 0xe16 gewonnen (O195) */
extern signed char g_dng04_goblins_0109;                /* ds:2610 Dungeon 4: Kampf an 0x109 gewonnen (O195) - erster Gegner G */
extern signed char g_dng04_goblins_040d;                /* ds:2611 Dungeon 4: Kampf an 0x40d gewonnen (O195) - erster Gegner G */
extern signed char g_dng04_rantzen_1e08;                /* ds:2612 Dungeon 4: Kampf 0x9b an 0x1e08/0x1d08 gewonnen (nur bei g_ */
extern signed char g_dng04_zombies_1219;                /* ds:2613 Dungeon 4: Kampf an 0x1219 gewonnen (O195) - erster Gegner  */
extern signed char g_dng04_zombies_2d0e;                /* ds:2614 Dungeon 4: Kampf an 0x2d0e gewonnen (O195) - erster Gegner  */
extern signed char g_dng04_zombies_0919;                /* ds:2615 Dungeon 4: Kampf an 0x919 gewonnen (O195) - erster Gegner Z */
extern signed char g_dng04_orcs_200c;                   /* ds:2616 Dungeon 4: Kampf an 0x200c gewonnen (O195) - erster Gegner  */
extern signed char g_dng04_orcs_2510;                   /* ds:2617 Dungeon 4: Kampf an 0x2510 gewonnen (O195) - erster Gegner  */
extern signed char g_dng04_orcs_0413;                   /* ds:2618 Dungeon 4: Kampf an 0x413 gewonnen (O195) - erster Gegner O */
extern signed char g_dng04_orcs_0601;                   /* ds:2619 Dungeon 4: Kampf an 0x601 gewonnen (O195) - erster Gegner O */
extern signed char g_dng04_orcs_1402;                   /* ds:261a Dungeon 4: Kampf an 0x1402 gewonnen (O195) - erster Gegner  */
extern signed char g_dng04_orcs_1c02;                   /* ds:261b Dungeon 4: Kampf an 0x1c02 gewonnen (O195) - erster Gegner  */
extern signed char g_heather_recipe_read;               /* ds:261c Pergament 0x150 gelesen: USE.LTX[98] "Transformationsanleit */
extern signed char g_street_flag1;                      /* ds:261d vor dem Gespraech L_STREET 1 auf 0 gesetzt (O185 lowangen_w */
extern signed char g_street_flag2;                      /* ds:261e wie g_street_flag1 */
extern signed char g_street_guards;                     /* ds:261f Lowangen: Zahl der Gardisten der Patrouille (W4 + 3; O185, L */
extern signed char g_street_thugs;                      /* ds:2620 Lowangen: Zahl der Verfolger der Strassenbande (W15 + 5; O18 */
extern signed char g_camp_juggler_met;                  /* ds:2621 Gaukler als Gast am Lagerfeuer gewesen (O067/O073) */
extern signed char g_camp_druid_met;                    /* ds:2622 Druide als Gast am Lagerfeuer gewesen (O067/O073) */
extern signed char g_camp_hunter_met;                   /* ds:2623 Jaeger als Gast am Lagerfeuer gewesen (O067/O073) */
extern signed char g_camp_vagabond_met;                 /* ds:2624 Streuner als Gast am Lagerfeuer gewesen (O067/O073) */
extern signed char g_tavgas_visits;                     /* ds:2625 Schenken in Gashok: Gespraeche gezaehlt, beim dritten Sites: */
extern signed char g_inn_vision_state;                  /* ds:2626 Vision in der Herberge von Gashok (Ort 2..3, Sites::inn_vis */
extern signed char g_swamp_chief_refused;               /* ds:2627 vermutet: Echsen-Dorfvorsteher (SWAMP.TLK, Zustand 4) abgew */
extern signed char g_swamp_chief_offended;              /* ds:2628 vermutet: Echsen-Dorfvorsteher (SWAMP.TLK, Zustand 2) verpr */
extern signed char g_dng04_monster_killed;              /* ds:2629 Dungeon 4: das Untier, das die Eier der Echsen frisst (Schli */
extern signed char g_dng04_tower_open;                  /* ds:262a Dungeon 4: Auftrag der Alten im Sueden (SWAMP.TLK, O102): 1  */
extern signed char g_sites_challenges;                  /* ds:262b Sonderorte: Zahl der Herausforderungen 1..4, -1 erledigt (O */
extern signed char g_sites_rematch;                     /* ds:262c Sonderorte: Revanche moeglich 1, -1 angeboten, -2 gewonnen  */
extern signed char g_guide_avail;                       /* ds:262d Fuehrer verfuegbar (O164 guide_offer bei 1; O005 timers_dail */
extern signed char g_finster_anger;                     /* ds:262e Zorn der Zwerge nach Frevel: Stunden 1..10 in Stadt 3 (R10  */
extern signed char g_diary_new_items;                   /* ds:2630 Merker: erste Funde von Gegenstaenden ins Tagebuch (give_her */
extern signed char g_low_stone_delivered;               /* ds:2631 Salamanderstein in Lowangen abgegeben (O185 lowangen_arriva */
extern signed char g_low_stone_days;                    /* ds:2632 Tage seit der Abgabe des Salamandersteins in Lowangen (O185 */
extern signed char g_voyage_done;                       /* ds:2633 Schiffsreise der Hafenstadt gesehen (O183 town_ship_voyage s */
extern signed char g_elsurion_met;                      /* ds:2634 vermutet: Treffen mit Elsurion Sternlicht in Kvirasim erleb */
extern signed char g_dng13_smoke_timer;                 /* ds:2635 Dungeon 13: Qualm nach dem Abfackeln des brennenden Pelzes  */
extern signed char g_dng10_countdown;                   /* ds:2636 Dungeon 10: Zaehler bis zum Ereignis (O000 dng10_countdown; */
extern signed char g_dng11_floor_collapsed;             /* ds:2637 Dungeon 11: Boden unter dem Hebel eingebrochen (O062 dng11_ */
extern signed char g_binge_hole1_looted;                /* ds:2638 Binge: Schatulle aus dem Wandloch 1 (Dungeon 11) gezogen (O */
extern signed char g_binge_hole2_trapped;               /* ds:2639 Binge: Rattenfalle im Wandloch 2 (Dungeon 11) ausgeloest (O */
extern signed char g_dng10_tools_found;                 /* ds:263a Dungeon 10: Geraete in der Ecke untersucht (O000 dng10_tool */
extern signed char g_dng10_parchment_taken;             /* ds:263b Dungeon 10: Pergament (Runendokument) aus der Tasche des Sk */
extern signed char g_binge_torches_lit;                 /* ds:263c Fackeln am Ingerimm-Relief entzuendet (O000): Dungeon 10..1 */
extern signed char g_binge_shaft_stone_ap;              /* ds:263d Binge: AP fuer den lockeren Stein unter der Schachtplatte v */
extern signed char g_dng10_flask_taken;                 /* ds:263e Dungeon 10: Flaeschchen aus der Tasche des Skeletts genomme */
extern signed char g_dng12_pit_side;                    /* ds:263f Dungeon 12: Seite der Grube, von der die Gruppe kommt (Ziel */
extern signed char g_port_haggled;                      /* ds:2640 Ueberfahrt ausgehandelt (O249) */
extern signed char g_port_price;                        /* ds:2641 ausgehandelter Preis in 100 Hellern (O249) */
extern signed char g_dng12_cart_seen;                   /* ds:2642 Dungeon 12: Lore beschrieben (O063) */
extern signed char g_dng12_lever;                       /* ds:2643 Dungeon 12: Hebelstellung (O064 dng12_lever) */
extern signed char g_dng10_ring_taken;                  /* ds:2644 Dungeon 10: Asthenilring aus dem Kohlebecken gezogen (O060  */
extern signed char g_sacrilege;                         /* ds:2645 Frevel begangen (O000 Altar, O019, O064 bei Ansehens-Abzug; */
extern signed char g_low_burglary_result;               /* ds:2646 Lowangen: Einbruch ins Schauhaus (1 tagsueber ertappt, 2 La */
extern signed char g_dng05_supplies_taken;              /* ds:2647 Dungeon 5: 10 Proviantpakete aus den Vorraeten genommen (PHE */
extern signed char g_dng05_recipe_taken;                /* ds:2648 Dungeon 5: alte Schriftrolle (Rezept fuer Geldscheisser) an  */
extern signed char g_dng05_pit_seen;                    /* ds:264a Dungeon 5: Fallgrube an 0x214 erreicht (entdeckt oder hinein */
extern signed char g_dng05_switch_0114;                 /* ds:264b Dungeon 5: wechselt bei jedem Betreten von 0x114 zwischen 0  */
extern signed char g_sites_vision_ok;                   /* ds:264c Sonderorte: Probe in inn_vision gelungen (O166; O257) */
extern signed char g_sites_vision_failed;               /* ds:264d Sonderorte: Probe in inn_vision misslungen (O166; O257) */
extern signed char g_sites_night_met;                   /* ds:264e Sonderorte: naechtliche Begegnung zu Ende gefuehrt (O166; O */
extern signed char g_trnd_once1_done;                   /* ds:264f Zufallsereignis town_rnd_once_01 gesehen (O239) */
extern signed char g_trnd_once2_done;                   /* ds:2650 Zufallsereignis town_rnd_once_02 abgeschlossen (O239) */
extern signed char g_trnd_twice_state;                  /* ds:2651 Stufe von town_rnd_twice_0c 0..2 (O239) */
extern signed char g_dng05_finale_seen;                 /* ds:2652 Dungeon 5: Schlussszene vor der Phex-Statue (phex_finale) an */
extern signed char g_dng05_bolt_trap_armed;             /* ds:2653 Dungeon 5: Bolzenfalle an 0x160b scharf (0x170b setzt 1, 0x1 */
extern signed char g_dng05_offering_made;               /* ds:2654 Dungeon 5: Dukaten in den Opferstock gelegt - die Bolzenfall */
extern signed char g_dng05_skeletons_beaten;            /* ds:2655 Dungeon 5: Skelettkrieger (Fall 0x67, PHEXTEMP.LTX[33]) besi */
extern signed char g_dng05_scroll_taken;                /* ds:2656 Dungeon 5: vergilbte Schriftrolle (Rezept fuer Zaubertrank)  */
extern signed char g_dng05_riddle_solved;               /* ds:2657 Dungeon 5: Frage der Steinplatte "Wann ist das Licht des Phe */
extern signed char g_dng05_puzzle_won;                  /* ds:2658 Dungeon 5: Legespiel an 0x30b gewonnen (O208 Puzzle_run; PHE */
extern signed char g_dng05_door_070d_found;             /* ds:2659 Dungeon 5: Geheimtuer an 0x70d entdeckt, geht erst nach dem  */
extern signed char g_dng05_door_0402_found;             /* ds:265a Dungeon 5: Geheimtuer an 0x402 entdeckt, geht erst nach dem  */
extern signed char g_dng05_door_1106_found;             /* ds:265b Dungeon 5: Geheimtuer an 0x1106 entdeckt (PHEXTEMP.LTX[40];  */
extern signed char g_dng06_noise;                       /* ds:265c Dungeon 6 (Orkfestung, BLUT1.LTX): Laerm aus misslungenen Sc */
extern signed char g_dng06_alarm;                       /* ds:265d Dungeon 6: Alarm - mehr Gegner in den Zufallskaempfen (O261  */
extern signed char g_dng07_noise;                       /* ds:265e Dungeon 7/8 (Orkfestung, untere Ebenen): Laerm aus misslunge */
extern signed char g_dng07_alarm;                       /* ds:265f Dungeon 7: Alarm im Orkgefaengnis - groessere Gegnergruppen  */
extern signed char g_dng08_noise;                       /* ds:2660 Dungeon 8: Laerm durch Schmerzensschreie (Bolzen an 0x213 +1 */
extern signed char g_dng08_alarm;                       /* ds:2661 Dungeon 8: Alarm (BLUT3.LTX[53] an der grossen Halle, Kampf  */
extern signed char g_dng06_rack_done;                   /* ds:2664 Dungeon 6: Waffenstaender an 0x1d02 leer genommen bzw. zersc */
extern signed char g_dng06_orcs_1b04;                   /* ds:2665 Dungeon 6: Kampf an 0x1b04.. nach misslungenem Schleichen g */
extern signed char g_dng06_helmet_taken;                /* ds:2666 Dungeon 6: Topfhelm an 0x90a genommen (BLUT1.LTX[17]; O261) */
extern signed char g_dng06_statue_looted;               /* ds:2667 Dungeon 6: Brazoragh-Statue gepluendert (BLUT1.LTX[24]; O262 */
extern signed char g_dng06_statue_destroyed;            /* ds:2668 Dungeon 6: Brazoragh-Statue zerschlagen (BLUT1.LTX[25]/[26]; */
extern signed char g_dng06_statue_ignored;              /* ds:2669 Dungeon 6: Brazoragh-Statue ignoriert (Praios -10, einmalig; */
extern signed char g_dng06_hornets_destroyed;           /* ds:266a Dungeon 6: die beiden Hornissen (Kriegsarmbrueste) an 0x1807 */
extern signed char g_dng06_pit_1305;                    /* ds:266b Dungeon 6: Klappgrube an 0x1305 (BLUT1.LTX[33..36]: 1 Stein- */
extern signed char g_dng06_pit_1519;                    /* ds:266c Dungeon 6: Klappgrube an 0x1519 (wie g_dng06_pit_1305) */
extern signed char g_dng06_rockfall1;                   /* ds:266d Dungeon 6: Steinschlag an 0x307/0x709/0x30c ausgeloest (blut */
extern signed char g_dng06_rockfall2;                   /* ds:266e Dungeon 6: Steinschlag an 0x30a/0x60c ausgeloest (blut_rockf */
extern signed char g_dng06_skeleton_dug;                /* ds:266f Dungeon 6: Zwergenskelett unter dem Geroell an 0x410 freigel */
extern signed char g_dng06_beer_drunk;                  /* ds:2670 Dungeon 6: aus dem vollen Bierfass an 0xf15 getrunken (O262; */
extern signed char g_dng06_dagger_taken;                /* ds:2671 Dungeon 6: Dolch aus dem Gravesh-Schrein genommen (BLUT1.LTX */
extern signed char g_dng06_dogs_state;                  /* ds:2672 Dungeon 6: Kaefig der orkischen Kriegshunde (BLUT1.LTX[60..6 */
extern signed char g_dng06_tripwire_seen;               /* ds:2673 Dungeon 6: Alarmdraht an 0x1819 bemerkt (BLUT1.LTX[75]; O261 */
extern signed char g_dng06_tripwire_hit;                /* ds:2674 Dungeon 6: Alarmdraht an 0x1819 nicht bemerkt (O261 setzt 1; */
extern signed char g_dng06_hay_state;                   /* ds:2675 Dungeon 6: Heu an 0x1e1c (BLUT1.LTX[78..85]): 1 angezuendet  */
extern signed char g_dng06_latrine_seen;                /* ds:2676 Dungeon 6: Latrine an 0x161d beschrieben (BLUT1.LTX[86]; O26 */
extern signed char g_dng06_hall_seen;                   /* ds:2677 Dungeon 6: grosse Halle an 0x1117 beschrieben (BLUT1.LTX[93] */
extern signed char g_dng06_beer_drained;                /* ds:2678 Dungeon 6: Bierfass an 0xf15 leergetrunken (BLUT1.LTX[94..99 */
extern signed char g_dng06_ogre_beer_tasted;            /* ds:2679 Dungeon 6: Ogerbier gekostet (BLUT1.LTX[112]; O262) */
extern signed char g_dng06_stew_eaten;                  /* ds:267a Dungeon 6: Eintopf bzw. Gulasch gegessen (BLUT1.LTX[100..108 */
extern signed char g_dng06_orcs_0e10;                   /* ds:267b Dungeon 6: Kampf an 0xe10/0xe11 gewonnen (O261) - erster Ge */
extern signed char g_dng06_bolt_wall_armed;             /* ds:267c Dungeon 6: Bolzenwand an 0x710 scharf (0x70f setzt 1, 0x711  */
extern signed char g_dng06_beetle_pit_armed;            /* ds:267d Dungeon 6: Fallgrube an 0x814 scharf (0x914 setzt 1, 0x714 0 */
extern signed char g_dng06_beetle_pit_fallen;           /* ds:267e Dungeon 6: in die Fallgrube mit den Schroetern an 0x814 gefa */
extern signed char g_dng07_dogs_door_2c;                /* ds:267f Dungeon 7: Kampfhunde hinter der Tuer (Lock 0x2c) besiegt (B */
extern signed char g_dng07_dogs_door_2e;                /* ds:2680 Dungeon 7: Kampfhunde hinter der Tuer (Lock 0x2e) besiegt (O */
extern signed char g_dng07_kennel1_searched;            /* ds:2681 Dungeon 7: Hundezwinger 1 durchsucht (O264) */
extern signed char g_dng07_kennel2_searched;            /* ds:2682 Dungeon 7: Hundezwinger 2 durchsucht, roter Ring (O264) */
extern signed char g_dng07_guards_beaten;               /* ds:2683 Dungeon 7: Wachen an 0xd05 (hinter der Nordtuer) besiegt (O2 */
extern signed char g_dng07_shelf_searched;              /* ds:2684 Dungeon 7: Ruestungsregal an 0x1015 als verschiebbar erkannt */
extern signed char g_dng07_shelf_hole_found;            /* ds:2685 Dungeon 7: Oeffnung ueber der Tuer hinter dem Regal entdeckt */
extern signed char g_dng07_orcs_1606;                   /* ds:2686 Dungeon 7: Zufallskampf an 0x1606 gewonnen (O266 dng07_figh */
extern signed char g_dng07_orcs_0d08;                   /* ds:2687 Dungeon 7: Zufallskampf an 0xd08/0xd0d/0xd0f/0xd19 gewonnen */
extern signed char g_dng07_orcs_100b;                   /* ds:2688 Dungeon 7: Kampf an 0x100b gewonnen, Zangenschluessel gefun */
extern signed char g_dng07_orcs_140b;                   /* ds:2689 Dungeon 7: Kampf an 0x140b gewonnen (O266) - erster Gegner  */
extern signed char g_dng07_orcs_120f;                   /* ds:268a Dungeon 7: Kampf an 0x120f gewonnen (O266) - erster Gegner  */
extern signed char g_dng07_store_seen;                  /* ds:268b Dungeon 7: Lagerraum beschrieben (O264) */
extern signed char g_dng07_statue_scraped;              /* ds:268c Dungeon 7: Gold von der Brazoragh-Statue abgeschabt (BLUT2.L */
extern signed char g_dng07_rockfall1;                   /* ds:268d Dungeon 7: Steinschlag 0x1616 (blut_rockfall) */
extern signed char g_dng07_rockfall2;                   /* ds:268e Dungeon 7: Steinschlag 0x1a1a/0x1c1d (blut_rockfall) */
extern signed char g_dng07_pit1_fallen;                 /* ds:268f Dungeon 7: Fallgrube 0xd18 ausgeloest (O264) */
extern signed char g_dng07_maggots_done;                /* ds:2690 Dungeon 7: Madenhaufen, Kampf gewonnen (O264) */
extern signed char g_dng07_cave_seen;                   /* ds:2691 Dungeon 7: Hoehle beschrieben (O264) */
extern signed char g_dng07_junk_finds;                  /* ds:2692 Dungeon 7: Zahl der Funde im Unrat 0..4 (O264) */
extern signed char g_dng07_beetles_1b15;                /* ds:2693 Dungeon 7: Kampf gegen Schroeter an 0x1b14..0x1b16 gewonnen  */
extern signed char g_dng07_beetles_1d17;                /* ds:2694 Dungeon 7: Kampf gegen Schroeter an 0x1d17.. gewonnen (O266) */
extern signed char g_dng07_beetles_181c;                /* ds:2695 Dungeon 7: Kampf gegen Schroeter an 0x171c/0x181c/0x181d gew */
extern signed char g_dng07_feed_room_seen;              /* ds:2696 Dungeon 7: Futterkammer (O264) */
extern signed char g_dng07_orcs_0a0d;                   /* ds:2697 Dungeon 7: Kampf an 0xa0d gewonnen, goldener Brillenschlues */
extern signed char g_dng07_latrine_seen;                /* ds:2698 Dungeon 7: Latrine (O264; bleibt 0) */
extern signed char g_dng07_orcs_050f;                   /* ds:2699 Dungeon 7: Zufallskampf an 0x50f/0xc0f gewonnen (O266) - er */
extern signed char g_dng07_orcs_0305;                   /* ds:269a Dungeon 7: Kampf an 0x305 gewonnen (O266) - erster Gegner O */
extern signed char g_dng07_pit2_fallen;                 /* ds:269b Dungeon 7: Fallgrube 0x312 ausgeloest (O264) */
extern signed char g_dng07_pit3_fallen;                 /* ds:269c Dungeon 7: Fallgrube 0x30c ausgeloest (O264) */
extern signed char g_dng07_orcs_0319;                   /* ds:269d Dungeon 7: Kampf an 0x319/0x51b gewonnen (nur bei Alarm ode */
extern signed char g_dng07_stew_eaten;                  /* ds:269e Dungeon 7: Eintopf gegessen (O264) */
extern signed char g_dng07_kitchen_looted;              /* ds:269f Dungeon 7: Kochgeraete/Vorraete genommen (O264) */
extern signed char g_dng07_straw_amulet;                /* ds:26a0 Dungeon 7: Silberamulett im Zellenstroh an 0x111 gefunden (B */
extern signed char g_dng07_corpse_sword_taken;          /* ds:26a1 Dungeon 7: Schwert hinter dem Spinnennetz beim Skelett genom */
extern signed char g_dng07_web_131a;                    /* ds:26a2 Dungeon 7: Spinnennetz an 0x131a zerteilt (BLUT2.LTX[84]; O2 */
extern signed char g_dng07_web_1321;                    /* ds:26a3 Dungeon 7: Spinnennetz an 0x1321 zerteilt (O265) */
extern signed char g_dng07_spiders_1321;                /* ds:26a4 Dungeon 7: Hoehlenspinnen am Netz 0x1321 besiegt (O265) */
extern signed char g_dng07_web_1022;                    /* ds:26a5 Dungeon 7: Spinnennetz an 0x1022 zerteilt (O265) */
extern signed char g_dng07_spiders_1022;                /* ds:26a6 Dungeon 7: Hoehlenspinnen am Netz 0x1022 besiegt (O265) */
extern signed char g_dng07_web_1024;                    /* ds:26a7 Dungeon 7: Spinnennetz an 0x1024 zerteilt (O265) */
extern signed char g_dng07_spiders_0e25;                /* ds:26a8 Dungeon 7: Hoehlenspinnen am Netz 0xe25 besiegt (O265) */
extern signed char g_dng07_web_0e25;                    /* ds:26a9 Dungeon 7: Spinnennetz an 0xe25 zerteilt (O265) */
extern signed char g_dng07_web_0922;                    /* ds:26aa Dungeon 7: Spinnennetz an 0x922 zerteilt (O265) */
extern signed char g_dng07_spiders_0922;                /* ds:26ab Dungeon 7: Hoehlenspinnen am Netz 0x922 besiegt (O265) */
extern signed char g_dng07_web_0720;                    /* ds:26ac Dungeon 7: Spinnennetz an 0x720 zerteilt (O265) */
extern signed char g_dng07_web_0320;                    /* ds:26ad Dungeon 7: Spinnennetz an 0x320/0x323 zerteilt (O265) */
extern signed char g_dng07_spiders_0320;                /* ds:26ae Dungeon 7: Hoehlenspinnen am Netz 0x320/0x323 besiegt (O265) */
extern signed char g_dng07_web_0927;                    /* ds:26af Dungeon 7: Spinnennetz an 0x927 zerteilt (O265) */
extern signed char g_dng07_spiders_0927;                /* ds:26b0 Dungeon 7: Hoehlenspinnen am Netz 0x927 besiegt (O265) */
extern signed char g_dng07_web_032b;                    /* ds:26b1 Dungeon 7: Spinnennetz an 0x32b zerteilt (O265) */
extern signed char g_dng07_spiders_032b;                /* ds:26b2 Dungeon 7: Hoehlenspinnen am Netz 0x32b besiegt (O265) */
extern signed char g_dng07_nest_destroyed;              /* ds:26b3 Dungeon 7: Spinnennest zerstoert (O264); bis dahin an 0x124  */
extern signed char g_dng07_rockfall3;                   /* ds:26b4 Dungeon 7: Steinschlag 0xe22 (blut_rockfall) */
extern signed char g_dng07_rockfall4;                   /* ds:26b5 Dungeon 7: Steinschlag 0xa27/0xb27 (blut_rockfall) */
extern signed char g_dng07_niche_looted;                /* ds:26b6 Dungeon 7: Nische der Spinnenkoenigin gepluendert (O264) */
extern signed char g_dng07_amulet_found;                /* ds:26b7 Dungeon 7: Spiegelamulett im Stroh der Zelle gefunden (O264) */
extern signed char g_dng08_smithy_seen;                 /* ds:26b8 Dungeon 8: Schmiede an 0xd06 beschrieben (BLUT3.LTX[0]; O267 */
extern signed char g_dng08_storeroom_shown;             /* ds:26b9 Dungeon 8: Materiallager an 0xa02 gezeigt (BLUT3.LTX[11]/[12 */
extern signed char g_dng08_banner_torn;                 /* ds:26ba Dungeon 8: Kriegsbanner der Orks in der Truhe an 0x1203 zerr */
extern signed char g_dng08_hatchet_taken;               /* ds:26bb Dungeon 8: Handbeil des Ogers an 0x1602 genommen (BLUT3.LTX[ */
extern signed char g_dng08_orcs_0d04;                   /* ds:26bc Dungeon 8: Kampf an 0xd04 gewonnen (O268 dng08_fights) - er */
extern signed char g_dng08_orcs_0e06;                   /* ds:26bd Dungeon 8: Kampf an 0xe06 gewonnen (O268) - erster Gegner O */
extern signed char g_dng08_orcs_0a06;                   /* ds:26be Dungeon 8: Kampf an 0xa06/0xa0a gewonnen (O268) - erster Ge */
extern signed char g_dng08_ogres_1603;                  /* ds:26bf Dungeon 8: Kampf an 0x1603 gewonnen (O268) - erster Gegner  */
extern signed char g_low_vindaria_known;                /* ds:26c1 Lowangen: Vindaria Egelbronn bekannt (O031; O185) */
extern signed char g_binge_hole3_looted;                /* ds:26c3 Binge: Heiltrank aus dem Wandloch 3 (Dungeon 13) genommen ( */
extern signed char g_dng10_junk_found;                  /* ds:26c4 Dungeon 10: Krempel durchsucht (O000 dng10_junk) */
extern signed char g_dng16_plate_found;                 /* ds:26c5 Dungeon 16: Steinplatte im Schutt gefunden (O001) */
extern signed char g_dng16_plate_lifted;                /* ds:26c6 Dungeon 16: Steinplatte beiseite geschoben (O001) */
extern signed char g_dng16_pit_trap;                    /* ds:26c7 Dungeon 16: Falle in der Grube ausgeloest (O001) */
extern signed char g_dng16_casket_taken;                /* ds:26c8 Dungeon 16: Schatulle aus der Grube geborgen (O001) */
extern signed char g_dng16_double_key_state;            /* ds:26c9 Dungeon 16 (Ingerimms Schmiede): Tuer mit dem Doppelbartsch */
extern signed char g_dng10_vision_ap;                   /* ds:26ca Dungeon 10: AP fuer die Vision am Altar vergeben (O000) */
extern signed char g_dng11_lever_on;                    /* ds:26cb Dungeon 11: Hebel an 0x60d umgelegt, Bolzenfalle 2 gesperrt */
extern signed char g_dng11_plate_down;                  /* ds:26cc Dungeon 11: Bodenplatte 0x50a gedrueckt, Bolzenfalle 3 scha */
extern signed char g_dng11_trap1_found;                 /* ds:26cd Dungeon 11: Bolzenfalle 1 entdeckt (O061 dng11_bolt_trap_fo */
extern signed char g_dng11_trap2_found;                 /* ds:26ce Dungeon 11: Bolzenfalle 2 entdeckt (O061) */
extern signed char g_dng11_block_down;                  /* ds:26cf Dungeon 11: Fallblock hinter der Gruppe herabgesenkt (O061  */
extern signed char g_dng11_winch_state;                 /* ds:26d1 Dungeon 11: Kurbellager 0 leer, 1 Kurbel eingesetzt, 2 zwei */
extern signed char g_dng13_wheel_turned;                /* ds:26d2 Dungeon 13: Rad gedreht, Wasser fliesst (O065 dng13_wheel;  */
extern signed char g_dng14_plate_tried;                 /* ds:26d3 Dungeon 14: Platte am Beckengrund schon versucht (O066 dng1 */
extern signed char g_dng11_plate_seen;                  /* ds:26d4 Dungeon 11: Platte 0x1003 betreten (O062 dng11_plate) */
extern signed char g_dng11_trap3_found;                 /* ds:26d5 Dungeon 11: Bolzenfalle 3 entdeckt (O061) */
extern signed char g_dng14_plate_tries;                 /* ds:26d6 Dungeon 14: verbleibende Hebeversuche bis zum Werkzeughinwe */
extern signed char g_dng11_trap2_fired;                 /* ds:26d7 Dungeon 11: Bolzenfalle 2 ausgeloest (O061 dng11_bolt_trap) */
extern signed char g_dng11_trap3_fired;                 /* ds:26d8 Dungeon 11: Bolzenfalle 3 ausgeloest (O061 dng11_bolt_trap) */
extern signed char g_place083_attacked;                 /* ds:26d9 Ort 83: Bogenschuetzen angegriffen - beim naechsten Mal sof */
extern signed char g_binge_anvil_hours;                 /* ds:26da vermutet: Stundenzaehler fuer die Hammerschlaege in der Bin */
extern signed char g_dng11_wardrobe_seen;               /* ds:26dc Dungeon 11: Kleiderschrank des Kobolds gesehen (O062 dng11_ */
extern signed char g_dng14_waterskin_found;             /* ds:26dd Dungeon 15: magischen Wasserschlauch im Becken gefunden (O0 */
extern signed char g_dng14_plate_lifted;                /* ds:26de Dungeon 14: Platte mit dem Brecheisen gehoben, Treppe frei  */
extern signed char g_dng11_kobold_met;                  /* ds:26df Dungeon 11: Gespraech mit dem Kobold gefuehrt (O062 dng11_k */
extern signed char g_dng11_riddle_solved;               /* ds:26e0 Dungeon 11: Raetsel des Kobolds geloest, Silberschluessel e */
extern signed char g_mine_cart_pos;                     /* ds:26e1 Lorenfahrt in Dungeon 12: Fortschritt 2..16, -1 = Ende (O06 */
extern signed char g_dng16_plates_dir;                  /* ds:26e2 Dungeon 16: Richtung im Plattenkorridor (0 von Osten 0xe0b, */
extern signed char g_dng16_grid_moved;                  /* ds:26e3 Dungeon 16: Gitter des Wasserbeckens verschoben (O002) */
extern signed char g_dng16_hero_wet;                    /* ds:26e4 Dungeon 16: Held am Wasserbecken nass geworden (O002) */
extern signed char g_dng12_beams_done;                  /* ds:26e5 Dungeon 12: morsche Balken herabgestuerzt (O064) */
extern signed char g_dng12_beams_seen;                  /* ds:26e6 Dungeon 12: morsche Balken bemerkt (O064) */
extern signed char g_dng12_debris_ap;                   /* ds:26e7 Dungeon 12: AP fuer das Schuttwegraeumen vergeben (bis 3; O */
extern signed char g_ingerimm_verse_read;               /* ds:26e8 Gegenstand 0x12b entziffert: USE.LTX[106] "Alle Raeder steh */
extern signed char g_tev_lowangen_key_noted;            /* ds:26e9 Tagebuch zum Lowangen-Schluessel geschrieben (O125 place_ev */
extern signed char g_ambush_count;                      /* ds:26ea Zahl der Ueberfaelle durch Verfolger (O212 ambush_fig */
extern signed char g_dng10_chest_opened;                /* ds:26eb Dungeon 10: verrottete Truhe geoeffnet (O000 dng10_rotten_c */
extern signed char g_dng12_skeleton_found;              /* ds:26ec Dungeon 12: Kampf 0x74 gewonnen, Skelett gefunden (O063) */
extern signed char g_parchments_read;                   /* ds:26ed Pergamente 0x13a gelesen: USE.LTX[117]/[118] (Ornament mit  */
extern signed char g_topic_gavron;                      /* ds:26ee Gespraechsthema 0x30 "Gavron" frei (STDTALKS.LTX[194] "Wir  */
extern signed char g_dng16_chest_looted;                /* ds:26ef Dungeon 16: Schatztruhe leer gepluendert (O001) */
extern signed char g_dng04_village_thanked;             /* ds:26f0 Dungeon 4: Echsen nach dem Tod des Untiers erneut gesprochen */
extern signed char g_dng10_silver_key_in_lock;          /* ds:26f1 Dungeon 10: Silberschluessel steckt im Portalschloss (O060  */
extern signed char g_dng16_sword_done;                  /* ds:26f2 Dungeon 16: Golem fertig bzw. besiegt, Schwert uebergeben ( */
extern signed char g_finale_done;                       /* ds:26f3 Spielende gesehen (O035 Finale::run): R01 main_loop -> GAME */
extern signed char g_dng10_gold_key_in_lock;            /* ds:26f4 Dungeon 10: Goldschluessel steckt im Portalschloss (O060 dn */
extern signed char g_quest_anim_seen;                   /* ds:26f5 Zwischensequenz 0x0c gesehen (ItemUse::quest_anim O048: NSC */
extern signed char g_guest_lock_t2;                     /* ds:26f6 Schenke 2: Sperre der Person 0x93 in Stunden (O241; O005) */
extern signed char g_guest_lock_t19;                    /* ds:26f7 Schenken 19..22: Sperre der Person 0x94 (O241; O005) */
extern signed char g_guest_lock_t23;                    /* ds:26f8 Schenke 23: Sperre der Person 0x95 (O241; O005) */
extern signed char g_guest_lock_t24;                    /* ds:26f9 Schenke 24: Sperre der Person 0x96 (O241; O005) */
extern signed char g_dng14_in_pool;                     /* ds:26fa Dungeon 15: Gruppe ist im Becken (0x107/0x10c rein, 0x106/0 */
extern signed char g_dng12_stash28_empty;               /* ds:26fb Dungeon 12: Ablage 0x28 leer (O063) */
extern signed char g_dng11_ramp_falls;                  /* ds:26fc Dungeon 11: Stuerze von der Grubenrampe 0..4 (O062 dng11_ra */
extern signed char g_dng13_ambush_won;                  /* ds:26fd Dungeon 13: Kampf an 0x603 gewonnen (O065 dng13_ambush) */
extern signed char g_sites_night_done;                  /* ds:26fe Sonderort-Stadt: naechtliche Begegnung gesehen (O166 sites_ */
extern signed char g_dng16_plate_steps;                 /* ds:2700 Dungeon 16: Schritte auf den Gluehplatten (MU-Probe +9 - n; */
extern signed char g_dng16_sign_read;                   /* ds:2701 Dungeon 16: Tafel an der Ledertruhe gelesen (O001) */
extern signed char g_dng16_sacrilege;                   /* ds:2702 Dungeon 16: Frevel (Truhe gepluendert, Stein genommen, Gole */
extern signed char g_dng11_ramp_armed;                  /* ds:2703 Dungeon 11: Feld 0x905 betreten, an 0x906 folgt dng11_ramp_ */
extern signed char g_dng16_stone_taken;                 /* ds:2704 Dungeon 16: Salamanderstein genommen (O001) */
extern signed char g_dng16_forging;                     /* ds:2705 Dungeon 16: Golem schmiedet (O001/O002) */
extern signed char g_dng16_golem_fought;                /* ds:2706 Dungeon 16: gegen den Golem gekaempft (O001/O002) */
extern signed char g_dng16_medallion_given;             /* ds:2707 Dungeon 16: Steinmedaillon dem Golem gegeben (Dialog GOLEM2 */
extern signed char g_dng16_forge_timer;                 /* ds:2708 Dungeon 16: Schmiedezeit, je 5 Minuten -1 (R10; O002 setzt  */
extern signed char g_dng16_wall_timer;                  /* ds:2709 Dungeon 16: Wartezeit bis das Wandstueck (10, 11) wieder ge */
extern signed char g_dng16_artifact_traded;             /* ds:270a Dungeon 16: Asthenilschwert gegen den Salamanderstein getau */
extern signed char g_dng16_leather_taken;               /* ds:270b Dungeon 16: Lederzeug aus der Truhe genommen, Durchgang off */
extern signed char g_sites_explored;                    /* ds:270c Sonderorte: explore erledigt (O166; O257) */
extern signed char g_low_arrival_due;                   /* ds:270f Ankunftsszene in Lowangen steht an (O163 nach dem Orklager,  */
extern signed char g_dng08_shaman_room_shown;           /* ds:2710 Dungeon 8: Raum des Schamanen an 0x1907 beschrieben (BLUT3.L */
extern signed char g_dng08_mummy_fought;                /* ds:2711 Dungeon 8: Zwergenmumie aus dem Sarkophag besiegt (BLUT3.LTX */
extern signed char g_dng08_sarcophagus_opened;          /* ds:2712 Dungeon 8: Sarkophag geoeffnet - die Mumie erhebt sich (O267 */
extern signed char g_dng08_whirlweed_found;             /* ds:2713 Dungeon 8: Wirselkraut im Stroh an 0x1813 (BLUT3.LTX[48]; O2 */
extern signed char g_dng08_hall_0c15;                   /* ds:2714 Dungeon 8: Blick in die grosse Halle von 0xc15 (BLUT3.LTX[50 */
extern signed char g_dng08_pit_0e15;                    /* ds:2715 Dungeon 8: Klappgrube an 0xe15 (O267 setzt 1, dng08_pit; BLU */
extern signed char g_dng08_pit_armed;                   /* ds:2716 Dungeon 8: Klappgrube 0xe15 scharf (0xd15 setzt 1, 0xf15 0;  */
extern signed char g_dng08_collar_131c;                 /* ds:2717 Dungeon 8: goldenes Halsband im Stroh an 0x131c (BLUT3.LTX[5 */
extern signed char g_dng08_collar_0d10;                 /* ds:2718 Dungeon 8: goldenes Halsband im Stroh an 0xd10 (BLUT3.LTX[58 */
extern signed char g_dng08_orc_veteran_1907;            /* ds:2719 Dungeon 8: Kampf an 0x1907/0x190a gewonnen (O268) - erster  */
extern signed char g_dng08_orcs_1e19;                   /* ds:271a Dungeon 8: Kampf an 0x1e19 gewonnen (O268) - erster Gegner  */
extern signed char g_dng08_orcs_1e15;                   /* ds:271b Dungeon 8: Kampf an 0x1e15 gewonnen (O268) - erster Gegner  */
extern signed char g_dng08_orcs_1a11;                   /* ds:271c Dungeon 8: Kampf an 0x1a11 gewonnen (O268) - erster Gegner  */
extern signed char g_dng08_orcs_1516;                   /* ds:271d Dungeon 8: Kampf an 0x1516/0x1615 gewonnen (O268) - erster  */
extern signed char g_dng08_orcs_121b;                   /* ds:271e Dungeon 8: Kampf an 0x121b gewonnen (O268) - erster Gegner  */
extern signed char g_dng08_ogres_0e19;                  /* ds:271f Dungeon 8: Kampf an 0xe19 gewonnen (O268) - erster Gegner O */
extern signed char g_dng08_orcs_1113;                   /* ds:2720 Dungeon 8: Kampf an 0x1113 gewonnen (O268) - erster Gegner  */
extern signed char g_dng08_orcs_1110;                   /* ds:2721 Dungeon 8: Kampf an 0x1110 gewonnen (O268) - erster Gegner  */
extern signed char g_dng08_orcs_0d0f;                   /* ds:2722 Dungeon 8: Kampf an 0xd0f gewonnen (O268) - erster Gegner O */
extern signed char g_dng08_orcs_0f0c;                   /* ds:2723 Dungeon 8: Kampf an 0xf0c gewonnen (O268) - erster Gegner O */
extern signed char g_dng08_grain_1e0e;                  /* ds:2724 Dungeon 8: Kornsaecke an 0x1e0e angestochen (BLUT3.LTX[30..3 */
extern signed char g_dng08_grain_1e0f;                  /* ds:2725 Dungeon 8: Kornsaecke an 0x1e0f angestochen (O268) */
extern signed char g_dng08_grain_1e10;                  /* ds:2726 Dungeon 8: Kornsaecke an 0x1e10 angestochen (O268) */
extern signed char g_dng08_grain_1e0d;                  /* ds:2727 Dungeon 8: Kornsaecke an 0x1e0d angestochen (O268) */
extern signed char g_dng13_stash29_empty;               /* ds:2728 Dungeon 13: Ablage 0x29 leer (O065 dng13_ambush) */
extern signed char g_dng08_goulash_eaten;               /* ds:2729 Dungeon 8: Gulasch an 0x810 gegessen (BLUT3.LTX[60..65]; O26 */
extern signed char g_dng08_straw_0612;                  /* ds:272a Dungeon 8: Schnaps und Schwert im Stroh an 0x612 (BLUT3.LTX[ */
extern signed char g_dng08_hall_0415;                   /* ds:272b Dungeon 8: Blick in die grosse Halle von 0x415 (BLUT3.LTX[50 */
extern signed char g_dng08_trap_0213_defused;           /* ds:272c Dungeon 8: Bolzenfalle bei 0x213 entschaerft (BLUT3.LTX[79.. */
extern signed char g_dng08_lever;                       /* ds:272d Dungeon 8: Hebel umgelegt (wechselt; BLUT3.LTX[83..86]) - be */
extern signed char g_dng08_hall_fight;                  /* ds:272e Dungeon 8: Kampf in der grossen Halle gewonnen (Alarm; O268) */
extern signed char g_dng08_orcs_0511;                   /* ds:272f Dungeon 8: Kampf an 0x511/0x515 gewonnen (O268) - erster Ge */
extern signed char g_dng08_orcs_020d;                   /* ds:2730 Dungeon 8: Kampf an 0x20d.. gewonnen (O268) - erster Gegner */
extern signed char g_dng08_orcs_0309;                   /* ds:2731 Dungeon 8: Kampf an 0x309.. gewonnen (O268) - erster Gegner */
extern signed char g_dng02_druids_1c05;                 /* ds:2732 Dungeon 2: Kampf an 0x1c05 gewonnen (O130 dng02_fights) - e */
extern signed char g_dng02_cultists_0802;               /* ds:2733 Dungeon 2: Kampf an 0x802 gewonnen (O130) - erster Gegner K */
extern signed char g_dng02_cultists_0402;               /* ds:2734 Dungeon 2: Kampf an 0x402 gewonnen (O130) - erster Gegner K */
extern signed char g_dng02_cultists_020b;               /* ds:2735 Dungeon 2: Kampf an 0x20b gewonnen (O130) - erster Gegner K */
extern signed char g_dng02_cultists_070d;               /* ds:2736 Dungeon 2: Kampf an 0x70d gewonnen (O130) - erster Gegner K */
extern signed char g_dng02_cultists_041c;               /* ds:2737 Dungeon 2: Kampf an 0x41c gewonnen (O130) - erster Gegner K */
extern signed char g_dng02_sabretooth_0b17;             /* ds:2738 Dungeon 2: Kampf an 0xb17 gewonnen (O130) - erster Gegner S */
extern signed char g_dng02_sabretooth_0f0c;             /* ds:2739 Dungeon 2: Kampf an 0xf0c gewonnen (O130) - erster Gegner S */
extern signed char g_dng02_sabretooth_0f1c;             /* ds:273a Dungeon 2: Kampf an 0xf1c gewonnen (O130) - erster Gegner S */
extern signed char g_dng02_cave_spiders_150c;           /* ds:273b Dungeon 2: Kampf an 0x150c gewonnen (O130) - erster Gegner  */
extern signed char g_dng02_amulet1_timer;               /* ds:273c Dungeon 2: Amuletteil 1 vom Skelettkrieger an 0x130e (O130): */
extern signed char g_dng02_amulet2_timer;               /* ds:273d Dungeon 2: Amuletteil 2 (Skelettkrieger an 0x1e0d), wie g_dn */
extern signed char g_dng02_amulet3_timer;               /* ds:273e Dungeon 2: Amuletteil 3 (Skelettkrieger an 0x1617), wie g_dn */
extern signed char g_dng02_amulet4_timer;               /* ds:273f Dungeon 2: Amuletteil 4 (Skelettkrieger an 0x1e17), wie g_dn */
extern signed char g_dng02_cultists_171d;               /* ds:2740 Dungeon 2: Kampf an 0x171d gewonnen (O130) - erster Gegner  */
extern signed char g_dng02_cultists_1c1b;               /* ds:2741 Dungeon 2: Kampf an 0x1c1b gewonnen (O130) - erster Gegner  */
extern signed char g_dng02_step_toggle;                 /* ds:2742 Dungeon 2: wechselt bei jedem Schritt 0/1, die Amulett-Zaehl */
extern signed char g_jail_freed;                        /* ds:2743 Gruppe aus der Gefangenschaft befreit (O005 jail_tick -> Me */
extern signed char g_mugging_state;                     /* ds:2744 Ueberfall: 0 noch keiner, 1/2 Geschlecht schon dran, 3 alle  */
extern signed char g_gashok_arrested;                   /* ds:2745 in Gashok verhaftet: ALL_SECT.LTX[76] "Die Buettel schnuere */
extern signed char g_blackmage_done;                    /* ds:2746 Schwarzmagier-Szene erledigt: Salamanderstein abgegeben bzw */
extern signed char g_pending_msg;                       /* ds:2747 Meldung g_text_buf2 steht an (R12 view_frame: Klang 0xa5 + d */
extern signed char g_dng05_mummies_0e1d;                /* ds:2748 Dungeon 5: Kampf an 0xe1d gewonnen (O204 dng05_fights) - er */
extern signed char g_dng05_mummies_091f;                /* ds:2749 Dungeon 5: Kampf an 0x91f gewonnen (O204) - erster Gegner M */
extern signed char g_dng05_mummies_021e;                /* ds:274a Dungeon 5: Kampf an 0x21e gewonnen (O204) - erster Gegner M */
extern signed char g_dng05_priests_0819;                /* ds:274c Dungeon 5: Kampf an 0x819 gewonnen, danach dort Truhe 8 (O2 */
extern signed char g_dng05_priests_0a0e;                /* ds:274d Dungeon 5: Kampf an 0xa0e gewonnen (O204) - erster Gegner P */
extern signed char g_dng05_priests_0a0a;                /* ds:274e Dungeon 5: Kampf an 0xa0a gewonnen (O204) - erster Gegner P */
extern signed char g_dng05_skeletons_020e;              /* ds:2751 Dungeon 5: Kampf an 0x20e gewonnen (O204) - erster Gegner S */
extern signed char g_dng05_priests_1c0d;                /* ds:2753 Dungeon 5: Kampf an 0x1c0d gewonnen (O204) - erster Gegner  */
extern signed char g_dng05_priests_1501;                /* ds:2754 Dungeon 5: Kampf an 0x1501 gewonnen (O204) - erster Gegner  */
extern signed char g_sites_reward;                      /* ds:2755 Sonderorte: Gegenstand 0x129 nach dem Abendkampf erhalten ( */
extern signed char g_guide_count;                       /* ds:2756 Zahl der Fuehrer-Angebote (O164, hoechstens 5) */
extern signed char g_npc8_days;                         /* ds:2757 Tage mit Altrogolan (NSC 8) in der Gruppe (O005 timers_hour */
extern signed char g_gavron_chase;                      /* ds:2758 Verfolgung Gavrons in Lowangen: Stufe 1..8, < 0 vorbei (O185 */
extern signed char g_low_bridge_y;                      /* ds:275a Lowangen: Zeile vor der Bruecke fuer das Umdrehen beim Brue */
extern signed char g_dng04_mage_killed;                 /* ds:275b Dungeon 4: Zauberer im Turm besiegt (FIGHT_ID_SPECIAL2), dan */
extern signed char g_dng07_prison_event;                /* ds:275c Gefangenschaft in Dungeon 7: R12 ruft dng07_prison_day bei 1 */
extern signed char g_dng07_prison_days;                 /* ds:275d Dungeon 7: Tage in der Gefaengniszelle 1..6 (O264 dng07_pris */
extern signed char g_dng07_talk1_done;                  /* ds:275e Dungeon 7: Gespraech BLUT.TLK 0 an der Zelle gefuehrt (O264) */
extern signed char g_dng07_talk2_done;                  /* ds:275f Dungeon 7: Gespraech BLUT.TLK 1 gefuehrt (O264) */
extern signed char g_low_dragan_task3;                  /* ds:2760 Lowangen: dritter Auftrag Dragans offen (> 0; O031, O259) */
extern signed char g_dng04_monster_fought;              /* ds:2761 Dungeon 4: Kampf gegen das Untier (0x72) an 0x1c10/0x1c11 ge */
extern signed char g_finster_refuse_msg;                /* ds:2762 Zwerg weist die Gruppe an der Tuer ab (O019 setzt bei Freve */
extern signed char g_dng01_grate_first;                 /* ds:2763 Dungeon 1: Fallgitter ist zum ersten Mal hinter der Gruppe h */
extern signed char g_dng01_hooks_held;                  /* ds:2764 Dungeon 1: Fallgitter - 1 ein Held (g_dng01_hook_hero) haelt */
extern signed char g_dng01_skull_state;                 /* ds:2765 Dungeon 1: Totenkopf-Ornament am Gitter (FINAL.LTX[75..84]): */
extern signed char g_tjolmar_vault;                     /* ds:2766 Gewoelbe unter Tjolmar: 1 betreten (O251; R12 -> area_event_ */
extern signed char g_dng01_thorn_tree_state;            /* ds:2767 Dungeon 1: Baeumchen mit Dornen (FINAL.LTX[86..92]): 1 gesto */
extern signed char g_dng01_branch_state;                /* ds:2768 Dungeon 1: Wand mit Aesten (FINAL.LTX[93..97]): 1..3 Aeste i */
extern signed char g_dng01_cultists_0806;               /* ds:2769 Dungeon 1: Kampf an 0x806/0x807 gewonnen (auch durch die Go */
extern signed char g_dng01_cultists_0e06;               /* ds:276a Dungeon 1: Kampf an 0xe06/0xe07 gewonnen (O132) - erster Ge */
extern signed char g_dng01_troll_fought;                /* ds:276b Dungeon 1: angeketteter Troll auf der Truhe an 0x106 besiegt */
extern signed char g_dng01_ice_state;                   /* ds:276c Dungeon 1: Brunnen mit Eisblock und Kriegerin (FINAL.LTX[110 */
extern signed char g_dng01_hesinde_done;                /* ds:276d Dungeon 1: Opfer an der Hesinde-Statue gebracht (FINAL.LTX[1 */
extern signed char g_dng01_ghouls_fought;               /* ds:276e Dungeon 1: Kampf gegen die drei Ghule gewonnen (FINAL.LTX[11 */
extern signed char g_dng01_illusion_wall2;              /* ds:276f Dungeon 1: Illusionswand an 0x1c0f durchschaut (FINAL.LTX[12 */
extern signed char g_dng01_illusion_wall1;              /* ds:2770 Dungeon 1: Illusionswand an 0x1811 vor der schwarzen Statue: */
extern signed char g_dng01_cave_spiders_1808;           /* ds:2771 Dungeon 1: Kampf an 0x1808.. gewonnen (O132) - erster Gegne */
extern signed char g_phex_finale_done;                  /* ds:2773 Phex-Finale (O007 phex_finale) gesehen: Tiefhusen (Stadt 5) */
extern signed char g_dng05_poltergeist;                 /* ds:2774 Dungeon 5: Poltergeist der Schatzkammer (Truhe 0x30, PHEXTEM */
extern signed char g_dng05_skeletons_1214;              /* ds:2775 Dungeon 5: Kampf an 0x1214 gewonnen (O204) - erster Gegner  */
extern signed char g_dng05_skeletons_1a1d;              /* ds:2776 Dungeon 5: Kampf an 0x1a1d gewonnen (einer von vier vor der */
extern signed char g_dng05_skeletons_1b1e;              /* ds:2777 Dungeon 5: Kampf an 0x1b1e gewonnen (O204) - erster Gegner  */
extern signed char g_dng05_skeletons_1c1d;              /* ds:2778 Dungeon 5: Kampf an 0x1c1d gewonnen (O204) - erster Gegner  */
extern signed char g_dng05_skeletons_1b1c;              /* ds:2779 Dungeon 5: Kampf an 0x1b1c gewonnen (O204) - erster Gegner  */
extern signed char g_dng05_wall_1a1b_open;              /* ds:277a Dungeon 5: nach den vier Kaempfen um 0x1b1d Wand (0x1a, 0x1b */
extern signed char g_lock_trap_off;                     /* ds:277b Falle der Schloesser entschaerft (O108 lock_trap: dann nur T */
extern signed char g_dng08_copper_disc_taken;           /* ds:277c Dungeon 8: Kupferscheibe aus dem Kaestchen mit der Mumienhan */
extern signed char g_dng06_orc_veteran_1d02;            /* ds:277d Dungeon 6: Kampf an 0x1d02 gewonnen (O261 dng06_fights) - e */
extern signed char g_dng06_orcs_1503;                   /* ds:277e Dungeon 6: Kampf an 0x1503 gewonnen (O261) - erster Gegner  */
extern signed char g_dng06_orcs_0f03;                   /* ds:277f Dungeon 6: Kampf an 0xf03 gewonnen (O261) - erster Gegner O */
extern signed char g_dng06_orcs_0402;                   /* ds:2780 Dungeon 6: Kampf an 0x402 gewonnen (O261) - erster Gegner O */
extern signed char g_dng06_orcs_1009;                   /* ds:2781 Dungeon 6: Kampf an 0x1009 gewonnen (O261) - erster Gegner  */
extern signed char g_dng07_brandy_taken;                /* ds:2782 Dungeon 7: Schnapsflasche im Stroh an 0x150a genommen (BLUT2 */
extern signed char g_dng08_latrine_armed;               /* ds:2783 Dungeon 8: Latrine beim Weitergehen beschreiben (0x181b/0x81 */
extern signed char g_dng03_thief_pending;               /* ds:2784 Dungeon 3: Treppe von Dungeon 2 genommen (O130 setzt 1), R12 */
extern signed char g_dng03_sorcerer_0e0c;               /* ds:2785 Dungeon 3: Kampf an 0xe0c gewonnen (O131) - erster Gegner Z */
extern signed char g_dng03_stone_stolen;                /* ds:2786 Dungeon 3: der Mann in lila Aura hat den Salamanderstein gen */
extern signed char g_final_fight_won;                   /* ds:2787 Endkampf FIGHT_ID_SPECIAL in Dungeon 3 gewonnen (O131: Stein */
extern signed char g_dng01_cave_spiders_0101;           /* ds:2788 Dungeon 1: Kampf an 0x101.. gewonnen (O132) - erster Gegner */
extern signed char g_dng06_straw_1502;                  /* ds:2789 Dungeon 6: Strohlager an 0x1502 durchsucht (11 Silbertaler,  */
extern signed char g_dng06_straw_0d02;                  /* ds:278a Dungeon 6: Strohlager an 0xd02 durchsucht (1 Dukaten, 8 Silb */
extern signed char g_dng06_straw_100b;                  /* ds:278b Dungeon 6: Ogerstroh an 0x100b - Saebel genommen (BLUT1.LTX[ */
extern signed char g_dng06_straw_0a08;                  /* ds:278c Dungeon 6: Strohlager an 0xa08 - Arbach (BLUT1.LTX[45]; O262 */
extern signed char g_dng06_straw_0b08;                  /* ds:278d Dungeon 6: Strohlager an 0xb08 - orkischer Schmuck (BLUT1.LT */
extern signed char g_dng06_straw_0f0a;                  /* ds:278e Dungeon 6: Strohlager an 0xf0a - Heiltrank (BLUT1.LTX[49]; O */
extern signed char g_sfight_seen;                       /* ds:278f Sonderkampf 0x493 schon einmal begonnen (O143 bei Flucht; O0 */
extern signed char g_dng06_ogre_weapons_smashed;        /* ds:2790 Dungeon 6: schwere Waffen der Oger an 0x1715 zerschlagen (BL */
extern signed char g_tev082_thicket_done;               /* ds:2791 Weg 82: Orientierungsprobe im Waldpfad bei Gashok erledigt  */
extern signed char g_dng07_was_prisoner;                /* ds:2792 Gefangenschaft im Orklager (Reise -2 -> Dungeon 7, O038) er */
extern signed char g_sfight_won;                        /* ds:2793 Sonderkampf 0x493 gewonnen (Sondergegner bei le_flee; start_ */
extern signed char g_low_tunnel_arrival;                /* ds:2794 Ankunft in Lowangen durch den Geheimgang (Reise -5, O038; E */
extern signed char g_travel_state;                      /* ds:2795 Reisezustand (-2, -7, 0, 6 ...) */
extern signed char g_low_brooch_taken;                  /* ds:2796 Lowangen: Brosche aus dem Schauhaus genommen (O031) */
extern signed char g_salamander_stolen;                 /* ds:2797 Salamanderstein vom Dieb im Dungeon 3 genommen (O131: 1; nac */
extern signed char g_dng11_air_counter;                 /* ds:2798 Dungeon 11: Schritte im abgeschlossenen Raum (5/12/18: Luft */
extern signed char g_reward_given;                      /* ds:2799 talk_reward andere Arten: wie g_reward2_given (Originalfehl */
extern signed char g_reward2_given;                     /* ds:279a talk_reward Art 2: hoher AP-Bonus nur, wenn schon gesetzt - */
extern char g_save_play_end[];                          /* ds:27c2 Ende des Blocks PLAY (O106) */
extern char g_save_glob[];                              /* ds:27c3 Anfang des Spielstandblocks GLOB (O106) */
extern signed char g_inn_room;                          /* ds:27c4 Herberge: gebuchtes Zimmer (-1 keins?; O090, O005 loescht) */
extern signed char g_group_sizes[];                     /* ds:27c5 Zahl der Helden je Gruppe (O200_0ACE, O202_0200/035A/0434) */
extern signed long g_god_favor[];                       /* ds:27cb Ansehen je Gottheit (O164 Spende; BE GODS_ESTIMATION) */
extern signed long g_god_favor_rondra;                  /* ds:27d3 = g_god_favor[2] (Rondra nach BE-Zaehlung, vermutet; eigene */
extern signed long g_god_favor_boron;                   /* ds:27df = g_god_favor[5] (Boron nach BE-Zaehlung: Toten begraben +4 */
extern signed long g_god_favor_hesinde;                 /* ds:27e3 = g_god_favor[6] (Hesinde, eigenes Public): ab 100 bietet di */
extern signed long g_god_favor_tsa;                     /* ds:27eb = g_god_favor[8] (Tsa nach BE-Zaehlung, vermutet; eigenes P */
extern signed long g_god_favor_ingerimm;                /* ds:27f7 = g_god_favor[11] (Ingerimm, eigenes Public): Opfer +, Frev */
extern signed char g_npc_months;                        /* ds:2807 Dauer der NSC-Begleitung (BE NPC_MONTHS; >= 99 / 0x7f: Absch */
extern signed short g_nr_groups;                        /* ds:2809 Zahl der belegten Gruppen (O075: hoechstens 6) */
extern signed short g_group_no;                         /* ds:280b Nummer der aktiven Gruppe (Vergleich mit hero.group_no, O076 */
extern signed short g_inn_days;                         /* ds:280d Herberge: gebuchte Tage (O005 zaehlt herunter, O090) */
extern signed short g_inn_town;                         /* ds:280f Herberge: Stadt der Buchung (group.town bzw. 4) (O178; O005  */
extern signed short g_inn_loc;                          /* ds:2811 Herberge: g_loc_id der Buchung (O178; O005 loescht mit g_inn */
extern signed short g_inn_price;                        /* ds:2813 Herberge: Gesamtpreis der Buchung (Storno erstattet ihn) (O1 */
extern signed short g_camp_food_mod;                    /* ds:2815 Lager: Zuschlag der Nahrungs-/Wassersuche (O071) */
extern signed short g_camp_herb_mod;                    /* ds:2817 Lager: Zuschlag der Kraeutersuche (O071) */
extern signed short g_rest_mod;                         /* ds:2819 Rastmodifikator (O072 Regeneration: > 90 -> -99; travel_spee */
extern char g_absent_hero1[16];                         /* ds:281b Heldenname (file_name) des ersten zurueckgelassenen Helden ( */
extern char g_absent_hero2[16];                         /* ds:282b Heldenname des zweiten zurueckgelassenen Helden */
extern signed long g_dng06_alarm_delay;                 /* ds:283b vermutet: Zeitzaehler im Orklager (O261: 0x1c2 nach dem Heu */
extern signed short g_loc_id;                           /* ds:283f Nummer des aktuellen Ortes in seiner Art (Laden, Schenke, Er */
extern signed short g_loc_name;                         /* ds:2841 Textnummer des Ortsnamens in den Ortstexten (txt[g_loc_name] */
extern signed long g_dng10_altar_coins;                 /* ds:2843 Dungeon 10: Muenzen auf dem Altar in Hellern (O000) */
extern signed long g_dng16_chest_coins;                 /* ds:2847 Dungeon 16: in die Schatztruhe gelegte Heller (O001) */
extern signed short g_exit_dir;                         /* ds:285f Blickrichtung nach dem Verlassen des Ortes (O030 Rueckkehr;  */
extern signed short g_house_hint_seen[];                /* ds:2861 Bewohner mit Rat schon besucht, je Eintrag von g_house_hints */
extern signed char g_area_type;                         /* ds:2883 Art der 3D-Ansicht: AREA_TOWN (1, O046 Stadt) / AREA_DUNGEON */
extern struct hero far *g_dng01_hook_hero;              /* ds:2884 Dungeon 1: Held, der die Haken im Totenkopf-Ornament gezogen */
extern char g_camp_job_names[6][16];                    /* ds:2888 Lagerauftraege: Dateinamen der Helden [0] Kraeuter, [1]/[2]  */
extern char g_camp_watch_names[3][16];                  /* ds:28b8 Lager: Dateinamen der drei Wachen (= g_camp_job_names[3], ei */
extern signed char g_camp_herb_hours;                   /* ds:28e8 Lager: Stunden der Kraeutersuche als Auftrag (O071) */
extern signed short g_dng05_offered_ducats;             /* ds:28e9 Dungeon 5: in den Opferstock gelegte Dukaten (ab 51 oeffnet  */
extern signed short g_dng05_poltergeist_ticks;          /* ds:28eb Dungeon 5: Zaehler der Klangdienst-Takte seit dem Erscheinen */
extern char g_save_glob_end[];                          /* ds:28ed Ende des Blocks GLOB (O106) */
extern char g_save_mons[];                              /* ds:28ee Anfang des Spielstandblocks MONS (O106) */
extern signed char g_mon_known[];                       /* ds:28ef Gegnerart schon einmal besiegt (AP nur noch first_ap / 10) */
extern char g_save_mons_end[];                          /* ds:296a Ende des Blocks MONS (O106) */
extern char g_save_locs[];                              /* ds:296b Anfang des Spielstandblocks LOCS (O106) */
extern struct smith_repair g_smith_repairs[];           /* ds:296c Gegenstand in Reparatur je Schmiede (9 Bytes; ohne eigenes  */
extern signed char g_temple_desecrated[];               /* ds:29d8 je Tempel (g_loc_id, 0x1d Stueck): entweiht (O044 temple_des */
extern signed char g_inn_closed[];                      /* ds:29f5 je Herberge (g_loc_id): geschlossen bzw. Hausverbot (O178; O */
extern signed char g_tavern_banned[];                   /* ds:2a10 je Schenke (g_loc_id): Hausverbot (O006) */
extern signed char g_smith_banned[];                    /* ds:2a2a Hausverbot je Schmiede (BE SMITH_KICKED_FLAGS; O230/O231; O */
extern signed char g_shop_banned[];                     /* ds:2a3f je Laden (g_loc_id): Hausverbot (O006, O226) */
extern signed char g_peddler_banned;                    /* ds:2a57 fahrender Haendler 1/2: Hausverbot nach gescheitertem Feilsc */
extern signed char g_peddler3_banned;                   /* ds:2a58 fahrender Haendler 3: Hausverbot (O069) */
extern signed char g_lodging_banned[5];                 /* ds:2a59 je Rasthaus (g_loc_id, 5 Stueck): Hausverbot (O070) */
extern signed char g_inn_has_room[];                    /* ds:2a5e je Herberge (g_loc_id 4..12): Zimmer frei (O178 inn_room) */
extern signed char g_inn_room_rolls[];                  /* ds:2a62 Tageswurf je Herberge 4..12: Zimmer frei (Chance aus INN.DA */
extern signed char g_shop_sells_food[];                 /* ds:2a63 je Laden (g_loc_id 8..12): Kraemer fuehrt Nahrung (shop_buil */
extern signed char g_shop_food_rolls[];                 /* ds:2a6b Tageswurf je Laden 8..12 (20 %): Kraemer fuehrt Nahrung (O0 */
extern signed char g_shed_free_today;                   /* ds:2a70 Tageswurf 60 %: Schuppen fuer 5 Silberlinge je Kopf frei (X */
extern signed char g_dragan_room_today;                 /* ds:2a71 Tageswurf 50 %: auf Dragans Empfehlung doch ein Zimmer (XDS */
extern struct month_roll g_market_rolls[4];             /* ds:2a72 Tageswuerfe der Maerkte (4 x 11 Bytes, Ende base == -1; O00 */
extern struct market g_markets[];                       /* ds:2a74 Marktdaten je Ort (g_loc_id): Aufschlag, Stufe, Haendlerin j */
extern char g_save_locs_end[];                          /* ds:2a9e Ende des Blocks LOCS (O106) */
extern signed short g_items_plural2[];                  /* ds:2a9f zweite Gegenstandsliste fuer die Mehrzahl (loose_random_item */
extern signed short g_items_plural[];                   /* ds:2ad9 Gegenstaende mit Pluralform (Liste fuer is_in_word_array; gr */
extern signed short g_forbidden_items_01[];             /* ds:2af3 nicht benutzbare Gegenstaende JUGGLER (Ende -1; O032) */
extern signed short g_forbidden_items_02[];             /* ds:2b2f nicht benutzbare Gegenstaende HUNTER (Ende -1; O032) */
extern signed short g_forbidden_items_03[];             /* ds:2b6f nicht benutzbare Gegenstaende WARRIOR (Ende -1; O032) */
extern signed short g_forbidden_items_04[];             /* ds:2b91 nicht benutzbare Gegenstaende ESTRAY (Ende -1; O032) */
extern signed short g_forbidden_items_05[];             /* ds:2bcf nicht benutzbare Gegenstaende THORWALIAN (Ende -1; O032) */
extern signed short g_forbidden_items_06[];             /* ds:2bfd nicht benutzbare Gegenstaende DWARF (Ende -1; O032) */
extern signed short g_forbidden_items_07[];             /* ds:2c27 nicht benutzbare Gegenstaende WITCH (Ende -1; O032) */
extern signed short g_forbidden_items_08[];             /* ds:2ccb nicht benutzbare Gegenstaende DRUID (Ende -1; O032) */
extern signed short g_forbidden_items_09[];             /* ds:2d7b nicht benutzbare Gegenstaende MAGE (Ende -1; O032) */
extern signed short g_forbidden_items_10[];             /* ds:2e33 nicht benutzbare Gegenstaende GREEN_ELF (Ende -1; O032) */
extern signed short g_forbidden_items_11[];             /* ds:2e6f nicht benutzbare Gegenstaende ICE_ELF (Ende -1; O032) */
extern signed short g_forbidden_items_12[];             /* ds:2ead nicht benutzbare Gegenstaende SYLVAN_ELF (Ende -1; O032) */
extern signed short far *g_forbidden_items[];           /* ds:2ee7 = g_forbidden_item_lists - 1 (MAP-Public des 1-basierten Zug */
extern signed short far *g_forbidden_item_lists[12];    /* ds:2eeb je Heldentyp 1..12 Liste der nicht benutzbaren Gegenstaende  */
extern struct ranged_weapon g_ranged_weapons_table[];   /* ds:2f1b Fernwaffen: Zuschlaege je Entfernung (BE RANGED_WEAPONS_TABL */
extern struct weapon_desc g_weapons_table[78];          /* ds:2f7b Waffentabelle (BE WEAPONS_TABLE; 78 x 8 Bytes) */
extern signed char g_weapons_bf[];                      /* ds:2f7e = &g_weapons_table[0].bf (eigenes Public; Schrittweite 8) */
extern struct weapon_atpa g_weapons_atpa[];             /* ds:2f80 = &g_weapons_table[0].at_mod (eigenes Public) */
extern struct weapon_class g_weapons_class[];           /* ds:2f82 Waffentabelle (8 Bytes je Eintrag, Index item_desc.table_ind */
extern struct armor_desc g_armors_table[];              /* ds:31eb Ruestungstabelle (BE ARMORS_TABLE; RS, BE) */
extern struct usable_desc g_usable_table[];             /* ds:3222 Tabelle der benutzbaren Gegenstaende je table_index (Ladunge */
extern signed short g_poison_potions[];                 /* ds:325d Gifttraenke (nicht fuer NSC; O253), Ende -1 (BE POISON_POTIO */
extern signed short g_items_ask_use[];                  /* ds:3271 Kraeuter/Traenke mit Rueckfrage verzehren/benutzen (O047, d */
extern signed short g_herbs_toxic[];                    /* ds:327f giftige Kraeuter (O253 txt[34]), Ende -1 (BE HERBS_TOXIC) */
extern signed short g_herbs_uneatable[];                /* ds:3289 nicht essbare Kraeuter (O253 txt[33]), Ende -1 (BE HERBS_UNE */
extern signed short g_elixir_potions[];                 /* ds:3297 Elixiere: Eigenschaft (Index) + 5 (O253), Ende -1 (BE ELIXIR */
extern signed short g_bad_elixirs[];                    /* ds:32a7 schlechte Elixiere: Eigenschaft - 7 (O253), Ende -1 (BE BAD_ */
extern signed short g_items_attack[];                   /* ds:32b7 Angriffsgegenstaende (Wurfwaffen, Bomben ...): im Kampf mit */
extern signed short g_teleport_spin;                    /* ds:32c5 Drehzaehler der Teleport-Animation (O129 setzt 0, SEG023 vi */
extern signed short g_vis_start;                        /* ds:32c7 Startindex in der Sichtliste g_vis_order (0, Dungeon 0x10; O */
extern signed short g_ems_cur_page;                     /* ds:32c9 zuletzt eingeblendete logische EMS-Seite (R02 EMS_map4) */
extern char far *g_file_names[14];                      /* ds:32cb Archivdateinamen: [0] "STARCD.DAT", [2] "SPEECHCD.DAT", [4]  */
extern char far *g_fmt_cd_path;                         /* ds:32db Zeiger auf "X:\DATA\%s" (Ausweichpfad, R06) */
extern char far *g_fmt_temp_path;                       /* ds:32df Zeiger auf "..\CTEMP\%s" (Ausweichpfad, R06) */
extern struct rect g_clip_rect;                         /* ds:3303 Clipping-Rechteck der Bildausgabe (R18 setzt es zum Zeichnen */
extern signed short g_clip_y0;                          /* ds:3305 = g_clip_rect.y0 (eigenes Public; O112 fig_anim_clip) */
extern signed short g_clip_x1;                          /* ds:3307 = g_clip_rect.x1 (eigenes Public) */
extern signed short g_clip_y1;                          /* ds:3309 = g_clip_rect.y1 (eigenes Public) */
extern signed short g_mouse_min_x;                      /* ds:330b Mausbereich links (O075_0B50: 1, sonst 0) */
extern struct mouse_range g_mouse_range;                /* ds:330b Bewegungsbereich der Maus als Ganzes (O007, O199 sichern/set */
extern signed short g_mouse_min_y;                      /* ds:330d Mausbereich oben (O075_0B50: 0x9d, sonst 0) */
extern signed short g_mouse_max_x;                      /* ds:330f groesstes x des Mausbereichs (Bildschirmbreite; R23 begrenzt */
extern signed short g_mouse_max_y;                      /* ds:3311 groesstes y des Mausbereichs (ds:330d = kleinstes y) */
extern char g_cursor_bmp[];                             /* ds:3313 Bmp-Objekt des Mauszeigerbilds (R18 mouse_set_cursor, mouse_ */
extern char g_cursor_bg_bmp[];                          /* ds:331d Bmp-Objekt des Hintergrunds unter dem Mauszeiger (R18 mouse_ */
extern struct Bmp g_bmp_mark_le;                        /* ds:3327 Bmp: Rahmenbild ueber dem Portraet bei LE-Verlust (NVF 0x49  */
extern struct Bmp g_bmp_mark_ae;                        /* ds:3331 Bmp: Rahmenbild ueber dem Portraet bei AE-Verlust (NVF 0x49  */
extern struct point g_gui_buttons_pos[9];               /* ds:3341 Lage der 9 Symbolfelder (BE GUI_BUTTONS_POS; Screen::draw_ic */
extern char g_gui_buttons_pos_y[];                      /* ds:3343 = &g_gui_buttons_pos[0].y (eigenes Public); +0x22 = ds:3365  */
extern struct mouse_area g_fig_mouse_areas[];           /* ds:347d Mausbereiche des Kampfbildschirms (kein eigenes Public: ds:3 */
extern signed short g_portrait_x[];                     /* ds:3491 x-Positionen der Portraets in der Heldenleiste (7 Worte) */
extern struct mouse_area g_dlg_areas[];                 /* ds:349f Mausbereiche des Dialogfensters (Titelleiste, Zahleneingabe; */
extern struct mouse_area g_action_table_merchant[];     /* ds:352b Mausbereiche der Laeden (Kaufen/Verkaufen, -> g_mouse_areas2 */
extern signed short g_disease_prices[];                 /* ds:353f Zuschlag Heilkunde je Krankheit (BE DISEASE_PRICES; O090 lie */
extern signed short g_disease_delays[];                 /* ds:354f je Krankheit: Modifikator der Heilerprobe (BE DISEASE_DELAYS */
extern signed short g_poison_prices[];                  /* ds:355f Zuschlag Heilkunde je Gift (BE POISON_PRICES; O090 liest Byt */
extern signed short g_poison_delays[];                  /* ds:3573 je Gift: Modifikator der Heilerprobe (BE POISON_DELAYS; O174 */
extern char far *g_path_fmts[3];                        /* ds:3587 Pfadformate: [0] "..\TEMP\%s", [1] "..\GAMES\%s", [2] "\DATA */
extern char g_dsa_version_string[];                     /* ds:3593 "DSA VERSION" Kennung der Spielstaende und Helden (DSA 1 gl */
extern signed char g_version_a;                         /* ds:35a5 Versionsbyte (Spielstandkopf zuletzt; hero.ver[0]) */
extern signed char g_version_b;                         /* ds:35a6 Versionsbyte (hero.ver[2]) */
extern signed char g_version_c;                         /* ds:35a7 Versionsbyte, beim Laden verglichen (hero.ver[1]; vermutet  */
extern signed char g_version_d;                         /* ds:35a8 Versionsbyte (Spielstandkopf zuerst; hero.ver[3]) */
extern signed short g_portrait_marks[];                 /* ds:35a9 Markierung je Portraet (O201_0C25: Gegenstand benutzbar) */
extern char g_file_main[];                              /* ds:35c6 Dateiobjekt Hauptarchiv (class File, O037) */
extern char g_file_3[];                                 /* ds:35fe Archivobjekt (struct arc, R06 arc_*; Bilder/NVF in O098/O188 */
extern char g_file_fx[];                                /* ds:365a Dateiobjekt FX.DAT (g_file_names[8]/[9], Klaenge; O032, O003 */
extern char g_arc_fx[];                                 /* ds:3692 Archivobjekt zu FX.DAT (struct arc, R06; Klaenge, O018 arc_m */
extern char g_file_speech[];                            /* ds:36ee Dateiobjekt Sprachausgabe (Nummern <= 1000, R04 snd_speech) */
extern char g_file_raw[];                               /* ds:3726 Dateiobjekt RAW.DAT (g_file_names[12]/[13]; Texturbilder der */
extern char g_file_cd[];                                /* ds:375e Dateiobjekt STARCD.DAT (g_file_names[0]; FLI-Animationen O02 */
extern char g_file_speech_ext[];                        /* ds:3796 Dateiobjekt zweite Sprachdatei (Nummern > 1000, R04) */
extern char g_cd_cache[];                               /* ds:37ce Festplatten-Cache fuer Dateien von der CD (Klasse Cache, O00 */
extern char g_font[];                                   /* ds:37e4 Schrift-/Textobjekt (class Font : struct font, O248/R20) */
extern void far *g_font_dst;                            /* ds:37f4 Zielbild der Schrift g_font (+0x10; O094 sichert es, O026/O */
extern signed short g_font_tab0;                        /* ds:381f = g_font.tabs[0] (eigenes Public; Dlg::layout merkt d */
extern char g_snd_music[];                              /* ds:382d Klangobjekt Musik (CD-Musik ueber O004) */
extern struct fight_def g_fight_defs[];                 /* ds:3851 = g_fight_defs_ab - 0xab (gefaltete Basis fuer den Index Kam */
extern signed short g_music_off;                        /* ds:387a nie gesetzt (immer 0): CD-Musik abgeschaltet (O035 Finale:: */
extern signed short g_music_ready;                      /* ds:387c Musik eingerichtet (O032 nach SndSetup); O035 wartet dann a */
extern signed short g_music_id_req;                     /* ds:3892 angeforderte Musiknummer = Feld id_req (+0x65) von g_snd_mus */
extern char g_snd_fx[];                                 /* ds:38b6 Klangobjekt Klangeffekte (R04 snd_play) */
extern signed short g_speech_off;                       /* ds:3903 nie gesetzt (immer 0): Sprachausgabe abgeschaltet (O042 Spe */
extern signed short g_sound_ready;                      /* ds:3905 Klang eingerichtet (O032 nach SndSetup); O072 Wecken mit Kl */
extern signed short g_speech_pos;                       /* ds:391b Sprachausgabe: Position/Marke (& 0x3fff, Speech::get_pos O0 */
extern signed short g_sheet_enabled;                    /* ds:393f Heldenbogen erlaubt (O009/O095 nur bei != 0); Startwert 1,  */
extern signed short g_portrait_drag;                    /* ds:3941 Portraets mit der Maus verschiebbar (R02 handle_input; O068/ */
extern signed short g_paused;                           /* ds:3945 Pause aktiv (R18; R02: keine Optionen-Taste) */
extern signed short g_temple_visited;                   /* ds:3947 Tempel besucht (O044: 0 im Temple-Konstruktor, 1 nach templ */
extern signed short g_temple_visited2;                  /* ds:3949 wie g_temple_visited (O044); wird nie gelesen */
extern signed short g_game_state;                       /* ds:394b Spielzustand (7/8 nach Tod, 0x63, 2 = ...) */
extern signed long g_fight_xms;                         /* ds:394f XMS- bzw. EMS-Handle des Kampf-Hintergrunds (O018, O022, O14 */
extern signed short g_input_preset;                     /* ds:3953 Texteingabefeld zeigt den uebergebenen Text als Vorgabe (nur */
extern signed char g_hero_excluded;                     /* ds:3955 Platz eines Helden, der bei der Heldenwahl ausgelassen wird  */
extern signed char g_sheet_hunger_shown;                /* ds:3956 Heldenbogen: angezeigter Hunger (Bars_tick; 100 = blinkt, -1 */
extern signed char g_sheet_thirst_shown;                /* ds:3957 Heldenbogen: angezeigter Durst (Bars_tick) */
extern signed short g_dialog_depth;                     /* ds:3958 Schachtelungszaehler (Pause, Menue: ++/--); bei != 0 laufen  */
extern signed char g_loaded_dungeon;                    /* ds:395a geladener Dungeon der 3D-Ansicht (O014; -1 = neu laden, O038 */
extern signed char g_loaded_town;                       /* ds:395b geladene Stadt der 3D-Ansicht (O046; -1 = neu laden, O038/O0 */
extern signed char g_fight_screen_on;                   /* ds:395d Kampfbildschirm aktiv (Figuren animiert; 0 im Schnellkampf)  */
extern signed char g_fight_flag;                        /* ds:395e Kampfmerker */
extern signed short g_fight_end;                        /* ds:395f Kampfende-Merker (3/4/5) */
extern signed long g_timer_ticks;                       /* ds:3961 seit dem letzten do_timers vergangene Ticks (R21 time_tick + */
extern signed long g_timer_minutes;                     /* ds:3965 vergangene Minuten (90 Ticks; R21) */
extern signed long g_timer_5min;                        /* ds:3969 vergangene 5-Minuten-Einheiten (450 Ticks; R21) */
extern signed long g_timer_15min;                       /* ds:396d vergangene Viertelstunden (1350 Ticks; R21) */
extern signed long g_timer_hours;                       /* ds:3971 vergangene Stunden (5400 Ticks; R21) */
extern signed long g_timer_4hours;                      /* ds:3975 vergangene 4-Stunden-Einheiten (21600 Ticks; R21) */
extern signed long g_timer_days;                        /* ds:3979 vergangene Tage (R21 Tageswechsel) */
extern signed long g_timer_months;                      /* ds:397d vergangene Monate (R21) */
extern signed long g_timer_years;                       /* ds:3981 vergangene Jahre (R21) */
extern signed short g_at_1000;                          /* ds:3985 Merker 10 Uhr (R21; R10: Rausch ausschlafen) */
extern signed short g_at_0800;                          /* ds:3987 Merker 8 Uhr (R21; R10 timers_events Tagesereignisse) */
extern signed short g_at_1400;                          /* ds:3989 Merker 14 Uhr (R21; R10 loescht ihn ausser bei Gruppe.f20 == */
extern signed short g_at_2000_2200;                     /* ds:398b 1 zwischen 20 und 22 Uhr (R21) */
extern signed short g_at_0600_0900;                     /* ds:398d 1 zwischen 6 und 9 Uhr (R21) */
extern unsigned short g_idle_ticks;                     /* ds:398f Leerlaufzaehler fuer den Bildschirmschoner (O224 Tm::tick, R */
extern signed short g_saver_due;                        /* ds:3991 Bildschirmschoner faellig (O224 Tm::tick; R02 screensaver_ru */
extern unsigned char g_loc_herb;                        /* ds:3993 Kraut der Gegend: Fundchance +10 %, Hoechstzahl +1 (O071 cam */
extern signed short g_resting;                          /* ds:3994 Gruppe ruht/schlaeft (O000 Bewusstlosigkeit, O072 Lager, O1 */
extern signed short g_leave_location;                   /* ds:3996 Ort sofort verlassen: beendet die Schleifen von Laden, Temp */
extern signed short g_speech_ext_avail;                 /* ds:3998 zweite Sprachdatei vorhanden (Sprachnummern > 1000, R04) */
extern signed short g_cd_music;                         /* ds:399a Musik von CD (R04: snd_play/snd_service gehen an g_cd) */
extern signed long g_disk_free;                         /* ds:399c freier Plattenplatz beim Start (O032 main) */
extern signed short g_mem_too_low;                      /* ds:39a4 zu wenig XMS/EMS-Speicher: Start abbrechen (R05 Init_ctor/sw */
extern signed short g_first_start;                      /* ds:39a6 Spielstand LASTGAME fehlt (erster Start: Vorspann und Spielm */
extern signed long g_disk_needed;                       /* ds:39a8 benoetigter freier Plattenplatz (10 MB; O032 main) */
extern char g_str_temp_dir[];                           /* ds:39ac "\..\TEMP" (O032 prepare_dirs) */
extern char g_str_files_low[];                          /* ds:3a59 Meldung FILES zu klein (O032 main) */
extern char g_str_version[];                            /* ds:3aa5 Versionszeile "Sternenschweif Version C1.02" (O032 main) */
extern char g_str_lastgame_3ac4[];                      /* ds:3ac4 "LASTGAME" (O032 main) */
extern char g_str_lastgame_3acd[];                      /* ds:3acd "LASTGAME" (O032 main) */
extern char g_str_gam_3ad6[];                           /* ds:3ad6 ".GAM" (O032 main: abschneiden) */
extern char g_str_gam_3adb[];                           /* ds:3adb ".GAM" (O032 main: anhaengen) */
extern char g_str_lastgame_3ae0[];                      /* ds:3ae0 "LASTGAME" (O032 main: savegame_read) */
extern char g_str_mem_low1[];                           /* ds:3ae9 Meldung zu wenig Speicher, Zeile 1 (O032 init_memory) */
extern char g_str_mem_low2[];                           /* ds:3afc Meldung zu wenig Speicher, Zeile 2 */
extern char g_str_mem_low3[];                           /* ds:3b21 Meldung zu wenig Speicher, Zeile 3 */
extern char g_str_mem_low4[];                           /* ds:3b58 Meldung zu wenig Speicher, Zeile 4 */
extern char g_str_mem_low5[];                           /* ds:3b9b Meldung zu wenig Speicher, Zeile 5 */
extern char g_str_mem_low6[];                           /* ds:3be2 Meldung zu wenig Speicher, Zeile 6 */
extern char g_str_mem_low7[];                           /* ds:3c29 Meldung zu wenig Speicher, Zeile 7 */
extern char g_str_drive_root[];                         /* ds:3c55 "X:\" (O032 prepare_dirs: Laufwerk eingesetzt) */
extern char g_str_all_3c59[];                           /* ds:3c59 "*.*" (O032 prepare_dirs) */
extern signed char g_temple_miracle_bonus[];            /* ds:3c5e je Gott: Bonus fuer Wunder (x Ansehen / 100) (O033) */
extern signed char g_temple_miracle_dice[];             /* ds:3c6d je Gott: Wuerfel fuer Wunder (Maximum = Wiederbelebung) (O03 */
extern signed char g_temple_god_anim[];                 /* ds:3c7c je Gott: Wunderanimation 0x7e (0) bzw. 0x7f (1), Bild 0x6c/0 */
extern signed short g_text_block;                       /* ds:3c8c zuletzt geladener Textblock (load_text_block), -1 */
extern struct hero_ptrs7 g_hero_ptrs_null;              /* ds:3c8e sieben Nullzeiger als Vorlage (O034 group_scatter) */
extern struct hero_ptrs7 g_hero_ptrs_null2;             /* ds:3caa sieben Nullzeiger als Vorlage (O034 group_scatter, zweiter Z */
extern struct mouse_area g_sel_areas[];                 /* ds:3cc6 Auswahlbild: 18 Mausbereiche (Aktion 1..18) */
extern signed short g_options_shown;                    /* ds:3d8e Optionen-Menue wurde gezeigt (R02 handle_options_key) */
extern signed short g_cl_finale[];                      /* ds:3d90 Dateiliste der Spielende-Sequenz (FLI 0xa013..0xa01b, Ende - */
extern signed short g_cl_finale_speech[];               /* ds:3da6 Dateiliste der Spielende-Sequenz, Sprachausgabe (0xc415..0xc */
extern unsigned char g_cl_finale_st[];                  /* ds:3db8 Zustaende zu den Dateilisten der Spielende-Sequenz (O035) */
extern char g_str_lastgame_3dbc[];                      /* ds:3dbc "LASTGAME" (O035 Finale::run loescht diesen Spielstand) */
extern struct finale_step g_finale_script[];            /* ds:3dc6 Skript der Spielende-Sequenz bis FINALE_OP_END (O036; ohne e */
extern signed short g_file_last_id;                     /* ds:3e74 zuletzt geoeffnete Datei-id (File::open/load, O037) */
extern char g_str_star_dat_o037[];                      /* ds:3e76 "STAR.DAT" (File::is_loose: Bitfeld nur, wenn die Datei exis */
extern char g_str_dummy_o037[];                         /* ds:3e7f "DUMMY" (File::exists: Platzhalter-Namen gelten als fehlend) */
extern char g_tmap_areas[];                             /* ds:3e86 Mausbereichsliste der Reisekarte (travel_enter -> g_mouse_ar */
extern char g_cl_travel[];                              /* ds:3eea Dateiliste der Reisekarte fuer load_pic_list (Ende -1) */
extern char g_cl_travel_st[];                           /* ds:3f50 Zustandsbits zur Dateiliste g_cl_travel */
extern char g_cl_intro[];                               /* ds:3f5e Dateiliste des Vorspanns (O039) fuer load_pic_list */
extern char g_cl_intro_speech[];                        /* ds:3f7c Dateiliste des Vorspanns mit Sprachausgabe (bei g_cd_speech */
extern char g_cl_intro_st[];                            /* ds:3f8a Zustandsbits zu g_cl_intro/g_cl_intro_speech */
extern struct intro_scene g_intro_scenes[];             /* ds:3f8e Szenen des Vorspanns {Animation, Bild in 0x346} (O039, 12) */
extern char g_intro_empty[];                            /* ds:3fbe "" (O039: Szenen ohne Untertitel) */
extern char g_intro_blank[];                            /* ds:3fbf "" (O039: Untertitel loeschen) */
extern char g_cl_anim3[];                               /* ds:3fc0 Dateiliste der Bildfolge Anim3 (O040) fuer load_pic_list */
extern char g_cl_anim3_speech[];                        /* ds:3fe0 Dateiliste Anim3 mit Sprachausgabe (bei g_cd_speech) */
extern char g_cl_anim3_st[];                            /* ds:4006 Zustandsbits zu g_cl_anim3/g_cl_anim3_speech */
extern struct cue_lines16 g_anim3_lines;                /* ds:400c Vorlage Untertitel/Sprache der Anim3-Folge 0x338 mit Sprach */
extern char g_cl_outro1[];                              /* ds:4072 Dateiliste der Abspannfolge Outro1 (O041) fuer load_pic_lis */
extern char g_cl_outro1_speech[];                       /* ds:407e Dateiliste Outro1 mit Sprachausgabe (bei g_cd_speech) */
extern char g_cl_outro_st[];                            /* ds:40a2 Zustandsbits zu g_cl_outro1/g_cl_outro1_speech */
extern struct outro_lines9 g_outro2_lines;              /* ds:40a8 Vorlage Zeilen Outro2::plain (O041, Texte zur Laufzeit) */
extern struct outro_lines9 g_outro2_voice_lines;        /* ds:4102 Vorlage Zeilen Outro2::voice */
extern struct outro_lines10 g_outro1_lines;             /* ds:415c Vorlage Zeilen Outro1::plain */
extern struct outro_lines10 g_outro1_voice_lines;       /* ds:41c0 Vorlage Zeilen Outro1::voice */
extern char g_str_gus[];                                /* ds:4244 "Forte UltraSound(TM) Digital Sound" (O042 init: g_snd_hide_ */
extern char g_str_ad[];                                 /* ds:4267 "AD" (O042 mtype) */
extern char g_str_opl[];                                /* ds:426a "OPL" (O042 mtype) */
extern char g_str_mt[];                                 /* ds:426e "MT" (O042 mtype; dahinter die Mausbereiche von O043) */
extern char g_sndsetup_areas_sel[];                     /* ds:4272 Mausbereiche der Treiberauswahl (O043 select; ohne eigenes P */
extern char g_sndsetup_areas_input[];                   /* ds:429a Mausbereiche der Hexwert-Eingabe (O043 input_hex; ohne eigen */
extern char g_sndsetup_areas_key[];                     /* ds:42b8 Mausbereich "Taste druecken" (O043 music_menu/digi_menu; ohn */
extern char g_str_insert_cd[];                          /* ds:42ce "Bitte Sternenschweif-CD einlegen und Taste druecken!" (O043 */
extern char g_str_no_cd_drive[];                        /* ds:4309 "Kein CD-Laufwerk gefunden!" (O043, CD_ERR_NO_DRIVE) */
extern char g_str_mscdex_old[];                         /* ds:432b "MSCDEX-Version ist aelter als 2.10!" (O043, CD_ERR_MSCDEX_O */
extern char g_str_not_cd_drive[];                       /* ds:4355 "Angegebenes Laufwerk ist kein CD-Laufwerk!" (O043, CD_ERR_N */
extern char g_fname_sound_adv[];                        /* ds:4387 "SOUND.ADV" (Musiktreiber, O043) */
extern char g_fname_digi_adv[];                         /* ds:4391 "DIGI.ADV" (Digitaltreiber, O043) */
extern char g_str_drivers_dir[];                        /* ds:439a "..\DRIVERS\" (O043 write_driver) */
extern char g_str_adv_mask[];                           /* ds:43c4 "*.ADV" (O043 scan) */
extern signed long g_xms_handles[10];                   /* ds:43dc XMS-Bloecke zu 16 MB, beim Start reserviert (R05 Init_ctor,  */
extern signed short g_xms_reserved;                     /* ds:4404 XMS-Bloecke reserviert (= g_xms_handles[10]; ein elfter Bloc */
extern struct str80 g_path_init;                        /* ds:4406 80 Nullbytes: Vorbelegung des Pfadpuffers in R05 load_archiv */
extern struct str5 g_ltx_count_init;                    /* ds:4456 5 Nullbytes: Vorbelegung des Zahlpuffers in R05 load_ltx (St */
extern char g_str_mem_need1[];                          /* ds:4523 "\nSternenschweif benoetigt einen installierten XMS/..." (R0 */
extern char g_str_mem_need2[];                          /* ds:4561 "und mindestens   688128 Byte freien XMS/EMS Speich..." (R05 */
extern char g_str_mem_need_ems[];                       /* ds:4597 "Wovon mindestens 622592 Bytes als EMS Speicher und" (R05, d */
extern char g_str_mem_need_xms[];                       /* ds:45ca "65536 Bytes als XMS Speicher ansprechbar sein muess..." (R0 */
extern char g_fmt_mem_have_ems[];                       /* ds:4612 "\nIhr System hat nur %8ld Byte freien EMS Speicher..." (R05 */
extern char g_fmt_mem_have_xms[];                       /* ds:4648 "%8ld Byte freien XMS Speicher." (R05, vorhandener XMS-Speic */
extern char g_str_mem_double1[];                        /* ds:467a "\nSollten beide Werte in Ihrer DOS-Anzeige den Anf..." (R05 */
extern char g_str_mem_double2[];                        /* ds:46c3 "so nutzt Ihr Speichermanager den Speicher doppelt!" (R05, Z */
extern char g_str_mem_hint1[];                          /* ds:46f6 "\nDa Ihre momentane Systemkonfiguration einen stab..." (R05 */
extern char g_str_mem_hint2[];                          /* ds:4739 "von Sternenschweif verhindert, bitten wir Sie vor ..." (R05 */
extern char g_str_mem_hint3[];                          /* ds:4780 "die Hinweise im beiliegenden Faltblatt und die Bei..." (R05 */
extern char g_str_mem_hint4[];                          /* ds:47c7 "in der Datei CONFIG.BSP zu beruecksichtigen." (R05, Zeile 4 */
extern char g_fmt_mem_have[];                           /* ds:47f3 "\nIhr System hat nur %8ld Byte freien XMS/EMS Spei..." (R05 */
extern char g_fmt_ovr_xms[];                            /* ds:482a "\nXMS (%ld KByte) for C++ Overlay handling utilize..." (R05 */
extern char g_str_ovr_none[];                           /* ds:485e "\nNo memory available to C++ Overlay handler." (R05, kein O */
extern char g_str_drive_root[];                         /* ds:488b "X:\" (R05, Laufwerk fuer das Startverzeichnis) */
extern char g_str_all_files[];                          /* ds:488f "*.*" (R05, Temp-Dateien loeschen) */
extern char g_fn_star_dat[];                            /* ds:4893 "star.dat" (R05, ONEFILE-Fassung) */
extern char g_str_onefile[];                            /* ds:489c "\nONEFILE Version" (R05, Meldung ONEFILE-Fassung) */
extern char g_fn_dpatch[];                              /* ds:48ad "dpatch.dat" (R05) */
extern char g_fn_filename_ltx[];                        /* ds:48b8 "filename.ltx" (R05) */
extern char g_fn_fx_dat[];                              /* ds:48c5 "fx.dat" (R05) */
extern char g_fn_fpatch[];                              /* ds:48cc "fpatch.dat" (R05) */
extern char g_fn_fxname_ltx[];                          /* ds:48d7 "fxname.ltx" (R05) */
extern char g_fn_speech_dat[];                          /* ds:48e2 "speech.dat" (R05) */
extern char g_fn_spatch[];                              /* ds:48ed "spatch.dat" (R05) */
extern char g_fn_pakname_ltx[];                         /* ds:48f8 "pakname.ltx" (R05, Pruefung) */
extern char g_fn_pakname_ltx2[];                        /* ds:4904 "pakname.ltx" (R05, Laden) */
extern char g_fn_raw_dat[];                             /* ds:4910 "raw.dat" (R05) */
extern char g_fn_rpatch[];                              /* ds:4918 "rpatch.dat" (R05) */
extern char g_fn_rawname_ltx[];                         /* ds:4923 "rawname.ltx" (R05) */
extern char g_fn_path_cfg[];                            /* ds:492f "..\path.cfg" (R05, CD-Laufwerk) */
extern char g_fn_attic_fli[];                           /* ds:493b "attic.fli" (R05, Kennung der CD) */
extern char g_fn_cdfname_ltx[];                         /* ds:4945 "cdfname.ltx" (R05, von CD) */
extern char g_fn_cdfname_ltx2[];                        /* ds:4951 "cdfname.ltx" (R05, Pruefung) */
extern char g_fn_cdfname_ltx3[];                        /* ds:495d "cdfname.ltx" (R05, Laden) */
extern char g_fn_attic_fli2[];                          /* ds:4969 "attic.fli" (R05, ohne path.cfg) */
extern char g_fn_speechcd_dat[];                        /* ds:4973 "speechcd.dat" (R05, im Verzeichnis) */
extern char g_fn_speechcd_dat2[];                       /* ds:4980 "speechcd.dat" (R05, auf der CD) */
extern char g_fn_cdpname_ltx[];                         /* ds:498d "cdpname.ltx" (R05, Pruefung) */
extern char g_fn_cdpname_ltx2[];                        /* ds:4999 "cdpname.ltx" (R05, Laden) */
extern char g_fn_dcdpatch[];                            /* ds:49a5 "dcdpatch.dat" (R05) */
extern char g_fn_scdpatch[];                            /* ds:49b2 "scdpatch.dat" (R05) */
extern signed char far *g_temple_god_locs[];            /* ds:49ea je Gott 1..14: Liste der Tempel (g_loc_id), Ende -1 (O044 te */
extern TEMPLE_PTR g_temple;                             /* ds:4a26 aktuelles Tempelobjekt (O044) */
extern char g_temple_fmt_and[];                         /* ds:4a2a " %s " (temple_diary_party: " und ") */
extern char g_temple_fmt_join[];                        /* ds:4a2f "%s %s" (temple_str_join) */
extern char g_temple_fmt_leave[];                       /* ds:4a35 "%s %s" (temple_str_leave) */
extern char g_town_occl_04[];                           /* ds:4a3c Stadt 4: Verdeckung je Feldtyp (O046 town_make_events -> g_c */
extern char g_town_occl_05[];                           /* ds:4a5a Stadt 5: Verdeckung je Feldtyp (O046 -> g_cell_occludes) */
extern struct view_obj g_town_objs_1[];                 /* ds:4a78 Objektliste Stadt 1 (struct view_obj, Ende idx -1, -2: Rest  */
extern struct view_obj g_town_objs_2[];                 /* ds:4ac3 Objektliste Stadt 2 (struct view_obj, Ende idx -1, -2: Rest  */
extern struct view_obj g_town_objs_6[];                 /* ds:4b14 Objektliste Stadt 6 (struct view_obj, Ende idx -1, -2: Rest  */
extern struct view_obj g_town_objs_3[];                 /* ds:4b5c Objektliste Stadt 3 (struct view_obj, Ende idx -1, -2: Rest  */
extern struct view_obj g_town_objs_4[];                 /* ds:4b86 Objektliste Stadt 4 (struct view_obj, Ende idx -1, -2: Rest  */
extern struct view_obj g_town_objs_5[];                 /* ds:4bdd Objektliste Stadt 5 (struct view_obj, Ende idx -1, -2: Rest  */
extern struct view_area g_town_areas[];                 /* ds:4c25 = g_town_view_areas - 1 (MAP-Public des 1-basierten Zugriffs */
extern struct view_area g_town_view_areas[6];           /* ds:4c37 Beschreibung je Stadt 1..6 (Index group.town - 1; O034/O046/ */
extern char g_cl_town[];                                /* ds:4ca3 Dateiliste der Stadtansicht (allgemein) fuer load_pic_list */
extern char g_cl_town_st[];                             /* ds:4cc5 Zustandsbits zu g_cl_town */
extern char g_cl_town1[];                               /* ds:4ccb Dateiliste der Stadtansicht (Stadt 1) fuer load_pic_list */
extern char g_cl_town1_st[];                            /* ds:4cd9 Zustandsbits zu g_cl_town1 */
extern char g_cl_town2[];                               /* ds:4cdb Dateiliste der Stadtansicht (Stadt 2) fuer load_pic_list */
extern char g_cl_town2_st[];                            /* ds:4ceb Zustandsbits zu g_cl_town2 */
extern char g_cl_town3[];                               /* ds:4ced Dateiliste der Stadtansicht (Stadt 3) fuer load_pic_list */
extern char g_cl_town3_st[];                            /* ds:4cfd Zustandsbits zu g_cl_town3 */
extern char g_cl_town4[];                               /* ds:4cff Dateiliste der Stadtansicht (Stadt 4) fuer load_pic_list */
extern char g_cl_town4_st[];                            /* ds:4d13 Zustandsbits zu g_cl_town4 */
extern char g_cl_town5[];                               /* ds:4d17 Dateiliste der Stadtansicht (Stadt 5) fuer load_pic_list */
extern char g_cl_town5_st[];                            /* ds:4d25 Zustandsbits zu g_cl_town5 */
extern char g_cl_town6[];                               /* ds:4d27 Dateiliste der Stadtansicht (Stadt 6) fuer load_pic_list */
extern char g_cl_town6_st[];                            /* ds:4d35 Zustandsbits zu g_cl_town6 */
extern struct item_combo g_item_combos[];               /* ds:4dd8 kombinierbare Gegenstandspaare (O048 combo_index), Ende item */
extern signed short g_recipe_items[];                   /* ds:4fb8 die 14 Rezepte (= g_recipes[i].item_id) als Faecher des Reze */
extern signed short g_recipe_outcomes[];                /* ds:4fd4 Ergebnis je Rezept (= g_recipes[i].outcome; Namen im Rezeptb */
extern signed short g_keyring_keys[];                   /* ds:4ff0 die 26 Schluessel, die an den Schluesselbund 0x145 passen */
extern struct item_list7 g_weapons_no_poison;           /* ds:5024 Waffen, die nicht vergiftet werden koennen (O047_1451) */
extern signed short g_item_anim_snd;                    /* ds:5032 Klang der Animation schon gespielt (O048; kein eigenes Publi */
extern struct srgb g_item_anim_color;                   /* ds:5034 Blendfarbe der Zwischensequenz (O048 quest_anim; Linkname _s */
extern char g_str_star_dat[];                           /* ds:5038 "STAR.DAT" (R06 arc_is_set) */
extern char g_str_dummy[];                              /* ds:5041 "DUMMY" (R06 arc_exists) */
extern char g_str_at[];                                 /* ds:514c "@" (Zeilenumbruch nach der Antwort, Conv::print) */
extern char g_str_ace[];                                /* ds:514e "ACE" Kennung der Animationsdateien (Anim_load, Archiv) */
extern char g_str_ace2[];                               /* ds:5152 "ACE" Kennung der Animationsdateien (Anim_load, Dateiobjekt) */
extern char g_str_mem_alloc_fail[];                     /* ds:5158 "Memory Allocation Failure (%ld Bytes / %ld left)!" */
extern char g_str_mem_magic_free[];                     /* ds:518c "Someone has destroyed the magic values for this block!" (me */
extern char g_str_mem_free_error[];                     /* ds:51e3 "Error freeing up allocated Memory!" */
extern char g_str_mem_no_entries[];                     /* ds:5208 "Running out of Allocation Entries!" */
extern char g_str_mem_magic_check[];                    /* ds:522d "Someone has destroyed the magic values for this block!" (me */
extern char g_amap_areas[];                             /* ds:5284 Mausbereichsliste der Automap (automap_open -> g_mouse_areas */
extern char g_amap_handler_areas[];                     /* ds:52ca Mausbereiche fuer den Rueckruf cursor_for_cell (automap_open */
extern signed short g_reveal_by_spell;                  /* ds:5310 Kartenzauber deckt auf (O235 f_1434 um reveal_map): Felder 0 */
extern char g_str_amap[];                               /* ds:5312 "AMAP" Kennung des Automap-Blocks im Spielstand */
extern char g_fmt_col4_531c[];                          /* ds:531c "\xf4%s\xf0" (automap_run, Zoomgrenze) */
extern char g_walk_default[];                           /* ds:5322 begehbare Feldtypen (Ende 0xff), Vorgabe der Automap-Reise ( */
extern char g_walk_town5[];                             /* ds:533d begehbare Feldtypen in Stadt 5 (O053) */
extern char g_walk_town4[];                             /* ds:534a begehbare Feldtypen in Stadt 4 (O053) */
extern char g_walk_dng_01[];                            /* ds:5358 begehbare Feldtypen Dungeon 1 (Ende -1; g_walk_dng, O053) */
extern char g_walk_dng_02[];                            /* ds:5389 begehbare Feldtypen Dungeon 2 (Ende -1; g_walk_dng, O053) */
extern char g_walk_dng_03[];                            /* ds:5394 begehbare Feldtypen Dungeon 3 (Ende -1; g_walk_dng, O053) */
extern char g_walk_dng_04[];                            /* ds:53a4 begehbare Feldtypen Dungeon 4 (Ende -1; g_walk_dng, O053) */
extern char g_walk_dng_05[];                            /* ds:53bf begehbare Feldtypen Dungeon 5 (Ende -1; g_walk_dng, O053) */
extern char g_walk_dng_06[];                            /* ds:53d5 begehbare Feldtypen Dungeon 6 (Ende -1; g_walk_dng, O053) */
extern char g_walk_dng_07[];                            /* ds:53f9 begehbare Feldtypen Dungeon 7 (Ende -1; g_walk_dng, O053) */
extern char g_walk_dng_08[];                            /* ds:541d begehbare Feldtypen Dungeon 8 (Ende -1; g_walk_dng, O053) */
extern char g_walk_town3[];                             /* ds:5441 begehbare Feldtypen in Stadt 3 bzw. Dungeon 9 (O053) */
extern char g_walk_dng_10[];                            /* ds:5471 begehbare Feldtypen Dungeon 10 (Ende -1; g_walk_dng, O053) */
extern char g_walk_dng_11[];                            /* ds:5482 begehbare Feldtypen Dungeon 11 (Ende -1; g_walk_dng, O053) */
extern char g_walk_dng_12[];                            /* ds:5492 begehbare Feldtypen Dungeon 12 (Ende -1; g_walk_dng, O053) */
extern char g_walk_dng_13[];                            /* ds:54a8 begehbare Feldtypen Dungeon 13 (Ende -1; g_walk_dng, O053) */
extern char g_walk_dng_14[];                            /* ds:54ba begehbare Feldtypen Dungeon 14/15 (Ende -1; g_walk_dng, O053 */
extern char g_walk_dng_16[];                            /* ds:54ce begehbare Feldtypen Dungeon 16 (Ende -1; g_walk_dng, O053) */
extern char far *g_walk_dungeon[];                      /* ds:54db = g_walk_dng - 1 (MAP-Public des 1-basierten Zugriffs; die 4 */
extern char far *g_walk_dng[16];                        /* ds:54df begehbare Feldtypen je Dungeon 1..16 (Index group.dungeon -  */
extern struct dir_steps g_dir_steps;                    /* ds:551f BCC-Vorlage des lokalen dirs (Schritte rechts/oben/links/unt */
extern struct point10 g_ring_init;                      /* ds:552f BCC-Vorlage des lokalen Ringpuffers q (10 Felder, Nullen) in */
extern char g_str_ani[];                                /* ds:5558 "ANI" Kennung der Bildanimationen (O055 ani_load; sonst alte */
extern char g_str_der[];                                /* ds:555c "der" (grammar_def_article; erster Text von g_grammar_articl */
extern char g_str_die[];                                /* ds:5560 "die" */
extern char g_str_des[];                                /* ds:5568 "des" */
extern char g_str_den[];                                /* ds:556c "den" */
extern char g_str_dem[];                                /* ds:5570 "dem" */
extern char far *g_grammar_articles[];                  /* ds:559f Artikeltexte "der", "die", "das", "des", ... (BE GRAMMAR_ART */
extern struct grammar_row g_grammar_def_table[];        /* ds:55db Artikelzeilen (bestimmt) je Kasus, 3 Indizes in g_grammar_ar */
extern struct grammar_row g_grammar_indef_table[];      /* ds:560b Artikelzeilen (unbestimmt) je Kasus; [8] = ohne Artikel ds:5 */
extern char far *g_fmt_s_s;                             /* ds:566b Zeiger auf "%s %s" (BE STR_S_S_PTR) */
extern char far *g_fmt_von_s_s;                         /* ds:566f Zeiger auf "von%s %s" (BE STR_VON_S_S_PTR) */
extern signed short g_mon_no_article[];                 /* ds:5673 Gegnerarten ohne Artikel (Liste fuer is_in_word_array, -1 =  */
extern signed short g_grammar_buf_no;                   /* ds:5677 naechster der 4 Puffer von grammar_article (BE GRAMMAR_BUF_N */
extern struct grammar_row g_grammar_gender_bits;        /* ds:5679 Geschlechtsbits fuer grammar_article {0x1000, 0x2000, 0x3000 */
extern char g_str_er[];                                 /* ds:567f "er" (grammar_pronoun; BE GRAMMAR_PRONOUNS_ER) */
extern char g_str_sie[];                                /* ds:5682 "sie" */
extern char g_str_sein[];                               /* ds:5689 "sein" */
extern char g_str_ihr[];                                /* ds:568e "ihr" */
extern char g_str_ihn[];                                /* ds:5692 "ihn" */
extern char g_str_ihm[];                                /* ds:5696 "ihm" */
extern char g_str_seine[];                              /* ds:56a0 "seine" */
extern char g_str_ihre[];                               /* ds:56a6 "ihre" */
extern char g_str_seinen[];                             /* ds:56ab "seinen" */
extern char g_str_ihren[];                              /* ds:56b2 "ihren" */
extern char far *g_grammar_pronouns[];                  /* ds:56b8 Pronomentexte er, sie, es, sein, ihr, ihn, ihm (BE GRAMMAR_P */
extern signed char g_grammar_pronoun_idx[];             /* ds:56d4 Index in g_grammar_pronouns je Pronomen und Geschlecht (3 je */
extern struct score7 g_score7_zero;                     /* ds:56fc sieben Nullworte (O058 preselect_agile_hero; kein eigenes Pu */
extern struct dng_target g_dng11_kobold_dest_single[];  /* ds:575e Dungeon 11: Ziel jedes einzelnen Helden beim Abschied des K */
extern struct dng_target g_dng11_kobold_dest_split[];   /* ds:5764 Dungeon 11: Ziele der Gruppenhaelften (n == 1; [0] auch n = */
extern struct dng_target g_dng11_kobold_dest_far[];     /* ds:5770 Dungeon 11: Ziele der Gruppenhaelften in Dungeon 10/12 (n = */
extern char g_dng11_password[];                         /* ds:5794 "ELCH" Antwort auf das Raetsel des Kobolds (O062 dng11_kobo */
extern struct hero far *g_dng12_cart_hero;              /* ds:579a Dungeon 12: Held in der Lore (O063) */
extern struct hero_ptrs7 g_dng14_hero_ptrs_null;        /* ds:57b4 sieben Nullzeiger als Vorlage (O066 dng14_swim; kein eigene */
extern struct camp_foe g_camp_foes[3][5];               /* ds:57dc Gegner naechtlicher Ueberfaelle je Gebiet (0, 1..3, 4/5) un */
extern char g_peddler_fmt_slots_free[];                 /* ds:581c "/^%d" (peddler_buy_draw_slots; wie g_fmt_slots_free) */
extern char g_peddler_fmt_slots_used[];                 /* ds:5821 "%d^" (peddler_buy_draw_slots) */
extern struct herb_find g_gather_herbs_table[13];       /* ds:5826 Kraeuter fuer camp_gather_herbs (12 x struct herb_find + Endemarke 0xff) */
extern signed char g_camp_water_season[4];              /* ds:585a Zuschlag Wassersuche je Jahreszeit (date_season) (O071) */
extern signed char g_camp_hunt_season[4];               /* ds:585e Zuschlag Jagd je Jahreszeit (O071) */
extern signed char g_camp_herb_season[4];               /* ds:5862 Zuschlag Kraeutersuche je Jahreszeit (O071) */
extern signed char g_camp_water_weather[6];             /* ds:5866 Zuschlag Wassersuche je Wetter g_camp_weather (O071) */
extern signed char g_camp_hunt_weather[6];              /* ds:586c Zuschlag Jagd je Wetter (O071) */
extern signed char g_camp_herb_weather[6];              /* ds:5872 Zuschlag Kraeutersuche je Wetter (O071) */
extern char g_gather_herbs_str_found[];                 /* ds:5878 "%d^%s" (camp_gather_herbs: Anzahl und Kraut) */
extern signed short g_camp_ambush_watch;                /* ds:5882 Lager: Ueberfall bei Wache n (1..3), 0 = zufaellig (O072 cam */
extern struct camp_encounter g_camp_encounters[];       /* ds:5884 naechtliche Ueberfaelle (struct camp_encounter, O072 camp_se */
extern signed short g_name_buf_no;                      /* ds:5974 naechster Ringpuffer (0..3) */
extern struct mr_table g_mr_modificators;               /* ds:5976 MR-Zuschlag je Heldentyp (13 Bytes) */
extern char g_str_gen_s[];                              /* ds:5983 Genitiv-Endung nach Kleinbuchstabe */
extern char g_str_gen_s2[];                             /* ds:5985 Genitiv-Endung nach Grossbuchstabe */
extern char g_str_empty_5988[];                         /* ds:5988 "" (Einzahl in change_attrib) */
extern char g_str_empty_5989[];                         /* ds:5989 "" (Einzahl in change_attrib) */
extern char g_str_char_magic[];                         /* ds:598a "CHAR" Blockkennung Held im Spielstand (O077) */
extern struct lock_type g_chest_types[];                /* ds:5990 Truhenarten {KK, Zuschlag Schloesser, Zuschlag Foramen} (str */
extern signed short g_quiet;                            /* ds:5a0e Merker: Aenderungsmeldungen (O077_00FB, O076_0832) unterdrue */
extern signed short g_items_keep[];                     /* ds:5a10 Gegenstaende, die drop_item_at nicht ablegt (is_in_word_arra */
extern struct start_gear g_start_gear;                  /* ds:5a16 BCC-Vorlage des lokalen Felds gear[5] in startup_equipment ( */
extern struct start_gear_types g_start_gear_types;      /* ds:5a20 BCC-Vorlage des lokalen Felds by_type[12][4] in startup_equi */
extern struct attrib_prio g_lvl_attrib_prio[13];        /* ds:5a72 = g_lvl_attrib_order - 1 (MAP-Public des 1-basierten Zugriff */
extern struct attrib_prio g_lvl_attrib_order[12];       /* ds:5a80 Eigenschaften-Reihenfolge je Typ 1..12 beim automatischen St */
extern struct incs12 g_lvl_skill_incs;                  /* ds:5b28 Talentsteigerungen je Stufe und Typ (O086; kein eigenes Pub */
extern struct incs6 g_lvl_spell_incs;                   /* ds:5b34 Zaubersteigerungen je Stufe, Typ 7..12 (O086) */
extern struct incs6 g_lvl_incs_swap;                    /* ds:5b3a umverteilbare Steigerungen je Zaubertyp (O086) */
extern signed short g_lvl_spells_witch[];               /* ds:5b40 Zauberliste Hexe fuer den Stufenanstieg (O086, Ende -1) */
extern signed short g_lvl_spells_druid[];               /* ds:5b46 Zauberliste Druide (O086) */
extern signed short g_lvl_spells_school1[];             /* ds:5b4e Zusatzliste Magierakademie 0 beim Stufenanstieg (O086, BE MA */
extern signed short g_lvl_spells_school2[];             /* ds:5b64 Zusatzliste Magierakademie 1 (O086, BE MAGIC_SCHOOLS_2) */
extern signed short g_lvl_spells_school3[];             /* ds:5b7c Zusatzliste Magierakademie 2 (O086, BE MAGIC_SCHOOLS_3) */
extern signed short g_lvl_spells_school4[];             /* ds:5b94 Zusatzliste Magierakademie 3 (O086, BE MAGIC_SCHOOLS_4) */
extern signed short g_lvl_spells_school5[];             /* ds:5bac Zusatzliste Magierakademie 4 (O086, BE MAGIC_SCHOOLS_5) */
extern signed short g_lvl_spells_school6[];             /* ds:5bc2 Zusatzliste Magierakademie 5 (O086, BE MAGIC_SCHOOLS_6) */
extern signed short g_lvl_spells_school7[];             /* ds:5bd6 Zusatzliste Magierakademie 6 (O086, BE MAGIC_SCHOOLS_7) */
extern signed short g_lvl_spells_school8[];             /* ds:5bf4 Zusatzliste Magierakademie 7 (O086, BE MAGIC_SCHOOLS_8) */
extern signed short g_lvl_spells_school9[];             /* ds:5c0c Zusatzliste Magierakademie 8 (O086, BE MAGIC_SCHOOLS_9) */
extern signed short far *g_lvl_spells_school[];         /* ds:5c26 Zauberliste je Magierakademie (O086) */
extern signed short g_lvl_spells_green_elf[];           /* ds:5c4a Zauberliste Auelf (O086) */
extern signed short g_lvl_spells_sylvan_elf[];          /* ds:5c58 Zauberliste Waldelf (O086) */
extern signed short g_lvl_spells_ice_elf[];             /* ds:5c66 Zauberliste Firnelf (O086) */
extern struct range8 g_school_spell_ranges[];           /* ds:5c7e Zauberbereich je Magierakademie {erster, letzter} (BE MAGIC_ */
extern char g_fmt_name_ae[];                            /* ds:5c90 Format "%s~%d" Name/AE (select_hero_ae) */
extern char g_fmt_name_le[];                            /* ds:5c96 Format "%s~%d" Name/LE (select_hero_le) */
extern char g_str_empty_5c9c[];                         /* ds:5c9c "" (Einzahl, skill_cure_poison) */
extern char g_str_empty_5c9d[];                         /* ds:5c9d "" (Einzahl, skill_cure_poison) */
extern char g_str_empty_5c9e[];                         /* ds:5c9e "" (Einzahl, skill_cure_disease) */
extern char g_str_empty_5c9f[];                         /* ds:5c9f "" (Einzahl, skill_cure_disease) */
extern struct select_spell_tmpl g_select_spell_tmpl;    /* ds:5ca0 Vorlagen fuer select_spell (O091) */
extern struct col_pos8 g_spell_cols1;                   /* ds:5cb2 Spalten der Zauberseite 4 (O094) */
extern struct col_pos4 g_spell_cols2;                   /* ds:5ce2 Spalten der Zauberseite 5 (O094) */
extern char g_str_empty_5cfa[];                         /* ds:5cfa "" (O094 Seite 1, ohne Modifikator) */
extern char g_str_empty_5cfb[];                         /* ds:5cfb "" (O094 Seite 1, ohne Modifikator) */
extern char g_fmt_load_hl[];                            /* ds:5cfc "\xf1%d\xf0" Last hervorgehoben (O094) */
extern char g_fmt_bp_hl[];                              /* ds:5d01 "\xf1%d\xf0" BP hervorgehoben (O094) */
extern char g_fmt_load[];                               /* ds:5d06 "%d" Last (O094) */
extern char g_fmt_bp[];                                 /* ds:5d09 "%d" BP (O094) */
extern struct col_pos7 g_skill_cols;                    /* ds:5d0c Spalten der Talentseite (O095) */
extern char g_fmt_item_name[];                          /* ds:5d36 "%s %s " Gegenstandsname und Zusatz (O095) */
extern char g_str_empty_5d3d[];                         /* ds:5d3d "" (O095) */
extern char g_fmt_item_name2[];                         /* ds:5d3e "%s %s " (O095) */
extern char g_str_empty_5d45[];                         /* ds:5d45 "" (O095) */
extern struct map_event g_dng16_key_event1;             /* ds:5d46 Feldereignis (struct map_event, Typ 1) nach dem Doppelbarts */
extern struct map_event g_dng16_key_event2;             /* ds:5d4a Feldereignis (struct map_event, Typ 2) nach dem Doppelbarts */
extern signed short g_pick_delay;                       /* ds:5d4e Sperre der Klickauswahl in Bildern (R12 pick_rect setzt 25) */
extern signed short g_last_cam_cell_y;                  /* ds:5d50 zuletzt gesehene Kamerazelle y (R12 cell_event: Ereignis be */
extern signed short g_last_cam_cell_x;                  /* ds:5d52 zuletzt gesehene Kamerazelle x (R12 cell_event) */
extern struct cursor_areas g_view_cursor_areas;         /* ds:5d54 Mausbereiche der 3D-Ansicht fuer set_cursor (7 x struct mous */
extern struct area_desc g_area_descs[17];               /* ds:5d9c Ereignisvorgabe je Dungeon 0..16 (Rueckruf, Kopie in area_ev */
extern struct area_desc g_town_descs[];                 /* ds:5e1c = g_town_area_descs - 1 (MAP-Public des 1-basierten Zugriffs */
extern struct area_desc g_town_area_descs[6];           /* ds:5e24 Ereignisvorgabe je Stadt 1..6 (Kartenereignis-Rueckruf, Inde */
extern char g_str_calcbuf_full[];                       /* ds:5e54 "CalcBuffer full." */
extern char g_str_too_many_vectors[];                   /* ds:5e65 "Too many vectors." */
extern char g_err_view_mem[];                           /* ds:5e78 "OUT OF MEMORY" (O096 view_alloc_lists) */
extern char g_err_view_init[];                          /* ds:5e86 "CAN'T INITIALIZE ..." (O096 view_load_palette) */
extern char g_err_too_many_tex[];                       /* ds:5eb2 "TOO MANY BITMAPS" (O097 tex_get) */
extern char g_err_tex_load[];                           /* ds:5ec3 "CAN'T LOAD BITMAP" (O097 tex_get) */
extern char g_err_obj_mem[];                            /* ds:5ed5 "OUT OF MEMORY" (O097 obj_load) */
extern signed short g_cam_speed_set;                    /* ds:5ee4 zuletzt gesetzte Geschwindigkeit (cam_set_speed) */
extern signed short g_cam_bounced;                      /* ds:5ee6 Kamera ist abgeprallt (cam_move) */
extern signed short g_func_trace[10];                   /* ds:5ee8 letzte 10 Funktionsnummern (R14 push_fc schiebt nach, guru_m */
extern signed short g_dev_screen_on;                    /* ds:5efc Entwickler-Infobildschirm aktiv (O011 dev_info_screen; R18 r */
extern signed short g_xms_free_kb;                      /* ds:5f00 freier XMS-Speicher in KB nach dem Reservieren (R05; O011 ze */
extern signed short g_min_stack;                        /* ds:5f02 kleinster gesehener Stackzeiger (DEBUG.LTX "Minstack"; O011) */
extern signed short g_mem_debug;                        /* ds:5f06 Speicherpruefung an (Waechterwoerter, Fehlermeldungen; R07) */
extern signed short g_mem_debug2;                       /* ds:5f08 Speicher-Fehlersuche (Startwert 1 neben g_mem_debug, O032 m */
extern signed short g_func_trace_on;                    /* ds:5f0a Funktionsnummern mitschreiben (O011 Befehl an/aus) */
extern void far *g_debug_txt_buf;                       /* ds:5f18 Textliste DEBUG.LTX (0x1f0) des Testmenues/Infobildschirms ( */
extern char far * far *g_debug_txt;                     /* ds:5f1c Zeiger auf die Texte von DEBUG.LTX (O007, O011) */
extern struct str3 g_str_crlf;                          /* ds:5f20 "\r\n" (R14 Zeilenende der ALLOC.CAT-Eintraege) */
extern char g_str_guru[];                               /* ds:5f23 "STARTRAIL  --  GURU Meditation\r\n" (R14) */
extern char g_fmt_guru_fcs[];                           /* ds:5f5d "\r\nFCs: %d ... XMS handles: %d\r\n" (R14 Bildschirm) */
extern char g_fmt_guru_fcs_log[];                       /* ds:5f95 "%s\r\nFCs: %d ... XMS handles: %d\r\n" (R14 Protokoll) */
extern char g_fmt_guru_dest[];                          /* ds:5fcf "\r\nDest: 0x%lx\r\n" (R14) */
extern char g_fmt_guru_src[];                           /* ds:5fdf "\r\nSource: 0x%lx\r\n" (R14) */
extern char g_str_alloc_cat[];                          /* ds:5ff1 "ALLOC.CAT" (R14 Liste der Speicherbloecke) */
extern char g_fmt_alloc_entry[];                        /* ds:5ffb "Allocentry %4d: Start: %p End: %p Len: %ld%s" (R14) */
extern char g_fmt_guru_addr[];                          /* ds:6028 "\r\nAddress: 0x%lx\r\n" (R14 Bildschirm) */
extern char g_fmt_guru_addr_log[];                      /* ds:603b "\r\nAddress: 0x%lx\r\n" (R14 Protokoll) */
extern char g_str_schweif_cat[];                        /* ds:604e "SCHWEIF.CAT" (R14 Speicherauszug) */
extern signed char g_diary_ctrl[];                      /* ds:605c Steuerzeichen im Tagebuchtext: [0] 0xa0 Seitenkopf, [1] Eint */
extern signed char g_diary_ctrl_entry;                  /* ds:605d 0xa1 Eintrag (= g_diary_ctrl[1], eigenes Public) */
extern signed char g_diary_ctrl_own;                    /* ds:605e 0xa2 eigener Eintrag (= g_diary_ctrl[2]) */
extern signed char g_diary_ctrl_end;                    /* ds:605f 0xa3 Ende eines Eintrags (= g_diary_ctrl[3]) */
extern signed char g_diary_tag[];                       /* ds:6060 Namensmarken 0xa4..0xa9 (Kennung des Begriffs folgt; Kapitel */
extern signed char g_diary_tag4;                        /* ds:6064 0xa8 (= g_diary_tag[4], eigenes Public) */
extern signed char g_diary_tag5;                        /* ds:6065 0xa9 (= g_diary_tag[5]); dahinter ds:6066 die Mausbereiche d */
extern struct mouse_area g_diary_areas[9];              /* ds:6066 Mausbereiche des Tagebuchs (9 x struct mouse_area, O103; kei */
extern struct names7 g_diary_chapter_names;             /* ds:60c0 statische Kopie der Kapitelnamen fuer die Suche (C++-Initial */
extern char g_str_diary_dat[];                          /* ds:60dc "DIARY.DAT" (diary_add lesen) */
extern char g_str_diary_dat2[];                         /* ds:60e6 "DIARY.DAT" (diary_add schreiben) */
extern char g_str_diary_txt[];                          /* ds:60f0 "..\DIARY.TXT" (diary_print_menu) */
extern char g_fmt_diary_c[];                            /* ds:60fe "%c" (diary_append Kopf) */
extern char g_fmt_diary_c2[];                           /* ds:6102 "%c" (diary_append Eintrag) */
extern char g_fmt_diary_end[];                          /* ds:6106 "@@%c" (diary_append Ende) */
extern char g_fmt_diary_s[];                            /* ds:610c "%s" (diary_show_page) */
extern char g_str_diary_dat3[];                         /* ds:610f "DIARY.DAT" (diary_draw) */
extern char g_str_diary_dat4[];                         /* ds:6119 "DIARY.DAT" (diary_load_page) */
extern char g_str_diary_dat5[];                         /* ds:6124 "DIARY.DAT" (O105) */
extern char g_str_diary_txt2[];                         /* ds:612e "..\DIARY.TXT" (O105 Druckdatei) */
extern char g_str_lastgame[];                           /* ds:6148 "LASTGAME" (O106 Ersatzname) */
extern char g_str_glob[];                               /* ds:6151 "GLOB" Blockkennung (O106) */
extern char g_str_runv[];                               /* ds:6156 "RUNV" (O106) */
extern char g_str_mons[];                               /* ds:615b "MONS" (O106) */
extern char g_str_play[];                               /* ds:6160 "PLAY" (O106) */
extern char g_str_locs[];                               /* ds:6165 "LOCS" (O106) */
extern char g_str_temp[];                               /* ds:616e "TEMP" (O106) */
extern struct mouse_area g_savemenu_areas[];            /* ds:618a Mausbereiche des Spielstandmenues (O107) */
extern char g_str_gam_mask[];                           /* ds:6207 "*.GAM" (O107) */
extern struct lock_type g_lock_types[];                 /* ds:6230 Schlossarten {KK x 2, Zuschlag Schloesser, Zuschlag Zauber}  */
extern char g_edit_chars[];                             /* ds:6288 erlaubte Zeichen des Texteingabefelds Edit (is_in_byte_array */
extern signed char g_anim_slots[];                      /* ds:62e4 Belegung der XMS-Plaetze der Animationsobjekte (O016, 30 + 1 */
extern signed char g_anim_slots_extra[];                /* ds:6302 = g_anim_slots[30] (eigenes Public), Plaetze der zusaetzlich */
extern char g_fmt_dbg_k_i[];                            /* ds:630c "k=%d, i=%d" (O116 tevent_049: Debugmeldung bei unbekanntem */
extern struct dxy g_fig_obj_dxy[];                      /* ds:631c Bildversatz je Feldobjekt-Bildnummer */
extern struct dxy g_fig_tail_dxy[4];                    /* ds:650c Versatz des Schwanzfelds eines Zweifeldgegners je Blickricht */
extern signed short g_fig_transparent_objs[];           /* ds:651c Objekte (Feldwert - 50), die die Sichtlinie nicht sperren (W */
extern signed short g_fig_origin_x;                     /* ds:6544 Bildlage des Feldes (0, 0) x (10) */
extern signed short g_fig_origin_y;                     /* ds:6546 Bildlage des Feldes (0, 0) y (116) */
extern signed char g_fig_stand_frames[][4];             /* ds:6548 Standbild je Richtung (Helden ohne Waffenart 4/6 und Gegner; */
extern signed char g_fig_hero_stand_wc4[4];             /* ds:654c Standbild je Richtung mit Waffenart 4 (hand_weapon_class) */
extern signed char g_fig_hero_stand_wc6[4];             /* ds:6550 Standbild je Richtung mit Waffenart 6 */
extern signed char g_fig_hero_sleep_frames[4];          /* ds:6554 Bild schlafend/bewusstlos je Richtung */
extern signed char g_fig_stone_frames[4];               /* ds:6558 Bild versteinert je Richtung (Helden und Gegner) */
extern signed char g_fig_step_frames[4];                /* ds:655c Schrittbild je Richtung (fig_enemy_move/fig_hero_place) */
extern signed char g_fig_step2_frames[4];               /* ds:6560 Doppelschritt je Richtung */
extern signed char g_fig_attack_frames[4];              /* ds:6564 Angriffsbild je Richtung ohne Waffe (Helden) bzw. Gegner */
extern signed char g_fig_parry_frames[4];               /* ds:6568 Paradebild je Richtung ohne Waffe (Helden) bzw. Gegner */
extern signed char g_fig_hero_attack_frames[4][4];      /* ds:656c Angriffsbild je Nahkampf-Waffenart 0..3 und Richtung */
extern signed char g_fig_action_frames[7][4];           /* ds:657c Paradebild je Nahkampf-Waffenart 0..3, Schuss-/Wurfbild je F */
extern signed char g_fig_spell_frames[2][4];            /* ds:6598 Zauberbild je Richtung ([0] Hexe bzw. Gegner, sonst [spell_i */
extern signed char g_fig_spell_hit_frames[4];           /* ds:65a4 Bild unter Zauberwirkung je Richtung */
extern signed char g_fig_hero_use_frames[4];            /* ds:65a8 Bild Gegenstand benutzen je Richtung (fig_hero_use_seq) */
extern signed char g_fig_hero_throw_frames[4];          /* ds:65ac Wurfbild je Richtung (fig_hero_throw_seq) */
extern signed char g_fig_hero_pose_frames[4];           /* ds:65b0 Einzelbild je Blickrichtung des Helden (fig_hero_pose_seq O */
extern unsigned char g_twofield_mons[];                 /* ds:65b4 Gegnerarten mit zwei Feldern (is_in_byte_array2) */
extern unsigned char g_mons_corpse_blocks[];            /* ds:65c1 Gegnerarten, deren Leiche das Feld sperrt (enemy_blocks), En */
extern unsigned char g_gfx_humanoid[];                  /* ds:65c5 Kampfgrafiken menschenaehnlicher Gegner (enemy_is_humanoid), */
extern unsigned char g_gfx_undead[];                    /* ds:65dd Kampfgrafiken untoter Gegner (enemy_is_undead: Boron-Schutz) */
extern signed short g_fight_flee_all;                   /* ds:65e5 Merker: alle Gegner fliehen (O192 enemy_sub_le, O128 fight_d */
extern char g_dng03_password[];                         /* ds:65e8 "ARKANDOR" (O131: Name des Herrn dieser Gemaeuer, strncmp 8  */
extern char g_dng03_letters_ar[];                       /* ds:65f1 "AR" (O131: in die Statue gemeisselte Buchstaben, event_noti */
extern char g_dng03_letters_or[];                       /* ds:65f4 "OR" (O131 event_notice) */
extern char g_dng03_letters_ka[];                       /* ds:65f7 "KA" (O131 event_notice) */
extern char g_dng03_letters_nd[];                       /* ds:65fa "ND" (O131 event_notice) */
extern struct point g_dng01_scatter_pos[10];            /* ds:6600 Dungeon 1: zehn Zielfelder {x, y} der Verstreuungsfalle (O13 */
extern struct hero_ptrs7 g_dng01_scatter_init;          /* ds:6628 Dungeon 1: sieben Nullzeiger als Vorlage der Heldenliste in  */
extern signed short g_fight_timer;                      /* ds:6644 Zeitgeber-Handle des Kampfes (fight_timer_start), -1 = keine */
extern char g_env_left_idx[];                           /* ds:6646 Umgebungsfelder mit linkem Nachbarn (is_in_byte_array) */
extern char g_env_right_idx[];                          /* ds:664f Umgebungsfelder mit rechtem Nachbarn */
extern struct dirs4 g_fig_dirs4;                        /* ds:6658 vier Richtungen {dx, dy} (Wegsuche) */
extern struct fight_def g_fight_defs_ab[7];             /* ds:6668 Kampfdefinitionen der Kampfnummern 0xab..0xb1 (Index Kampfn */
extern struct encounter g_encounters[3];                /* ds:666b = g_fight_defs_ab[0].groups: drei Gegnergruppen des Kampfes  */
extern signed short g_fight_ab_loot;                    /* ds:667f = g_fight_defs_ab[0].loot[0] (eigenes Public): Beute des Ka */
extern struct range8 g_fig_ranges[];                    /* ds:684b Abstandsbereiche der Gegner zu den Helden (fight_group.range */
extern struct cxy g_fig_entry_pos[5];                   /* ds:6851 Eintrittsfeld je Seite (5) */
extern struct cxy g_env_cell_offs[12];                  /* ds:685b zwoelf Teilfelder eines Umgebungsfeldes */
extern signed short g_fight_env_build;                  /* ds:6873 Kampffeld wird aus der Umgebung gebaut (0x9c gilt als begehb */
extern char g_fight_map_file2[];                        /* ds:6875 Datei mit der gesicherten 3D-Zellentabelle (fight_setup_env  */
extern char g_fight_map_file3[];                        /* ds:687b dieselbe Datei (fight_setup_env loescht sie) */
extern unsigned char g_sfight_figs[];                   /* ds:6882 Sonderkampf 0x493: Figur-IDs der vier Teile der Sonderfigur  */
extern unsigned char g_sfight_fig1;                     /* ds:6883 Sonderkampf 0x493: Figur-ID Teil 1 (= g_sfight_figs[1], eige */
extern unsigned char g_sfight_fig2;                     /* ds:6884 Sonderkampf 0x493: Figur-ID Teil 2 (= g_sfight_figs[2], eige */
extern unsigned char g_sfight_fig3;                     /* ds:6885 Sonderkampf 0x493: Figur-ID Teil 3 (= g_sfight_figs[3], eige */
extern signed short g_sfight_part_x[];                  /* ds:6886 Sonderkampf 0x493: feste Bildlage x der Teile 0/1 (sfight_fi */
extern signed short g_sfight_part_y[];                  /* ds:688e Sonderkampf 0x493: feste Bildlage y der Teile 0/1 */
extern signed short g_sfight_pos;                       /* ds:6896 Sonderkampf 0x493: Lage der Sonderfigur im Bildraster (13 Sp */
extern signed short g_sfight_phase;                     /* ds:6898 Sonderkampf 0x493: Phase (1 nach der Einleitung; O142) */
extern struct point g_sfight_grid_xy[];                 /* ds:689a Sonderkampf 0x493: Bildlage je Lage im Raster (13 Spalten) */
extern signed short g_sfight_blocked[];                 /* ds:69d2 Sonderkampf 0x493: Lagen, die die Sonderfigur nicht betritt  */
extern struct cxy g_sfight_walls[];                     /* ds:69ec Sonderkampf 0x493: Felder FIG_CELL_WALL (Ende x == -1) */
extern struct cxy g_sfight_walls2[];                    /* ds:6a1a Sonderkampf 0x493: Felder FIG_CELL_WALL2 (Ende x == -1) */
extern struct cxy g_sfight_cells[];                     /* ds:6a2e Sonderkampf 0x493: Felder der Sonderfigur FIG_CELL_SFIGHT (E */
extern signed short g_sfight_seq_on;                    /* ds:6af8 Sonderkampf 0x493: Zwischensequenz laeuft (Ereignisse nach g */
extern struct sfight_starts g_sfight_starts;            /* ds:6afa Sonderkampf 0x493: Startplaetze der Helden (kein eigenes Pub */
extern struct sfight_spells g_sfight_spells;            /* ds:6b16 Sonderkampf 0x493: Zauber des Sondergegners (13, zufaellig;  */
extern struct seq27 g_sfight_spell_seq1;                /* ds:6b30 Sonderkampf 0x493: Bildfolge Zauber (ungerade Nummer), Ende  */
extern struct seq25 g_sfight_spell_seq2;                /* ds:6b4b Sonderkampf 0x493: Bildfolge Zauber (gerade Nummer), Ende -1 */
extern struct points7 g_sfight_pic_size;                /* ds:6b64 Sonderkampf 0x493: Groesse der sieben Bildausschnitte (sfigh */
extern struct points7 g_sfight_pic_pos;                 /* ds:6b80 Sonderkampf 0x493: Lage der sieben Bildausschnitte */
extern struct hero_ptrs7 g_hero_ptrs7_null;             /* ds:6b9c sieben Nullzeiger als Vorlage (O143 fight_split_escaped) */
extern struct hero_ptrs7 g_hero_ptrs7_null2;            /* ds:6bb8 sieben Nullzeiger als Vorlage (O143 fight_split_escaped) */
extern char g_buy_screen_str_d_s[];                     /* ds:6bdf "%d %s" (BE BUY_SCREEN_STR_D_S; O143 fig_split_ap) */
extern signed short g_fig_anim_busy;                    /* ds:6be6 Schutz gegen Wiedereintritt (fight_anim_tick) */
extern struct fig_keys g_fig_keys;                      /* ds:6be8 Tasten der Kampfaktionen '1'..'7' (kein eigenes Public: ds:6 */
extern struct dirs4 g_fig_dirs_back;                    /* ds:6bf6 Richtungen {dx, dy} der Rueckverfolgung (fig_find_path_backt */
extern struct dirs4 g_fig_dirs_search;                  /* ds:6c06 Richtungen der Breitensuche (fight_path_search) */
extern struct dirs4 g_fig_dirs_enemy;                   /* ds:6c16 Schrittrichtungen beim Bewegen eines Gegners (fig_enemy_move */
extern struct dirs4 g_fig_dirs_hero;                    /* ds:6c26 Schrittrichtungen beim Bewegen eines Helden (fig_hero_place) */
extern struct dirs4 g_fig_dirs_ki;                      /* ds:6c36 Richtungen der Nahkampf-Zielsuche (enemy_turn; kein eigenes  */
extern char g_str_weapon_fmt[];                         /* ds:6c46 "%s %s %s" (O155 fig_menu_change_weapon: Name, zerbrochen, v */
extern char g_str_empty_6c4f[];                         /* ds:6c4f "" (O155; kein eigenes Public: ds:6c46 + 9) */
extern char g_str_empty_6c50[];                         /* ds:6c50 "" (O155; kein eigenes Public) */
extern char g_str_empty_6c51[];                         /* ds:6c51 "" (O155; kein eigenes Public) */
extern char g_str_empty_6c52[];                         /* ds:6c52 "" (O155; kein eigenes Public) */
extern signed char g_af_spell_list[16];                 /* ds:6c54 Zauber der automatisch kaempfenden Helden (ki_select_spell_t */
extern char g_str_space_6c66[];                         /* ds:6c66 " " (O163 ford_crossing: ohne Wasserschlaeuche) */
extern struct incs12 g_chargen_skill_incs;              /* ds:6c74 Talentsteigerungen bei der Erschaffung je Typ (O167; kein e */
extern struct incs6 g_chargen_spell_incs;               /* ds:6c80 Zaubersteigerungen je Zaubertyp (O167) */
extern struct incs6 g_chargen_incs_swap;                /* ds:6c86 umverteilbare Steigerungen je Zaubertyp (O167) */
extern struct xy14 g_chargen_attrib_pos;                /* ds:6c8c Lage der Eigenschaften auf Seite 0 (O168; kein eigenes Publ */
extern struct range8x9 g_chargen_school_ranges;         /* ds:6cc4 Hauszauber-Bereich je Akademie (O170; kein eigenes Public) */
extern struct type_list13 g_chargen_type_list_init;     /* ds:6cd6 13 Nullbytes: Vorbelegung der Typliste (O170; kein eigenes  */
extern struct house_hint g_house_hints[];               /* ds:6ce4 Haeuser der Bewohner mit Rat (Stadt, x, y; Ende -1; O173 hou */
extern signed char g_fig_msg_colors[];                  /* ds:6d18 Farbe je Kampfmeldung */
extern struct portrait_idx12 g_dsa1_pics_m;             /* ds:6d30 erste Pruefsumme der maennlichen Portraets je Typ (O176) */
extern struct portrait_idx11 g_dsa1_pics_f;             /* ds:6d3c erste Pruefsumme der weiblichen Portraets je Typ (O176) */
extern char g_str_roa_game[];                           /* ds:6d47 "ROA GAME" Kennung englischer DSA-1-Spielstaende (O176) */
extern signed short g_dsa1_words_v1[];                  /* ds:6d50 Wortfelder des fruehen DSA-1-Satzes (Byteordnung drehen, En */
extern signed short g_dsa1_longs_v1[];                  /* ds:6d60 Langwortfelder des fruehen Satzes (O177) */
extern signed short g_dsa1_words[];                     /* ds:6d6a Wortfelder des DSA-1-Satzes (O177) */
extern signed short g_dsa1_longs[];                     /* ds:6d7a Langwortfelder des DSA-1-Satzes (O177) */
extern struct item_list23 g_dsa1_slot_map;              /* ds:6d84 DSA-1-Inventarplatz -> DSA-2-Platz (O177) */
extern struct item_list25 g_dsa1_drop_items;            /* ds:6db2 beim Import entfallende Gegenstaende (O177) */
extern struct item_list23 g_dsa1_slot_map_v1;           /* ds:6de4 dasselbe fuer das fruehe Format (O177) */
extern struct item_list25 g_dsa1_drop_items_v1;         /* ds:6e12 dasselbe fuer das fruehe Format (O177) */
extern INN_PTR g_inn;                                   /* ds:6e44 aktuelles Herbergsobjekt (O178; O179 Ereignis-Rueckruf) */
extern unsigned char g_tmap_mark1_tick;                 /* ds:6e48 Reisekarte: Animationszaehler Kartenpunkt 1 (O187 mark1_anim */
extern unsigned char g_tmap_mark2_tick;                 /* ds:6e49 Reisekarte: Animationszaehler Kartenpunkt 2 (O187 mark2_anim */
extern signed short g_tmap_last_row;                    /* ds:6e4a Reisekarte: tile_y beim letzten load_col (O187) */
extern signed char g_loc_icons_loaded[];                /* ds:6e4c Nummer des je Symbolfeld geladenen Symbols (Datei 6), -1 Gru */
extern char g_str_text_too_long[];                      /* ds:6e56 "Text ist viel, viel zu lange!!!!" (Dlg::layout, guru */
extern signed short g_npc_file_ids[];                   /* ds:6e78 Dateinummern der NSC-Heldendateien je npc_id (O196, O200) */
extern signed short g_sky_shade;                        /* ds:6e8a zuletzt gesetzte Daemmerungsstufe des Himmels (O198 view_dra */
extern signed short g_options_open;                     /* ds:6e8c Optionen-Menue offen (O199 setzt 1/0; R02: Optionen-Taste nu */
extern struct opt_entry g_opt_entries[];                /* ds:6e8e Eintraege des Optionen-Menues (17 x 14 Bytes; O199; ohne eig */
extern signed char far *g_opt_sub_val;                  /* ds:6ebe = g_opt_entries[3].val (Untertitel; eigenes Public; O199 app */
extern signed char g_opt_sub_img0;                      /* ds:6ec2 = g_opt_entries[3].img[0] (eigenes Public; O199 apply) */
extern signed char g_opt_sub_img1;                      /* ds:6ec3 = g_opt_entries[3].img[1] (-1 = Untertitel nicht abschaltbar */
extern struct mouse_area g_opt_areas[];                 /* ds:6f7c Mausbereiche des Optionen-Menues (Regler x0/x1 je nach Wert; */
extern signed char g_opt_defaults[];                    /* ds:703a Vorgaben der 17 Eintraege des Optionen-Menues (-1 = unveraen */
extern char g_str_part[];                               /* ds:704c Kennung "PART" des Gruppenblocks im Spielstand */
extern char g_dng05_answer1[];                          /* ds:7052 "NACHTS" (O204: Antwort auf die Frage der Steinplatte, strnc */
extern char g_dng05_answer2[];                          /* ds:7059 "BEI NACHT" (O204) */
extern char g_dng05_answer3[];                          /* ds:7063 "NACHT" (O204) */
extern char g_str_space_707a[];                         /* ds:707a " " (O206) */
extern char g_str_space_707c[];                         /* ds:707c " " (O206) */
extern char g_str_crlf_707e[];                          /* ds:707e "\r\n" (O206 print_line, Datei; kein eigenes Public) */
extern struct str20 g_print_path_init;                  /* ds:7082 "..\\" (20 Bytes, Vorlage des Dateinamens, O207) */
extern char g_str_space_7096[];                         /* ds:7096 " " (O207 Leerzeile) */
extern char g_str_space_70ac[];                         /* ds:70ac " " (O207) */
extern char g_str_space_70ae[];                         /* ds:70ae " " (O207) */
extern unsigned char g_scene_places[];                  /* ds:70b6 Orte, deren Ereignis die Reiseoberflaeche abbaut (pla */
extern signed short g_encounter_enemies[];              /* ds:70ea Gegnergruppen der Zufallsbegegnungen unterwegs (O212  */
extern struct mouse_area g_rose_dir_areas[];            /* ds:7140 Mausbereiche der Kompassrose je Richtung 1..8, relati */
extern struct mouse_area g_rose_areas[];                /* ds:719a Mausbereich der ganzen Kompassrose (Aktion 9 = Rose v */
extern struct route_list42 g_water_routes;              /* ds:71ae 42 Wege mit Wasserstelle (O215: bei 3/5 des Weges Was */
extern struct hero_bar_areas g_hero_bar_areas;          /* ds:7204 Mausbereiche der Portraetleiste fuer select_hero_bar (8 x 10 */
extern signed char g_rose_placed;                       /* ds:7254 Kompassrose schon platziert (rose_draw: sonst Ecke suchen; r */
extern char g_str_dbg_ambassador[];                     /* ds:7256 "Botschafter" (debug_ending_menu) */
extern char g_str_dbg_sudran[];                         /* ds:7262 "Sudran Alatzer" (debug_ending_menu) */
extern char g_str_dbg_adeptus[];                        /* ds:7271 "Adeptus" (debug_ending_menu) */
extern char g_str_dbg_mob[];                            /* ds:7279 "der Mob poebelt" (debug_ending_menu) */
extern char g_str_dbg_back[];                           /* ds:7288 "zurueck" (debug_ending_menu) */
extern char g_str_dbg_good_end[];                       /* ds:728f "gutes Ende" (debug_ending_menu: Ja -> g_trial_lost = 0) */
extern struct mouse_area g_sel_list_areas[];            /* ds:729a Mausbereiche der Auswahlliste Sel (O225 loop; vorher doppelt */
extern SHOP_PTR g_shop;                                 /* ds:72cc aktuelles Ladenobjekt (Klasse Shop, O226; O006 shop_leave) */
extern char g_fmt_slots_free[];                         /* ds:72d4 "/^%d" (shop_buy_draw_slots) */
extern char g_fmt_slots_used[];                         /* ds:72d9 "%d^" (shop_buy_draw_slots) */
extern signed short g_merchant_items_posx[];            /* ds:72de Raster der Laeden: x der 15 Felder (shop_grid_input) */
extern signed short g_merchant_items_posy[];            /* ds:72fc Raster der Laeden: y der 15 Felder (shop_grid_input) */
extern void far *g_smith;                               /* ds:731a aktuelle Schmiede (class Smith, O230; smith_talk_event) */
extern char g_str_empty_731e[];                         /* ds:731e "" (O231: Abschlusstext ausser in Stadt 4) */
extern char g_blaster_keys[];                           /* ds:7320 "AIDT": Kennbuchstaben der BLASTER-Variable (O232) */
extern signed short g_blaster_bases[];                  /* ds:7325 Zahlenbasis je Kennbuchstabe {16, 10, 10, 10} (O232) */
extern signed short far *g_blaster_dest[4];             /* ds:732d Ziel je Kennbuchstabe A/I/D/T: &g_blaster_desc.io/.irq/.dma, */
extern char far *g_blaster_music_drv[];                 /* ds:7339 = g_blaster_music_drivers - 1 (MAP-Public des 1-basierten Zu */
extern char far *g_blaster_music_drivers[4];            /* ds:733d Musiktreiber je Kartentyp 1..4 (Index type - 1; O232 blaster */
extern char far *g_blaster_digi_drv[];                  /* ds:7349 = g_blaster_digi_drivers - 1 (MAP-Public des 1-basierten Zug */
extern char far *g_blaster_digi_drivers[4];             /* ds:734d Digitaltreiber je Kartentyp 1..4 (Index type - 1; O232) */
extern char far *g_blaster_card_name[];                 /* ds:7359 = g_blaster_card_names - 1 (MAP-Public des 1-basierten Zugri */
extern char far *g_blaster_card_names[4];               /* ds:735d Kartenname je Kartentyp 1..4 (Index type - 1; O232) */
extern struct ail_drv_desc g_blaster_result;            /* ds:736d Treiberbeschreibung mit Port/IRQ aus BLASTER (O232 blaster_d */
extern char g_str_blaster[];                            /* ds:7461 "BLASTER" (Umgebungsvariable, O232) */
extern char g_str_hex_digits[];                         /* ds:7469 "0123456789ABCDEF" (O232 blaster_parse) */
extern struct heal_ani_frames g_heal_ani_frames;        /* ds:747a Bildfolge der Heilanimation {0, 1, 2, 1, 0} (O233 Cast::heal */
extern signed short g_amap_teleport;                    /* ds:7486 Automap-Reise springt auf ein bekanntes Feld statt Wegsuche  */
extern signed short g_amap_teleported;                  /* ds:7488 Sprung erfolgt (O053 automap_travel; O235 prueft) */
extern struct analues_item g_analues_items[];           /* ds:748a Analues: Gegenstaende mit eigenem Text {id, noetiges Ergebni */
extern signed long g_odem_items;                        /* ds:75b0 Odem Arcanum: zwei Gegenstaende (185, 184) mit Sondertext, a */
extern struct txt_idx9 g_nekropathia_txt;               /* ds:75b4 Nekropathia: neun Antworttexte (Zaubertextnummern; O236 Stru */
extern signed short g_trnd_old_width;                   /* ds:75c8 Fensterbreite vor dem Zufallsereignis (O239) */
extern char g_trnd_fmt2[];                              /* ds:75ca "%s%s" (O239 town_rnd_donation_amount) */
extern char g_trnd_fmt1[];                              /* ds:75cf "%s" (O239 town_rnd_donation_amount) */
extern struct tavern far *g_tavern;                     /* ds:75d2 aktuelle Schenke (class Tavern : struct tavern, O240/O241/O */
extern char g_str_empty_75d8[];                         /* ds:75d8 "" (O242 tavern_night_event: Titel) */
extern unsigned char g_font_charmap[];                  /* ds:75de Zeichencodes der 0x58 Zeichensatzeintraege (" ABC...Z" Umlau */
extern char g_str_at[];                                 /* ds:7636 "@" (Zeilenumbruch; R20 print_text haengt ihn an) */
extern struct special_day g_special_days[];             /* ds:7638 besondere Tage (Monat, Tag, Nummer -> date.special), Ende mo */
extern signed char g_months_spring[];                   /* ds:76ba Monate des Fruehlings 10, 11, 12, -1 (date_season; kein eige */
extern signed char g_months_summer[];                   /* ds:76be Monate des Sommers 1, 2, 3, -1 (date_season; _g_763a+0x84) */
extern signed char g_months_winter[];                   /* ds:76c2 Monate des Winters 7, 8, 9, -1 (date_season; _g_763a+0x88) */
extern signed char g_sky_arc_y[];                       /* ds:76c6 Hoehe von Sonne/Mond je Position 0..0x4b der Himmelsanzeige  */
extern signed short g_conv_topic_tab[];                 /* ds:771e Textdatei je Thema 7..0x39 (O254; Zugriff [t - 7] in O257, gefa */
extern signed char g_sky_pos;                           /* ds:7712 zuletzt gezeichnete Position von Sonne/Mond (Date_draw) */
extern char g_str_time[];                               /* ds:7713 "TIME" Kennung im Spielstand (Date_save) */
extern char g_str_empty_7718[];                         /* ds:7718 "" (Einzahl in herb_effect; kein eigenes Public: _s_7713+5) */
extern char g_str_empty_7719[];                         /* ds:7719 "" (herb_effect; _s_7713+6) */
extern char g_str_empty_771a[];                         /* ds:771a "" (herb_effect; _s_7713+7) */
extern char g_str_empty_771b[];                         /* ds:771b "" (herb_effect; _s_7713+8) */
extern char g_str_empty_771c[];                         /* ds:771c "" (herb_effect; _s_7713+9) */
extern char g_str_xdia[];                               /* ds:7784 "XDIA" (Kennung im Spielstand, Conv::save/load) */
extern char g_fmt_conv_note[];                          /* ds:7789 "%s%s" (Conv::note: Kopf und Antwort) */
extern CONV_CB g_conv_callbacks[];                      /* ds:778e Rueckrufe je Gespraech (Conv.callback; Aufruf (0, Thema) ->  */
extern char far *g_script_keywords[];                   /* ds:784a Schluesselwoerter der Skripte "#", "PIC", "TOLERANCE", "TOPI */
extern char far *g_script_answer_kw[];                  /* ds:7866 Antwort-Schluesselwoerter "NULL", "LIST_TRUE", "LIST_FALSE", */
extern signed short g_never_confiscated[];              /* ds:78c6 Gegenstaende, die bei der Gefangennahme bleiben (O263), Ende */
extern char g_dng08_str_wow[];                          /* ds:78cc "Wow!" (O267 Fall 0x79: Wand (0x0b, 0x19) verschwindet) */
extern unsigned long g_anim_ticks;                      /* ds:78d2 Taktzaehler (Zeitgeber-Interrupt; O112 Kampfanimation) */
extern signed char g_mouse_isr_lock;                        /* ds:78d8 Sperre der Mausbehandlung (SEG023 mouse_isr; O199 waehrend  */
extern signed short g_edge_lo[208];                     /* ds:78da Kantentabelle R24: kleinste Koordinate je Spalte bzw. Zeile (7FFFh = leer) */
extern signed short g_edge_hi[208];                     /* ds:7a7a Kantentabelle R24: groesste Koordinate (8001h = leer) */
extern signed short g_edge_lo_z[208];                   /* ds:7c1a Kantentabelle R24: z am Ende lo (Nebel) */
extern signed short g_edge_hi_z[208];                   /* ds:7dba Kantentabelle R24: z am Ende hi */
extern signed short g_edge_lo_u[208];                   /* ds:7f5a Kantentabelle R24: Texturspalte u am Ende lo */
extern signed short g_edge_lo_v[208];                   /* ds:80fa Kantentabelle R24: Texturzeile v am Ende lo (kein eigenes Public) */
extern signed short g_edge_hi_u[208];                   /* ds:829a Kantentabelle R24: u am Ende hi (kein eigenes Public) */
extern signed short g_edge_hi_v[208];                   /* ds:843a Kantentabelle R24: v am Ende hi (kein eigenes Public) */
extern struct tex_ref g_cur_tex;                        /* ds:85da Lage der aktuellen Textur fuer die Fuellroutinen (R12 setzt, R24 laedt DS:ESI daraus) */
extern unsigned short g_cur_tex_w;                      /* ds:85e0 Breite der aktuellen Textur (texture.w; R12 -> R24, O198 Him */
extern unsigned short g_shade_segs[32];                 /* ds:85e4 Segmente der 32 Schattierungstabellen zu 256 Bytes in g_shad */
extern unsigned char g_fog_table[32];                   /* ds:8624 Helligkeitstabelle 32 Stufen (R12 fog_table/fade_table) */
extern char g_str_aif[];                                /* ds:865c "AIF" (Pic::check, Kennung im Dateiobjekt) */
extern char g_str_aif2[];                               /* ds:8660 "AIF" (Pic::check, Kennung im Archivobjekt) */
extern unsigned short g_random_seed;                    /* ds:869e Zufalls-Saat (R37 random_schweif; BE RANDOM_SCHICK_SEED) */
extern void far *g_scale_code;                          /* ds:86b1 Puffer 4 KB fuer den Skalier-Code, den R46 scale_lines erzeu */
extern void far *g_dng16_pal_save;                      /* ds:8b4e Dungeon 16: gesicherte Palette waehrend der Golem-Sequenzen */
extern signed char g_saved_area_type;                   /* ds:8b52 g_area_type beim Auslagern der 3D-Ansicht (O010 view_swap_ou */
extern signed short g_view_timer;                       /* ds:8b53 AIL-Zeitgeber der 3D-Ansicht (O010 view_timer_on/off) */
extern signed short g_cutscene_key;                     /* ds:8b56 letzte Taste der Zwischensequenz (cutscene_key, KEY_CTRL_Q = */
extern struct talk_desc far *g_cutscene_talk;           /* ds:8b58 Dialog fuer die Untertitel der Zwischensequenz (talk_cutscen */
extern void far *g_area_events;                         /* ds:8b5c Ereignisobjekt (O129) des Gebiets (O014 f_0c7c; R10 event_se */
extern signed char g_dng_light;                         /* ds:8b60 Dungeon: Grundhelligkeit fuer fade_table (O014 dng_make_even */
extern signed char g_dng_light2;                        /* ds:8b61 Dungeon: zweiter Wert fuer fade_table (O014) */
extern signed short g_fight_clock_save;                 /* ds:8b66 gesichertes g_clock_update waehrend des Kampfes */
extern signed short g_fight_rounds;                     /* ds:8b68 Zahl der Runden des letzten Kampfes */
extern void far *g_fight_buf;                           /* ds:8b6a Bildpuffer des Kampfes (= Fight.buf) */
extern char far *g_anim_big_buf;                        /* ds:8b6e Bildpuffer fuer grosse Figurenbilder (g_anim_big_buf_mem + 0 */
extern char far *g_anim_big_buf_mem;                    /* ds:8b72 Zusatzpuffer im Sonderkampf 0x493 (0x2f44 Bytes) */
extern signed short g_sfight_show_text;                 /* ds:8b76 Sonderkampf 0x493: Untertitel zeigen */
extern signed short g_sfight_hide_text;                 /* ds:8b78 Sonderkampf 0x493: Untertitel wegnehmen */
extern void far *g_sfight2_txt_buf;                     /* ds:8b7a Sonderkampf 0x495: Texte 0x43 (txt_load_list) */
extern char far * far *g_sfight2_txt;                   /* ds:8b7e Sonderkampf 0x495: Zeigerliste der Texte 0x43 */
extern signed char g_sfight2_right[];                   /* ds:8b82 Sonderkampf 0x495: genannte richtige Woerter (Textnummer, 3) */
extern signed char g_sfight2_wrong[];                   /* ds:8b85 Sonderkampf 0x495: genannte falsche Woerter (Textnummer, 3) */
extern signed short g_fli_error;                        /* ds:8b88 Fehler der FLI-Wiedergabe (FLI_ERR_*, O025) */
extern signed short g_fli_key;                          /* ds:8b8a Taste, die die FLI-Wiedergabe beendet hat (O024 fli_cb_key) */
extern unsigned short g_fli_max_frames;                 /* ds:8b8c Bildgrenze fuer fli_play_frames (O024 fli_cb_frames) */
extern signed char g_view_swapped;                      /* ds:8b8f Bildschirm/Ansicht ausgelagert (SEG001 do_travel_mode setzt */
extern void far *g_inv;                                 /* ds:8b90 aktives Heldenbogen-/Inventarobjekt (Klasse Inv, O008) */
extern signed char g_view_recam;                        /* ds:8b94 Kamera aus der Gruppenposition neu setzen (O014/O046) */
extern char g_init[];                                   /* ds:8b96 Startobjekt (class Init, R05; O032 globals_ctor/globals_dtor */
extern struct mouse_bg g_mouse_bg;                      /* ds:8b97 gesicherter Hintergrund unter dem Mauszeiger (struct mouse_b */
extern signed short g_mouse_bg_cw;                      /* ds:8b9b = g_mouse_bg.cw (eigenes Public; R18 mouse_set_cursor) */
extern signed short g_mouse_bg_ch;                      /* ds:8b9d = g_mouse_bg.ch (eigenes Public) */
extern signed short g_cursor_w;                         /* ds:8b9f Breite des Mauszeigerbilds = g_mouse_bg.w (eigenes Public; R */
extern signed short g_cursor_h;                         /* ds:8ba1 Hoehe des Mauszeigerbilds = g_mouse_bg.h (eigenes Public) */
extern signed short g_cursor_nr;                        /* ds:8ba3 Nummer des Mauszeigerbilds (-1 = aus Speicher; R12: 6..11 Be */
extern signed short g_cursor_nr_prev;                   /* ds:8ba5 vorige Nummer des Mauszeigerbilds (R18 mouse_set_cursor) */
extern signed short g_mouse_lpending;                   /* ds:8bab linker Klick wartet auf Doppelklick (R23 mouse_isr; nach 4 T */
extern signed short g_mouse_lclick;                     /* ds:8bad Maus-Merker */
extern signed short g_mouse_ldclick;                    /* ds:8baf Doppelklick links? (R02: Doppelklick auf Portraet oeffnet de */
extern signed short g_mouse_ldown;                      /* ds:8bb1 linke Maustaste gedrueckt (R23 mouse_isr; R12: 0 -> Richtung */
extern signed short g_mouse_rpending;                   /* ds:8bb3 rechte Taste, Gegenstueck zu g_mouse_lpending (nur R18 mouse */
extern signed short g_mouse_rclick;                     /* ds:8bb5 rechte Maustaste/Menue-Merker */
extern signed short g_mouse_rdclick;                    /* ds:8bb7 Doppelklick rechts (Gegenstueck zu g_mouse_ldclick; O193 set */
extern signed short g_mouse_rdown;                      /* ds:8bb9 rechte Maustaste gedrueckt (R23 mouse_isr) */
extern signed short g_mouse_event;                      /* ds:8bbb Maus-Ereignis anstehend (R18 fragt dann die Mausbereiche ab; */
extern unsigned char g_mouse_moved;                     /* ds:8bbd Mauszeiger neu zeichnen (R23/R18 setzen, mouse_update loesch */
extern signed char g_mouse_busy;                        /* ds:8bbe Zeiger wird gerade gezeichnet/geloescht (Sperre gegen R23 mo */
extern unsigned char g_mouse_irq_init;                  /* ds:8bbf Maus-Ereignis-Handler gesetzt (O193 mouse_irq_init/mouse_res */
extern unsigned char g_mouse_locked;                    /* ds:8bc0 Maus gesperrt (mouse_show/hide wirken nur bei 0) */
extern signed short g_mouse_posx;                       /* ds:8bc1 Maus x */
extern signed short g_mouse_posy;                       /* ds:8bc3 Maus y */
extern signed short g_cursor_hot_x;                     /* ds:8bc5 Hotspot des Mauszeigers x */
extern signed short g_cursor_hot_y;                     /* ds:8bc7 Hotspot des Mauszeigers y */
extern signed short g_mouse_clickx;                     /* ds:8bc9 Klickposition x */
extern signed short g_mouse_clicky;                     /* ds:8bcb Klickposition y */
extern signed short g_mouse_posx_bak;                   /* ds:8bcd Mausposition beim letzten Zeichnen x (R18 mouse_draw; BE MOU */
extern signed short g_mouse_posy_bak;                   /* ds:8bcf Mausposition beim letzten Zeichnen y (BE MOUSE_POSY_BAK) */
extern signed short g_mouse_show_cnt;                   /* ds:8bd1 Sichtbarkeitszaehler des Mauszeigers (0 = sichtbar; mouse_sh */
extern signed short g_mouse_flag;                       /* ds:8bd3 nur in O193 mouse_init_vars auf 0 gesetzt, wird nie gelesen */
extern signed short g_key_flag;                         /* ds:8bd5 Richtungsbits der Tastatur-ISR (1 vor, 2 zurueck, 4 links, 8 */
extern unsigned char far *g_cursor_nvf;                 /* ds:8bd7 NVF-Daten der Mauszeigerbilder (R18 mouse_set_cursor) */
extern void far *g_mouse_areas;                         /* ds:8bdb Mausbereichsliste (Hintergrund) */
extern void far *g_mouse_areas2;                        /* ds:8bdf Mausbereichsliste (Vordergrund) */
extern signed short g_key_ascii;                        /* ds:8be3 ASCII-Teil der letzten Taste (R18 poll_input) */
extern signed short g_key_scan;                         /* ds:8be5 Scancode der letzten Taste (R18 poll_input) */
extern signed short g_key_shift;                        /* ds:8be7 Umschalttasten (bioskey(2) & 3, R18 poll_input; 3 = beide -> */
extern unsigned short g_key;                            /* ds:8be9 letzte Taste (bioskey), ds:8beb Tastenpuffer */
extern signed short g_key_buf;                          /* ds:8beb Tastenpuffer (erste Taste ausser Cursortasten, R12 kbd_poll) */
extern signed short g_action;                           /* ds:8bed Eingabe-Aktion (Taste oder Mausbereich, R18 poll_input) */
extern signed short g_mouse_handler_on;                 /* ds:8bef Mauszeiger-Rueckruf aktiv (R12 install_cursor) */
extern void far *g_mouse_handler_areas;                 /* ds:8bf1 Mausbereiche fuer den Rueckruf (R18 f_0a9e) */
extern void (far *g_mouse_handler)(short);              /* ds:8bf5 Rueckruf mit dem Wert des Mausbereichs (R12 set_cursor, R02  */
extern void interrupt (*g_mouse_handler_bak)(void);     /* ds:8bf9 alter Vektor 79h (O193 mouse_irq_init; BE MOUSE_HANDLER_BAK) */
extern void interrupt (*g_old_int9)(void);              /* ds:8bfd alter Tastatur-Interrupt (R18 set_keyboard_isr) */
extern signed short g_cast_spell_id[];                  /* ds:8c01 [0] = zuletzt gewirkter Zauber (O233 cast_spell; O137 Duplic */
extern signed short g_spell_cancelled;                  /* ds:8c03 Zauber nicht gewirkt/abgebrochen: keine Animation (O233/O194 */
extern signed short g_spell_success;                    /* ds:8c05 Zauber gelungen (O233/O194 setzen 1; Animation mit Geschoss/ */
extern signed char g_spell_level;                       /* ds:8c07 gewaehlte Stufe von Nihilatio Gravitas/Solidirid (O234) */
extern signed short g_spell_cost_paid;                  /* ds:8c0c bezahlte AE des letzten Zaubers (O233 cast_spell) */
extern signed short g_spell_summon_mon;                 /* ds:8c0e Beschwoerung: Gegnerart (O234: 4/0x11/0x50) */
extern signed char g_spell_summon_renegade;             /* ds:8c10 Beschwoerung: Gegner kaempft fuer die Helden (enemy.renegade */
extern signed short g_spell_by_item;                    /* ds:8c11 Zauber aus einem Gegenstand: keine Pruefungen, Probe 99 (O01 */
extern struct spell_flags g_spell_flags;                /* ds:8c11 = g_spell_by_item .. g_spell_cost_mod als Struktur (O194/O23 */
extern signed short g_spell_no_sound;                   /* ds:8c13 Zauber ohne Sprachausgabe/Klang (O141/O158..O161 vor Gegnerz */
extern signed short g_spell_event;                      /* ds:8c15 Zauber auf ein Ereignis hin (Destructibo ohne Auswahl, Motor */
extern signed char g_spell_cost_mod;                    /* ds:8c17 Zuschlag auf die Zauberkosten (O233_003D; O047: -1 = Gegenst */
extern signed char far *g_item_demand;                  /* ds:8c18 Nachfragezaehler je Gegenstand (shop_trade: ab +10 bzw. -10  */
extern signed short g_cam_x;                            /* ds:8c1c Kamera x (168 je Zelle) */
extern signed short g_cam_y;                            /* ds:8c1e Kamera y */
extern signed short g_cam_z;                            /* ds:8c20 Kamera z (168 je Zelle) */
extern signed short g_cam_rx;                           /* ds:8c24 Kamera x innerhalb der Zelle (0..0xa7; O198 view_draw) */
extern signed short g_cam_ry;                           /* ds:8c26 Kamera y innerhalb der Zelle (O198: 0) */
extern signed short g_cam_rz;                           /* ds:8c28 Kamera z innerhalb der Zelle (0..0xa7) */
extern signed short g_cam_rot_x;                        /* ds:8c2c Kameradrehung fuer cam_set_angles (&g_cam_rot_x: x, y = g_ca */
extern signed short g_cam_rot_y;                        /* ds:8c2e Kameradrehung y (= g_cam_pitch; O198 Himmel) */
extern signed short g_cam_angle;                        /* ds:8c30 Blickrichtung 0..0x3ff (O014/O046/O198; R12) */
extern signed short g_cam_eye_x;                        /* ds:8c34 Augenversatz x (O198: 0; R25) */
extern signed short g_cam_eye_y;                        /* ds:8c36 Augenhoehe ueber dem Boden der Zelle (O198 aus den Wandprofi */
extern signed short g_cam_eye_z;                        /* ds:8c38 Augenversatz z (O198: 0; R25) */
extern signed short g_cam_ang[3];                       /* ds:8c3c Kamerawinkel (3) */
extern signed short g_cam_ang_y;                        /* ds:8c3e = g_cam_ang[1] (eigenes Public; R12 draw_stars) */
extern signed short g_cam_ang_z;                        /* ds:8c40 = g_cam_ang[2] (eigenes Public; R12 draw_stars) */
extern struct vtx g_cam_vec;                            /* ds:8c44 Bewegungsvektor ungedreht (0, 0, Geschwindigkeit << 8) */
extern struct vtx g_cam_dir;                            /* ds:8c4c Bewegungsvektor gedreht */
extern signed short g_cam_neg[3];                       /* ds:8c54 negierte Kamerawinkel */
extern signed long g_cam_fx;                            /* ds:8c5c Kamera x Festkomma (<< 8) */
extern signed long g_cam_fy;                            /* ds:8c60 Kamera y Festkomma (<< 8) */
extern signed long g_cam_fz;                            /* ds:8c64 Kamera z Festkomma (<< 8) */
extern void far *g_shade_buf;                           /* ds:8c70 Puffer der Schattierungstabellen (0x2010 Bytes; g_shade_segs */
extern signed short g_proj_dist;                        /* ds:8c78 Augabstand der Projektion (R25 view_set_projection: 32; R24 edge_trace_persp; in O032 g_8c74[8]) */
extern signed short g_proj_scale;                       /* ds:8c7a Massstab der Projektion * 1/16 (R25 view_set_projection: 90; in O032 g_8c74[8]) */
extern signed short g_nr_vis_faces;                     /* ds:8c7c Zahl der Eintraege in g_face_list (R25 obj_project, hoechstens 129) */
extern signed short g_nr_clip_faces;                    /* ds:8c7e belegte Plaetze im Puffer beschnittener Flaechen (R25 clip_face, hoechstens 16; R12 setzt 0) */
extern signed short g_nr_vtx;                           /* ds:8c82 Zahl projizierter Punkte (R12: > Puffer -> "Too many vectors */
extern signed short g_sin_table[1024];                  /* ds:8c84 Sinustabelle 1024 Eintraege, Festkomma 1.15 (Kosinus = Index */
extern signed short g_view_init_flag;                   /* ds:9484 3D-Ansicht: beim Laden auf 0x0f gesetzt (O014/O046), wird n */
extern signed short g_nr_textures;                      /* ds:9486 Zahl der Texturen im Bild-Cache (hoechstens 60; O097 tex_get */
extern char (far *g_tex_names)[12];                     /* ds:9488 Namen der Texturen im Bild-Cache (je 12 Zeichen; O097 tex_ge */
extern signed short g_nr_vtx_paths;                     /* ds:948e Zahl der bewegten Eckpunkte g_vtx_paths (O097 obj_read) */
extern unsigned short g_obj_key;                        /* ds:9490 Schluesselwort der Objektdateien (rueckgekoppeltes Schiebere */
extern signed short g_view_cell_y;                      /* ds:9492 Zelle der Gruppe y absolut (cam_set_from_group) */
extern signed short g_view_cell_x;                      /* ds:9494 Zelle der Gruppe x absolut (cam_set_from_group) */
extern signed short g_cam_cell_y;                       /* ds:9496 Kamerazelle y absolut (O198 view_draw; R12: group.y = g_cam_ */
extern signed short g_cam_cell_x;                       /* ds:9498 Kamerazelle x absolut (O198 view_draw; R12: group.x = g_cam_ */
extern signed short g_map_x0;                           /* ds:949a Kartenursprung x: Zelle x = group.x + g_map_x0 (O046, O183,  */
extern signed short g_map_y0;                           /* ds:949c Kartenursprung y: Zelle y = group.y + g_map_y0 (O046, O183,  */
extern signed short g_view_input_on;                    /* ds:949e 3D-Ansicht nimmt Bewegungseingaben an (SEG023 view_timer_cb */
extern signed short g_cam_pitch;                        /* ds:94a0 Blickhoehe/Neigung -128..128 (R12 move_input, Tasten 0x10/0x */
extern signed short g_cam_speed;                        /* ds:94a2 Geschwindigkeit -5..8 (R12 move_input -> cam_set_speed(v >>  */
extern signed short g_cam_roll;                         /* ds:94a4 Seitenneigung beim Drehen (R12 move_input) */
extern unsigned char far *g_face_depth;                 /* ds:94a6 Sortierschluessel je sichtbarer Flaeche (groesstes z, 1-basiert; Flaechenblock +0x102; R25) */
extern signed short far *g_face_next;                   /* ds:94aa Verkettung der sichtbaren Flaechen (Index, [0] = erste) */
extern unsigned short far *g_shade_table;               /* ds:94ae Helligkeit je Winkel 0..255 (R12 draw_object, FACE_SHADED) */
extern struct obj3d far *g_obj_cur;                     /* ds:94b2 aktuelles 3D-Objekt (O097, O198 view_draw) */
extern struct obj3d far * far *g_cell_objs;             /* ds:94b6 3D-Objekte je Kartenzellentyp (get_cell_a) */
extern struct vtx far *g_vtx_buf;                       /* ds:94ba Punktpuffer der 3D-Ausgabe (R25 obj_project; R12 sichert dre */
extern struct texture far *g_textures;                  /* ds:94be Bild-Cache der Texturen (60 Eintraege; O097 tex_get, O010 te */
extern struct face_ent far *g_face_list;                /* ds:94c2 sichtbare Flaechen (struct face_ent, 18 Bytes) */
extern struct vtx_path far *g_vtx_paths[32];            /* ds:94c6 bewegte Eckpunkte (O097 obj_read; O010; kein eigenes Public, */
extern signed short g_view_pal_n;                       /* ds:9546 belegte Farben der Palette g_view_pal (0xa0 nach dem Laden;  */
extern struct rgb g_view_pal[256];                      /* ds:9548 Palette der 3D-Ansicht (set_palette, Palettenbild; O010/O096 */
extern struct vis_order far *g_vis_order;               /* ds:9848 Sichtliste je Blickrichtung (Datei 0x405, 32 x 0x80 Bytes; O */
extern struct map_cell (far *g_cells)[MAP_SIZE];        /* ds:984c Zellentabelle der 3D-Karte 64 x 64 (struct map_cell, R17 bui */
extern unsigned short g_face_seg;                       /* ds:9850 Segment des Flaechenblocks (Paragraphengrenze; O096; R25) */
extern void far *g_face_block;                          /* ds:9852 Flaechenblock 0x61c Bytes (O096 view_alloc_lists/view_free_l */
extern BMP_PTR g_hud_top_left;                          /* ds:985c Rahmenteil der 3D-Ansicht oben links (Bmp; O014/O046 view_sa */
extern BMP_PTR g_hud_top_right;                         /* ds:9860 Rahmenteil oben rechts (Bmp) */
extern BMP_PTR g_hud_bottom_left;                       /* ds:9864 Rahmenteil unten links (Bmp) */
extern BMP_PTR g_hud_bottom_right;                      /* ds:9868 Rahmenteil unten rechts (Bmp) */
extern BMP_PTR g_render_bmp;                            /* ds:986c Bmp ueber g_renderbuf_ptr (320 x 135; O091 gibt frei) */
extern signed long g_pick_event;                        /* ds:9870 angeklicktes Ereignis (struct map_event als long, -1 = keins */
extern signed short g_pick_depth;                       /* ds:9874 Tiefe der angeklickten Flaeche */
extern signed short g_pick_angle;                       /* ds:9876 Winkel der angeklickten Flaeche (pick_center: >= 80) */
extern struct map_event g_cell_event;                   /* ds:9878 Ereignis der betretenen Zelle (R12 cell_event; O014/O046 cal */
extern signed short g_nr_click;                         /* ds:987c Zahl der Klickflaechen im Bild (hoechstens 8) */
extern signed long g_click_event[8];                    /* ds:987e Ereignis je Klickflaeche (struct map_event als long) */
extern signed short g_click_depth[8];                   /* ds:989e mittlere Tiefe je Klickflaeche */
extern signed short g_click_angle[8];                   /* ds:98ae Winkel je Klickflaeche (face_ent.angle) */
extern struct rect g_click_rect[8];                     /* ds:98be Bildschirmrechteck je Klickflaeche */
extern struct cell_xy g_prof_cells[4];                  /* ds:98fe Zellen der 4 Profilplaetze (-1 = frei) */
extern signed short g_prof_zmin[4][6][32];              /* ds:990e Wandprofil: kleinste Tiefe [Platz 4][Flaeche 6][Spalte 32] ( */
extern signed short g_prof_zmax[4][6][32];              /* ds:9f0e Wandprofil: groesste Tiefe */
extern signed short g_prof_ymin[4][6][32];              /* ds:a50e Wandprofil: Hoehe bei kleinster Tiefe */
extern signed short g_prof_ymax[4][6][32];              /* ds:ab0e Wandprofil: Hoehe bei groesster Tiefe */
extern signed short g_prof_count[4];                    /* ds:b10e Flaechen je Profilplatz */
extern signed short g_prof_next;                        /* ds:b11a naechster Profilplatz (0..3) */
extern signed short g_view_indoor;                      /* ds:b11c vermutet: 3D-Ansicht mit Decke (Dungeon ausser 4 -> 1, Stad */
extern signed char g_step_active;                       /* ds:b11e Schrittbewegung laeuft (R12 move_input) */
extern signed char g_step_pending;                      /* ds:b11f vermutet: Schrittmerker der 3D-Bewegung, wird bei jedem neu */
extern signed short g_step_target;                      /* ds:b120 Zielwinkel der Schrittdrehung (Vielfaches von 0x100) */
extern signed short g_step_turn;                        /* ds:b122 Drehschritt je Bild (-8/0/8) */
extern signed short g_move_flags;                       /* ds:b124 Bewegungsbits (1 vor, 2 zurueck, 4 drehen, 8; R12/R13) */
extern signed short g_step_flags;                       /* ds:b126 Bewegungsbits der laufenden Schrittbewegung (-> g_move_flags */
extern void far *g_renderbuf_ptr;                       /* ds:b128 Zeichenpuffer der 3D-Ansicht (BE RENDERBUF_PTR; R11/R12) */
extern signed short g_skip_cell_event;                  /* ds:b12c Betreten-Ereignis der naechsten Zelle unterdruecken (O129 ev */
extern signed short g_view_active;                      /* ds:b12e 3D-Ansicht eingerichtet (O096 view_alloc 1, view_leave 0; O0 */
extern signed short g_tex_anim_frame;                   /* ds:b132 aktuelles Bild der Texturanimation (O097, O198) */
extern signed short g_tex_anim_frames;                  /* ds:b134 Bilder der Texturanimation (3/4; O097) */
extern signed short g_tex_anim_due;                     /* ds:b136 Texturanimation faellig (Zeitgeber R23 setzt 1, O198 schalte */
extern signed short g_tex_anim_rate;                    /* ds:b138 Takte je Bild der Texturanimation (0 = aus; O097, Zeitgeber  */
extern signed short g_tex_anim_tick;                    /* ds:b13a Taktzaehler der Texturanimation (Zeitgeber R23) */
extern struct texture far *g_tex_anim_a[4];             /* ds:b13c Texturanimation: Bilder der ersten Flaechen (O097; kein eige */
extern struct texture far *g_tex_anim_b[4];             /* ds:b14c Texturanimation: zweite Bilderfolge */
extern struct texture far *g_tex_anim_c[4];             /* ds:b15c Texturanimation: dritte Bilderfolge */
extern unsigned short g_ems_frame;                      /* ds:b16c EMS-Seitenrahmen (Segment; R02 EMS_copy) */
extern signed short g_swap_kb;                          /* ds:b16e Groesse des Auslagerungsspeichers der 3D-Ansicht in KB (700, */
extern unsigned char g_star_colors[8];                  /* ds:b170 Farben der Sterne nach Entfernung (8) */
extern signed short g_stars_reset;                      /* ds:b178 vor dem Erzeugen der Sterne auf 0 gesetzt (O096 view_init_s */
extern struct vtx g_stars[100];                         /* ds:b17a 100 Sterne des Nachthimmels (R12 draw_stars) */
extern signed short g_icon_group;                       /* ds:b49a Anzeige: andere Gruppe auf dem Feld (O014/O046) */
extern signed short g_icon_groups;                      /* ds:b49c Anzeige: mehr als eine Gruppe (O014/O046) */
extern signed short g_icon_camp;                        /* ds:b49e Anzeige: Rasten erlaubt (O014/O046) */
extern signed short g_view_input_blocked;               /* ds:b4a0 3D-Ansicht: Mausauswahl gesperrt (R12 move_input nur bei 0; */
extern void far *g_cell_occludes;                       /* ds:b4a2 Verdeckung je Feldtyp der Karte (unsigned char, Tabelle je S */
extern unsigned char g_near_objs[81];                   /* ds:b4a6 3D-Ansicht: Objekt-Nr. + 1 der 9x9 Nachbarzellen (0 = nicht  */
extern unsigned char g_near_occl[81];                   /* ds:b4f7 3D-Ansicht: Verdeckung der 9x9 Nachbarzellen (g_cell_occlude */
extern signed short g_cursor_no;                        /* ds:b550 aktueller Mauszeiger der Kartenansicht (R08 cursor_for_cell; */
extern void far *g_names_star;                          /* ds:b552 Namensliste STAR (Daten; filename.ltx bzw. 20 Bytes bei star */
extern void far *g_names_star_idx;                      /* ds:b556 Namensliste STAR (Zeigertabelle) */
extern void far *g_names_fx;                            /* ds:b55a Namensliste FX (fxname.ltx) */
extern void far *g_names_fx_idx;                        /* ds:b55e Namensliste FX (Zeigertabelle) */
extern void far *g_names_speech;                        /* ds:b562 Namensliste SPEECH (pakname.ltx) */
extern void far *g_names_speech_idx;                    /* ds:b566 Namensliste SPEECH (Zeigertabelle) */
extern void far *g_names_raw;                           /* ds:b56a Namensliste RAW (rawname.ltx) */
extern void far *g_names_raw_idx;                       /* ds:b56e Namensliste RAW (Zeigertabelle) */
extern void far *g_names_starcd;                        /* ds:b572 Namensliste STARCD (cdfname.ltx) */
extern void far *g_names_starcd_idx;                    /* ds:b576 Namensliste STARCD (Zeigertabelle) */
extern void far *g_names_speechcd;                      /* ds:b57a Namensliste SPEECHCD (cdpname.ltx) */
extern void far *g_names_speechcd_idx;                  /* ds:b57e Namensliste SPEECHCD (Zeigertabelle) */
extern void far *g_patch_starcd;                        /* ds:b582 Patchtabelle dcdpatch.dat (7 Bytes, R05) */
extern void far *g_patch_speechcd;                      /* ds:b586 Patchtabelle scdpatch.dat (8 Bytes) */
extern void far *g_patch_star;                          /* ds:b58a Patchtabelle dpatch.dat (0x9c Bytes, nur ONEFILE-Fassung) */
extern void far *g_patch_fx;                            /* ds:b58e Patchtabelle fpatch.dat (0x17 Bytes) */
extern void far *g_patch_speech;                        /* ds:b592 Patchtabelle spatch.dat (0x1f Bytes) */
extern void far *g_patch_raw;                           /* ds:b596 Patchtabelle rpatch.dat (g_nr_raw_files / 8 + 1 Bytes) */
extern char g_dialog[];                                 /* ds:b59a Dialogfenster (class Dlg, O190; C++: (Dlg far *)g_dialog) */
extern signed short g_dialog_sel;                       /* ds:b5aa = Dlg.sel von g_dialog (eigenes Public): vorgewaehlte Zeile  */
extern signed short g_silent_mode;                      /* ds:b5ba nie gesetzt (BSS, immer 0): Schalter fuer einen Modus ohne  */
extern signed short g_blink_off;                        /* ds:b5bc Blinkzeichen aus (O009/O187 nur bei 0); wird nie gesetzt */
extern signed short g_fight_running;                    /* ds:b5dc Kampfobjekt besteht (O018: 1 im Konstruktor, 0 im Destruktor */
extern signed short g_silent_mode2;                     /* ds:b5e0 nie gesetzt (BSS, immer 0): wie g_silent_mode (R12 move_inp */
extern signed char g_dlg_box_flag;                      /* ds:b61e vermutet: Merker des Dialogfensters, vor vielen dlg_box_bmp */
extern char g_bmp_spare[];                              /* ds:b627 Bmp-Objekt (O032 globals_dtor: gb_delete), sonst nirgends b */
extern char g_ofile[];                                  /* ds:b635 Arbeitsdatei in ..\TEMP (class TempFile, O243) */
extern void far *g_cd;                                  /* ds:b639 CD-Objekt (O004 CD_music/CD_update) */
extern char g_nvf_heads[];                              /* ds:b63d NVF-Dateiobjekt (class Nvf, O098) fuer Portraets/Bilder */
extern signed short g_map_w;                            /* ds:b656 Breite der 3D-Karte (Kartenobjekt ds:b656 +0, struct map_gri */
extern signed short g_map_h;                            /* ds:b658 Hoehe der 3D-Karte (Kartenobjekt ds:b656 +2) */
extern signed short g_pp20_index;                       /* ds:b666 Nummer des angezeigten Vollbilds (BE DSA1 PP20_INDEX; 0x94 R */
extern void far *g_vga_buf;                             /* ds:b668 Bildschirmpuffer (A000:0000; == g_vga_buf -> Maus aus/ein) */
extern unsigned char far *g_loc_icon_buf;               /* ds:b66c Puffer der 9 Symbole je 24 x 24 (Screen::load_icon) */
extern char far *g_text_output_buf;                     /* ds:b670 Textpuffer fuer sprintf-Meldungen (ds:b670) */
extern char far *g_text_buf2;                           /* ds:b674 zweiter Textpuffer (Dateinamen, Meldungen) */
extern void far *g_ttx_buf;                             /* ds:b678 Textliste der Haupttexte GLOBLTXT.LTX (zu g_ttx; O032 init_m */
extern char far * far *g_ttx;                           /* ds:b67c Zeigerliste der Haupttexte (b67c[0x193] MR-Meldung ...) */
extern unsigned char far *g_pal_buf;                    /* ds:b680 Palettenpuffer 0x300 Bytes (O032 legt ihn an; R42 gb_new_fil */
extern unsigned char far *g_pal_dark;                   /* ds:b684 Palettenpuffer Ausblendziel (pal_fade_out/pal_fade_in; danac */
extern struct video_state g_video_state;                /* ds:b688 beim Start gesicherter Videozustand (R43_003C; R43 restore_v */
extern char g_input_buf[];                              /* ds:b690 Eingabepuffer von dlg_message_flag (Namen, Spielstand; Schn */
extern unsigned char g_icon_pic_group;                  /* ds:b6b9 Bild der Anzeige g_icon_group (0x0f/0x2d; O188 screen_draw_i */
extern unsigned char g_icon_pic_groups;                 /* ds:b6ba Bild der Anzeige g_icon_groups (0x1d/0x44) */
extern unsigned char g_icon_pic_camp;                   /* ds:b6bd Bild der Anzeige g_icon_camp (0x41 = Rasten moeglich, 0x2f) */
extern char far *g_menu_texts[];                        /* ds:b6c1 Zeiger auf die Texte einer Auswahl (dlg_choose; O074 12 Eint */
extern signed char g_menu_tail[];                       /* ds:b6ff vermutet: Fortsetzung von g_menu_texts (85 Bytes; O006 lies */
extern unsigned char huge *g_snd_buf0;                  /* ds:b754 Abspielpuffer 0 fuer digitale Klaenge (0x2800 Bytes, R04) */
extern unsigned char huge *g_snd_buf1;                  /* ds:b758 Abspielpuffer 1 fuer digitale Klaenge (0x2800 Bytes, R04) */
extern signed short g_snd_vol_req;                      /* ds:b75c snd_set_vol zweiter Wert; 0x7f -> Musiklautstaerke nachfuehr */
extern signed short g_snd_vol_req2;                     /* ds:b75e snd_set_vol erster Wert (R04) */
extern struct hero far *g_heroes;                       /* ds:b760 Zeiger auf die sieben Heldenstrukturen (je 0x5ef) */
extern char g_bars[];                                   /* ds:b769 Balkenanzeige-Objekt der Portraetleiste (struct bars, O057;  */
extern signed short g_status_on;                        /* ds:b76d vermutet: Statusanzeigen an - Win::prepare sichert und loes */
extern char g_screen[];                                 /* ds:b77e Spielbildschirm (class Screen, O188) */
extern char g_screen_pic[];                             /* ds:b77f Bildobjekt im Bildschirmobjekt (Hintergrund-Animationen) */
extern unsigned char far *g_screen_pic_pal;             /* ds:b799 Palette der Hintergrund-Animation im Bildobjekt g_screen_pi */
extern signed short g_screen_pic_state;                 /* ds:b7b4 Feld +0x35 des Bildobjekts g_screen_pic: 0 vor dem Abspiele */
extern char g_snd_speech[];                             /* ds:b848 Klangobjekt Sprachausgabe (R04_05ED) */
extern struct item_desc far *g_items;                   /* ds:b84a Gegenstandsbeschreibungen (0x0e Bytes je Eintrag) */
extern char far * far *g_item_names;                    /* ds:b84e Gegenstandsnamen */
extern char g_date[];                                   /* ds:b856 Spielzeit-/Datumsobjekt (R21 timewarp_*) */
extern signed long g_daytime;                           /* ds:b857 Tageszeit in Ticks (Teil von g_date) */
extern signed char g_date_weekday;                      /* ds:b85b Wochentag 0..6 (Teil von g_date, game_date.weekday) */
extern unsigned char g_date_day;                        /* ds:b85d Tag (Teil von g_date; O005 vergleicht 16.12.17) */
extern unsigned char g_date_month;                      /* ds:b85e Monat (Teil von g_date) */
extern unsigned char g_date_year;                       /* ds:b85f Jahr (Teil von g_date) */
extern char g_dialog2[];                                /* ds:b86e Dialog-Objekt (meist) */
extern char g_decomp_work[];                            /* ds:bda8 138 Bytes Arbeitsbereich, unbenutzter erster Parameter von  */
extern char g_weather[];                                /* ds:be32 Wetter-/Klimaobjekt (Wx_next je Tag, O005) */
extern char g_map[];                                    /* ds:be36 Kartenobjekt (Klasse Map, 0x28d) */
extern char g_diary[];                                  /* ds:c0c3 Tagebuchobjekt (O103) */
extern signed short g_market_buy;                       /* ds:c0ee Laden 0x18 (Marktstand, Ladendaten 13 Bytes ab hier wie O226 */
extern struct shop_info g_market_shop;                  /* ds:c0ee Ladendaten des Marktstands (13 Bytes, Sicht ueber g_market_b */
extern signed short g_market_sell;                      /* ds:c0f0 Laden 0x18: Aufschlag Verkauf in % */
extern signed char g_market_type;                       /* ds:c0f2 Laden 0x18: Art (1 Waffen, 2 Kraeuter, 3 Kraemer) */
extern signed char g_market_lvl;                        /* ds:c0f3 Laden 0x18: Stufe */
extern signed char g_market_female;                     /* ds:c0fa Laden 0x18: Haendlerin */
extern char g_travel[];                                 /* ds:c0fb Reiseobjekt (class Travel, 0x11c Bytes ab hier, weitere Pub */
extern signed short g_travel_rose_x;                    /* ds:c149 = Travel.rose_x von g_travel (eigenes Public): Kompassrose (O1 */
extern signed short g_travel_rose_y;                    /* ds:c14b = Travel.rose_y (eigenes Public; O187) */
extern unsigned char g_route_km;                        /* ds:c1c5 Laenge des aktuellen Weges in km (O072: x 1000 gegen group. */
extern signed char g_camp_weather;                      /* ds:c1ca Wetter 0..5 fuer die Lagerzuschlaege (Wasser/Jagd/Kraeuter  */
extern char g_tmap[];                                   /* ds:c217 Kartenobjekt der Reisekarte (Klasse O187) */
extern void far *g_tmap_buf;                            /* ds:c307 Bildspeicher der Reisekarte (0x11e30 Bytes, travel_enter) */
extern void far *g_tmap_ptr;                            /* ds:c30b zweiter Zeiger auf g_tmap_buf (travel_enter) */
extern struct group g_groups[NR_GROUPS];                /* ds:c30f die sechs Gruppen (je 0x49 Bytes) */
extern struct group far *g_group;                       /* ds:c4c5 aktive Gruppe (Zeiger in g_groups) */
extern struct hero far *g_sheet_hero;                   /* ds:c4c9 Held im Heldenbogen (add_equip_boni: Anzeige bei g_pp20_inde */
extern signed short g_in_fight;                         /* ds:c4cd Kampf laeuft (Kampfschleife O018) */
extern signed char g_fig_enemies_escaped;               /* ds:c4cf Zahl der ueber den Rand geflohenen Gegner (fig_enemy_move) */
extern signed short g_refresh;                          /* ds:c4d0 Merker: Bildschirm neu aufbauen */
extern signed short g_refresh_portraits;                /* ds:c4d2 Merker: Portraets neu (O076 bei Tod/Bildwechsel) */
extern signed short g_hero_died;                        /* ds:c4d4 Merker: Held gestorben ausserhalb des Kampfs (O076_01F3) */
extern signed short g_bg_restored;                      /* ds:c4d6 Bildschirmbereich wurde zurueckgeschrieben (Win::rest */
extern signed short g_in_wait;                          /* ds:c4d8 1 waehrend wait_ticks die Eingabe pollt */
extern signed char g_on_boat;                           /* ds:c4da Bootsfahrt (O126: 1 gebucht - ALL_SECT.LTX[116] "In %s habe */
extern struct mem_move g_xms_move;                      /* ds:c4db XMS-Kopierauftrag fuer xms_move_any (R06, O034) */
extern char far * far *g_mon_names;                     /* ds:c4ef Gegnernamen je Gegnerart (enemy_name) */
extern void far *g_mon_names_buf;                       /* ds:c4f3 Textpuffer der Gegnernamen (Texte 0x26) */
extern struct enemy far *g_enemies;                     /* ds:c4f7 Gegner im Kampf (0x49 Bytes je Eintrag) */
extern void far *g_portrait_buf;                        /* ds:c4fb Puffer fuer ein Portraet (O076_0BD6) */
extern void far *g_indos_flag;                          /* ds:c4ff Zeiger auf das InDOS-Byte (O032: dos_get_indos_ptr, INT 21h/34h; R23 Zeitgeber nur bei 0) */
extern signed char g_hero_slot_aux[4][7];               /* ds:c503 vier Bytes je Heldenplatz [4][7], beim Heldentausch mitgeta */
extern signed char g_speech_on;                         /* ds:c51f Sprachausgabe eingeschaltet (R04 snd_speech; O042_0f97-Abfra */
extern signed char g_ail_timer_users;                   /* ds:c521 Zahl der Benutzer des AIL-Zeitgebers (0 -> AIL_startup / AIL */
extern signed long g_mem_min_free;                      /* ds:c524 kleinster gesehener freier Speicher (farcoreleft, R07) */
extern signed long g_mem_free_start;                    /* ds:c528 freier Speicher beim Start (farcoreleft; O032 init_memory, O */
extern void far *g_anim_cur;                            /* ds:c52c aktuelles Animationsobjekt (Klasse Anim; Anim_ctor/Anim_dtor */
extern signed short g_anim_running;                     /* ds:c530 Animationsobjekt laeuft (Anim_start setzt 1, Anim_stop/advan */
extern void far *g_fight_obj;                           /* ds:c532 Kampfobjekt (im Kampf != 0) */
extern signed char g_snd_service_due;                   /* ds:c537 Klangdienste faellig (R23 Timer setzt 1; R18 poll_input ruft */
extern signed short g_anim_timer;                       /* ds:c538 Zeitgebertakte seit dem letzten Bildwechsel (Zeitgeber R23 z */
extern char g_fight_list[];                             /* ds:c53a Kampf-Objektliste (O175) */
extern signed short g_fig_anim_removed;                 /* ds:c614 Merker: Figur entfernt (fig_anim_event Ereignis 32) */
extern signed char g_fig_target_killed;                 /* ds:c688 Ziel der laufenden Aktion faellt (tot): Sturzbild statt Stan */
extern signed char g_fig_action_flag;                   /* ds:c689 vermutet: Merker einer Heldenaktion, vor jeder Aktion auf 0 */
extern signed char g_enemy_unmushroomed;                /* ds:c68a Gegnerzauber Verwandlung beenden hat einen Pilz zurueckverw */
extern signed short g_fight_max_enemies;                /* ds:c68b Kampfstaerke: nur so viele Gegner der Aufstellung (O022; O11 */
extern signed short g_save_fd;                          /* ds:c68d Dateinummer des offenen Spielstands (O106 Speichern/Laden, O */
extern signed short g_date_flag;                        /* ds:c68f Date_ctor setzt 0, sonst unbenutzt */
extern signed short g_clock_update;                     /* ds:c691 Zeitanzeige nachfuehren (R01 game_idle ruft dann O250_0299(g */
extern signed short g_is_day;                           /* ds:c693 Tag (1) bzw. Nacht (0), gesetzt beim Zeichnen der Uhr */
extern signed short g_menu_line;                        /* ds:c696 Menuezeile unter der Maus / gewaehlte Zeile 1..n (R02 set_m */
extern signed short g_timers_lock;                      /* ds:c698 Sperrwort fuer do_timers (R01 game_idle, R21 timewarp ueberg */
extern signed short g_ems_avail;                        /* ds:c69a EMS vorhanden (R05 Init_ctor = ems_present(); R12: Texturen  */
extern signed long g_swap_handle;                       /* ds:c69c Handle des Auslagerungsspeichers der 3D-Ansicht (EMS bzw. XM */
extern signed short g_swap_used_kb;                     /* ds:c6a0 belegte KB im Auslagerungsspeicher (g_swap_pos / 1024; O097, */
extern signed long g_tex_base;                          /* ds:c6a2 Basisadresse der Texturen ohne EMS */
extern signed long g_swap_pos;                          /* ds:c6a6 Schreib-/Leseposition im Auslagerungsspeicher (O010; R05 set */
extern void far *g_view_swap_pos;                       /* ds:c6aa 3D-Ansicht ausgelagert: Lage im Auslagerungsspeicher (signed */
extern signed long g_view_swap_end;                     /* ds:c6ae Ende der ausgelagerten 3D-Ansicht im Auslagerungsspeicher (O */
extern signed short g_swap_nr_objs;                     /* ds:c6b2 Zahl der ausgelagerten 3D-Objekte (O010 view_swap_out/in) */
extern char far *g_grammar_bufs;                        /* ds:c6b4 4 x 40 Bytes fuer grammar_article (O032 legt sie an; BE GRAM */
extern char far *g_grammar_name_buf;                    /* ds:c6b8 Puffer fuer grammar_plural/grammar_name (O032; BE GRAMMAR_BU */
extern signed short g_automap_redraw;                   /* ds:c6c0 Automap im Stadtmodus neu zeichnen (O046 bei 1; O052 setzt  */
extern signed short g_c6c2;                             /* ds:c6c2 (O014/O046/O030: Ansicht neu einrichten) */
extern signed short g_dialog_xms;                       /* ds:c6c4 XMS-Handle fuer den Hintergrund des Dialogfensters (Win::Win */
extern unsigned long g_tamper_count;                    /* ds:c6c6 Manipulationszaehler (R23 check_integrity +1 je Fehlbefund; R10: > 50 -> LE-Verlust; O107, O144) */
extern char g_copyprot[];                               /* ds:c6ca Handbuchabfrage (class CopyProt, 0x4b Bytes, SEG003; O032 CopyProt_ctor/_dtor) */
extern signed char g_mem_saving;                        /* ds:c715 Sparbetrieb wegen knappen Speichers (R05 swap_mem_init; O014 */
extern signed short g_swap_handle2;                     /* ds:c716 zweiter Speicherblock (XMS 351 KB) bzw. im Sparbetrieb = g_s */
extern signed short g_anim_no_skip;                     /* ds:c718 verpasste Takte nicht durch Ueberspringen von Bildern nachho */
extern char far *g_version_str;                         /* ds:c71a Versionszeile "Sternenschweif Version ..." (50 Bytes; O032 m */
extern unsigned char far *g_palette;                    /* ds:c71e Palette (Zeiger, 0x300 Bytes; O032 legt sie an, R02 set_pale */
extern struct mem_block far *g_mem_blocks;              /* ds:c722 Belegungstabelle der Speicherverwaltung (1000 x struct mem_b */
extern char g_start_dir[];                              /* ds:c726 Startverzeichnis "X:\..." (R05 Init_setdir; Init_dtor kehrt  */
extern char g_start_dir_path[];                         /* ds:c729 = g_start_dir + 3: Pfad ohne Laufwerk (eigenes Public; getcu */
extern signed short g_select_shown;                     /* ds:c776 Auswahlbildschirm (O034 select_screen): angezeigte Markieru */
extern signed short g_select_next;                      /* ds:c778 Auswahlbildschirm: gewuenschte Markierung 1..18 (Tasten, R0 */
extern BMP_PTR g_sel_frame_bmp;                         /* ds:c77a Auswahlbild: gesicherter Hintergrund des Rahmens (Bmp 0x22 x */
extern BMP_PTR g_sel_text_bmp;                          /* ds:c77e Auswahlbild: gesicherter Hintergrund des Textes (Bmp 0xb4 x  */
extern void far *g_sel_txt_buf;                         /* ds:c782 Auswahlbild: Puffer der Texte 0x36a */
extern char far * far *g_sel_txt;                       /* ds:c786 Auswahlbild: Texte 0x36a */
extern void far *g_sel_saved_areas;                     /* ds:c78a Auswahlbild: gesicherte g_mouse_handler_areas */
extern void (far *g_sel_saved_handler)(short);          /* ds:c78e Auswahlbild: gesicherter g_mouse_handler */
extern signed short g_sel_saved_on;                     /* ds:c792 Auswahlbild: gesicherter g_mouse_handler_on */
extern signed short g_finale_key;                       /* ds:c794 letzte Taste waehrend des Spielende-Skripts (Alt-Q 0x1011 be */
extern signed short g_finale_text_nr;                   /* ds:c796 Nummer des angezeigten Texts im Spielende-Skript (O036) */
extern char far *g_finale_text;                         /* ds:c798 angezeigter Text im Spielende-Skript (O036) */
extern struct finale_step far *g_finale_step;           /* ds:c79c naechster Schritt des Spielende-Skripts (O036) */
extern char far * far *g_finale_texts;                  /* ds:c7a0 Textliste der Spielende-Sequenz (Finale::txt; O036) */
extern void far *g_ttxt2_buf;                           /* ds:c7a4 Puffer der Reisetexte 0xad */
extern char far * far *g_ttxt2;                         /* ds:c7a8 Reisetexte 0xad (Zustandsanzeige O216) */
extern void far *g_ttxt3_buf;                           /* ds:c7ac Puffer der Reisetexte 0x1f1 */
extern char far * far *g_ttxt3;                         /* ds:c7b0 Reisetexte 0x1f1 (Feldnummer eingeben, O219/O223) */
extern void far *g_tev_buf;                             /* ds:c7b4 Puffer der Reiseereignistexte 0x13e (travel_enter, O164, O21 */
extern char far * far *g_tev_txt;                       /* ds:c7b8 Reiseereignistexte 0x13e (O164, O210) - frueher g_ltx_town */
extern void far *g_tev_loc_buf;                         /* ds:c7bc Puffer der Ereignistexte des Ortes (travel.evtxt_file) */
extern char far * far *g_tev_loc_txt;                   /* ds:c7c0 Ereignistexte des Ortes (travel.evtxt_file; O164) - frueher  */
extern void far *g_ttxt_buf;                            /* ds:c7c4 Puffer der Reisetexte 0xa4 */
extern char far * far *g_ttxt;                          /* ds:c7c8 Reisetexte 0xa4 (Menue der Reisekarte, Rueckfragen O223) */
extern struct travel_place far *g_places;               /* ds:c7cc Ortstabelle der Reisekarte (3 Bytes je Ort, Datei 0xa1; trav */
extern struct route far *g_routes;                      /* ds:c7d0 Wegtabelle der Reisekarte (0x106 Wege zu 10 Bytes, Datei 0xa */
extern struct loot_item far *g_tev049_loot;             /* ds:c7d4 Weg 49: Fundstuecke im verlassenen Holzfaellerlager (Datei  */
extern signed short g_points_route;                     /* ds:c7d8 Weg, dessen Wegpunkte geladen sind (route_load_points; trave */
extern signed short g_tev_reload;                       /* ds:c7da Ereignistexte nach travel_leave neu laden (travel_enter) */
extern signed short g_points_len;                       /* ds:c7dc Laenge der geladenen Wegpunkte in Bytes - 8 (route_load_poin */
extern signed short g_snd_mouse_hidden;                 /* ds:c7de Maus waehrend eines digitalen Klangs ausgeblendet (R04) */
extern signed short g_snd_hide_mouse;                   /* ds:c7e0 Maus waehrend digitaler Klaenge ausblenden (R04) */
extern unsigned short g_timbre_size;                    /* ds:c7e2 Laenge der gelesenen Klangfarbe (O042 Sound::timbre) */
extern struct timbre_entry g_timbre_entry;              /* ds:c7e4 gelesener Eintrag der Klangfarbenbibliothek (O042 Sound::tim */
extern signed short g_ems_free_pages;                   /* ds:c7ea freie EMS-Seiten zu 16 KB beim Start (R05 Init_ctor; O011 ze */
extern signed short g_town_steps;                       /* ds:c7ec Schrittzaehler der Stadtansicht (alle 8 Schritte Ueberfall-W */
extern TOWN_PTR g_town_events;                          /* ds:c7ee Ereignisobjekt der Stadt (class Town, O046 town_make_events; */
extern struct compass far *g_compass;                   /* ds:c7f2 Kompassobjekt der 3D-Ansicht (R11 Compass_ctor; O010/O014/O0 */
extern signed short g_view_pos_x;                       /* ds:c7f6 Gruppe/Kamera x innerhalb der Karte (cam_z - map_x0 * 168;  */
extern signed short g_view_pos_y;                       /* ds:c7fa Gruppe/Kamera y innerhalb der Karte (cam_x - map_y0 * 168;  */
extern signed short g_mem_checking;                     /* ds:c7fe mem_check laeuft (Rekursionssperre, R07) */
extern signed short g_amap_gfx;                         /* ds:c800 Kacheldatei der Automap (automap_open; Zoomwechsel) */
extern signed short g_amap_tick;                        /* ds:c802 Taktzaehler der Markenanimation 5..0 (automap_animate) */
extern signed short g_amap_frame;                       /* ds:c804 Bild 0..5 der Markenanimation (automap_animate) */
extern signed char far *g_mon_genders;                  /* ds:c806 Geschlecht je Gegnerart (Satzbyte 0x2f, 0x7b Bytes; O018 leg */
extern void far *g_binge_txt_buf;                       /* ds:c80a Puffer der Binge-Texte BINGE.LTX (Datei 0x2f9; O059 txt_loa */
extern char far * far *g_binge_txt;                     /* ds:c80e Binge-Texte BINGE.LTX (O059) */
extern signed short g_peddler;                          /* ds:c812 fahrender Haendler 1..3 (O013 aus dem Gespraech, 0 = keiner; */
extern signed short g_peddler_stock;                    /* ds:c814 fahrender Haendler: Vorrat in Plaetzen (O013: 15/10/10), dan */
extern signed short g_peddler_markup;                   /* ds:c816 fahrender Haendler: Aufschlag in % (shop_info.markup; O013,  */
extern signed short g_peddler_markup_food;              /* ds:c818 fahrender Haendler: Aufschlag in % fuer Nahrung (shop_info.m */
extern signed char g_peddler_type;                      /* ds:c81a fahrender Haendler: Art SHOP_WEAPONS/SHOP_GENERAL (shop_info */
extern signed char g_peddler_lvl;                       /* ds:c81b fahrender Haendler: Stufe der Waren (shop_info.lvl) */
extern signed short g_peddler_extra[3];                 /* ds:c81c fahrender Haendler: drei Zusatzwaren (shop_info.extra; kein  */
extern signed char g_peddler_female;                    /* ds:c822 fahrender Haendler: Haendlerin (Bild 0x82..0x87, Texte; shop */
extern signed char g_camp_fight_pending;                /* ds:c824 Kampf 0xab nach naechtlichem Ueberfall im Dungeonlager fael */
extern struct group far *g_menu_groups[];               /* ds:c826 Gruppen zu den Menuezeilen von grp_select */
extern char far *g_menu_buf;                            /* ds:c83e 600-Byte-Puffer fuer die Menuezeilen von grp_select */
extern char g_name_bufs[4][40];                         /* ds:c842 vier Ringpuffer fuer Heldennamen (O076_0000) */
extern signed short g_sheet_dirty;                      /* ds:c8e2 Merker: Kampfwerte geaendert, Heldenbogen neu (unequip/add_e */
extern struct skill_tpl (far *g_auto_skills)[25];       /* ds:c8e4 Talentvorlagen je Typ fuer den Stufenanstieg (Datei 0x9c, O */
extern struct inc_counter far *g_inc_skills;            /* ds:c8e8 Steigerungszaehler je Talent (BE INC_SKILLS_COUNTER; O086/O0 */
extern struct inc_counter far *g_inc_spells;            /* ds:c8ec Steigerungszaehler je Zauber (BE INC_SPELLS_COUNTER; O086/O0 */
extern struct school_spells far *g_school_spells;       /* ds:c8f0 Zauberlisten der Magierakademien (BE MAGIC_SCHOOLS_INDEX; O0 */
extern struct hero far *g_radio_heroes[];               /* ds:c8f4 Helden zur Auswahlliste g_menu_texts (O088) */
extern void far *g_dbg_dest;                            /* ds:c910 Zieladresse fuer den Absturzbericht (R14 gibt sie aus, wenn  */
extern void far *g_dbg_src;                             /* ds:c914 Quelladresse fuer den Absturzbericht (R14; nirgends gesetzt) */
extern struct talk_desc g_talk;                         /* ds:c918 laufender Dialog (struct talk_desc; O184, O002, O031, O185,  */
extern void far *g_diary_saved_areas;                   /* ds:c93e Tagebuch: gesicherte g_mouse_handler_areas */
extern void (far *g_diary_saved_handler)(short);        /* ds:c942 Tagebuch: gesicherter g_mouse_handler */
extern signed short g_diary_saved_handler_on;           /* ds:c946 Tagebuch: gesicherter g_mouse_handler_on */
extern signed short g_lock_cell_bits;                   /* ds:c948 Feldbits nach dem Schloss (0x2000 = offen, -1 = nichts; O108 */
extern void far *g_anim_small_buf;                      /* ds:c94a Puffer fuer kleine Figurenbilder (0x550 Bytes, O016) */
extern signed short g_fight_id;                         /* ds:c94e Kampfnummer (= Fight.id; O076: Klang bei 0x495) */
extern signed short g_nr_enemies;                       /* ds:c950 Zahl der Gegner im Kampf (O022) (BE NR_OF_ENEMIES) */
extern signed char g_quick_fight;                       /* ds:c952 Schnellkampf/Schnellmodus (doppelter Schaden, kein Bild) */
extern BMP_PTR far *g_fig_obj_bmps;                     /* ds:c953 Standbilder der Feldobjekte je Bildnummer (150 Bmp-Zeiger) */
extern signed char far *g_fig_obj_ids;                  /* ds:c957 Figur-IDs der Feldobjekte (150) */
extern FIGLIST_PTR g_fig_list;                          /* ds:c95b Figurenliste des Kampfes (= &Fight.list) */
extern signed short g_fig_nr_objs;                      /* ds:c95f Zahl der Feldobjekte in g_fig_obj_ids */
extern FIGANIM_PTR far *g_fig_extra_anims;              /* ds:c961 zehn zusaetzliche Animationsobjekte (Treffer-/Zauberbilder,  */
extern struct fig_node far *g_fig_tmpl;                 /* ds:c965 Vorlage fuer fig_list_add/-remove (zeigt auf Fight.tmpl) */
extern signed char g_fig_path[50];                      /* ds:c969 Weg der Bewegung: Richtungen 0..3, Ende -1, dahinter -2 = zu */
extern unsigned char far *g_fight_walkmap;              /* ds:c99b Begehbarkeitskarte des Kampffelds (25 x 24) */
extern struct dirty_rect g_fig_dirty[];                 /* ds:c99f neu zu zeichnende Rechtecke (20, Ende x0 == -1) */
extern signed char g_fig_nr_dirty;                      /* ds:ca67 Zahl der Eintraege in g_fig_dirty */
extern signed short g_fig_redraw_all;                   /* ds:ca68 Kampfbild: alles neu zeichnen */
extern signed short g_fig_dirty_full;                   /* ds:ca6a Vollbild neu (g_fig_dirty enthaelt nur noch den Bildschirm) */
extern char far *g_fig_text;                            /* ds:ca6c Textzeile des Kampfbilds (100 Bytes; 1 = von fight_printer b */
extern signed short g_fig_redraw_top;                   /* ds:ca70 Kampfbild: Anzeige oben (Akteur) neu zeichnen */
extern signed short g_fig_redraw_bottom;                /* ds:ca72 Kampfbild: Anzeige unten (Ziel) neu zeichnen */
extern signed short g_fig_redraw_text;                  /* ds:ca74 Kampfbild: Textzeile neu zeichnen */
extern signed short g_fig_target_x;                     /* ds:ca76 Zielfeld der letzten Wegsuche x (fight_path_search) */
extern signed short g_fig_target_y;                     /* ds:ca78 Zielfeld der letzten Wegsuche y */
extern signed short g_sfight2_x;                        /* ds:ca7a Sonderkampf 0x495: Bildlage des Gegners x */
extern signed short g_sfight2_y;                        /* ds:ca7c Sonderkampf 0x495: Bildlage des Gegners y */
extern signed short g_fig_nr_hits;                      /* ds:ca7e Zahl der getroffenen Helden einer Flaechen-/Mehrfachaktion ( */
extern struct hero far *g_fig_hit_heroes[7];            /* ds:ca80 getroffene Helden (7; kein eigenes Public) */
extern FIGANIM_PTR g_fig_hit_anims[7];                  /* ds:ca9c Animationen je getroffenem Held (7) */
extern signed short g_dng03_last_pos;                   /* ds:cab8 Dungeon 3: Feld des letzten Zufallsereignisses (Geheimtuer-V */
extern signed char g_env_side;                          /* ds:caba Eintrittsseite der Gruppe 0..3, 4 = Mitte (O140 fight_entry_ */
extern signed short far *g_env_kind;                    /* ds:cabb Feldart der 43 Umgebungsfelder (Woerter) */
extern signed short far *g_env_side_kind;               /* ds:cabf Feldart der seitlichen Nachbarn (links 0.., rechts 10..) */
extern signed char far *g_env_tab0;                     /* ds:cac3 Umgebungstabelle 0 (0x56 Bytes, Datei 0x1fc) */
extern signed char far *g_env_tab1;                     /* ds:cac7 Umgebungstabelle 1 */
extern signed char far *g_env_tab2;                     /* ds:cacb Umgebungstabelle 2 */
extern signed char far *g_env_tab3;                     /* ds:cacf Umgebungstabelle 3 */
extern signed char far *g_env_tab4;                     /* ds:cad3 Umgebungstabelle 4 */
extern struct cxy far *g_env_cell_pos;                  /* ds:cad7 Feldlage je Umgebungsfeld im Kampffeld */
extern signed char far *g_env_dirtabs[5];               /* ds:cadb Umgebungstabellen je Eintrittsseite (5 Zeiger, O140 setzt si */
extern signed char far *g_env_dirtab1;                  /* ds:cadf = g_env_dirtabs[1] (eigenes Public) */
extern signed char far *g_env_dirtab2;                  /* ds:cae3 = g_env_dirtabs[2] (eigenes Public) */
extern signed char far *g_env_dirtab3;                  /* ds:cae7 = g_env_dirtabs[3] (eigenes Public) */
extern signed char far *g_env_dirtab4;                  /* ds:caeb = g_env_dirtabs[4] (eigenes Public) */
extern unsigned char (far *g_env_wall_tab)[12];         /* ds:caef Wandstuecke je Wandform [16][12] */
extern signed char (far *g_env_neighbours)[9];          /* ds:caf3 Nachbartabelle der Umgebungsfelder [10][9] (-1 = Rand) */
extern signed char g_env_wall_flag;                     /* ds:caf7 Kampffeld-Erzeugung: Merker in fight_wall_shape */
extern unsigned char far *g_env_cells;                  /* ds:caf8 Kampffeld-Erzeugung: Kopie der 3D-Zellentabelle (0x4000 Byte */
extern signed char far *g_env_kinds;                    /* ds:cafc Kampffeld-Erzeugung: Feldart je Zellentyp (50 Bytes, Datei 0 */
extern signed short g_sfight_ofs_x;                     /* ds:cb00 Sonderkampf 0x493: Bildlage der Sonderfigur x (O141_11A4) */
extern signed short g_sfight_ofs_y;                     /* ds:cb02 Sonderkampf 0x493: Bildlage y */
extern signed short g_sfight_ofs2_x;                    /* ds:cb04 Sonderkampf 0x493: zweite Bildlage x (Ereignis 38/40) */
extern signed short g_sfight_ofs2_y;                    /* ds:cb06 Sonderkampf 0x493: zweite Bildlage y */
extern signed short g_sfight_ofs3_x;                    /* ds:cb08 Sonderkampf 0x493: dritte Bildlage x (Ereignis 39) */
extern signed short g_sfight_ofs3_y;                    /* ds:cb0a Sonderkampf 0x493: dritte Bildlage y */
extern signed short g_sfight_event;                     /* ds:cb0c Sonderkampf 0x493: letztes Ereignis der Sonderfigur */
extern signed short g_swap_i_v1;                        /* ds:cb0e Schleifenzaehler von dsa1_swap_hero_v1 (O177) */
extern signed short far *g_swap_p_v1;                   /* ds:cb10 Listenzeiger von dsa1_swap_hero_v1 (O177) */
extern signed short g_swap_i;                           /* ds:cb14 Schleifenzaehler von dsa1_swap_hero (O177) */
extern signed short far *g_swap_p;                      /* ds:cb16 Listenzeiger von dsa1_swap_hero (O177) */
extern signed short g_nr_items;                         /* ds:cb1a Zahl der Gegenstandsarten (Eintraege in g_items) */
extern signed char g_tmap_loaded;                       /* ds:cb1c Kartenbild der Reisekarte liegt im EMS/XMS (O187 read_map) */
extern signed short g_key_ctrl;                         /* ds:cb1e Strg gedrueckt (bioskey(2) & 4, R18 poll_input) */
extern signed short g_key_alt;                          /* ds:cb20 Alt gedrueckt (bioskey(2) & 8, R18 poll_input) */
extern signed short g_key_last;                         /* ds:cb22 letzte Taste unveraendert (R18 poll_input; g_key wird geloes */
extern signed char g_key_pad5;                          /* ds:cb24 Ziffernblock 5 gedrueckt (Tastatur-ISR R18; R12: Blickhoehe  */
extern unsigned short g_view_frames;                    /* ds:cb26 Bildzaehler der 3D-Ansicht (O198; Himmelsbewegung) */
extern signed short g_menu_sel;                         /* ds:cb28 gewaehlter Eintrag im Optionen-Menue (O199; R18 menu_hover_s */
extern signed short g_rose_last_mx;                     /* ds:cb2a zuletzt ausgewertete Mausposition fuer die Rose (O214 */
extern signed short g_rose_last_my;                     /* ds:cb2c (y) */
extern struct saver g_saver;                            /* ds:cb2e Bildschirmschoner-Objekt (struct saver, Klasse Tm O224; R02  */
extern struct ail_drv_desc g_blaster_desc;              /* ds:cb34 Arbeitskopie der Treiberbeschreibung beim Zerlegen von BLAST */
extern signed short g_blaster_type;                     /* ds:cb4c Kartentyp aus BLASTER (T, 1..4; O232) */
extern void far *g_trnd_txt_buf;                        /* ds:cb4e Texte der Zufallsereignisse (O239, txt_load_list) */
extern char far * far *g_trnd_txt;                      /* ds:cb52 Texte 0x36d der Zufallsereignisse (O239) */
extern BMP_PTR g_trnd_pic;                              /* ds:cb56 Bild des Zufallsereignisses (O239 town_rnd_open: new Bmp) */
extern signed short g_tavern_crowd;                     /* ds:cb5a Andrang in der Schenke (O241; O006 > 1) */
extern signed short g_dng06_statue_ducats;              /* ds:cb6c Dungeon 6: der Brazoragh-Statue geopferte Dukaten, beim Plue */
extern signed short g_random_seed2;                     /* ds:cb74 zweite Zufalls-Saat = Zaehler der Timer-ISR (BE RANDOM_SCHIC */

extern unsigned long far g_fli_ticks;                  /* FAR_D:0000 Zeitgeber-Takte der FLI-Wiedergabe (eigenes Far-Segment; O024 fli_timer_tick) */

/* Welle KS (SEG001/SEG003/SEG023, Startpruefung R05): bisher nicht deklarierte Daten */
extern unsigned long g_copyprot_sum;                    /* ds:35c2 Sollwert der Pruefsumme ueber SEG003 6..0x277 (0x10bcc; R23 check_integrity, O011) */
extern signed short g_copyprot_passed;                  /* ds:3e6e Handbuchabfrage: Antwort richtig (CopyProt::encrypt 1, compare bei Abweichung 0) */
extern unsigned long g_save_count;                      /* ds:3e70 je Speichern + 100 (R01 savegame_op; Startwert 1; sonst nicht gelesen) */
extern signed char g_game_finished2;                    /* ds:262f nie gesetzt (immer 0): wie g_finale_done -> GAME_STATE_FINI */
extern signed short g_dbg_raster_mouse;                 /* ds:5f04 Fehlersuche: Maus-ISR faerbt Farbe 0 beim Ein-/Austritt (set_color) */
extern signed short g_dbg_raster_timer;                 /* ds:5f0c Fehlersuche: Zeitgeber-ISR und -Rueckrufe faerben Farbe 0 */
extern signed short g_fight_timer_ticks;                /* ds:78d6 Takte des Kampf-Zeitgebers (R23 fight_timer_cb, Pruefung alle 250) */
extern signed short g_mouse_lclick_time;                /* ds:8ba7 Zeitpunkt (g_random_seed2) des ersten Linksklicks (R23 mouse_isr/timer_isr: Doppelklick-Frist 4 Takte) */
extern signed char g_npc_months_seen;                   /* ds:8b8e zuletzt behandelter Wert von g_npc_months (R01 main_loop) */
extern signed short g_screen_pic_run;                   /* ds:b7bc = g_screen_pic.run (eigenes Public; R01 game_idle) */
extern signed short g_tmap_mark1_x;                     /* ds:c20b Reisekarte: Kartenpunkt 1 (R01 game_idle -> tmap_mark1_anim) */
extern signed short g_tmap_mark1_y;                     /* ds:c20d */
extern signed short g_tmap_mark2_x;                     /* ds:c20f Reisekarte: Kartenpunkt 2 (tmap_mark2_anim) */
extern signed short g_tmap_mark2_y;                     /* ds:c211 */
extern signed short g_mem_check_countdown;              /* ds:c522 Zeitgeber-ISR: alle 20 Takte mem_check */
extern signed char g_timer_isr_busy;                    /* ds:c536 Zeitgeber-ISR laeuft (Sperre gegen Wiedereintritt; R26 prueft) */
extern signed char g_view_timer_busy;                   /* ds:cb6e Rueckruf view_timer_cb laeuft (Sperre; R26 prueft) */
extern signed char g_fight_timer_busy;                  /* ds:cb6f Rueckruf fight_timer_cb laeuft (Sperre; R26 prueft) */
#ifdef __cplusplus
extern void interrupt (far *g_old_timer_isr)(...);      /* ds:cb70 alter INT-1Ch-Vektor (timer_init/timer_restore) */
#else
extern void interrupt (far *g_old_timer_isr)();
#endif

#endif
