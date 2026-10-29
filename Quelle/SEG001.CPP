/* SEG001 - Spielschleife: Reisemodus (do_travel_mode), Hauptschleife main_loop (Verteiler Ort/Dungeon/Reise/Stadt,
 * Tod, NSC, Stufenanstieg, Lowangen-Einbruch, Spielende), Leerlaufschritt game_idle (Klangdienste, Zeitgeber,
 * Anzeigen, Bildschirmschoner; mit dem markierten Aufruf der Kopierschutzpruefung) und Speichern des markierten
 * Spielstands savemenu_save (aus dem Spielstandmenue O107).
 * (seg001 in SCHWEIF.EXE, Offsets R01_000D..R01_06E4; kein CD-Audio-Treiber wie DSA1 seg001, keine Abfrage)
 *
 * C++! BCC -mlarge -3 -Ot -c -Y SEG001.CPP   (mit -Og ebenfalls gleich)
 *
 * Funktionen (Offset Name):
 *   000D do_travel_mode       Klang/Animation aus, Reisekarte (travel_main), danach Art der 3D-Ansicht setzen
 *   009D main_loop            Hauptschleife bis g_game_state != 0 (O032 main)
 *   0366 copyprot_idle_begin  leere Marke: Anfang des Suchbereichs der Musterpruefung (R23 fight_timer_cb)
 *   036B game_idle            Leerlaufschritt (aus allen Warte-/Eingabeschleifen)
 *   04D4 copyprot_idle_end    leere Marke: Ende des Suchbereichs
 *   04D9 copyprot_save_begin  leere Marke: Anfang des zweiten Suchbereichs
 *   04DE savemenu_save        markierten Spielstand nach Rueckfrage speichern (frueher savegame_op)
 *   06E0 copyprot_save_end    leere Marke: Ende des zweiten Suchbereichs
 *
 * Kopierschutz: game_idle ruft copyprot_check (SEG003), savemenu_save copyprot_stub (leer) - beide Aufrufe stehen
 * hinter dem Muster COPYPROT_MARK (jmp short ueber die vier Bytes 87 23 31 56). Der Kampf-Zeitgeber (R23
 * fight_timer_cb) sucht zwischen den Marken nach Muster + "call far" + Ziel und zaehlt sonst g_tamper_count hoch.
 * Die Marken muessen deshalb unmittelbar vor den Aufrufen stehen (inline-asm, die Bytes sind Teil der Pruefung).
 *
 * Auffaelligkeiten des Originals (bewusst nachgebaut):
 *   - main_loop: switch auf g_low_burglary_result mit "default: break;" (eigener Sprung).
 *   - main_loop: delete des Spielstandmenues hinterlaesst einen Sprung auf den direkt folgenden Befehl.
 */
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:153c..1542, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
/* Phase B: eigenes BSS (ds:8b8e..8b96) in Adressreihenfolge */
signed char g_npc_months_seen;
signed char g_view_swapped;
void far *g_inv;
signed char g_view_recam;

#define M	((SaveMenu far *)menu)

/* Muster vor einem vom Kampf-Zeitgeber ueberwachten Aufruf: jmp short +4, dann 87 23 31 56 (COPYPROT_MARKER);
 * direkt dahinter muss der Aufruf (call far ptr Ziel) folgen. */
#define COPYPROT_MARK()	asm db 0ebh, 04h, 87h, 23h, 31h, 56h

/* R01_000D: Reisemodus (Gruppe ist unterwegs) */
void do_travel_mode(void)
{
	signed short start = 0;

	snd_obj_stop(g_snd_music);
	ani_stop(g_screen_pic);
	g_pp20_index = PP20_TRAVELMAP;
	swap_out_screen();
	g_area_type = AREA_NONE;
	g_view_swapped = 1;
	if (g_mem_saving == 1)
		g_loaded_dungeon = -1;
	snd_obj_stop(g_snd_fx);
	Speech_stop(g_snd_speech);
	travel_main((TRAVEL_PTR)g_travel, start);
	if (g_group->dungeon) {
		g_area_type = AREA_DUNGEON;
		return;
	}
	if (g_group->town)
		g_area_type = AREA_TOWN;
}

/* R01_009D: Hauptschleife (O032 main nach dem Laden) */
void main_loop(void)
{
	struct hero far *npc;

	while (g_game_state == GAME_STATE_RUNNING) {
		if (g_finale_done == 1)
			g_game_state = GAME_STATE_FINISHED;
		if (g_game_finished2 == 1)
			g_game_state = GAME_STATE_FINISHED;
		if (g_game_state != GAME_STATE_FINISHED) {
			if (g_group->get_loctype())
				enter_location();
			else if (g_group->dungeon)
				dng_main();
			else if (group_get_on_route(g_group))
				do_travel_mode();
			else if (g_group->town)
				town_main();

			if (g_hero_died) {
				check_heroes_down();
				if (g_area_type != AREA_NONE)
					cam_set_from_group();
			}

			npc = g_heroes + 6;
			if (npc->type && (g_group->town || g_game_state == GAME_STATE_FINALE) &&
				g_npc_months >= 1 && g_npc_months != g_npc_months_seen) {
				groups_check_npc();
				g_npc_months_seen = g_npc_months;
			}

			if (g_pp20_index > 0 && g_in_fight == 0 &&
				(g_game_state == GAME_STATE_RUNNING || g_game_state == GAME_STATE_FINALE) &&
				!g_group->get_loctype())
				check_level_up();

			if (g_low_burglary_result) {
				/* Lowangen: Einbruch ins Schauhaus (1 tagsueber ertappt, 2 ..., 3 Gruppe versprengt) */
				if (group_find_mergeable() != -1)
					groups_merge();
				switch (g_low_burglary_result) {
				case 1: {
					Ev far *ev = new Ev;
					ev->choice_49();
					delete ev;
					break;
				}
				case 2: {
					Ev far *ev = new Ev;
					ev->choice_56();
					delete ev;
					break;
				}
				case 3:
					group_scatter(3, 0);
					break;
				default:
					break;
				}
				g_low_burglary_result = 0;
			}

			if (g_game_state == GAME_STATE_DEAD)
				show_pic_337();
			else if (g_game_state == GAME_STATE_END4 || g_game_state == GAME_STATE_END8)
				show_end_sequence();

			if (g_game_state == GAME_STATE_DEAD || g_game_state == GAME_STATE_2 ||
				g_game_state == GAME_STATE_FINISHED || g_game_state == GAME_STATE_END4) {
				/* Spielstandmenue (nur Laden): erfolgreich geladen -> weiterspielen */
				SaveMenu far *menu;

				swap_out_screen();
				menu = new SaveMenu;
				if (savegame_menu(menu, 1))
					g_game_state = g_leave_location = 0;
				delete menu;
			}

			if (g_game_state == GAME_STATE_FINALE) {
				Finale fin;
				fin.run();
			}
		} else {
			/* "Einen Spielstand, der nach Ende von Sternenschweif gespeichert wurde, kann nicht wieder in
			 *  Sternenschweif geladen werden." */
			dlg_message(g_dialog, g_ttx[474]);
		}
	}
}

/* R01_0366: Marke (Beginn des ueberwachten Bereichs um game_idle) */
void copyprot_idle_begin(void)
{
}

/* R01_036B: Leerlaufschritt */
void game_idle(void)
{
	if (g_fight_obj)
		fight_anim_tick((FIGHT_PTR)g_fight_obj);
	if (g_snd_service_due == 1) {
		if (g_refresh_portraits != 0 && g_pp20_index == 4 && g_silent_mode == 0)
			screen_portraits(g_screen);

		COPYPROT_MARK();
		copyprot_check();

		if (g_screen_pic_run)
			anim_tick(g_screen_pic);
		snd_service(g_snd_music);
		snd_speech_service(g_snd_speech);
		snd_service(g_snd_fx);
		if (g_anim_running) {
			Anim_tick(g_anim_cur);
		} else {
			if (g_silent_mode == 0)
				do_timers(&g_timers_lock);
			if ((g_pp20_index == 4 || g_pp20_index == PP20_TRAVELMAP) && g_silent_mode == 0 &&
				g_clock_update)
				Date_draw(g_date, 0);
			bars_refresh(g_bars);
			if ((g_pp20_index == PP20_SHEET || g_pp20_index == 15) && g_inv_warn_enabled)
				inv_warn_signal(g_inv);
			if (g_pp20_index == PP20_TRAVELMAP && g_silent_mode == 0) {
				if (g_rose_active)
					tmap_mark1_anim(g_tmap, g_tmap_mark1_x, g_tmap_mark1_y);
				if (g_rose_two_marks)
					tmap_mark2_anim(g_tmap, g_tmap_mark2_x, g_tmap_mark2_y);
			}
			saver_tick(&g_saver);
		}
		g_snd_service_due = 0;
	}
}

/* R01_04D4: Marke (Ende des ueberwachten Bereichs um game_idle) */
void copyprot_idle_end(void)
{
}

/* R01_04D9: Marke (Beginn des ueberwachten Bereichs um savemenu_save) */
void copyprot_save_begin(void)
{
}

/* R01_04DE: markierten Spielstand speichern; mode 1 = nur Laden erlaubt, ask = vorher nachfragen.
 * Rueckgabe 1 = gespeichert. */
signed short savemenu_save(void far *menu, signed short mode, signed short ask)
{
	char far *p;
	char path[40];
	char name[14];
	char text[100];
	char shown[14];
	register signed short ok = 0;
	register signed short fd;

	strcpy(name, M->names[M->sel + M->first].name);
	strcat(name, ".GAM");
	sprintf(path, g_path_fmts[1], name);
	if (mode == 1) {
		/* "Im Moment ist diese Option nicht verfuegbar!" */
		dlg_message(g_dialog, M->txt[22]);
	} else if (M->count > 0) {
		COPYPROT_MARK();
		copyprot_stub();

		g_snd_service_due = 1;
		game_idle();
		g_save_count += 100;
		fd = _open(path, O_RDONLY | O_BINARY);
#if FIX_0232
		/* Fix #232: Handle 0 ist gueltig (gleiche Codelaenge, der ueberwachte Aufruf bleibt unveraendert) */
		if (fd >= 0) {
#else
		if (fd > 0) {
#endif
			/* "Spiel <%s> existiert bereits. Wirklich ueberschreiben?" */
			strcpy(shown, M->names[M->sel + M->first].name);
			p = strchr(shown, ' ');
			if (p != 0) {
				*p = 0;
			} else {
				p = strchr(shown, '.');
				if (p != 0)
					*p = 0;
			}
			sprintf(text, M->txt[24], shown);
			close(fd);
		} else {
			/* "Spiel wirklich sichern?" */
			strcpy(text, M->txt[2]);
		}
		if (ask == 0 || dlg_yes_no(g_dialog, text) == 1) {
			automap_load_seen(g_map);
			savegame_save_cur(M);
			savemenu_update(M);
			ok = 1;
		}
	} else {
		/* "Keine Spielstaende mehr verfuegbar. Zunaechst neue anlegen!" */
		dlg_message(g_dialog, M->txt[19]);
	}
	return ok;
}

/* R01_06E0: Marke (Ende des ueberwachten Bereichs um savemenu_save) */
void copyprot_save_end(void)
{
}
