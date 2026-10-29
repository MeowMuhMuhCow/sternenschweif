/* s_struct.h - gemeinsame Datenstrukturen von DSA 2 Sternenschweif (Teil von schweif.h)
 *
 * Verbindliche Fassung, zusammengefuehrt aus den lokalen Kopien der csrc-Module (Inventar und Konflikte:
 * names/daten/*_agg.txt, names/konflikte.tsv). Offsets in den Kommentaren, Groessen ohne Ausrichtung (-a-).
 * Feldnamen englisch nach Bright Eyes (BE). Die Heldenstruktur entspricht der von DSA 1 (BE "HERO_*",
 * SIZEOF_HERO 0x6da) bis +0x1a7 mit Versatz +0x12 (DSA 2 hat 0x12 Bytes davor); danach eigene DSA-2-Felder.
 * Unbekannte Felder heissen f<offset> (z. B. f4f0) und behalten ihren Typ; Fuellbytes heissen _rNNN.
 *
 * Bitfelder sind wie im Original "unsigned short b:1" (8 Bits belegen 1 Byte, BCC packt bytebreit).
 * Einige Module erzeugen ihren Code mit "unsigned char"-Bitfeldern (C++) - siehe konflikte.tsv.
 *
 * Inline-Elementfunktionen (nur C++) erklaeren die Parameterkopien/-Og-Plaetze im Original: jede Expansion
 * legt eigene Kopien an. Sie kosten nichts, solange sie nicht benutzt werden - neue nach dem Muster unten
 * ergaenzen (Namenskonvention: KONVENTIONEN.md).
 */
#ifndef S_STRUCT_H
#define S_STRUCT_H

/* ======================================================================================================
 * Held (0x5ef Bytes): g_heroes[7] (ds:b760), Heldendateien *.CHR
 * ====================================================================================================*/

struct hero_attrib {			/* 3 Bytes, hero +0x46 + 3*i (BE HERO_ATTRIB_ORIG/ATTRIB/ATTRIB_MOD) */
	signed char normal;		/* +0 Grundwert */
	signed char cur;		/* +1 aktueller Wert */
	signed char mod;		/* +2 Modifikator; wirksamer Wert = cur + mod */
};

struct hero_ill {			/* 5 Bytes: Krankheit (+0xc0) bzw. Gift (+0xe8) */
	signed char state;		/* +0 ILL_STATE_SICK (-1) krank/vergiftet, ILL_STATE_RECOVER (1) genesend */
	signed char days;		/* +1 Dauer (Tage bzw. Ticks), wird je Durchlauf erhoeht */
	signed char loss_kk;		/* +2 abgezogene KK (Bannstaub: abgezogene AE) */
	signed char loss_ge;		/* +3 abgezogene GE */
	unsigned short effect:1;	/* +4 Bit 0: Wirkung ist eingetreten (Werte abgezogen) */
	unsigned short diagnosed:1;	/*    Bit 1 erkannt: erste Heilkunde-Probe gelungen (O090 setzt, O074 loescht) */
	unsigned short ill_b2:1;
	unsigned short ill_b3:1;
	unsigned short ill_b4:1;
	unsigned short ill_b5:1;
	unsigned short ill_b6:1;
	unsigned short ill_b7:1;
};

struct inv_slot {			/* 0x1b Bytes, hero +0x1a8 + 0x1b*i (BE: struct inventory, dort 14 Bytes) */
	signed short item_id;		/* +0x00 Gegenstand (Index in g_items, ds:b84a); 0 = leer */
	signed short quantity;		/* +0x02 Anzahl bzw. Ladungen */
	unsigned short broken:1;	/* +0x04 Bit 0 (BE inventory_flags) */
	unsigned short half_empty:1;	/*       Bit 1 Wasserschlauch halb leer */
	unsigned short empty:1;		/*       Bit 2 Wasserschlauch leer */
	unsigned short magic:1;		/*       Bit 3 magisch (O085 "magic") */
	unsigned short inv_b4:1;	/*       Bit 4 */
	unsigned short poison_expurgicum:1; /*   Bit 5 */
	unsigned short poison_vomicum:1; /*      Bit 6 */
	unsigned short magic_revealed:1; /*      Bit 7 Magie bekannt */
	signed char f05;		/* +0x05 */
	signed char bf;			/* +0x06 Bruchfaktor (O085 "bf"; O048/O062 nennen es "charges" - offen) */
	signed char rs_lost;		/* +0x07 verlorener RS (BE) */
	signed char lighting_timer;	/* +0x08 Brenndauer Fackel/Lampe (BE) */
	signed char poison_type;	/* +0x09 Gift auf der Waffe (O079 "poison", O085 "ptype") */
	signed char poison_charges;	/* +0x0a verbleibende Giftladungen (O079 "charges", O085 "pcnt") */
	unsigned long key_bits;	/* +0x0b Schluesselbund 0x145: belegte Faecher (Bit i = g_keyring_keys[i]; O092 box_has/box_set) */
	signed char _r0f;
	signed char f10;		/* +0x10 (O084) */
	signed char _r11[0x1b - 0x11];
#ifdef __cplusplus
	signed short is_breakable() { return (unsigned char)bf == 0x9d ? 0 : 1; }	/* O236 (bf unsigned verglichen) */
	signed short get_quantity() { return quantity; }	/* O263 (Argument fuer unsigned-char-Parameter: mov ax / push ax) */
#endif
};

struct hero_effect {			/* 7 Bytes, hero +0x55b, 20 Stueck (R02 add_effect/remove_effect) */
	signed long time;		/* +0 Restzeit (Ticks) */
	signed char type;		/* +4 Art */
	signed char index;		/* +5 Index (z. B. Eigenschaft) */
	signed char value;		/* +6 Betrag */
};

struct hero {				/* 0x5ef Bytes */
	char version[0x0c];		/* +0x000 Kennung "DSA VERSION" (O171 kopiert g_dsa_version_string hinein) */
	unsigned char ver[4];		/* +0x00c Versionsbytes (O171/O177: g_version_a/c/b/d); Spielstand-Satz ab hier (O077) */
	signed short portrait;		/* +0x010 Bildnummer des Portraets in der Kopfbilddatei (O076, O093) */
	char file_name[0x10];		/* +0x012 Name der Heldendatei (BE HERO_NAME; O071/O088 vergleichen ihn) */
	char name[0x10];		/* +0x022 angezeigter Name (BE HERO_NAME2) */
	signed char nr_items;		/* +0x032 Zahl belegter Inventarplaetze (BE NR_INVENTORY_SLOTS_FILLED) */
	signed char type;		/* +0x033 Heldentyp HERO_TYPE_* (>= 7 zauberkundig) */
	signed char sex;		/* +0x034 Geschlecht (0 = maennlich) */
	unsigned char height;		/* +0x035 Groesse (cm) */
	signed short weight;		/* +0x036 Gewicht (Unzen) */
	signed char god;		/* +0x038 Gottheit */
	signed char level;		/* +0x039 Stufe */
	signed long ap;			/* +0x03a Abenteuerpunkte */
	signed long money;		/* +0x03e Geld (Heller); Gruppengeld wird auf die Helden verteilt (O200) */
	signed char rs_bonus;		/* +0x042 RS-Bonus (BE RS_BONUS1) */
	signed char rs_bonus2;		/* +0x043 (BE RS_BONUS2) */
	signed char rs_be;		/* +0x044 Ruestungsbehinderung (BE RS_BE) */
	signed char bp_left;		/* +0x045 Bewegungspunkte im Kampf (O150 "bp", O154 add45) */
	struct hero_attrib attrib[14];	/* +0x046 Eigenschaften ATTRIB_* */
	signed short le_max;		/* +0x070 LE-Grundwert (BE LE_ORIG) */
	signed short le;		/* +0x072 Lebensenergie */
	signed short ae_max;		/* +0x074 AE-Grundwert (BE AE_ORIG) */
	signed short ae;		/* +0x076 Astralenergie */
	signed char mr;			/* +0x078 Magieresistenz (ohne mr_mod, O076_0832) */
	signed char atpa_base;		/* +0x079 AT/PA-Basis = (IN + KK + GE) / 5 (O076_0940) */
	signed char at[7];		/* +0x07a AT je Waffentalent 0..6 */
	signed char pa[7];		/* +0x081 PA je Waffentalent 0..6 */
	signed char at_mod;		/* +0x088 (BE AT_MOD) */
	signed char pa_mod;		/* +0x089 (BE PA_MOD) */
	signed char weapon_type;	/* +0x08a Art der gefuehrten Waffe (BE WEAPON_TYPE) */
	signed char attack_type;	/* +0x08b Kampfweise (BE ATTACK_TYPE; O154 set8b) */
	signed char le_mod;		/* +0x08c dauerhaft verlorene LE (O076_0549) (BE LE_MOD) */
	signed char timer_id;		/* +0x08d (BE TIMER_ID) */
	signed char start_gear;		/* +0x08e (BE START_GEAR) */
	signed char herbs;		/* +0x08f (BE HERBS) */
	signed char hunger_timer;	/* +0x090 (BE HUNGER_TIMER) */
	signed char hunger;		/* +0x091 Hunger in % (O005, O076_0E64) */
	signed char thirst;		/* +0x092 Durst in % (O005, O076_0EA7) */
	signed char fighter_id;		/* +0x093 Kampffigur (BE FIGHTER_ID) */
	signed char viewdir;		/* +0x094 Blickrichtung im Kampf (BE VIEWDIR) */
	signed char actions;		/* +0x095 Aktionen (BE ACTIONS) */
	signed char action_id;		/* +0x096 Kampfaktion (O154 "Aktion"; O076: 100 tot, 12 bewusstlos) */
	signed char spell_id;		/* +0x097 letzter Zauber im Kampf (BE SPELL_ID) */
	signed char enemy_id;		/* +0x098 Ziel im Kampf (O079 "enemy_id", O154 "Ziel") */
	signed char group_no;		/* +0x099 Gruppe 0..5 (O074 "grp"; Vergleich mit g_group_no) */
	signed char money_share;	/* +0x09a NSC: Anteil am Gruppengeld in % (O200_0084); BE: TEMPLE_ID */
	signed char npc_id;		/* +0x09b NSC-Nummer, 0 = kein NSC (O196 "npc", O200) */
	signed char group_pos;		/* +0x09c (BE GROUP_POS) */
	signed long heal_timer;		/* +0x09d (BE HEAL_TIMER) */
	signed long staffspell_timer;	/* +0x0a1 (BE STAFFSPELL_TIMER) */
	signed char recipe_id;		/* +0x0a5 Alchimie-Rezept (BE RECIPE_ID) */
	signed char recipe_timer;	/* +0x0a6 (BE RECIPE_TIMER) */
	signed char ruhe_koerper;	/* +0x0a7 (BE RUHE_KOERPER) */
	signed char blind;		/* +0x0a8 Kampfrunden geblendet (BE BLIND) */
	signed char ecliptifactus;	/* +0x0a9 (BE ECLIPTIFACTUS) */
	signed char saftkraft;		/* +0x0aa (BE SAFTKRAFT) */
	signed char fireban;		/* +0x0ab (BE FIREBAN) */
	signed char invisible;		/* +0x0ac (BE INVISIBLE) */
	signed char sprite_no;		/* +0x0ad Kampfgrafik (BE SPRITE_NO) */
	signed char alchemy_inn_id;	/* +0x0ae (BE ALCHEMY_INN_ID) */
	signed short escape_position;	/* +0x0af (BE ESCAPE_POSITION) */
	signed char jail;		/* +0x0b1 (BE JAIL) */
	signed char axxeleratus;	/* +0x0b2 (BE AXXELERATUS) */
	signed char drunk;		/* +0x0b3 betrunken (BE DRUNK; R10: O074_0872) */
	signed char parried;		/* +0x0b4 in dieser Runde schon pariert (O135: Paradebild nur beim ersten Mal; O080: 0
					   je Runde; O158/O159/O161: Angreifer AT +2, keine Parade mehr) (BE HERO_IS_TARGET) */
	signed char fb5;		/* +0x0b5 (Kampf); 99 = Zauber aus einem Gegenstand (O047; O235/O236: ohne Eingabe, Hoechstwirkung) */
	unsigned short fb6_b0:1;	/* +0x0b6 Bit 0 (O080/O157; O154 setb6) */
	unsigned short fb6_b1:1;
	unsigned short fb6_b2:1;
	unsigned short fb6_b3:1;
	unsigned short fb6_b4:1;
	unsigned short fb6_b5:1;
	unsigned short fb6_b6:1;
	unsigned short fb6_b7:1;
	signed char fb7;		/* +0x0b7 */
	signed char rest_bonus;	/* +0x0b8 Erholungsbonus der naechsten Regeneration (O072 camp_regenerate setzt, O074 hero_sleep
				   addiert ihn zu LE/AE und loescht ihn) (frueher fb8) */
	signed char watches;		/* +0x0b9 Wachen in dieser Rast (O071/O072; ab 2 keine Erholung group_sleep, je Wache -2) (frueher fb9) */
	unsigned short rest_magic:1;	/* +0x0ba Bit 0: in der Rast gezaubert (O072 camp_use_magic; Erholung -1) (frueher fba_b0) */
	unsigned short rest_food:1;	/*        Bit 1: in der Rast Nahrung/Wasser gesucht (O071 camp_find_food) (frueher fba_b1) */
	unsigned short rest_herbs:1;	/*        Bit 2: in der Rast Kraeuter gesucht (O071 camp_gather_herbs; Erholung -2) (frueher fba_b2) */
	unsigned short fba_b3:1;
	unsigned short fba_b4:1;
	unsigned short fba_b5:1;
	unsigned short fba_b6:1;
	unsigned short fba_b7:1;
	signed char mr_mod;		/* +0x0bb MR-Modifikator; wirksame MR = mr + mr_mod (O076_07FB) */
	/* +0x0bc Zustandsbits (BE hero_flags +0xaa) */
	unsigned short dead:1;		/* +0xbc.0 tot */
	unsigned short asleep:1;	/* +0xbc.1 schlaeft (O076_01F3 weckt, O080_0679) */
	unsigned short petrified:1;	/* +0xbc.2 versteinert */
	unsigned short brewing:1;	/* +0xbc.3 braut (BE) */
	unsigned short chamaelioni:1;	/* +0xbc.4 (BE) */
	unsigned short renegade:1;	/* +0xbc.5 (BE) */
	unsigned short unconscious:1;	/* +0xbc.6 bewusstlos (O080_0679, O089/O081 Merker) */
	unsigned short tied:1;		/* +0xbc.7 gefesselt (BE) */
	/* +0x0bd */
	unsigned short scared:1;	/* +0xbd.0 veraengstigt (O089 Angstgift, nur im Kampf) */
	unsigned short ueber_eis:1;	/* +0xbd.1 Ueber Eis wirkt (EFFECT_UEBER_EIS; O235 spell_ueber_eis) (frueher bd1) */
	unsigned short bd2:1;		/* +0xbd.2 (BE: duplicatus) */
	unsigned short tame:1;		/* +0xbd.3 gezaehmt/beherrscht (O194 Bannbaladin der Gegner) (BE tame; frueher bd3) */
	unsigned short harmless:1;	/* +0xbd.4 Harmlose Gestalt (EFFECT_HARMLOS; Portraet 0x3a4, O076) (frueher bd4) */
	unsigned short gods_pissed:1;	/* +0xbd.5 Goetter erzuernt: keine Tempelwunder fuer den Helden (O033; BE gods_pissed; frueher bd5) */
	unsigned short transformed:1;	/* +0xbd.6 verwandelt (O233 Verwandlung beenden: Eigenschaften +1) (BE transformed; frueher bd6) */
	unsigned short saftkraft_on:1;	/* +0xbd.7 Saft, Kraft, Monstermacht wirkt (EFFECT_SAFTKRAFT_ON; O236) (frueher bd7) */
	/* +0x0be weitere Bits (DSA 2) */
	unsigned short scharfes_auge:1;	/* +0xbe.0 Scharfes Auge wirkt (EFFECT_SCHARFES_AUGE; O236) (frueher be0) */
	unsigned short see_und_fluss:1;	/* +0xbe.1 In See und Fluss wirkt (EFFECT_SEE_UND_FLUSS; O237 fuer die Gruppe) (frueher be1) */
	unsigned short sooty:1;	/* +0xbe.2 verrusst, CH - 2 (vermutet; Wasserstellen O002/O065/O066 waschen ab, CH + 2) */
	unsigned short adler_wolf:1;	/* +0xbe.3 Adler, Wolf: Tiergestalt (EFFECT_ADLER_WOLF; Portraet 0x3a4 Bild n-1, O076) (frueher be3) */
	unsigned short flim_flam:1;	/* +0xbe.4 Flim Flam: Licht (EFFECT_FLIM_FLAM; O237) (frueher be4) */
	unsigned short dunkelheit:1;	/* +0xbe.5 Dunkelheit (EFFECT_DUNKELHEIT; O237) (frueher be5) */
	unsigned short be6:1;
	unsigned short be7:1;
	signed char fbf;		/* +0x0bf */
	struct hero_ill illness[8];	/* +0x0c0 Krankheiten ILLNESS_* (Eintrag 0 unbenutzt) */
	struct hero_ill poison[10];	/* +0x0e8 Gifte POISON_* (Eintrag 0 unbenutzt) */
	signed char skills[NR_SKILLS];	/* +0x11a Talentwerte TA_* */
	unsigned char skill_incs;	/* +0x14e gespeicherte Talentsteigerungen (BE TA_RISE) */
	signed char spells[NR_SPELLS];	/* +0x14f Zauberwerte SP_* */
	unsigned char spell_incs;	/* +0x1a5 gespeicherte Zaubersteigerungen (BE SP_RISE) */
	signed char magic_school;	/* +0x1a6 Magierakademie (BE MAGIC_SCHOOL) */
	signed char staffspell_lvl;	/* +0x1a7 Stabzauberstufe (BE STAFFSPELL_LVL) */
	struct inv_slot inv[NR_INV_SLOTS]; /* +0x1a8 Inventar (0..14 am Koerper, 15..30 Rucksack) */
	signed short load;		/* +0x4ed Traglast (BE LOAD) */
	signed char f4ef;		/* +0x4ef */
	signed char f4f0;		/* +0x4f0 (O076_0E64/0EA7 setzen 0) */
	signed short target_x;		/* +0x4f1 Zielfeld im Kampf (O137 Zauberwirkung/Beschwoerung, O153..O156) */
	signed short target_y;		/* +0x4f3 */
	signed short gardianum;		/* +0x4f5 Schutzpunkte des Gardianum (O233: AE + Stufe; O080: 0 nach dem Kampf) (frueher f4f5) */
	signed char duplicatus;		/* +0x4f7 Spiegelbilder des Duplicatus 0..2 (O137 setzt 1/2, O112 zeichnet die
					   Figur 2- bzw. 3-mal, O161: AT des Gegners / 2 bzw. / 3, O076: 0 beim Tod) */
	signed char fur_hands;		/* +0x4f8 Pelz an den Haenden (Dungeon 13, O065): 1..5 brennender Pelz breitet sich aus
					   (O005 hero_fur_tick, BINGE.LTX[22..26]), 6 kuehler Pelz, -1 geheilt (FF kehrt zurueck), 0 keiner
					   (frueher f4f8, in O005 als "Sucht" gedeutet) */
	signed char slot_pos;		/* +0x4f9 Platz in der Portraetleiste (R02_051F, O201) */
	signed char attack_style;	/* +0x4fa Kampfstil aus dem Kampfmenue (O154): 0 normal, 1 -> attack_type 2, sonst -2 */
	signed char _r4fb[0x553 - 0x4fb];
	signed short saved_le_max;	/* +0x553 gemerkte Werte (R02), O076_0EEA fuellt die 8 Bytes */
	signed short saved_le;		/* +0x555 */
	signed short saved_ae_max;	/* +0x557 */
	signed short saved_ae;		/* +0x559 */
	struct hero_effect effects[20];	/* +0x55b zeitlich begrenzte Wirkungen (R02_087D add_effect) */
	void far *f5e7;			/* +0x5e7 (O091, O206) */
	char far * far *txt;		/* +0x5eb Textliste (O006, O090, O206, O207) */

#ifdef __cplusplus
	/* --- Praedikate ---------------------------------------------------------------------------- */
	signed short is_mage() { return type >= 7; }	void set_rest_bonus(signed char v) { rest_bonus = v; }	/* O072 camp_regenerate (Parameterkopie) */
	void set_watches(signed char v) { watches = v; }	/* O071/O072 */
	void set_rest_magic(signed char v) { rest_magic = v; }	/* O072 camp_use_magic */
	void set_rest_food(signed char v) { rest_food = v; }	/* O071 camp_rest_jobs */
	void set_rest_herbs(signed char v) { rest_herbs = v; }	/* O071 camp_rest_jobs */
	char far *get_name() { return name; }	/* O129 event_notice: this-Kopie [bp-4] */
	signed short is_healable() { return dead || petrified ? 0 : 1; }	/* O006 Heilen */
	signed char mr_eff() { return mr + mr_mod; }	/* O091 test_spell: wirksame MR */
	unsigned char is_dead() { return dead; }	/* O091: Rueckgabe unsigned char -> and ax,1 / or al,al */
	signed short get_le() { return le; }		/* O121 tevent_142/143: als int gelesen (mov ax, Store al) */
	signed char get_npc_id() { return npc_id; }	/* O261/O267: group_get_npc(g)->get_npc_id() (this-Kopie) */
	void set_staffspell_lvl(signed char v) { staffspell_lvl = v; }	/* O091 */
	void set_portrait(signed short v) { portrait = v; }	/* O093 select_portrait */

	/* --- Krankheiten/Gifte: Setter je Feld, add_* begrenzen bei 0 (Original: "else ;") ---------- */
	void set_ill_state(signed short i, signed char v) { illness[i].state = v; }
	void set_ill_days(signed short i, signed char v) { illness[i].days = v; }
	void set_ill_kk(signed short i, signed char v) { illness[i].loss_kk = v; }
	void set_ill_ge(signed short i, signed char v) { illness[i].loss_ge = v; }
	void set_ill_effect(signed short i) { illness[i].effect = 1; }
	void clr_ill_effect(signed short i) { illness[i].effect = 0; }
	void add_ill_days(signed short i, signed char v) { illness[i].days += v; if (illness[i].days < 0) illness[i].days = 0; else ; }
	void add_ill_kk(signed short i, signed char v) { illness[i].loss_kk += v; if (illness[i].loss_kk < 0) illness[i].loss_kk = 0; else ; }
	void add_ill_ge(signed short i, signed char v) { illness[i].loss_ge += v; if (illness[i].loss_ge < 0) illness[i].loss_ge = 0; else ; }
	void clr_ill_diagnosed(signed short i) { illness[i].diagnosed = 0; }	/* O074 */
	void set_ill_diagnosed(signed short i) { illness[i].diagnosed = 1; }	/* O090 */

	void set_poi_state(signed short i, signed char v) { poison[i].state = v; }
	void set_poi_days(signed short i, signed char v) { poison[i].days = v; }
	void set_poi_effect(signed short i, unsigned char v) { poison[i].effect = v; }
	void set_poi_kk(signed short i, signed char v) { poison[i].loss_kk = v; }	/* O074 */
	void set_poi_ge(signed short i, signed char v) { poison[i].loss_ge = v; }	/* O074 */
	void clr_poi_effect(signed short i) { poison[i].effect = 0; }	/* O074 */
	void clr_poi_diagnosed(signed short i) { poison[i].diagnosed = 0; }	/* O074 */
	void set_poi_diagnosed(signed short i) { poison[i].diagnosed = 1; }	/* O090 */
	void add_poi_days(signed short i, signed char v) { poison[i].days += v; if (poison[i].days < 0) poison[i].days = 0; else ; }
	void add_poi_kk(signed short i, signed char v) { poison[i].loss_kk += v; if (poison[i].loss_kk < 0) poison[i].loss_kk = 0; else ; }
	void add_poi_ge(signed short i, signed char v) { poison[i].loss_ge += v; if (poison[i].loss_ge < 0) poison[i].loss_ge = 0; else ; }

	/* --- Licht im Inventar (R10 sub_light_timers) ------------------------------------------------ */
	void set_inv_light_timer(signed short i, signed char v) { inv[i].lighting_timer = v; }
	void set_inv_item_id(signed short i, signed short v) { inv[i].item_id = v; }

	/* --- einfache Setter ------------------------------------------------------------------------- */
	void set_hunger(signed char v) { hunger = v; }
	void set_thirst(signed char v) { thirst = v; }
	void raise_ap(signed long v) { ap = ap + v; }	/* O074 add_hero_ap: ohne Klemmung, "ap = ap + v" */
	void set_money_share(signed char v) { money_share = v; }	/* O074 hero_dismiss (Tempel) */
	void add_hunger(signed char v) { hunger += v; if (hunger < 0) hunger = 0; else ; if (hunger > 100) hunger = 100; else ; }	/* O005 */
	void add_thirst(signed char v) { thirst += v; if (thirst < 0) thirst = 0; else ; if (thirst > 100) thirst = 100; else ; }	/* O005 */
	void set_jail(signed char v) { jail = v; }	/* O005 jail_tick */
	void set_f4f0(signed char v) { f4f0 = v; }	/* O005: letzte Hunger-/Durstmeldung */
	void set_fb5(signed char v) { fb5 = v; }	/* O080 group_set_fb5 */
	void set_fb6_b0(signed char v) { fb6_b0 = v; }	/* O080 group_set_fb6_b0 */
	void set_money(signed long v) { money = v; }			/* O200, O202 */
	void set_group_no(signed short v) { group_no = v; }		/* O202 (Byte gespeichert) */
	void set_group_pos(signed short v) { group_pos = v; }		/* O075, O202 (Byte gespeichert) */
	void set_slot_pos(signed char v) { slot_pos = v; }		/* O201 */
	void set_blind(signed char v) { blind = v; }			/* O128 fight_hero_turn */
	void set_ecliptifactus(signed char v) { ecliptifactus = v; }	/* O128 fight_hero_turn */
	void set_fighter_id(signed char v) { fighter_id = v; }		/* O022 fig_init_heroes */
	void set_viewdir(signed char v) { viewdir = v; }		/* O022 fig_init_heroes */
	/* Welle N11 (Kampfmenue/KI/Aktionen O154..O161) */
	void set_bp_left(signed char v) { bp_left = v; if (bp_left > 12) bp_left = 12; else ; }	/* O155 (hoechstens 12) */
	void add_bp_left(signed char v) { bp_left += v; if (bp_left < 0) bp_left = 0; else ; }	/* O154/O155 (nicht unter 0) */
	void set_action_id(signed char v) { action_id = v; }		/* O154 */
	void set_spell_id(signed char v) { spell_id = v; }		/* O155/O157 */
	void set_enemy_id(signed char v) { enemy_id = v; }		/* O154..O157 */
	void set_attack_type(signed char v) { attack_type = v; }	/* O154 */
	void set_attack_style(signed char v) { attack_style = v; }	/* O154 */
	void set_target_x(signed short v) { target_x = v; }		/* O154..O156 */
	void set_target_y(signed short v) { target_y = v; }
	unsigned char is_scared() { return scared; }			/* O155 (Rueckgabe unsigned char) */
	signed short is_weapon_breakable() { return inv[INV_SLOT_RIGHT_HAND].bf == -99 ? 0 : 1; }	/* O158/O161 */

	/* --- Inventarplaetze (O083, O202) ------------------------------------------------------------ */
	void clr_inv_half_empty(signed short i) { inv[i].half_empty = 0; }
	void clr_inv_empty(signed short i) { inv[i].empty = 0; }
	void set_inv_empty(signed short i) { inv[i].empty = 1; }			/* O253, O177 */
	void set_inv_half_empty(signed short i) { inv[i].half_empty = 1; }		/* O253, O177 */
	void set_inv_poison_type(signed short i, signed short v) { inv[i].poison_type = v; }
	void set_inv_poison_charges(signed short i, signed char v) { inv[i].poison_charges = v; }
	signed short get_inv_rs_lost(signed short i) { return inv[i].rs_lost; }
	signed short is_inv_breakable(signed short i) { return inv[i].bf == -99 ? 0 : 1; }	/* bf -99 = unzerbrechlich */
	void dec_inv_bf(signed short i) { if ((inv[i].bf == -99 ? 0 : 1) && inv[i].bf > 0) inv[i].bf--; else ; }
	void inc_inv_bf(signed short i) { if ((inv[i].bf == -99 ? 0 : 1) && inv[i].bf < 20) inv[i].bf++; }	/* O062 Kobold: Waffen weich (bis 20) */
	void add_load(signed short v) { load += v; }					/* O085 */
	void add_inv_quantity(signed short i, signed char v) { inv[i].quantity += v; }	/* O085 */
	void set_inv_quantity(signed short i, signed char v) { inv[i].quantity = v; }	/* O085 */
	struct inv_slot get_inv(signed short i) { return inv[i]; }			/* O085 (SCOPY@ in Temporaere) */
	void set_inv(signed short i, struct inv_slot s) { inv[i] = s; }			/* O085 (rep movsw) */
	void dec_inv_quantity(signed short i) { inv[i].quantity--; }			/* O047 */
	void clr_inv_b4(signed short i) { inv[i].inv_b4 = 0; }				/* O047 */
	void set_weapon_poison_charges(signed char v) { inv[INV_SLOT_RIGHT_HAND].poison_charges = v; } /* O047 */
	void clr_inv_poison_type(signed short i) { inv[i].poison_type = 0; }		/* O079 (Gift verbraucht) */

	/* --- Welle N13 (Magie) ------------------------------------------------------------------------ */
	void inc_spell(signed short i) { spells[i]++; }		/* O087 (Parameterkopie vor inc byte) */
	void set_spell(signed short i, signed char v) { spells[i] = v; }	/* O234 Heptagon-Patzer */
	void set_inv_magic_known(signed short i) { inv[i].magic_revealed = 1; }	/* O235 Analues/Odem Arcanum */
	signed short get_weapon_type() { return weapon_type; }	/* O236 (cbw vor einem signed-char-Parameter) */
	signed short get_drunk() { return drunk; }		/* O179 inn_night_theft: mov al / cbw / or ax,ax */

	/* --- Welle N18 (Heldenerschaffung O026/O167..O171, Stufenanstieg O086, DSA-1-Import O176/O177) ------- */
	void set_sex(signed char v) { sex = v; }				/* O167/O169 */
	void set_type(signed char v) { type = v; }				/* O169/O170 */
	void set_attrib_normal(signed short i, signed char v) { attrib[i].normal = v; }	/* O167/O169 (Wuerfelwert) */
	void set_atpa_base(signed char v) { atpa_base = v; }			/* O167 */
	void set_at(signed short i, signed char v) { at[i] = v; }		/* O167 */
	void set_pa(signed short i, signed char v) { pa[i] = v; }
	void add_at(signed short i, signed char v) { at[i] += v; }
	void add_pa(signed short i, signed char v) { pa[i] += v; }
	void inc_at(signed short i) { at[i]++; }				/* O167/O168 */
	void dec_at(signed short i) { at[i]--; }				/* O168 */
	void inc_pa(signed short i) { pa[i]++; }				/* O168 */
	void dec_pa(signed short i) { pa[i]--; }				/* O167/O168 */
	void set_skill_incs(unsigned char v) { skill_incs = v; }		/* O167 */
	void add_skill_incs(signed char v) { skill_incs += v; }			/* O167/O086 (add byte) */
	void set_spell_incs(unsigned char v) { spell_incs = v; }		/* O167 */
	void add_spell_incs(signed char v) { spell_incs += v; }			/* O167/O170/O086 */
	void add_spell(signed short i, signed char v) { spells[i] += v; }	/* O167 Akademie-Zauber */
	void set_magic_school(signed char v) { magic_school = v; }		/* O167 */
	void set_le_max(signed short v) { le_max = v; }				/* O167 */
	void set_le(signed short v) { le = v; }
	void set_ae_max(signed short v) { ae_max = v; }
	void set_ae(signed short v) { ae = v; }
	void set_height(unsigned char v) { height = v; }			/* O167 */
	void set_weight(signed short v) { weight = v; }
	void set_god(signed char v) { god = v; }				/* O167 Sternzeichen */
	void set_ver0(unsigned char v) { ver[0] = v; }				/* O171/O177 Versionsbytes */
	void set_ver1(unsigned char v) { ver[1] = v; }
	void set_ver2(unsigned char v) { ver[2] = v; }				/* O171 */
	void set_ver3(unsigned char v) { ver[3] = v; }
	void set_inv_bf(signed short i, signed short v) { inv[i].bf = v; }		/* O177 (Wort-Parameter) */
	void set_inv_rs_lost(signed short i, signed short v) { inv[i].rs_lost = v; }	/* O177 (Wort-Parameter) */
	void set_inv_magic(signed short i) { inv[i].magic = 1; }			/* O176/O177 */
	void set_inv_b4(signed short i) { inv[i].inv_b4 = 1; }				/* O177 */
#ifdef __STRING_H
	void set_name(char far *s) { strcpy(name, s); }				/* O177 (Kopie des Zeigers) */
	void set_file_name(char far *s) { strcpy(file_name, s); }		/* O177 */
	void set_from_nr_items(void far *s, signed short n) { memcpy(&nr_items, s, n); }	/* O177: Rest des DSA-1-Satzes */
#endif
#endif
};

struct recipe {				/* 0x1c Bytes, g_recipes[14] (ds:196a): Alchimie-Rezepte (BE ALCHEMY_RECIPES) */
	signed short item_id;		/* +0x00 Rezept (Gegenstand) */
	signed short ingredients[10];	/* +0x02 Zutaten, -1 = Ende */
	signed short outcome;		/* +0x16 Ergebnis (Gegenstand; 0x153 Sonderfall, O074) */
	signed short ae;		/* +0x18 AE-Kosten */
	signed char difficulty;		/* +0x1a Zuschlag der Alchimie-Probe */
	signed char duration;		/* +0x1b Dauer (O090) */
};

/* Teilsicht auf das Dialogobjekt g_dialog (Klasse aus O190, bis sie in s_class.h steht):
 * ((struct dlg_view far *)g_dialog)->sel ergibt dieselben Bytes wie ein Zugriff auf das Objekt selbst (O088) */
struct dlg_view {
	signed char _r00[0x10];
	signed short sel;		/* +0x10 markierte (vorgewaehlte) Zeile der Auswahl (O190 f_074a) */
#ifdef __cplusplus
	void set_sel(signed short v) { sel = v; }	/* O088 select_hero_from_list */
#endif
};

struct group_ptrs6 {			/* 24 Bytes: sechs Gruppenzeiger als Strukturkopie (O005 jail_tick, g_group_ptrs_null) */
	struct group far *p[6];
};

struct srgb {				/* 3 Bytes: Palettenfarbe mit Vorzeichen (O034 Ueberblendungen rechnen signed) */
	signed char r, g, b;
};

/* --- FLI-Animationen (O024 Abspielschleife, O025 Rahmen/Chunks, O270 Assembler-Entpacker) ------------- */
struct fli_header {			/* 0x80 Bytes: Kopf einer FLI-Datei (Autodesk Animator) */
	unsigned long size;		/* +0x00 Dateigroesse */
	unsigned short magic;		/* +0x04 FLI_MAGIC */
	unsigned short frames;		/* +0x06 Zahl der Bilder */
	unsigned short width;		/* +0x08 */
	unsigned short height;		/* +0x0a */
	unsigned short depth;		/* +0x0c */
	unsigned short flags;		/* +0x0e */
	signed short speed;		/* +0x10 Zeitgebertakte (70 Hz) je Bild */
	char _r12[0x80 - 0x12];
};

struct fli_screen {			/* 0x12 Bytes: Ausgabeziel der FLI-Entpacker (g_fli_screen, ds:1522) */
	signed long f00;
	signed short width;		/* +0x04 320 */
	signed short height;		/* +0x06 200 (Zeilenzahl fuer FLI_BRUN) */
	signed short pitch;		/* +0x08 320 (vermutet) */
	unsigned char far *scr;		/* +0x0a Bildpuffer (= g_fli_vga, eigenes Public: A000:0000) */
	unsigned char far *pal;		/* +0x0e Palettenpuffer (= g_fli_pal, eigenes Public) */
};

struct fli_frame {			/* 0x10 Bytes: Rahmenkopf */
	signed long size;		/* +0x00 Groesse samt Kopf (signed: sub dword/jnz in O025) */
	unsigned short magic;		/* +0x04 FLI_FRAME_MAGIC */
	signed short chunks;		/* +0x06 Zahl der Chunks */
	char _r08[8];
};

struct fli_chunk {			/* Chunk-Kopf (6 Bytes), danach die Daten */
	unsigned long size;		/* +0x00 Groesse samt Kopf */
	unsigned short type;		/* +0x04 FLI_COLOR .. FLI_COPY */
};

/* Abbruch-Rueckruf der Abspielschleife fli_play (Bild n von frames, Durchlauf loops; != 0 = Ende). Aufrufer ausserhalb
 * von O024 (O002, O013) deklarieren ihn ohne loops (Prototyp von fli_play), O024 ruft ihn mit drei Parametern. */
typedef signed short (far *fli_cb)(unsigned short n, unsigned short frames);
typedef signed short (far *fli_cb3)(unsigned short n, unsigned short frames, unsigned short loops);
struct chr_entry {			/* 0x24 Bytes: gespeicherter Held in der Auswahlliste (O034 chr_list_*) */
	char name[0x10];		/* +0x00 angezeigter Name */
	char file_name[0x10];		/* +0x10 Name der Heldendatei */
	signed short arc_id;		/* +0x20 Archiv (0xfb temporaer, 0xfa Spielstand) */
	signed short slot;		/* +0x22 Satznummer im Archiv */
};

struct chance_rec {			/* 6 Bytes: Satz aus Datei 3 (g_file_3, O005 timers_daily) */
	signed char _r0[4];
	signed char chance;		/* +4 Wahrscheinlichkeit in % */
	signed char _r5;
};

struct month_roll {			/* 11 Bytes, g_market_rolls[] (O005 timers_daily) */
	signed short base;		/* +0 Untergrenze fuer random_interval(base, 20), -1 = Ende */
	signed char _r2[4];
	signed short value;		/* +6 gewuerfelter Wert */
	signed char flag8;		/* +8 je 50 % */
	signed char flag9;		/* +9 */
	signed char flag10;		/* +0x0a */
};

struct mr_table {			/* 13 Bytes, g_mr_modificators (ds:5976): MR-Zuschlag je Heldentyp */
	signed char v[13];
};

struct weapon_class {			/* 8 Bytes, g_weapons_class (ds:2f82 = Byte +7 von g_weapons_table[0] (N2: struct weapon_desc
					 * ab ds:2f7b, dort _r7), eigenes Public), Index item_desc.table_index (O080) */
	signed char category;		/* +0 Kategorie: < 4 Nahkampf (hand_weapon_class_melee), >= 4 Fernkampf */
	signed char _r1[7];
};

struct point {				/* 4 Bytes */
	signed short x;
	signed short y;
};

struct dir_steps { struct point d[4]; };	/* 16 Bytes: Schrittweiten (x, y) je Richtung (O053 g_dir_steps, Strukturkopie) */
struct point10 { struct point p[10]; };	/* 40 Bytes: Ringpuffer von 10 Feldern (O053 g_ring_init, Strukturkopie) */

struct col_pos {			/* 6 Bytes: Spalte im Heldenbogen (O093/O094/O095) */
	signed short x;			/* +0 Name ab x */
	signed short x_val;		/* +2 Wert rechtsbuendig bis x_val */
	signed short y;			/* +4 erste Zeile */
};
struct col_pos4 { struct col_pos c[4]; };	/* als Strukturkopie (O094) */
struct col_pos7 { struct col_pos c[7]; };	/* (O095) */
struct col_pos8 { struct col_pos c[8]; };	/* (O094) */

struct range_idx {			/* 2 Bytes: g_skills_index (ds:1bc2), g_spells_index (ds:22be) - Gruppen {erstes, Anzahl} */
	signed char first;		/* +0 erstes Talent TA_* */
	signed char count;		/* +1 Anzahl */
};

struct skill_list9 {			/* 9 Bytes, g_select_skill_defaults (ds:010e), als Kopie erweitert (O006) */
	signed char skill[9];
};

struct str6 {				/* 6 Bytes Zeichenkette als Strukturkopie (O006 g_select_skill_lvlup) */
	char s[6];
};

struct bytes12 {			/* 12 Bytes als Strukturkopie (O091 select_spell) */
	signed char b[12];
};

struct select_spell_tmpl {		/* 18 Bytes, g_select_spell_tmpl (ds:5ca0): Vorlagen fuer select_spell (O091) */
	struct str6 fmt;		/* +0 Format "%s~%d" */
	struct bytes12 usable;		/* +6 zwoelf Einsen (benutzbare Zauberklassen) */
};

struct spell_desc {			/* 12 Bytes, g_spell_descriptions[86] (ds:1eaa) (BE SPELL_DESCRIPTIONS) */
	signed char herotype;		/* +0 (BE: 0 Druide, 1 Magier, 2 Hexe, 3..5 Elfen) */
	signed char a1;			/* +1 Probeneigenschaften ATTRIB_* */
	signed char a2;			/* +2 */
	signed char a3;			/* +3 */
	signed char ae_cost;		/* +4 AE-Kosten (O233) */
	signed char where;		/* +5 -1 nicht im Kampf, 0 immer, 1 nur im Kampf */
	signed char f6;			/* +6 */
	signed char target_type;	/* +7 Zielart (O091: 1 kein Ziel moeglich, 4 ohne Wahl) */
	signed char range;		/* +8 (BE RANGE) */
	signed char fight;		/* +9 gegen MR (Kampfzauber) */
	signed short snd;		/* +0x0a Klang (O233) */
};

struct staffspell_desc {		/* 6 Bytes, g_staffspell_descriptions (ds:1e80) (BE STAFFSPELL_DESCRIPTIONS) */
	signed char a1;			/* +0 Probeneigenschaften */
	signed char a2;
	signed char a3;
	signed char mod;		/* +3 Zuschlag */
	signed char ae_cost;		/* +4 AE-Kosten */
	signed char ae_mod;		/* +5 dauerhafter AE-Verlust */
};

struct skill_attribs {			/* 4 Bytes, g_skill_attribs[52] (ds:1af2): Probeneigenschaften je Talent */
	signed char a1;
	signed char a2;
	signed char a3;
	signed char max_incs;		/* +3 Steigerungen je Stufe (BE SKILL_DESCRIPTIONS +3; O087 ueber g_skill_max_incs) */
};

struct txt_idx9 {			/* 18 Bytes: neun Textnummern (g_nekropathia_txt, ds:75b4; O236 als Strukturkopie) */
	signed short txt[9];
};

struct analues_item {			/* 6 Bytes: Gegenstand mit Analues-Text (g_analues_items, ds:748a, Ende item_id -1; O235) */
	signed short item_id;		/* +0 Gegenstand */
	signed short min;		/* +2 noetiges Probenergebnis */
	signed short txt;		/* +4 Zaubertext */
};

struct spell_flags {			/* 7 Bytes ab ds:8c11 (g_spell_flags = Sicht auf g_spell_by_item ..): je Zauber geloescht */
	signed short by_item;		/* +0 = g_spell_by_item */
	signed short no_sound;		/* +2 = g_spell_no_sound */
	signed short event;		/* +4 = g_spell_event */
	signed char cost_mod;		/* +6 = g_spell_cost_mod */
};

struct heal_ani_frames {		/* 10 Bytes: Bildfolge der Heilanimation am Portraet (g_heal_ani_frames, O233 Cast::heal_ani) */
	signed short frame[5];		/* Bildnummern in NVF 0x49 */
};

struct lock_type {			/* 3 Bytes: Schlossart (g_lock_types, ds:6230; O108 Lock::init) */
	unsigned char force;		/* +0 Aufbrechen: noetige KK x 2 (init halbiert) */
	unsigned char pick_mod;		/* +1 Zuschlag Schloesserknacken */
	unsigned char spell_mod;	/* +2 Zuschlag Foramen/Claudibus */
};

struct skill_max_incs {			/* 4 Bytes: Teilsicht ab g_skill_attribs[0].max_incs (ds:1af5, eigenes Public) */
	signed char max_incs;		/* +0 Steigerungen je Stufe (O087 lvl_inc_skill) */
	signed char _r1[3];
};

struct inc_counter {			/* 2 Bytes: Steigerungszaehler je Talent/Zauber (g_inc_skills, g_inc_spells; BE
					   INC_SKILLS_COUNTER/INC_SPELLS_COUNTER) */
	signed char tries;		/* +0 Fehlversuche in Folge (3 = keine weiteren) */
	signed char incs;		/* +1 gelungene Steigerungen dieser Stufe */
};

struct school_spells {			/* 0x21 Bytes: Zauberliste einer Magierakademie (*g_school_spells, BE
					   MAGIC_SCHOOLS_INDEX): drei Steigerungen fuer diese Zauber (O087) */
	signed char f00;		/* +0x00 */
	signed short spells[16];	/* +0x01 Wortliste fuer is_in_word_array, -1 = Ende */
};

struct school_bonus {			/* 0x21 Bytes: dieselbe Datei 0x1f8 (struct school_spells) aus Sicht der Heldenerschaffung
					   (O167): Zahl der Akademiezauber, Zauber und ihr Startbonus */
	signed char count;		/* +0x00 Zahl der Eintraege (hoechstens 8) */
	signed short spells[8];		/* +0x01 Zauber SP_* */
	signed short bonus[8];		/* +0x11 Zuschlag auf den Startwert */
};

/* --- Tabellen der Heldenerschaffung (O026/O167..O171) und des Stufenanstiegs (O086) -------------------------- */
struct type_req {			/* 2 Bytes: Typvoraussetzung (Datei 0x1f5, 13 Typen x 6) */
	unsigned char attrib;		/* +0 Eigenschaft ATTRIB_* */
	unsigned char value;		/* +1 Mindestwert; mit TYPE_REQ_MAX (Bit 7) Hoechstwert value & 0x7f; 0 = kein Eintrag */
};

struct skill_tpl {			/* 4 Bytes: Vorlage der automatischen Talentsteigerung (Datei 0x9c, 13 Typen x 25) */
	signed short skill;		/* +0 Talent TA_* */
	signed short max;		/* +2 bis zu diesem Wert (O086) */
};

struct money_range {			/* 6 Bytes: Startgeld je Typ (g_type_money[typ] -> Liste bis lim >= W20) */
	signed short lim;		/* +0 Grenze fuer den W20 */
	signed short lo;		/* +2 Spanne in Silberstuecken (x 10 Heller) */
	signed short hi;		/* +4 */
};

struct height_range {			/* 2 Bytes: Groesse je Typ (g_type_height) */
	unsigned char lo;		/* +0 cm */
	unsigned char hi;		/* +1 */
};

struct attrib_prio {			/* 14 Bytes: Reihenfolge der Eigenschaften beim automatischen Stufenanstieg (O086) */
	unsigned char pos[7];		/* +0 positive Eigenschaften (zuerst versucht) */
	unsigned char neg[7];		/* +7 negative Eigenschaften */
};

struct incs12 { signed char v[12]; };	/* Steigerungen je Typ 1..12 (g_chargen_skill_incs, g_lvl_skill_incs; Strukturkopie) */
struct incs6 { signed char v[6]; };	/* je Zaubertyp 7..12 (g_chargen_spell_incs ...; Strukturkopie) */
struct type_list13 { signed char type[13]; };	/* Liste waehlbarer Typen (g_chargen_type_list_init; Strukturkopie, O170) */
struct str16 { char s[16]; };		/* Namenspuffer (g_chargen_name_init; Strukturkopie, O026) */
struct str20 { char s[20]; };		/* Pfadvorlage "..\\" (g_print_path_init; Strukturkopie, O207) */
struct xy14 { struct { signed short x, y; } p[14]; };	/* Lage der 14 Eigenschaften im Heldenbogen (g_chargen_attrib_pos) */
struct portrait_idx12 { signed char v[12]; };	/* erstes Portraet je Typ in der Pruefsummentabelle (O176 g_dsa1_pic_idx_m) */
struct portrait_idx11 { signed char v[11]; };	/* (O176 g_dsa1_pic_idx_f) */
struct item_list23 { signed short v[23]; };	/* DSA-1-Inventarplatz -> DSA-2-Platz (O177 g_dsa1_slot_map) */
struct item_list25 { signed short v[25]; };	/* Gegenstaende, die beim Import entfallen (O177 g_dsa1_drop_items, Ende -1) */

/* ======================================================================================================
 * Gegenstandsbeschreibung (0x0e Bytes): Tabelle *ds:b84a (g_items), Index = inv_slot.item_id
 * ====================================================================================================*/
struct grammar_row {			/* 6 Bytes: Zeile der Artikel-/Geschlechtstabellen (O056, BE struct dummy) */
	signed short v[3];
};

struct item_desc {
	signed short gfx;		/* +0x00 Bildnummer (O085/O099 "pic") */
	unsigned short armor:1;		/* +0x02 Bit 0 Ruestung (BE item_flags) */
	unsigned short weapon:1;	/*       Bit 1 Waffe */
	unsigned short useable:1;	/*       Bit 2 benutzbar */
	unsigned short food:1;		/*       Bit 3 Nahrung */
	unsigned short stackable:1;	/*       Bit 4 stapelbar */
	unsigned short herb_potion:1;	/*       Bit 5 Kraut/Trank/Gift */
	unsigned short undropable:1;	/*       Bit 6 nicht ablegbar */
	unsigned short item_b7:1;	/*       Bit 7 */
	unsigned short f03_b0:1;	/* +0x03 weitere Bits (O083 c0..c7) */
	unsigned short f03_b1:1;
	unsigned short f03_b2:1;
	unsigned short f03_b3:1;
	unsigned short f03_b4:1;
	unsigned short f03_b5:1;
	unsigned short f03_b6:1;
	unsigned short f03_b7:1;
	signed char subtype;		/* +0x04 Unterart (O008 "type") (BE SUBTYPE) */
	signed char table_index;	/* +0x05 Index in die Waffen-/Ruestungstabelle (g_armors_table ds:31eb) */
	signed short weight;		/* +0x06 Gewicht (Unzen) */
	signed short price;		/* +0x08 Preis */
	signed char price_unit;		/* +0x0a (Haendler O099/O100/O226) - Bedeutung pruefen */
	signed char f0b;		/* +0x0b (O068/O069/O085; O226 "lvl") */
	signed char magic;		/* +0x0c magisch (O085_1503 setzt inv_slot.magic; O163 sucht danach, O202_0BB2 behaelt sie) */
	signed char gender;		/* +0x0d grammatisches Geschlecht fuer Artikel (O056) */
#ifdef __cplusplus
	signed short is_armor() { return armor; }		/* O263 */
	signed short is_weapon() { return weapon; }		/* O263 */
	signed short is_food() { return food; }			/* O263 */
	signed short is_herb_potion() { return herb_potion; }	/* O263 */
	signed short is_magic() { return magic; }		/* O263 (mov al / cbw / or ax,ax) */
	signed short is_recipe() { return useable && table_index == 2; }	/* O252 (benutzbar ueber g_usable_table[2] = read_text) */
#endif
};

/* Tabellen zu item_desc.table_index (BE WEAPONS_TABLE/ARMORS_TABLE; DSA 2: 8 Bytes je Waffe) */
struct weapon_desc {			/* 8 Bytes, g_weapons_table (ds:2f7b) */
	signed char dmg_d6;		/* +0 Schaden: Zahl der W6 (O084_0000 calc_damage_range) */
	signed char dmg_add;		/* +1 Schaden: Zuschlag */
	signed char kk_bonus;		/* +2 (BE KK-Zuschlag) */
	signed char bf;			/* +3 Bruchfaktor (O084_140D, Startwert O085_1503 ueber g_weapons_bf) */
	signed char ranged;		/* +4 (BE RANGED_INDEX) */
	signed char at_mod;		/* +5 AT-Modifikator (O083_133A ueber g_weapons_atpa, O084_0000) */
	signed char pa_mod;		/* +6 PA-Modifikator */
	signed char _r7;
};

struct ranged_weapon {			/* 8 Bytes, g_ranged_weapons_table (ds:2f1b) je weapon_desc.ranged (BE RANGED_WEAPONS_TABLE) */
	signed char dmg_mod[7];		/* +0 Schadenszuschlag je Entfernungsstufe 0..6 (O079 hero_attack_damage) */
	signed char handicap;		/* +7 Grundzuschlag der Fernkampfprobe */
};

struct weapon_atpa {			/* 8 Bytes, g_weapons_atpa (ds:2f80 = &g_weapons_table[0].at_mod, eigenes Public) */
	signed char at_mod;
	signed char pa_mod;
	signed char _r2[6];
};

struct armor_desc {			/* 2 Bytes, g_armors_table (ds:31eb) */
	signed char rs;			/* +0 Ruestungsschutz */
	signed char be;			/* +1 Behinderung */
};

struct usable_desc {			/* 3 Bytes, g_usable_table (ds:3222) je item_desc.table_index benutzbarer Gegenstaende */
	signed char charges;		/* +0 Ladungen beim Erzeugen (O085_1503) */
	signed char func;		/* +1 Index in g_item_use_funcs (O047 ItemUse::use) */
	signed char _r2;
};

struct item_list7 {			/* 14 Bytes: Gegenstandsliste fuer is_in_word_array (O047: g_weapons_no_poison) */
	signed short item[7];
};

struct score7 {				/* 14 Bytes: Wertung je Held (O058, Vorlage g_score7_zero als Strukturkopie) */
	signed short v[7];
};

struct item_combo {			/* 6 Bytes: kombinierbares Gegenstandspaar (g_item_combos, ds:4dd8, Ende item1 -1; O048 combo_index) */
	signed short item1;
	signed short item2;		/* (Reihenfolge beliebig) */
	signed short func;		/* Index in g_item_use_funcs */
};

struct start_gear {			/* 10 Bytes, g_start_gear (ds:5a16): fuenf Grundgegenstaende (O084_15C0) */
	signed short item[5];
};

struct start_gear_types {		/* 0x60 Bytes, g_start_gear_types (ds:5a20): je Heldentyp 4 Gegenstaende, -1 Ende */
	struct { signed short item[4]; } type[12];
};

struct loot_item {			/* 2 Bytes: Fundstueck (O116 tevent_049: Datei 0xaa FEAT049.DAT, 6 Eintraege) */
	unsigned char id;		/* +0 Gegenstand */
	signed char n;			/* +1 Anzahl */
};

struct dng_target {			/* 6 Bytes: Zielfeld in einem Dungeon (O062 g_dng11_kobold_dest_*) */
	signed short x;			/* +0 */
	signed short y;			/* +2 */
	signed char dungeon;		/* +4 group.dungeon */
	signed char _r5;
};

struct item_entry {			/* 6 Bytes: Auswahlliste der Gegenstaende (O083 make_item_list/select_item) */
	signed short slot;		/* Inventarplatz */
	char far *name;			/* Name (39-Byte-Puffer) */
};

/* ======================================================================================================
 * Gruppe (0x49 Bytes): Klasse Group (O200/O201/O202, freie Gruppenschleifen in O203), g_groups[6] (ds:c30f),
 * g_group (ds:c4c5) = aktive. Die Methoden sind normale Elementfunktionen (this = erstes Argument, gleiche
 * Bytes wie die freien Funktionen group_* aus s_proto.h, die C-Module benutzen); Namen: Methode "add_money"
 * = freie Funktion "group_add_money" (namen.tsv). Bestaetigt mit O075/O200..O202 (Welle N2, LINKGLEICH).
 * ====================================================================================================*/
struct group {
	struct hero far *heroes[7];	/* +0x00 Heldenzeiger; [6] = NSC */
	signed char dir;		/* +0x1c Blickrichtung 0..3 (O014/O046: g_cam_angle = (dir << 8) + 0x100) */
	signed char f1d;		/* +0x1d */
	signed char loctype;		/* +0x1e Ortsart LOC_* (0 = Strasse/Gang; O005/O006/O090/O183, frueher "dir") */
	signed char old_loctype;	/* +0x1f vorige Ortsart (set_loctype: nur, wenn loctype Bit 7 frei) */
	signed char town;		/* +0x20 Stadt 1..6 (3D-Ansicht O046, AREA_TOWN), 0 = keine */
	signed char old_town;		/* +0x21 vorige Stadt (set_town) */
	signed char dungeon;		/* +0x22 Dungeon 1..16 (3D-Ansicht O014, AREA_DUNGEON), 0 = keiner */
	signed char old_dungeon;	/* +0x23 voriger Dungeon (set_dungeon) */
	signed char f24;		/* +0x24 (O201_064D: Teil des Standorts) */
	signed char _r25;
	signed short x;			/* +0x26 Position x */
	signed short old_x;		/* +0x28 vorige Position x */
	signed short y;			/* +0x2a Position y */
	signed short old_y;		/* +0x2c vorige Position y */
	signed char nr_heroes;		/* +0x2e Zahl der Helden (O200 Konstruktor "Zaehler") */
	signed long money;		/* +0x2f Gruppengeld (O200/O201 rechnen unsigned: Casts im Modul) */
	signed long route_pos;		/* +0x33 unterwegs: zurueckgelegte Strecke (O201_064D; unsigned in O072/O201/O213..O223) */
	signed char route_from;		/* +0x37 unterwegs: Start der Strecke (O201_064D; O200: 1, O034: 0x67) */
	signed char route_to;		/* +0x38 unterwegs: Ziel der Strecke (O201_064D) */
	signed short on_route;		/* +0x39 Gruppe ist unterwegs (O201_0DEA/0DF8) */
	signed long route_len;		/* +0x3b Laenge der Strecke (O201_064D: Gegenrichtung route_len - route_pos) */
	signed short turn_pos;		/* +0x3f unterwegs: Strecke beim letzten Umkehren (O213/O215/O216/O217) */
	signed char turns;		/* +0x41 Zahl der Umkehrungen auf dem Weg (ungerade: Gegenrichtung) */
	unsigned char turn_from;	/* +0x42 urspruenglicher Weg vor dem Umkehren: Start (vermutet; O213, O217) */
	unsigned char turn_to;		/* +0x43 Ziel (vermutet) */
	signed char poltergeist_bound;	/* +0x44 vom Poltergeist im Phex-Tempel festgehalten (O204 setzt bei g_dng05_poltergeist,
					   O235 Transversalis gesperrt; frueher f44) */
	signed short screen_x;		/* +0x45 Bildschirmposition (O014_0851: n * 0xa8 + 0x54) */
	signed short screen_y;		/* +0x47 */

#ifdef __cplusplus
	/* Positions-Setter (O014/O064): vorigen Wert merken, neuen setzen */
	void set_pos_x(signed short v, signed short sx) { screen_x = sx; old_x = x; x = v; }	/* R12 view_frame */
	void set_pos_y(signed short v, signed short sy) { screen_y = sy; old_y = y; y = v; }
	void set_screen_x(signed short v) { screen_x = v; }	/* O014 */
	void set_screen_y(signed short v) { screen_y = v; }
	void go_x(signed short v, signed short sx) { set_screen_x(sx); set_x(set_x(v)); }	/* O014 group_set_screen_pos
							   (Kopien: v, sx, Ergebnis des inneren set_x) */
	void go_y(signed short v, signed short sy) { set_screen_y(sy); set_y(set_y(v)); }
	signed short set_x(signed short v) { old_x = x; return x = v; }
	signed short set_y(signed short v) { old_y = y; return y = v; }
	signed char set_loctype2(signed char v) { old_loctype = loctype; return loctype = v; }	/* (frueher set_dir; unbenutzt) */
	signed short get_loctype() { return loctype; }	/* O038, O090: +0x1e als int gelesen */
	signed short set_loctype(signed short v)		/* O034, O038, O046, O183: vorige nur ohne Bit 7 merken */
	{ if (!(loctype & 0x80)) old_loctype = loctype; else ; return loctype = v; }
	void set_x_v(signed short v) { old_x = x; x = v; }	/* O032 init_game_state (ohne Rueckgabe) */
	void set_y_v(signed short v) { old_y = y; y = v; }	/* O032 */
	/* Welle N23 (Dungeonverteiler O060/O061): Feld setzen samt Bildschirmposition v * 0xa8 + 0x54 */
	signed short set_x_screen(signed short v) { screen_x = v * 0xa8 + 0x54; old_x = x; return x = v; }
	signed short set_y_screen(signed short v) { screen_y = v * 0xa8 + 0x54; old_y = y; return y = v; }
	void set_dungeon_v(signed char v) { old_dungeon = dungeon; dungeon = v; }	/* O062 (ohne Rueckgabe) */
	signed short set_loctype_c(signed char v)		/* O032: wie set_loctype, Parameter signed char ([bp-5]) */
	{ if (!(loctype & 0x80)) old_loctype = loctype; else ; return loctype = v; }
	signed char set_town(signed char v) { old_town = town; return town = v; }		/* O034, O038 */
	signed char set_dungeon(signed char v) { old_dungeon = dungeon; return dungeon = v; }	/* O034, O038 */
	void set_dir(signed short v) { dir = v; }		/* O014, O046, O129 (Byte gespeichert) */
	void set_route_from(unsigned char v) { route_from = v; }	/* O038, O223 (Parameterkopie) */
	void set_route_to(unsigned char v) { route_to = v; }		/* O038, O223 */
	void set_route_pos(signed long v) { route_pos = v; }		/* O038, O223 (long-Kopie, mov eax) */
	void clr_hero(signed short i) { heroes[i] = 0; }	/* O075 */
	unsigned long get_route_pos() { return route_pos; }	/* O201_064D: unsigned gelesen (mov eax) */
	/* Reiseteil O213/O215/O216 (Parameterkopien; Strecken dort unsigned) */
	void set_route_len(signed long v) { route_len = v; }		/* O215 (long-Kopie) */
	unsigned long get_route_len() { return route_len; }		/* O215 (mov eax, nur ax gespeichert) */
	void set_turn_pos(signed short v) { turn_pos = v; }		/* O216 */
	void set_turns(signed long v) { turns = v; }			/* O215/O216: cbw / movsx eax / Store nur al */
	signed long get_turns() { return (signed short)turns; }		/* O213/O215: mov al / cbw / movsx eax,ax */
	void set_turn_from(unsigned char v) { turn_from = v; }		/* O217 travel_reverse (Parameterkopie) */
	void set_turn_to(unsigned char v) { turn_to = v; }		/* O217 */
	signed short get_nr_heroes() { return nr_heroes; }		/* O215/O216: mov al / cbw */
	signed short get_y() { return y; }				/* O186: Wort lesen, nur al speichern */
	signed long get_money() { return money; }			/* O114 tevent_025: mov eax (Store nur al) */

	/* --- O200 --------------------------------------------------------------------------------------- */
	group();						/* O200_0000 Group_ctor */
	unsigned long add_money(signed long n);			/* O200_0084 */
	unsigned long sub_money(signed long n);			/* O200_016A */
	unsigned long money_share();				/* O200_01FC */
	signed long sum_ap();					/* O200_0225 */
	signed short count_check1();				/* O200_027F */
	signed short count_check2();				/* O200_02D9 */
	signed short count_check3();				/* O200_0333 */
	void get_ill(signed short ill, signed short chance);	/* O200_038D */
	void infect(signed short ill, signed short chance, struct hero far *except); /* O200_03ED */
	void add_le(signed short le);				/* O200_0457 */
	void heal_full();					/* O200_04A8 */
	void add_le_range(signed short lo, signed short hi);	/* O200_0502 */
	void sub_le(signed short le);				/* O200_0567 */
	void sub_le_range(signed short lo, signed short hi);	/* O200_05B8 */
	void sub_le_dice(signed char dice, signed char add, signed short ignore_rs); /* O200_061D */
	void sub_le_keep(signed short le, signed short min_le);	/* O200_06B7 */
	void add_le_max(signed short v);			/* O200_073C */
	void sub_le_max(signed short v);			/* O200_078D */
	void add_ae(signed short v);				/* O200_07DE */
	void sub_ae(signed short v);				/* O200_082F */
	void add_ae_max(signed short v);			/* O200_0880 */
	void sub_ae_max(signed short v);			/* O200_08D1 */
	void clr_hunger_thirst();				/* O200_0922 */
	void sub_hunger_thirst(signed short hunger, signed short thirst); /* O200_096F */
	void sub_hunger_thirst_max(signed short hunger, signed short thirst); /* O200_09D1 */
	void clr_hunger();					/* O200_0A46 */
	void clr_thirst();					/* O200_0A8A */
	void remove_npc(struct hero far *npc);			/* O200_0ACE */

	/* --- O201 --------------------------------------------------------------------------------------- */
	signed short has_hero_named(char far *name);		/* O201_0000 */
	signed short has_hero(struct hero far *h);		/* O201_007F */
	signed short count_check2b();				/* O201_00CD */
	signed short count_alive();				/* O201_0127 */
	signed short hero_pos(struct hero far *h);		/* O201_017C */
	signed short hero_with_item(signed short id, signed short pos); /* O201_01C3 */
	signed short all_have_item(signed short id, signed short pos); /* O201_02A4 */
	signed short find_food(struct hero far *first, signed short far *hero_no, signed short far *pslot,
		signed short far *pid);				/* O201_03AF */
	signed short find_drink(struct hero far *first, signed short far *hero_no, signed short far *pslot,
		signed short far *pid);				/* O201_04C1 */
	void add_effect(signed long time, signed char type, signed char index, signed char value); /* O201_05D3 */
	signed short same_place(struct group far *o);		/* O201_064D */
	void swap_slots(signed short a, signed short b);	/* O201_0781 */
	signed short draw_hero_bar(signed short slot, signed short active); /* O201_0957 */
	void compact();						/* O201_0B5B */
	signed short mark_can_use(signed short id);		/* O201_0C25 */
	void draw_portraits();					/* O201_0CD8 */
	signed short get_on_route();				/* O201_0DEA */
	void set_on_route(signed short v);			/* O201_0DF8 */

	/* --- O202 --------------------------------------------------------------------------------------- */
	signed short refill_water();				/* O202_0000 */
	void add_ap(signed short ap);				/* O202_00A6 */
	void add_ap_each(signed short ap);			/* O202_0112 */
	void sub_ap_each(signed short ap);			/* O202_0167 */
	signed short free_slot();				/* O202_01BC */
	void add_hero(struct hero far *h, signed short slot);	/* O202_0200 */
	void dismiss_hero(signed short slot, signed short v);	/* O202_035A */
	void remove_hero(signed short slot);			/* O202_0434 */
	void inc_skill(signed short skill);			/* O202_04FD */
	void dec_weapons_bf();					/* O202_054E */
	void mark_weapon(char far *fmt, signed short mode);	/* O202_0599 */
	signed short kk_at_least(signed short n);		/* O202_0609 */
	signed short kk2_at_least(signed short n);		/* O202_0642 */
	void reset_rest();					/* O202_06AC */
	void sleep(signed short mod);				/* O202_070B */
	signed short give_item(signed short id, signed short n);	/* O202_0777 */
	signed short give_item_each(signed short id, signed short n);	/* O202_0976 */
	signed short mage_test_spell(signed short spell, signed char mod); /* O202_0AB7 */
	void drop_item(signed short id);			/* O202_0B44 */
	void drop_most_items();					/* O202_0BB2 */
	void lose_random_items(signed short n);			/* O202_0C70 */
	signed short remove_item(signed short id);		/* O202_0CEF */
	signed short count_free_knapsack();			/* O202_0D68 */
	signed short save();					/* O202_0DCF */
	signed short load();					/* O202_0EBB */
	signed short any_in_jail();				/* O202_0F7A */
	signed short has_dwarf();				/* O202_0FCE */
	signed short has_elf();					/* O202_1032 */
	struct hero far *get_hero(signed short n) { return heroes[n]; }	/* O263 (Parameterkopie beim Weiterschalten) */
#endif
};

/* ======================================================================================================
 * Gegner im Kampf (0x49 Bytes): Tabelle g_enemies (ds:c4f7, 20 Eintraege), Klasse Enemy (O192), gefuellt von
 * fight_fill_enemy (O022_01D5) aus dem 0x30-Byte-Satz der Datei 0x48 (struct mon_rec). Aufbau wie BE
 * ENEMY_SHEET_* (DSA 1: 0x3e Bytes) mit Versatz +8 bis +0x31 (Eigenschaften als Tripel wie beim Helden) und +7 ab
 * +0x32 (DSA 2 hat kein DUMMY3); Schuss/Wurf als Woerter. Namen nach BE, durch DSA-2-Module bestaetigt (N7).
 * ====================================================================================================*/
struct enemy {
	unsigned char mon_id;		/* +0x00 Gegnerart (Index Datei 0x48, g_mon_names) (O143/O153/O154: unsigned) */
	signed char gfx_id;		/* +0x01 Kampfgrafik; Figurbild = gfx_id + 0xb5 (O153 fig_enemy_pic) */
	signed char rs;			/* +0x02 RS */
	signed char f03;		/* +0x03 (aus Satzbyte 3; BE: ATTRIB_ORIG) */
	struct hero_attrib attrib[7];	/* +0x04 Eigenschaften (O022: normal = cur = Satzwert) */
	signed short le_max;		/* +0x19 LE-Grundwert */
	signed short le;		/* +0x1b LE (O079/O233) */
	signed short ae_max;		/* +0x1d */
	signed short ae;		/* +0x1f */
	signed char mr;			/* +0x21 MR (O091 Zauberprobe) */
	signed char first_ap;		/* +0x22 AP beim ersten Sieg (O143: bekannt -> first_ap / 10) (BE FIRSTAP) */
	signed char attacks;		/* +0x23 Aktionen je Runde (O147: attacks_left = attacks) */
	signed char at;			/* +0x24 AT (O079) */
	signed char pa;			/* +0x25 PA (O079) */
	signed short dam1;		/* +0x26 Schaden 1 (Wuerfelcode, O192_029B) */
	signed short dam2;		/* +0x28 Schaden 2 */
	signed char bp_max;		/* +0x2a BP-Grundwert, hoechstens 10 (O022) (BE BP_ORIG) */
	signed char bp;			/* +0x2b BP der Runde (O147: bp = bp_max) */
	signed char magic;		/* +0x2c magisch (O079) */
	signed char mag_id;		/* +0x2d Zauberrepertoire (BE MAG_ID) */
	signed char fighter_id;		/* +0x2e Figur in der Kampfliste, -1 = keine */
	signed char viewdir;		/* +0x2f Blickrichtung 0..3 */
	signed char attacks_left;	/* +0x30 verbleibende Aktionen (O128: -- je Zug) */
	signed char level;		/* +0x31 Stufe */
	signed char action_id;		/* +0x32 Kampfaktion FIG_ACTION_* (O020: 12 nichts, 2 Angriff, 15, 4) */
	signed char cur_spell;		/* +0x33 Zauber (O141: aus g_sfight_spells fuer enemy_cast_spell; O138: Geschossbild
					   nach Paritaet); Sonderkampf O020/O141: Angriffsart 0..3 (BE CUR_SPELL) */
	signed char enemy_id;		/* +0x34 Ziel: 0 keins, 1..9 Held (Index + 1), >= 10 Gegner (Index + 10) */
	signed char saftkraft;		/* +0x35 (BE SAFTKRAFT) */
	signed char blind;		/* +0x36 Runden geblendet (O128: Zug entfaellt, --) */
	signed char broken;		/* +0x37 Waffe zerbrochen (BE BROKEN) */
	unsigned short dead:1;		/* +0x38.0 tot (O192_0413) (BE enemy_flags1) */
	unsigned short asleep:1;	/* +0x38.1 schlaeft (O128: wacht zu f->round + 10 % auf) */
	unsigned short petrified:1;	/* +0x38.2 versteinert */
	unsigned short busy:1;		/* +0x38.3 */
	unsigned short f38_b4:1;
	unsigned short tied:1;		/* +0x38.5 gefesselt */
	unsigned short mushroom:1;	/* +0x38.6 */
	unsigned short illusion:1;	/* +0x38.7 */
	unsigned short tame:1;		/* +0x39.0 (BE enemy_flags2) */
	unsigned short renegade:1;	/* +0x39.1 abtruennig (O152, O153_0C04; O192_0413 loescht es) */
	unsigned short scared:1;	/* +0x39.2 flieht (O152: le_flee >= le; O128: alle bei g_65e5) */
	unsigned short dancing:1;	/* +0x39.3 */
	unsigned short f39_b4:1;
	unsigned short f39_b5:1;	/* +0x39.5 (O143: mit renegade nur 1 AP) */
	unsigned short f39_b6:1;
	unsigned short f39_b7:1;
	signed char f3a;		/* +0x3a (BE UNUSED8) */
	signed char size;		/* +0x3b Groesse (O079) */
	signed char round_appear;	/* +0x3c erscheint in Runde n (O147 fight_latecomers), 0 = von Anfang an */
	signed char is_animal;		/* +0x3d Tier (O144: eigener Trefferklang 0x53) */
	signed short shots;		/* +0x3e Schuesse (O192_0741 zaehlt herunter) */
	signed short shot_dam;		/* +0x40 Schuss-Schaden (Wuerfelcode) */
	signed short throws;		/* +0x42 Wuerfe */
	signed short throw_dam;		/* +0x44 Wurf-Schaden */
	signed char le_flee;		/* +0x46 flieht bei LE <= le_flee (O152; Sonderkampf O020: Ende) */
	signed char f47;		/* +0x47 (Satzbyte 0x2f, auch g_mon_f47[Art]) */
	signed char parried;		/* +0x48 in dieser Runde schon pariert (O136: Paradebild nur beim ersten Mal;
					   O147 fig_round_enemies: 0) - Gegenstueck zu hero.parried */

#ifdef __cplusplus
	signed char mr_eff() { return mr; }	/* O091 test_spell */
	unsigned char is_mushroom() { return mushroom; }	/* O091 test_spell: Zauber wirkungslos */
	unsigned char get_mon_id() { return mon_id; }	/* O143: Wert vor der Zieladresse laden */
	unsigned char is_scared() { return scared; }	/* O155 (Rueckgabe unsigned char) */

	/* Setter (O022 fight_fill_enemy, O020 sfight_enemy_turn): jede Expansion legt eine Parameterkopie an */
	void set_mon_id(signed char v) { mon_id = v; }
	void set_gfx_id(signed char v) { gfx_id = v; }
	void set_rs(signed char v) { rs = v; }
	void set_f03(signed char v) { f03 = v; }
	void set_attrib(signed short i, signed char v) { attrib[i].normal = attrib[i].cur = v; }
	void set_le_max(signed short v) { le_max = v; }
	void set_le(signed short v) { le = v; }
	void set_ae_max(signed short v) { ae_max = v; }
	void set_ae(signed short v) { ae = v; }
	void set_mr(signed char v) { mr = v; }
	void set_first_ap(signed char v) { first_ap = v; }
	void set_attacks(signed char v) { attacks = v; }
	void set_at(signed char v) { at = v; }
	void set_pa(signed char v) { pa = v; }
	void set_dam1(signed short v) { dam1 = v; }
	void set_dam2(signed short v) { dam2 = v; }
	void set_bp_max(signed char v) { bp_max = v; }
	void set_magic(signed char v) { magic = v; }
	void set_mag_id(signed char v) { mag_id = v; }
	void set_fighter_id(signed char v) { fighter_id = v; }
	void set_viewdir(signed char v) { viewdir = v; }
	void set_level(signed char v) { level = v; }
	void set_action_id(signed char v) { action_id = v; }
	void set_enemy_id(signed char v) { enemy_id = v; }
	void set_size(signed char v) { size = v; }
	void set_round_appear(signed char v) { round_appear = v; }
	void set_is_animal(signed char v) { is_animal = v; }
	void set_shots(signed short v) { shots = v; }
	void set_shot_dam(signed short v) { shot_dam = v; }
	void set_throws(signed short v) { throws = v; }
	void set_throw_dam(signed short v) { throw_dam = v; }
	void set_le_flee(signed char v) { le_flee = v; }
	void set_f47(signed char v) { f47 = v; }
	/* Welle N9 (O137/O138/O141/O147) */
	void set_renegade(signed char v) { renegade = v; }		/* O137 Beschwoerung */
	void set_attacks_left(signed char v) { attacks_left = v; }	/* O138, O147 */
	void set_bp(signed char v) { bp = v; }				/* O147 */
	void set_cur_spell(signed char v) { cur_spell = v; }		/* O141 */
	/* Welle N13 (Zauberwirkungen): add_* ohne Begrenzung (O194 begrenzt danach selbst), *_min0 begrenzt bei 0 */
	void add_rs(signed char v) { rs += v; }				/* O194 */
	void add_at(signed char v) { at += v; }				/* O194 */
	void add_pa(signed char v) { pa += v; }				/* O194 */
	void add_rs_min0(signed char v) { rs += v; if (rs < 0) rs = 0; else ; }	/* O236 */
	void add_at_min0(signed char v) { at += v; if (at < 0) at = 0; else ; }	/* O234, O236 */
	void add_pa_min0(signed char v) { pa += v; if (pa < 0) pa = 0; else ; }	/* O234, O236 */
#endif
};

/* Zielart des Gegners aus enemy_id (wie target_type beim Helden), je Vergleich neu ausgewertet (O138, O194) */
#define ENEMY_TARGET_TYPE(e) ((signed char)((unsigned char)(e)->enemy_id < TARGET_ID_ENEMY ? \
	((unsigned char)(e)->enemy_id == 0 ? TARGET_SELF : TARGET_HERO) : TARGET_ENEMY))

struct mon_spell_desc {		/* 10 Bytes: Gegnerzauber, g_mon_spell_descriptions (ds:22d6) je enemy.cur_spell
					   (BE MON_SPELL_DESCRIPTIONS, dort 8 Bytes) */
	signed char ae_cost;		/* +0 AE-Kosten (O194 cost) */
	signed char mode;		/* +1 Zielart: 3 Gegner, 1 Gegner/abtruennig, 2 Held/eigene Seite, 0 selbst
					   (O151 get_foe_attack_mode) */
	signed char ranged;		/* +2 Fernzauber (O152: Zielsuche Reichweite 99 statt 1) (BE UNKN1) */
	signed char attrib1;		/* +3 Probeneigenschaften (O192 enemy_test_spell) */
	signed char attrib2;		/* +4 */
	signed char attrib3;		/* +5 */
	signed char vs_mr;		/* +6 gegen MR des Ziels (O192 enemy_test_spell) */
	signed char ani_kind;		/* +7 Wirkungsbild (= mon_spell_ani.kind, O138) (BE ANI_ID) */
	signed short snd;		/* +8 Klang (O160/O161; fehlt er im Archiv: 0x92) */
};

struct spell_snd {			/* 12 Bytes: Teilsicht ab g_spell_descriptions[0].snd (ds:1eb4, eigenes Public) */
	signed short snd;		/* +0 Klang des Heldenzaubers (O158/O159) */
	signed char _r2[10];
};

struct mon_rec {			/* 0x30 Bytes: Gegnersatz der Datei 0x48 (O022 fight_fill_enemy; BE MONSTER_*) */
	signed char mon_id;		/* +0x00 */
	signed char gfx_id;		/* +0x01 */
	signed char rs;			/* +0x02 */
	signed char f03;		/* +0x03 */
	signed short attrib[7];		/* +0x04 Eigenschaften (Wuerfelcode, fight_roll_value) */
	signed short le;		/* +0x12 (Wuerfelcode) */
	signed short ae;		/* +0x14 (Wuerfelcode) */
	signed short mr;		/* +0x16 (Wuerfelcode) */
	signed char first_ap;		/* +0x18 */
	signed char attacks;		/* +0x19 */
	signed char at;			/* +0x1a */
	signed char pa;			/* +0x1b */
	signed short dam1;		/* +0x1c */
	signed short dam2;		/* +0x1e */
	signed char bp;			/* +0x20 */
	signed char magic;		/* +0x21 */
	signed char mag_id;		/* +0x22 */
	signed char level;		/* +0x23 */
	signed char size;		/* +0x24 */
	signed char is_animal;		/* +0x25 */
	signed short shots;		/* +0x26 */
	signed short shot_dam;		/* +0x28 */
	signed short throws;		/* +0x2a */
	signed short throw_dam;		/* +0x2c */
	signed char le_flee;		/* +0x2e */
	signed char f2f;		/* +0x2f -> enemy.f47, g_mon_f47[Art] */
};

/* ======================================================================================================
 * Kampf: Daten zum Kampfobjekt (Klasse Fight, 0xfb1 Bytes, s_class.h) - Kampffeld, Aufstellung, Ablauf-
 * beschreibungen der Kampffiguren, Neuzeichnen. Inventar der Felder: Welle N7 (O018..O022, O128..O153).
 * ====================================================================================================*/
struct dxy {				/* 4 Bytes: Versatz {dx, dy} (g_fig_twofield_dxy, g_fig_obj_dxy, g_fig_dirs4) */
	signed short dx;
	signed short dy;
};

struct cxy {				/* 2 Bytes: Feldversatz {x, y} (Kampffeld-Erzeugung O139/O140) */
	signed char x;
	signed char y;
};

struct range8 {				/* 2 Bytes: Bereich {lo, hi} (g_fig_ranges: Abstand der Gegner zu den Helden) */
	signed char lo;
	signed char hi;
};
struct range8x9 { struct range8 r[9]; };	/* Hauszauberbereich je Akademie (g_chargen_school_ranges; Strukturkopie, O170) */

struct dirs4 { struct dxy d[4]; };	/* vier Richtungen als Strukturkopie (O139 fight_path_to_start) */

struct ani_step {			/* 4 Bytes: Schritt einer Ablaufbeschreibung (Ende: frame == 0xff) */
	unsigned char frame;		/* +0 Einzelbild (bzw. Folge) */
	unsigned char type;		/* +1 Art (3 = mit Ereignissen; O112_0127: Art 4 = fertig) */
	unsigned char count;		/* +2 Wiederholungen bzw. letztes Ereignis (0xff = Dauer) */
	unsigned char event;		/* +3 Ereignis am Ende des Schritts (ANI_EV_*: 0x1f Partner starten, 0x20 Ende,
					   0x22 + Richtung Blickrichtung setzen; O112/O133) */
};

struct ani_seq {			/* 0xa2 Bytes: Ablaufbeschreibung einer Kampffigur (O016 Konstruktor, O112_0101) */
	signed short figure;		/* +0x00 Figur-/Bildnummer (0x91 Feldobjekt, 0x9f Sonderkampf, 0xb5.. Helden) */
	struct point far *path;		/* +0x02 Bewegungspfad (Bildschirmpunkte; O133: letzter Punkt -> Feld) */
	struct fig_anim far *linked;	/* +0x06 Partner-Animation (Geschoss/Effekt), Ereignis 0x1f startet sie (O112) */
	struct ani_step step[38];	/* +0x0a Schritte */
};

#ifdef __cplusplus
class Bmp;
#endif

struct fig_frame {			/* 0x0e Bytes: Kopf des geladenen Einzelbilds (fig_anim.frame_hdr; O016 new, O112) */
	signed short x;			/* +0x00 Bildversatz zur Figurlage */
	signed short y;			/* +0x02 */
	signed short w;			/* +0x04 Breite */
	signed short h;			/* +0x06 Hoehe */
	signed long size;		/* +0x08 gepackte Groesse in der Datei */
	unsigned char pack;		/* +0x0c Packart (decomp) */
	unsigned char event;		/* +0x0d Ereignis dieses Einzelbilds (fig_anim_draw liefert es) */
#ifdef __cplusplus
	fig_frame(signed short x0, signed short y0, signed short w0, signed short h0, signed long sz,
		unsigned char ev, unsigned char pk) {	/* O016 fig_anim_load_frame (Parameterkopien, fd vor fc) */
		x = x0; y = y0; w = w0; h = h0; size = sz; event = ev; pack = pk;
	}
#endif
};

struct fig_anim {			/* 0xe0 Bytes: Animationsobjekt einer Kampffigur (Klasse FigAnim: O016/O112) */
	struct ani_seq seq;		/* +0x00 Kopie der Ablaufbeschreibung */
	unsigned char step;		/* +0xa2 laufender Schritt in seq.step[] */
	signed short seq_id;		/* +0xa3 Folge des laufenden Schritts (Satz in der Bilddatei; O133 Aktion 33: Lage der
					   Sonderfigur -> g_sfight_pos; O112: Sonderfall Kampf 0x493) (frueher fa3) */
	signed short started;		/* +0xa5 laeuft (O112 zeichnet/taktet nur dann; O137/O138: Effekt sofort starten) */
	signed short ticked;		/* +0xa7 in diesem Takt weitergeschaltet (fig_anim_step) */
	signed short sync;		/* +0xa9 vom Vorgaenger der Kette angestossen: beim naechsten Takt weiterschalten */
	signed short running;		/* +0xab Folge laeuft, 0 = beendet (Ereignis 0x20, Ende der Schritte) */
	signed long seq_pos;		/* +0xad Dateiposition der Einzelbilder der Folge */
	signed char loaded;		/* +0xb1 Folgenkopf des Schritts geladen (fig_anim_load_seq); 0 = neuer Schritt (O144:
					   Bildversatz = Feld) (frueher moving) */
	signed short fb2;		/* +0xb2 (aus dem Folgensatz, +6) */
	signed char fb4;		/* +0xb4 (aus dem Folgensatz, +8) */
	signed short ofs_x;		/* +0xb5 Bildversatz der Folge */
	signed short ofs_y;		/* +0xb7 */
	unsigned char nr_frames;	/* +0xb9 Zahl der Einzelbilder der Folge */
	unsigned char delay;		/* +0xba Takte je Einzelbild (delay_fix bzw. g_anim_delays[Geschwindigkeit]) */
	signed char delay_fix;		/* +0xbb feste Takte je Einzelbild, -1 = nach Geschwindigkeit (O020/O022/O134: 10/15,
					   Geschosse/Effekte O137/O138/O142: 0) (frueher prio) */
	signed char fbc;		/* +0xbc (fig_anim_reset: 0) */
	unsigned long last_tick;	/* +0xbd g_anim_ticks beim letzten Weiterschalten */
	unsigned char frame;		/* +0xc1 aktuelles Einzelbild der Folge */
	unsigned char frame_loaded;	/* +0xc2 geladenes Einzelbild, 0xff = keins */
	unsigned char loops;		/* +0xc3 Durchlaeufe des Schritts (Wiederholungen; bei Einzelbild-Dauerschritten je
					   Takt +1 - O020: Ereignisse danach) (frueher cur_frame) */
	signed char skip;		/* +0xc4 nur jedes zweite Zeichnen schaltet weiter (fig_anim_draw mit slow) */
	struct fig_frame far *frame_hdr;	/* +0xc5 Kopf des geladenen Einzelbilds */
#ifdef __cplusplus
	Bmp far *bmp;			/* +0xc9 Einzelbild (Bmp ueber pixels) */
#else
	void far *bmp;
#endif
	struct fig_anim far *next;	/* +0xcd verkettete Animation (laeuft im Takt mit; O142 set_next) */
	unsigned char phase;		/* +0xd1 Taktphase (je fig_anim_step umgeschaltet; Abgleich in der Kette) */
	unsigned char path_pos;		/* +0xd2 naechster Pfadpunkt in seq.path (O133: path[path_pos - 1] = letzter) (frueher
					   path_len) */
	void far *pixels;		/* +0xd3 Bildpuffer (g_anim_small_buf, g_anim_big_buf oder eigener) */
	signed short pos_frame;		/* +0xd7 Einzelbild, hinter dem pos steht */
	signed long pos;		/* +0xd9 Dateiposition hinter Einzelbild pos_frame */
	signed char slot;		/* +0xdd XMS-Platz in g_anim_slots (kleine Bilder: slot * 0x550 + 0x1f400) */
	signed short in_xms;		/* +0xde kleines Bild liegt im XMS-Platz */
#ifdef __cplusplus
	void set_next(struct fig_anim far *n) { next = n; }	/* O142 (Parameterkopie per mov eax) */
#endif
};

struct anim_delays {			/* 8 Bytes: Takte je Einzelbild nach Geschwindigkeitsstufe 0..3 (g_anim_delays: 0, 3, 7, 15;
					   O016 kopiert die Tabelle als Strukturkopie) */
	signed short d[4];
};

struct fight_field {			/* 0x26d Bytes: Kampffeld (Fight.field; O139 aus der Umgebung, O023_0341 aus Datei 0x8f) */
	signed char f00[0x14];		/* +0x00 */
	unsigned char type;		/* +0x14 Art der Waende (1/3: +20, 2: +45; O021: 6) */
	unsigned char cells[24][25];	/* +0x15 Feldinhalt (0 frei, 0x32.. Waende/Objekte, 0xfd..0xff Sonderfelder) */
};

struct fight_monster {			/* 5 Bytes: Gegner der Aufstellung (Fight.monsters, BE FIGHT_MONSTERS) */
	signed char mon_id;		/* +0 Gegnerart, 0 = leer */
	signed char x;			/* +1 Feld */
	signed char y;			/* +2 */
	signed char viewdir;		/* +3 Blickrichtung */
	signed char round_appear;	/* +4 Erscheinungsrunde */
};

struct fight_hero_pos {			/* 4 Bytes: Startplatz eines Helden (Fight.heroes, BE FIGHT_PLAYERS) */
	signed char x;			/* +0 Feld, -1 = keiner */
	signed char y;			/* +1 */
	signed char viewdir;		/* +2 Blickrichtung */
	signed char asleep;		/* +3 schlaeft zu Beginn (O022: set_asleep) */
};

struct fight_group {			/* 4 Bytes: Gegnergruppe einer Kampfdefinition */
	signed char mon_id;		/* +0 Gegnerart */
	signed char count;		/* +1 Anzahl */
	signed char range;		/* +2 Abstand zu den Helden: Index in g_fig_ranges */
	signed char round_appear;	/* +3 Erscheinungsrunde */
};

struct fight_def {			/* 0x45 Bytes: Kampfdefinition, g_fight_defs_ab[Kampfnummer - 0xab] (O140) */
	signed char intro_done;		/* +0x00 -> Fight.intro_done */
	signed char _r1[2];
	struct fight_group groups[5];	/* +0x03 */
	signed short loot[20];		/* +0x17 Beute (Gegenstaende, 0 = Ende) */
	signed short ducats;		/* +0x3f Geld: Dukaten */
	signed short silver;		/* +0x41 Silbertaler */
	signed short heller;		/* +0x43 Heller */
};

struct fig_rect {			/* 11 Bytes: gezeichnete Figur (Fight.draw_tab, O144 fight_draw_figures) */
	unsigned char id;		/* +0 Figur-ID, 0xff = Ende */
	signed short x0;		/* +1 */
	signed short y0;		/* +3 */
	signed short x1;		/* +5 */
	signed short y1;		/* +7 */
	signed short frame;		/* +9 Bildnummer bzw. Einzelbild */
};

struct dirty_rect {			/* 10 Bytes: neu zu zeichnendes Rechteck (g_fig_dirty[20], Ende x0 == -1) */
	signed short x0;
	signed short y0;
	signed short x1;
	signed short y1;
	signed short used;		/* +8 */
};

struct fight_msg {			/* 14 Bytes: Kampfmeldung (Objekt g_fight_list, Klasse Lst O175) */
	signed short id;		/* +0x00 Ereignis (Text txt[id], Farbe g_fig_msg_colors[id]), 0 = Ende */
	signed short value;		/* +0x02 Wert (Schaden) */
	signed char type1;		/* +0x04 Art von obj1: 2 = Held, sonst Gegner */
	void far *obj1;			/* +0x05 */
	signed char type2;		/* +0x09 */
	void far *obj2;			/* +0x0a */
};

struct fight_msgs {			/* 0x14e Bytes: Meldungsliste des Kampfes (g_fight_list, Klasse Lst O175) */
	signed short count;		/* +0x00 Zahl der Meldungen (Lst_add*, hoechstens 14) */
	signed short cur;		/* +0x02 angezeigte Meldung */
	signed short delay;		/* +0x04 Restdauer */
	signed short delay_init;	/* +0x06 Dauer je Meldung */
	struct fight_msg e[15];		/* +0x08 */
	signed short active;		/* +0xda */
	signed short snd_done;		/* +0xdc Klang zur Meldung schon gespielt */
	void far *screen;		/* +0xde Zielpuffer (Lst_set) */
	void far *txt_buf;		/* +0xe2 Texte 0x9a */
	char far * far *txt;		/* +0xe6 */
	char buf[0x50];			/* +0xea Meldungstext */
	signed char _r13a[0x14e - 0x13a];
};

struct hero_ptrs7 {			/* 28 Bytes: sieben Heldenzeiger als Strukturkopie (O143: make_group-Argumente; O034 group_scatter) */
	struct hero far *h[7];
};

struct fig_keys {			/* 14 Bytes: Tasten der Kampfaktionen (g_fig_keys, O144 fight_check_keys) */
	signed short k[7];
};

struct frame_event {			/* 2 Bytes: Ereignis ab Einzelbild (Sonderkampf-Sequenzen O020) */
	unsigned char frame;		/* +0 ab diesem Einzelbild der Sonderfigur */
	signed char code;		/* +1 Ereignis (Text/Klang) */
};
struct frame_events6 { struct frame_event e[6]; };	/* als Strukturkopie (O020 sfight_seq_end) */
struct frame_events8 { struct frame_event e[8]; };	/* (O020 sfight_seq_intro) */
struct sfight_frames { signed short frame[7]; };	/* Bildfolgen der Hauptsequenz (O020 sfight_seq_main) */

/* --- Kampf Teil B (Welle N9) ---------------------------------------------------------------------------- */
struct ani_step9 {			/* Teilsicht: Schritt um ein Byte verschoben (O136 fig_enemy_turn) */
	unsigned char prev_event;	/* +0 = event des vorigen Schritts */
	unsigned char frame;		/* +1 */
	unsigned char type;		/* +2 */
	unsigned char count;		/* +3 */
};

struct ani_seq9 {			/* Teilsicht auf struct ani_seq mit Schritten ab +9 (O136: e[n].prev_event schreibt
					   das Ereignis des zuletzt angehaengten Schritts) */
	signed short figure;		/* +0x00 */
	signed char _r02[7];
	struct ani_step9 e[38];		/* +0x09 */
};

struct sfight_starts {			/* 28 Bytes: Startplaetze der Helden im Sonderkampf (g_sfight_starts, O141) */
	struct fight_hero_pos p[7];
};

struct sfight_spells {			/* 26 Bytes: Zauber des Sondergegners (g_sfight_spells, O141 sfight_special_attack) */
	signed short sp[13];
};

struct seq27 { signed char frame[27]; };	/* Bildfolge, Ende -1 (g_sfight_spell_seq1, O142) */
struct seq25 { signed char frame[25]; };	/* (g_sfight_spell_seq2) */
struct points7 { struct point p[7]; };		/* sieben Punkte als Strukturkopie (O142 sfight_special_picture) */

struct spell_ani {			/* 12 Bytes: Teilsicht ab g_spell_descriptions[0].f6 (ds:1eb0, eigenes Public) */
	signed char kind;		/* +0 Wirkungsbild des Zaubers (O137/O138: 0 bzw. 2 Standbild, 3 Feldobjekt 0x5b,
					   5 Verwandlung, 7/8 Beschwoerung, sonst Effekt 0xd8) */
	signed char _r1[11];
};

struct mon_spell_ani {			/* 10 Bytes: Teilsicht ab Byte +7 der Gegnerzauber (ds:22d6, 10 Bytes je Zauber) */
	signed char kind;		/* +0 Wirkungsbild wie spell_ani.kind */
	signed char _r1[9];
};

/* --- ani: Hintergrundanimationen eines Bildes (Objekt g_screen_pic; Laden O055 nach BE load_ani, Abspielen O054 wie
 * die ANI-Bereiche im Zeitgeber von DSA 1). Ein Grundbild (w x h) mit bis zu 10 Teilanimationen (struct ani_area,
 * BE ANI_AREA_*), je Teil bis zu 10 Varianten (Flags, Phasenfolgen). */
struct ani_flags {			/* 1 Byte: Flags einer Teilanimation je Variante */
	unsigned short pingpong:1;	/* Bit 0 am Ende rueckwaerts statt von vorn (BE ANI_AREA_CYCLIC) */
	unsigned short masked:1;	/* Bit 1 maskiert zeichnen (Modus 4); setzt ani.multi (gemeinsames Neuzeichnen) */
	unsigned short off:1;		/* Bit 2 Teil ruht */
	unsigned short b3:1;
	unsigned short order:4;		/* Bits 4..7 Zeichenreihenfolge (ani.order) */
};

struct ani_change {			/* 4 Bytes: Phase einer Teilanimation (BE ANI_AREA_CHANGES_TB) */
	unsigned char pic;		/* +0 Einzelbild (1..), 0 = keins */
	unsigned char code;		/* +1 Marke (ani.stop: Abspielen bis zu dieser Marke) */
	signed short dur;		/* +2 Dauer (Zeitgebertakte / 2) */
};

struct ani_changes {			/* 0x28 Bytes: Phase je Variante 0..9 (Variante 0 = aktive) */
	struct ani_change v[10];
};

struct ani_area {			/* 0x92 Bytes: Teilanimation (BE ANI_AREA, DSA 1: 0x107 Bytes) */
	char name[5];			/* +0x00 Name (4 Zeichen + 0) */
	signed short x;			/* +0x05 Lage im Grundbild */
	unsigned char y;		/* +0x07 */
	unsigned char h;		/* +0x08 */
	signed short w;			/* +0x09 */
	struct ani_flags fl[10];	/* +0x0b Flags je Variante (0 = aktive) */
	unsigned char nr_pics;		/* +0x15 Zahl der Einzelbilder */
	void far *pics[25];		/* +0x16 Einzelbilder im Speicher (ani.in_mem), Bild n = pics[n - 1] */
	signed short nr_changes[10];	/* +0x7a Zahl der Phasen je Variante */
	struct ani_changes far *changes;	/* +0x8e Phasen */
};

struct ani {				/* 0xc9 Bytes: Animationsobjekt eines Bildes (Klasse Ani, g_screen_pic) */
	signed char _r00[8];
	signed short w;			/* +0x08 Grundbild */
	signed short f0a;		/* +0x0a (Dateiwert vor der Palette) */
	signed short f0c;		/* +0x0c (Dateiwert vor der Palette) */
	unsigned char h;		/* +0x0e */
	signed char count;		/* +0x0f Zahl der Teilanimationen (BE ANI_AREACOUNT) */
	struct ani_area far *areas;	/* +0x10 Teilanimationen */
	char far *pic;			/* +0x14 Grundbild */
	unsigned char packed;		/* +0x18 PP20-gepackt */
	unsigned char nr_colors;	/* +0x19 Farben der Palette */
	char huge *pal;			/* +0x1a Palette (im Datenblock bzw. eigener Puffer) */
	signed char multi;		/* +0x1e ein Teil maskiert: bei jeder Aenderung alle Teile neu (ani_flags.masked) */
	signed char nr_variants;	/* +0x1f */
	signed char nr_changes[11];	/* +0x20 Phasen je Teil (Datei) */
	signed char f2b[10];		/* +0x2b (Version 1: aus der Datei; altes Format: [0] = 1) */
	signed short mode;		/* +0x35 Zeichenmodus des Grundbilds (Bmp::put; 5 = halt) */
	signed short pal_size;		/* +0x37 Farben fuer set_palette (Vorgabe 0x40) */
	signed short x;			/* +0x39 Lage */
	signed short y;			/* +0x3b */
	signed short run;		/* +0x3d laeuft */
	signed short busy;		/* +0x3f zeichnet gerade */
	signed short id;		/* +0x41 Datei, -1 = keine */
	signed short old_format;	/* +0x43 Datei ohne "ANI"-Kopf */
	char huge *data;		/* +0x45 Datenblock (ganze Datei) bzw. Grundbild */
	signed short stop_at;		/* +0x49 Abspielen bis Marke (> 0), bis Ende (-1), 0 = ohne */
	signed short timer[10];		/* +0x4b Restdauer je Teil (BE ANI_AREA_TIMEOUT) */
	signed short cur[10];		/* +0x5f Phase je Teil (BE ANI_AREA_STATUS) */
	signed short step[10];		/* +0x73 Schrittweite +1/-1 (pingpong) */
	signed short order[10];		/* +0x87 Teil je Zeichenposition */
#ifdef __cplusplus
	Bmp far *bmp[10];		/* +0x9b Bildobjekt je Teil (in_mem) */
#else
	void far *bmp[10];
#endif
	signed short in_mem;		/* +0xc3 alle Einzelbilder im Speicher (sonst je Schritt aus der Datei) */
	signed long pos;		/* +0xc5 Dateiversatz */
};

struct anim_view {			/* Teilsicht auf das Animationsobjekt (Klasse Anim O050/O051, 0xaf Bytes; O134/O189) */
	signed char _r00[0x19];
	signed short nr_seqs;		/* +0x19 Zahl der Sequenzen (Dateikopf, O050) */
	signed char _r1b[0x21 - 0x1b];
	unsigned char rate;		/* +0x21 Takte je Bild (O134: 3 bzw. 2) (frueher end_mode) */
	signed char _r22[4];
	unsigned char rounds;		/* +0x26 abgeschlossene Durchlaeufe (O134 wartet auf den ersten) (frueher done) */
#ifdef __cplusplus
	void set_rate(unsigned char v) { rate = v; }	/* O134 (frueher set_end_mode) */
#endif
};

/* ======================================================================================================
 * Residente Dienste (R02 Eingabe/EMS, R04 Klang, R06 Archiv, R07 Speicher, R11 Kompass, R12/R13 3D-Ansicht)
 * ====================================================================================================*/
/* --- 3D-Ansicht (R12 Zeichnen, R13 Kollision, R24 Polygone) ------------------------------------------- */
struct vtx {				/* 8 Bytes, Punkt bzw. Winkeltripel (x, y, z, w unbenutzt) */
	signed short x, y, z, w;
};

struct vtx_list {			/* Punktliste eines 3D-Objekts */
	signed short n;
	struct vtx v[1];
};

struct tex_ref {			/* 6 Bytes, Lage einer Textur (g_cur_tex) */
	unsigned long off;		/* +0 Offset (EMS: >> 14 = Seite) bzw. Abstand zu g_tex_base */
	signed short seg;		/* +4 Segment, 0 = noch im EMS bzw. relativ */
};

struct texture {			/* 12 Bytes: Textur, Eintrag im Bild-Cache g_textures (O097 tex_load; R12 -> R24) */
	unsigned short w;		/* +0 Breite (-> g_cur_tex_w) (frueher f00) */
	unsigned short h;		/* +2 Hoehe (O198: Himmel ohne die letzten 4 Zeilen) */
	unsigned short nr_colors;	/* +4 Farben der Bildpalette */
	struct tex_ref ref;		/* +6 Lage im Auslagerungsspeicher (beim Laden g_swap_pos, seg 0) */
};

struct uv_pair {			/* 4 Bytes: Texturkoordinaten einer Ecke (face3d.uv) */
	signed short u, v;
};

struct face3d {				/* 0x48 Bytes: Flaeche eines 3D-Objekts */
	signed short nx, ny, nz;	/* +0x00 Normale (Laenge 0x100; O010 obj3d_prepare) */
	signed char _r06[0x10 - 6];	/* +0x06; +0x08/+0x0a/+0x0c: Normale in Kamerakoordinaten (R25 obj_project schreibt sie) */
	signed short nverts;		/* +0x10 Zahl der Ecken */
	unsigned short flags;		/* +0x12 FACE_* */
	struct texture far *tex;	/* +0x14 Textur (FACE_TEXTURED); einfarbig: 24-Bit-Farbe aus der Datei, Byte +0x17
					   danach Palettenindex (O010); FACE_CLICK: struct map_event (O198 traegt die Zelle
					   ein) */
	signed short idx[8];		/* +0x18 Eckenindizes */
	signed short uv[16];		/* +0x28 Texturkoordinaten je Ecke (u, v; poly_edges; O097 als struct uv_pair) */
};

struct vtx_path {			/* n * 8 + 4 Bytes: bewegter Eckpunkt (vtx.x == VTX_PATH, vtx.y = Index in g_vtx_paths) */
	signed short n;			/* +0 Zahl der Lagen */
	signed short m;			/* +2 (aus der Objektdatei) */
	struct vtx e[1];		/* +4 Lagen (x, y, z) */
};

struct cell_rot {			/* 8 Bytes, Drehwinkel des Zellobjekts (get_cell_c) */
	signed short ax, ay, az, aw;
};

struct obj3d {				/* 0x2c Bytes: 3D-Objekt (je Kartenzellentyp g_cell_objs[]; O097 obj_load) */
	signed short x, y, z;		/* +0x00 Lage relativ zur Kamera (O198 view_draw) */
	signed short _r06;
	struct cell_rot rot;		/* +0x08 Drehung (get_cell_c); rot.az = Eigenwinkel (FACE_ROTATE; frueher angle) */
	struct vtx box_min;		/* +0x10 Huellquader (O010 obj3d_prepare: -max, -max, max) */
	struct vtx box_max;		/* +0x18 (max, max, max) */
	struct vtx_list far *vl;	/* +0x20 Punkte */
	signed short nfaces;		/* +0x24 */
	struct face3d far * far *faces;	/* +0x26 */
	signed short phase;		/* +0x2a Tuerstufe 0..15 (O198 aus der Karte) bzw. Animationsphase */
};

struct vis_order {			/* 0x80 Bytes je Blickrichtung (32): Reihenfolge der Nachbarzellen (Datei 0x405,
					   g_vis_order; O198), dx[i] bis 0x7f, dy[i] */
	signed char dx[0x40];
	signed char dy[0x40];
};

struct view_obj {			/* 3 Bytes: Objektliste je Stadt/Dungeon (view_area.objs; O097), Ende idx -1 (-2: Rest
					   nur ohne Sparbetrieb) */
	signed char idx;		/* +0 Platz in g_cell_objs */
	signed short file;		/* +1 Objektdatei */
};

struct face_ent {			/* 18 Bytes, sichtbare Flaeche (g_face_list, Reihenfolge g_face_next) */
	struct face3d far *face;	/* +0x00 */
	signed char f04[6];		/* +0x04 erster Punktindex des Objekts (R25: FS), +0x06/+0x08 Normale x, y (Kamera) */
	signed short angle;		/* +0x0a Normale z in Kamerakoordinaten = Ausrichtung zur Kamera (Schattierung, Klickauswahl) */
	signed char f0c[2];		/* +0x0c Kopie von face3d +0x0e (R25, unbenutzt) */
	signed long event;		/* +0x0e Ereignis der Klickflaeche (struct map_event als long) */
};

struct rect {				/* 8 Bytes */
	signed short x0, y0, x1, y1;
};

struct map_event {			/* 4 Bytes, Ereignis an einer Kartenzelle (-1L = keins) */
	signed short type;		/* -2 = Zelle betreten (R12 cell_event) */
	unsigned char x, y;		/* Achtung (N8): R12 cell_event schreibt nach x die Zeile (g_9496) und nach y die
					   Spalte (g_9498); O183/O249 lesen +2 als Zeile, +3 als Spalte - Namen vermutlich
					   vertauscht (R12 view_frame setzt g_dng16_key_event1 dagegen x = g_map_x0 + 3) */
};

/* --- Textbildschirm (0x18 Bytes, Code O181): Basis der Bildfolgen mit Texten (O039/O040/O041 Intro/Outro, O166).
 * Sichert den Zustand der Schrift g_font, laedt eine Textliste und schreibt Zeilen ueber einen frei geraeumten
 * Streifen am unteren Bildrand. Freie Funktionen tscr_* (C-Modul O181, in den C++-Modulen bisher Win::open ...). */
struct text_screen {
	signed short old_box[4];	/* +0x00 Schriftkasten vor dem Oeffnen (font_get_box) */
	signed short old_bg;		/* +0x08 Schriftfarben vor dem Oeffnen (font_get_colors: fg, bg) */
	signed short old_fg;		/* +0x0a */
	void far *txt_buf;		/* +0x0c Textliste (txt_load_list) */
	char far * far *txt;		/* +0x10 */
	signed char _r14[4];
};

struct cue_line {			/* 6 Bytes: Untertitel und Sprachnummer (O040 g_anim3_lines) */
	char far *text;			/* +0 (zur Laufzeit eingetragen) */
	signed short snd;		/* +4 Sprachnummer, 0 = Ende */
};

struct cue_lines16 {			/* 0x60 Bytes, als Ganzes auf den Stack kopiert */
	struct cue_line e[16];
};

struct outro_line {			/* 10 Bytes: Zeile der Abspannfolgen (O041) */
	char far *text;			/* +0 (zur Laufzeit eingetragen) */
	signed short snd;		/* +4 Sprachnummer */
	signed short ani;		/* +6 Hintergrund-Animation (0x3b5..0x3b8) */
	signed short change;		/* +8 1 = nach der Zeile zur Animation der naechsten Zeile wechseln */
};

struct outro_lines9 {
	struct outro_line e[9];
};

struct outro_lines10 {
	struct outro_line e[10];
};

struct repair_item {			/* 7 Bytes: Eintrag der Auswahlliste der Schmiede (O230/O231; BE SELLITEMS) */
	signed short item_id;		/* +0 Gegenstand, 0 = Ende */
	signed short price;		/* +2 Preis je Einheit */
	signed short unit;		/* +4 Preiseinheit (item_desc.price_unit: 1, 10 ...) */
	signed char slot;		/* +6 Inventarplatz */
};

struct smith_repair {			/* 9 Bytes: Gegenstand in Reparatur je Schmiede, g_smith_repairs[g_loc_id] (O230/O231) */
	signed short item_id;		/* +0 Gegenstand, 0 = keiner */
	signed char since[7];		/* +2 Abgabezeitpunkt (date_now / date_since, O250) */
};

/* --- Schenke (O240 class Tavern, Methoden auch als freie Funktionen tavern_* in O241/O242; BE seg059 do_tavern) --- */
struct tavern {				/* 0x1f Bytes, Zeiger g_tavern */
	void far *txt_buf;		/* +0x00 Texte 0x1e (0xa0 Stueck) */
	char far * far *txt;		/* +0x04 */
	signed short price_mod;		/* +0x08 Preisaufschlag in % (7-Byte-Satz g_loc_id der Datei 0x1f4: +08..+0e) */
	signed short quality;		/* +0x0a Stufe 0..20 (klein = vornehm: Preis 6 - quality / 4; <= 5 / <= 15 / sonst) */
	signed char female;		/* +0x0c Wirtin (Texte 0x80/0x82 statt 0x7f/0x81) */
	signed short event_chance;	/* +0x0d Chance in % fuer die stuendliche Einladung (tavern_hourly_event) */
	signed short done;		/* +0x0f Schenke verlassen */
	signed short hours;		/* +0x11 geoeffnet: 1 = 11..3 Uhr (Stufe 6..13), 2 = 16..3 Uhr, 0 = geschlossen */
	signed short bill;		/* +0x13 offene Zeche (tavern_add_bill, tavern_pay_bill) */
	signed short seat;		/* +0x15 Platz 1..3 (Menue txt[8..10]; 3 = bei den Gaesten: Geruechte/Personen) */
	signed short drinks;		/* +0x17 Trinkrunden: Zechprobe erschwert um drinks * 5 - 20 */
	signed short seat_changes;	/* +0x19 Platzwechsel bei Andrang (3/4 Warnung, 5 Rauswurf) */
	signed short rounds;		/* +0x1b ausgegebene Runden (tavern_round; > 3 eigene Texte) */
	signed short script;		/* +0x1d Gespraechsskript (script_load), -1 = keins */
};

struct intro_scene {			/* 4 Bytes, g_intro_scenes[] (O039 Vorspann) */
	signed short ani;		/* +0 Animationsdatei (Anim) */
	signed short pic;		/* +2 Bild in der NVF-Datei 0x346 */
};

struct area_desc {			/* 8 Bytes, g_area_descs[] (ds:5d9c) je Dungeon: Vorgabe fuer area_events.desc */
	signed short (far *handler)(void far *, void far *);	/* +0 Rueckruf (Objekt, struct map_event far *), R12 call_handler */
	signed char _r4[4];
};

struct area_events {			/* 0x12 Bytes, Ereignisobjekt eines Dungeons (O129; O014 legt es an, g_area_events) */
	void far *txt_buf;		/* +0x00 Ereignistexte (txt_load_list) */
	char far * far *txt;		/* +0x04 */
	signed short type;		/* +0x08 Dungeon (group.dungeon) */
	struct area_desc desc;		/* +0x0a Kopie von g_area_descs[type] */
#ifdef __cplusplus
	/* Methoden (O129); freie Namen Event_ctor, event_set_cell ... in s_proto.h */
	area_events(signed short type, signed short txt_file, signed short txt_n);	/* O129_0000 Event_ctor */
	~area_events();									/* O129_02A8 Event_dtor */
	void set_cell(signed short y, signed short x, signed char type, signed short flags, signed short rot);
	signed short fight1(signed short id, signed short n, signed short c);
	signed short fight2(signed short id, signed short n, signed short c, signed short id2, signed short n2,
		signed short c2);
	signed short fight(signed short id, signed short n, signed short c, signed short d,
		signed short id2, signed short n2, signed short c2, signed short d2,
		signed short id3, signed short n3, signed short c3, signed short d3);
	void set_cell_bits(signed short y, signed short x, signed short bits);
	void teleport(signed short from);
	void notice(char far *what);
	void dragon_claw(struct hero far *h);	/* O129_06AF event_dragon_claw (frueher creature) */
#endif
};

struct route {				/* 10 Bytes, Weg der Reisekarte: g_routes[0x106] (*ds:c7d0, Datei 0xa2) */
	unsigned char from;		/* +0 Ort */
	unsigned char to;		/* +1 Ort */
	unsigned char dir_from;		/* +2 Richtung 0..7 am Ort from (Richtungsbit O223; Begegnungsstufe O212) */
	unsigned char dir_to;		/* +3 Richtung am Ort to */
	signed char terrain;		/* +4 Gelaende -> travel.terrain */
	signed char road;		/* +5 Wegzustand -> travel.road */
	unsigned char len;		/* +6 Laenge (x 1000) -> travel.route_len */
	signed char danger;		/* +7 Zuwachs travel.danger (O215) */
	signed char event;		/* +8 Wegereignis tevent_run(Weg): 1..49 nach 1/2, ab 50 nach 1/30 und 29/30 (O215/O216) */
	signed char known;		/* +9 Weg bekannt (O209, O223) */
};

struct travel_place {			/* 3 Bytes, Ort der Reisekarte: g_places[] (*ds:c7cc, Datei 0xa1) */
	unsigned char id;		/* +0 Ort */
	unsigned char dirs;		/* +1 bekannte Richtungen (Bits, O219/O223) */
	unsigned char dirs_all;		/* +2 alle Richtungen (place_reveal_dirs: +2 -> +1) */
};

struct dir_bits {			/* 1 Byte: Richtungen 1..8 als Bits (unsigned short b:1 -> Bytezugriff, O214/O219/O223);
					   Bit k = Richtung k + 1 (travel.dir_sel), Lage der Teilbilder in rose_compose (O220): */
	unsigned short d0:1;		/* Norden */
	unsigned short d1:1;		/* Nordosten */
	unsigned short d2:1;		/* Osten */
	unsigned short d3:1;		/* Suedosten */
	unsigned short d4:1;		/* Sueden */
	unsigned short d5:1;		/* Suedwesten */
	unsigned short d6:1;		/* Westen */
	unsigned short d7:1;		/* Nordwesten */
};

struct dir_set {			/* 3 Bytes: struct travel_place mit Bitsicht (Travel.place, O220 f_095f als Wert) */
	unsigned char id;
	struct dir_bits dirs;		/* +1 bekannte Richtungen */
	struct dir_bits dirs_all;	/* +2 */
};

/* --- Haendler (O226..O229, Klasse Shop in s_class.h) ------------------------------------------------- */
struct shop_info {			/* 13 Bytes, Ladendaten: Datensatz g_loc_id der Datei 0x1f3, Marktstand ds:c0ee */
	signed short markup;		/* +0 Aufschlag in % (Waren ohne food) */
	signed short markup_food;	/* +2 Aufschlag in % fuer Nahrung (item_desc.food) */
	signed char type;		/* +4 Art SHOP_WEAPONS/SHOP_HERBS/SHOP_GENERAL */
	signed char lvl;		/* +5 Stufe: Waren mit item_desc.f0b >= lvl */
	signed short extra[3];		/* +6 drei Zusatzwaren */
	signed char female;		/* +0xc Haendlerin (Texte/Bilder) */
};

struct ware {				/* 7 Bytes, Warenliste des Ladens (shop.wares, g_nr_items Eintraege) */
	signed short id;		/* +0 Gegenstand, 0 = leer */
	signed short price;		/* +2 Preis je Einheit */
	signed short unit;		/* +4 Preiseinheit (item_desc.price_unit: 1, 10, sonst) */
	signed char slot;		/* +6 Verkauf: Inventarplatz des Helden; Kauf: -1 */
};

struct route_list42 { signed short r[42]; };	/* 0x54 Bytes: Wegnummern (g_water_routes, als Strukturkopie O215) */

struct market {				/* 11 Bytes, g_markets[] (ds:2a74) je Ort: Vorlage fuer den Marktstand (Laden 0x18) */
	signed short markup;		/* +0 Aufschlag Kauf/Verkauf in % */
	signed char _r2[2];
	signed char lvl;		/* +4 Stufe */
	signed char _r5;
	signed char female[3];		/* +6 Haendlerin je Stand 1..3 */
	signed char _r9[2];
};

struct view_area {			/* 18 Bytes, Beschreibung einer Stadt bzw. eines Dungeons fuer die 3D-Ansicht:
					   g_town_areas[] (ds:4c25, O046), g_dng_areas[] (ds:0bbd, O014, O143) - frueher loc_desc */
	signed short map;		/* +0x00 Kartendatei (load_map_all, Automap) */
	signed short gfx;		/* +0x02 Grafik (g_mem_saving == 0) */
	signed short gfx2;		/* +0x04 Grafik bei g_mem_saving */
	signed short notes;		/* +0x06 Notizdatei der Automap */
	signed short pal_pic;		/* +0x08 Palettenbild */
	signed short tiles;		/* +0x0a Kacheldatei der Automap */
	signed short music;		/* +0x0c Musik */
	struct view_obj far *objs;	/* +0x0e Objektliste (O097 view_load_objects) */
};

struct encounter {			/* 4 Bytes, Gegnergruppe einer Begegnung, g_encounters[3] (ds:666b; O072, O129) */
	signed char id;			/* +0 Gegnerart (O072: aus der Tabelle ds:5884) */
	signed char n;			/* +1 Anzahl */
	signed char f2;			/* +2 */
	signed char f3;			/* +3 */
};

struct cell_xy {			/* 4 Bytes, Zellkoordinaten (R13 Profil-Cache) */
	signed short x, y;
};

/* --- Karte 64 x 64 (R17 Kartenobjekt, R19 Zellzugriff, R12/R13 3D-Ansicht) ----------------------------- */
struct map_cell {			/* 4 Bytes, Zelle der Zellentabelle g_cells[64][64] (R17 build_cells) */
	unsigned char type;		/* +0 Feldtyp = Index in g_cell_objs (get_cell_a) */
	signed short flags;		/* +1 CELL_* (get_cell_b) */
	unsigned char rot;		/* +3 Drehung des Zellobjekts 1..8 (get_cell_c: Winkel rot << 8) */
};

struct tile_info {			/* 3 Bytes, Kachelbeschreibung je Feldnummer (R17 load_tiles) */
	unsigned char type;		/* +0 Feldtyp -> map_cell.type */
	signed char flags;		/* +1 TILE_BLOCKED, Bits 0..2 Drehung (TILE_ROT_MASK) */
	unsigned char kind;		/* +2 TILE_KIND_WALL0 -> CELL_WALL0 */
};

struct map_grid {			/* 0x10 Bytes, Kartenobjekt (R17 new_map), z. B. ds:b656 */
	signed short w;			/* +0x00 Breite (64) */
	signed short h;			/* +0x02 Hoehe (64) */
	signed short f04, f06;		/* +0x04 */
	unsigned short far *data;	/* +0x08 Feldnummern w x h (Wort je Feld, Low-Byte = Nummer) */
	struct tile_info far *tiles;	/* +0x0c Kacheltabelle (nur waehrend load_map_all) */
};

struct mem_block {			/* 8 Bytes, g_mem_blocks[1000] (R07): belegter Block mit Waechterwoertern */
	unsigned char far *start;	/* +0 Blockanfang ("GUYS"), 0 = Eintrag frei */
	unsigned char far *end;		/* +4 hinteres Waechterwort ("GUYE") */
};

struct snd_buf {			/* 12 Bytes, Miles AIL sound_buff (an den DSP-Treiber uebergeben) */
	unsigned short pack_type;	/* +0 */
	unsigned short sample_rate;	/* +2 */
	unsigned char huge *data;	/* +4 Daten im Puffer g_snd_buf0/1 */
	unsigned long len;		/* +8 Laenge */
};

struct ail_drv_desc {			/* 0x18 Bytes, Miles AIL drvr_desc (AIL_describe_driver; O042, O043, O232) */
	signed short min_api;		/* +0x00 */
	signed short type;		/* +0x02 AIL_DSP_DRVR (2) / AIL_XMIDI_DRVR (3) */
	char suffix[4];			/* +0x04 Endung der Klangdaten ("AD", "OPL", "MT"; Sound::mtype) */
	char far *dev_names;		/* +0x08 Geraetenamen, je mit 0 abgeschlossen (Sound::drvname) */
	signed short io;		/* +0x0c Vorgabe Port */
	signed short irq;		/* +0x0e Vorgabe IRQ */
	signed short dma;		/* +0x10 Vorgabe DMA */
	signed short drq;		/* +0x12 Vorgabe DRQ */
	signed short service_rate;	/* +0x14 */
	unsigned short display_size;	/* +0x16 */
};

struct sound {				/* 0x89 Bytes, Klangobjekt (R04, O042): g_snd_music, g_snd_fx, g_snd_speech */
	signed short drv;		/* +0x00 AIL-Treiber-Handle (1 = Sprachausgabe moeglich) */
	signed short seq[8];		/* +0x02 XMIDI-Sequenz-Handles (-1 = frei) */
	struct ail_drv_desc far *desc;	/* +0x12 Treiberbeschreibung */
	void far *drv_image;		/* +0x16 geladener Treiber (Sound::load, AIL_register_driver) */
	void far *seq_state[8];		/* +0x1a Zustandspuffer je Sequenz (Groesse state_size; Typ 3) */
	void far *timbre_cache;		/* +0x3a Klangfarben-Cache (Groesse cache_size; Typ 3) */
	void far *load_buf;		/* +0x3e Ladepuffer (snd_exists; Typ 3: Sequenzdaten, Sound::music) */
	void far *drv_buf;		/* +0x42 Puffer der Treiberdatei (Sound::load, nie freigegeben) */
	signed char src;		/* +0x46 SND_SRC_FILES (1) / SND_SRC_ARCHIVE (2) */
	unsigned long state_size;	/* +0x47 Groesse eines Sequenz-Zustandspuffers (Typ 3) */
	unsigned short cache_size;	/* +0x4b Groesse des Klangfarben-Caches (Typ 3) */
	signed short disabled;		/* +0x4d kein Treiber */
	signed short enabled;		/* +0x4f Klang eingeschaltet */
	signed short music_nr;		/* +0x51 geladenes Musikstueck (-1 = keins; Sound::play) */
	void far *file;			/* +0x53 Dateiobjekt der Klaenge (O037) */
	void far *file_speech;		/* +0x57 Dateiobjekt der Sprachausgabe */
	void far *arch;			/* +0x5b Archivobjekt (R06) */
	signed short buf;		/* +0x5f aktueller Puffer 0/1 */
	signed short playing;		/* +0x61 */
	signed short id;		/* +0x63 laufender Klang (SND_ID_*-Bits) */
	signed short id_req;		/* +0x65 zuletzt angeforderter Klang */
	unsigned char f67[2];		/* +0x67 */
	unsigned long remain;		/* +0x69 noch zu ladende Bytes */
	unsigned long pos;		/* +0x6d Leseposition */
	struct snd_buf sbuf;		/* +0x71 */
	struct snd_buf sbuf2;		/* +0x7d */
#ifdef __cplusplus
	void set_enabled(signed short v) { enabled = v; }	/* O106 (Parameterkopie) */
#endif
};

/* --- Zeichenketten als Strukturkopie (Vorbelegung lokaler Puffer per SCOPY wie im Original) ----------- */
struct str3 { char s[3]; };		/* R14 g_str_crlf */
struct str5 { char s[5]; };		/* R05 g_ltx_count_init */
struct str9 { char s[9]; };		/* O010 g_str_emm */
struct str80 { char s[80]; };		/* R05 g_path_init */

struct video_state {			/* 8 Bytes, R43: beim Start gesicherter Videozustand (g_video_state) */
	signed short page;		/* +0 Bildschirmseite */
	signed short mode;		/* +2 Videomodus */
	signed short cols;		/* +4 Spalten */
	signed short rows;		/* +6 Zeilen */
};

/* --- Maus (R18, R23 mouse_isr) ------------------------------------------------------------------------ */
struct mouse_area {			/* 10 Bytes, Mausbereichsliste (Ende: x0 == -1), get_click_action/get_mouse_action */
	signed short x0, y0, x1, y1;	/* +0 Rechteck */
	signed short action;		/* +8 Aktion (-> g_action) */
};

struct cursor_areas {			/* 72 Bytes: Mausbereiche der 3D-Ansicht (g_view_cursor_areas, R12 set_cursor) */
	struct mouse_area a[7];
	signed short end;		/* -1 */
};

/* --- Mausbereiche der Portraetleiste (select_hero_bar, O217), Strukturkopie ------------------------- */
struct hero_bar_areas {			/* 80 Bytes, ds:7204 */
	struct mouse_area a[8];
};

struct mouse_bg {			/* 12 Bytes, g_mouse_bg (ds:8b97): gesicherter Hintergrund unter dem Zeiger */
	signed short x, y;		/* +0 Lage */
	signed short cw, ch;		/* +4 gesicherte Breite/Hoehe (am Rand beschnitten); eigene Publics */
	signed short w, h;		/* +8 Zeigergroesse = g_cursor_w/g_cursor_h (eigene Publics) */
};

/* --- Schrift (R20, Objekt g_font ds:37e4; Klasse aus O248) ------------------------------------------- */
struct glyph {				/* 4 Bytes, Zeichen im Zeichensatz (font.glyphs[Zeichencode]) */
	signed short off;		/* +0 Spalte im Zeichensatzbild */
	signed char idx;		/* +2 Nummer in der Zeichentabelle */
	unsigned char width;		/* +3 Breite */
#ifdef __cplusplus
	void set(signed short o, signed char i, unsigned char w) { off = o; idx = i; width = w; }	/* R20 font_init */
#endif
};

struct font_char {			/* 4 Bytes, Zeichensatzdatei: Eintrag je Zeichen */
	unsigned char f0;
	unsigned char w;		/* +1 Breite */
	unsigned char f2;
	unsigned char h;		/* +3 Hoehe */
};

struct font_file {			/* Zeichensatzdatei: Zahl der Eintraege, Eintraege, danach die Bitmuster */
	signed short n;
	struct font_char e[1];
};

struct colset {				/* 5 Bytes, Farbsatz: Vordergrund + 4 Zusatzfarben (Steuerzeichen 0xf1..0xf4) */
	unsigned char fg, c1, c2, c3, c4;
};

struct font {				/* 0x49 Bytes, gepackt (Felder an ungeraden Offsets) */
	signed char f00[4];
	struct font_file far *file;	/* +0x04 Zeichensatzdatei */
	struct glyph far * far *glyphs;	/* +0x08 je Zeichencode (0 = nicht im Zeichensatz) */
	void far *bg_pic;		/* +0x0c Hintergrundbild fuer font_rect */
	unsigned char far *screen;	/* +0x10 Zielpuffer (SCREEN_W breit); == g_vga_buf -> Maus aus/ein */
	signed short max_w;		/* +0x14 groesste Zeichenbreite */
	signed short max_h;		/* +0x16 groesste Zeichenhoehe = Hoehe der Bitmuster */
	signed short x;			/* +0x18 Schreibposition */
	signed short line_h;		/* +0x1a Zeilenabstand */
	unsigned char bits_start;	/* +0x1c Index in file->e[], ab dem die Bitmuster beginnen */
	unsigned char transp;		/* +0x1d 1 = transparenter Hintergrund */
	unsigned char bg;		/* +0x1e Hintergrundfarbe, zugleich Farbtabelle [colsel] */
	struct colset cs;		/* +0x1f */
	struct colset save;		/* +0x24 gesicherter Farbsatz (font_push_colset) */
	unsigned char colsel;		/* +0x29 Farbtabellen-Versatz (0 bzw. 0x01..0x04 + 0xf0 ueber Steuerzeichen) */
	unsigned char shadow;		/* +0x2a Schatten */
	unsigned char old_bg;		/* +0x2b vorige Farben (font_restore_colors) */
	unsigned char old_fg;		/* +0x2c */
	signed short indent;		/* +0x2d Einzug */
	signed short indent_line;	/* +0x2f Einzug bis zu dieser Zeile */
	signed short align;		/* +0x31 1 = zentriert */
	signed short box_w;		/* +0x33 Textkasten: Breite */
	signed short box_x;		/* +0x35 */
	signed short box_y;		/* +0x37 */
	signed short y;			/* +0x39 Schreibposition */
	signed short tabs[7];		/* +0x3b Tabulatorpositionen */
};

/* --- NVF-Bilddatei: Kopf nach Typbyte und Bildzahl (O098 Nvf, R41 nvf_desc) ----------------------------- */
struct nvf_dim {			/* 4 Bytes, NVF_FILE_RAW (einmal) / NVF_FILE_RAW_SIZES (je Bild) */
	unsigned short w;
	unsigned short h;
};

struct nvf_dims_sizes {			/* NVF_FILE_PACKED/_PACKED2: Masse, dann Groesse je Bild */
	unsigned short w;
	unsigned short h;
	signed long size[1];
};

struct nvf_dim_size {			/* 8 Bytes, NVF_FILE_PACKED_SIZES/_SIZES2: je Bild Masse und Groesse */
	unsigned short w;
	unsigned short h;
	signed long size;
};

/* --- Gegenstandsablage (O238 Stash, Arbeitsdatei 0x2c6) ----------------------------------------------- */
struct stash_entry {			/* 4 Bytes */
	signed short id;		/* +0 Gegenstand (ITEM_MONEY = Geld in Hundertern), <= 0 = frei */
	unsigned char n;		/* +2 Anzahl */
	signed char _r3;
};

/* --- Lamellen-Einblendung (R15 fade_in_pic) ---------------------------------------------------------- */
struct fade_state {			/* 12 Bytes */
	signed short step;		/* +0x00 Durchgang 0..7 */
	signed short off_even;		/* +0x02 Lamellenversatz gerader Zeilen (0..7, steigend) */
	signed short off_odd;		/* +0x04 Lamellenversatz ungerader Zeilen (7..0, fallend) */
	signed short w_new;		/* +0x06 Pixel je 8er-Block aus dem neuen Bild (0..8) */
	signed short w_old;		/* +0x08 Pixel je 8er-Block aus dem alten Bild (8..0) */
	signed short tick;		/* +0x0a Teiler fuer w_new/w_old (hier jeder Durchgang) */
};

struct timestamp {			/* 7 Bytes: Zeitpunkt (O250 date_now/date_set/date_since; Lagerhaus O012/O099/O100) */
	signed long ticks;		/* +0x00 Tageszeit in Ticks */
	signed char day;		/* +0x04 */
	signed char month;		/* +0x05 */
	signed char year;		/* +0x06 */
};

/* --- Spielzeit (R21): Datumsobjekt g_date (ds:b856; nicht "date": dos.h) ---------------------------------------------------- */
struct game_date {				/* 0x0c Bytes */
	signed char special;		/* +0x00 besonderer Tag (g_special_days[].id), 0 = keiner */
	signed long ticks;		/* +0x01 Tageszeit in Ticks 0..TICKS_PER_DAY-1 (= g_daytime) */
	signed char weekday;		/* +0x05 Wochentag 0..6 */
	signed char week;		/* +0x06 Woche im Monat */
	signed char day;		/* +0x07 Tag 1..30; Namenlose Tage -5..-1 (0 = Jahreswechsel) */
	signed char month;		/* +0x08 Monat 1..12, 13 = Namenlose Tage */
	signed char year;		/* +0x09 Jahr */
	signed short week_reset;	/* +0x0a Monat endete mitten in der Woche: naechste Woche ist Woche 1 */
};

struct special_day {			/* 3 Bytes, g_special_days[] (Ende: month == -1) */
	signed char month;
	signed char day;
	signed char id;			/* -> date.special */
};

struct rgb {				/* 3 Bytes, Palette (R02 set_color) */
	unsigned char r, g, b;
};

struct palette {			/* 0x302 Bytes: Palette mit Farbanzahl (O096 pal_set_rgb, O097 tex_load) */
	signed short n;
	struct rgb c[256];
};

struct saver {				/* 6 Bytes, Bildschirmschoner (Code O224, frueher Klasse Tm; g_saver ds:cb2e) */
	unsigned long idle_limit;	/* +0 Leerlaufgrenze (Stufe g_saver_level x 1080 Ticks) */
	signed short busy;		/* +4 Animation laeuft (R02 screensaver_run) */
#ifdef __cplusplus
	saver();				/* O224_0000 Konstruktor: busy = 0 (globales Objekt g_saver, O224) */
	void tick();				/* O224_0031 saver_tick: Leerlauf pruefen, g_saver_due setzen */
	signed short set_level(signed char v);	/* O224_00D0 saver_set_level: Stufe 0..3, alte zurueck */
	void anim();				/* O224_0102 saver_anim: Kachel-Animation bis Taste/Maus */
	signed short wait(signed short n);	/* O224_0259 saver_wait: n x poll, 1 = beendet */
	void poll();				/* O224_0290 saver_poll: Tastatur/Maus/Klangdienste */
#endif
};

struct bars {				/* 0x15 Bytes, Balkenanzeige der Portraetleiste (Klasse Bars, O057), g_bars */
	unsigned char blink1;		/* +0x00 Blinkzaehler 1 */
	unsigned char phase1;		/* +0x01 Blinkphase 1 */
	unsigned char blink2;		/* +0x02 */
	unsigned char phase2;		/* +0x03 */
	signed short active;		/* +0x04 Anzeige aktiv (R02 bars_refresh) */
	signed short col[2];		/* +0x06 Farben */
	signed short col2[2];		/* +0x0a */
	signed char drawn[7];		/* +0x0e leerer Platz schon geloescht (R02 bars_clear_slot) */
};

struct mem_move {			/* 16 Bytes, XMS-/EMS-Kopierauftrag (R02 EMS_copy, R06 g_xms_move fuer xms_move_any); signed: R06 braucht es (shl/shrd-Form) */
	signed long len;		/* +0x00 Laenge */
	signed short src_handle;	/* +0x04 Quelle: Handle, 0 = konventioneller Speicher */
	signed long src;		/* +0x06 Quelle: Adresse (seg:off) bzw. Offset im Block */
	signed short dst_handle;	/* +0x0a Ziel: Handle, 0 = Speicher */
	signed long dst;		/* +0x0c Ziel */
};

struct arc_entry {			/* 8 Bytes, XMS-Inhaltsverzeichnis des Archivs je Datei (R06) */
	signed long off;		/* +0 Position im XMS */
	signed long len;		/* +4 Laenge, Bit 31 = temporaer (seit arc_mark) */
#ifdef __cplusplus
	void set(signed long o, signed long l) { len = l; off = o; }
	int is_temp() { return (len & 0x80000000) != 0; }
#endif
};

struct arc_stats {			/* 9 Bytes, Ergebnis von arc_info */
	signed long total;		/* +0 XMS gesamt */
	signed long used;		/* +4 XMS belegt */
	signed char xms;		/* +8 XMS benutzt */
};

struct arc {				/* 0x5c Bytes, Archivobjekt (R06), z. B. g_file_3 */
	char far * far *names;		/* +0x00 Dateinamentabelle */
	void far *buf;			/* +0x04 Puffer 4 KB fuers XMS-Kopieren */
	struct arc_entry far *tab;	/* +0x08 XMS-Inhaltsverzeichnis */
	signed short far *blocks;	/* +0x0c XMS-Handles (128-KB-Bloecke) */
	signed long xpos;		/* +0x10 Fuellstand im XMS */
	signed long pos;		/* +0x14 Leseposition */
	signed short nblk;		/* +0x18 aktueller XMS-Block */
	unsigned short handle;		/* +0x1a 0x8000|id XMS, 0x4000|fd Archiv, sonst DOS-Handle */
	signed char xms;		/* +0x1c XMS benutzt */
	signed char mark;		/* +0x1d arc_mark aktiv */
	signed short save_blk;		/* +0x1e Zustand bei arc_mark */
	signed long save_xpos;		/* +0x20 */
	signed long save_pos;		/* +0x24 */
	char far *arcname;		/* +0x28 Name der Archivdatei */
	char far *bitmap;		/* +0x2c Bitfeld "Datei liegt einzeln vor" */
	char name[14];			/* +0x30 Namenspuffer */
	signed short have_arc;		/* +0x3e Archivdatei vorhanden */
	signed long len;		/* +0x40 Laenge der Datei im Archiv */
	signed long remain;		/* +0x44 noch zu lesen */
	signed long start;		/* +0x48 Anfang im Archiv */
	signed long end;		/* +0x4c Ende im Archiv */
	signed short nfiles;		/* +0x50 */
	signed short nblocks;		/* +0x52 */
	char far *dir;			/* +0x54 Namensdatei */
	signed char _r58[4];
};

struct nvf_desc {			/* 0x19 Bytes, NVF-Entpacker (R41 nvf_new/nvf_unpack; BE struct nvf_desc); gleicher
					 * Aufbau wie die Klasse Nvf (O098), die direkt aus der Datei liest */
	unsigned char far *buf;		/* +0x00 Zeilenpuffer NVF_ROW_BUF Bytes (Spiegeln) */
	unsigned char far *dst;		/* +0x04 Ziel */
	unsigned char huge *src;	/* +0x08 NVF-Datei im Speicher */
	signed short nr;		/* +0x0c Bildnummer (wird auf 0..Bildzahl - 1 begrenzt) */
	signed short stride;		/* +0x0e Zeilenabstand im Ziel (NVF_OUT_RLE, NVF_OUT_RLE_MIRROR) */
	signed char type;		/* +0x10 Ausgabeart NVF_OUT_* */
	unsigned short far *width;	/* +0x11 Ergebnis: Breite */
	unsigned short far *height;	/* +0x15 Ergebnis: Hoehe */
};

/* ======================================================================================================
 * Welle N17: Dialoge (O101/O102/O013, DSA1 do_talk), Gespraeche mit Themen (O049/O257/O260), Tagebuch (O103..O105)
 * ====================================================================================================*/
/* --- Dialog nach DSA1 do_talk/load_tlk: Zustandsautomat aus einer Dialogdatei (O101 talk), Sonderfunktionen
 * talk_special (O102) und talk_* (O013). Objekt g_talk ds:c918 (bzw. lokal). Dateiaufbau: Wort Partnerzahl, je
 * Partner struct talk_hdr, je Partner Wort n + n Zustaende, Textblock (Wort Laenge, Wort Textzahl, Texte). */
struct talk_state {			/* 8 Bytes, Zustand */
	signed short txt;		/* +0 Text (ab base), Bit 15 = Sonderfunktion talk_special, -1 = nur Sonderfunktion */
	unsigned char ans[3];		/* +2 Antworttexte (ab base, 0 = keine Antwort) */
	unsigned char next[3];		/* +5 Folgezustand je Antwort (0xff = Ende) */
};

struct talk_hdr {			/* 12 Bytes, Kopf je Partner in der Dialogdatei */
	signed short f0;
	signed short f2;
	signed short base;		/* +4 erster Text des Partners */
	signed short title;		/* +6 Titeltext (-1 = leer) */
	signed short pic;		/* +8 Bild (load_portrait_buf) */
	signed short esc_ends;		/* +a Abbruch beendet den Dialog */
};

struct talk_desc {			/* 0x26 Bytes, laufender Dialog */
	signed short partner;		/* +0x00 Gespraechspartner (Nummer in der Datei) */
	signed short state;		/* +0x02 aktueller Zustand */
	signed short next;		/* +0x04 Folgezustand aus talk_special (-1 = nach Antwort) */
	signed short id;		/* +0x06 Dialogdatei */
	signed short esc_ends;		/* +0x08 Abbruch (-1) beendet den Dialog, sonst Folgezustand 1 */
	signed short done;		/* +0x0a Ende */
	char far *buf;			/* +0x0c Textpuffer 3000 Bytes (talk_format) */
	char far *src;			/* +0x10 Text des Zustands */
	char far *data;			/* +0x14 Textblock der Datei */
	char far * far *txt;		/* +0x18 Zeigertabelle der Texte */
	signed short title;		/* +0x1c Titeltext (-1 = leer) */
	signed short pic;		/* +0x1e Bild */
	signed short base;		/* +0x20 erster Text des Partners */
	struct talk_state far *states;	/* +0x22 Zustaende des Partners */
};

/* Rueckruf eines Gespraechs (g_conv_callbacks, Conv.callback): (0, Thema) -> Bedingung, (1, Wert) nach der Antwort */
typedef signed short (far *CONV_CB)(signed short mode, signed short v);

struct names6 {				/* 0x60 Bytes: 6 Namen zu 16 Bytes (talk_choose_hero, Strukturkopie) */
	char n[6][16];
};

struct heroes6 {			/* 0x18 Bytes: 6 Heldenzeiger */
	struct hero far *h[6];
};

struct talk_tpl {			/* ds:0636: Vorlagen fuer O013 (nur +0 hat ein eigenes Public) */
	signed short pics_end;		/* +0x00 leere Bildliste (load_pic_list in talk_reward) */
	struct names6 names;		/* +0x02 leere Namen (ds:0638) */
	struct heroes6 heroes;		/* +0x62 Nullzeiger (ds:0698) */
};

struct sel_rec {			/* 0x17 Bytes, Datensatz der Datei 0x334 (Sel O225; Namensmarken im Tagebuch O104/O105) */
	signed char id;			/* +0x00 Begriff, -1 = Ende */
	signed char avail;		/* +0x01 verfuegbar (sonst nicht in der Liste) */
	signed char lvl;		/* +0x02 Stufe (Kapitel; im Tagebuch die Marke g_diary_tag[i]) */
	char name[20];			/* +0x03 */
};

/* --- Tagebuch (O103..O105): Objekt g_diary ds:c0c3 (0x2b Bytes); C++: class Diary (s_class.h, Code O103/O104),
 * C-Modul O105 mit struct diary und freien Funktionen diary_*(DIARY_PTR). Datei DIARY.DAT (Arbeitsdatei 0 in g_ofile):
 * Wort Seitenzahl, Byte Kapitel, je Seite long Laenge + verschluesselter Text (diary_encrypt). Steuerzeichen im
 * Text: g_diary_ctrl (Kopf/Eintrag/eigener Eintrag/Ende), Namensmarken g_diary_tag + Kennung (Datei 0x334). */
struct diary {
	unsigned short n_pages;		/* +0x00 Seitenzahl */
	unsigned short page;		/* +0x02 aktuelle Seite */
	char far *text;			/* +0x04 Seitentext (1000 Bytes) */
	char far *buf;			/* +0x08 Arbeitspuffer (1000 Bytes) */
	char far *txt_buf;		/* +0x0c Texte 0x331 */
	char far * far *txt;		/* +0x10 */
	signed char _r14[4];
	signed short lines;		/* +0x18 Druck: Zeilen auf der Seite */
	signed char find_tag;		/* +0x1a Suche: Kennung bzw. -1 = eigene Eintraege */
	signed char find_id;		/* +0x1b Suche: Begriff */
	signed char chapter;		/* +0x1c Kapitel der letzten Seite (Tag des Datums) */
	signed char chapters[8];	/* +0x1d Suchmenue: Liste der Kapitel (diary_chapters) */
	signed short refs;		/* +0x25 Zaehler fuer diary_open/diary_close */
	signed short done;		/* +0x27 Ende der Tagebuch-Schleife */
	signed short lock;		/* +0x29 > 0: Eintraege erlaubt (diary_lock/diary_unlock) */
};
#ifdef __cplusplus
class Diary;
#define DIARY_PTR Diary far *		/* Tagebuch (O103..O105) */
#else
#define DIARY_PTR struct diary far *
#endif

struct names7 {				/* 7 Textzeiger (Kapitelnamen der Tagebuchsuche, Strukturkopie) */
	char far *p[7];
};

struct compass {			/* 6 Bytes, Kompass der 3D-Ansicht (R11), g_compass */
	unsigned char far *buf;		/* +0 NVF-Datei 0x3b4 (Rahmenbilder, Bild 0 = Richtungsband) */
	signed short pic_no;		/* +4 Bildnummer des Rahmens (O010: 1/2, O014: 2) */
};

struct map {				/* 0x28d Bytes, Automap-Objekt g_map (C-Sicht; C++: class Automap in s_class.h, O052/O053) */
	signed char f000[0xee];		/* +0x000 */
	signed short see_through[5];	/* +0x0ee durchsichtige Feldtypen (Woerter fuer is_in_word_array, Ende -1) */
	signed char _rf8[0x16f - 0xf8];
	signed char (far *seen)[64];	/* +0x16f Aufdeckkarte 64 x 64 (1 gesehen, 0x40 Zauber, 0x80 Standort) */
	signed char _r173[0x28d - 0x173];
};
/* ======================================================================================================
 * Helden aus DSA 1 (Schicksalsklinge), Import O176/O177 (Vorlage: Bright Eyes common.h HERO_*; 0x6da Bytes mit Bild).
 * Zwei Satzformate: dsa1_hero (Bright-Eyes-Aufbau, Bild +0x2da) und das fruehere dsa1_hero_v1 ohne zweiten Namen
 * (alle Felder ab +0x10 um 0x10 frueher, Bild +0x2ca). Nur benannt, was O176/O177 lesen.
 * ====================================================================================================*/
struct dsa1_item {			/* 0x0e Bytes (BE struct inventory) */
	signed short item_id;		/* +0x00 */
	signed char quantity;		/* +0x02 (O177 liest nur das untere Byte) */
	signed char _r03;
	unsigned short broken:1;	/* +0x04 (BE inventory_flags) */
	unsigned short half_empty:1;
	unsigned short empty:1;
	unsigned short magic:1;
	unsigned short bit4:1;
	unsigned short poison_expurgicum:1;
	unsigned short poison_vomicum:1;
	unsigned short magic_revealed:1;
	signed char _r05;
	signed char bf;			/* +0x06 */
	signed char rs_lost;		/* +0x07 */
	signed char lighting_timer;	/* +0x08 */
	signed char poison_type;	/* +0x09 */
	signed char poison_charges;	/* +0x0a */
	signed char _r0b[3];
};

struct dsa1_hero {			/* DSA-1-Held (BE-Aufbau) */
	char name[0x10];		/* +0x000 HERO_NAME (Dateiname) */
	char name2[0x10];		/* +0x010 HERO_NAME2 (angezeigter Name) */
	signed char nr_items;		/* +0x020 */
	signed char type;		/* +0x021 */
	signed char sex;		/* +0x022 */
	signed char _r23[0x7b - 0x23];
	signed char timer_id;		/* +0x07b Kennung in den Modifikations-Timern */
	signed char _r7c[0x89 - 0x7c];
	signed char npc_id;		/* +0x089 */
	signed char group_pos;		/* +0x08a 0 = nicht in der Gruppe */
	signed char _r8b[0xab - 0x8b];
	unsigned char flags2;		/* +0x0ab Bits (BE hero_flags +0xab: 4 seen_phantom, 7 encouraged) */
	signed char _rac[0x196 - 0xac];
	struct dsa1_item inv[23];	/* +0x196 */
	signed short load;		/* +0x2d8 */
	signed char pic[0x400];		/* +0x2da Portraet 32 x 32 (Amiga-Fassung 0x300 Bytes) */
};

struct dsa1_hero_v1 {			/* frueheres DSA-1-Format ohne zweiten Namen */
	char name[0x10];		/* +0x000 */
	signed char nr_items;		/* +0x010 */
	signed char type;		/* +0x011 */
	signed char sex;		/* +0x012 */
	signed char _r13[0x6b - 0x13];
	signed char timer_id;		/* +0x06b */
	signed char _r6c[0x79 - 0x6c];
	signed char npc_id;		/* +0x079 */
	signed char group_pos;		/* +0x07a */
	signed char _r7b[0x9b - 0x7b];
	unsigned char flags2;		/* +0x09b */
	signed char _r9c[0x186 - 0x9c];
	struct dsa1_item inv[23];	/* +0x186 */
	signed short load;		/* +0x2c8 */
	signed char pic[0x400];		/* +0x2ca */
};

#define DSA1_FLAGS2_SEEN_PHANTOM	0x10	/* dsa1_hero.flags2 (BE seen_phantom) */
#define DSA1_FLAGS2_ENCOURAGED		0x80	/* (BE encouraged) */

struct dsa1_timer {			/* 8 Bytes: Modifikations-Timer im DSA-1-Spielstand (BE MODIFICATION_TIMERS, 100 Stueck) */
	signed long time;		/* +0 Restzeit, 0 = frei */
	unsigned short offset;		/* +4 Versatz des Bytes im Heldensatz */
	signed char target;		/* +6 Kennung des Helden (dsa1_hero.timer_id) */
	unsigned char value;		/* +7 Betrag, der beim Import wieder abgezogen wird */
};

/* --- Welle N22: Wohnhaus (O173) ---------------------------------------------------------------------------- */
struct house_hint {			/* 3 Bytes, g_house_hints (ds:6ce4; Ende town == -1): Haus eines Bewohners mit Rat */
	signed char town;		/* +0 Stadt */
	signed char x;			/* +1 Feld */
	signed char y;			/* +2 */
};

#endif
