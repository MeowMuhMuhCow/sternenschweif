/* OVL023 - Overlay O023: Kampfsaetze und Kampffeld aus den Dateien 0x8e/0x8f (entspricht DSA1 seg027 read_fight_lst/
 * count_fight_enemies/load_fightbg/load_scenario).
 * fight_read_rec(f): Kampfsatz f->id (216 Bytes ab Fight.name, BE FIGHT.LST) aus der Arbeitsdatei 0x8e lesen,
 *   Nummer auf Anzahl - 1 begrenzt; fight_write_rec(f): zurueckschreiben (nur Nummer <= 170).
 * fight_count_monsters(f, id): belegte Gegnerplaetze (20 zu 5 Bytes) im Kampfsatz id (> 170 -> 99).
 * fight_load_bg(f): Hintergrundbild je Kampffeld-Art (f->field->type 4/6: Bild 0x3e/0x40 aus g_file_main, Bildschirm
 *   loeschen, ganze Palette; sonst Archivbild 0x4b + Art, Farben 0x60..0x7f) ueber ein lokales Pic nach f->buf und
 *   von dort per g_xms_move ins XMS bzw. EMS (Handle g_fight_xms).
 * fight_read_scenario(f): Kampffeld Satz f->scenario - 1 (0x26d Bytes) aus der Archivdatei 0x8f nach f->field.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL023.CPP   -> 983/983 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit -Og ebenso)
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH; f_01af hiess fight_show_pic)
 * Schluessel: Laengen/Positionen mit sizeof -> unsigned: "id * sizeof(struct fight_rec) + 2" ergibt movzx eax,
 *   "(n - 1) * (signed long)sizeof(struct fight_field) + 2" ergibt movsx + imul eax.
 *   Lokales Pic pic; -> Konstruktor O269_004E, Destruktor O269_00AD(&pic, 2) automatisch.
 *   Bildnummer 0x4b + (signed char)f->field->type (Feld im Header unsigned char; Original cbw).
 *   Palette ab Farbe 0x60: pic.get_palette() + 0x60 (struct rgb, 3 Bytes) = alte Rechnung "+ 0x120".
 */
#include <stdio.h>
#include <fcntl.h>
#include "schweif.h"

#define FIGHT_LST_FILE		0x8e	/* Arbeitsdatei der Kampfsaetze (BE FIGHT.LST) */
#define FIGHT_SCENARIO_FILE	0x8f	/* Archivdatei der Kampffelder (BE SCENARIO.LST) */

/* Kampfsatz in der Datei 0x8e (= Fight +0xed5..+0xfac) */
struct fight_rec {
	char name[0x13];
	signed char intro_done;
	signed short scenario;
	struct fight_monster monsters[20];
	struct fight_hero_pos heroes[7];
	signed short loot[30];
	signed short money[3];
};

void fight_read_rec(Fight far *f)
{
	signed short cnt;

	ofile_open(g_ofile, FIGHT_LST_FILE);
	ofile_read(g_ofile, &cnt, sizeof(cnt));
	if (f->id > cnt) f->id = cnt - 1;
	ofile_seek(g_ofile, f->id * sizeof(struct fight_rec) + 2, SEEK_SET);
	ofile_read(g_ofile, f->name, sizeof(struct fight_rec));
	ofile_close(g_ofile);
}

void fight_write_rec(Fight far *f)
{
	signed short cnt = FIGHT_ID_MAX_DEF;

	if (f->id > cnt) return;
	ofile_create(g_ofile, FIGHT_LST_FILE, O_RDWR);
	ofile_read(g_ofile, &cnt, sizeof(cnt));
	ofile_seek(g_ofile, f->id * sizeof(struct fight_rec) + 2, SEEK_SET);
	ofile_write(g_ofile, f->name, sizeof(struct fight_rec));
	ofile_close(g_ofile);
}

/* Zahl der Gegner im Kampfsatz id (BE count_fight_enemies); liest den Satz nach f->name */
signed short fight_count_monsters(Fight far *f, signed short id)
{
	register signed short i;
	register signed short cnt = 0;

	if (id > FIGHT_ID_MAX_DEF) {
		cnt = 99;
	} else {
		ofile_open(g_ofile, FIGHT_LST_FILE);
		ofile_seek(g_ofile, id * sizeof(struct fight_rec) + 2, SEEK_SET);
		ofile_read(g_ofile, f->name, sizeof(struct fight_rec));
		ofile_close(g_ofile);
		for (i = 0; i < 20; i++) {
			if (f->monsters[i].mon_id) cnt++;
		}
	}
	return cnt;
}

/* Hintergrundbild des Kampffelds laden und ins XMS/EMS kopieren (BE load_fightbg) */
void fight_load_bg(Fight far *f)
{
	Pic pic;

	if (f->field->type == 4) {
		pic.load(0x3e, f->buf);
		mouse_hide(0);
		clear_screen_buf(g_vga_buf);
		mouse_show(0);
		set_palette(pic.get_palette(), 0, 0x80);
	} else if (f->field->type == 6) {
		pic.load(0x40, f->buf);
		mouse_hide(0);
		clear_screen_buf(g_vga_buf);
		mouse_show(0);
		set_palette(pic.get_palette(), 0, 0x80);
	} else {
		pic.loadf((signed char)f->field->type + 0x4b, f->buf);
		set_palette(pic.get_palette() + 0x60, 0x60, 0x20);
	}
	g_xms_move.len = 0xfa00;
	g_xms_move.src_handle = 0;
	g_xms_move.src = (signed long)f->buf;
	g_xms_move.dst_handle = (signed short)g_fight_xms;
	g_xms_move.dst = 0;
	if (g_fight_use_ems == 1) {
		EMS_copy(&g_xms_move);
	} else {
		xms_move_any(&g_xms_move);
	}
}

/* Kampffeld f->scenario aus der Archivdatei 0x8f (BE load_scenario) */
void fight_read_scenario(Fight far *f)
{
	signed short cnt;

	arc_open(g_file_3, FIGHT_SCENARIO_FILE);
	arc_read(g_file_3, &cnt, sizeof(cnt));
	if (f->scenario > cnt) f->scenario = cnt;
	arc_seek(g_file_3, (f->scenario - 1) * (signed long)sizeof(struct fight_field) + 2, SEEK_SET);
	arc_read(g_file_3, f->field, sizeof(struct fight_field));
	arc_close(g_file_3);
}
