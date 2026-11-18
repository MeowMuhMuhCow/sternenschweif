/* s_const.h - Konstanten und Aufzaehlungen fuer DSA 2 Sternenschweif (Teil von schweif.h)
 *
 * Quellen: Bright Eyes DSA 1 (src/custom/schick/rewrite_m302de/common.h, "BE"), DSA-2-Module (Kopfkommentare).
 * Enum-Konstanten erzeugen keinen Code; sie duerfen ueberall statt der Zahlen stehen (gleiche Bytes).
 * Mit "DSA1" markierte Listen sind aus DSA 1 uebernommen und in DSA 2 nur stichprobenartig bestaetigt.
 */
#ifndef S_CONST_H
#define S_CONST_H

/* --- Eigenschaften: Index in hero.attrib[] (14 Eintraege zu 3 Bytes ab +0x46) ------------------- */
enum {
	ATTRIB_MU = 0,		/* Mut */
	ATTRIB_KL = 1,		/* Klugheit */
	ATTRIB_CH = 2,		/* Charisma */
	ATTRIB_FF = 3,		/* Fingerfertigkeit */
	ATTRIB_GE = 4,		/* Gewandtheit */
	ATTRIB_IN = 5,		/* Intuition */
	ATTRIB_KK = 6,		/* Koerperkraft */
	ATTRIB_AG = 7,		/* Aberglaube (negative Eigenschaften 7..13) */
	ATTRIB_HA = 8,		/* Hoehenangst */
	ATTRIB_RA = 9,		/* Raumangst */
	ATTRIB_GG = 10,		/* Goldgier */
	ATTRIB_TA = 11,		/* Totenangst */
	ATTRIB_NG = 12,		/* Neugier */
	ATTRIB_JZ = 13		/* Jaehzorn */
};

/* --- Heldentypen (hero.type +0x33); Typ >= 7 = Zauberkundige (hero::is_mage) -------------------- */
enum {
	HERO_TYPE_NONE = 0,
	HERO_TYPE_JUGGLER = 1,		/* Gaukler */
	HERO_TYPE_HUNTER = 2,		/* Jaeger */
	HERO_TYPE_WARRIOR = 3,		/* Krieger */
	HERO_TYPE_ESTRAY = 4,		/* Streuner */
	HERO_TYPE_THORWALIAN = 5,	/* Thorwaler */
	HERO_TYPE_DWARF = 6,		/* Zwerg */
	HERO_TYPE_WITCH = 7,		/* Hexe */
	HERO_TYPE_DRUID = 8,		/* Druide */
	HERO_TYPE_MAGE = 9,		/* Magier (O076: Typ 9 mit Stabzauberstufe >= 4 spart 2 AE) */
	HERO_TYPE_GREEN_ELF = 10,	/* Auelf */
	HERO_TYPE_ICE_ELF = 11,		/* Firnelf */
	HERO_TYPE_SYLVAN_ELF = 12	/* Waldelf */
};

/* --- Krankheiten: Index in hero.illness[1..7] (+0xc0, 5 Bytes) ------------------------------------ */
enum {
	ILLNESS_NONE = 0,
	ILLNESS_WUNDFIEBER = 1,
	ILLNESS_DUMPFSCHAEDEL = 2,
	ILLNESS_BLAUE_KEUCHE = 3,
	ILLNESS_PARALYSE = 4,
	ILLNESS_SCHLACHTFELDFIEBER = 5,
	ILLNESS_FROSTSCHAEDEN = 6,
	ILLNESS_TOLLWUT = 7
};
#define NR_ILLNESSES	8		/* Eintrag 0 unbenutzt */

/* --- Gifte: Index in hero.poison[1..9] (+0xe8, 5 Bytes) ------------------------------------------- */
enum {
	POISON_NONE = 0,
	POISON_SHURINKNOLLE = 1,
	POISON_ARAX = 2,
	POISON_ANGSTGIFT = 3,
	POISON_SCHLAFGIFT = 4,
	POISON_GOLDLEIM = 5,
	POISON_KROETENSCHEMEL = 6,
	POISON_LOTUSGIFT = 7,
	POISON_KUKRIS = 8,
	POISON_BANNSTAUB = 9
};
#define NR_POISONS	10		/* Eintrag 0 unbenutzt */

/* Zustand eines Krankheits-/Gifteintrags (hero_ill.state) */
#define ILL_STATE_SICK		(-1)	/* krank / vergiftet */
#define ILL_STATE_RECOVER	1	/* genesend / Gift laesst nach */

/* --- Talente: Index in hero.skills[52] (+0x11a), DSA1-Liste (O076: 7/8 = Fernkampf bestaetigt) ---- */
enum {
	TA_WAFFENLOS = 0, TA_HIEBWAFFEN = 1, TA_STICHWAFFEN = 2, TA_SCHWERTER = 3, TA_AEXTE = 4,
	TA_SPEERE = 5, TA_ZWEIHAENDER = 6, TA_SCHUSSWAFFEN = 7, TA_WURFWAFFEN = 8,
	TA_AKROBATIK = 9, TA_KLETTERN = 10, TA_KOERPERBEHERRSCHUNG = 11, TA_REITEN = 12,
	TA_SCHLEICHEN = 13, TA_SCHWIMMEN = 14, TA_SELBSTBEHERRSCHUNG = 15, TA_TANZEN = 16,
	TA_VERSTECKEN = 17, TA_ZECHEN = 18,
	TA_BEKEHREN = 19, TA_BETOEREN = 20, TA_FEILSCHEN = 21, TA_GASSENWISSEN = 22, TA_LUEGEN = 23,
	TA_MENSCHENKENNTNIS = 24, TA_SCHAETZEN = 25,
	TA_FAEHRTENSUCHEN = 26, TA_FESSELN = 27, TA_ORIENTIERUNG = 28, TA_PFLANZENKUNDE = 29,
	TA_TIERKUNDE = 30, TA_WILDNISLEBEN = 31,
	TA_ALCHIMIE = 32, TA_ALTE_SPRACHEN = 33, TA_GEOGRAPHIE = 34, TA_GESCHICHTE = 35,
	TA_GOETTER_KULTE = 36, TA_KRIEGSKUNST = 37, TA_LESEN = 38, TA_MAGIEKUNDE = 39, TA_SPRACHEN = 40,
	TA_ABRICHTEN = 41, TA_FAHRZEUGE = 42, TA_FALSCHSPIEL = 43, TA_HEILEN_GIFT = 44,
	TA_HEILEN_KRANKHEITEN = 45, TA_HEILEN_WUNDEN = 46, TA_MUSIZIEREN = 47, TA_SCHLOESSER = 48,
	TA_TASCHENDIEBSTAHL = 49,
	TA_GEFAHRENSINN = 50, TA_SINNESSCHAERFE = 51
};
#define NR_SKILLS	52

/* --- Zauber: Index in hero.spells[86] (+0x14f), Eintrag 0 unbenutzt. Reihenfolge wie DSA 1 und durch DSA 2 bestaetigt
 * (Welle N13: Zauberroutinen g_spell_handlers O233..O237 je Nummer, Selbstproben test_spell(.., SP_*), Bright Eyes
 * schweif.cpp names_spell). DSA-2-Menuenamen abweichend: 11 "Herr der Tiere", 19/20 "Geister bannen/rufen",
 * 24 "Elementar herbeirufen", 37 "Tiere heilen", 47 "Harmlos", 78 "Brenne", 81 "Erstarre", 83 "Schmelze". ---- */
enum {
	SP_NONE = 0,
	SP_BEHERRSCHUNG_BRECHEN = 1, SP_DESTRUCTIBO = 2, SP_GARDIANUM = 3, SP_ILLUSIONEN_ZERSTOEREN = 4,
	SP_VERWANDLUNG_BEENDEN = 5,
	SP_BAND_UND_FESSEL = 6, SP_BANNBALADIN = 7, SP_BOESER_BLICK = 8, SP_GROSSE_GIER = 9,
	SP_GROSSE_VERWIRRUNG = 10, SP_HERR_UEBER_DAS_TIERREICH = 11, SP_HORRIPHOBUS = 12,
	SP_MAGISCHER_RAUB = 13, SP_RESPONDAMI = 14, SP_SANFTMUT = 15, SP_SOMNIGRAVIS = 16, SP_ZWINGTANZ = 17,
	SP_FUROR_BLUT = 18, SP_GEISTER_AUSTREIBEN = 19, SP_GEISTER_BESCHWOEREN = 20, SP_HEPTAGON = 21,
	SP_KRAEHENRUF = 22, SP_SKELETTARIUS = 23,
	SP_ELEMENTARE_BESCHWOEREN = 24, SP_NIHILATIO_GRAVITAS = 25, SP_SOLIDIRID = 26,
	SP_AXXELERATUS = 27, SP_FORAMEN = 28, SP_MOTORICUS = 29, SP_SPURLOS = 30, SP_TRANSVERSALIS = 31,
	SP_UEBER_EIS = 32,
	SP_BALSAM = 33, SP_HEXENSPEICHEL = 34, SP_KLARUM_PURUM = 35, SP_RUHE_KOERPER = 36,
	SP_TIERE_BESPRECHEN = 37,
	SP_ADLERAUG = 38, SP_ANALUES = 39, SP_EIGENSCHAFTEN = 40, SP_EXPOSAMI = 41, SP_ODEM_ARCANUM = 42,
	SP_PENETRIZZEL = 43, SP_SENSIBAR = 44,
	SP_CHAMAELIONI = 45, SP_DUPLICATUS = 46, SP_HARMLOSE_GESTALT = 47, SP_HEXENKNOTEN = 48,
	SP_BLITZ = 49, SP_ECLIPTIFACTUS = 50, SP_EISENROST = 51, SP_FULMINICTUS = 52, SP_IGNIFAXIUS = 53,
	SP_PLUMBUMBARUM = 54, SP_RADAU = 55, SP_SAFT_KRAFT = 56, SP_SCHARFES_AUGE = 57,
	SP_HEXENBLICK = 58, SP_NEKROPATHIA = 59,
	SP_ADLER_WOLF = 60, SP_ARCANO_PSYCHOSTABILIS = 61, SP_ARMATRUTZ = 62, SP_CH_STEIGERN = 63,
	SP_FEUERBANN = 64, SP_FF_STEIGERN = 65, SP_GE_STEIGERN = 66, SP_IN_STEIGERN = 67,
	SP_KK_STEIGERN = 68, SP_KL_STEIGERN = 69, SP_MU_STEIGERN = 70, SP_MUTABILI = 71, SP_PARALUE = 72,
	SP_SALANDER = 73, SP_IN_SEE_UND_FLUSS = 74, SP_VISIBILI = 75,
	SP_ABVENENUM = 76, SP_AEOLITUS = 77, SP_BRENNE_TOTER_STOFF = 78, SP_CLAUDIBUS = 79,
	SP_DUNKELHEIT = 80, SP_WEICHES_ERSTARRE = 81, SP_FLIM_FLAM = 82, SP_HARTES_SCHMELZE = 83,
	SP_SILENTIUM = 84, SP_STURMGEBRUELL = 85
};
#define NR_SPELLS	86

/* --- Gespraechsskripte (O260 Script::keyword, O049 Conv::ask): Antwort-Schluesselwoerter ------------------ */
#define SCRIPT_ANS_NULL		-1	/* keine Antwort: no_answer bzw. Zufallstext Thema 0x63 */
#define SCRIPT_ANS_LIST_TRUE	-2	/* Zufallsantwort (Art 2) */
#define SCRIPT_ANS_LIST_FALSE	-3	/* Zufallsalternative (Art 3) */
#define SCRIPT_ANS_LIST		-4	/* Zufallsantwort (Art 2) */
#define SCRIPT_ANS_NOTMORE	-5	/* "nichts mehr" (Zufallstext Thema 100) */

/* --- Zauberklassen (Gruppen von g_spells_index, ds:22be {erster Zauber, Anzahl}) ---------------------------- */
enum {
	SPELL_CLASS_ANTIMAGIE = 0, SPELL_CLASS_BEHERRSCHUNG = 1, SPELL_CLASS_DAEMONOLOGIE = 2,
	SPELL_CLASS_ELEMENTAR = 3, SPELL_CLASS_BEWEGUNG = 4, SPELL_CLASS_HEILUNG = 5, SPELL_CLASS_HELLSICHT = 6,
	SPELL_CLASS_ILLUSION = 7, SPELL_CLASS_KAMPF = 8, SPELL_CLASS_VERSTAENDIGUNG = 9, SPELL_CLASS_VERWANDLUNG = 10,
	SPELL_CLASS_VERAENDERUNG = 11
};
#define NR_SPELL_CLASSES	12

/* --- Ergebnis einer Zauberroutine (Cast.result O233, Spell O194): sonst AE-Kosten > 0 ---------------------- */
#define SPELL_RES_NONE		0	/* ohne Wirkung, keine Kosten (Text txt[0x8d] bzw. eigener) */
#define SPELL_RES_DEFAULT	(-1)	/* gelungen, Grundkosten hero_spell_cost (Vorgabe des Konstruktors) */
#define SPELL_RES_FAIL		(-2)	/* misslungen: Kosten fail_cost bzw. halbe Grundkosten */
#define SPELL_RES_ABORT		(-4)	/* abgebrochen (falsches Ziel, schon wirksam ...), keine Kosten */

/* --- Gegnerzauber (Zauberroutine g_mon_spell_handlers[n], O194; BE mspell_*, DSA1 ebenso nummeriert) ------- */
enum {
	MSP_NONE = 0,
	MSP_VERWANDLUNG_BEENDEN = 1, MSP_BANNBALADIN = 2, MSP_BOESER_BLICK = 3, MSP_HORRIPHOBUS = 4,
	MSP_AXXELERATUS = 5, MSP_BALSAM = 6, MSP_BLITZ = 7, MSP_EISENROST = 8, MSP_FULMINICTUS = 9,
	MSP_IGNIFAXIUS = 10, MSP_PLUMBUMBARUM = 11, MSP_SAFT_KRAFT = 12, MSP_ARMATRUTZ = 13, MSP_PARALUE = 14
};
#define NR_MON_SPELLS	15

/* --- Zauberbeschreibung: Herkunft
 (spell_desc.herotype, BE SPELL_DESC_HEROTYPE_*; O087 Steigerungsgrenze 2) -- */
enum {
	SPELL_DESC_HEROTYPE_DRUID = 0,	/* Druidenzauber */
	SPELL_DESC_HEROTYPE_MAGE = 1,	/* Magierzauber */
	SPELL_DESC_HEROTYPE_WITCH = 2,	/* Hexenzauber */
	SPELL_DESC_HEROTYPE_GELF = 3,	/* Auelfenzauber */
	SPELL_DESC_HEROTYPE_IELF = 4,	/* Firnelfenzauber */
	SPELL_DESC_HEROTYPE_SELF = 5	/* Waldelfenzauber */
};

/* --- Inventar (hero.inv[31], +0x1a8, 0x1b Bytes je Platz) ------------------------------------------ */
#define NR_INV_SLOTS		31
#define INV_SLOT_RIGHT_HAND	3	/* Waffe (O080_0595, O194: +0x1f9 = inv[3].id) */
#define INV_SLOT_LEFT_HAND	12	/* zweite Hand (O083: Waffen nur an 3/12, Zweihaender-Konflikt 3 <-> 12) */
#define INV_SLOT_KNAPSACK	15	/* erster Rucksackplatz (O085_1503: Rucksack 15..30) */

/* --- Kampfziel hero.enemy_id (+0x98, unsigned gelesen; O077) ----------------------------------------- */
#define TARGET_ID_SELF		0	/* der Held selbst */
#define TARGET_ID_HERO		1	/* 1..9: Held Nr. id - 1 */
#define TARGET_ID_ENEMY		10	/* 10..29: Gegner g_enemies[id - 10] */
#define TARGET_ID_SPECIAL	30	/* 30..: Sondergegner g_enemies[id - 30] */
#define TARGET_ID_NONE		0xff	/* kein Ziel */
/* Zieltyp (target_type) */
enum {
	TARGET_NONE = 0,
	TARGET_ENEMY = 1,
	TARGET_HERO = 2,		/* anderer Held */
	TARGET_SPECIAL = 4,		/* Sondergegner */
	TARGET_SELF = 5
};

/* --- Gruppen -------------------------------------------------------------------------------------- */
#define NR_GROUPS		6	/* g_groups[6] (ds:c30f) */
#define NR_GROUP_HEROES		7	/* group.heroes[7]; Platz 6 = NSC (O201) */
#define NR_HEROES		7	/* g_heroes[7] (ds:b760) */

/* --- Spielzeit (R21): 1 Tick = 1/1,5 s ------------------------------------------------------------ */
#define TICKS_PER_MINUTE	90L
#define TICKS_PER_HOUR		5400L
#define TICKS_PER_DAY		129600L
#define TICKS_PER_HALF_DAY	64800L	/* 12 Stunden (R21 time_tick rechnet die Teiler je Halbtag) */
#define DAYS_PER_WEEK		7
#define DAYS_PER_MONTH		30
#define MONTHS_PER_YEAR		12
#define NAMELESS_DAYS		5	/* Namenlose Tage am Jahresende (date.day -5..-1) */

/* --- Nummer des angezeigten Vollbilds (g_pp20_index, ds:b666; BE DSA1: PP20_INDEX) ----------------- */
#define PP20_TRAVELMAP		0x94	/* Reisekarte (O209_0000 zeichnet Bild 0x94); O076: dort kein Neuaufbau */
#define PP20_SHEET		14	/* Heldenbogen mit Koerperbild (O015 Anlege-/Zauberanimationen, O008 inv_show_body) */
/* 4, 14, 15 kommen in den Heldenwert-Modulen vor (14/15: Heldenbogen?) - Bedeutung noch offen */

/* --- Art der 3D-Ansicht (g_area_type, ds:2883) --------------------------------------------------- */
#define AREA_NONE		0
#define AREA_TOWN		1	/* Stadt: O046 Hauptschleife (group.town); R08 reveal_map 5 x 5 mit Sichtpruefung */
#define AREA_DUNGEON		2	/* Dungeon: O014 Hauptschleife (group.dungeon) - bis N8 waren die Namen vertauscht */

/* --- Ortsart group.loctype (+0x1e; Verteiler O030 f_0000, Hauptschleife R01: loctype != 0 -> O030) ------- */
#define LOC_NONE		0	/* kein Ort: Strasse bzw. Gang (3D-Ansicht) */
#define LOC_TEMPLE		1	/* Tempel (O044) */
#define LOC_TAVERN		2	/* Schenke (O240; O006 Talente bei Andrang) */
#define LOC_INN			3	/* Herberge (O178; O005: kein Hunger/Durst) */
#define LOC_MERCHANT		4	/* Haendler/Laden (O226; O006 Taschendiebstahl) */
#define LOC_SMITH		5	/* O230: Gegenstand abgeben/abholen, Oeffnungszeiten (vermutet Schmied) */
#define LOC_HEALER		6	/* Heiler (O174) */
#define LOC_WAREHOUSE		7	/* Lagerhaus (O012) */
#define LOC_CAMP_WILD		8	/* Lager unterwegs (O071 run(0); O038: danach g_travel_state 3); 8..10, LOC_INN: Alchimie (O006) */
#define LOC_CAMP_TOWN		9	/* Rasten in der Stadt (O071 run(1); O046 Aktion 0x86) */
#define LOC_CAMP_DUNGEON	10	/* Rasten im Dungeon (O071 run(2); O014 Aktion setdir(10)) */
#define LOC_LODGING		12	/* O070 Essen/Uebernachten (Herberge?) */
#define LOC_HOUSE		14	/* Haus mit Zufallstext je Richtung (O173); O183: Vorgabe fuer andere Tueren */
#define LOC_EXIT		15	/* Rueckfrage: Gruppe verlaesst den Ort (O030) */
#define LOC_SPECIAL		17	/* Ereignisort g_loc_id (O030 f_0df9: O166/O031) */
#define LOC_DATA		0x80	/* Bit 7: Ort n + 1 der Stadt, Daten per loc_load_data(n) nachladen (O030); set_loctype
					   merkt solche Werte nicht als old_loctype */
/* 11, 13, 16: O030 ohne Funktion */

/* --- Reisekarte (Welle N12: O217..O223) ------------------------------------------------------------ */
#define NR_ROUTES		0x106	/* Eintraege in g_routes */
#define ROUTE_NONE		999	/* route_find/route_first/route_find_area: kein Weg */
#define ROUTE_END		0xff	/* Ende der Route travel.route[] bzw. kein Zielort (route_dest_by_dir) */
#define ROUTE_MAX		40	/* Laenge von travel.route[] */
#define PLACE_AREA_FIRST	0xe6	/* place_area: Orte 2..20 einer Gegend -> 0xe6..0xeb */
/* travel.action (+0xf8): Anforderung waehrend eines Weges (travel_route O215 fuehrt sie aus) */
#define TRAVEL_ACT_TURN		1	/* umkehren (Rastmenue, O164 Fuehrer, O212) -> travel_reverse */
#define TRAVEL_ACT_REROUTE	2	/* Route ab dem Ziel des Weges neu (Rastmenue Punkt 4) -> route_rebuild */
#define TRAVEL_ACT_TURNED	3	/* travel_reverse ausgefuehrt: Ereignismerker tauschen (O215) */
#define TRAVEL_ACT_AUTO		4	/* weiter ohne Menue (travel_step) */

/* --- Haendler (O226..O229): shop_info.type, g_loc_id des Marktstands ------------------------------- */
#define SHOP_WEAPONS		1	/* Waffenhaendler: Ruestungen/Waffen (item_desc.armor/weapon) */
#define SHOP_HERBS		2	/* Kraeuterhaendler: item_desc.herb_potion */
#define SHOP_GENERAL		3	/* Kraemer: alles uebrige */
#define SHOP_MARKET		0x18	/* g_loc_id des Marktstands (O183 market_stall, Ladendaten g_market_shop) */

/* --- Gegenstaende (inv_slot.item_id, Index in g_items; Werte wie BE DSA1, R10 bestaetigt) ----------- */
#define ITEM_TORCH_ON		0x16	/* Fackel brennend (R10 sub_light_timers: abgebrannt -> drop_item) */
#define ITEM_LANTERN_OFF	0x19	/* Laterne aus */
#define ITEM_LANTERN_ON		0xf9	/* Laterne an (R10: leer -> ITEM_LANTERN_OFF) */
#define ITEM_FOOD_PACKAGE	0x2d	/* Proviantpaket (O013 Abgabe im Dialog, O184 ev_tribute) */
#define ITEM_LOWANGEN_KEY	0x133	/* Lowangen-Schluessel (O013 talk_give_lowangen_key) */
#define ITEM_STONE_MEDAILLON	0x13c	/* Steinmedaillon (O102 Dialog 0x13c) */
#define ITEM_MAGISTRATE_DOC	0x13f	/* Magistratsdokument (O013: Abgabe guenstiger, Belohnung) */
#define ITEM_PARCEL		0x14a	/* Paeckchen (O013 talk_check_parcel, O102) */
#define ITEM_DOCUMENT_A		0x14b	/* Dokument (O013 talk_hand_over_document) */
#define ITEM_DOCUMENT_B		0x14c	/* Dokument */
#define ITEM_WATERSKIN		0x1e	/* Wasserschlauch (Bits empty/half_empty; O083, O202) (BE ebenfalls 30) */
#define ITEM_RECIPE_BOOK	0xbf	/* Rezeptbuch: 14 Faecher = Rezepte g_recipe_items (O048/O252; O077_08C2, O084_0000) (frueher ITEM_BOX_BF) */
#define ITEM_BOX_BF	ITEM_RECIPE_BOOK	/* Altname (Welle N15) */
#define ITEM_KEYRING		0x145	/* Schluesselbund mit 26 Faechern (Tabelle ds:4ff0; O077_0932, O085_1B2F, O203) */
#define ITEM_MONEY		0x160	/* Pseudo-Gegenstand Geld in Ablagen (O238: Anzahl x 100 Heller) */
#define ITEM_DEPOT_TICKET_1	0xa4	/* Lagerschein Lagerhaus 1 (O099 nach dem Einlagern, O100 Voraussetzung zum Abholen) */
#define ITEM_DEPOT_TICKET_2	0xaa	/* Lagerschein Lagerhaus 2 */
#define ITEM_DEPOT_TICKET_3	0xb0	/* Lagerschein Lagerhaus 3 */
#define ITEM_PICKLOCKS		0x23	/* Dietriche (O108 Lock::pick) (BE ebenfalls 0x23) */
#define ITEM_OIL		0x29	/* Oel (O237 Brenne: Laterne fuellen) (BE ebenfalls 0x29) */
#define ITEM_FLASK_BRONZE	0x2a	/* Bronzeflasche (leere Oelflasche, O237) (BE ebenfalls 0x2a) */
#define ITEM_TORCH_OFF		0x41	/* Fackel aus (O237 Brenne) (BE ebenfalls 0x41) */
#define ITEM_WITCHES_BROOM	0x8c	/* Hexenbesen (O236 Radau setzt ihn voraus) (BE-Nummer, vermutet) */
/* Welle N15: Gegenstaende (Nummern wie DSA1/BE, Wirkung in O079/O252..O254 bestaetigt) */
#define ITEM_DAGGER	0x0e	/* Dolch (Kukrisdolch wird dazu, O079) (BE-Nummer) */
#define ITEM_BEER	0x17	/* Bier (O253: betrunken, Glasflasche bleibt nicht) (BE-Nummer) */
#define ITEM_SCROLL	0x1d	/* Schriftrolle (O252: wird beim Abschreiben zum Rezeptbuch) (BE-Nummer) */
#define ITEM_FLASK_GLASS	0x1f	/* Glasflasche (leer, nach Traenken/Getraenken) (BE-Nummer) */
#define ITEM_SHIRT	0x30	/* Hemd (O263) (BE-Nummer) */
#define ITEM_TROUSERS	0x31	/* Hose (O263) (BE-Nummer) */
#define ITEM_SHOES	0x32	/* Schuhe (O263) (BE-Nummer) */
#define ITEM_WRITING_SET	0x24	/* Schreibzeug (O252 Rezept abschreiben, Ladungen) (BE-Nummer) */
#define ITEM_HARP	0x25	/* Harfe (O252) (BE-Nummer) */
#define ITEM_FLUTE	0x2e	/* Floete (O252) (BE-Nummer) */
#define ITEM_SHURIN_POISON	0x37	/* Shurinknollengift (BE-Nummer) */
#define ITEM_ARAX_POISON	0x38	/* Araxgift (BE-Nummer) */
#define ITEM_ANGST_POISON	0x39	/* Angstgift (BE-Nummer) */
#define ITEM_SLEEP_POISON	0x3a	/* Schlafgift (BE-Nummer) */
#define ITEM_GOLDLEIM	0x3b	/* Goldleim (BE-Nummer) */
#define ITEM_EINBEERE	0x3c	/* Einbeere (LE + W6) (BE-Nummer) */
#define ITEM_WHIRLWEED	0x3d	/* Wirselkraut (LE + 10) (BE-Nummer) */
#define ITEM_GULMOND_LEAF	0x3f	/* Gulmondblatt (KK + 2) (BE-Nummer) */
#define ITEM_TARNELE	0x40	/* Tarnele (ruhe_koerper = 1) (BE-Nummer) */
#define ITEM_TINDERBOX	0x55	/* Zunderkaestchen (Ladungen) (BE-Nummer) */
#define ITEM_WINE	0x5b	/* Weinflasche (BE-Nummer) */
#define ITEM_BRANDY	0x5c	/* Schnapsflasche (BE-Nummer) */
#define ITEM_LUTE	0x5f	/* Laute (O252) (BE-Nummer) */
#define ITEM_MENGBILAR	0x6d	/* Mengbilar (O079) (BE-Nummer) */
#define ITEM_BELMART	0x7b	/* Belmartblatt (gegen Gift bis 5) (BE-Nummer) */
#define ITEM_MENCHAL	0x7d	/* Menchalkaktus (gegen Gift bis 10) (BE-Nummer) */
#define ITEM_ATMONBLUETE	0x7f	/* Atmonbluete (Talente 9..18 + 2) (BE-Nummer) */
#define ITEM_THONNYS	0x83	/* Thonnysbluete (BE-Nummer) */
#define ITEM_LOTUS_POISON	0x8d	/* Lotusgift (BE-Nummer) */
#define ITEM_KUKRIS	0x8e	/* Kukris (Gift) (BE-Nummer) */
#define ITEM_BANNSTAUB	0x8f	/* Bannstaub (AE-Verlust) (BE-Nummer) */
#define ITEM_KROETEN_POISON	0x90	/* Kroetenschemelgift (BE-Nummer) */
#define ITEM_LE_POTION	0x91	/* Heiltrank (BE-Nummer) */
#define ITEM_STRONG_LE_POTION	0x92	/* starker Heiltrank (BE-Nummer) */
#define ITEM_AE_POTION	0x9a	/* Zaubertrank (BE-Nummer) */
#define ITEM_STRONG_AE_POTION	0x9b	/* starker Zaubertrank (BE-Nummer) */
#define ITEM_SABER_MAGIC	0xac	/* magischer Saebel (+1 gegen Untote, O079) (BE-Nummer) */
#define ITEM_ANTIVENOM	0xb4	/* Gegengift (BE-Nummer) */
#define ITEM_MAGIC_BREADBAG	0xb8	/* magischer Brotbeutel (wird nicht verbraucht) (BE-Nummer) */
#define ITEM_MAGIC_WATERSKIN	0xb9	/* magischer Wasserschlauch (wird nicht verbraucht) (BE-Nummer) */
#define ITEM_KUKRIS_DAGGER	0xc1	/* Kukrisdolch (O079) (BE-Nummer) */
#define ITEM_KUKRIS_MENGBILAR	0xc8	/* Kukrismengbilar (O079) (BE-Nummer) */
#define ITEM_SILVER_MACE	0xd6	/* Silberstreitkolben (+4 gegen Untote, O079) (BE-Nummer) */
#define ITEM_ANTI_SICKNESS_ELIXIR	0xdf	/* Antikrankheitselixier (BE-Nummer) */
#define ITEM_WUNDERKUR	0xec	/* Wunderkur (BE-Nummer) */
#define ITEM_SLEEPING_POTION	0xed	/* Schlaftrunk (ruhe_koerper = 3) (BE-Nummer) */
#define ITEM_MIASTHMATICUM	0xee	/* Miasthmaticum (BE-Nummer) */
#define ITEM_HYLAILIC_FIRE	0xef	/* Hylailer Feuer (BE-Nummer) */

/* --- Schloss (class Lock, O108): Zustand lock.state (Datensatz in Datei 0x2c8) ---------------------------- */
enum {
	LOCK_CLOSED = 0,		/* verschlossen */
	LOCK_OPEN = 1,			/* geoeffnet (Schluessel, Dietrich, Foramen) */
	LOCK_OPENED = 2,		/* offen (ohne Meldung, setzt nur g_lock_cell_bits; vermutet: von aussen gesetzt) */
	LOCK_BROKEN = 3,		/* aufgebrochen */
	LOCK_SEALED = 4			/* zugezaubert (Claudibus: Werte verdoppelt) */
};

/* --- Klang (R04 struct sound, Miles AIL 2) ---------------------------------------------------------- */
#define SND_BUF_SIZE		0x2800	/* Groesse der beiden Abspielpuffer */
#define SND_ID_SPEECH		0x8000	/* Klangnummer: Sprachausgabe (id & 0x7fff, > 1000: g_file_speech_ext) */
#define SND_ID_ARCHIVE		0x4000	/* Klangnummer: im Archiv (id & 0x3fff) */
#define SND_SRC_FILES		1	/* sound.src: Klaenge aus Dateien */
#define SND_SRC_ARCHIVE		2	/* sound.src: Klaenge aus dem Archiv */
#define AIL_DSP_DRVR		2	/* ail_drv_desc.type: digitaler Treiber */
#define AIL_XMIDI_DRVR		3	/* ail_drv_desc.type: XMIDI-Treiber */
#define DAC_STOPPED		0	/* AIL_sound_buffer_status */
#define DAC_DONE		3
#define SEQ_PLAYING		1	/* AIL_sequence_status */

/* --- Wirkungsarten hero.effects[].type (R02 add_effect/remove_effect/effects_tick) -------------------- */
#define NR_HERO_EFFECTS		20
enum {
	EFFECT_ATTRIB = 0,		/* change_attrib(index, value) */
	EFFECT_SKILL = 1,		/* add_skill_value(index, value) */
	EFFECT_AT = 2,			/* at[index] */
	EFFECT_PA = 3,			/* pa[index] */
	EFFECT_SPELL = 4,		/* spells[index] */
	EFFECT_MR = 5,			/* add_mr_mod(value) */
	EFFECT_HUNGER_TIMER = 6,	/* hero +0x90 */
	EFFECT_AXXELERATUS = 7,
	EFFECT_SAFTKRAFT = 8,
	EFFECT_RS_BONUS = 9,
	EFFECT_FIREBAN = 10,
	EFFECT_INVISIBLE = 11,
	EFFECT_UEBER_EIS = 12,		/* Bits +0xbd/+0xbe setzen bzw. loeschen: hero.ueber_eis (Zauber Ueber Eis) */
	EFFECT_HARMLOS = 13,		/* hero.harmless (Harmlose Gestalt) */
	EFFECT_SAFTKRAFT_ON = 14,	/* hero.saftkraft_on (Saft, Kraft, Monstermacht) */
	EFFECT_SCHARFES_AUGE = 15,	/* hero.scharfes_auge */
	EFFECT_SEE_UND_FLUSS = 16,	/* hero.see_und_fluss (group_add_effect) */
	EFFECT_ADLER_WOLF = 17,		/* hero.adler_wolf */
	EFFECT_FLIM_FLAM = 18,		/* hero.flim_flam */
	EFFECT_DUNKELHEIT = 19		/* hero.dunkelheit */
};
/* alte Namen (bis Welle N12) */
#define EFFECT_BIT_BD1	EFFECT_UEBER_EIS
#define EFFECT_BIT_BD4	EFFECT_HARMLOS
#define EFFECT_BIT_BD7	EFFECT_SAFTKRAFT_ON
#define EFFECT_BIT_BE0	EFFECT_SCHARFES_AUGE
#define EFFECT_BIT_BE1	EFFECT_SEE_UND_FLUSS
#define EFFECT_BIT_BE3	EFFECT_ADLER_WOLF
#define EFFECT_BIT_BE4	EFFECT_FLIM_FLAM
#define EFFECT_BIT_BE5	EFFECT_DUNKELHEIT

/* --- Tastencodes (g_key, bioskey: Scancode << 8 | ASCII) ------------------------------------------ */
#define KEY_ESC			0x011b
#define KEY_RETURN		0x1c0d
#define KEY_F1			0x3b00
#define KEY_F2			0x3c00
#define KEY_F3			0x3d00
#define KEY_F4			0x3e00
#define KEY_F5			0x3f00
#define KEY_F8			0x4200
#define KEY_F9			0x4300
#define KEY_CTRL_E		0x1205
#define KEY_CTRL_J		0x240a
#define KEY_CTRL_O		0x180f
#define KEY_CTRL_S		0x1f13
#define KEY_UP			0x4800
#define KEY_DOWN		0x5000
#define KEY_LEFT		0x4b00
#define KEY_RIGHT		0x4d00
#define KEY_PAD5		0x4c00	/* Ziffernblock 5 (nur Scancode verglichen) */
#define KEY_SPACE		0x3920
#define KEY_PGUP		0x4900	/* Bild auf: Dialog abbrechen (O189/O190/O205) */
#define KEY_PGDN		0x5100	/* Bild ab: Dialog abbrechen */
#define KEY_INS			0x5200
#define KEY_DEL			0x5300
#define KEY_HOME		0x4700	/* Pos1 (O103, O110) */
#define KEY_END			0x4f00	/* Ende (O103, O110) */
#define KEY_F10			0x4400	/* Pause (R18 poll_input) */
#define KEY_CTRL_P		0x1910	/* Pause (R18 poll_input) */
#define KEY_CTRL_Q		0x1011	/* Spiel beenden (R18 poll_input, R14 guru_meditation) */
#define SCAN_UP			0x48	/* Scancodes (g_key_scan, Tastatur-ISR R18) */
#define SCAN_HOME		0x47
#define SCAN_LEFT		0x4b
#define SCAN_PAD5		0x4c
#define SCAN_RIGHT		0x4d
#define SCAN_END		0x4f
#define SCAN_DOWN		0x50

/* --- Eingabe-Aktionen (g_action, R02 handle_input / O034 game_menu) --------------------------------- */
#define ACTION_ICON_1		0x81	/* Symbolfelder 1..9 der Ortsmenues (BE ACTION_ID_ICON_1..9) */
#define ACTION_ICON_2		0x82
#define ACTION_ICON_3		0x83
#define ACTION_ICON_4		0x84
#define ACTION_ICON_5		0x85
#define ACTION_ICON_6		0x86
#define ACTION_ICON_7		0x87
#define ACTION_ICON_8		0x88
#define ACTION_ICON_9		0x89
#define ACTION_OPTIONS		0xe0	/* Optionen-Menue (handle_options_key) */
#define ACTION_DIARY		0xe1	/* Tagebuch (O103) */
#define ACTION_MENU_SCREEN	0xe2	/* Auswahlbild (O034_154A) */
#define ACTION_DATE		0xe3	/* Datum anzeigen (F4) */
#define ACTION_PORTRAIT		0xf1	/* 0xf1..0xf7: Klick auf Portraet 0..6 */
#define ACTION_MENU		0xfd	/* Spielmenue */

/* --- Flaechenbits face3d.flags (3D-Ansicht R12/R13; Bedeutung aus der Verwendung) ------------------ */
#define FACE_CULL		0x0001	/* einseitig: Rueckseite verwerfen (R25 obj_project, Kreuzprodukt der ersten drei Ecken) */
#define FACE_SHADED		0x0002	/* Helligkeit nach Winkel (g_shade_table) */
#define FACE_TEXTURED		0x0008	/* texturiert (texture.ref, sonst einfarbig) */
#define FACE_MASKED		0x0010	/* Textur mit Durchsicht, Farbe 0 (poly_tex_cols_m, poly_tex_rows_m) */
#define FACE_NEAR		0x0020	/* mit FACE_TEXTURED zeilenweise fuellen (poly_edges horizontal = 1, poly_tex_rows*: waagerechte
					   Flaechen, Boden/Decke); in der Stadt nur dann nahe perspektivische Kanten (poly_edges_persp) */
#define FACE_CLICK		0x0040	/* Klickflaeche (nicht gezeichnet, Rechteck in g_click_rect) */
#define FACE_SOLID		0x0080	/* Wand fuer die Kollision (R13 cell_profile) */
#define FACE_ROTATE		0x0100	/* dreht sich mit der Blickrichtung (draw_object) */
#define MAP_EVENT_ENTER		(-2)	/* map_event.type: Zelle betreten (R12 cell_event) */

/* --- Karte 64 x 64 (R17 map_grid, g_cells; R19 get_cell_*) ----------------------------------------- */
#define MAP_SIZE		64	/* Breite/Hoehe der Zellentabelle g_cells */
#define MAP_CELL_INVALID	0xff	/* get_map_cell ausserhalb der Karte */
#define CELL_BLOCKED		0x0001	/* map_cell.flags: aus TILE_BLOCKED (Bedeutung vermutet) */
#define CELL_CHANGED		0x1000	/* map_cell.flags: Zelle durch Ereignis geaendert (O129 setzt; R12: Ereignis nur ohne) */
#define CELL_WALL0		0x2000	/* map_cell.flags: Flaeche 0 des Zellobjekts ist Wand (R13 cell_profile) */
#define TILE_BLOCKED		0x80	/* tile_info.flags Bit 7 -> CELL_BLOCKED */
#define TILE_ROT_MASK		0x07	/* tile_info.flags Bits 0..2: Drehung - 1 */
#define TILE_KIND_WALL0		1	/* tile_info.kind -> CELL_WALL0 */

/* --- Packarten der Bilddaten (R16 decomp, NVF-Dateien R41) ------------------------------------------ */
#define PACK_NONE		0	/* ungepackt (jede andere Zahl: memcpy) */
#define PACK_RLE		1	/* RLE mit Marke 7Fh, zeilenweise (decomp_rle) */
#define PACK_RLE2		2	/* RLE mit Laengenbyte (decomp_rle2) */
#define PACK_PP20		0x32	/* PowerPacker PP20 (R29 unpack_pp20) */

/* NVF-Bilddateien (R41 nvf_process, O098): Typbyte der Datei */
#define NVF_FILE_RAW		0	/* gleich grosse Bilder, ungepackt */
#define NVF_FILE_RAW_SIZES	1	/* Breite/Hoehe je Bild, ungepackt */
#define NVF_FILE_PACKED		2	/* gleich grosse Bilder, Laenge je Bild, gepackt */
#define NVF_FILE_PACKED_SIZES	3	/* Breite/Hoehe/Laenge je Bild, gepackt */
#define NVF_FILE_PACKED2	4	/* wie 2 (Unterschied zu 2 vermutet: RLE- statt PP20-Daten) */
#define NVF_FILE_PACKED_SIZES2	5	/* wie 3 (ebenso) */
#define NVF_TYPE_MASK		0x7f
#define NVF_FLAG_LEN		0x80	/* entpackte Laenge am Ende der PP20-Daten (BE: va) */
/* Ausgabeart nvf_desc.type */
#define NVF_OUT_RAW		0	/* Typ 0/1 kopieren, sonst PP20 entpacken */
#define NVF_OUT_RLE		2	/* RLE, Zeilenabstand stride */
#define NVF_OUT_RLE_DENSE	3	/* RLE, Zeilen dicht (Abstand = Breite) */
#define NVF_OUT_RLE_MIRROR	4	/* RLE waagrecht gespiegelt, Zeilenabstand stride */
#define NVF_OUT_RLE_MIRROR_DENSE 5	/* RLE gespiegelt, Zeilen dicht */
#define NVF_RLE_MARK		0x7f	/* RLE: 7Fh, Anzahl, Wert */
#define NVF_ROW_BUF		100	/* Zeilenpuffer nvf_desc.buf (Bilder bis 100 Punkte breit spiegelbar) */

/* --- Bildschirm (VGA-Modus 13h, g_vga_buf) ---------------------------------------------------------- */
#define SCREEN_W		320	/* Breite = Zeilenlaenge im Bildpuffer */
#define SCREEN_H		200
/* Zeichenmodi von R46 blit_lines (R42 gb_blit_m, Klasse Bmp put(.., mode)); Ziel SCREEN_W Bytes je Zeile */
#define BLIT_COPY		0	/* kopieren */
#define BLIT_OR			1	/* odern */
#define BLIT_XOR		2	/* exklusiv odern */
#define BLIT_AND		3	/* unden */
#define BLIT_MASKED		4	/* Farbe 0 durchsichtig */
#define BLIT_BEHIND		5	/* nur wo das Ziel eine Farbe < 0xa0 hat (dahinter zeichnen) */
#define BLIT_MASKED_FF		6	/* Farbe 0xff durchsichtig */
#define VGA_CRTC_INDEX		0x3d4	/* CRTC-Indexregister (R45 wait_for_vsync) */
#define VGA_CRTC_DATA		0x3d5	/* CRTC-Datenregister */
#define VGA_INPUT_STATUS	0x3da	/* Eingangsstatus 1 */
#define VGA_ST_NO_DISPLAY	0x01	/* Status: Anzeige aus (Ruecklauf waagrecht oder senkrecht) */
#define VGA_ST_VRETRACE		0x08	/* Status: senkrechter Strahlruecklauf */
#define CRTC_VRETRACE_END	0x11	/* CRTC-Register "Vertical Retrace End" */
#define CRTC_VRE_BIT5		0x20	/* darin Bit 5 (Vertikal-Interrupt aus), R45 loescht es */

/* --- Speicherverwaltung (R07) ----------------------------------------------------------------------- */
#define NR_MEM_BLOCKS		1000	/* Eintraege in g_mem_blocks */

/* --- Videomodus (R43) ------------------------------------------------------------------------------- */
#define VIDEO_MODE_VGA		0x13	/* 320 x 200, 256 Farben */
#define NR_COLORS		256	/* Palette: 3 Bytes je Farbe (struct rgb) */
#define DAC_MAX			0x3f	/* groesster Farbwert einer Komponente (VGA-DAC 6 Bit; R44 pal_fade) */

/* --- Schrift (R20 struct font): Zeichensatz und Steuerzeichen in Texten ----------------------------- */
#define NR_FONT_CHARS		0x58	/* Zeichen im Zeichensatz (g_font_charmap) */
#define FONT_CH_NEWLINE		'@'	/* neue Zeile (auch CR/LF) */
#define FONT_CH_TAB		'~'	/* naechster Tabulator */
#define FONT_CH_SPACE		'^'	/* Leerzeichen (ohne Umbruch) */
#define FONT_CH_COLSET1		0xf1	/* 0xf1..0xf4: Farbsatz 1..4 (font.colsel = c + 0x10) */
#define FONT_CH_COLSET4		0xf4
#define FONT_CH_SHADOW_ON	0xf5	/* Schatten an */
#define FONT_CH_NORMAL		0xf0	/* Schatten aus, Grundfarbe */
#define FONT_ALIGN_CENTER	1	/* font.align: zentriert */

/* --- Umlaute im Zeichensatz (Codepage 437; R30 char_tolower/char_toupper) ---------------------------- */
#define CH_AE_UPPER		0x8e	/* Ae */
#define CH_OE_UPPER		0x99	/* Oe */
#define CH_UE_UPPER		0x9a	/* Ue */
#define CH_AE_LOWER		0x84	/* ae */
#define CH_OE_LOWER		0x94	/* oe */
#define CH_UE_LOWER		0x81	/* ue */

/* --- Dateiobjekte (O037 File, O243 TempFile) -------------------------------------------------------- */
#define FILE_FD_ARC		0x4000	/* File.fd: Handle gehoert zur Archivdatei (Eintrag im Archiv) */
#define FILE_NAME_LEN		14	/* Dateiname im Archivkopf/in der Namensliste */
#define ARC_ENTRY_SIZE		20	/* Archivkopf: Wort "vorhanden", Name[14], long Versatz */
#define FILE_READ_CHUNK		0xfa00	/* Lesen aus dem Archiv in Bloecken */

/* --- AIF-Bilder (O269 Pic) ------------------------------------------------------------------------- */
#define PIC_HDR_SIZE		30	/* Dateikopf (liegt im Objekt selbst) */
#define PIC_PACK_RAW		0	/* Pic.pack: ungepackt */
#define PIC_PACK_RLE		1	/* RLE 0x7f n v */
#define PIC_PACK_PACKBITS	2	/* PackBits */
#define PIC_PACK_PP20		3	/* PowerPacker */
#define PIC_RLE_MARK		0x7f	/* RLE: 7Fh, Anzahl, Wert */

/* --- Kampf (Welle N7: O018..O022, O128..O153) ------------------------------------------------------- */
#define NR_ENEMIES		20	/* g_enemies[20] (ds:c4f7) */
#define FIGHT_ID_SPECIAL	0x493	/* Sonderkampf mit Zwischensequenzen (O020/O141/O142) */
#define FIGHT_ID_EMPTY		0x494	/* Kampf ohne Gegnerzuege */
#define FIGHT_ID_SPECIAL2	0x495	/* Sonderkampf 2 (O021) */
#define FIGHT_ID_MAX_DEF	0xaa	/* bis hier Kampfdaten aus Datei 0x8f (O023), sonst aus der Umgebung (O140) */
#define FIG_FIELD_W		24	/* Kampffeld 24 x 24 Felder */
#define FIG_MAP_PITCH		25	/* Zeilenlaenge der Begehbarkeitskarte g_fight_walkmap und von fight_field.cells */
/* Inhalt der Begehbarkeitskarte (fight_set_cell/fight_find_cell): Figuren mit ihrer ID */
#define FIG_ID_HERO		1	/* 1..7: Held get_hero_index(h) + 1 (wie TARGET_ID_HERO) */
#define FIG_ID_ENEMY		10	/* 10..29: Gegner fight_obj_index(e) + 10 */
#define FIG_ID_ENEMY_TAIL	30	/* 30..49: Schwanzfeld eines Zweifeldgegners */
#define FIG_CELL_HERO_POS	0xc9	/* Aufstellung: Platz eines Helden (O140) */
#define FIG_CELL_ENEMY_POS	0xca	/* Aufstellung: Platz eines Gegners (O140) */
#define FIG_CELL_START		0xcb	/* Eintrittsfeld der Gruppe (O139/O140) */
#define FIG_CELL_BLOCKED	0xfe	/* gesperrt (-2, fight_cell_blocked) */
#define FIG_TWOFIELD_NONE	0xff	/* fig_node.twofielded: keine Zweifeldfigur */
enum {					/* fig_node.kind (BE FIGHTER_IS_ENEMY) */
	FIG_KIND_OBJ = 0,
	FIG_KIND_ENEMY = 1,
	FIG_KIND_HERO = 2
};
enum {					/* Kampfaktionen hero.action_id / enemy.action_id (Werte wie BE FIG_ACTION_*) */
	FIG_ACTION_MOVE = 1,
	FIG_ACTION_MELEE_ATTACK = 2,
	FIG_ACTION_GUARD = 3,		/* Sichern (O155 fig_menu_guard; BE FIG_ACTION_GUARD) */
	FIG_ACTION_SPELL = 4,
	FIG_ACTION_USE_ITEM = 5,
	FIG_ACTION_WAIT = 12,		/* (O020: Sondergegner tut nichts) */
	FIG_ACTION_RANGE_ATTACK = 15
};
/* --- Kampf Teil B (Welle N9: O135..O150 Animationsfolgen, Sonderkampf, Ziele, Wegsuche, Bewegung) ------- */
enum {					/* ani_step.event: Ereignis am Ende eines Schritts (O112/O133) */
	ANI_EV_START_LINKED = 0x1f,	/* Partner-Animation seq.linked starten (Geschoss/Effekt) */
	ANI_EV_END = 0x20,		/* Ende: Figur entfernen (Geschoss, Effekt) */
	ANI_EV_VIEWDIR = 0x22		/* 0x22 + Richtung: Blickrichtung der Figur setzen (fig_enemy_turn) */
};
#define ANI_FRAME_END		0xff	/* ani_step.frame: Ende der Ablaufbeschreibung */
#define FIG_FRAME_FALL		0x26	/* Einzelbild Treffer/Sturz (nach Drehung zu Richtung 1) */
#define FIG_FIGURE_OBJ		0x91	/* ani_seq.figure fuer Geschosse/Effekte/Feldobjekte */
#define FIG_FIGURE_SFIGHT	0x9f	/* ani_seq.figure der Sonderfigur (Sonderkampf 0x493) */
#define FIG_FIGURE_SFIGHT2	0x42	/* ani_seq.figure des Gegners im Sonderkampf 0x495 (O021; grosser Bildpuffer 0xcb20, O016) */
#define FIG_FIGURE_HERO		0xb5	/* ani_seq.figure der Helden 0xb5..0xc8 (und 0xfc; O016 fig_anim_sound) */
#define FIG_FIGURE_HERO_LAST	0xc8
#define FIG_FIGURE_HERO_X	0xfc
#define FIG_FIGURE_ENEMY	0xc9	/* ani_seq.figure der Gegner 0xc9..0xf9 (gfx_id + 0xb5) */
#define FIG_FIGURE_ENEMY_LAST	0xf9
#define ANI_EV_SOUND		0x0c	/* Ereignisse 0x0c..0x1e: Klang (fig_anim_sound) */
#define ANI_EV_SOUND_LAST	0x1e
#define ANI_EV_SFIGHT		0x21	/* fig_anim_draw: Sonderfigur im Kampf 0x493 in bestimmten Folgen (O020) */
#define ANI_EV_PATH		0x29	/* fig_anim_draw: kein Ereignis, Pfad laeuft */
#define ANIM_SLOTS		40	/* XMS-Plaetze fuer kleine Einzelbilder (g_anim_slots) */
#define ANIM_SMALL_MAX		0x550	/* kleine Einzelbilder (w * h) im XMS-Platz bzw. g_anim_small_buf */
#define ANIM_XMS_BASE		0x1f400L /* XMS-Versatz des ersten Platzes (hinter zwei Bildschirmen) */
#define FIG_CELL_OBJ		50	/* Feldwerte >= 50: Waende/Objekte (fig_obj_transparent(v - 50)) */
#define FIG_CELL_GOAL		0xcb	/* Zielmarke fuer fight_path_search (fig_move_check; = FIG_CELL_START) */
#define FIG_CELL_WALL		0xcc	/* gesperrt (Hindernis, Sonderkampf-Waende, unbewegliche Figur) */
#define FIG_CELL_WALL2		0xcd	/* gesperrt (zweite Hindernisliste des Sonderkampfs) */
#define FIG_CELL_HERO_GOAL	0xce	/* 0xce..0xd4: Zielmarke Held 0..6 (fig_mark_hero_targets) */
#define FIG_CELL_ENEMY_GOAL	0xd5	/* 0xd5..0xe8: Zielmarke Gegner 0..19 (fig_mark_enemy_targets) */
#define FIG_CELL_SFIGHT		0xfd	/* Feld der Sonderfigur (gilt als Gegner 10, g_sfight_cells) */
#define SFIGHT_GRID_W		13	/* Bildraster der Sonderfigur: 13 Spalten (g_sfight_pos % 13) */
enum {					/* fig_move_check: Ergebnis problem (BE FIG_move_hero) */
	MOVE_OK = 0,
	MOVE_ESCAPE = 1,		/* Ziel ueber den Rand: Flucht */
	MOVE_TOO_FAR = 2,		/* nicht genug BP */
	MOVE_BLOCKED = 3,		/* Feld besetzt */
	MOVE_NO_PATH = 4,		/* kein Weg */
	MOVE_CANCEL = 5			/* Abbruch (Esc/rechte Maustaste) */
};
enum {					/* fig_move_check: Fluchtrichtung ueber den Kartenrand (nicht die Pfadrichtung) */
	ESCAPE_NONE = 0,
	ESCAPE_LEFT = 1,		/* x < 0, Pfadschritt 2 */
	ESCAPE_DOWN = 2,		/* y < 0, Pfadschritt 1 */
	ESCAPE_UP = 3			/* y > 23, Pfadschritt 3 */
};
#define ACTION_MOUSE_MOVED	0x3e7	/* fig_move_hero: Mauszeiger auf ein anderes Feld bewegt (in g_action) */
#define FIG_PATH_END		(-1)	/* g_fig_path: Ende */
#define FIG_PATH_SHORT		(-2)	/* g_fig_path: hinter dem Ende - BP reichen nicht fuer den ganzen Weg */

/* --- Reiseobjekt Travel.status[] / shown[] (Statusbild und Texte der Reisekarte, O216) ----------------------- */
#define TST_ROAD		0	/* Wegzustand (travel.road) */
#define TST_TERRAIN		1	/* Gelaende (travel.terrain) */
#define TST_WEATHER		2	/* Wetter (Wx_get) */
#define TST_SPEED		3	/* Tempostufe 0..4 (travel_level) */
#define TSH_DAY			3	/* nur shown[]: angezeigt Tag (1) bzw. Nacht (0), g_is_day */

/* --- Zeitzaehler g_ingame_timers[] (Index wie BE INGAME_TIMER_*; DSA 2 bestaetigt: O192) ---------------------- */
#define INGAME_TIMER_RONDRA_NO_SPELLS	0	/* Rondra-Wunder: Zauber gesperrt (O192 enemy_test_spell: Gegner zaubern nicht) */
#define INGAME_TIMER_STURMGEBRUELL	26	/* Sturmgebruell gewirkt (O237 setzt 0x1c2; Leser offen) (vermutet) */
#define INGAME_TIMER_SILENTIUM		27	/* Silentium: Sperre/Dauer 0x1c2 Ticks (O237: nur bei 0 wirksam) */
#define INGAME_TIMER_BORON_UNDEAD	5	/* Boron-Wunder: Schutz vor Untoten (O192 enemy_attack_damage: halber Schaden) */
#define INGAME_TIMER_EFFERD_FIND_WATER	1	/* Efferd-Wunder: Wasser im Lager immer gefunden (3 Tage; O033) */
#define INGAME_TIMER_EFFERD_SAFE_PASSAGE 2	/* Efferd-Wunder: sichere Seefahrt (3 Tage; O033) */
#define INGAME_TIMER_FIRUN_HUNT		3	/* Firun-Wunder: Jagd im Lager immer erfolgreich (3 bzw. 7 Tage; O033) */
#define INGAME_TIMER_TRAVIA_SAFE_REST	4	/* Travia-Wunder: kein Ueberfall in der Nacht (7 Tage; O033) */
#define INGAME_TIMER_AKROBATIK		6	/* Talent Akrobatik gesperrt (BE) */
#define INGAME_TIMER_MUSIZIEREN		7	/* Talent Musizieren gesperrt (BE) */
#define INGAME_TIMER_FLIM_FLAM		8	/* Flim Flam wirkt (BE) */
#define INGAME_TIMER_DARKNESS		9	/* Dunkelheit wirkt (BE) */
#define INGAME_TIMER_PRAIOS_MU		10	/* Praios-Wunder MU +1 gesperrt (1 Tag; O033) */
#define INGAME_TIMER_PRAIOS_MR		11	/* Praios-Wunder MR +99 gesperrt (3 Tage; O033) */
#define INGAME_TIMER_RONDRA_SWORDS	12	/* Rondra-Wunder Schwerter/AT gesperrt (3 Tage; O033) */
#define INGAME_TIMER_RONDRA_MAGIC_WEAPON 13	/* Rondra-Wunder magische Waffe gesperrt (1 Tag; O033) */
#define INGAME_TIMER_EFFERD_SWIM	14	/* Efferd-Wunder Schwimmen +2 gesperrt (4 Tage; O033) */
#define INGAME_TIMER_BORON_TA		15	/* Boron-Wunder TA -1 gesperrt (4 Tage; O033) */
#define INGAME_TIMER_HESINDE_ANALUES	16	/* Hesinde-Wunder Analues +1 gesperrt (4 Tage; O033) */
#define INGAME_TIMER_HESINDE_MR		17	/* Hesinde-Wunder MR +5 gesperrt (3 Tage; O033) */
#define INGAME_TIMER_FIRUN_SATED	18	/* Firun-Wunder satt gesperrt (7 Tage; O033) */
#define INGAME_TIMER_PHEX_THIEF		19	/* Phex-Wunder Schleichen/Taschendiebstahl gesperrt (3 Tage; O033) */
#define INGAME_TIMER_PHEX_FEILSCHEN	20	/* Phex-Wunder Feilschen gesperrt (3 Tage; O033) */
#define INGAME_TIMER_PHEX_FF		21	/* Phex-Wunder FF +1 gesperrt (3 Tage; O033) */
#define INGAME_TIMER_INGERIMM_MAGIC_WEAPON 22	/* Ingerimm-Wunder magische Waffe gesperrt (1 Tag; O033) */
#define INGAME_TIMER_RAHJA_TALENTS	23	/* Rahja-Wunder Tanzen/Betoeren gesperrt (7 Tage; O033) */
#define INGAME_TIMER_RAHJA_CH		24	/* Rahja-Wunder CH +1 gesperrt (3 Tage; O033) */
#define INGAME_TIMER_RAHJA_TALENTS_PERMANENT 25	/* Rahja-Wunder Tanzen/Betoeren dauerhaft gesperrt (7 Tage; O033) */
/* Skript der Spielende-Sequenz (struct finale_step.op, O036) */
enum {
	FINALE_OP_END = 0,		/* Ende */
	FINALE_OP_FLI = 1,		/* FLI-Animation arg (mit Rueckruf finale_fli_cb) */
	FINALE_OP_SPEECH = 2,		/* Sprachausgabe arg (bei g_cd_speech) */
	FINALE_OP_TEXT = 3,		/* Text txt[arg] zeigen (bei g_subtitles) */
	FINALE_OP_STOP = 4		/* Musik anhalten */
};

/* CD-Audio (Klasse CD, O004; SndSetup O043) */
#define CD_PORT			0xcdcd	/* SndSetup.music_port: Musik von CD-Audio statt Treiber */
#define CD_ERR_NO_DRIVE		0x3b9	/* kein CD-Laufwerk (MSCDEX) */
#define CD_ERR_MSCDEX_OLD	0x3ba	/* MSCDEX aelter als 2.10 */
#define CD_ERR_NOT_CD		0x3bb	/* angegebenes Laufwerk ist kein CD-Laufwerk */
#define CD_ERR_OPEN		0x3bc	/* Tuer offen bzw. keine CD (CD::change) */
#define CD_ERR_TRACK		0x3bd	/* Titel nicht auf der CD (CD::play) */
#define CD_ERR_PLAY		0x3be	/* Titel spielt nicht (CD::play) */
#define CD_ERR_DISC		0x3bf	/* falsche bzw. unbekannte CD (CD::read_disc, CD::music) */

/* Art des Musiktreibers (Sound::mtype nach ail_drv_desc.suffix, O042) */
#define SND_MTYPE_AD		1	/* "AD" (AdLib) */
#define SND_MTYPE_OPL		2	/* "OPL" */
#define SND_MTYPE_MT		3	/* "MT" (Roland MT-32: Klangfarben aus SAMPLE.MT) */
#define SND_MTYPE_OTHER		4

/* Schluessel des CD-Caches (O003): Typ in Bits 13..15 | Dateinummer */
#define CACHE_TYPE_MAIN		0x2000	/* g_file_main (STAR.DAT) */
#define CACHE_TYPE_FX		0x4000	/* g_file_fx (FX.DAT) */
#define CACHE_TYPE_SPEECH	0x6000	/* g_file_speech (SPEECH.DAT) */
#define CACHE_TYPE_RAW		0x8000	/* g_file_raw (RAW.DAT) */
#define CACHE_TYPE_CD		0xa000	/* g_file_cd (STARCD.DAT, FLI-Animationen) */
#define CACHE_TYPE_SPEECH_EXT	0xc000	/* g_file_speech_ext (SPEECHCD.DAT, Nummer - 1000) */
#define CACHE_TYPE_MASK		0xe000
#define CACHE_NR_MASK		0x1fff

/* Mauszeiger (mouse_set_cursor) */
#define CURSOR_WAIT		5	/* Wartezeiger beim Laden in den CD-Cache (O003, O007) */

#define ITEM_SALAMANDER_STONE	0x117	/* Salamanderstein (O007 blackmage_scene: entscheidet den Ausgang; SEQUENCE.LTX) */

/* --- Welle N18: Heldenerschaffung, Heldendateien, Spielstaende ------------------------------------------------ */
#define GAME_MODE_BEGINNER	0	/* g_game_mode: Anfaenger (Werte und Steigerungen automatisch) */
#define GAME_MODE_ADVANCED	1	/* Fortgeschrittene (BE GAME_MODE_ADVANCED = 2) */
#define FILE_HEROES		0xfb	/* Heldendatei des Spielstands (TEMP; ueber Namenseintrag 0 von g_names_star_idx) */
#define FILE_HEROES_ALL		0xfa	/* Heldendatei aller erschaffenen Helden (TEMP und Kopie in GAMES) */
#define CHARGEN_PAGE_MAIN		0	/* CharGen.page: Grundwerte */
#define CHARGEN_PAGE_ATPA		4	/* AT/PA; 1..3 Talente */
#define CHARGEN_PAGE_SPELLS		5	/* 5..10 Zauber (nur Zauberkundige) */
#define CHARGEN_PAGE_LAST		10
#define TYPE_REQ_MAX		0x80	/* type_req.value: Hoechstwert statt Mindestwert */
#define SAVE_TAG_CHAR		0x52414843L	/* Blockkennungen im Spielstand ("CHAR" ...), als long gelesen */
#define SAVE_TAG_PART		0x54524150L
#define SAVE_TAG_XDIA		0x41494458L
#define SAVE_TAG_TIME		0x454d4954L
#define SAVE_TAG_AMAP		0x50414d41L
#define SAVE_TAG_GLOB		0x424f4c47L
#define SAVE_TAG_RUNV		0x564e5552L
#define SAVE_TAG_MONS		0x534e4f4dL
#define SAVE_TAG_PLAY		0x59414c50L
#define SAVE_TAG_LOCS		0x53434f4cL
#define SAVE_TAG_TEMP		0x504d4554L
#define SAVE_BLOCKS_START	0x114L	/* erster Block hinter Kopf (0x10) und Beschreibung ("DESC" + 0x100) */
#define NR_SAVEGAMES		100	/* SaveMenu.names */
#define ITEM_MAGE_STAFF		0x85	/* Magierstab (O177: Magier bekommen ihn beim DSA-1-Import neu) (BE ebenfalls 0x85) */

/* --- FLI-Animationen (O024/O025) ------------------------------------------------------------------------- */
#define FLI_MAGIC		0xaf11	/* fli_header.magic */
#define FLI_FRAME_MAGIC		0xf1fa	/* fli_frame.magic */
#define FLI_COLOR		11	/* Chunk: Palette (Paare Sprung/Anzahl, 6 Bit) */
#define FLI_LC			12	/* Chunk: zeilenweise Aenderungen */
#define FLI_BLACK		13	/* Chunk: Bild loeschen */
#define FLI_BRUN		15	/* Chunk: lauflaengenkodiertes Vollbild */
#define FLI_COPY		16	/* Chunk: unkomprimiertes Vollbild */
#define FLI_ERR_OPEN		0x3b5	/* g_fli_error: Datei nicht zu oeffnen */
#define FLI_ERR_READ		0x3b6	/* Lesefehler */
#define FLI_ERR_MAGIC		0x3b7	/* falsche Kennung (Kopf/Rahmen) */
#define FLI_ERR_MEM		0x3b8	/* kein Speicher fuer den Rahmen */
#define FLI_SCREEN_SIZE		0x7d00	/* 320 x 200 Bytes */

/* --- 3D-Objekte (O010/O097) ------------------------------------------------------------------------------- */
#define VTX_PATH		0x7fff	/* vtx.x: bewegter Eckpunkt, vtx.y = Index in g_vtx_paths */

/* --- Welle N21: Gegenstaende und Werte der Orts-/Dungeonereignisse (Namen nach ITEMS.LTX) --- */
#define ITEM_ASTHENIL_RING      0x11e   /* Asthenilring (O000 Vision am Altar, O001 schuetzt, O060 aus dem Kohlebecken) */
#define ITEM_RUNE_DOCUMENT      0x12b   /* Runendokument (O000 Ingerimm-Relief, O060 Skelett) */
#define ITEM_DOUBLE_KEY         0xd0    /* Doppelbartschluessel (O001 Dungeon 16) */
#define ITEM_ASTHENIL_SWORD     0xeb    /* Asthenilschwert (O001/O002 Golem, Tausch gegen den Salamanderstein) */
#define ITEM_LEATHER_TROUSERS   0x137   /* Lederhose (O001/O002: Schutz vor den Gluehplatten und der Esse) */
#define ITEM_LEATHER_BOOTS      0x138   /* Lederstiefel */
#define ITEM_LEATHER_JERKIN     0x139   /* Lederwams */
#define ITEM_STONE_MEDALLION    0x13c   /* Steinmedaillon (O002: fuer den Golem) */
#define ITEM_GOLD_KEY           0xc3    /* Goldschluessel (reich verziert): Schluessel zur Binge (O019, O258 Priester, O060 Tore) */
#define ITEM_FOOD_PACKAGE       0x2d    /* Proviantpaket (O031 Gardehaus: Vorraete zaehlen) */
#define ITEM_BROOCH             0xfd    /* Brosche (O031 Schauhaus, Auftrag Dragans) */
#define ITEM_SHOVEL             0x49    /* Schaufel (O064 Stollen freischaufeln) */
#define ITEM_ROPE               0x79    /* Seil (O064 Grube) */
#define ITEM_WAR_AXE            0x87    /* Kriegsbeil (O064 Zwergengrab) */
#define ITEM_ORE_LUMP           0xb5    /* Erzklumpen (O063 Erzhaufen) */
#define ITEM_AXLE_GREASE        0x156   /* Wagenschmiere (O063 Lore) */
#define ITEM_LIQUORICE          0x59    /* Lakritze (O073: Gast bekommt 5 Stueck) */
#define ITEM_SWEETS             0x5a    /* Bonbons (O073: Gast bekommt 10 Stueck) */
#define ITEM_COPPER_KETTLE      0x127   /* Kupferkessel (O113 Haendler mit Kiepe, 12 Silber) */
#define ITEM_BOOTS              0x33    /* Stiefel (O115 tevent_045) */
#define INV_SLOT_FEET           14      /* Inventarplatz Fuesse (O115; O213 INV_FEET) */
#define ITEM_PICKAXE            0x5d    /* Hacke (O119 tevent_110: Weg freilegen) */
#define ITEM_SHORTBOW           9       /* Kurzbogen (O120 Jagd) */
#define ITEM_ARROW              10      /* Pfeil */
#define ITEM_CROSSBOW           0x0c    /* Armbrust */
#define ITEM_BOLT               0x0d    /* Bolzen */
#define ITEM_TWO_HANDED_SWORD   0x12    /* Zweihaender (O120 tevent_135, 1500 Heller) */
#define ITEM_LONGBOW            0x13    /* Langbogen */
#define ITEM_LOWANGEN_KEY       0x133   /* Lowangen-Schluessel (O125 place_event_103) */

/* --- Welle N23: Gegenstaende und Werte der Dungeonereignisse (Namen nach ITEMS.LTX) --- */
#define ITEM_SILVER_KEY         0xc9    /* Silberschluessel (reich verziert): Schluessel aus der Binge (O060 Portal, O062 Losungswort) */
#define ITEM_GEMS               0xfa    /* Edelsteine (O059 Schatulle im Wandloch) */
#define ITEM_CRANK              0x157   /* Kurbel (O061 Kurbellager hinter dem Fallblock) */
#define ITEM_CHAIN              0x158   /* Kette (O061 Mechanik des Fallblocks) */
#define ITEM_FIRE_POWDER        0x13b   /* Feuerpulver (O065 in den Feuerschlund geworfen) */
#define ITEM_CROWBAR            0x1a    /* Brecheisen (O066 Platte am Beckengrund anheben) */
#define ITEM_OTTER_PELT         0x123   /* Otterfell (O118 tevent_094, O122 tevent_167 Tausch gegen starken Heiltrank) */
#define ITEM_MANDRAKE           0x7e    /* Alraune (O122 tevent_165) */
#define ITEM_SNOWSHOES          0x34    /* Schneeschuhe (O122 tevent_181: Schneewehen) */
#define ITEM_SALAMANDER_STONE_FAKE 0xfc /* Salamanderstein (Faelschung; O017 Schwarzmagier: "Das ist eine Faelschung!") */
#define ITEM_ARTHERION_SWORD    0xff    /* Schwert des Artherion (O017 place_event_087) */
#define ITEM_ARTHERION_BOW      0x100   /* Bogen des Artherion (O017 place_event_087) */

/* --- Welle N24: Gegenstaende und Werte der Reiseereignisse O123/O124 und der Dungeons 1..8 (Namen nach
 * ITEMS.LTX) --- */
#define ITEM_DRIED_WHIRLWEED    0x10b   /* Trockenwirselkraut (O124 tevent_234, O130 Krug) */
#define ITEM_EINBEERE_DRIED     0x136   /* Vierblaettrige Einbeere (trocken), zweite Sorte 0x135 (O124 tevent_234) */
#define ITEM_BRONZE_KEY         0xc2    /* Bronzeschluessel (O130 Dungeon 2: im alten Stiefel, oeffnet die Wand 0x1211) */
#define ITEM_FISHHOOK           0x1c    /* Angelhaken (O130 unter dem Kissen) */
#define ITEM_RECIPE_SLEEPING_POTION 0xf2    /* Rezept fuer Schlaftrunk (O130 unter den Papieren) */
#define ITEM_AMULET_PART1       0x10d   /* Amuletteil 1 (O130: von den Skelettkriegern, vier Teile ergeben den sonnenfoermigen Schluessel) */
#define ITEM_AMULET_PART2       0x10e   /* Amuletteil 2 (O130) */
#define ITEM_AMULET_PART3       0x10f   /* Amuletteil 3 (O130) */
#define ITEM_AMULET_PART4       0x110   /* Amuletteil 4 (O130) */
#define ITEM_BLACK_STATUETTE    0xf8    /* Schwarze Statuette (O132 Dungeon 1: aus der schwarzen Statue, saugt Dunkelheit auf) */
#define ITEM_DOCUMENT_15A       0x15a   /* Dokument (O132 Dungeon 1: Pergament bei den Gegnern am Eingang; ITEMS.LTX[346]) */
#define ITEM_FLAME_KEY          0x15b   /* Flammenschluessel (O132 Dungeon 1: im Dornenbaum-Kuebel und im Brunnen der Kriegerin; Schloss 0xae) */
#define ITEM_SWAMP_RANTZE       0x146   /* Sumpfrantze (O195: jault bei der richtigen Heidekrautstaude auf) */
#define ITEM_HEATHER_MAGIC      0x14e   /* Heidekraut, die achtzehnte Staude am Wasser (O195 Dungeon 4; ITEMS.LTX[334], vermutlich der verwandelte Vermisste) */
#define ITEM_HEATHER            0x14f   /* Heidekraut, eine der anderen Stauden (O195; ITEMS.LTX[335]) */
#define ITEM_RECIPE_GELDSCHEISSER 0x152   /* Rezept fuer Geldscheisser (O204 Phex-Tempel: alte Schriftrolle an 0xb12, Gitter faellt, Skelette) */
#define ITEM_RECIPE_MAGIC_POTION 0xce    /* Rezept fuer Zaubertrank (O204: vergilbte Schriftrolle an 0xc18, oeffnet das Gitter wieder) */
#define ITEM_BLUE_KEY           0x134   /* Schluessel (blau) (O204: zwischen den Gebeinen der Skelettkrieger) */
#define ITEM_PHEX_HELMET        0x15e   /* Phexenhelm (O204 Truhe 0x30, der Poltergeist holt ihn zurueck) */
#define ITEM_PHEX_SHIELD        0x15f   /* Phexenschild (O204 Truhe 0x30, der Poltergeist holt ihn zurueck) */
#define ITEM_ORC_JEWELRY        0x113   /* Orkschmuck (O262 Brazoragh-Statue, Strohlager) */
#define ITEM_ORC_SABRE          0x112   /* Orksaebel (Arbach; O262 Strohlager) */
#define ITEM_SABRE              3       /* Saebel (O262 Strohlager der Oger) */
#define ITEM_WAR_AXE_8          8       /* Kriegsbeil (ITEMS.LTX[8], alte Axt des Zwergenskeletts in O262; zweites Kriegsbeil ITEM_WAR_AXE 0x87) */
#define INGAME_TIMER_DNG06_DIRT 29      /* Dungeon 6: Held im Stroh eingesaut, Wirkung fuer 0x1fa40 Ticks (O262 dng06_straw; g_ingame_timers + 0x74) */
#define ITEM_POT_HELMET         0x4d    /* Topfhelm (O261 Dungeon 6 an 0x90a) */
#define ITEM_HEAVY_DAGGER       0x6e    /* Dolch (schwer) (O261 Gravesh-Schrein) */
#define INGAME_TIMER_DNG06_WELL 34      /* Dungeon 6: Rast an Brunnen/Traenke - LE + 1 und Wirkung einmal je 0x1fa40 Ticks (O261; g_ingame_timers + 0x88) */
#define ITEM_SPEAR              5       /* Speer (O265: zum Stochern im Loch ueber der Tuer; auch N22 O163) */
#define ITEM_PIKE               0x2c    /* Pike (O265) */
#define ITEM_FIGHTING_STAFF     0x45    /* Kampfstab (O265) */
#define ITEM_GOLD_JEWELRY       0x4a    /* Goldschmuck (O265: Gold der Brazoragh-Statue) */
#define ITEM_MIRROR_AMULET      0x141   /* Spiegelamulett (O265 Stroh in der Zelle) */
#define ITEM_ORNATE_SWORD       0x154   /* Schwert mit reich verziertem Griff (O265 hinter dem Spinnennetz; ITEMS.LTX[340]) */
#define INGAME_TIMER_DNG07_DIRT 30      /* Dungeon 7: im Stroh eingesaut, Wirkung fuer 0x1fa40 Ticks (O265 dng07_straw; g_ingame_timers + 0x78) */
#define ITEM_PINCER_KEY         0x12e   /* Zangenschluessel (O266: Ork nach dem Kampf an 0x100b, BLUT2.LTX[48]) */
#define ITEM_GLASSES_KEY        0xde    /* Brillenschluessel (golden) (O266: Ork nach dem Kampf an 0xa0d, BLUT2.LTX[46]) */
#define INGAME_TIMER_DNG07_PATROL 33      /* Dungeon 7: Patrouille an 0x30e/0x310/0x317 besiegt, kommt nach 0x2a30 Ticks wieder (O266; g_ingame_timers + 0x84) */
#define ITEM_BATTLE_AXE         0x0b    /* Streitaxt (O267: Handbeil eines Ogers, BLUT3.LTX[16]) */
#define ITEM_SWORD              1       /* Schwert (O267 Strohlager) */
#define ITEM_MACE_12A           0x12a   /* Streitkolben aus dem Gravesh-Schrein (O267; ITEMS.LTX[298]) */
#define ITEM_COPPER_DISC        0x78    /* Kupferscheibe (O267 Kaestchen mit der Mumienhand) */
#define INGAME_TIMER_DNG08_WELL 36      /* Dungeon 8: Wasserfass, LE + 1 und Wirkung einmal je 0x1fa40 Ticks (O267; g_ingame_timers + 0x90) */

/* --- Welle N22: Kalender, Wetter, Orte der Reisekarte ------------------------------------------------------ */
/* Jahreszeit (date_season, O250; Texte GLOBLTXT.LTX[322 + n]) */
#define SEASON_WINTER		0	/* Monate 7..9 (Firun, Tsa, Phex) */
#define SEASON_SPRING		1	/* Monate 10..12 (Peraine, Ingerimm, Rahja) */
#define SEASON_SUMMER		2	/* Monate 1..3 (Praios, Rondra, Efferd) */
#define SEASON_AUTUMN		3	/* sonst (Monate 4..6 Travia, Boron, Hesinde; Namenlose Tage) */
/* Wetter Wx.level (Wx_get, O256; Anzeigetexte MMASK.LTX[15 + 3 * n ..]) */
#define WEATHER_HOT		0	/* "Ganz schoen heiss heute." */
#define WEATHER_FINE		1	/* "Heute ist ein schoener Tag!" */
#define WEATHER_CLOUDY		2	/* "Das Wetter ist heute mittelpraechtig!" */
#define WEATHER_RAIN		3	/* "Leichter Regen, aber Ihr seid noch nicht total durchnaesst." */
#define WEATHER_STORM		4	/* "Ein scheussliches Wetter, Ihr seid voellig durchnaesst." */
/* Orte (group.town; Namen GLOBLTXT.LTX[355 + n]): 1..6 Staedte mit 3D-Karte, 7..19 Doerfer der Reisekarte */
#define TOWN_KVIRASIM		1
#define TOWN_GASHOK		2
#define TOWN_FINSTERKOPPEN	3
#define TOWN_LOWANGEN		4
#define TOWN_TIEFHUSEN		5
#define TOWN_TJOLMAR		6
#define TOWN_ANSVELL		7
#define TOWN_AUMOND		8
#define TOWN_HILLHAUS		9
#define TOWN_NEULOWANGEN	10
#define TOWN_REICHSEND		11
#define TOWN_NORDHAG		12
#define TOWN_YRRAMIS		13
#define TOWN_ARSINGEN		14
#define TOWN_SVELLMIA		15
#define TOWN_SIBRA		16
#define TOWN_HILVALLA		17
#define TOWN_NORHUS		18
#define TOWN_HILTORP		19
#define TTX_TOWN_NAMES		355	/* g_ttx[TTX_TOWN_NAMES + town] = Ortsname */
/* Gebaeude der Doerfer (O126/O127 place_event_*): set_loctype(LOC_DATA | n) betritt Ort n der Ortsdaten */
#define LOC_VILLAGE_INN		(LOC_DATA | 1)	/* Herberge */
#define LOC_VILLAGE_TEMPLE	(LOC_DATA | 2)	/* Tempel */
#define LOC_VILLAGE_SMITH	(LOC_DATA | 3)	/* Schmiede (Hiltorp) */

#define ITEM_SPEAR_D8		0xd8	/* Speer (zweite Art; O163 deer_hunt) */
#define ITEM_HEAVY_CROSSBOW	0x101	/* schwere Armbrust (O163 deer_hunt) */
#define INGAME_TIMER_WELL_LOWANGEN	37	/* Brunnen in Lowangen: LE + 1 einmal am Tag (O186; g_ingame_timers + 0x94) */
#define INGAME_TIMER_DNG07_STENCH	32	/* Dungeon 7: Gestank aus dem Hundezwinger (Wirkung fuer einen Tag; O264, g_ingame_timers + 0x80) */
#define INGAME_TIMER_WELL_DNG07	35	/* Dungeon 7: Ziehbrunnen, LE + 1 einmal am Tag (O264, + 0x8c) */
/* --- Spielzustand g_game_state (ds:394b; R01 main_loop) ------------------------------------------------------ */
#define GAME_STATE_RUNNING	0	/* Spiel laeuft (main_loop) */
#define GAME_STATE_DEAD		1	/* alle Helden kampfunfaehig (O074 check_heroes_down): Bild 0x337, Spielstandmenue */
#define GAME_STATE_2		2	/* Spielstandmenue ohne Esc (O107); Bedeutung offen */
#define GAME_STATE_END4		4	/* Endsequenz (show_end_sequence), dann Spielstandmenue */
#define GAME_STATE_FINISHED	5	/* nach dem Spielende geladen (g_finale_done/g_game_finished2): Meldung g_ttx[474], Ende */
#define GAME_STATE_END8		8	/* Endsequenz (show_end_sequence) */
#define GAME_STATE_FINALE	0x63	/* Finale (Klasse Finale, O035) */

/* --- Welle KS: Kopierschutz (SEG003 Handbuchabfrage, R05 CD-Pruefung), Zeitgeber (SEG023) ---------------------- */
#define FILE_PROTECT_LTX	0x368	/* PROTECT.LTX: Handbuchfrage (Seite/Zeile/Wort) und unbenutzte Kartenfragen */
#define FILE_PASSWORD_DAT	0x3a3	/* PASSWORD.DAT: Wort Anzahl, dann je 25 Bytes struct pw_rec */
#define COPYPROT_MARKER		0x56312387L	/* Muster vor markierten Aufrufen: EB 04 87 23 31 56 (jmp short +4, 4 Bytes) */
#define TIMER_INT		0x1c	/* BIOS-Zeitgeber-Rueckruf (timer_init haengt timer_isr ein) */

/* --- Phase B: Gegenstaende der Startausruestung (O084 startup_equipment; Namen nach ITEMS.LTX) --------------- */
#define ITEM_KNIFE              0x04	/* Messer */
#define ITEM_OBSIDIAN_DAGGER    0x15	/* Vulkanglasdolch */
#define ITEM_HAMMER             0x1b	/* Hammer */
#define ITEM_LEATHER_ARMOR      0x35	/* Lederharnisch */
#define ITEM_MACE               0x42	/* Streitkolben */
#define ITEM_BLANKET            0x48	/* Decke */
#define ITEM_ROBE               0x4b	/* Robe */
#define ITEM_THROWING_KNIFE     0x62	/* Wurfmesser */
#define ITEM_RAPIER             0x75	/* Rapier */
#define ITEM_JORUGA_ROOT        0x82	/* Jorugawurzel */
#define ITEM_SKRAJA             0x86	/* Skraja */
#define ITEM_SEAL_KILLER        0x8a	/* Robbentoeter */

/* Kalender (Abschlusswelle A): Monate date.month 1..12 (Name g_ttx[4 + Monat], GLOBLTXT.LTX[5..16]);
 * besondere Tage date.special 1..33 (Name g_ttx[273 + n], GLOBLTXT.LTX[274..306]; Tabelle g_special_days SEG021) */
#define MONTH_PRAIOS               1
#define MONTH_RONDRA               2
#define MONTH_EFFERD               3
#define MONTH_TRAVIA               4
#define MONTH_BORON                5
#define MONTH_HESINDE              6
#define MONTH_FIRUN                7
#define MONTH_TSA                  8
#define MONTH_PHEX                 9
#define MONTH_PERAINE              10
#define MONTH_INGERIMM             11
#define MONTH_RAHJA                12
#define MONTH_NAMELESS_DAYS       13	/* vermutet: Namenlose Tage (Date::next_day: month > 12 -> day -5..-1) */
#define HOLIDAY_SOMMERSONNENWENDE    1	/* Sommersonnenwende */
#define HOLIDAY_PRAIOSFEST           2	/* Praiosfest */
#define HOLIDAY_SCHWUR               3	/* Tag des Schwurs */
#define HOLIDAY_SCHWERTFEST          4	/* Schwertfest */
#define HOLIDAY_WASSER               5	/* Tag des Wassers */
#define HOLIDAY_RASTULLAH4           6	/* Vierter Rastullahellah */
#define HOLIDAY_NEBELFEST            7	/* Nebelfest */
#define HOLIDAY_FISCHERFEST          8	/* Fischerfest */
#define HOLIDAY_PRUEFUNGSFEST        9	/* Pruefungsfest */
#define HOLIDAY_HEIMKEHR             10	/* Tag der Heimkehr */
#define HOLIDAY_FRUECHTE             11	/* Fest der eingebrachten Fruechte */
#define HOLIDAY_HELDEN               12	/* Tag der Helden */
#define HOLIDAY_TREUE                13	/* Tag der Treue */
#define HOLIDAY_TOTENFEST            14	/* Totenfest */
#define HOLIDAY_RASTULLAH5           15	/* Fuenfter Rastullahellah */
#define HOLIDAY_KAISER               16	/* Des Kaisers Geburtstag */
#define HOLIDAY_GROSSER_SCHLAF       17	/* Tag des grossen Schlafes */
#define HOLIDAY_ROHALS_VERHUELLUNG   18	/* Rohals Verhuellung */
#define HOLIDAY_ERLEUCHTUNG          19	/* Erleuchtungsfest */
#define HOLIDAY_JAGD                 20	/* Tag der Jagd */
#define HOLIDAY_IFIRN                21	/* Tag der Ifirn */
#define HOLIDAY_RASTULLAH1           22	/* Erster Rastullahellah */
#define HOLIDAY_ERNEUERUNG           23	/* Tag der Erneuerung */
#define HOLIDAY_PHEX                 24	/* Tag des Phex */
#define HOLIDAY_GLUECKSTAG           25	/* Glueckstag */
#define HOLIDAY_VERSENKUNG           26	/* Versenkungsfest */
#define HOLIDAY_SAATFEST             27	/* Saatfest */
#define HOLIDAY_RASTULLAH2           28	/* Zweiter Rastullahellah */
#define HOLIDAY_FEUER                29	/* Tag des Feuers */
#define HOLIDAY_AUFBRUCH             30	/* Tag des Aufbruchs */
#define HOLIDAY_WAFFENSCHMIEDE       31	/* Tag der Waffenschmiede */
#define HOLIDAY_FREUDEN              32	/* Fest der Freuden */
#define HOLIDAY_REINIGUNG            33	/* Reinigungsfest */

/* Gespraechsthemen der Gespraechsfuehrung (Conv, O257; Abschlusswelle A): Datei je Thema g_conv_topic_tab[t - 7] (O254),
 * 7..0x29 TALKS_00..TALKS_34.LTX, 0x39 TALKS_35.LTX; Themen ab 0x2a mit Frage STDTALKS.LTX[n] laut XDIALOG.DAT (Satz je Thema). */
#define TOPIC_KVIRASIM           0x07
#define TOPIC_GASHOK             0x08
#define TOPIC_LOWANGEN           0x09
#define TOPIC_TIEFHUSEN          0x0a
#define TOPIC_TJOLMAR            0x0b
#define TOPIC_FINSTERKOPPEN      0x0c
#define TOPIC_ANSVELL            0x0d
#define TOPIC_SALAMANDERSTEINE   0x0e
#define TOPIC_FINSTERKAMM        0x0f
#define TOPIC_SVELLTTAL          0x10
#define TOPIC_SVELLTSUEMPFE      0x11
#define TOPIC_THASCH             0x12
#define TOPIC_BLUTZINNEN         0x13
#define TOPIC_RORWHED            0x14
#define TOPIC_HANDEL             0x15
#define TOPIC_KRAEUTER           0x16
#define TOPIC_WAFFEN             0x17
#define TOPIC_KRANKHEITEN        0x18
#define TOPIC_GIFTE              0x19
#define TOPIC_SCHMIEDEKUNST      0x1a
#define TOPIC_JAGD               0x1b
#define TOPIC_GOD_PRAIOS         0x1c
#define TOPIC_GOD_RONDRA         0x1d
#define TOPIC_GOD_EFFERD         0x1e
#define TOPIC_GOD_TRAVIA         0x1f
#define TOPIC_GOD_BORON          0x20
#define TOPIC_GOD_HESINDE        0x21
#define TOPIC_GOD_FIRUN          0x22
#define TOPIC_GOD_TSA            0x23
#define TOPIC_GOD_PHEX           0x24
#define TOPIC_GOD_PERAINE        0x25
#define TOPIC_GOD_INGERIMM       0x26
#define TOPIC_GOD_RAHJA          0x27
#define TOPIC_GOD_SWAFNIR        0x28
#define TOPIC_GOD_IFIRN          0x29
#define TOPIC_BINGE              0x2a
#define TOPIC_SALAMANDERSTEIN    0x2b
#define TOPIC_INGRAMOSCH         0x2c
#define TOPIC_STERNENSCHWEIF     0x2d
#define TOPIC_SIEBENQUELL        0x2e
#define TOPIC_EGELBRONN          0x2f
#define TOPIC_GAVRON             0x30
#define TOPIC_ARTHERION          0x31
#define TOPIC_HENSGER            0x32
#define TOPIC_BANNSTRAHLER       0x33
#define TOPIC_VERMUMMTE          0x34
#define TOPIC_ARMBRUSTBOLZEN     0x35
#define TOPIC_ASCHEHAUFEN        0x36
#define TOPIC_ZUGEZOGENE         0x37
#define TOPIC_KUSLIK             0x38
#define TOPIC_MUEHLE             0x39
#define TOPIC_REISENDE           0x3a
#define TOPIC_ROGOLAN            0x48
#define TOPIC_FLUECHTLINGE       0x49
#define TOPIC_ECHSENMENSCHEN     0x4a
#define TOPIC_STADTMAUER         0x4b
#define TOPIC_FLUCHTTUNNEL       0x4c
#define TOPIC_ARSINGEN           0x4d
#define TOPIC_SIBRA              0x4e
#define TOPIC_TRAVIATEMPEL       0x4f
#define TOPIC_ROGLIMA            0x50
#define TOPIC_BERMAN             0x51
#define TOPIC_VISTELLA           0x52
#define TOPIC_ORKENTOD           0x53
#define TOPIC_VORMTANN           0x54
#define TOPIC_FARMION            0x55
#define TOPIC_DESDIRA            0x56
#define TOPIC_SVELLTJEPALAST     0x57
#define TOPIC_THORESCHA          0x58
#define TOPIC_PAKT               0x59
#define TOPIC_VINSALTER          0x5a
#define TOPIC_AKADEMIEN          0x5b
#define TOPIC_MEISTERSCHMIEDE    0x5c
#define TOPIC_ZASKIA             0x5d
#define TOPIC_KYSIRA             0x5e
#define TOPIC_SUMPFRANTZEN       0x5f
#define TOPIC_GEHEIMAUSGANG      0x60

#endif
