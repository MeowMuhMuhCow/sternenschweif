/* s_proto.h - Prototypen der modulübergreifend benutzten Funktionen (Teil von schweif.h)
 * ERZEUGT von names/tools/namen.py aus namen.tsv und den Deklarationen der csrc-Module - nicht von
 * Hand aendern, sondern names/daten/manuell_*.tsv bzw. proto_fest.tsv pflegen und neu erzeugen.
 * Parametertypen: Mehrheit der Module; unbekannte Objektzeiger als void far *. Abweichungen, die Code
 * erzeugen koennen (char/short, signed/unsigned), stehen in names/konflikte.tsv.
 * Kommentar je Block: Segment (Rnn resident, Onnn Overlay). */
#ifndef S_PROTO_H
#define S_PROTO_H

/* O000 */
void dng10_altar(struct area_events far *);
void dng10_anvil(struct area_events far *);
void dng10_relief(struct area_events far *);
void dng10_sharp_object(struct area_events far *);
void dng10_tools(struct area_events far *);
void dng10_junk(struct area_events far *);
void dng10_countdown(struct area_events far *);
void dng10_rotten_chest(struct area_events far *);

/* O001 */
short dng16_handler(struct area_events far *, struct map_event far *);
short dng16_door(struct area_events far *, short, short, short);
void dng16_stone_plate(struct area_events far *);
void dng16_hot_plates(struct area_events far *, short);
void dng16_plates_turn_back(struct area_events far *, short);
void dng16_walled_passage(struct area_events far *);
short dng16_double_key(struct area_events far *, short);
void dng16_leather_chest(struct area_events far *);
void dng16_treasure_chest(struct area_events far *);
void dng16_salamander_chest(struct area_events far *);
void dng16_exit(struct area_events far *);
void dng16_rubble(struct area_events far *);

/* O002 */
short dng16_seq_golem_cb(unsigned short, unsigned short);
short dng16_seq_forge_cb(unsigned short, unsigned short);
void dng16_water_basin(struct area_events far *);
void dng16_lava_wall(struct area_events far *);
void dng16_stone_door(struct area_events far *);
void dng16_golem(struct area_events far *);
void dng16_golem_forge(struct area_events far *);
void dng16_golem_fight(struct area_events far *);
short dng16_forge_heat(struct area_events far *);

/* O003 */
void Cache_ctor(void far *, short, signed long);
void Cache_dtor(void far *, short);
void far * Keylist_ctor(void far *, short);
void Keylist_dtor(void far *, short);
short load_pic_list(void far *, void far *, void far *, short, short);
short keylist_add(void far *, short, short);
void keylist_load(void far *, void far *);
void cache_purge(void far *, short);
void cache_clear(void far *);

/* O004 */
signed short cd_get_label(signed short, char far *);
signed short cd_check_drive(signed short);
void CD_stop(void far *);
void CD_music(void far *, short);
void CD_update(void far *);
void CD_volume(void far *, short);

/* O005 */
void timers_hourly(short far *);
void timers_4hourly(short far *);
void timers_daily(short far *);
void hero_hunger_thirst(short far *, struct hero far *);
void hero_disease_tick(short far *, struct hero far *);
signed long ap_for_level(short);
void check_level_up(void);
void inn_days_tick(void);
void npc_pay_tick(void);
void jail_tick(void);
void show_daily_hint(void);
void hero_fur_tick(struct hero far *);
void dng13_smoke_damage(void);

/* O006 */
signed char get_skill_value(struct hero far *, short);
void set_skill_value(struct hero far *, short, signed char);
void add_skill_value(struct hero far *, short, signed char);
short use_skill(struct hero far *, signed char, short);
short lvl_select_skill(struct hero far *, short);

/* O007 */
void scene_line_speech(char far *, signed short);
void scene_line_anim(char far *, signed char);
signed short blackmage_scene(void);
void phex_finale(void);
void scene_line(char far *, signed short, signed short);
signed short scene_choice(char far *, char far *, char far *, signed short);
void debug_test_menu(void);
void phex_finale_fight(void);

/* O008 */
void far * Inv_ctor(short, short);
void Inv_dtor(void far *, short);
void inv_set_cursor(void far *, short);
void inv_reset_cursor(void far *);
void inv_draw_slot(void far *, void far *, short, short);
void inv_reselect(void far *);
void inv_sel_next(void far *);
void inv_sel_prev(void far *);
void inv_update_marks(void far *);
void inv_draw_slots(void far *, void far *);

/* O009 */
void inv_confirm(void far *);
void inv_next_hero(void far *);
void inv_toggle_body(void far *);
void inv_draw(void far *);
void open_hero_sheet(void);
void inv_warn_signal(void far *);

/* O010 */
short view_init(void);
short obj3d_prepare(struct obj3d far *);
void swap_write(void far *, unsigned long);
void swap_read(void far *, unsigned long);
void view_swap_out(void);
void view_swap_in(void);
void view_suspend(void);
short swap_out_screen(void);
void swap_in_screen(void);
void swap_save_file(void);
void swap_alloc(void);
void view_timer_off(void);
void view_timer_on(void);
void tex_debug_view(void);
short ems_check(void);
short ems_frame_seg(void);
short ems_free_pages(void);
short ems_alloc(short, void far *);
short ems_free(short);

/* O011 */
void dev_info_screen(void);
void debug_log(char far *);
void dev_beep(void);
signed long heap_free_total(void);
signed short dos_open_files(void);

/* O012 */
void store_save(STORE_PTR, signed short);
void store_grid_mark(STORE_PTR);
signed short store_grid_input(STORE_PTR, signed short far *);
signed short store_is_open(STORE_PTR);
void store_run(STORE_PTR);
signed short store_can_enter(STORE_PTR);
signed short store_check_time(STORE_PTR, signed long);
void store_lock(STORE_PTR, signed short);
signed short store_check_fee(STORE_PTR);
void store_load(STORE_PTR);
void store_create_file(STORE_PTR);
void store_talk(STORE_PTR);

/* O013 */
void talk_hand_over_weapons(struct talk_desc far *);
void talk_hand_over_items(struct talk_desc far *);
void talk_reward(struct talk_desc far *, short);
void talk_check_dwarf(struct talk_desc far *);
void talk_get_dwarf_name(struct talk_desc far *, char far *);
void talk_give_lowangen_key(struct talk_desc far *);
void talk_rejoin_leave(struct talk_desc far *);
void talk_take_heroes(struct talk_desc far *);
void talk_rejoin_heroes(struct talk_desc far *);
void talk_choose_hero(struct talk_desc far *, short, char far *);
void talk_rejoin_hero(struct talk_desc far *, char far *);
void talk_npc_leaves(struct talk_desc far *);
void talk_game_won(struct talk_desc far *);
void talk_nop(void);
void talk_check_group_size(struct talk_desc far *);
void talk_check_quest_anim(struct talk_desc far *);
void talk_cast_analues(struct talk_desc far *);
void talk_check_parcel(struct talk_desc far *);
void talk_lose_items(struct talk_desc far *, short);
void talk_fight_95(struct talk_desc far *);
short talk_hand_over_document(struct talk_desc far *);
short talk_fight_97(struct talk_desc far *, short);
short talk_price(struct talk_desc far *, short);
void talk_set_peddler(struct talk_desc far *, short);
void talk_nop2(struct talk_desc far *);
void talk_nop3(struct talk_desc far *);
void talk_subtitle(struct talk_desc far *, short);
short cutscene_key(short, unsigned short);
short talk_cutscene(short, short, struct talk_desc far *);

/* O014 */
void dng_main(void);
void dng_load(void);
void group_set_screen_pos(short, short);
void dng_save_frame(void);
void view_free_frame(void);
void dng_automap(void);
short dng_action(void);
void dng_make_events(void);
void dng_update_icons(void);
void dng_set_light(void);

/* O015 */
void sheet_slot_changed(signed char, short, short);
void sheet_cast_anim(void);
short sheet_equip_anim_id(signed char, short, short);
void xms_save_screen(void far *);
void xms_restore_screen(void far *);

/* O016 */
void far * FigAnim_ctor(void far *, struct ani_seq far *, short);
void FigAnim_dtor(void far *, short);
void fig_anim_load_seq(FIGANIM_PTR);
void fig_anim_load_frame(FIGANIM_PTR);
void fig_anim_find_seq(FIGANIM_PTR, short, short);
signed char fig_anim_next_frame(FIGANIM_PTR);
void fig_anim_unpack(FIGANIM_PTR, BMP_PTR, void far *);
void fig_anim_sound(FIGANIM_PTR, unsigned char);

/* O017 */
void place_event_007(TRAVEL_PTR);
void place_event_017(TRAVEL_PTR);
void place_event_018(TRAVEL_PTR);
void place_event_039(TRAVEL_PTR);
void place_event_041(TRAVEL_PTR);
void place_event_048(TRAVEL_PTR);
void place_event_065(TRAVEL_PTR);
void place_event_066(TRAVEL_PTR);
void place_event_083(TRAVEL_PTR);
void place_event_087(TRAVEL_PTR);
void place_event_100(TRAVEL_PTR);

/* O018 */
short start_fight(short, short);
void far * Fight_ctor(void far *, short, short);
void Fight_dtor(void far *, short);
short fight_run(void far *);
void fight_nop(void);

/* O019 */
void dng09_enter_building(short, struct map_event far *);
short town03_handler(struct area_events far *, struct map_event far *);
short dng09_handler(struct area_events far *, struct map_event far *);

/* O020 */
char far * sfight_event_text(FIGHT_PTR, short, short far *);
char far * sfight_event_text2(FIGHT_PTR, short, short far *);
void sfight_seq_main(FIGHT_PTR);
void sfight_seq_intro(FIGHT_PTR);
void sfight_intro(FIGHT_PTR);
void sfight_seq_end(FIGHT_PTR);
short sfight_hit_chance(struct hero far *, short);
short sfight_enemy_turn(FIGHT_PTR);

/* O021 */
void sfight2_setup(FIGHT_PTR);
void sfight2_place_enemy(FIGHT_PTR);
void sfight2_intro(FIGHT_PTR);
void sfight2_say(FIGHT_PTR, char far *, short, short);
short sfight2_check_word(FIGHT_PTR, char far *);
FIGANIM_PTR sfight2_seq_stand(FIGHT_PTR, struct ani_seq far *);
FIGANIM_PTR sfight2_seq_appear(FIGHT_PTR, struct ani_seq far *);
FIGANIM_PTR sfight2_seq_attack(FIGHT_PTR, struct ani_seq far *);
FIGANIM_PTR sfight2_seq_vanish(FIGHT_PTR, struct ani_seq far *);

/* O022 */
void fight_setup(FIGHT_PTR);
void fill_enemy_sheet(FIGHT_PTR, short, signed char, signed char);
void fig_add_enemy(FIGHT_PTR, struct enemy far *, short, short);
void fig_init_enemies(FIGHT_PTR);
void fig_init_heroes(FIGHT_PTR);
void fight_make_walkmap(FIGHT_PTR);
void fig_add_field_objs(FIGHT_PTR);
void fight_draw_bg(FIGHT_PTR);
void fight_intro_text(FIGHT_PTR);

/* O023 */
void fight_write_rec(FIGHT_PTR);
void fight_read_rec(FIGHT_PTR);
short fight_count_monsters(FIGHT_PTR, short);
void fight_load_bg(FIGHT_PTR);
void fight_read_scenario(FIGHT_PTR);

/* O024 */
short fli_play(short, short, fli_cb, short);
short fli_play_frames(short, unsigned short, short);
short fli_play_file(short, short);

/* O025 */
short fli_read_frame(struct fli_screen far *, short);
void fli_clear_screen(void);
short fli_open(short, struct fli_header far *);

/* O026 */
void chargen_draw_name(CHARGEN_PTR);
void chargen_enter_name(CHARGEN_PTR);
void chargen_draw_portrait(CHARGEN_PTR);
void chargen_select_portrait(CHARGEN_PTR);
short chargen_run(CHARGEN_PTR);
void chargen_redraw(CHARGEN_PTR);
short chargen_can_move_attribs(CHARGEN_PTR);

/* O027 */
void opening_scene3(struct opening far *);
void opening_fade_in(struct opening far *, unsigned char far *);

/* O028 */
void opening_play_fli(struct opening far *);
void opening_fli_part(struct opening far *, signed short, unsigned char far *);

/* O029 */
void herofile_copy(char far *, char far *);
short savegame_read_hero(short, short, struct hero far *);
void herofile_remove(short, short);
void hero_delete_chr(char far *);
short herofile_find(char far *, short far *, short);
void herofile_write(short, void far *, short);
void herofile_append(void far *, short);
void herofile_save(struct hero far *, short);
short hero_load_chr(short, short, short, short);
void hero_save_file(struct hero far *);

/* O030 */
void enter_location(void);
void loc_load_data(signed char);
void loc_fade_out(void);
void loc_return_redraw(void);

/* O031 */
void far * Ev_ctor(void far *);
void Ev_dtor(void far *, short);
void ev_market_hall(void far *);
void ev_magistrate(void far *);
void ev_eolan(void far *);
void ev_house_low49(void far *);
void ev_academy(void far *);
void ev_smithy(void far *);
void ev_showhouse(void far *);
void ev_guardhouse(void far *);
void ev_city_gate1(void far *);
void ev_city_gate2(void far *);
void ev_fortress_gate(void far *);
void ev_trade_office(void far *);
void ev_altrogolan(void far *);
void ev_ailian(void far *);
void ev_vindaria(void far *);
void ev_jandora(void far *);
void ev_dragan(void far *);
void ev_talk_49(void far *);
void ev_zaskia(void far *);
void ev_talk_4a(void far *);
void ev_peddler_redhead(void far *);
void ev_falkert(void far *);
void ev_deringorn(void far *);
void ev_offering_bowl(void far *);
void ev_bridge_toll(void far *);

/* O032 */
void game_free_all(void);
signed long disk_free(void);
void prepare_dirs(void);
signed short init_memory(void);
void free_swap_handles(void);
void init_game_state(void);
void check_speech_file(void);
signed short dos_files_total(void);
void create_amap_watch_file(void);
void globals_ctor(void);
void globals_dtor(void);

/* O033 */
void temple_miracle(TEMPLE_PTR);
void miracle_group_effect(TEMPLE_PTR, signed char, signed long, signed char, signed char);
void miracle_heal(TEMPLE_PTR, signed short, char far *);
void miracle_resurrect(TEMPLE_PTR, char far *);
void miracle_show_anim(TEMPLE_PTR);

/* O034 */
void clear_screen_buf(void far *);
void pal_fade_out(void);
void pal_fade_in(void far *);
void group_imprison(short);
short group_scatter(short, short);
char far * code_to_text(unsigned char);
void read_record(void far *, short);
void save_screen(void);
void restore_screen(void);
short dice_code_roll(short);
void far * load_portrait_buf(short);
void pal_blend(struct srgb far *, short);
void pal_blend2(struct srgb far *, struct srgb far *, short, short, short);
void load_palette_arc3(void);
void load_palette_arc3b(void far *);
short get_group_index(struct group far *);
void prepare_spell_icons(void);
void game_menu(void);
void txt_build_index(void far *, void far *, signed long);
short group_find_mergeable(void);
void show_fullscreen_pic2(void);
void show_fullscreen_pic(void);
short hero_file_active(char far *);
short chr_list_has(char far *, short, struct chr_entry far *);
void chr_list_remove(struct chr_entry far *, short, short);
short chr_list_collect(struct chr_entry far *, short);
void load_text_block(short);
signed char code_to_colsel(unsigned char);
void select_screen(void);
void select_screen_update(void);
signed long clamp_min_l(signed long, signed long);
signed long clamp_max_l(signed long, signed long);
char far * str_cut_at(char far *, signed char);
void pic_to_bmp(void far *, short, BMP_PTR, struct srgb far *, short);
void bmp_free_data(BMP_PTR);
void pal_blend_from_color(struct srgb, struct srgb far *, short);
void pal_blend_to_color(struct srgb far *, struct srgb, short);
void pal_blend_from_dark(struct srgb far *, short);
void pal_mix(struct srgb far *, struct srgb far *, struct srgb far *, short, short);
void dissolve_in(unsigned char far *);
void show_pic_337(void);
void show_end_sequence(void);
void group_leave_place(short);

/* O035 */
void Finale_ctor(FINALE_PTR);
void Finale_dtor(FINALE_PTR, signed short);
void Finale_run(FINALE_PTR);

/* O036 */
void finale_play_script(FINALE_PTR);

/* O037 */
void far * File_ctor_names(void far *, char far * far *, short);
void File_ctor(void far *, void far *, void far *, void far *, void far *, short);
void File_dtor(void far *, short);
short file_open(void far *, short);
short file_open_mode(void far *, short, unsigned short);
signed long file_load(void far *, short, void far *);
signed long File_read_all(void far *, void far *);
signed long file_read(void far *, void far *, signed long);
signed long File_write(void far *, void far *, signed long);
void file_close(void far *);
signed long file_length2(void far *);
signed long file_length(void far *, short);
signed long file_seek(void far *, signed long, short);
short file_create(void far *, short, short);
char far * file_getname(void far *, short);
short File_find(char far *, char far *);
short file_is_loose(void far *, short);
short file_open_handle(void far *, short);
short file_open_handle_mode(void far *, short, short);
short File_exists(void far *, short);
short file_open_in_arc(void far *, short);
void file_dummy(void);

/* O038 */
void Travel_ctor(TRAVEL_PTR);
void Travel_dtor(TRAVEL_PTR, short);
void travel_enter(TRAVEL_PTR);
void travel_leave(TRAVEL_PTR);
short route_load_points(TRAVEL_PTR, short);
void travel_main(TRAVEL_PTR, signed char);
signed char travel_step(TRAVEL_PTR);
signed char travel_arrive(TRAVEL_PTR);

/* O039 */
void Intro_ctor_o039(void far *);
void Intro_dtor_o039(void far *, short);
void Intro_run_o039(void far *);
void pal_crossfade(unsigned char far *, unsigned char far *, short);

/* O040 */
void Anim3_ctor(void far *);
void Anim3_dtor(void far *, short);
void Anim3_run(void far *);
char far * Anim3_cue_text(void far *, unsigned short, short far *);
void Anim3_play(void far *, short, short, unsigned char, short, short, short, short);
void Anim3_blank(void far *, unsigned char far *);

/* O041 */
void Outro1_ctor(void far *);
void Outro1_dtor(void far *, short);
void Outro2_ctor(void far *);
void Outro2_dtor(void far *, short);
void Outro2_run(void far *);
void Outro1_run(void far *);
void Outro2_plain(void far *);
void Outro2_voice(void far *);
void Outro1_plain(void far *);
void Outro1_voice(void far *);

/* O042 */
void Sound_ctor(void far *, void far *, void far *, void far *);
void Sound_ctor_o042(void far *, void far *, void far *);
void Sound_dtor(void far *, short);
void Sound_play(void far *, short);
void Sound_start(void far *, short);
void Sound_stop_music(void far *);
short snd_obj_stop(void far *);
void Sound_volume2(void far *, short, short);
void Sound_volume3(void far *, short, short, short);
void Sound_wait(char far *);
short Sound_mtype(void far *);
void Sound_nocallback(void far *);
void Speech_stop(void far *);
void Speech_volume(void far *, short);
void Speech_set(void far *, short);
short speech_active(void far *);
void Speech_wait(void far *);

/* O044 */
signed short temple_is_open(TEMPLE_PTR);
void temple_run(TEMPLE_PTR);
void temple_add_heroes(TEMPLE_PTR, signed short);
void temple_dismiss_heroes(TEMPLE_PTR, signed short);
signed short temple_erase_hero(TEMPLE_PTR);
void temple_load_pic(TEMPLE_PTR, signed short);
void temple_redraw(TEMPLE_PTR);
void temple_talk(TEMPLE_PTR);
void temple_desecrate(TEMPLE_PTR);
void temple_load_kind(TEMPLE_PTR);
void temple_find_god(TEMPLE_PTR);
void temple_diary_party(TEMPLE_PTR);
char far *temple_str_join(TEMPLE_PTR, struct hero far *);
char far *temple_str_leave(TEMPLE_PTR, struct hero far *);

/* O045 */
void o045_ani_halt(void);
void finale_play(void);

/* O046 */
void cam_set_from_group(void);
void automap_view(short, short, short);
void town_main(void);
void town_load(void);
void town_automap(void);
short town_action(void);
void town_make_events(void);
void view_save_frame(void);
void town_update_icons(void);
void town_daytime_cells(void);
void town_set_cell(short, short, signed char, short, short);
void town_mugging(void);
void town_random_event(short);
void town_cache_files(void);

/* O047 */
void item_read_text(void far *);

/* O048 */
short item_combo_index(void far *, short, short);
void item_use_keyring(void far *);
void item_use_quest_items(void far *);
void item_maintain_weapons(void far *);
short item_in_recipe_book(void far *, short);
short item_on_keyring(void far *, short);
short item_anim_cb(unsigned short, unsigned short);
void item_quest_anim(void far *);

/* O049 */
short conv_run(void far *, short);
void conv_print(void far *, char far *, short);
void conv_mark_topic(void far *, short);
short conv_ask(void far *, short);
void conv_draw_topics(void far *, short);
char far * conv_random_text(void far *, short, signed char);
void conv_draw(void far *, short);
void conv_clear_new(void far *);
void conv_mouse_on(void far *);
void conv_mouse_off(void far *);
void conv_dummy(void far *);
void conv_answer_frame(void far *);

/* O050 */
void Anim_tick(void far *);
void Anim_advance(void far *);
void Anim_unpack(void far *, void far *);
unsigned char Anim_running(void far *);
void Anim_load(void far *);
void Anim_load_pal(void far *, short, short);
signed long Anim_max_frame_size(void far *);
void Anim_load_frame(void far *);
void Anim_select_seq(void far *, short);

/* O051 */
void far * Anim_ctor(void far *, short, short, short);
void Anim_dtor(void far *, short);
void Anim_size(void far *, short far *, short far *);
void Anim_area(void far *, short far *, short far *, short far *, short far *);
void Anim_frame_size(void far *, short far *, short far *);
void Anim_rect(void far *, short far *, short far *, short far *, short far *);
void Anim_pal(void far *, short, short);
void Anim_setbuf_bmp(void far *, void far *);
void Anim_setbuf(void far *, short, short, void far *);
void Anim_setbg(void far *, void far *);
void Anim_setbg_buf(void far *, short, short, void far *);
void Anim_clr_buf(void far *);
void Anim_clr_bg(void far *);
void Anim_start(void far *, short, short, short);
void Anim_stop(void far *);
void Anim_step(void far *);
void Anim_wait(void far *, short);
void Anim_next(void far *);

/* O052 */
void Automap_ctor(void far *);
void Automap_dtor(void far *, short);
void automap_open(void far *, short, short, short);
void automap_close(void far *);
void automap_load_cells(void far *, short);
void automap_set_town_type(void far *, short);
void automap_show(void far *);
void automap_draw(void far *);
void automap_load_seen(void far *);
short automap_save(void far *);
short automap_load(void far *);
void automap_run(void far *);
void automap_frame(void far *);
void automap_load_tiles(void far *, short);
void automap_make_bmps(void far *);
void automap_load_tile_imgs(void far *, short);
void automap_scroll_up(void far *);
void automap_scroll_down(void far *);
void automap_scroll_right(void far *);
void automap_scroll_left(void far *);
void automap_center(void far *);
void automap_reveal_all(void far *);
void automap_load_bg(void far *);
void automap_blit(void far *, char far *, signed long);
void automap_draw_tile(void far *, short, short, short);
void automap_cursor_right(void far *);
void automap_cursor_down(void far *);
void automap_cursor_left(void far *);
void automap_cursor_up(void far *);
void automap_update_cursor(void far *);
void automap_edit_note(void far *);
void automap_animate(void far *);
void automap_erase_marks(void far *);
void automap_erase_cursor(void far *);
void automap_clear_notes(void far *);
void automap_del_note(void far *);
void automap_put_note(void far *, void far *, short);
void automap_get_note(void far *, void far *, short);
char far * automap_find_note(void far *);
short automap_find_slot(void far *);
void automap_save_note(void far *);
void automap_load_notes(void far *);
void automap_save_notes(void far *);

/* O053 */
void automap_nop(void far *);
void automap_cell_write(void far *, short, short, short, signed char);
void automap_random_cell(void far *);
void automap_init(void far *, short);
void automap_free_cells(void far *);
short automap_travel(void far *);
short automap_path_search(void far *, char far *, signed char far *, short far *, short far *, short);
void automap_path_trace(void far *, char far *, signed char far *);
signed char automap_cell_read(void far *, short, short, short);

/* O054 */
void anim_select_variant(void far *, short, signed char);
void anim_reset(void far *, short);
void anim_sound(short, signed char);
void anim_load_pic(void far *, short, short, void far *);
void anim_tick(void far *);
void anim_load(void far *, short, short);
void anim_load_variant(void far *, short, short, signed char);
void anim_free_all(void far *);
void anim_init(void far *);

/* O055 */
void ani_ctor(void far *);
void ani_dtor(void far *, short);
void ani_clear(void far *);
void ani_pal_black(void far *);
void ani_set_pal(void far *, void far *);
void ani_set_pos(void far *, short, short);
void ani_set_pal_size(void far *, short);
void ani_halt(void far *);
void ani_stop(void far *);
void ani_play(void far *, short);
void ani_play_until(void far *, short, signed char);
signed long ani_calc_size(void far *, signed long);
void ani_load(void far *, short);

/* O056 */
char far * grammar_article(short, short, short);
char far * grammar_plural(short, char far *, short);
char far * grammar_name(char far *);
char far * grammar_pronoun(short, short, short);
char far * grammar_def_article(short, short);
char far * grammar_pronoun_of(short, short, short);

/* O057 */
void Bars_ctor(void far *);
void Bars_draw(void far *, short, short, short, short, short, void far *);
void Bars_tick(void far *);

/* O058 */
void preselect_best_skill(struct group far *, short);
void preselect_best_attrib(struct group far *, short);
void preselect_hero(struct group far *, struct hero far *);
void preselect_agile_hero(struct group far *);
void preselect_hunter(struct group far *);
void preselect_for_task(struct group far *, signed char);

/* O058 */

/* O059 */
void binge_wall_hole(struct area_events far *, short);
void binge_shaft(struct area_events far *);
void binge_shaft_up(struct area_events far *, short);
void binge_shaft_down(struct area_events far *, short);
short binge_has_rope(struct area_events far *);

/* O060 */
short dng10_handler(struct area_events far *, struct map_event far *);
short dng10_door(struct area_events far *, short, short, short);
void dng10_portal_enter(struct area_events far *);
void dng10_portal_leave(struct area_events far *);
void dng10_portal_locked(struct area_events far *);
void dng10_skeleton(struct area_events far *);
void dng10_ingerimm_statue(struct area_events far *);
void dng10_coal_basin(struct area_events far *);
void dng10_coal_basin_plain(struct area_events far *);
void dng10_move_basin(struct area_events far *);
void dng10_grab_ring(struct area_events far *, struct hero far *);

/* O061 */
short dng11_handler(struct area_events far *, struct map_event far *);
short dng11_door(struct area_events far *, short, short, short);
void dng11_lever_switch(struct area_events far *);
void dng11_pressure_plate(struct area_events far *);
void dng11_bolt_trap(struct area_events far *, short, short);
void dng11_bolt_trap_found(struct area_events far *, short);
void dng11_winch(struct area_events far *);
void dng11_block_falls(struct area_events far *);

/* O062 */
void dng11_lever_floor(struct area_events far *);
void dng11_ramp_climb(struct area_events far *);
void dng11_plate(struct area_events far *);
void dng11_wardrobe(struct area_events far *);
void dng11_kobold_chest(struct area_events far *);
void dng11_kobold(struct area_events far *);
void dng11_kobold_soften_weapons(struct area_events far *);
short dng11_kobold_gift(struct area_events far *);
void dng11_kobold_riddle(struct area_events far *);
void dng11_kobold_send_away(struct area_events far *, short);

/* O063 */
short dng12_handler(struct area_events far *, struct map_event far *);
short dng12_door(struct area_events far *, short, short, short);
void dng12_latrine(struct area_events far *);
void dng12_skeleton_fight(struct area_events far *);
void dng12_skeleton(struct area_events far *);
void dng12_buffer_stop(struct area_events far *);
void dng12_track_end(struct area_events far *);
void dng12_mine_cart(struct area_events far *);
void dng12_cart_ride(struct area_events far *);
void dng12_ore_chute(struct area_events far *);
void dng12_ore_heap(struct area_events far *);
void dng12_tool_wall(struct area_events far *);
void dng12_forge(struct area_events far *);

/* O064 */
void dng12_lever(struct area_events far *);
void dng12_pit(struct area_events far *);
void dng12_pit_jump(struct area_events far *, struct hero far *);
void dng12_pit_recover(struct area_events far *, struct hero far *);
void dng12_altar_inscription(struct area_events far *);
void dng12_dwarf_grave(struct area_events far *, short, short, short);
void dng12_dig_rubble(struct area_events far *, short, short);
void dng12_dig_collapse(struct area_events far *, short, short);
void dng12_clear_debris(struct area_events far *);
void dng12_rotten_beams(struct area_events far *);

/* O065 */
short dng13_handler(struct area_events far *, struct map_event far *);
short dng13_door(struct area_events far *, short, short, short);
void dng13_ambush(struct area_events far *);
void dng13_wash(struct area_events far *);
void dng13_lava_shaft(struct area_events far *);
void dng13_wheel(struct area_events far *);
void dng13_gargoyles(struct area_events far *);
void dng13_gargoyles_gone(struct area_events far *);
void dng13_fur_wall_cold(struct area_events far *);
void dng13_salt_seam(struct area_events far *);
void dng13_fur_wall_burning(struct area_events far *);
short dng13_fur_cure(struct area_events far *, short);
struct hero far * dng13_fur_victim(struct area_events far *, short);
void dng13_fur_burn(struct area_events far *, short);

/* O066 */
short dng14_handler(struct area_events far *, struct map_event far *);
void dng14_swim(struct area_events far *);
void dng14_dive(struct area_events far *);
void dng14_wash(struct area_events far *);
void dng14_floor_plate(struct area_events far *);

/* O067 */
signed short camp_encounter(CAMP_PTR, signed char, struct hero far *);
void camp_visitor(CAMP_PTR, struct hero far *);
signed char camp_visitor_type(CAMP_PTR);
void camp_reveal_area(CAMP_PTR, signed char);
void camp_place_set_dir(CAMP_PTR, unsigned char, signed char);

/* O068 */
void peddler_buy_screen(PEDDLER_PTR);
void peddler_buy_draw_grid(PEDDLER_PTR);
void peddler_buy_draw_item(PEDDLER_PTR, signed short);
void peddler_buy_sum(PEDDLER_PTR, signed short far *, signed short far *);
void peddler_buy_draw_money(PEDDLER_PTR, signed long);
void peddler_buy_draw_weight(PEDDLER_PTR, signed short, signed short);
void peddler_buy_draw_slots(PEDDLER_PTR, signed short, signed short);
void peddler_format_money(PEDDLER_PTR, char far *, signed long);
void peddler_grid_mark(PEDDLER_PTR, signed short);

/* O069 */
void peddler_run(PEDDLER_PTR);
void peddler_build_wares(PEDDLER_PTR);
void peddler_add_ware(PEDDLER_PTR, signed short, signed short);
signed short peddler_trade(PEDDLER_PTR, signed long);
signed short peddler_grid_input(PEDDLER_PTR, signed short far *);

/* O070 */
signed short lodging_is_open(LODGING_PTR);
void lodging_run(LODGING_PTR);
void lodging_ask_price(LODGING_PTR);
void lodging_eat(LODGING_PTR);
void lodging_sleep(LODGING_PTR);
void lodging_pay(LODGING_PTR);

/* O071 */
void far * Camp_ctor(short, short);
void Camp_dtor(void far *, short);
signed short camp_run(CAMP_PTR, signed char);
void camp_menu_wild(CAMP_PTR);
void camp_menu_rest(CAMP_PTR);
void camp_choose_watch(CAMP_PTR);
void camp_find_food(CAMP_PTR);
void camp_gather_herbs(CAMP_PTR);
void camp_use_skill(CAMP_PTR);
void camp_reset_watch(CAMP_PTR, signed char, signed char);
void camp_eat_drink(CAMP_PTR);
void camp_menu_jobs(CAMP_PTR);
void camp_clear_jobs(CAMP_PTR);
signed short camp_rest_jobs(CAMP_PTR);

/* O072 */
void camp_set_foes(signed short, signed short, signed short);
signed short camp_night_encounter(void);
void camp_use_magic(CAMP_PTR);
void camp_group_menu(CAMP_PTR);
signed short camp_other_group_near(CAMP_PTR);
signed short camp_rest(CAMP_PTR);
void camp_regenerate(CAMP_PTR, signed short);
void camp_wake_up(CAMP_PTR, signed short);

/* O073 */
void camp_visitor_vagabond(CAMP_PTR);
void camp_visitor_hunter(CAMP_PTR);
void camp_visitor_druid(CAMP_PTR);
void camp_visitor_juggler(CAMP_PTR);

/* O074 */
void money_to_string(char far *, unsigned long);
short select_from_list(char far *, short, short);
short get_free_hero_slot(void);
short count_heroes_check2(void);
void check_heroes_down(void);
void sub_ae_max(struct hero far *, short);
void add_hero_ae2(struct hero far *, short);
signed long add_hero_ap(struct hero far *, short);
signed long sub_hero_ap(struct hero far *, short);
void sub_le_max(struct hero far *, short);
void add_le_max(struct hero far *, short);
void hero_get_poison(struct hero far *, short);
void hero_get_ill(struct hero far *, short);
void hero_disease_test(struct hero far *, short, short);
void hero_get_drunken(struct hero far *);
short hero_cure_poison(struct hero far *);
void hero_restore_le_mod(struct hero far *);
short hero_brewing_in_inn(struct hero far *, short);
void hero_dismiss(struct hero far *, signed char);
short count_heroes(void);
short hero_cure_disease(struct hero far *);
void hero_get_sober(struct hero far *);
void hero_sleep(struct hero far *, short);
short bargain(struct hero far *, short, signed long, short, signed char);
short hero_has_ingredients(struct hero far *, short);
short do_alchemy(struct hero far *, short, short);
short hero_clear_renegade(struct hero far *);

/* O075 */
void make_group(short, ...);
void groups_merge(void);
void group_switch_next(short);
void swap_heroes_dialog(void);
void drag_hero_portrait(short);
void grp_split(void);
struct group far * grp_choose(char far *, short, short, short);
void grp_menu_entry(struct group far *, short);
struct group far * grp_select(char far *);

/* O076 */
char far * hero_name_gen(struct hero far *, short);
short is_hero_available_in_group(struct hero far *);
void sub_ae_splash(struct hero far *, short);
void add_hero_ae(struct hero far *, short);
void sub_hero_le(struct hero far *, short);
void hunger_thirst_damage(struct hero far *);
void add_hero_le(struct hero far *, short);
short test_attrib(struct hero far *, short, short);
short test_attrib3(struct hero far *, short, short, short, signed char);
void add_mr_mod(struct hero far *, signed char);
void calc_mr(struct hero far *, signed char);
void calc_atpa_base(struct hero far *);
short test_skill(struct hero far *, short, signed char);
short portrait_file(struct hero far *);
void far * load_portrait(struct hero far *);
void load_portrait_bmp(struct hero far *, BMP_PTR);
void hero_inc_skill(struct hero far *, short);
void sub_hunger(struct hero far *, short);
void sub_thirst(struct hero far *, short);
void fill_saved_values(struct hero far *, signed char);

/* O077 */
short calc_bp(struct hero far *);
void change_attrib_normal(struct hero far *, short, signed char);
void change_attrib(struct hero far *, short, signed char);
void set_attrib(struct hero far *, short, signed char);
short hero_at(struct hero far *);
short hero_pa(struct hero far *);
short hero_at_weapon(struct hero far *, short);
short hero_pa_weapon(struct hero far *, short);
struct hero far * hero_mr_target(struct hero far *);
void hero_set_target(struct hero far *, struct hero far *);
struct enemy far * enemy_mr_target(struct hero far *);
signed char target_type(struct hero far *);
short hero_save(struct hero far *);
short hero_load(struct hero far *);
signed long hero_take_weapons(struct hero far *);
short hero_find_item_box(struct hero far *, short);
short hero_find_key(struct hero far *, short);
short can_use_item(struct hero far *, short);
short hero_mr_bonus(struct hero far *);

/* O078 */
void far * Chest_ctor(void far *, short, short);
void Chest_dtor(void far *, short);
void chest_init(void far *);
signed short chest_try_open(void far *);
signed short chest_pick(void far *);
signed short chest_count_items(void far *);
signed short chest_foramen(void far *);
signed short chest_force_open(void far *);
void chest_take_items(void far *);
signed short chest_list_items(void far *, char far *);
void chest_show_msg(void far *, signed char);
void chest_set_item(void far *, signed short, unsigned char, signed short);

/* O079 */
struct hero far * fig_choose_next_hero(struct group far *);
struct hero far * fig_get_first_active_hero(struct group far *);
short hero_is_inactive(struct hero far *);
short hero_attack_damage(struct hero far *, void far *, short);

/* O080 */
short hand_weapon_class(struct hero far *);
void set_asleep(struct hero far *, signed char);
void set_unconscious(struct hero far *, signed char);
short scared_hero_fled(struct group far *);
short fight_round_heroes(struct group far *);
void fight_end_heroes(struct group far *);
void group_set_fb5(struct group far *, signed char);
void group_set_fb6_b0(struct group far *, signed char);
short hand_weapon_class_melee(struct hero far *);

/* O081 */
void disease_effect(struct hero far *);

/* O082 */
void rabies_effect(struct hero far *, char far * far *);

/* O083 */
short select_item(struct hero far *, char far *, short);
short move_item(struct hero far *, short, short);
short drop_item_at(struct hero far *, short);
short set_item_broken(struct hero far *, short, signed char);
short hero_find_food(struct hero far *, short far *, short far *);
short hero_find_drink(struct hero far *, short far *, short far *);
void hero_dec_weapons_bf(struct hero far *);
short get_full_waterskin_pos(struct hero far *);
short has_hero_equipped(struct hero far *, short);
short make_item_list(struct hero far *, short, struct item_entry far *);
short can_hero_use_item(struct hero far *, short);
short can_item_at_pos(struct hero far *, short, short);
short two_hand_collision(struct hero far *, short, short);
short has_hero_stacked(struct hero far *, short);
void unequip(struct hero far *, short, short);
void add_equip_boni(struct hero far *, struct hero far *, short, short, short);
void loose_random_item(struct hero far *, short, char far *, char far *);

/* O084 */
short hero_weapon_type2(struct hero far *);
short hero_weapon_type(struct hero far *);
short hero_mark_weapon(struct hero far *, char far *, short);
short hero_remove_item(struct hero far *, short);
short hero_count_item(struct hero far *, short);
void print_item_description(struct hero far *, short);
void hero_add_rs_lost(struct hero far *, short, signed char);
void hero_add_inv_f10(struct hero far *, short, signed char);
short hero_weapon_state(struct hero far *, short);
void startup_equipment(struct hero far *, short);

/* O085 */
short drop_item(struct hero far *, short, short);
short give_hero_new_item(struct hero far *, short, short, short);
void give_item_at(struct hero far *, short, short, short);
short hero_take_key(struct hero far *, short, short);
short pass_item(struct hero far *, struct hero far *, short, short);

/* O086 */
void lvl_redraw(void far *);
void lvl_inc_attrib(void far *);
void lvl_dec_attrib(void far *);
void lvl_inc_le(void far *);
void lvl_inc_mr(void far *);
void lvl_auto_skills(void far *);
void lvl_auto_spells(void far *);
void lvl_inc_points(void far *);
void Lvl_run(void far *, struct hero far *);

/* O087 */
void lvl_inc_spell(void far *, short);
void lvl_inc_skill(void far *, short);
short can_inc_skills(struct hero far *);
short can_inc_spells(struct hero far *);
void lvl_inc_skill_auto(void far *, short);
void lvl_inc_spell_auto(void far *, short);

/* O088 */
struct hero far * get_random_hero(struct group far *);
struct hero far * get_random_hero2(struct group far *);
struct hero far * get_first_active_hero(struct group far *);
struct hero far * get_first_hero(struct group far *);
struct hero far * get_second_active_hero(struct group far *);
struct hero far * get_active_hero(struct group far *, char far *);
struct hero far * get_active_mage(struct group far *, char far *);
struct hero far * get_any_hero(struct group far *, char far *);
struct hero far * group_get_npc(struct group far *);
struct hero far * get_mage_hero(struct group far *);
struct hero far * get_next_hero(struct group far *, struct hero far *);
struct hero far * get_prev_hero(struct group far *, struct hero far *);
struct hero far * get_best_hero_skill(struct group far *, short);
struct hero far * get_worst_hero_skill(struct group far *, short);
struct hero far * get_hero_attrib_max(struct group far *, short);
struct hero far * get_hero_attrib_min(struct group far *, short);
struct hero far * get_hero_staffspell(struct group far *, signed char);
struct hero far * get_random_alive_hero(struct group far *);
struct hero far * get_most_wounded_hero(struct group far *);
struct hero far * get_active_hero_forced(struct group far *, char far *);
struct hero far * get_dead_hero(struct group far *);
struct hero far * get_renegade_hero(struct group far *);
struct hero far * get_diseased_hero(struct group far *);
struct hero far * get_next_mage(struct group far *, struct hero far *);
struct hero far * get_prev_mage(struct group far *, struct hero far *);
struct hero far * get_random_hero_weapon_type(struct group far *, short);
struct hero far * get_random_hero_with_item(struct group far *, short, short);
struct hero far * get_hero_rs_bonus_min(struct group far *);
struct hero far * get_hero_rs_be_max(struct group far *);
struct hero far * get_hero_of_type(struct group far *, signed char);
struct hero far * get_random_asleep_hero(struct group far *);
struct hero far * get_hero_with_item(struct group far *, short);
struct hero far * select_hero_ae(struct group far *, char far *);
struct hero far * select_hero_le(struct group far *, char far *);
struct hero far * get_hero_by_file_name(struct group far *, char far *);
struct hero far * get_hero_max_le(struct group far *);

/* O089 */
void poison_effect(struct hero far *);

/* O090 */
short skill_cure_poison(struct hero far *, signed char);
short skill_cure_disease(struct hero far *, struct hero far *, signed char, short);
short plan_alchemy(struct hero far *);
short skill_appraise(struct hero far *, signed char, short);

/* O091 */
short use_magic(struct hero far *);
short select_spell(struct hero far *, short);
short test_spell(struct hero far *, short, signed char);
short use_spell(struct hero far *, short, short);

/* O092 */
void use_item(struct hero far *, short);
short box_has(struct hero far *, short, short);
void box_set(struct hero far *, short, short, short);
void hero_consume_item(struct hero far *, struct hero far *, short);
void combine_items(struct hero far *, short, short);

/* O093 */
void inv_switch_hero(struct inv_sheet far *);
void inv_draw_spell(struct inv_sheet far *, short, short, short, short, short);
short select_portrait(struct hero far *, char far *);
void inv_menu_page1(struct inv_sheet far *);
void inv_menu_page2(struct inv_sheet far *);
void inv_menu_page3(struct inv_sheet far *);
void inv_menu_page45(struct inv_sheet far *);

/* O094 */
void inv_draw_page1(void far *);
void inv_draw_page2(void far *);
void inv_draw_page3(void far *);
void inv_draw_page4(void far *);
void inv_draw_page5(void far *);
void inv_draw_inventory(void far *);
void inv_draw_body_slots(void far *);
void inv_knapsack_open(void far *);
void inv_knapsack_close(void far *);

/* O095 */
void hero_sheet_loop(struct inv_sheet far *, struct hero far *);
short rename_hero(struct inv_sheet far *);
void inv_draw_skills(struct inv_sheet far *);

/* O096 */
void view_alloc_lists(void);
void view_free_lists(void);
void view_alloc(void);
void view_leave(void);
short view_alloc_face_list(void);
short obj3d_alloc(struct obj3d far *, unsigned short, short);
void obj3d_free(struct obj3d far *);
void view_load_vis_order(void);
void view_init_stars(void);
void view_load_palette(void);
void pal_set_rgb(struct palette far *, unsigned char, unsigned char, unsigned char, unsigned char);
short pal_find_nearest(struct rgb far *, short);

/* O097 */
void view_load_objects(void);
void view_load_dng_objects(void);
short tex_load(struct texture far *, short);
struct texture far * tex_get(char far *);
unsigned char far * obj_read_string(unsigned char far *, char far *);
void obj_load(short, short);
short obj_read(short, struct obj3d far *);
void unused_view_stub(void);

/* O098 */
void far * Nvf_ctor(void far *);
void Nvf_dtor(void far *, short);
void nvfile_set(void far *, void far *, void far *, short, signed char, short far *, short far *, short);
void nvfile_load(void far *, short, void far *, void far *, short, short, short far *, short far *, short);
signed long nvfile_process(void far *, short);
void nvfile_info(void far *, short, short, short far *, short far *);
short nvfile_count(void far *, short);
void nvfile_pal(void far *, short, void far *, unsigned short);
short nvfile_dims(void far *, short, short far *, short far *);

/* O099 */
void store_deposit(STORE_PTR);
signed short store_deposit_check(STORE_PTR);
void store_deposit_commit(STORE_PTR);
void store_deposit_draw_grid(STORE_PTR, struct hero far *);
void store_deposit_draw_item(STORE_PTR, signed short, struct hero far *);
void store_deposit_fill(STORE_PTR, struct hero far *);
void store_deposit_mark_hero(STORE_PTR);

/* O100 */
void store_fetch(STORE_PTR);
void store_fetch_commit(STORE_PTR);
signed short store_fetch_check(STORE_PTR, signed short far *);
void store_fetch_draw_grid(STORE_PTR);
void store_fetch_fill(STORE_PTR);

/* O101 */
void talk(struct talk_desc far *, short, short, short);
void talk_load(struct talk_desc far *, short);
void talk_format(struct talk_desc far *);

/* O102 */
void talk_special(struct talk_desc far *);

/* O103 */
void Diary_ctor(void far *);
short diary_add(void far *, void far *, short);
void diary_lock(void far *);
void diary_unlock(void far *);
void diary_run(void far *);
void diary_write(void far *);
void diary_search(void far *);
void diary_print_menu(void far *);

/* O104 */
short diary_append(void far *, char far *, short);
void diary_mark_names(void far *);
void diary_mark_word(void far *, struct sel_rec far *, char far *, char far *, short far *);
void diary_show_page(void far *, unsigned short);
void diary_draw(void far *, short, void far *);
short diary_load_page(void far *, unsigned short);
void diary_place_str(void far *, char far *);
void diary_date_str(void far *, char far *);
short diary_count_lines(void far *, char far *);

/* O105 */
void diary_encrypt(DIARY_PTR, short);
void diary_decrypt(DIARY_PTR, short);
void diary_open(DIARY_PTR);
void diary_close(DIARY_PTR);
short diary_chapters(DIARY_PTR);
void diary_hero_names(DIARY_PTR);
void diary_print_pages(DIARY_PTR, unsigned short, unsigned short);
void diary_print_page(DIARY_PTR);
void diary_print_centered(DIARY_PTR, char far *);
void diary_print_rule(DIARY_PTR);
void diary_wrap(DIARY_PTR, char far *);
void diary_print_text(DIARY_PTR, char far *, short);
void diary_print_line(DIARY_PTR, char far *);

/* O106 */
void savegame_save_cur(SAVEMENU_PTR);
short savegame_write(SAVEMENU_PTR, char far *);
short savegame_seek_block(SAVEMENU_PTR, signed long, short);
short savegame_read(SAVEMENU_PTR, char far *);
short savegame_load_cur(SAVEMENU_PTR);
short savegame_load(SAVEMENU_PTR, char far *);
short savegame_write_header(SAVEMENU_PTR);
short savegame_read_header(SAVEMENU_PTR);
void savegame_delete(SAVEMENU_PTR, short);
short savegame_check(SAVEMENU_PTR, char far *, char far *);

/* O107 */
short savegame_menu(SAVEMENU_PTR, short);
void savemenu_input(SAVEMENU_PTR);
short savemenu_cmp(const void far *, const void far *);
short savemenu_scan(SAVEMENU_PTR);
void savemenu_draw_list(SAVEMENU_PTR);
void savemenu_select_cur(SAVEMENU_PTR);
void savemenu_scroll_down(SAVEMENU_PTR);
void savemenu_scroll_up(SAVEMENU_PTR);
short savegame_write_desc(SAVEMENU_PTR);
short savegame_read_desc(SAVEMENU_PTR);
short savemenu_write_desc(SAVEMENU_PTR, short);
short savemenu_read_desc(SAVEMENU_PTR, short);
short savemenu_exists(SAVEMENU_PTR, char far *);
short savemenu_new(SAVEMENU_PTR);
void savegame_make_name(char far *, char far *);
void savemenu_update(SAVEMENU_PTR);
void savemenu_clear_desc(SAVEMENU_PTR);
void savemenu_touch(SAVEMENU_PTR);

/* O108 */
void far * Lock_ctor(void far *, short);
void Lock_dtor(void far *, short);
void lock_init(void far *);
short lock_try_open(void far *);
short lock_pick(void far *);
short lock_claudibus(void far *);
short lock_foramen(void far *);
short lock_force_open(void far *);
short lock_use_key(void far *);
short lock_trap(void far *);

/* O109 */
void o109_init_stub(void);

/* O112 */
void seq_set(FIGANIM_PTR, struct ani_seq far *);
short seq_done(FIGANIM_PTR);
short fig_anim_draw(FIGANIM_PTR, void far *, short, short, signed char, short far *, short far *, short far *, short far *, short far *, short);
short fig_anim_step(FIGANIM_PTR);
void fig_anim_reset(FIGANIM_PTR);
void fig_anim_clip(FIGANIM_PTR, signed char, short, short, short);

/* O113 */
void tevent_000(TRAVEL_PTR);
void tevent_001(TRAVEL_PTR);
void tevent_002(TRAVEL_PTR);
void tevent_003(TRAVEL_PTR);
void tevent_004(TRAVEL_PTR);
void tevent_005(TRAVEL_PTR);
void tevent_006(TRAVEL_PTR);
void tevent_007(TRAVEL_PTR);
void tevent_008(TRAVEL_PTR);

/* O114 */
void tevent_009(TRAVEL_PTR);
void tevent_010(TRAVEL_PTR);
void tevent_012(TRAVEL_PTR);
void tevent_013(TRAVEL_PTR);
void tevent_014(TRAVEL_PTR);
void tevent_016(TRAVEL_PTR);
void tevent_017(TRAVEL_PTR);
void tevent_019(TRAVEL_PTR);
void tevent_020(TRAVEL_PTR);
void tevent_022(TRAVEL_PTR);
void tevent_023(TRAVEL_PTR);
void tevent_024(TRAVEL_PTR);
void tevent_025(TRAVEL_PTR);
void tevent_030(TRAVEL_PTR);
void tevent_031(TRAVEL_PTR);
void tevent_032(TRAVEL_PTR);
void tevent_034(TRAVEL_PTR);
void tevent_035(TRAVEL_PTR);
void tevent_036(TRAVEL_PTR);

/* O115 */
void tevent_037(TRAVEL_PTR);
void tevent_038(TRAVEL_PTR);
void tevent_039(TRAVEL_PTR);
void tevent_041(TRAVEL_PTR);
void tevent_042(TRAVEL_PTR);
void tevent_043(TRAVEL_PTR);
void tevent_044(TRAVEL_PTR);
void tevent_045(TRAVEL_PTR);
void tevent_046(TRAVEL_PTR);
void tevent_048(TRAVEL_PTR);

/* O116 */
void tevent_049(TRAVEL_PTR);
void tevent_050(TRAVEL_PTR);
void tevent_051(TRAVEL_PTR);
void tevent_052(TRAVEL_PTR);
void tevent_053(TRAVEL_PTR);
void tevent_054(TRAVEL_PTR);
void tevent_055(TRAVEL_PTR);
void tevent_059(TRAVEL_PTR);
void tevent_061(TRAVEL_PTR);
void tevent_062(TRAVEL_PTR);
void tevent_063(TRAVEL_PTR);
void tevent_064(TRAVEL_PTR);

/* O117 */
void tevent_066(TRAVEL_PTR);
void tevent_067(TRAVEL_PTR);
void tevent_069(TRAVEL_PTR);
void tevent_070(TRAVEL_PTR);
void tevent_071(TRAVEL_PTR);
void tevent_072(TRAVEL_PTR);
void tevent_073(TRAVEL_PTR);
void tevent_075(TRAVEL_PTR);
void tevent_076(TRAVEL_PTR);
void tevent_077(TRAVEL_PTR);
void tevent_078(TRAVEL_PTR);

/* O118 */
void tevent_081(TRAVEL_PTR);
void tevent_082(TRAVEL_PTR);
void tevent_083(TRAVEL_PTR);
void tevent_084(TRAVEL_PTR);
void tevent_085(TRAVEL_PTR);
void tevent_086(TRAVEL_PTR);
void tevent_087(TRAVEL_PTR);
void tevent_088(TRAVEL_PTR);
void tevent_089(TRAVEL_PTR);
void tevent_092(TRAVEL_PTR);
void tevent_093(TRAVEL_PTR);
void tevent_094(TRAVEL_PTR);
void tevent_096(TRAVEL_PTR);
void tevent_097(TRAVEL_PTR);
void tevent_098(TRAVEL_PTR);
void tevent_099(TRAVEL_PTR);
void unused_tevent_o118(void);

/* O119 */
void tevent_100(TRAVEL_PTR);
void tevent_101(TRAVEL_PTR);
void tevent_103(TRAVEL_PTR);
void tevent_104(TRAVEL_PTR);
void tevent_105(TRAVEL_PTR);
void tevent_106(TRAVEL_PTR);
void tevent_108(TRAVEL_PTR);
void tevent_109(TRAVEL_PTR);
void tevent_110(TRAVEL_PTR);
void tevent_111(TRAVEL_PTR);
void tevent_112(TRAVEL_PTR);
void tevent_113(TRAVEL_PTR);
void tevent_114(TRAVEL_PTR);
void tevent_116(TRAVEL_PTR);
void tevent_117(TRAVEL_PTR);
void tevent_119(TRAVEL_PTR);

/* O120 */
void tevent_121(TRAVEL_PTR);
void tevent_122(TRAVEL_PTR);
void tevent_123(TRAVEL_PTR);
void tevent_125(TRAVEL_PTR);
void tevent_126(TRAVEL_PTR);
void tevent_127(TRAVEL_PTR);
void tevent_128(TRAVEL_PTR);
void tevent_129(TRAVEL_PTR);
void tevent_131(TRAVEL_PTR);
void tevent_132(TRAVEL_PTR);
void tevent_133(TRAVEL_PTR);
void tevent_134(TRAVEL_PTR);
void tevent_135(TRAVEL_PTR);
void tevent_136(TRAVEL_PTR);
void tevent_137(TRAVEL_PTR);
void tevent_138(TRAVEL_PTR);

/* O121 */
void tevent_140(TRAVEL_PTR);
void tevent_141(TRAVEL_PTR);
void tevent_142(TRAVEL_PTR);
void tevent_143(TRAVEL_PTR);
void tevent_144(TRAVEL_PTR);
void tevent_145(TRAVEL_PTR);
void tevent_146(TRAVEL_PTR);
void tevent_147(TRAVEL_PTR);
void tevent_148(TRAVEL_PTR);
void tevent_149(TRAVEL_PTR);
void tevent_151(TRAVEL_PTR);
void tevent_152(TRAVEL_PTR);
void tevent_153(TRAVEL_PTR);
void tevent_154(TRAVEL_PTR);

/* O122 */
void tevent_157(TRAVEL_PTR);
void tevent_158(TRAVEL_PTR);
void tevent_159(TRAVEL_PTR);
void tevent_160(TRAVEL_PTR);
void tevent_161(TRAVEL_PTR);
void tevent_162(TRAVEL_PTR);
void tevent_163(TRAVEL_PTR);
void tevent_164(TRAVEL_PTR);
void tevent_165(TRAVEL_PTR);
void tevent_166(TRAVEL_PTR);
void tevent_167(TRAVEL_PTR);
void tevent_168(TRAVEL_PTR);
void tevent_170(TRAVEL_PTR);
void tevent_171(TRAVEL_PTR);
void tevent_172(TRAVEL_PTR);
void tevent_174(TRAVEL_PTR);
void tevent_175(TRAVEL_PTR);
void tevent_177(TRAVEL_PTR);
void tevent_178(TRAVEL_PTR);
void tevent_179(TRAVEL_PTR);
void tevent_181(TRAVEL_PTR);
void tevent_182(TRAVEL_PTR);
void tevent_183(TRAVEL_PTR);
void tevent_185(TRAVEL_PTR);
void tevent_186(TRAVEL_PTR);
void tevent_187(TRAVEL_PTR);
void tevent_188(TRAVEL_PTR);
void tevent_190(TRAVEL_PTR);
void unused_tevent_o122(TRAVEL_PTR);

/* O123 */
void tevent_191(TRAVEL_PTR);
void tevent_192(TRAVEL_PTR);
void tevent_193(TRAVEL_PTR);
void tevent_195(TRAVEL_PTR);
void tevent_198(TRAVEL_PTR);
void tevent_200(TRAVEL_PTR);
void tevent_201(TRAVEL_PTR);
void tevent_202(TRAVEL_PTR);
void tevent_204(TRAVEL_PTR);
void tevent_205(TRAVEL_PTR);
void tevent_209(TRAVEL_PTR);
void tevent_211(TRAVEL_PTR);
void tevent_213(TRAVEL_PTR);
void tevent_214(TRAVEL_PTR);
void tevent_217(TRAVEL_PTR);
void tevent_218(TRAVEL_PTR);
void tevent_221(TRAVEL_PTR);

/* O124 */
void tevent_225(TRAVEL_PTR);
void tevent_228(TRAVEL_PTR);
void tevent_229(TRAVEL_PTR);
void tevent_230(TRAVEL_PTR);
void tevent_234(TRAVEL_PTR);
void tevent_238(TRAVEL_PTR);
void tevent_239(TRAVEL_PTR);
void tevent_242(TRAVEL_PTR);
void tevent_244(TRAVEL_PTR);
void tevent_245(TRAVEL_PTR);
void tevent_246(TRAVEL_PTR);
void tevent_247(TRAVEL_PTR);
void tevent_248(TRAVEL_PTR);
void tevent_250(TRAVEL_PTR);
void tevent_251(TRAVEL_PTR);
void tevent_253(TRAVEL_PTR);
void tevent_254(TRAVEL_PTR);
void tevent_257(TRAVEL_PTR);
void tevent_258(TRAVEL_PTR);
void tevent_259(TRAVEL_PTR);

/* O125 */
void place_event_103(TRAVEL_PTR);
void place_event_104(TRAVEL_PTR);
void place_event_106(TRAVEL_PTR);
void place_event_114(TRAVEL_PTR);
void place_event_117(TRAVEL_PTR);
void place_event_128(TRAVEL_PTR);
void place_event_131(TRAVEL_PTR);
void place_event_142(TRAVEL_PTR);
void place_event_170(TRAVEL_PTR);
void place_event_171(TRAVEL_PTR);
void place_event_183(TRAVEL_PTR);
void place_event_185(TRAVEL_PTR);
void place_event_187(TRAVEL_PTR);
void place_event_189(TRAVEL_PTR);
void place_event_196(TRAVEL_PTR);
void place_event_197(TRAVEL_PTR);
void place_event_198(TRAVEL_PTR);

/* O126 */
void place_event_028(TRAVEL_PTR);
void place_event_030(TRAVEL_PTR);
void place_event_031(TRAVEL_PTR);

/* O127 */
void place_event_021(TRAVEL_PTR);
void place_event_022(TRAVEL_PTR);
void place_event_023(TRAVEL_PTR);
void place_event_024(TRAVEL_PTR);
void place_event_025(TRAVEL_PTR);
void place_event_026(TRAVEL_PTR);
void place_event_027(TRAVEL_PTR);
void place_event_029(TRAVEL_PTR);
void place_event_032(TRAVEL_PTR);

/* O128 */
void fight_hero_turn(FIGHT_PTR);
void fight_enemy_turn(FIGHT_PTR);
void fight_do_round(FIGHT_PTR);

/* O129 */
void Event_dtor(void far *, short);
void event_set_cell(void far *, short, short, signed char, short, short);
short event_fight1(void far *, short, short, short);
short event_fight2(void far *, short, short, short, short, short, short);
short event_fight(void far *, short, short, short, short, short, short, short, short, short, short, short, short);
void event_set_cell_bits(void far *, short, short, short);
void far * Event_ctor(void far *, short, short, short);
void event_teleport(void far *, short);
void event_notice(void far *, char far *);
void event_dragon_claw(void far *, struct hero far *);

/* O130 */
short dng02_handler(struct area_events far *, struct map_event far *);
short dng02_fights(struct area_events far *, short);
void dng02_amulet_timers(struct area_events far *);
short dng02_door(struct area_events far *, short, short, short);
void dng02_amulet_part1(struct area_events far *);
void dng02_amulet_part2(struct area_events far *);
void dng02_amulet_part3(struct area_events far *);
void dng02_amulet_part4(struct area_events far *);

/* O131 */
void dng03_stone_thief(void far *);
short dng03_handler(struct area_events far *, struct map_event far *);
short dng03_fights(struct area_events far *, short);

/* O132 */
void dng01_entry_fight(void far *);
void dng01_melt_ice(void far *);
void dng01_move_groups(short, short, short, short);
signed char dng01_black_statue(struct area_events far *);
void dng01_scatter_trap(struct area_events far *);
short dng01_handler(struct area_events far *, struct map_event far *);
short dng01_door(struct area_events far *, short, short, short, struct map_event far *);
void dng01_document(struct area_events far *);
void dng01_ice_warrior(struct area_events far *);
void dng01_hesinde_statue(struct area_events far *);
void dng01_ghouls(struct area_events far *);
void dng01_thorn_tree(struct area_events far *);
void dng01_grate(struct area_events far *);
void dng01_vanishing_wall(struct area_events far *, short);
void dng01_branches(struct area_events far *, short);
void dng01_skull_ornament(struct area_events far *);
void dng01_blue_disc(struct area_events far *, short);
void dng01_grate_falls(struct area_events far *, signed char);

/* O133 */
void fig_anim_event(FIGHT_PTR, struct fig_node far *, unsigned char);

/* O134 */
void fight_quick_stop(FIGHT_PTR);
void fight_quick_end(FIGHT_PTR, short);
void fight_timer_stop(void);
void fight_timer_start(void);
void fight_quick_start(FIGHT_PTR);
FIGANIM_PTR fight_new_extra_anim(void far *);

/* O135 */
void fig_turn(FIGHT_PTR, struct hero far *, short, short, short far *, struct ani_seq far *, short);
FIGANIM_PTR fig_stand_pic(FIGHT_PTR, struct hero far *, struct ani_seq far *);
FIGANIM_PTR fig_hero_attack_seq(FIGHT_PTR, struct hero far *, struct ani_seq far *);
FIGANIM_PTR fig_hero_defend_seq(FIGHT_PTR, struct hero far *, struct ani_seq far *, short);

/* O136 */
FIGANIM_PTR fig_stand_seq(FIGHT_PTR, struct enemy far *, struct ani_seq far *);
void fig_enemy_turn(FIGHT_PTR, struct enemy far *, signed char far *, short, short far *, void far *, short);
FIGANIM_PTR fig_enemy_attack_seq(FIGHT_PTR, struct enemy far *, struct ani_seq far *);
FIGANIM_PTR fig_enemy_defend_seq(FIGHT_PTR, struct enemy far *, struct ani_seq far *, short);

/* O137 */
FIGANIM_PTR fig_hit_pic(FIGHT_PTR, struct hero far *, struct ani_seq far *);
FIGANIM_PTR fig_hero_shoot_seq(FIGHT_PTR, struct hero far *, struct ani_seq far *);
FIGANIM_PTR fig_hero_spell_seq(FIGHT_PTR, struct hero far *, struct ani_seq far *);
FIGANIM_PTR fig_hero_spell_hit_seq(FIGHT_PTR, struct hero far *, struct ani_seq far *, short, short);
FIGANIM_PTR fig_hero_pose_seq(FIGHT_PTR, struct hero far *, struct ani_seq far *);
FIGANIM_PTR fig_hero_use_seq(FIGHT_PTR, struct hero far *, struct ani_seq far *);
FIGANIM_PTR fig_hero_throw_seq(FIGHT_PTR, struct hero far *, struct ani_seq far *, short);

/* O138 */
FIGANIM_PTR fig_enemy_hit_seq(FIGHT_PTR, struct enemy far *, struct ani_seq far *);
FIGANIM_PTR fig_enemy_shoot_seq(FIGHT_PTR, struct enemy far *, struct ani_seq far *);
FIGANIM_PTR fig_enemy_spell_seq(FIGHT_PTR, struct enemy far *, struct ani_seq far *);
FIGANIM_PTR fig_enemy_spell_hit_seq(FIGHT_PTR, struct enemy far *, struct ani_seq far *, short, short);

/* O139 */
short fight_env_cell(FIGHT_PTR, short, short);
void fight_env_scan(FIGHT_PTR);
void fight_load_grid(FIGHT_PTR);
short fight_wall_shape(FIGHT_PTR, short);
short fight_path_to_start(FIGHT_PTR, short, short);
short fight_block_unreachable(FIGHT_PTR);
short fight_is_wall(FIGHT_PTR, short, short);
short fight_is_obj(FIGHT_PTR, short, short);
void fight_build_field(FIGHT_PTR);

/* O140 */
void fight_setup_env(FIGHT_PTR);
void fight_place_hero(FIGHT_PTR, signed char far *, signed char far *, signed char far *);
short fight_nearest_hero(FIGHT_PTR, signed char, signed char, signed char far *, signed char far *);
void fight_place_enemy(FIGHT_PTR, short, signed char far *, signed char far *, signed char far *, short);
short fight_nearest_enemy(FIGHT_PTR, signed char, signed char, signed char far *, signed char far *);
void fight_heroes_face_enemies(FIGHT_PTR);
void fight_make_field(FIGHT_PTR);
void fight_entry_side(FIGHT_PTR);

/* O141 */
void sfight_enemy_action(FIGHT_PTR);
void sfight_fig_pos(FIGHT_PTR, short, short far *, short far *);
void sfight_setup(FIGHT_PTR);
void sfight_place_enemy(FIGHT_PTR, struct enemy far *);
void sfight_do_action(FIGHT_PTR, struct enemy far *);
void sfight_attack(FIGHT_PTR, struct enemy far *, struct hero far *);
void sfight_area_attack(FIGHT_PTR, struct enemy far *, struct hero far *);
void sfight_special_attack(FIGHT_PTR, struct enemy far *);

/* O142 */
FIGANIM_PTR sfight_enemy_reset(FIGHT_PTR, struct ani_seq far *);
FIGANIM_PTR sfight_enemy_move(FIGHT_PTR, struct ani_seq far *, short);
FIGANIM_PTR sfight_enemy_end_seq(FIGHT_PTR, struct ani_seq far *);
FIGANIM_PTR sfight_seq_action(FIGHT_PTR, struct ani_seq far *, short);
FIGANIM_PTR sfight_seq_walk_random(FIGHT_PTR, struct ani_seq far *);
FIGANIM_PTR sfight_seq_area_attack(FIGHT_PTR, struct ani_seq far *);
FIGANIM_PTR sfight_seq_special(FIGHT_PTR, struct ani_seq far *);
FIGANIM_PTR sfight_seq_spell(FIGHT_PTR, struct ani_seq far *, short);
void sfight_special_picture(FIGHT_PTR);

/* O143 */
void fight_finish(FIGHT_PTR, short far *);
void fight_split_escaped(FIGHT_PTR);
void fig_loot_monsters(FIGHT_PTR);
void fig_split_ap(FIGHT_PTR);

/* O144 */
short fight_draw(FIGHT_PTR);
void fight_add_rect(short, short, short, short);
short all_heroes_down(struct group far *);
void fight_check_keys(FIGHT_PTR);
void fight_restore_bg(FIGHT_PTR);
void fight_draw_hero_box(FIGHT_PTR, short, struct hero far *);
void fight_draw_enemy_box(FIGHT_PTR, short, struct enemy far *);
short fight_rect_overlap(FIGHT_PTR, short, short, short, short, short, short, short, short);
void fight_draw_flush(FIGHT_PTR);
void fight_add_obj_rect(FIGHT_PTR, struct fig_rect far *, unsigned char, short, short, short, short, short, signed char);
void fight_draw_figures(FIGHT_PTR);
void fight_anim_tick(FIGHT_PTR);
void fight_recolor(char far *, short, signed char);
short fight_printer(struct fight_msgs far *);

/* O145 */
void fig_mark_hero_targets(FIGHT_PTR, short, short, short);
void fig_mark_enemy_targets(FIGHT_PTR, short, short, short);
void fig_prepare_search(FIGHT_PTR, short);
short bresenham_line(short, short, short, short, struct point far *);
short fig_line_free(FIGHT_PTR, short, short, struct point far *, short);
short fig_find_sfight_cell(FIGHT_PTR, short, short, short far *, short far *, short);
short fig_line_of_sight(FIGHT_PTR, short, short, short, short, short);
unsigned char fig_cb_select_target(FIGHT_PTR, short far *, short far *, short);

/* O146 */
unsigned char fight_path_search(FIGHT_PTR, void far *, short, short, short, short);
void fig_find_path_backtrack(FIGHT_PTR, unsigned char far *, short, short, short, signed char, short, short, short);
short fig_count_dir_changes(FIGHT_PTR, signed char far *);
short fig_path_add_target(FIGHT_PTR, short far *, short far *, short far *, short far *, unsigned char far *, short, short, unsigned char);

/* O147 */
void fight_status_text(char far *);
void screen_flash(void);
short fig_fight_continues(FIGHT_PTR);
short fig_round_enemies(FIGHT_PTR, short);
void fig_tidy_monsters(FIGHT_PTR);
short fig_is_cell_89_8c(short);
short place_obj_on_cb(FIGHT_PTR, short, short, unsigned char, unsigned char, signed char);
void fight_latecomers(FIGHT_PTR);
short fig_all_heroes_escaped(FIGHT_PTR);
void fig_find_free_cell(FIGHT_PTR, short, short, short far *, short far *, short, short);
short fig_count_active_enemies(FIGHT_PTR);
void fig_clear_hits(void);

/* O148 */
void fight_msg_names(FIGHT_PTR, struct hero far *, struct hero far *);
struct enemy far * fig_choose_next_enemy(FIGHT_PTR);
void fig_next_side(FIGHT_PTR, signed char far *, short far *, short, short);
void fig_draw_hero_bar(FIGHT_PTR);
short fig_enemies_le_sum(FIGHT_PTR);
short fig_enemies_ae_sum(FIGHT_PTR);
short fig_obj_transparent(FIGHT_PTR, short);
void fig_set_mark(FIGHT_PTR, signed char);

/* O149 */
void fig_hero_place(FIGHT_PTR, struct hero far *);
void fig_enemy_move(FIGHT_PTR, struct enemy far *);

/* O150 */
short fig_move_pathlen(FIGHT_PTR);
void fig_move_check(FIGHT_PTR, struct hero far *, short far *, short far *, short far *, short far *, short far *, short far *);
void fig_move_hero(FIGHT_PTR, struct hero far *, short far *, short far *);

/* O151 */
short get_foe_attack_mode(void far *, short, short);
short enemy_spell_target_ok(struct enemy far *, short);

/* O152 */
short enemy_turn(FIGHT_PTR, struct enemy far *, short, short);
short test_foe_melee_attack(FIGHT_PTR, short, short, short, short, short);
short enemy_spell_attack(FIGHT_PTR, struct enemy far *, short, short, short);
short enemy_range_attack(FIGHT_PTR, struct enemy far *, short, short, short);

/* O153 */
void fig_list_hide(void far *, signed char);
void fig_list_show(void far *, signed char);
void fig_list_resort(void far *);
void fig_list_purge(void far *);
short fig_list_exists(void far *, unsigned char);
struct fig_node far * fig_list_find(void far *, signed char);
struct fig_node far * fig_list_find_anim(void far *, void far *);
struct fig_node far * fig_list_find_twofield(void far *, signed char);
void fig_list_getpos(void far *, signed char, short far *, short far *);
void fig_list_remove(void far *, signed char, short);
signed char fig_list_add(void far *, short);
short manhattan_dist(short, short, short, short);
void fight_set_cell(void far *, short, short, unsigned char);
short fight_find_cell(signed char, short far *, short far *);
short fight_cell_blocked(short, short);
unsigned char fight_obj_index(struct enemy far *);
short fight_check_state(struct enemy far *);
short enemy_is_twofielded(struct enemy far *);
short fight_dir_hero(void far *, struct hero far *);
short fight_dir_enemy(void far *, struct enemy far *);
short fight_hero_pic(struct hero far *);
void far * FigList_ctor(void far *);
void FigList_dtor(void far *, short);
void fig_list_clear(void far *);
void fig_list_mark_remove(void far *, unsigned char);
void fig_list_mark_resort(void far *, unsigned char);
struct fig_node far * fig_list_last(void far *);
unsigned char fig_list_alloc_id(void far *);
short fig_enemy_pic(struct enemy far *);
short enemy_is_active(struct enemy far *);
short enemy_is_active_or_asleep(struct enemy far *);
short enemy_is_renegade_on_field(struct enemy far *);
short fig_turn_steps(void far *, short, short);
short dir_opposite(void far *, short);
short fight_dir_hero_straight(void far *, struct hero far *);
short fight_dir_enemy_straight(void far *, struct enemy far *);
struct hero far * fig_get_hero(struct group far *, signed char);

/* O154 */
short fig_menu(FIGHT_PTR, struct hero far *, short, short);
void fig_menu_move(FIGHT_PTR, struct hero far *, short far *, short far *);
short fig_menu_attack(FIGHT_PTR, struct hero far *, short, short);

/* O155 */
short fig_menu_guard(FIGHT_PTR, struct hero far *);
short fig_menu_spell(FIGHT_PTR, struct hero far *, short, short);
short fig_menu_use_item(FIGHT_PTR, struct hero far *, short, short);
void fig_menu_take_item(FIGHT_PTR, struct hero far *);
void fig_menu_change_weapon(FIGHT_PTR, struct hero far *);
void fig_menu_show_values(FIGHT_PTR, struct hero far *);
void fig_menu_drop_item(FIGHT_PTR, struct hero far *);
short fig_menu_repeat(FIGHT_PTR, struct hero far *);
short fig_target_in_reach(FIGHT_PTR, struct hero far *);

/* O156 */
short ki_change_hero_weapon(FIGHT_PTR, struct hero far *);
short ki_target_valid(FIGHT_PTR, short, short);
short ki_best_target(unsigned char far *, short);
signed char ki_search_target(FIGHT_PTR, short, short, short, short, short);
short ki_range_target(FIGHT_PTR, struct hero far *, short, short, short);
short ki_spell_mode(FIGHT_PTR, short, short);
short ki_spell_useful(FIGHT_PTR, struct hero far *, short);
signed char ki_select_spell_target(FIGHT_PTR, struct hero far *, short, short, short);

/* O157 */
short range_attack_check_ammo(FIGHT_PTR, struct hero far *, short);
void ki_hero(FIGHT_PTR, struct hero far *, short, short);

/* O158 */
void fig_hero_attack_hero(FIGHT_PTR, struct hero far *, struct hero far *);
void fig_hero_shoot_hero(FIGHT_PTR, struct hero far *, struct hero far *);
void fig_hero_spell_hero(FIGHT_PTR, struct hero far *, struct hero far *);

/* O159 */
void fig_do_hero_action(FIGHT_PTR, struct hero far *);
void fig_hero_attack_enemy(FIGHT_PTR, struct hero far *, struct enemy far *, signed char);
void fig_hero_shoot_enemy(FIGHT_PTR, struct hero far *, struct enemy far *);
void fig_hero_spell_enemy(FIGHT_PTR, struct hero far *, struct enemy far *);

/* O160 */
void fig_enemy_attack_enemy(FIGHT_PTR, struct enemy far *, struct enemy far *, short);
void fig_enemy_shoot_enemy(FIGHT_PTR, struct enemy far *, struct enemy far *);
void fig_enemy_spell_enemy(FIGHT_PTR, struct enemy far *, struct enemy far *);

/* O161 */
void fig_do_enemy_action(FIGHT_PTR, struct enemy far *);
void fig_enemy_attack_hero(FIGHT_PTR, struct enemy far *, struct hero far *);
void fig_enemy_shoot_hero(FIGHT_PTR, struct enemy far *, struct hero far *);
void fig_enemy_spell_hero(FIGHT_PTR, struct enemy far *, struct hero far *);

/* O162 */
void fig_use_item_hero(FIGHT_PTR, struct hero far *, struct hero far *);
void fig_use_item_enemy(FIGHT_PTR, struct hero far *, struct enemy far *);
void fig_hidden_enemies_text(FIGHT_PTR);

/* O163 */
void deer_hunt(TRAVEL_PTR);
signed char flag_not(TRAVEL_PTR, signed char);
void ford_crossing(TRAVEL_PTR);
void ferry_crossing(TRAVEL_PTR, signed short, signed short);
void place_event_nop(TRAVEL_PTR);
void swamp_path(TRAVEL_PTR, signed char);
void orc_camp_luggage(TRAVEL_PTR);

/* O164 */
void guide_offer(TRAVEL_PTR, signed char);
void random_encounter(TRAVEL_PTR, signed char, signed char);
void show_once_anim(TRAVEL_PTR);
signed char group_has_special_item(void);
void travel_scene_31(void);
signed char shrine_donation(void);
void armed_hero_fight(TRAVEL_PTR, signed char);
void npc6_encounter(TRAVEL_PTR);
void spring_event(void);

/* O165 */
void valley_ambush(TRAVEL_PTR);
void mountain_rope(TRAVEL_PTR);
void group_damage(TRAVEL_PTR, signed char, signed char, signed char, signed short, signed short);
signed short rockslide_dig(TRAVEL_PTR);
void avalanche(TRAVEL_PTR);

/* O166 */
short town_sites_event(TOWN_PTR, struct map_event far *);
void Sites_ctor(void far *);
void Sites_dtor(void far *, short);
void town_sites_shock(TOWN_PTR);
void town_sites_hint(TOWN_PTR);
void sites_inn_vision(void far *);
void sites_talk(void far *);
void sites_night_encounter(void far *);
void sites_script_a4(void);
void sites_challenge(void far *);
void sites_rematch(void far *);
void sites_evening_fight(void far *);
void sites_explore(void far *);
void sites_guide_limit(void);

/* O167 */
void chargen_roll_attribs(CHARGEN_PTR);
void chargen_calc_atpa(CHARGEN_PTR);
void chargen_auto_inc(CHARGEN_PTR);
void chargen_set_type_values(CHARGEN_PTR);
void chargen_auto_inc_skill(CHARGEN_PTR, short);
void chargen_auto_inc_spell(CHARGEN_PTR, short);

/* O168 */
void chargen_draw_attribs(CHARGEN_PTR);
void chargen_draw_page(CHARGEN_PTR);
void chargen_change_atpa(CHARGEN_PTR);

/* O169 */
void chargen_new_hero(CHARGEN_PTR);
void chargen_move_attribs(CHARGEN_PTR);
void chargen_save_bg(CHARGEN_PTR);
void chargen_restore_bg(CHARGEN_PTR, void far *);
void chargen_inc_skill(CHARGEN_PTR, short, short, char far *);
void chargen_toggle_sex(CHARGEN_PTR);

/* O170 */
void chargen_reset_incs(CHARGEN_PTR);
void chargen_inc_spell(CHARGEN_PTR, short);
void chargen_menu_spells(CHARGEN_PTR);
void chargen_menu_skills(CHARGEN_PTR);
void chargen_select_type(CHARGEN_PTR);

/* O171 */
short chargen_save_hero(CHARGEN_PTR);
void chargen_draw_type_pic(CHARGEN_PTR, void far *, short);

/* O172 */
short dlg_select_pic(void far *, short, short);

/* O173 */
void House_ctor(void far *);
void House_dtor(void far *, short);
void house_run(void far *);
short house_hint(void far *);

/* O174 */
signed short healer_is_open(HEALER_PTR);
void healer_run(HEALER_PTR);
void healer_cure_wounds(HEALER_PTR);
void healer_cure_disease(HEALER_PTR);
void healer_cure_poison(HEALER_PTR);
signed long healer_price(HEALER_PTR, signed long);
void healer_cure_stone(HEALER_PTR);
void healer_talk(HEALER_PTR);

/* O175 */
void Lst_ctor(void far *);
void Lst_dtor(void far *, short);
void Lst_set(void far *, void far *);
void Lst_init(void far *);
void Lst_add(void far *, short, short, signed char, struct hero far *, signed char, struct hero far *);
void Lst_add2(void far *, short, short, short, struct hero far *);
void Lst_add0(void far *, short, short);
void Lst_run(void far *);

/* O176 */
signed long dsa1_heroes_pos(short);
signed long dsa1_heroes_pos_roa(short);
short Save_load(void far *, short);
void save_check_hero(void far *, struct hero far *);
void save_undo_timers(void far *, struct dsa1_hero far *, short);
void save_undo_timers_v1(void far *, struct dsa1_hero_v1 far *, short);
short save_find_portrait(void far *, signed char far *, signed char, signed char, short);
void hero_clear_slot(struct hero far *, short);

/* O177 */
void swap16(void far *);
void swap32(void far *);
void bitrev16(unsigned short far *);
void bitrev32(unsigned long far *);
void dsa1_swap_hero_v1(char far *, signed long);
void dsa1_swap_hero(char far *, signed long);
short save_import_heroes(void far *, short, short);
short save_import_heroes_v1(void far *, short, short);

/* O178 */
void inn_eat(INN_PTR);
void inn_room(INN_PTR);
signed short inn_is_open(INN_PTR);
void inn_run(INN_PTR);
void inn_sleep(INN_PTR);
signed short inn_wait_midnight(INN_PTR);
signed short inn_wait_morning(INN_PTR);
void inn_leave(INN_PTR);

/* O179 */
void inn_apply_skill(INN_PTR);
void inn_apply_magic(INN_PTR);
void inn_theft_morning(INN_PTR);
signed short inn_theft_night(INN_PTR);
void inn_talk(INN_PTR);
signed short inn_conv_callback(signed short, signed short);

/* O181 */
void tscr_open_anim(struct text_screen far *, short, short, short);
void tscr_open_texts(struct text_screen far *, short, short);
void tscr_open(struct text_screen far *);
void tscr_close(struct text_screen far *, short);
void tscr_print(struct text_screen far *, char far *, short, short, short);
void tscr_print_txt(struct text_screen far *, short, short, short, short);
void tscr_print_font(struct text_screen far *, char far *, void far *, short, short, short);
void tscr_print_txt_font(struct text_screen far *, short, void far *, short, short, short);
void tscr_set_anim(struct text_screen far *, short, short);

/* O182 */
void Items_ctor(void far *);
void Items_dtor(void far *, short);
void items_change_price(struct item_desc far * far *, short, short);

/* O183 */
void far * Town_ctor(void far *, short, short, short);
void Town_dtor(void far *, short);
void town_enter(void far *, short, void far *);
short town_cell_dir(void far *, void far *);
short town_harbor_event(void far *, void far *);
void town_market_stall(void far *, short);
void town_ship_voyage(void far *);
void town_show_anim3(void far *);
void town_swim_check(void far *);

/* O184 */
void ev_show_pic_197(void far *);
void ev_tribute(void far *);
short ev_story_13(void far *);
void ev_talk_90(void far *);
short ev_count_sneak_fails(void far *, signed char);
void ev_choice_49(void far *);
void ev_choice_56(void far *);
void ev_wait_noon(void far *);

/* O185 */
void ev_gavron_chase(void far *);
void lowangen_drop_food(void);
void lowangen_arrival(signed short);
signed short lowangen_food_thugs(void);
void lowangen_weapon_check(void);

/* O186 */
signed short town_lowangen_event(TOWN_PTR, struct map_event far *);

/* O187 */
void TMap_ctor(void far *);
void TMap_dtor(void far *, short);
void tmap_new_pal(char far *);
short tmap_screen_x(void far *, short);
short tmap_screen_y(void far *, short);
void tmap_free_pal(char far *);
void tmap_set_pal(char far *);
void tmap_load(char far *, short);
void tmap_set_pos(void far *, short, short);
void tmap_draw(void far *);
signed char tmap_follow(char far *, short, short, unsigned char, unsigned char);
void tmap_plot_points(void far *, short far *, unsigned char);
void tmap_mark1_free(void far *);
void tmap_mark2_free(void far *);
void tmap_mark1_anim(void far *, short, short);
void tmap_mark2_anim(void far *, short, short);

/* O188 */
void far * Screen_ctor(void far *);
void screen_draw_base(void far *);
void screen_short_name(char far *, char far *, char far *);
void screen_portraits(void far *);
void screen_clear_portrait(void far *, short);
void screen_load_icon(void far *, short, short, short);
void screen_draw_icons(void far *);
void screen_redraw(void far *);
void screen_clear_textbar(void far *);
void screen_clear_view(void far *);
void screen_draw_names(char far *);
void screen_draw_name(void far *, short, short);
void screen_draw_name_normal(void far *, short);
void screen_textbar(void far *, void far *, unsigned char);
void screen_mark_portrait(signed char far *, short, short);
void draw_loc_icons(void far *, short, ...);
void screen_reset_icons(void far *);

/* O189 */
short dlg_count_lines(void far *, short, void far *);
void dlg_draw_choices(void far *, short, void far *);
void dlg_draw_button(void far *, short, short);
void dlg_mark_line(void far *, short);
short dlg_choose_loop(void far *, short, void far *);
void dlg_click_sound(void far *);
short dlg_run(void far *, void far *, char far *, char far *, short far *, void far *);
void dlg_layout(void far *);
void dlg_wait_message(void far *);
void dlg_calc_text(void far *);

/* O190 */
void far * Dlg_ctor(void far *);
short dlg_yes_no(void far *, char far *);
void dlg_message(void far *, void far *);
short dlg_input(void far *, short, char far *, short, short, short);
short dlg_choose(void far *, char far *, short, ...);
short dlg_box(void far *, void far *, char far *, char far *, short, ...);
short dlg_box_anim(void far *, void far *, char far *, char far *, short, ...);
short dlg_box_bmp(void far *, void far *, char far *, char far *, short, ...);
short dlg_lock(void far *);
void dlg_unlock(void far *);
void dlg_draw_pic(void far *);
void dlg_print_title(void far *);
void dlg_print_body(void far *);
void dlg_print_choices(void far *);
short dlg_message_flag(void far *, short, ...);
void far * Win_ctor(void far *);
void Win_dtor(void far *, short);
short dlg_lock_move(void far *);
void dlg_unlock_move(void far *);
short dlg_set_pos_x(void far *, short);
short dlg_set_pos_y(void far *, short);
short dlg_set16(void far *, short);
short dlg_num_loop(void far *, void far *);
short dlg_overlaps(void far *, short, short, short, short);
void dlg_fill_area(void far *, void far *);
void dlg_dummy(void far *);
void dlg_set_open(void far *);

/* O192 */
short enemy_type_check(struct enemy far *);
struct hero far * enemy_target_hero(struct enemy far *);
struct enemy far * enemy_target_enemy(struct enemy far *);
void enemy_sub_le(struct enemy far *, short, short);
signed char enemy_attack_value(struct enemy far *);
signed char enemy_parade_value(struct enemy far *);
struct enemy far * fig_get_enemy(unsigned char);
short enemy_is_humanoid(struct enemy far *);
void enemy_load_pic(struct enemy far *, void far *);
char far * enemy_name(struct enemy far *);
short enemy_is_gfx_24_18(struct enemy far *);
short enemy_is_gfx_25_35_36_23(struct enemy far *);
short enemy_is_gfx_17_29(struct enemy far *);
short enemy_is_gfx_2a(struct enemy far *);
short enemy_is_gfx_1c(struct enemy far *);
short enemy_is_gfx_42(struct enemy far *);
short enemy_is_fire(struct enemy far *);
void enemy_set_target_hero(struct enemy far *, struct hero far *);
short enemy_cast_spell(struct enemy far *, signed char);
short enemy_range_kind(struct enemy far *);
short enemy_blocks(struct enemy far *);
void far * Enemy_ctor(void far *);
void Enemy_dtor(void far *, short);
short enemy_is_gfx_19_3f(struct enemy far *);
short enemy_is_gfx_1b(struct enemy far *);
short enemy_is_gfx_1f(struct enemy far *);
short enemy_is_gfx_15(struct enemy far *);
short enemy_is_gfx_33_mon_7a(struct enemy far *);
short enemy_is_undead(struct enemy far *);
short enemy_is_dead(struct enemy far *);
void enemy_damage_range(struct enemy far *, short, short far *, short far *);
void enemy_set_target_enemy(struct enemy far *, struct enemy far *);
void enemy_sub_ae(struct enemy far *, short);
short enemy_attack_damage(struct enemy far *, void far *, short);
short enemy_range_damage(struct enemy far *, struct hero far *);
short enemy_test_attrib3(struct enemy far *, short, short, short, signed char);
short enemy_test_spell(struct enemy far *, short, signed char);

/* O193 */
void mouse_reset(void);
void mouse_init_vars(void);
void mouse_init(void);
void disable_mouse(void);
void mouse_irq_init(signed short, void interrupt (*)(void));
void mouse_reset_ehandler(void);

/* O194 */
short mon_spell_cost(short, short);
void far * Spell_ctor(void far *, struct enemy far *);
void Spell_dtor(void far *, short);
void mon_spell_damage(void far *, short);
short mon_spell_target_pa(void far *);
short mon_spell_target_rs(void far *);
short mon_spell_cast(void far *, short);

/* O195 */
short dng04_handler(struct area_events far *, struct map_event far *);
void dng04_snake(struct area_events far *, signed char far *);
short dng04_take_from_pool(struct area_events far *, short, signed char far *);
void dng04_deep_water(struct area_events far *);

/* O196 */
void groups_check_npc(void);
void add_npc(short);
short load_npc(struct hero far *, short);

/* O198 */
void view_draw(void);
void view_draw_sky(void);

/* O199 */
void far * Opt_ctor_o199(short, short);
void Opt_dtor_o199(void far *, short);
void Opt_run_o199(void far *);

/* O200 */
void far * Group_ctor(void far *);
void group_add_money(struct group far *, signed long);
void group_sub_money(struct group far *, signed long);
short group_count_check1(struct group far *);
short group_count_check2(struct group far *);
short group_count_check3(struct group far *);
void group_get_ill(struct group far *, short, short);
void group_infect(void far *, short, short, struct hero far *);
void group_add_le(struct group far *, short);
void group_sub_le(struct group far *, short);
void group_sub_le_range(struct group far *, short, short);
void group_remove_npc(struct group far *, struct hero far *);
signed long group_sum_ap(struct group far *);
void group_heal_full(struct group far *);
void group_add_le_range(struct group far *, short, short);
void group_sub_le_dice(struct group far *, short, short, short);
void group_sub_le_keep(struct group far *, short, short);
void group_add_ae(struct group far *, short);
void group_clr_hunger_thirst(struct group far *);
void group_sub_hunger_thirst(struct group far *, short, short);
void group_sub_hunger_thirst_max(struct group far *, short, short);
void group_clr_hunger(struct group far *);
void group_clr_thirst(struct group far *);

/* O201 */
short group_has_hero(struct group far *, struct hero far *);
short group_count_check2b(struct group far *);
short group_count_alive(struct group far *);
short group_hero_pos(struct group far *, struct hero far *);
void group_add_effect(struct group far *, signed long, signed char, signed char, signed char);
void group_set_on_route(struct group far *, short);
short group_find_food(struct group far *, struct hero far *, short far *, short far *, short far *);
short group_find_drink(struct group far *, struct hero far *, short far *, short far *, short far *);
short group_get_on_route(struct group far *);
short group_hero_with_item(struct group far *, short, short);
signed char group_all_have_item(struct group far *, short, short);
short group_same_place(struct group far *, struct group far *);
void portrait_dim(unsigned char far *);
short group_draw_hero_bar(struct group far *, short, short);
short group_mark_can_use(struct group far *, short);
void group_draw_portraits(struct group far *);

/* O202 */
short group_refill_water(struct group far *);
void group_add_ap(struct group far *, short);
void group_add_ap_each(struct group far *, short);
short group_free_slot(struct group far *);
void group_add_hero(struct group far *, struct hero far *, short);
void group_set_slot(struct group far *, short, short);
void group_sleep(struct group far *, short);
short group_give_item(struct group far *, short, short);
void group_drop_item(struct group far *, short);
void group_remove_hero(struct group far *, short);
void group_inc_skill(struct group far *, short);
void group_dec_weapons_bf(struct group far *);
void group_mark_weapon(struct group far *, char far *, short);
void group_reset_rest(struct group far *);
void group_give_item_each(struct group far *, short, short);
void group_drop_most_items(struct group far *);
void group_remove_item(struct group far *, short);
short group_count_free_knapsack(struct group far *);
void group_save(struct group far *);
void group_load(struct group far *);
short group_any_in_jail(struct group far *);
short group_has_dwarf(struct group far *);
short group_has_elf(struct group far *);

/* O203 */
short group_find_key(struct group far *, short);
short group_all_asleep(struct group far *);
short count_failed_skill(struct group far *, short, signed char);
signed char group_test_attrib_dmg(struct group far *, short, short, short);
short group_mr_bonus(struct group far *);
short group_free_capacity(struct group far *);
short group_find_item_box(struct group far *, short);
short group_count_bd2(struct group far *);
void group_kill_but_bd2(struct group far *);
void group_kill_all(struct group far *);
void hero_swap(struct group far *, struct hero far *, struct hero far *);
signed char group_test_attrib_disease(struct group far *, short, short, char far *, char far *, short, short, short);
signed char group_test_attrib_msg(struct group far *, short, short, char far *, char far *, short);
short group_count_item(struct group far *, short);
short group_take_item(struct group far *, short);

/* O204 */
void dng05_poltergeist(void far *);
short dng05_handler(struct area_events far *, struct map_event far *);
short dng05_fights(struct area_events far *, short);
void dng05_puzzle_result(struct area_events far *);
void dng05_skeleton_key(struct area_events far *);

/* O205 */
void win_save_bg(void far *);
void win_restore_bg(void far *);
void win_draw_frame(void far *, short);
void win_draw_frame_row(void far *, short, short, short);
short win_calc_x(void far *, short);
short win_calc_y(void far *, short);
void win_save_border(void far *, short, short, short, short, char far *);
void win_restore_border(void far *, short, short, short, short, char far *);
void win_drag(void far *, short, short, short, short);
void win_poll_input(void far *);
short win_wait(void far *, void far *);
void win_set_no_timeout(void far *);
void win_close(void far *);
void win_prepare(void far *, char far *, short, short);
short win_text_box(void far *, short, char far *);
short win_num_box(void far *, short, char far *, short, short, short);

/* O206 */
void hero_print_sheet(struct hero far *);
short print_line(struct hero far *, char far *);
char far * print_get_item(struct hero far *, short, short far *, short far *, short far *, short far *, short far *, short far *);

/* O207 */
void print_spells(struct hero far *);
void print_items(struct hero far *, short, short);
void print_file_name(struct hero far *, char far *);

/* O209 */
void travelmap_draw(TRAVEL_PTR);
void travelmap_draw_icons(TRAVEL_PTR);
short travelmap_walk_x(TRAVEL_PTR, short, short, short, signed char);
short travelmap_walk_y(TRAVEL_PTR, short, short, short, signed char);
void travelmap_show_route(TRAVEL_PTR, unsigned char);
void travelmap_route(TRAVEL_PTR, short);
void travelmap_draw_path(TRAVEL_PTR, short);
void travelmap_draw_path_to(TRAVEL_PTR, unsigned char);
void travelmap_set_point1(TRAVEL_PTR, unsigned char);
void travelmap_set_point2(TRAVEL_PTR, unsigned char);
unsigned short travelmap_route_part(TRAVEL_PTR, unsigned char, unsigned char, short, unsigned short, unsigned char, unsigned short);

/* O210 */
signed char travel_stage(TRAVEL_PTR);
signed char place_has_event(TRAVEL_PTR, unsigned char);
signed char place_event(TRAVEL_PTR);
signed char place_unloads_ui(TRAVEL_PTR);

/* O211 */
void tevent_run(TRAVEL_PTR, unsigned short);

/* O212 */
short fight_result(TRAVEL_PTR, signed char);
signed char encounter_level(TRAVEL_PTR, signed char);
void ambush_fight(TRAVEL_PTR, signed char);
void travel_encounter(TRAVEL_PTR, signed char, signed char);

/* O213 */
void travel_nop_o213(TRAVEL_PTR);
void travel_anim_off_leave(TRAVEL_PTR);
signed char travel_level(TRAVEL_PTR);
void travel_anim(TRAVEL_PTR);
void travel_anim_on(TRAVEL_PTR, short);
void travel_anim_off(TRAVEL_PTR, short);
void travel_weather_effects(TRAVEL_PTR);
void travel_ui_load(TRAVEL_PTR);
void travel_ui_unload(TRAVEL_PTR);
void travel_set_map_pos(TRAVEL_PTR, short);

/* O214 */
short travelmap_select_dir(TRAVEL_PTR);
void travel_redraw(TRAVEL_PTR);
short travel_menu(TRAVEL_PTR);

/* O215 */
short travel_resume(TRAVEL_PTR, short, short);
signed char travel_route(TRAVEL_PTR, unsigned char far *, unsigned char far *);

/* O216 */
void travel_follow_route(TRAVEL_PTR);
signed char travel_road_factor(TRAVEL_PTR, signed char);
signed char travel_weather_factor(TRAVEL_PTR, signed char);
signed char travel_health_factor(TRAVEL_PTR, signed char);
short travel_boat_speed(TRAVEL_PTR);
void travel_text_speed(TRAVEL_PTR, char far *);
void travel_text_weather(TRAVEL_PTR, char far *);
void travel_text_terrain(TRAVEL_PTR, char far *);
void travel_text_road(TRAVEL_PTR, char far *);
short travel_text_variant(TRAVEL_PTR);
void travel_status_update(TRAVEL_PTR);
void travel_status_draw(TRAVEL_PTR);

/* O217 */
void select_hero_bar(TRAVEL_PTR);
unsigned short travel_slow5_off(TRAVEL_PTR, unsigned short);
unsigned short travel_slow4_off(TRAVEL_PTR, unsigned short);
void travel_rest_menu(TRAVEL_PTR);
unsigned short travel_speed_normal(TRAVEL_PTR, unsigned short);
unsigned short travel_speed_hurry(TRAVEL_PTR, unsigned short, signed char);
unsigned short travel_reverse(TRAVEL_PTR, unsigned char far *, unsigned char far *, short);
void travel_stop(TRAVEL_PTR, signed char);
unsigned short travel_slow5_on(TRAVEL_PTR, unsigned short);
unsigned short travel_slow4_on(TRAVEL_PTR, unsigned short);

/* O218 */
void route_plan(TRAVEL_PTR, unsigned char);
void route_plan_simple(TRAVEL_PTR, unsigned char);
unsigned char route_choose_dir(TRAVEL_PTR, unsigned char far *, signed char);
unsigned char route_dest_by_dir(TRAVEL_PTR, unsigned char, unsigned char);

/* O219 */
void route_clear(TRAVEL_PTR);
signed short route_first(TRAVEL_PTR, unsigned char);
void travel_enter_town(TRAVEL_PTR, unsigned char);
void travel_enter_dungeon(TRAVEL_PTR, unsigned char);
short route_find(TRAVEL_PTR, unsigned char, unsigned char);
short route_find_area(TRAVEL_PTR, unsigned char, unsigned char);
signed char route_is_known(TRAVEL_PTR, unsigned char);
unsigned char place_count_dirs(TRAVEL_PTR, unsigned char);
unsigned char place_count_all_dirs(TRAVEL_PTR, unsigned char);
signed long route_start_index(TRAVEL_PTR, unsigned char);
unsigned char place_next_dest(TRAVEL_PTR, unsigned char, unsigned char);
unsigned char place_next_dest_all(TRAVEL_PTR, unsigned char, unsigned char);
unsigned char place_last_dest(TRAVEL_PTR, unsigned char);
unsigned char place_area(TRAVEL_PTR, unsigned char);
unsigned char route_dest_area(TRAVEL_PTR, unsigned char far *, unsigned char);

/* O220 */
void rose_alloc(TRAVEL_PTR);
void rose_free(TRAVEL_PTR);
short rose_mouse_area(TRAVEL_PTR, struct mouse_area far *);
short rose_dir_allowed(TRAVEL_PTR);
void rose_draw(TRAVEL_PTR);
void rose_move(TRAVEL_PTR, short);
void rose_compose(TRAVEL_PTR);
void rose_open(TRAVEL_PTR, struct dir_set);
void rose_hide(TRAVEL_PTR);
void rose_move_to(TRAVEL_PTR);
void rose_next_dir(TRAVEL_PTR);

/* O221 */
void o221_stub_0(void);
void o221_stub_1(void);
void o221_stub_2(void);
void o221_stub_3(void);
void o221_stub_4(void);
void o221_stub_5(void);
void o221_stub_6(void);
void o221_stub_7(void);
void o221_stub_8(void);
void o221_stub_9(void);
void o221_stub_10(void);
void o221_stub_11(void);
void o221_stub_12(void);
void o221_stub_13(void);
void o221_stub_14(void);
void o221_stub_15(void);
void o221_stub_16(void);

/* O222 */
void debug_nop1(void);
void debug_nop2(void);
void debug_ending_menu(void);
void debug_nop3(void);
void debug_nop4(void);
void debug_travel_events(TRAVEL_PTR);

/* O223 */
void travel_walk_planned(TRAVEL_PTR, unsigned char);
void travel_walk_simple(TRAVEL_PTR, unsigned char);
void place_reveal_dirs(TRAVEL_PTR, unsigned char);
void travel_reset_pos(TRAVEL_PTR);
void route_rebuild(TRAVEL_PTR, unsigned char);
void place_reveal_dir(TRAVEL_PTR, unsigned char, signed char);
void route_mark_known(TRAVEL_PTR, unsigned char, unsigned char);

/* O224 */
void saver_tick(void far *);
void saver_anim(void far *);
short saver_set_level(void far *, signed char);
short saver_wait(void far *, short);
void saver_poll(void far *);

/* O225 */
int sel_compar(const void far *, const void far *);

/* O226 */
void shop_leave(SHOP_PTR);
signed short shop_is_open(SHOP_PTR);
void shop_run(SHOP_PTR);
void shop_check_time(SHOP_PTR, signed long);
void shop_talk(SHOP_PTR);
void shop_build_wares(SHOP_PTR);
void shop_add_ware(SHOP_PTR, signed short, signed short, signed char);
void shop_format_money(SHOP_PTR, char far *, signed long);
signed short shop_trade(SHOP_PTR, signed long, signed char);
void shop_steal(SHOP_PTR);

/* O227 */
void shop_sell_screen(SHOP_PTR);
void shop_sell_draw_grid(SHOP_PTR, struct hero far *);
void shop_sell_draw_item(SHOP_PTR, signed short, struct hero far *);
void shop_sell_fill(SHOP_PTR, struct hero far *);
void shop_mark_hero(SHOP_PTR);

/* O228 */
void shop_buy_screen(SHOP_PTR);
void shop_buy_draw_grid(SHOP_PTR);
void shop_buy_draw_item(SHOP_PTR, signed short);
void shop_buy_sum(SHOP_PTR, signed short far *, signed short far *);
void shop_buy_draw_money(SHOP_PTR, signed long);
void shop_buy_draw_weight(SHOP_PTR, signed short, signed short);
void shop_buy_draw_slots(SHOP_PTR, signed short, signed short);

/* O229 */
int shop_compar(const void far *, const void far *);
void shop_grid_mark(SHOP_PTR, signed short);
signed short shop_grid_input(SHOP_PTR, signed short far *);

/* O230 */
void far * Smith_ctor(void far *);
void Smith_dtor(void far *, short);
short smith_check_open(void far *);
void smith_run(void far *);
void smith_ban(void far *);
void smith_check_closing(void far *, signed long);
void smith_talk(void far *);
void smith_pickup(void far *);
short smith_grid_input(void far *, short far *);
short smith_talk_event(short, short);

/* O231 */
void smith_repair(SMITH_PTR, struct hero far *);
void smith_draw_cursor(SMITH_PTR);
void smith_mark_hero(SMITH_PTR);
void smith_draw_grid(SMITH_PTR);
short smith_list_items(SMITH_PTR, struct hero far *);
struct hero far * smith_select_hero(SMITH_PTR);

/* O232 */
void snd_cfg_read(SNDSETUP_PTR);
void snd_cfg_unlink_drivers(SNDSETUP_PTR);
void snd_cfg_write(SNDSETUP_PTR);
signed short blaster_parse(SNDSETUP_PTR, struct ail_drv_desc far *, char far *, char far *, char far *);
signed short blaster_detect(SNDSETUP_PTR, char far *, signed short far *, signed short far *, signed short);

/* O233 */
short cast_damage(CAST_PTR, short);
short cast_target_pa(CAST_PTR);
short hero_spell_cost(short, short);
short spell_class_first(short);
short select_magic_user(void);
void far * Cast_ctor(void far *, struct hero far *);
void Cast_dtor(void far *, short);
void cast_heal_ani(CAST_PTR);
short cast_target_rs(CAST_PTR);
short cast_spell(CAST_PTR, short, short);
void spell_beherrschung_brechen(CAST_PTR);
void spell_destructibo(CAST_PTR);
void spell_gardianum(CAST_PTR);
void spell_illusionen_zerstoeren(CAST_PTR);
void spell_verwandlung_beenden(CAST_PTR);
void spell_band_und_fessel(CAST_PTR);
void spell_bannbaladin(CAST_PTR);
void spell_boeser_blick(CAST_PTR);

/* O234 */
void spell_grosse_gier(CAST_PTR);
void spell_grosse_verwirrung(CAST_PTR);
void spell_herr_ueber_das_tierreich(CAST_PTR);
void spell_horriphobus(CAST_PTR);
void spell_magischer_raub(CAST_PTR);
void spell_respondami(CAST_PTR);
void spell_sanftmut(CAST_PTR);
void spell_somnigravis(CAST_PTR);
void spell_zwingtanz(CAST_PTR);
void spell_furor_blut(CAST_PTR);
void spell_geister_austreiben(CAST_PTR);
void spell_geister_beschwoeren(CAST_PTR);
void spell_heptagon(CAST_PTR);
void spell_kraehenruf(CAST_PTR);
void spell_skelettarius(CAST_PTR);
void spell_elementare_beschwoeren(CAST_PTR);
void spell_nihilatio_gravitas(CAST_PTR);
void spell_solidirid(CAST_PTR);

/* O235 */
void spell_axxeleratus(CAST_PTR);
void spell_foramen(CAST_PTR);
void spell_motoricus(CAST_PTR);
void spell_spurlos(CAST_PTR);
void spell_transversalis(CAST_PTR);
void spell_ueber_eis(CAST_PTR);
void spell_balsam(CAST_PTR);
void spell_hexenspeichel(CAST_PTR);
void spell_klarum_purum(CAST_PTR);
void spell_ruhe_koerper(CAST_PTR);
void spell_tiere_besprechen(CAST_PTR);
void spell_adleraug(CAST_PTR);
void spell_analues(CAST_PTR);
void spell_eigenschaften(CAST_PTR);
void spell_exposami(CAST_PTR);
void spell_odem_arcanum(CAST_PTR);
void spell_penetrizzel(CAST_PTR);
void spell_sensibar(CAST_PTR);
void spell_chamaelioni(CAST_PTR);
void spell_duplicatus(CAST_PTR);
void spell_harmlose_gestalt(CAST_PTR);
void spell_hexenknoten(CAST_PTR);

/* O236 */
void spell_inc_attrib(CAST_PTR, short);
void spell_blitz(CAST_PTR);
void spell_ecliptifactus(CAST_PTR);
void spell_eisenrost(CAST_PTR);
void spell_fulminictus(CAST_PTR);
void spell_ignifaxius(CAST_PTR);
void spell_plumbumbarum(CAST_PTR);
void spell_radau(CAST_PTR);
void spell_saft_kraft(CAST_PTR);
void spell_scharfes_auge(CAST_PTR);
void spell_hexenblick(CAST_PTR);
void spell_nekropathia(CAST_PTR);
void spell_adler_wolf(CAST_PTR);
void spell_arcano_psychostabilis(CAST_PTR);
void spell_armatrutz(CAST_PTR);
void spell_ch_steigern(CAST_PTR);
void spell_feuerbann(CAST_PTR);
void spell_ff_steigern(CAST_PTR);
void spell_ge_steigern(CAST_PTR);
void spell_in_steigern(CAST_PTR);
void spell_kk_steigern(CAST_PTR);
void spell_kl_steigern(CAST_PTR);
void spell_mu_steigern(CAST_PTR);
void spell_mutabili(CAST_PTR);

/* O237 */
void spell_paralue(CAST_PTR);
void spell_salander(CAST_PTR);
void spell_in_see_und_fluss(CAST_PTR);
void spell_visibili(CAST_PTR);
void spell_abvenenum(CAST_PTR);
void spell_aeolitus(CAST_PTR);
void spell_brenne_toter_stoff(CAST_PTR);
void spell_claudibus(CAST_PTR);
void spell_dunkelheit(CAST_PTR);
void spell_weiches_erstarre(CAST_PTR);
void spell_flim_flam(CAST_PTR);
void spell_hartes_schmelze(CAST_PTR);
void spell_silentium(CAST_PTR);
void spell_sturmgebruell(CAST_PTR);

/* O238 */
void far * Stash_ctor(void far *, short, short);
void Stash_dtor(void far *, short);
void stash_set(void far *, short, short, unsigned char);
short stash_is(void far *, short, short, unsigned char);
void stash_add(void far *, short, short, unsigned char);
short stash_has_items(void far *);
short stash_count_items(void far *);
short stash_show(void far *);
short stash_update(void far *);
short stash_fill(void far *, char far *);
short stash_menu(void far *);

/* O239 */
void town_rnd_tallest_hero(void);
void town_rnd_magic_user(void);
void town_rnd_best_weapon(void);
void town_rnd_note_17(void);
void town_rnd_item83_arrest(void);
void town_rnd_dwarf_test(void);
void town_rnd_gift_7e(void);
void town_rnd_extortion(void);
void town_rnd_note_1a(void);
void town_rnd_choice_11(void);
void town_rnd_find_item_0e(void);
void town_rnd_note_00(void);
void town_rnd_once_01(void);
void town_rnd_once_02(void);
void town_rnd_lured_away(void);
void town_rnd_donation(short);
void town_rnd_donation_amount(void);
void town_rnd_paid_service(short);
void town_rnd_offer_1000(void);
void town_rnd_find_money(void);
void town_rnd_meeting_0b(void);
void town_rnd_twice_0c(void);
void town_rnd_meeting_0d(void);
void town_rnd_note_0e(void);
void town_rnd_mage_arrest(void);
void town_rnd_trader_800(void);
void town_rnd_open(short, short);
void town_rnd_close(void);

/* O240 */
void Tavern_ctor(void far *);
void Tavern_dtor(void far *, short);
short tavern_admit(struct tavern far *);
void tavern_run(struct tavern far *);
void tavern_set_crowd(struct tavern far *, signed long);
void tavern_check_ban(struct tavern far *);
void tavern_check_closing(struct tavern far *, signed long);
void tavern_drink_test(struct tavern far *, short);
void tavern_hourly_event(struct tavern far *);
short tavern_eat(struct tavern far *);
void tavern_order(struct tavern far *);
void tavern_add_bill(struct tavern far *, short);

/* O241 */
void tavern_leave(struct tavern far *);
void tavern_check_hours(struct tavern far *, signed long);
void tavern_entrance(struct tavern far *);
void tavern_round(struct tavern far *);
short tavern_pay_bill(struct tavern far *, short);
void tavern_use_skill(struct tavern far *);
void tavern_change_seat(struct tavern far *);
void tavern_talk(struct tavern far *);
short tavern_guests_left(struct tavern far *);
short tavern_talk_event(short, short);
short tavern_talk_event2(short, short);

/* O242 */
short tavern_order_dinner(struct tavern far *);
short tavern_pay_meal(struct tavern far *);
short tavern_order_drink(struct tavern far *);
void tavern_night_event(struct tavern far *);

/* O243 */
void far * TempFile_ctor(void far *);
short ofile_extract(void far *, short);
short ofile_open(void far *, short);
short ofile_create(void far *, short, short);
short ofile_open_mode(void far *, short, short, unsigned short);
signed long ofile_load(void far *, short, void far *);
signed long ofile_read_all(void far *, void far *);
signed long ofile_read(void far *, void far *, signed long);
signed long ofile_write(void far *, void far *, signed long);
short ofile_close(void far *);
signed long ofile_length(void far *);
signed long ofile_length_id(void far *, short);
void ofile_seek(void far *, signed long, short);

/* O248 */
void far * Font_ctor(void far *, short);
void Font_dtor(void far *, short);
short font_set_line_h(void far *, short);
short font_goto_x(void far *, short);
void font_goto_xy(void far *, short, short);
void txt_split(void far *, char far * far *, char far *, signed long);
void txt_load(void far *, short, char far *, char far * far *);
void txt_load_list(void far *, short, short, void far * far *, char far * far * far *);
short txt_max_width(void far *, char far *);

/* O249 */
void far * PortTxt_ctor(void far *);
void PortTxt_dtor(void far *, short);
short port_event(TOWN_PTR, struct map_event far *);
signed long port_haggle(TOWN_PTR);
void port_pay(TOWN_PTR, signed long);
void port_talk(TOWN_PTR);
void port_depart(TOWN_PTR);
void port_night_event(TOWN_PTR);
short port_harbor_master(TOWN_PTR);

/* O250 */
void Date_ctor(void far *);
void Date_print(void far *);
short date_season(void far *);
void Date_draw(void far *, short);
void Date_alloc(void far *);
void Date_save(void far *);
void Date_load(void far *);
void date_now(struct timestamp far *);
signed long date_since(struct timestamp far *);
void date_set(struct timestamp far *, signed short, signed long, signed char, signed char, signed char);
void Date_next_day(void far *);
void Date_free(void far *);

/* O251 */
signed short town_tjolmar_event(TOWN_PTR, struct map_event far *);
void tjolmar_dwarf_door(TOWN_PTR);

/* O252 */
void item_use_attack_item(void far *);
void item_play_instrument(void far *);
void item_read_heptagon_book(void far *);
void item_use_light(void far *);
void item_show_hidden_enemies(void far *);
void item_write_recipe(void far *);
void item_read_recipe_book(void far *);
void item_show_note(void far *);

/* O253 */
void item_consume(void far *);

/* O254 */
short item_poison_type(short);
void item_herb_effect(void far *);
void item_potion_effect(void far *);

/* O255 */
void view_cull_hidden(void);

/* O256 */
void Wx_ctor(void far *);
void Wx_dtor(void far *, short);
signed char Wx_get(void far *);
void Wx_next(char far *);
void Wx_set(void far *, signed char);

/* O257 */
void Conv_ctor(void far *);
void Conv_dtor(void far *, short);
signed char conv_set_topic(void far *, short, short);
void conv_topic_on(void far *, short);
void conv_dec_counter(void far *, short);
void conv_unlock_topic(void far *, short);
short conv_save(void far *);
short conv_load(void far *);
void conv_clr_counter(void far *, short);
void conv_greet(void far *);
void conv_bye(void far *);
void conv_insert_topic(void far *, short);
void conv_remove_temp(void far *);
void conv_script_topics(void far *);
void conv_relock(void far *);
void conv_place_topics(void far *);
signed char conv_get_counter(void far *, short);
void conv_set_counter(void far *, short);
void conv_compact_counters(void far *);
void conv_build_list(void far *);
void conv_refuse(void far *);
void conv_set_menu(void far *, short, ...);
short conv_menu(void far *, short);
void conv_draw_menu(void far *);
void conv_note(void far *);

/* O258 */
signed short conv_cb_inn5(signed short, signed short);
signed short conv_cb_frp22b(signed short, signed short);
signed short conv_cb_low04(signed short, signed short);
signed short conv_cb_kvir01(signed short, signed short);
signed short conv_cb_fins01(signed short, signed short);
signed short conv_cb_low02(signed short, signed short);
signed short conv_cb_low06(signed short, signed short);
signed short conv_cb_low07(signed short, signed short);
signed short conv_cb_low08(signed short, signed short);
signed short conv_cb_low15(signed short, signed short);
signed short conv_cb_low16(signed short, signed short);
signed short conv_cb_low22(signed short, signed short);
signed short conv_cb_low24(signed short, signed short);
signed short conv_cb_low25(signed short, signed short);
signed short conv_cb_low26(signed short, signed short);
signed short conv_cb_low27(signed short, signed short);
signed short conv_cb_low31(signed short, signed short);
signed short conv_cb_low32_lore(signed short, signed short);
signed short conv_cb_low33(signed short, signed short);
signed short conv_cb_low34(signed short, signed short);
signed short conv_cb_19(signed short, signed short);
signed short conv_cb_20(signed short, signed short);
signed short conv_cb_low42(signed short, signed short);
signed short conv_cb_low43(signed short, signed short);
signed short conv_cb_market(signed short, signed short);
signed short conv_cb_tjol09_brothel(signed short, signed short);
signed short conv_cb_low37(signed short, signed short);
signed short conv_cb_frp183b_meal(signed short, signed short);
signed short conv_cb_fst000_iwain(signed short, signed short);
signed short conv_cb_fst167_music(signed short, signed short);
signed short conv_cb_frp23b(signed short, signed short);
signed short conv_cb_frp25b(signed short, signed short);
signed short conv_cb_unused35(signed short, signed short);
signed short conv_cb_frp32b(signed short, signed short);
signed short conv_cb_low35(signed short, signed short);
signed short conv_cb_low36(signed short, signed short);
signed short conv_cb_low62_dragan(signed short, signed short);
signed short conv_cb_low63_shed(signed short, signed short);
signed short conv_cb_low65(signed short, signed short);
signed short conv_cb_gash(signed short, signed short);
signed short conv_cb_tavgas(signed short, signed short);
signed short conv_cb_gasinfo2(signed short, signed short);
signed short conv_cb_tief12b(signed short, signed short);
signed short conv_cb_swamp2(signed short, signed short);

/* O259 */
void conv_finish(void far *);
short conv_bye_scene(void far *);
void conv_scene_redraw(void far *);
void conv_scene_picture(void far *);
void conv_scene_lore(void far *, short);
void conv_scene_brothel(void far *, short);
void conv_scene_iwain(void far *);
short conv_scene_meal(void far *);
short conv_scene_music(void far *);
short conv_scene_dragan_task(void far *);
void conv_scene_shed(void far *);
void conv_scene_no_room(void far *);
void conv_scene_artherion(void far *);
void conv_scene_note(void far *, short);
void conv_scene_no_idea(void far *);

/* O260 */
short Script_is_used(void far *, short);

/* O261 */
short dng06_handler(struct area_events far *, struct map_event far *);
short dng06_fights(struct area_events far *, short);
void dng06_latrine_ambush(struct area_events far *);

/* O262 */
void blut_nop(void);
void blut_noise_check(struct area_events far *, signed char far *);
void dng06_brazoragh(struct area_events far *);
void dng06_pit(struct area_events far *, signed char far *);
short blut_rockfall(struct area_events far *, signed char far *, short, short, short, short, short, short, char far *);
void dng06_beer_barrel(struct area_events far *, struct hero far *, short);
void dng06_straw(struct area_events far *, struct hero far *, short);
short dng06_stores(struct area_events far *, short);
short dng06_weapon_rack(struct area_events far *, short);
short dng06_door(struct area_events far *, short, short, short, struct map_event far *);
void dng06_beetle_pit(struct area_events far *, struct hero far *);
void dng06_hornets(struct area_events far *);
void dng06_dwarf_skeleton(struct area_events far *);

/* O263 */
void group_give_clothes(void far *);
void group_confiscate(void far *);
void group_confiscate_herbs(void far *);
void group_confiscate_armor(void far *);
void group_confiscate_weapons(void far *);
void group_confiscate_rest(void far *);

/* O264 */
void dng07_prison_day(void far *);
signed short dng07_handler(struct area_events far *, struct map_event far *);
void dng07_spider_queen(void far *);

/* O265 */
short dng07_door(struct area_events far *, short, short, short, struct map_event far *);
short dng07_shelves(struct area_events far *, short);
void dng07_gold_statue(struct area_events far *);
void dng07_straw(struct area_events far *, struct hero far *, short);
void dng07_graffiti(struct area_events far *);
void dng07_web(struct area_events far *, short);
void dng07_web_corpse(struct area_events far *);

/* O266 */
short dng07_fights(struct area_events far *, short);
void dng07_pincer_key(struct area_events far *);
void dng07_glasses_key(struct area_events far *);

/* O267 */
short dng08_handler(struct area_events far *, struct map_event far *);
void dng08_bolt_trap(struct area_events far *, signed char, signed char far *);

/* O268 */
short dng08_door(struct area_events far *, short, short, short, struct map_event far *);
short dng08_fights(struct area_events far *, short);
short dng08_shelves(struct area_events far *, short);
void dng08_pit(struct area_events far *, signed char far *);

/* O269 */
void far * Pic_ctor_file(void far *, void far *);
void far * Pic_ctor(void far *);
short pic_load0(void far *, short);
short pic_load(void far *, short, void far *);
short pic_load_pal(void far *, short, void far *, void far *);
short pic_loadf0(void far *, short);
short pic_loadf(void far *, short, void far *);
short pic_loadf_pal(void far *, short, void far *, void far *);
short pic_sload0(void far *, short);
short pic_sload(void far *, short, void far *);
short pic_sload_pal(void far *, short, void far *, void far *);
short pic_sloadf0(void far *, short);
short pic_sloadf(void far *, short, void far *);
short pic_sloadf_pal(void far *, short, void far *, void far *);
short pic_do_load(void far *, short, void far *, void far *, signed char);
void pic_set_palette(void far *);
struct rgb far * pic_getpal(void far *);
void pic_get_size(void far *, short far *, short far *);
unsigned char far * pic_get_data(void far *);
void pic_get_size_id(void far *, short, short far *, short far *);
short pic_check(void far *, short);
short pic_read_header(void far *);
short pic_read_pal(void far *);
short pic_read_data(void far *, void far *);
unsigned char pic_get_byte(void far *);
short pic_decode(void far *, unsigned char far *);
void pic_unpack_raw(void far *, unsigned char far *);
void pic_unpack_rle(void far *, unsigned char far *);
void pic_rle(void far *, unsigned char far *);
void pic_rle_fast(void far *, unsigned char far *, unsigned char far *, short, short);
void pic_unpack_packbits(void far *, unsigned char far *);
void pic_packbits(void far *, unsigned char far *);
void pic_unpack_pp20(void far *, unsigned char far *);

/* O270 */
void fli_wait_retrace(void);
void fli_color_dac(unsigned char far *);
void fli_color_pal(unsigned char far *, unsigned char far *);
void fli_lc(unsigned char far *, unsigned char far *);
void fli_brun(unsigned char far *, unsigned char far *, short);
void fli_black(unsigned char far *, unsigned short);
void fli_copy(unsigned char far *, unsigned char far *, unsigned short);
unsigned char huge * fli_normalize(unsigned char huge *);

/* R27 */
void tmap_blit_rows(char far *, unsigned char far *, short);

/* R01 (Reise-/Spielschleife, Leerlaufschritt, Spielstand; Marken fuer die Musterpruefung des Kopierschutzes) */
void do_travel_mode(void);
void main_loop(void);
void copyprot_idle_begin(void);
void game_idle(void);
void copyprot_idle_end(void);
void copyprot_save_begin(void);
signed short savemenu_save(void far *, signed short, signed short);
void copyprot_save_end(void);

/* R02 */
short wait_ticks_key(short);
void wait_ticks(short);
char far * str_pad(char far *, short, char);
void set_palette(void far *, short, short);
void set_color(void far *, short);
void check_palette(void);
void set_mouse_handler2(short);
void set_menu_action(short);
void screensaver_reset(void far *);
void screensaver_run(void far *);
signed long clamp_long(signed long, signed long, signed long);
short check_hero(struct hero far *);
short check_hero2(struct hero far *);
short check_hero3(struct hero far *);
short get_item_pos(struct hero far *, short);
short hero_is_diseased(struct hero far *);
short hero_is_poisoned(struct hero far *);
short hero_is_diseased1(struct hero far *);
short hero_is_poisoned1(struct hero far *);
void update_hero_bars(struct hero far *, short);
short get_free_effect(struct hero far *);
void effects_tick(struct hero far *, signed long);
void add_effect(struct hero far *, short, signed long, signed char, signed char, signed char);
void remove_effect(struct hero far *, signed char);
unsigned char get_hero_index(struct hero far *);
short group_find_item(struct group far *, short);
void bars_refresh(void far *);
void bars_update_all(void far *);
short EMS_map(unsigned char, short, short);
short EMS_map4(unsigned char, short, short);
unsigned long EMS_copy(void far *);
void handle_input(void far *);
void handle_options_key(void);
void bars_clear_slot(void far *, short);

/* R03 (Handbuchabfrage/Kopierschutz, Klasse CopyProt in s_class.h; dazu Spielende und V86-Test) */
void copyprot_code_begin(void);
void copyprot_stub(void);
void copyprot_check(void);
void copyprot_code_end(void);
void exit_game(void);
void cleanup_game(short);
void CopyProt_ctor(void far *);
void copyprot_empty(void);
void CopyProt_dtor(void far *, short);
short cpu_v86_active(void);

/* R04 */
short snd_seq_state(void far *, short);
unsigned char huge * snd_cur_buf(void far *);
void snd_queue(void far *, unsigned char huge *, unsigned long);
void snd_service(void far *);
void snd_set_vol(short, short);
short snd_state(void far *, short);
short snd_busy(void far *, short);
void snd_speech(void far *, short);
short snd_speech_busy(void far *);
void snd_speech_service(void far *);
void snd_play2(void far *, short);
void snd_select(void far *, short);
void snd_play(void far *, short);
short snd_exists(void far *, short);
struct snd_buf far * snd_load(void far *, short);
void snd_start(void far *);
void snd_stop(void far *, short);

/* R05 */
int ctrl_break_handler(void);
void Init_ctor(void far *);
void Init_dtor(void far *, short);
void load_archives(void);
void load_ltx(char far *, void far * far *, void far * far *);
void swap_mem_init(void);
signed short cd_check(void);

/* R06 */
void arc_init(void far *, char far * far *, short, short);
void far * arc_new0(void far *, char far * far *, short, short);
void far * arc_new(void far *, void far *, void far *, void far *, void far *, short, short);
void arc_delete(void far *, short);
void arc_info(void far *, void far *);
short arc_open(void far *, short);
signed long arc_load(void far *, short, void far *);
short xms_alloc(void far *, signed long);
void xms_store(void far *, signed long);
signed long arc_read_all(void far *, void far *);
signed long arc_read(void far *, void far *, signed long);
signed long arc_read_raw(void far *, void far *, signed long);
short arc_close(void far *);
signed long arc_size(void far *);
signed long arc_file_size(void far *, short);
signed long arc_seek(void far *, signed long, short);
signed long xms_write(void far *, void far *, signed long, short, signed long);
void arc_mark(void far *);
void arc_release(void far *);
char far * arc_name(void far *, short);
short arc_find(void far *, char far *);
short arc_is_set(void far *, short);
short arc_open_raw(void far *, short);
short arc_open_mode(void far *, short, short);
short arc_exists(void far *, short);
short arc_open_dir(void far *, short);
void arc_dummy(void);

/* R07 */
void far * mem_alloc(unsigned long);
void far * mem_free(void far *);
short mem_free_slot(void);
void mem_check(void);
void far * mem_alloc_w(unsigned short);
void far * mem_alloc_l(unsigned long);
void mem_free_v(void far *);

/* R08 */
void cursor_for_cell(short);
void reveal_map(void far *, short, short);

/* R10 */
void sub_timers(short far *);
void timers_events(short far *);
void hero_timers(short far *, struct hero far *);
void sub_light_timers(short far *, struct hero far *);
void heroes_tick(short far *);
void do_timers(short far *);

/* R11 */
struct compass far * Compass_ctor(struct compass far *, short);
void Compass_dtor(struct compass far *, short);
void compass_draw(struct compass far *, short);
void compass_draw_strip(struct compass far *, short);

/* R12 */
void kbd_poll(void);
void view_frame(void);
void move_input(void);
void draw_object(void far *);
void draw_hud(void);
void pick_rect(short, short, short, short);
void pick_point(short, short);
void pick_mouse(void);
void pick_center(void);
void cell_event(void);
void draw_stars(void);
void set_cursor(short);
void install_cursor(void);
void fog_table(short, short);
void fade_table(short, short);
short call_handler(void far *, void far *);
short call_handler2(void far *, void far *);

/* R13 */
void edge(short, short, struct vtx far *, struct vtx far *);
void face_profile(short, short, short, struct vtx far *, short far *, short far *);
short cell_profile(short, short);
void cell_forget(short, short);
void cam_set_angles(short far *);
void cam_set_speed(short);
short can_stand(short, short);
short cam_move(void);
void cam_drop(void);

/* R14 */
void guru_meditation(char far *, unsigned char huge *);
void push_fc(short);

/* R15 */
void fade_in_pic(void far *, void far *, void far *, short, short);
void fade_line(void far *, unsigned char far *, unsigned char far *, unsigned char far *, short, short);

/* R16 */
void decomp(void far *, short, signed long, short, short, void far *, void far *);
void decomp_rle(void far *, unsigned char far *, unsigned char far *, short, short);
void decomp_rle2(void far *, unsigned char huge *, unsigned char huge *, signed long);

/* R17 */
void far * new_map(void far *);
void free_map(void far *, short);
void load_map(void far *, short);
void save_map(void far *, short);
unsigned char get_map_cell(void far *, short, short);
void set_map_cell(void far *, short, short, unsigned char);
void load_tiles(void far *, short);
void build_cells(void far *);
void load_map_all(void far *, short, short);

/* R18 */
void mouse_show(short);
void mouse_hide(short);
void mouse_action(void far *, void far *, void far *, void far *, void far *);
void mouse_move(short, short);
void call_mouse_isr(void);
void mouse_set_cursor(short, short);
void mouse_set_cursor_bmp(void far *, short, short, short, short, short);
void mouse_erase(void);
void mouse_draw(void);
void mouse_update(void);
short is_mouse_in_rect(short, short, short, short);
short is_mouse_in_rect_p(short far *);
short mouse_overlaps_rect(short, short, short, short);
short is_click_in_rect(short, short, short, short);
short get_click_action(void far *);
short get_mouse_action(void far *);
void poll_input(void);
void mouse_restore_bg(short far *);
void mouse_save_bg(short, short, void far *);
void mouse_blit(short, short, void far *);
void set_keyboard_isr(void);
void reset_keyboard_isr(void);
void mouse_clear_left(void);
void mouse_clear_right(void);
void clear_input(void);
void mouse_call_handler(void);
void menu_hover_select(short);

/* R19 */
unsigned char get_cell_a(short, short);
short get_cell_b(short, short);
struct cell_rot get_cell_c(short, short);

/* R20 */
void font_init(void far *);
short font_glyph(void far *, unsigned char, short far *);
void font_dummy1(void);
unsigned char far * font_glyph_bits(void far *, unsigned char);
void font_dummy2(void);
void font_draw_glyph(void far *, unsigned char, short, short, short);
short font_put_char(void far *, unsigned char, short, short);
void font_set_x(void far *, short);
void font_set_y(void far *, short);
void font_set_xy(void far *, short, short);
short font_get_y(void far *);
short font_char_width(void far *, unsigned char);
short font_text_width(void far *, char far *);
short font_height(void far *);
void font_get_box(void far *, short far *, short far *, short far *, short far *);
void font_set_colors(void far *, unsigned char, short);
void font_push_colset(void far *, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
void font_pop_colset(void far *);
void font_set_shadow(void far *, unsigned char);
void font_set_box(void far *, short, short, short, short);
short font_get_tab(void far *, short);
void font_set_tab(void far *, short, short);
void font_set_indent(void far *, short, short);
void font_get_indent(void far *, short far *, short far *);
void font_restore_colors(void far *);
void font_get_colors(void far *, short far *, short far *);
short font_get_shadow(void far *);
void font_rect(void far *, short, short, short, short);
void font_rect1(void far *, short, short, short, short, void far *, short);
short font_center_x(void far *, char far *, short, short);
short font_text_width0(void far *, char far *);
short font_print(void far *, char far *, short, short);
short font_print_wrapped(void far *, char far *);
short font_wrap(void far *, char far *);
void far * font_set_screen(void far *, void far *);
short print_text(char far *);

/* R21 */
void timewarp_until(void far *, signed long);
void timewarp(void far *, signed long);
void timewarp_minutes(void far *, signed long);
void timewarp_hours(void far *, signed long);
void timewarp_days(void far *, signed long);
void timewarp_months(void far *, signed long);
void timewarp_years(void far *, signed long);
void time_tick(void far *);

/* R22 */
void AIL_startup(void);
void AIL_shutdown(short, short);
short AIL_register_timer(void far *);
void AIL_release_timer_handle(short);
void AIL_start_timer(short);
void AIL_stop_timer(short);
void AIL_set_timer_frequency(short, signed long);
short AIL_register_driver(void far *);
void far * AIL_describe_driver(short);
void AIL_init_driver(short, short, short, short, short);
void AIL_index_VOC_block(short, void huge *, short, struct snd_buf far *);
void AIL_register_sound_buffer(short, short, struct snd_buf far *);
void AIL_format_sound_buffer(short, struct snd_buf far *);
short AIL_sound_buffer_status(short, short);
void AIL_start_digital_playback(short);
void AIL_set_digital_playback_volume(short, short);
void AIL_release_sequence_handle(short, short);
void AIL_stop_sequence(short, short);
short AIL_sequence_status(short, short);
short AIL_relative_volume(short, short);
void AIL_release_driver_handle(short);
short AIL_detect_device(short, short, short, short, short);
void AIL_pause_digital_playback(short);
void AIL_resume_digital_playback(short);
unsigned short AIL_state_table_size(short);
short AIL_register_sequence(short, void far *, short, void far *, void far *);
unsigned short AIL_default_timbre_cache_size(short);
void AIL_define_timbre_cache(short, void far *, unsigned short);
unsigned short AIL_timbre_request(short, short);
void AIL_install_timbre(short, short, short, void far *);
void AIL_start_sequence(short, short);
void AIL_resume_sequence(short, short);
void AIL_set_relative_volume(short, short, short, short);
void AIL_install_callback(short, void (far *)());
void AIL_cancel_callback(short);

/* R23 (Zeitgeber INT 1Ch, AIL-Rueckrufe, Maus-ISR, Pruefung des Kopierschutzes im Kampf-Zeitgeber) */
void timer_init(void);
void timer_restore(void);
#ifdef __cplusplus
void interrupt timer_isr(...);
#else
void interrupt timer_isr(void);
#endif
void view_timer_cb(void);
void fight_timer_cb(void);
void interrupt mouse_isr(void);
signed long long_sqrt(signed long);

/* R24 (Assembler, 3D-Engine: Kanten und Fuellroutinen) */
void edge_trace(short, short, short, short, short, short, short, short, short, short, short);	/* x0, y0, z0, x1, y1, z1, u0, v0, u1, v1, horizontal; intern (push cs/call near) */
void edge_trace_persp(short, short, short, short, short, short, short, short, short, short, short);	/* wie edge_trace, perspektivisch; intern */
void edges_clear(void);
void fill_scene(short);
void copy_3d_to_screen(void);
void poly_edges(short, short far *, struct vtx far *, short far *, short);	/* n, idx, vtx, uv, horizontal (1 = zeilenweise) */
void poly_edges_persp(short, short far *, struct vtx far *, short far *, short);
void poly_tex_cols(short);
void poly_tex_cols_m(short);
void poly_tex_rows(short);
void poly_tex_rows_m(short);
void poly_flat(short, short);

/* R25 (Assembler, 3D-Engine: Drehen, Abschneiden, Projektion; clip_face hat Registerschnittstelle, ohne Prototyp) */
short obj_project(void far *);
void view_set_projection(short, short);	/* Massstab, Augabstand (90, 32) */
void tex_copy_flat(short, signed long, short, signed long, signed long);

/* R26 */
void glyph_blit(unsigned char far *, unsigned char far *, unsigned char far *, unsigned char, unsigned char);
void glyph_blit_bg(unsigned char far *, unsigned char far *, unsigned char far *, unsigned char, unsigned char, unsigned char);

/* R28 */
unsigned short swap_u16(unsigned short);
signed long swap_u32(unsigned long);
unsigned long swap_u32_unused(unsigned long);

/* R29 */
void unpack_pp20(void far *, void far *, unsigned long);

/* R30 */
unsigned char char_tolower(unsigned char);
unsigned char char_toupper(unsigned char);
void str_tolower(char far *);
void str_toupper(char far *);

/* R31 */
void clear_kbd(void);
void wait_key_clear(void);

/* R32 */
short is_in_word_array(short, void far *);
short is_in_byte_array(signed char, void far *);
short is_in_byte_array2(signed char, void far *);

/* R33 */
void ppDecrunch(unsigned char far *, unsigned char far *, unsigned short, unsigned short, unsigned long);

/* R34 */
void fill_line(unsigned char far *, short, signed char);
void fill_v_line(unsigned char far *, short, signed char, short);
void vga_save_row(short, char far *);
void vga_restore_row(short, char far *);
void vga_restore_row_part(short, char far *, short);
void vga_save_col(short, char far *);
void vga_restore_col(short, char far *);
void vga_restore_col_part(short, char far *, short);

/* R35 */
short flat_enter(void);
void flat_leave(void);
void flat_copy(unsigned short, unsigned long, unsigned short, unsigned long, unsigned long);

/* R36 */
int EMS_get_num_pages_unalloced(void);
void far * EMS_norm_ptr(void far *);

/* R37 */
short random_interval(short, short);
short random_schweif(short);
short dice_roll(short, short, short);
void calc_damage_range(short, short, short, short far *, short far *);

/* R38 */
signed long xms_move_any(void far *);

/* R39 */
signed long xms_alloc_kb(short);
signed long xms_move(unsigned long);
void far * xms_lock(short);
signed long xms_unlock(short);
short xms_present(void);
short xms_free_kb(void);
void xms_free_block(short);
void xms_local_a20_on(void);
void xms_local_a20_off(void);

/* R40 */
signed long read_big(short, void far *, signed long);
signed long write_big(short, void far *, signed long);

/* R41 */
void far * nvf_new(void far *);
void nvf_delete(void far *, short);
void nvf_set(void far *, unsigned char far *, unsigned char huge *, short, signed char, unsigned short far *, unsigned short far *, short);
signed long nvf_process(void far *);
signed long nvf_unpack(void far *, void far *, void far *, short, signed char, void far *, void far *, short);

/* R42 */
void far * gb_new0(void far *);
void far * gb_new(void far *, short, short, void far *);
void far * gb_new_alloc(void far *, unsigned short, short);
void far * gb_new_file(void far *, char far *, unsigned short, short);
void far * gb_new_arch(void far *, unsigned short, short, short);
void gb_delete(void far *, short);
void gb_init(void far *, short, short, unsigned char far *);
void gb_free_lines(void far *);
void gb_blit(void far *, void far *, short, short);
void gb_blit_m(void far *, void far *, short, short, short);
void gb_blit_wh(void far *, void far *, short, short, short, short);
void gb_blit_whm(void far *, void far *, short, short, short, short, short);
void gb_blit_a(void far *, void far *, short, short, short, short, short, short);
void gb_blit_ab(void far *, void far *, short, short, short, short, short, short, short);
void gb_save(void far *, void far *, short, short);
void gb_save_wh(void far *, void far *, short, short, short, short);
void gb_set_base(void far *, void far *);
void gb_load_mem(void far *, void far *);
void gb_load_file(void far *, char far *);
void far * gb_data(void far *);
void gb_scale(void far *, void far *);
void gb_clear(void far *);
void gb_fill(void far *, short);
void gb_get_size(void far *, short far *, short far *);
void gb_rect(void far *, short, short, short, short, short);
void gb_hline(void far *, short, short, short, short);
void gb_vline(void far *, short, short, short, short);
void gb_put(void far *, void far *, short, short, short);
void gb_get(void far *, void far *, short, short);
void gb_get_wh(void far *, void far *, short, short, short, short);

/* R43 */
void set_video_mode(short);
void get_color(unsigned char far *, short);
void restore_video_mode(void far *);
void get_palette(void far *, short, short);
void far * dos_get_indos_ptr(void);
void save_display_stat(void far *);

/* R44 */
void pal_fade(void far *, void far *, short);

/* R45 */
void wait_for_vsync(void);
void wait_for_vsyncs(short);
void do_border(void far *, short, short, short, short, short);
void do_h_line(void far *, short, short, short, short);
void do_v_line(void far *, short, short, short, short);

/* R46 */
void blit_lines(short, short, short, short, short, unsigned char far * far *, signed char, void far *, short);
void grab_lines(short, short, short, short, unsigned char far * far *, void far *);
void scale_lines(unsigned char far * far *, short, short, unsigned char far * far *, short, short);

/* Altnamen der Welle N6 (Objektklassen): bisherige Namen, die parallel laufende Wellen noch benutzen
 * koennen - nach dem Zusammenfuehren durch die neuen Namen ersetzen und den Block loeschen. */
#define Txt_ctor Font_ctor
#define Txt_dtor Font_dtor
#define Txt_set1a font_set_line_h
#define Txt_split txt_split
#define Txt_maxwidth txt_max_width
#define File_ctor_o243 TempFile_ctor
#define File_open3 ofile_open_mode
#define File_load ofile_load
#define File_size ofile_length
#define File_fsize ofile_length_id
#define Nvf_pal nvfile_pal
#define Nvf_dims nvfile_dims
#define Pic_load pic_load0
#define Pic_load_o269 pic_load_pal
#define Pic_loadf pic_loadf
#define Pic_loadf_o269 pic_loadf_pal
#define Pic_dims pic_get_size
#define Pic_getdata pic_get_data
#define Pic_dims_o269 pic_get_size_id
#define List_ctor Stash_ctor
#define List_dtor Stash_dtor
#define List_set stash_set
#define List_is stash_is
#define List_add_o238 stash_add
#define List_hasany stash_has_items
#define List_show_o238 stash_show
#define ani_start ani_halt
#define dlg_message_pic dlg_box_anim
#define dlg_choose_pic dlg_box_bmp

/* Welle N8: Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define event_enemy_roll event_fight1
#define event_enemy_roll2 event_fight2
#define event_daytime_set event_fight
#define Town_go town_enter
#define g_current_town g_loc_id
#define Txt249_ctor PortTxt_ctor
#define Txt249_dtor PortTxt_dtor
#define map_refresh cam_set_from_group
#define Dng_ctor Travel_ctor
#define Dng_dtor Travel_dtor
#define loc_enter travel_enter
#define loc_leave travel_leave
#define load_offset_entry route_load_points
#define travel_set1e travel_anim_on
#define g_loc_entries g_routes
#define g_ltx_town g_tev_txt
#define g_ltx_loc g_tev_loc_txt
#define Map_ctor Automap_ctor
#define Map_dtor Automap_dtor
#define Map_open automap_open
#define Map_close automap_close
#define Map_loadmap automap_load_cells
#define Map_settype automap_set_town_type
#define Map_show automap_show
#define Map_draw automap_draw
#define Map_loadvis automap_load_seen
#define Map_save automap_save
#define Map_load automap_load
#define Map_run automap_run
#define Map_frame automap_frame
#define Map_f53_0000 automap_nop
#define Map_travel automap_travel
#define Map_random_step automap_random_cell
#define map_cell_io automap_cell_write
#define Map_enter automap_init
#define Map_leave automap_free_cells
#define g_loc_descs g_dng_areas
#define group_lose_items group_imprison
#define group_split group_scatter
#define save_record read_record
#define loc_desc view_area

/* Welle N13 (Magie): Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define Tm_tick saver_tick
#define Tm_anim saver_anim
#define Cast_damage cast_damage
#define Cast_mr cast_target_pa
#define g_8c01 g_cast_spell_id
#define g_8c03 g_spell_cancelled
#define g_8c11 g_spell_by_item
#define g_8c13 g_spell_no_sound

/* Welle N10: Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define travel_set_f1e travel_anim_on
#define travel_clr_f1e travel_anim_off
#define travel_clr_f1e_leave travel_anim_off_leave
#define route_length route_find
#define travel_event_texts place_event
#define travel_event travel_stage
#define Win_open tscr_open
#define Win_close tscr_close
#define Win_f0250 tscr_print
#define Win_text tscr_print_font
#define Win_f04c4 tscr_set_anim
#define g_2543 g_win_centered

/* Altnamen der Welle N11 (Kampf C) - nach dem Zusammenfuehren ersetzen und den Block loeschen. */
#define fight_hero_action range_attack_check_ammo
#define enemy_is_gfx_2b enemy_is_fire
#define enemy_atpa enemy_parade_value

/* Welle N12: Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define travel_walk_fwd travel_walk_planned
#define travel_walk_back travel_walk_simple
#define travel_speed_slow travel_speed_hurry

/* Welle N14: Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define g_253b g_subtitles
#define g_2546 g_cd_speech
#define g_2a2a g_smith_banned
#define g_259c g_guest_lock
#define g_25a3 g_guest_lock2
#define g_26f6 g_guest_lock_t2
#define g_26f7 g_guest_lock_t19
#define g_26f8 g_guest_lock_t23
#define g_26f9 g_guest_lock_t24
#define town_rnd_0000 town_rnd_tallest_hero
#define town_rnd_0276 town_rnd_magic_user
#define town_rnd_040e town_rnd_best_weapon
#define town_rnd_059a town_rnd_note_17
#define town_rnd_05d8 town_rnd_item83_arrest
#define town_rnd_0720 town_rnd_dwarf_test
#define town_rnd_0b62 town_rnd_gift_7e
#define town_rnd_0cd3 town_rnd_extortion
#define town_rnd_1104 town_rnd_note_1a
#define town_rnd_1142 town_rnd_choice_11
#define town_rnd_126b town_rnd_find_item_0e
#define town_rnd_1358 town_rnd_note_00
#define town_rnd_1393 town_rnd_once_01
#define town_rnd_13e6 town_rnd_once_02
#define town_rnd_15a5 town_rnd_lured_away
#define town_rnd_18cd town_rnd_donation
#define town_rnd_1b0f town_rnd_donation_amount
#define town_rnd_1cd6 town_rnd_paid_service
#define town_rnd_1e16 town_rnd_offer_1000
#define town_rnd_1f69 town_rnd_find_money
#define town_rnd_1fff town_rnd_meeting_0b
#define town_rnd_20de town_rnd_twice_0c
#define town_rnd_2198 town_rnd_meeting_0d
#define town_rnd_221b town_rnd_note_0e
#define town_rnd_2259 town_rnd_mage_arrest
#define town_rnd_24f3 town_rnd_trader_800

/* Altnamen der Welle N15 (Kampf-Rest, Gegenstaende, Inventar 2) - nach dem Zusammenfuehren ersetzen und den Block loeschen. */
#define fight_show_pic fight_load_bg
#define Anim_ctor_o016 FigAnim_ctor
#define Anim_dtor_o016 FigAnim_dtor
#define Inv_g_0000 inv_confirm
#define Inv_g_05b7 inv_next_hero
#define Inv_g_08f3 inv_toggle_body
#define Inv_g_09bb inv_draw
#define o058_0000 preselect_best_skill
#define item_in_box_bf item_in_recipe_book
/* Altnamen der Welle N16 (Haendler, Herberge, Tempel, Lager, Truhen) - nach dem Zusammenfuehren ersetzen und den Block loeschen. */
#define o012_064d store_check_time
#define Store_save store_save
#define Store_draw store_grid_mark
#define Store_input store_grid_input
#define Game_ctor CharGen_ctor
#define Game_dtor CharGen_dtor
#define Pray_run chargen_run
#define o029_02e3 hero_delete_chr
#define o029_074a hero_load_chr
#define o033_0000 temple_miracle
#define o044_1109 temple_desecrate
#define o068_0000 peddler_buy_screen
#define o069_034a peddler_build_wares
#define o069_0633 peddler_trade
#define o069_0905 peddler_grid_input
#define Camp_run camp_run
#define o071_19e3 camp_reset_watch
#define o072_03bc camp_use_magic
#define o072_050c camp_group_menu
#define o072_0c13 camp_rest
#define Chest_empty chest_count_items
#define Chest_set chest_set_item
#define o099_0000 store_deposit
#define o100_0000 store_fetch
#define Inn_eat inn_eat
#define Inn_room inn_room
#define group_check_drunk group_sleep
#define o250_09aa date_set
#define g_2545 g_opt_sound
#define g_25f9 g_peddler_away
#define g_25fa g_peddler3_away
#define g_280f g_inn_town
#define g_2811 g_inn_loc
#define g_29d8 g_temple_desecrated
#define g_2a57 g_peddler_banned
#define g_6e44 g_inn
#define g_c812 g_peddler

/* Welle N18: Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define g_254a g_game_mode
#define g_b690 g_input_buf

/* Altnamen der Welle N19 (Grafik, Animation, 3D-Ansicht) - nach dem Zusammenfuehren ersetzen und den Block loeschen. */
#define o024_0010 fli_play
#define o024_02b0 fli_play_frames
#define Anim_setbg2 Anim_unpack
#define o015_05e5 sheet_cast_anim
#define o055_0392 ani_play_until
#define o010_0000 view_init
#define o010_174c view_timer_on
#define o096_0091 view_alloc
#define o096_061d view_load_vis_order
#define o096_0700 view_load_palette
#define cam_update_cell view_draw
#define g_3956 g_sheet_hunger_shown
#define g_3957 g_sheet_thirst_shown
#define g_2538 g_anims_on
#define g_9546 g_view_pal_n
#define g_b12e g_view_active
#define g_c6aa g_view_swap_pos
#define g_b4a2 g_cell_occludes
#define g_85e0 g_cur_tex_w
#define g_32c7 g_vis_start
#define g_9496 g_cam_cell_y
#define g_9498 g_cam_cell_x
#define g_4a3c g_town_occl_04
#define g_4a5a g_town_occl_05
/* Altnamen der Welle N20 (Programmstart, Vorspann, Klang, Optionen) - nach dem Zusammenfuehren ersetzen und den Block loeschen. */
#define o032_0813 disk_free
#define o032_0a36 prepare_dirs
#define Intro_ctor_o035 Finale_ctor
#define Intro_dtor Finale_dtor
#define Intro_run_o035 Finale_run
#define g_3692 g_arc_fx

/* Welle N21: Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define g_group_money g_god_favor_ingerimm
#define g_263c g_binge_torches_lit
#define g_262e g_finster_anger
#define Ev_f1216 ev_offering_bowl
#define Ev_f1350 ev_bridge_toll
#define g_25b7 g_low_kysira_ref
#define g_25c4 g_low_beggars_scared
#define g_25c5 g_low_notices_read
#define g_25c7 g_low_deringorn_state
#define g_25c8 g_low_brooch_state
#define g_25c9 g_low_guard_talk
#define g_25ca g_low_gate1_open
#define g_25cb g_low_gate2_open
#define g_25cc g_low_dragan_task1
#define g_25ce g_low_ailian_state
#define g_25d4 g_low_falkert_known
#define g_25d6 g_low_dragan_news
#define g_25d7 g_low_gavron_house
#define g_26c1 g_low_vindaria_known
#define g_2760 g_low_dragan_task3
#define g_275a g_low_bridge_y
#define g_27eb g_god_favor_tsa
#define g_26e1 g_mine_cart_pos
#define o067_0000 camp_encounter
#define g_2708 g_dng16_forge_timer
#define g_2709 g_dng16_wall_timer

/* Welle N17: Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define grammar_item_name grammar_pronoun
#define g_mon_f47 g_mon_genders
#define script_load conv_run
#define Dlg_ctor_o257 Conv_ctor
#define Dlg_dtor Conv_dtor
#define dlg_set_flag conv_set_topic
#define dlg_flag1 conv_topic_on
#define dlg2_dec_counter conv_dec_counter

#define prn_ctor Diary_ctor
#define o184_0479 ev_show_pic_197
#define o257_0bb3 conv_save
#define o257_0c1a conv_load
#define o257_0ea4 conv_unlock_topic
#define o257_0d89 conv_clr_counter
#define o056_0594 grammar_def_article
#define g_c918 (&g_talk)
/* Welle N23: Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define o059_0000 binge_wall_hole
#define o059_074f binge_shaft
#define g_26d2 g_dng13_wheel_turned
#define g_2635 g_dng13_smoke_timer
#define hero_addiction_tick hero_fur_tick
#define town13_damage dng13_smoke_damage
#define g_2746 g_blackmage_done

/* Welle N24: Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define area_event_o131 dng03_stone_thief
#define event_creature event_dragon_claw
#define g_2784 g_dng03_thief_pending
#define g_25e4 g_dng03_password_solved
#define g_creature_met g_dragon_claw_found
#define g_creature_active g_coward_stuck
#define area_event_o132 dng01_entry_fight
#define o132_10ba dng01_melt_ice
#define g_276c g_dng01_ice_state
#define g_2607 g_dng04_rantzen_left
#define g_2629 g_dng04_monster_killed
#define g_262a g_dng04_tower_open
#define area_event_o204 dng05_poltergeist
#define g_2774 g_dng05_poltergeist
#define o262_0000 blut_nop
#define o262_0458 blut_rockfall
#define g_2670 g_dng06_beer_drunk
#define g_2672 g_dng06_dogs_state
#define o265_0000 dng07_door
#define o265_03db dng07_shelves
#define o265_0812 dng07_gold_statue
#define o265_0919 dng07_straw
#define o265_0ada dng07_graffiti
#define o265_0b44 dng07_web
#define o265_0f71 dng07_web_corpse
#define o266_0000 dng07_fights
#define g_dng07_pits_armed g_dng07_alarm
#define g_dng07_voices_heard g_dng07_guards_beaten

/* Altnamen der Welle N22 (Ortsereignisse 2, Stadtkarte, Kalender) - nach dem Zusammenfuehren ersetzen und den
 * Block loeschen. */
#define Tab_ctor Items_ctor
#define Tab_dtor Items_dtor
#define Tab_add items_change_price
#define pick_lock deer_hunt
#define g_270f g_low_arrival_due
#define o179_0000 inn_talk
#define Obj179_f0115 inn_apply_skill
#define Obj179_f0188 inn_apply_magic
#define Obj179_f029c inn_theft_morning
#define Obj179_f0403 inn_theft_night
#define o259_0023 conv_finish
#define encounter_run ev_gavron_chase
#define g_encounter_state g_gavron_chase
#define g_2766 g_tjolmar_vault
#define Map_ctor_o187 TMap_ctor
#define Map_dtor_o187 TMap_dtor
#define Map_newpal tmap_new_pal
#define Map_mapx tmap_screen_x
#define Map_mapy tmap_screen_y
#define Map_freepal tmap_free_pal
#define Map_setpal tmap_set_pal
#define Map_load_o187 tmap_load
#define Map_setpos tmap_set_pos
#define Map_draw_o187 tmap_draw
#define Map_move tmap_follow
#define Map_line tmap_plot_points
#define Map_fr1 tmap_mark1_free
#define Map_fr2 tmap_mark2_free
#define area_event_o264 dng07_prison_day
#define g_275c g_dng07_prison_event
#define g_25d5 g_dragan_task
#define o259_0931 conv_bye_scene
#define o163_00c8 ford_crossing
#define o163_01f2 ferry_crossing
#define o165_0180 valley_ambush
#define o165_027a mountain_rope
#define o185_0e95 lowangen_drop_food
#define g_25d1 g_dragan_tip_jandora
#define g_25d3 g_dragan_tip_raisha
#define g_261f g_street_guards
#define g_2620 g_street_thugs
#define o163_0000 orc_camp_luggage
#define o163_00c3 place_event_nop
#define o163_0d85 swamp_path
#define o165_0000 group_damage
#define o165_03d0 rockslide_dig
#define o165_07da avalanche
#define g_25b8 g_music_played

/* Welle KS (SEG001/SEG003/SEG023 und Startpruefung R05 als C): Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define sys_exit cleanup_game
#define music_init CopyProt_ctor
#define music_done CopyProt_dtor
#define ems_present cpu_v86_active
#define r03_0006 copyprot_code_begin
#define savegame_op savemenu_save
#define r05_0012 cd_check
#define r23_000f timer_init
#define r43_0130 dos_get_indos_ptr
#define g_music_ctl g_copyprot
#define g_c6c6 g_tamper_count
#define g_c4ff g_indos_flag

/* Abschlusswelle B (Assembler-Module SEG022 ... O270): Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define r31_0020 wait_key_clear
#define r39_015e xms_local_a20_on
#define r39_017a xms_local_a20_off
#define r43_003c save_display_stat

/* Abschlusswelle C (3D-Engine SEG024/SEG025): Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define fill_smth edges_clear
#define poly_setup poly_edges
#define poly_setup_clip poly_edges_persp
#define poly_tex poly_tex_cols
#define poly_tex_m poly_tex_cols_m
#define poly_tex_p poly_tex_rows
#define poly_tex_mp poly_tex_rows_m
#define r25_06c4 view_set_projection
#define g_8c7e g_nr_clip_faces
#define g_94a6 g_face_depth

/* Abschlusswelle A (Datennamen, Funktionsnamen, Halbnamen): Kompatibilitaet fuer alte Namen (bitte die neuen benutzen) */
#define g_1530 g_fli_pal
#define g_258a g_trial_lost
#define g_258d g_artherion_heard
#define g_258e g_artherion_hut_known
#define g_258f g_artherion_hut_visited
#define g_259b g_low_tunnel_known
#define g_25a6 g_topic_binge
#define g_25a8 g_topic_salamander_stone
#define g_25a9 g_topic_ingramosch
#define g_25aa g_topic_sternenschweif
#define g_25ab g_topic_siebenquell
#define g_25ac g_topic_egelbronn
#define g_26ee g_topic_gavron
#define g_25ae g_topic_artherion
#define g_256b g_topic_kvirasim
#define g_256c g_topic_gashok
#define g_256d g_topic_lowangen
#define g_256e g_topic_tiefhusen
#define g_256f g_topic_tjolmar
#define g_2570 g_topic_finsterkoppen
#define g_2571 g_topic_ansvell
#define g_2572 g_topic_salamandersteine
#define g_2573 g_topic_finsterkamm
#define g_2574 g_topic_svellttal
#define g_2575 g_topic_svelltsuempfe
#define g_2576 g_topic_thasch
#define g_2577 g_topic_blutzinnen
#define g_2578 g_topic_rorwhed
#define g_2634 g_elsurion_met
#define g_25c6 g_eolan_state
#define g_25cd g_altrogolan_absent
#define g_25d0 g_vindaria_state
#define g_25cf g_vindaria_choice
#define g_2626 g_inn_vision_state
#define g_2631 g_low_stone_delivered
#define g_2632 g_low_stone_days
#define g_26c9 g_dng16_double_key_state
#define g_5d46 g_dng16_key_event1
#define g_5d4a g_dng16_key_event2
#define g_26da g_binge_anvil_hours
#define g_2743 g_jail_freed
#define g_2745 g_gashok_arrested
#define g_2757 g_npc8_days
#define g_2773 g_phex_finale_done
#define g_2792 g_dng07_was_prisoner
#define g_2794 g_low_tunnel_arrival
#define g_2a62 g_inn_room_rolls
#define g_2a6b g_shop_food_rolls
#define g_2a70 g_shed_free_today
#define g_2a71 g_dragan_room_today
#define g_month_rolls g_market_rolls
#define g_37f4 g_font_dst
#define g_393f g_sheet_enabled
#define g_3994 g_resting
#define g_3996 g_leave_location
#define g_5d50 g_last_cam_cell_y
#define g_5d52 g_last_cam_cell_x
#define g_949e g_view_input_on
#define g_b11f g_step_pending
#define g_b4a0 g_view_input_blocked
#define g_no_portrait_msg g_silent_mode
#define g_b5e0 g_silent_mode2
#define g_b61e g_dlg_box_flag
#define g_b76d g_status_on
#define g_c0fb g_travel
#define g_c4da g_on_boat
#define g_c696 g_menu_line
#define g_c776 g_select_shown
#define g_c778 g_select_next
#define g_c7f6 g_view_pos_x
#define g_c7fa g_view_pos_y
#define g_c824 g_camp_fight_pending
#define g_c503 g_hero_slot_aux
#define g_04a8 g_cl_inventory
#define g_04ee g_cl_inventory_st
#define g_32b7 g_items_attack
#define g_3271 g_items_ask_use
#define g_261c g_heather_recipe_read
#define g_26e8 g_ingerimm_verse_read
#define g_26ed g_parchments_read
#define g_b6ff g_menu_tail
#define g_2542 g_opt_fight_keep_view
#define g_32c5 g_teleport_spin
#define g_25af g_kvirasim_arrived
#define g_2581 g_armed_fight_lost
#define g_c6c0 g_automap_redraw
#define g_8b8f g_view_swapped
#define g_b11c g_view_indoor
#define g_9484 g_view_init_flag
#define g_260b g_pic2_seen
#define g_2593 g_rondra_priestess_met
#define g_c689 g_fig_action_flag
#define g_c68a g_enemy_unmushroomed
#define g_25c3 g_dream_seen
#define g_b7b4 g_screen_pic_state
#define g_25b4 g_low25_topic59
#define g_bda8 g_decomp_work
#define g_26f5 g_quest_anim_seen
#define g_b5bc g_blink_off
#define g_3947 g_temple_visited
#define g_3949 g_temple_visited2
#define g_3903 g_speech_off
#define g_3905 g_sound_ready
#define g_387a g_music_off
#define g_387c g_music_ready
#define g_c1c5 g_route_km
#define g_c1ca g_camp_weather
#define g_0500 g_inv_warn_enabled
#define g_b178 g_stars_reset
#define g_8bd3 g_mouse_flag
#define g_26f3 g_finale_done
#define g_262f g_game_finished2
#define g_391b g_speech_pos
#define g_78d8 g_mouse_isr_lock
#define g_b627 g_bmp_spare
#define g_5f08 g_mem_debug2
#define g_2595 g_tev000_warrior_cursing
#define g_279a g_reward2_given
#define g_2799 g_reward_given
#define g_2628 g_swamp_chief_offended
#define g_2627 g_swamp_chief_refused
#define g_667f g_fight_ab_loot
#define g_283b g_dng06_alarm_delay
#define g_c68f g_date_flag
#define g_261d g_street_flag1
#define g_261e g_street_flag2
#define g_25b6 g_low34_ch_passed
#define g_94c4 g_face_list_seg
#define o030_0bda loc_return_redraw
#define o030_0cbe loc_fade_out
#define o097_nop unused_view_stub
#define o118_0000 unused_tevent_o118
#define o122_07c5 unused_tevent_o122
#define o011_stub_0a88 unused_dev_stub1
#define o011_stub_0a8d unused_dev_stub2
#define o011_stub_0a97 unused_dev_stub3
#define o007_stub_1456 unused_scene_stub
#define talk_check_26f5 talk_check_quest_anim
#define counter_280d_tick inn_days_tick
#define speech_get_391b speech_get_pos
#define conv_cb_unused_0a67 conv_cb_unused35
#define g_dng01_fight_0101 g_dng01_cave_spiders_0101
#define g_dng01_fight_0806 g_dng01_cultists_0806
#define g_dng01_fight_0e06 g_dng01_cultists_0e06
#define g_dng01_fight_1808 g_dng01_cave_spiders_1808
#define g_dng02_fight_020b g_dng02_cultists_020b
#define g_dng02_fight_0402 g_dng02_cultists_0402
#define g_dng02_fight_041c g_dng02_cultists_041c
#define g_dng02_fight_070d g_dng02_cultists_070d
#define g_dng02_fight_0802 g_dng02_cultists_0802
#define g_dng02_fight_0b17 g_dng02_sabretooth_0b17
#define g_dng02_fight_0f0c g_dng02_sabretooth_0f0c
#define g_dng02_fight_0f1c g_dng02_sabretooth_0f1c
#define g_dng02_fight_150c g_dng02_cave_spiders_150c
#define g_dng02_fight_171d g_dng02_cultists_171d
#define g_dng02_fight_1c05 g_dng02_druids_1c05
#define g_dng02_fight_1c1b g_dng02_cultists_1c1b
#define g_dng03_fight_041d g_dng03_skeleton_warriors_041d
#define g_dng03_fight_0702 g_dng03_mages_0702
#define g_dng03_fight_0704 g_dng03_mages_0704
#define g_dng03_fight_080c g_dng03_sorcerer_080c
#define g_dng03_fight_0b17 g_dng03_skeleton_warriors_0b17
#define g_dng03_fight_0d01 g_dng03_priests_0d01
#define g_dng03_fight_0e0c g_dng03_sorcerer_0e0c
#define g_dng03_fight_0e1d g_dng03_knight_0e1d
#define g_dng03_fight_130e g_dng03_heshthot_130e
#define g_dng03_fight_1313 g_dng03_zombies_1313
#define g_dng03_fight_1519 g_dng03_zombies_1519
#define g_dng03_fight_171d g_dng03_skeleton_warriors_171d
#define g_dng03_fight_191c g_dng03_skeletons_191c
#define g_dng03_fight_1b15 g_dng03_zombies_1b15
#define g_dng04_fight_0109 g_dng04_goblins_0109
#define g_dng04_fight_040d g_dng04_goblins_040d
#define g_dng04_fight_0413 g_dng04_orcs_0413
#define g_dng04_fight_0601 g_dng04_orcs_0601
#define g_dng04_fight_0919 g_dng04_zombies_0919
#define g_dng04_fight_1219 g_dng04_zombies_1219
#define g_dng04_fight_1402 g_dng04_orcs_1402
#define g_dng04_fight_1c02 g_dng04_orcs_1c02
#define g_dng04_fight_1e08 g_dng04_rantzen_1e08
#define g_dng04_fight_200c g_dng04_orcs_200c
#define g_dng04_fight_2510 g_dng04_orcs_2510
#define g_dng04_fight_2d0e g_dng04_zombies_2d0e
#define g_dng05_fight_020e g_dng05_skeletons_020e
#define g_dng05_fight_021e g_dng05_mummies_021e
#define g_dng05_fight_0819 g_dng05_priests_0819
#define g_dng05_fight_091f g_dng05_mummies_091f
#define g_dng05_fight_0a0a g_dng05_priests_0a0a
#define g_dng05_fight_0a0e g_dng05_priests_0a0e
#define g_dng05_fight_0e1d g_dng05_mummies_0e1d
#define g_dng05_fight_1214 g_dng05_skeletons_1214
#define g_dng05_fight_1501 g_dng05_priests_1501
#define g_dng05_fight_1a1d g_dng05_skeletons_1a1d
#define g_dng05_fight_1b1c g_dng05_skeletons_1b1c
#define g_dng05_fight_1b1e g_dng05_skeletons_1b1e
#define g_dng05_fight_1c0d g_dng05_priests_1c0d
#define g_dng05_fight_1c1d g_dng05_skeletons_1c1d
#define g_dng06_fight_0402 g_dng06_orcs_0402
#define g_dng06_fight_0e10 g_dng06_orcs_0e10
#define g_dng06_fight_0f03 g_dng06_orcs_0f03
#define g_dng06_fight_1009 g_dng06_orcs_1009
#define g_dng06_fight_1503 g_dng06_orcs_1503
#define g_dng06_fight_1b04 g_dng06_orcs_1b04
#define g_dng06_fight_1d02 g_dng06_orc_veteran_1d02
#define g_dng07_fight_0305 g_dng07_orcs_0305
#define g_dng07_fight_0319 g_dng07_orcs_0319
#define g_dng07_fight_050f g_dng07_orcs_050f
#define g_dng07_fight_0a0d g_dng07_orcs_0a0d
#define g_dng07_fight_0d08 g_dng07_orcs_0d08
#define g_dng07_fight_100b g_dng07_orcs_100b
#define g_dng07_fight_120f g_dng07_orcs_120f
#define g_dng07_fight_140b g_dng07_orcs_140b
#define g_dng07_fight_1606 g_dng07_orcs_1606
#define g_dng08_fight_020d g_dng08_orcs_020d
#define g_dng08_fight_0309 g_dng08_orcs_0309
#define g_dng08_fight_0511 g_dng08_orcs_0511
#define g_dng08_fight_0a06 g_dng08_orcs_0a06
#define g_dng08_fight_0d04 g_dng08_orcs_0d04
#define g_dng08_fight_0d0f g_dng08_orcs_0d0f
#define g_dng08_fight_0e06 g_dng08_orcs_0e06
#define g_dng08_fight_0e19 g_dng08_ogres_0e19
#define g_dng08_fight_0f0c g_dng08_orcs_0f0c
#define g_dng08_fight_1110 g_dng08_orcs_1110
#define g_dng08_fight_1113 g_dng08_orcs_1113
#define g_dng08_fight_121b g_dng08_orcs_121b
#define g_dng08_fight_1516 g_dng08_orcs_1516
#define g_dng08_fight_1603 g_dng08_ogres_1603
#define g_dng08_fight_1907 g_dng08_orc_veteran_1907
#define g_dng08_fight_1a11 g_dng08_orcs_1a11
#define g_dng08_fight_1e15 g_dng08_orcs_1e15
#define g_dng08_fight_1e19 g_dng08_orcs_1e19
#define g_fig_hero_frames_65b0 g_fig_hero_pose_frames
#define g_conv_topic_files (g_conv_topic_tab - 7)
/* Ende Abschlusswelle A */

#endif
