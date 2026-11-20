/*
 * dsa_dgroup_loader.inc - Laufzeit-Lader fuer das initialisierte Datensegment (DGROUP)
 * aus der Originaldatei SCHWEIF.EXE (Version C1.02 de, 1.387.552 Bytes,
 * MD5 da3b59ce86a82e4b8bb14d36ed37d4d0, Dateioffset 0x026EB0, Laenge 0x8B06 = 35.590 Bytes).
 *
 * Enthaelt 0 Originalbytes aus SCHWEIF.EXE.
 */

unsigned char g_schweif_dgroup[0x8b06];

/* Symbole ohne Deklaration in s_data.h */
extern unsigned char g_unused_0fd2[];
extern unsigned char g_unused_1534[];
extern unsigned char g_playflags_2[];
extern unsigned char g_unused_253e[];
extern unsigned char g_unused_254e[];
extern unsigned char g_unused_2550[];
extern unsigned char g_unused_257b[];
extern unsigned char g_unused_2589[];
extern unsigned char g_unused_2591[];
extern unsigned char g_unused_2598[];
extern unsigned char g_unused_25a7[];
extern unsigned char g_unused_25ad[];
extern unsigned char g_unused_25b3[];
extern unsigned char g_unused_260a[];
extern unsigned char g_unused_2649[];
extern unsigned char g_unused_2662[];
extern unsigned char g_unused_26c0[];
extern unsigned char g_unused_26c2[];
extern unsigned char g_unused_26d0[];
extern unsigned char g_unused_26db[];
extern unsigned char g_unused_26ff[];
extern unsigned char g_unused_270d[];
extern unsigned char g_unused_274b[];
extern unsigned char g_unused_274f[];
extern unsigned char g_unused_2752[];
extern unsigned char g_unused_2759[];
extern unsigned char g_unused_2772[];
extern unsigned char g_unused_279b[];
extern unsigned char g_unused_27d7[];
extern unsigned char g_unused_27e7[];
extern unsigned char g_god_favor_9_10[];
extern unsigned char g_unused_27fb[];
extern unsigned char g_loc_saved_flag[];
extern unsigned char g_unused_284b[];
extern unsigned char g_market_rolls_head[];
extern unsigned char g_unused_2a95[];
extern unsigned char g_unused_3221[];
extern unsigned char g_unused_325b[];
extern unsigned char g_3327[];
extern unsigned char g_3331[];
extern unsigned char g_gui_buttons_rest[];
extern unsigned char g_unused_35c5[];
extern unsigned char g_font_rest[];
extern unsigned char g_font_tabs_rest[];
extern unsigned char g_unused_3851[];
extern unsigned char g_unused_387e[];
extern unsigned char g_unused_3894[];
extern unsigned char g_snd_fx_rest[];
extern unsigned char g_unused_391d[];
extern unsigned char g_unused_3943[];
extern unsigned char g_unused_394d[];
extern unsigned char g_loc_exit_mark[];
extern unsigned char g_unused_39a0[];
extern unsigned char g_39b5[];
extern unsigned char g_39c0[];
extern unsigned char g_39ca[];
extern unsigned char g_39d7[];
extern unsigned char g_39e3[];
extern unsigned char g_39ee[];
extern unsigned char g_39fa[];
extern unsigned char g_3a03[];
extern unsigned char g_3a0b[];
extern unsigned char g_3a12[];
extern unsigned char g_3a18[];
extern unsigned char g_3a23[];
extern unsigned char g_3a2d[];
extern unsigned char g_3a35[];
extern unsigned char g_3a3c[];
extern unsigned char g_3a47[];
extern unsigned char g_3a53[];
extern unsigned char g_temple_locs_praios[];
extern unsigned char g_temple_locs_rondra[];
extern unsigned char g_temple_locs_efferd[];
extern unsigned char g_temple_locs_travia[];
extern unsigned char g_temple_locs_boron[];
extern unsigned char g_temple_locs_hesinde[];
extern unsigned char g_temple_locs_firun[];
extern unsigned char g_temple_locs_tsa[];
extern unsigned char g_temple_locs_phex[];
extern unsigned char g_temple_locs_peraine[];
extern unsigned char g_temple_locs_ingerimm[];
extern unsigned char g_temple_locs_rahja[];
extern unsigned char g_temple_locs_swafnir[];
extern unsigned char g_temple_locs_ifirn[];
extern unsigned char g_conv_areas[];
extern unsigned char g_str_das[];
extern unsigned char g_str_ein[];
extern unsigned char g_str_eine[];
extern unsigned char g_str_eines[];
extern unsigned char g_str_einer[];
extern unsigned char g_str_einen[];
extern unsigned char g_str_eine2[];
extern unsigned char g_str_einem[];
extern unsigned char g_str_no_article[];
extern unsigned char g_str_von[];
extern unsigned char g_str_es[];
extern unsigned char g_unused_65a0[];
extern unsigned char g_unused_65e7[];
extern unsigned char g_str_dot_6c64[];
extern unsigned char g_unused_6ce3[];
extern unsigned char g_unused_7255[];
extern unsigned char g_unused_7710[];
extern unsigned char g_unused_771d[];
extern unsigned char g_unused_8644[];
extern unsigned char g_unused_5efe[];
extern unsigned char g_unused_5f0e[];

struct dsa_dgroup_copy_entry {
    void    *dst;
    uint16_t ds_off;
    uint16_t size;
};

static const struct dsa_dgroup_copy_entry s_dgroup_copies[] = {
    { (void *)g_dng16_seq_pics              , 0x0094u,    14u }, /* OVL002 */
    { (void *)g_dng16_seq_pics_state        , 0x00a2u,     2u }, /* OVL002 */
    { (void *)&g_dng16_seq_golem_state      , 0x00a4u,     2u }, /* OVL002 */
    { (void *)&g_dng16_seq_forge_state      , 0x00a6u,     2u }, /* OVL002 */
    { (void *)&g_dng16_heat_marks_init      , 0x00a8u,    14u }, /* OVL002 */
    { (void *)&g_cd_volctl                  , 0x00ceu,     9u }, /* OVL004 */
    { (void *)&g_group_ptrs_null            , 0x00f0u,    24u }, /* OVL005 */
    { (void *)&g_select_skill_lvlup         , 0x0108u,     6u }, /* OVL006 */
    { (void *)&g_select_skill_defaults      , 0x010eu,    10u }, /* OVL006 */
    { (void *)g_cl_blackmage                , 0x011au,     6u }, /* OVL007 */
    { (void *)g_cl_blackmage_speech         , 0x0120u,    20u }, /* OVL007 */
    { (void *)g_cl_blackmage_st             , 0x0134u,     4u }, /* OVL007 */
    { (void *)g_cl_phex_finale              , 0x0138u,     6u }, /* OVL007 */
    { (void *)g_cl_phex_finale_speech       , 0x013eu,    32u }, /* OVL007 */
    { (void *)g_cl_phex_finale_st           , 0x015eu,     4u }, /* OVL007 */
    { (void *)&g_scene_mark_on              , 0x0162u,     2u }, /* OVL007 */
    { (void *)&g_scene_mark_off             , 0x0164u,     2u }, /* OVL007 */
    { (void *)&g_scene_fmt                  , 0x0166u,     7u }, /* OVL007 */
    { (void *)g_inv_status_slots            , 0x0172u,    10u }, /* OVL008 */
    { (void *)g_inv_slot_xy                 , 0x017cu,   132u }, /* OVL008 */
    { (void *)g_inv_mouse_knapsack          , 0x0200u,    80u }, /* OVL008 */
    { (void *)g_inv_areas_knapsack          , 0x0250u,   180u }, /* OVL008 */
    { (void *)g_inv_mouse_all               , 0x0304u,    30u }, /* OVL008 */
    { (void *)g_inv_areas_all               , 0x0322u,   350u }, /* OVL008 */
    { (void *)g_areas_sheet                 , 0x0480u,    40u }, /* OVL008 */
    { (void *)g_cl_inventory                , 0x04a8u,    70u }, /* OVL008 */
    { (void *)g_cl_inventory_st             , 0x04eeu,    10u }, /* OVL008 */
    { (void *)&g_inv_warn_enabled           , 0x0500u,     2u }, /* OVL009 */
    { (void *)&g_warn_blink_time            , 0x0502u,     4u }, /* OVL009 */
    { (void *)&g_warn_blink_init            , 0x0506u,     1u }, /* OVL009 */
    { (void *)&g_warn_snd_time              , 0x0507u,     4u }, /* OVL009 */
    { (void *)&g_warn_snd_init              , 0x050bu,     1u }, /* OVL009 */
    { (void *)&g_warn_blink_on              , 0x050cu,     2u }, /* OVL009 */
    { (void *)&g_str_emm                    , 0x050eu,     9u }, /* OVL010 */
    { (void *)g_err_swap_write              , 0x0517u,    41u }, /* OVL010 */
    { (void *)g_err_swap_read               , 0x0540u,    43u }, /* OVL010 */
    { (void *)g_fmt_tex_debug               , 0x056bu,    49u }, /* OVL010 */
    { (void *)g_cutscene_pics               , 0x0630u,     6u }, /* OVL013 */
    { (void *)&g_talk_tpl                   , 0x0636u,   122u }, /* OVL013 */
    { (void *)&g_cutscene_sub1              , 0x06b0u,     2u }, /* OVL013 */
    { (void *)&g_cutscene_sub2              , 0x06b2u,     2u }, /* OVL013 */
    { (void *)g_dng_occl_05                 , 0x06b4u,    24u }, /* OVL014 */
    { (void *)g_dng_occl_01                 , 0x06ccu,    29u }, /* OVL014 */
    { (void *)g_dng_occl_02                 , 0x06e9u,    24u }, /* OVL014 */
    { (void *)g_dng_occl_03                 , 0x0701u,    24u }, /* OVL014 */
    { (void *)g_dng_occl_09                 , 0x0719u,    21u }, /* OVL014 */
    { (void *)g_dng_occl_10                 , 0x072eu,    24u }, /* OVL014 */
    { (void *)g_dng_occl_11                 , 0x0746u,    24u }, /* OVL014 */
    { (void *)g_dng_occl_12                 , 0x075eu,    24u }, /* OVL014 */
    { (void *)g_dng_occl_13                 , 0x0776u,    24u }, /* OVL014 */
    { (void *)g_dng_occl_14                 , 0x078eu,    24u }, /* OVL014 */
    { (void *)g_dng_occl_16                 , 0x07a6u,    24u }, /* OVL014 */
    { (void *)g_dng_objs_01                 , 0x07beu,    87u }, /* OVL014 */
    { (void *)g_dng_objs_02                 , 0x0815u,    63u }, /* OVL014 */
    { (void *)g_dng_objs_03                 , 0x0854u,    69u }, /* OVL014 */
    { (void *)g_dng_objs_04                 , 0x0899u,    81u }, /* OVL014 */
    { (void *)g_dng_objs_09                 , 0x08eau,    66u }, /* OVL014 */
    { (void *)g_dng_objs_05                 , 0x092cu,    69u }, /* OVL014 */
    { (void *)g_dng_objs_06                 , 0x0971u,    90u }, /* OVL014 */
    { (void *)g_dng_objs_07                 , 0x09cbu,    72u }, /* OVL014 */
    { (void *)g_dng_objs_08                 , 0x0a13u,    75u }, /* OVL014 */
    { (void *)g_dng_objs_10                 , 0x0a5eu,    57u }, /* OVL014 */
    { (void *)g_dng_objs_11                 , 0x0a97u,    60u }, /* OVL014 */
    { (void *)g_dng_objs_12                 , 0x0ad3u,    78u }, /* OVL014 */
    { (void *)g_dng_objs_13                 , 0x0b21u,    66u }, /* OVL014 */
    { (void *)g_dng_objs_14                 , 0x0b63u,    33u }, /* OVL014 */
    { (void *)g_dng_objs_16                 , 0x0b84u,    75u }, /* OVL014 */
    { (void *)g_dng_pics                    , 0x0cefu,    34u }, /* OVL014 */
    { (void *)g_dng_pics_st                 , 0x0d11u,     4u }, /* OVL014 */
    { (void *)g_dng_pics_01                 , 0x0d15u,    22u }, /* OVL014 */
    { (void *)g_dng_pics_01_st              , 0x0d2bu,     4u }, /* OVL014 */
    { (void *)g_dng_pics_02                 , 0x0d2fu,    24u }, /* OVL014 */
    { (void *)g_dng_pics_02_st              , 0x0d47u,     4u }, /* OVL014 */
    { (void *)g_dng_pics_03                 , 0x0d4bu,    32u }, /* OVL014 */
    { (void *)g_dng_pics_03_st              , 0x0d6bu,     4u }, /* OVL014 */
    { (void *)g_dng_pics_04                 , 0x0d6fu,    20u }, /* OVL014 */
    { (void *)g_dng_pics_04_st              , 0x0d83u,     4u }, /* OVL014 */
    { (void *)g_dng_pics_05                 , 0x0d87u,    38u }, /* OVL014 */
    { (void *)g_dng_pics_05_st              , 0x0dadu,     6u }, /* OVL014 */
    { (void *)g_dng_pics_06                 , 0x0db3u,    26u }, /* OVL014 */
    { (void *)g_dng_pics_06_st              , 0x0dcdu,     4u }, /* OVL014 */
    { (void *)g_dng_pics_07                 , 0x0dd1u,    22u }, /* OVL014 */
    { (void *)g_dng_pics_07_st              , 0x0de7u,     4u }, /* OVL014 */
    { (void *)g_dng_pics_08                 , 0x0debu,    18u }, /* OVL014 */
    { (void *)g_dng_pics_08_st              , 0x0dfdu,     2u }, /* OVL014 */
    { (void *)g_dng_pics_09                 , 0x0dffu,    18u }, /* OVL014 */
    { (void *)g_dng_pics_09_st              , 0x0e11u,     2u }, /* OVL014 */
    { (void *)g_dng_pics_10                 , 0x0e13u,    32u }, /* OVL014 */
    { (void *)g_dng_pics_10_st              , 0x0e33u,     4u }, /* OVL014 */
    { (void *)g_dng_pics_11                 , 0x0e37u,    20u }, /* OVL014 */
    { (void *)g_dng_pics_11_st              , 0x0e4bu,     4u }, /* OVL014 */
    { (void *)g_dng_pics_12                 , 0x0e4fu,    18u }, /* OVL014 */
    { (void *)g_dng_pics_12_st              , 0x0e61u,     2u }, /* OVL014 */
    { (void *)g_dng_pics_13                 , 0x0e63u,    18u }, /* OVL014 */
    { (void *)g_dng_pics_13_st              , 0x0e75u,     2u }, /* OVL014 */
    { (void *)g_dng_pics_14                 , 0x0e77u,    18u }, /* OVL014 */
    { (void *)g_dng_pics_14_st              , 0x0e89u,     2u }, /* OVL014 */
    { (void *)g_dng_pics_16                 , 0x0e8bu,    32u }, /* OVL014 */
    { (void *)g_dng_pics_16_st              , 0x0eabu,     4u }, /* OVL014 */
    { (void *)g_dng_nogroup_01              , 0x0eafu,     3u }, /* OVL014 */
    { (void *)g_dng_nogroup_02              , 0x0eb2u,     3u }, /* OVL014 */
    { (void *)g_dng_nogroup_03              , 0x0eb5u,     4u }, /* OVL014 */
    { (void *)g_dng_nogroup_05              , 0x0eb9u,     4u }, /* OVL014 */
    { (void *)g_dng_nogroup_06              , 0x0ebdu,     4u }, /* OVL014 */
    { (void *)g_dng_nogroup_07              , 0x0ec1u,     4u }, /* OVL014 */
    { (void *)g_dng_nogroup_08              , 0x0ec5u,     6u }, /* OVL014 */
    { (void *)g_dng_nogroup_12              , 0x0ecbu,     3u }, /* OVL014 */
    { (void *)g_dng_nogroup_16              , 0x0eceu,     5u }, /* OVL014 */
    { (void *)g_dng_camp_04                 , 0x0ed3u,     4u }, /* OVL014 */
    { (void *)g_dng_nocamp_13               , 0x0ed7u,     3u }, /* OVL014 */
    { (void *)g_dng_nocamp_15               , 0x0edau,     4u }, /* OVL014 */
    { (void *)g_zoom_slot_xy                , 0x0edeu,    60u }, /* OVL015 */
    { (void *)g_zoom_skip_items             , 0x0f1au,     4u }, /* OVL015 */
    { (void *)&g_zoom_frame_color           , 0x0f1eu,     4u }, /* OVL015 */
    { (void *)&g_anim_delays                , 0x0f22u,     8u }, /* OVL016 */
    { (void *)&g_fight_use_ems              , 0x0f34u,     2u }, /* OVL018 */
    { (void *)&g_file3_marked               , 0x0f36u,     2u }, /* OVL018 */
    { (void *)g_fight_pic_keys              , 0x0f38u,    92u }, /* OVL018 */
    { (void *)g_fight_pic_states            , 0x0f94u,    12u }, /* OVL018 */
    { (void *)&g_fight_map_file             , 0x0fa0u,     8u }, /* OVL018 */
    { (void *)&g_finster_anger_stage        , 0x0fa6u,     2u }, /* OVL020 */
    { (void *)&g_sfight_frames              , 0x0fa8u,    14u }, /* OVL020 */
    { (void *)&g_sfight_intro_events        , 0x0fb6u,    16u }, /* OVL020 */
    { (void *)&g_sfight_end_events          , 0x0fc6u,    12u }, /* OVL020 */
    { (void *)g_unused_0fd2                 , 0x0fd2u,     2u }, /* OVL020 */
    { (void *)g_sfight_targets_b            , 0x0fd4u,    12u }, /* OVL020 */
    { (void *)g_sfight_targets_c            , 0x0fe0u,    12u }, /* OVL020 */
    { (void *)g_sfight_targets_d            , 0x0fecu,    12u }, /* OVL020 */
    { (void *)g_sfight_targets_a            , 0x0ff8u,    88u }, /* OVL020 */
    { (void *)g_sfight_targets_far          , 0x1050u,   318u }, /* OVL020 */
    { (void *)&g_sfight2_nr_right           , 0x118cu,     1u }, /* OVL021 */
    { (void *)&g_sfight2_nr_wrong           , 0x118du,     1u }, /* OVL021 */
    { (void *)&g_sfight2_starts             , 0x118eu,    28u }, /* OVL021 */
    { (void *)g_sfight_pics                 , 0x11aau,    58u }, /* OVL022 */
    { (void *)g_sfight_pic_states           , 0x11e4u,     8u }, /* OVL022 */
    { (void *)g_sfight2_pics                , 0x11ecu,    48u }, /* OVL022 */
    { (void *)g_sfight2_pic_states          , 0x121cu,     6u }, /* OVL022 */
    { (void *)g_fli_palette                 , 0x1222u,   768u }, /* OVL025 */
    { (void *)g_unused_1534                 , 0x1534u,     8u }, /* OVL025 */
    { (void *)g_type_le                     , 0x1542u,    26u }, /* OVL026 */
    { (void *)g_type_ae                     , 0x155cu,    26u }, /* OVL026 */
    { (void *)g_type_height                 , 0x1576u,    26u }, /* OVL026 */
    { (void *)g_type_weight                 , 0x1590u,    13u }, /* OVL026 */
    { (void *)g_chargen_areas_main          , 0x159du,    80u }, /* OVL026 */
    { (void *)g_chargen_areas_skills        , 0x15edu,    40u }, /* OVL026 */
    { (void *)g_chargen_areas_spells        , 0x1615u,    40u }, /* OVL026 */
    { (void *)g_portrait_rects              , 0x1669u,    72u }, /* OVL026 */
    { (void *)g_portrait_index_m            , 0x16b1u,    12u }, /* OVL026 */
    { (void *)g_portrait_index_f            , 0x16bdu,    11u }, /* OVL026 */
    { (void *)g_type_money_01               , 0x16c8u,    18u }, /* OVL026 */
    { (void *)g_type_money_02               , 0x16dau,    12u }, /* OVL026 */
    { (void *)g_type_money_03               , 0x16e6u,    36u }, /* OVL026 */
    { (void *)g_type_money_04               , 0x170au,    24u }, /* OVL026 */
    { (void *)g_type_money_05               , 0x1722u,    24u }, /* OVL026 */
    { (void *)g_type_money_06               , 0x173au,    24u }, /* OVL026 */
    { (void *)g_type_money_07               , 0x1752u,     6u }, /* OVL026 */
    { (void *)g_type_money_08               , 0x1758u,    36u }, /* OVL026 */
    { (void *)g_type_money_09               , 0x177cu,    30u }, /* OVL026 */
    { (void *)g_type_money_10               , 0x179au,    30u }, /* OVL026 */
    { (void *)g_type_money_11               , 0x17b8u,    12u }, /* OVL026 */
    { (void *)g_type_money_12               , 0x17c4u,    18u }, /* OVL026 */
    { (void *)g_chargen_pics                , 0x1806u,    44u }, /* OVL026 */
    { (void *)g_chargen_pics_state          , 0x1832u,     6u }, /* OVL026 */
    { (void *)&g_chargen_name_init          , 0x1838u,    16u }, /* OVL026 */
    { (void *)&g_chargen_type_shown         , 0x1848u,     2u }, /* OVL026 */
    { (void *)g_cl_opening                  , 0x184au,    26u }, /* OVL027 */
    { (void *)&g_opening_white              , 0x1868u,     4u }, /* OVL028 */
    { (void *)&g_nr_raw_files               , 0x1966u,     2u }, /* OVL032 */
    { (void *)&g_data_on_cd                 , 0x1968u,     2u }, /* OVL032 */
    { (void *)g_recipes                     , 0x196au,   392u }, /* OVL032 */
    { (void *)g_skill_attribs               , 0x1af2u,   208u }, /* OVL032 */
    { (void *)g_skills_index                , 0x1bc2u,    14u }, /* OVL032 */
    { (void *)g_staffspell_descriptions     , 0x1e80u,    42u }, /* OVL032 */
    { (void *)g_spell_descriptions          , 0x1eaau,  1044u }, /* OVL032 */
    { (void *)g_spells_index                , 0x22beu,    16u }, /* OVL032 */
    { (void *)g_spells_index2               , 0x22ceu,     8u }, /* OVL032 */
    { (void *)g_mon_spell_descriptions      , 0x22d6u,   150u }, /* OVL032 */
    { (void *)g_mon_spell_repertoire        , 0x236cu,    65u }, /* OVL032 */
    { (void *)g_spells_own_test             , 0x2425u,    40u }, /* OVL032 */
    { (void *)g_spells_no_sfight            , 0x244du,    32u }, /* OVL032 */
    { (void *)g_save_runv                   , 0x246du,     1u }, /* OVL032 */
    { (void *)g_ingame_timers               , 0x246eu,   200u }, /* OVL032 */
    { (void *)g_save_runv_end               , 0x2536u,     1u }, /* OVL032 */
    { (void *)g_save_play                   , 0x2537u,     1u }, /* OVL032 */
    { (void *)&g_anims_on                   , 0x2538u,     1u }, /* OVL032 */
    { (void *)&g_free_move                  , 0x2539u,     1u }, /* OVL032 */
    { (void *)g_playflags_2                 , 0x253au,     1u }, /* OVL032 */
    { (void *)&g_subtitles                  , 0x253bu,     1u }, /* OVL032 */
    { (void *)&g_print_to_file              , 0x253cu,     1u }, /* OVL032 */
    { (void *)&g_text_speed                 , 0x253du,     1u }, /* OVL032 */
    { (void *)g_unused_253e                 , 0x253eu,     2u }, /* OVL032 */
    { (void *)&g_anim_speed                 , 0x2540u,     1u }, /* OVL032 */
    { (void *)&g_fight_speed                , 0x2541u,     1u }, /* OVL032 */
    { (void *)&g_opt_fight_keep_view        , 0x2542u,     1u }, /* OVL032 */
    { (void *)&g_win_centered               , 0x2543u,     1u }, /* OVL032 */
    { (void *)&g_opt_music                  , 0x2544u,     1u }, /* OVL032 */
    { (void *)&g_opt_sound                  , 0x2545u,     1u }, /* OVL032 */
    { (void *)&g_cd_speech                  , 0x2546u,     1u }, /* OVL032 */
    { (void *)&g_vol_music                  , 0x2547u,     1u }, /* OVL032 */
    { (void *)&g_vol_fx                     , 0x2548u,     1u }, /* OVL032 */
    { (void *)&g_vol_speech                 , 0x2549u,     1u }, /* OVL032 */
    { (void *)&g_game_mode                  , 0x254au,     1u }, /* OVL032 */
    { (void *)&g_saver_level                , 0x254bu,     1u }, /* OVL032 */
    { (void *)&g_print_width                , 0x254cu,     1u }, /* OVL032 */
    { (void *)&g_print_page_len             , 0x254du,     1u }, /* OVL032 */
    { (void *)g_unused_254e                 , 0x254eu,     1u }, /* OVL032 */
    { (void *)&g_inv_show_body              , 0x254fu,     1u }, /* OVL032 */
    { (void *)g_unused_2550                 , 0x2550u,    26u }, /* OVL032 */
    { (void *)g_flags                       , 0x256au,     1u }, /* OVL032 */
    { (void *)&g_topic_kvirasim             , 0x256bu,     1u }, /* OVL032 */
    { (void *)&g_topic_gashok               , 0x256cu,     1u }, /* OVL032 */
    { (void *)&g_topic_lowangen             , 0x256du,     1u }, /* OVL032 */
    { (void *)&g_topic_tiefhusen            , 0x256eu,     1u }, /* OVL032 */
    { (void *)&g_topic_tjolmar              , 0x256fu,     1u }, /* OVL032 */
    { (void *)&g_topic_finsterkoppen        , 0x2570u,     1u }, /* OVL032 */
    { (void *)&g_topic_ansvell              , 0x2571u,     1u }, /* OVL032 */
    { (void *)&g_topic_salamandersteine     , 0x2572u,     1u }, /* OVL032 */
    { (void *)&g_topic_finsterkamm          , 0x2573u,     1u }, /* OVL032 */
    { (void *)&g_topic_svellttal            , 0x2574u,     1u }, /* OVL032 */
    { (void *)&g_topic_svelltsuempfe        , 0x2575u,     1u }, /* OVL032 */
    { (void *)&g_topic_thasch               , 0x2576u,     1u }, /* OVL032 */
    { (void *)&g_topic_blutzinnen           , 0x2577u,     1u }, /* OVL032 */
    { (void *)&g_topic_rorwhed              , 0x2578u,     1u }, /* OVL032 */
    { (void *)&g_tev_peddler_trail          , 0x2579u,     1u }, /* OVL032 */
    { (void *)&g_npc6_met                   , 0x257au,     1u }, /* OVL032 */
    { (void *)g_unused_257b                 , 0x257bu,     1u }, /* OVL032 */
    { (void *)&g_once_anim                  , 0x257cu,     1u }, /* OVL032 */
    { (void *)&g_slow5_on                   , 0x257du,     1u }, /* OVL032 */
    { (void *)&g_slow4_on                   , 0x257eu,     1u }, /* OVL032 */
    { (void *)&g_valley_asked               , 0x257fu,     1u }, /* OVL032 */
    { (void *)&g_mountain_asked             , 0x2580u,     1u }, /* OVL032 */
    { (void *)&g_armed_fight_lost           , 0x2581u,     1u }, /* OVL032 */
    { (void *)&g_tev099_rockslide           , 0x2582u,     1u }, /* OVL032 */
    { (void *)&g_salamander_reported        , 0x2583u,     1u }, /* OVL032 */
    { (void *)&g_tev_avalanche_done         , 0x2584u,     1u }, /* OVL032 */
    { (void *)&g_tev147_rockslide           , 0x2585u,     1u }, /* OVL032 */
    { (void *)&g_tev148_rockslide           , 0x2586u,     1u }, /* OVL032 */
    { (void *)&g_tev149_rockslide           , 0x2587u,     1u }, /* OVL032 */
    { (void *)&g_tev151_rockslide           , 0x2588u,     1u }, /* OVL032 */
    { (void *)g_unused_2589                 , 0x2589u,     1u }, /* OVL032 */
    { (void *)&g_trial_lost                 , 0x258au,     1u }, /* OVL032 */
    { (void *)&g_shrine_state               , 0x258bu,     1u }, /* OVL032 */
    { (void *)&g_shrine_paid                , 0x258cu,     1u }, /* OVL032 */
    { (void *)&g_artherion_heard            , 0x258du,     1u }, /* OVL032 */
    { (void *)&g_artherion_hut_known        , 0x258eu,     1u }, /* OVL032 */
    { (void *)&g_artherion_hut_visited      , 0x258fu,     1u }, /* OVL032 */
    { (void *)&g_artherion_done             , 0x2590u,     1u }, /* OVL032 */
    { (void *)g_unused_2591                 , 0x2591u,     1u }, /* OVL032 */
    { (void *)&g_ferry_landed               , 0x2592u,     1u }, /* OVL032 */
    { (void *)&g_rondra_priestess_met       , 0x2593u,     1u }, /* OVL032 */
    { (void *)&g_ferry_robbed               , 0x2594u,     1u }, /* OVL032 */
    { (void *)&g_tev000_warrior_cursing     , 0x2595u,     1u }, /* OVL032 */
    { (void *)&g_scene31_due                , 0x2596u,     1u }, /* OVL032 */
    { (void *)&g_tev111_seen                , 0x2597u,     1u }, /* OVL032 */
    { (void *)g_unused_2598                 , 0x2598u,     1u }, /* OVL032 */
    { (void *)&g_tev066_cub_killed          , 0x2599u,     1u }, /* OVL032 */
    { (void *)&g_finsterkopp_entered        , 0x259au,     1u }, /* OVL032 */
    { (void *)&g_low_tunnel_known           , 0x259bu,     1u }, /* OVL032 */
    { (void *)g_guest_lock                  , 0x259cu,     7u }, /* OVL032 */
    { (void *)g_guest_lock2                 , 0x25a3u,     3u }, /* OVL032 */
    { (void *)&g_topic_binge                , 0x25a6u,     1u }, /* OVL032 */
    { (void *)g_unused_25a7                 , 0x25a7u,     1u }, /* OVL032 */
    { (void *)&g_topic_salamander_stone     , 0x25a8u,     1u }, /* OVL032 */
    { (void *)&g_topic_ingramosch           , 0x25a9u,     1u }, /* OVL032 */
    { (void *)&g_topic_sternenschweif       , 0x25aau,     1u }, /* OVL032 */
    { (void *)&g_topic_siebenquell          , 0x25abu,     1u }, /* OVL032 */
    { (void *)&g_topic_egelbronn            , 0x25acu,     1u }, /* OVL032 */
    { (void *)g_unused_25ad                 , 0x25adu,     1u }, /* OVL032 */
    { (void *)&g_topic_artherion            , 0x25aeu,     1u }, /* OVL032 */
    { (void *)&g_kvirasim_arrived           , 0x25afu,     1u }, /* OVL032 */
    { (void *)&g_rose_active                , 0x25b0u,     1u }, /* OVL032 */
    { (void *)&g_rose_two_marks             , 0x25b1u,     1u }, /* OVL032 */
    { (void *)&g_binge_key_state            , 0x25b2u,     1u }, /* OVL032 */
    { (void *)g_unused_25b3                 , 0x25b3u,     1u }, /* OVL032 */
    { (void *)&g_low25_topic59              , 0x25b4u,     1u }, /* OVL032 */
    { (void *)&g_lore_told                  , 0x25b5u,     1u }, /* OVL032 */
    { (void *)&g_low34_ch_passed            , 0x25b6u,     1u }, /* OVL032 */
    { (void *)&g_low_kysira_ref             , 0x25b7u,     1u }, /* OVL032 */
    { (void *)&g_music_played               , 0x25b8u,     1u }, /* OVL032 */
    { (void *)&g_sites_shock_done           , 0x25b9u,     1u }, /* OVL032 */
    { (void *)&g_sites_hint_done            , 0x25bau,     1u }, /* OVL032 */
    { (void *)&g_sites_paid                 , 0x25bbu,     1u }, /* OVL032 */
    { (void *)&g_sites_evening              , 0x25bcu,     1u }, /* OVL032 */
    { (void *)&g_port_talk                  , 0x25bdu,     1u }, /* OVL032 */
    { (void *)&g_port_can_haggle            , 0x25beu,     1u }, /* OVL032 */
    { (void *)&g_port_paid                  , 0x25bfu,     1u }, /* OVL032 */
    { (void *)&g_port_poor                  , 0x25c0u,     1u }, /* OVL032 */
    { (void *)&g_port_deal                  , 0x25c1u,     1u }, /* OVL032 */
    { (void *)&g_ingramosch_talked          , 0x25c2u,     1u }, /* OVL032 */
    { (void *)&g_dream_seen                 , 0x25c3u,     1u }, /* OVL032 */
    { (void *)&g_low_beggars_scared         , 0x25c4u,     1u }, /* OVL032 */
    { (void *)&g_low_notices_read           , 0x25c5u,     1u }, /* OVL032 */
    { (void *)&g_eolan_state                , 0x25c6u,     1u }, /* OVL032 */
    { (void *)&g_low_deringorn_state        , 0x25c7u,     1u }, /* OVL032 */
    { (void *)&g_low_brooch_state           , 0x25c8u,     1u }, /* OVL032 */
    { (void *)&g_low_guard_talk             , 0x25c9u,     1u }, /* OVL032 */
    { (void *)&g_low_gate1_open             , 0x25cau,     1u }, /* OVL032 */
    { (void *)&g_low_gate2_open             , 0x25cbu,     1u }, /* OVL032 */
    { (void *)&g_low_dragan_task1           , 0x25ccu,     1u }, /* OVL032 */
    { (void *)&g_altrogolan_absent          , 0x25cdu,     1u }, /* OVL032 */
    { (void *)&g_low_ailian_state           , 0x25ceu,     1u }, /* OVL032 */
    { (void *)&g_vindaria_choice            , 0x25cfu,     1u }, /* OVL032 */
    { (void *)&g_vindaria_state             , 0x25d0u,     1u }, /* OVL032 */
    { (void *)&g_dragan_tip_jandora         , 0x25d1u,     1u }, /* OVL032 */
    { (void *)&g_dragan_tip_brin            , 0x25d2u,     1u }, /* OVL032 */
    { (void *)&g_dragan_tip_raisha          , 0x25d3u,     1u }, /* OVL032 */
    { (void *)&g_low_falkert_known          , 0x25d4u,     1u }, /* OVL032 */
    { (void *)&g_dragan_task                , 0x25d5u,     1u }, /* OVL032 */
    { (void *)&g_low_dragan_news            , 0x25d6u,     1u }, /* OVL032 */
    { (void *)&g_low_gavron_house           , 0x25d7u,     1u }, /* OVL032 */
    { (void *)&g_guide_won                  , 0x25d8u,     1u }, /* OVL032 */
    { (void *)&g_dng02_tools_state          , 0x25d9u,     1u }, /* OVL032 */
    { (void *)&g_dng02_jugs_state           , 0x25dau,     1u }, /* OVL032 */
    { (void *)&g_dng02_fishhook_taken       , 0x25dbu,     1u }, /* OVL032 */
    { (void *)&g_dng02_otter_fur_taken      , 0x25dcu,     1u }, /* OVL032 */
    { (void *)&g_dng02_dice_money_taken     , 0x25ddu,     1u }, /* OVL032 */
    { (void *)&g_dng02_recipe_taken         , 0x25deu,     1u }, /* OVL032 */
    { (void *)&g_dng02_secret_switch        , 0x25dfu,     1u }, /* OVL032 */
    { (void *)&g_dng02_keyhole_door         , 0x25e0u,     1u }, /* OVL032 */
    { (void *)&g_dng02_sun_lock_open        , 0x25e1u,     1u }, /* OVL032 */
    { (void *)&g_dng03_secret_door1         , 0x25e2u,     1u }, /* OVL032 */
    { (void *)&g_dng03_secret_door2         , 0x25e3u,     1u }, /* OVL032 */
    { (void *)&g_dng03_password_solved      , 0x25e4u,     1u }, /* OVL032 */
    { (void *)&g_dng03_voice_heard          , 0x25e5u,     1u }, /* OVL032 */
    { (void *)&g_dragon_claw_found          , 0x25e6u,     1u }, /* OVL032 */
    { (void *)&g_coward_stuck               , 0x25e7u,     1u }, /* OVL032 */
    { (void *)&g_dng03_combination_open     , 0x25e8u,     1u }, /* OVL032 */
    { (void *)&g_dng03_combination_tries    , 0x25e9u,     1u }, /* OVL032 */
    { (void *)&g_dng03_skeleton_warriors_171d, 0x25eau,     1u }, /* OVL032 */
    { (void *)&g_dng03_skeletons_191c       , 0x25ebu,     1u }, /* OVL032 */
    { (void *)&g_dng03_zombies_1519         , 0x25ecu,     1u }, /* OVL032 */
    { (void *)&g_dng03_skeleton_warriors_041d, 0x25edu,     1u }, /* OVL032 */
    { (void *)&g_dng03_zombies_1b15         , 0x25eeu,     1u }, /* OVL032 */
    { (void *)&g_dng03_skeleton_warriors_0b17, 0x25efu,     1u }, /* OVL032 */
    { (void *)&g_dng03_zombies_1313         , 0x25f0u,     1u }, /* OVL032 */
    { (void *)&g_dng03_heshthot_130e        , 0x25f1u,     1u }, /* OVL032 */
    { (void *)&g_dng03_sorcerer_080c        , 0x25f2u,     1u }, /* OVL032 */
    { (void *)&g_dng03_mages_0702           , 0x25f3u,     1u }, /* OVL032 */
    { (void *)&g_dng03_mages_0704           , 0x25f4u,     1u }, /* OVL032 */
    { (void *)&g_dng03_priests_0d01         , 0x25f5u,     1u }, /* OVL032 */
    { (void *)&g_dng03_knight_0e1d          , 0x25f6u,     1u }, /* OVL032 */
    { (void *)&g_dng03_desk_opened          , 0x25f7u,     1u }, /* OVL032 */
    { (void *)&g_tavern_dinner_asked        , 0x25f8u,     1u }, /* OVL032 */
    { (void *)&g_peddler_away               , 0x25f9u,     1u }, /* OVL032 */
    { (void *)&g_peddler3_away              , 0x25fau,     1u }, /* OVL032 */
    { (void *)&g_place197_done              , 0x25fbu,     1u }, /* OVL032 */
    { (void *)&g_dng04_path_281d            , 0x25fcu,     1u }, /* OVL032 */
    { (void *)&g_dng04_path_271c            , 0x25fdu,     1u }, /* OVL032 */
    { (void *)&g_dng04_rantzen_271b         , 0x25feu,     1u }, /* OVL032 */
    { (void *)&g_dng04_einbeeren_found      , 0x25ffu,     1u }, /* OVL032 */
    { (void *)&g_dng04_belmart_found        , 0x2600u,     1u }, /* OVL032 */
    { (void *)&g_dng04_snake_0c19           , 0x2601u,     1u }, /* OVL032 */
    { (void *)&g_dng04_snake_1817           , 0x2602u,     1u }, /* OVL032 */
    { (void *)&g_dng04_snake_211a           , 0x2603u,     1u }, /* OVL032 */
    { (void *)&g_dng04_snake_2419           , 0x2604u,     1u }, /* OVL032 */
    { (void *)&g_dng04_rantzen_1413         , 0x2605u,     1u }, /* OVL032 */
    { (void *)&g_dng04_rantzen_2701         , 0x2606u,     1u }, /* OVL032 */
    { (void *)&g_dng04_rantzen_left         , 0x2607u,     1u }, /* OVL032 */
    { (void *)&g_dng04_gulmond_found        , 0x2608u,     1u }, /* OVL032 */
    { (void *)&g_dng04_village_visited      , 0x2609u,     1u }, /* OVL032 */
    { (void *)g_unused_260a                 , 0x260au,     1u }, /* OVL032 */
    { (void *)&g_pic2_seen                  , 0x260bu,     1u }, /* OVL032 */
    { (void *)&g_price_level                , 0x260cu,     1u }, /* OVL032 */
    { (void *)&g_dng04_hedge_cut            , 0x260du,     1u }, /* OVL032 */
    { (void *)&g_dng04_rantzen_0405         , 0x260eu,     1u }, /* OVL032 */
    { (void *)&g_dng04_rantzen_0e16         , 0x260fu,     1u }, /* OVL032 */
    { (void *)&g_dng04_goblins_0109         , 0x2610u,     1u }, /* OVL032 */
    { (void *)&g_dng04_goblins_040d         , 0x2611u,     1u }, /* OVL032 */
    { (void *)&g_dng04_rantzen_1e08         , 0x2612u,     1u }, /* OVL032 */
    { (void *)&g_dng04_zombies_1219         , 0x2613u,     1u }, /* OVL032 */
    { (void *)&g_dng04_zombies_2d0e         , 0x2614u,     1u }, /* OVL032 */
    { (void *)&g_dng04_zombies_0919         , 0x2615u,     1u }, /* OVL032 */
    { (void *)&g_dng04_orcs_200c            , 0x2616u,     1u }, /* OVL032 */
    { (void *)&g_dng04_orcs_2510            , 0x2617u,     1u }, /* OVL032 */
    { (void *)&g_dng04_orcs_0413            , 0x2618u,     1u }, /* OVL032 */
    { (void *)&g_dng04_orcs_0601            , 0x2619u,     1u }, /* OVL032 */
    { (void *)&g_dng04_orcs_1402            , 0x261au,     1u }, /* OVL032 */
    { (void *)&g_dng04_orcs_1c02            , 0x261bu,     1u }, /* OVL032 */
    { (void *)&g_heather_recipe_read        , 0x261cu,     1u }, /* OVL032 */
    { (void *)&g_street_flag1               , 0x261du,     1u }, /* OVL032 */
    { (void *)&g_street_flag2               , 0x261eu,     1u }, /* OVL032 */
    { (void *)&g_street_guards              , 0x261fu,     1u }, /* OVL032 */
    { (void *)&g_street_thugs               , 0x2620u,     1u }, /* OVL032 */
    { (void *)&g_camp_juggler_met           , 0x2621u,     1u }, /* OVL032 */
    { (void *)&g_camp_druid_met             , 0x2622u,     1u }, /* OVL032 */
    { (void *)&g_camp_hunter_met            , 0x2623u,     1u }, /* OVL032 */
    { (void *)&g_camp_vagabond_met          , 0x2624u,     1u }, /* OVL032 */
    { (void *)&g_tavgas_visits              , 0x2625u,     1u }, /* OVL032 */
    { (void *)&g_inn_vision_state           , 0x2626u,     1u }, /* OVL032 */
    { (void *)&g_swamp_chief_refused        , 0x2627u,     1u }, /* OVL032 */
    { (void *)&g_swamp_chief_offended       , 0x2628u,     1u }, /* OVL032 */
    { (void *)&g_dng04_monster_killed       , 0x2629u,     1u }, /* OVL032 */
    { (void *)&g_dng04_tower_open           , 0x262au,     1u }, /* OVL032 */
    { (void *)&g_sites_challenges           , 0x262bu,     1u }, /* OVL032 */
    { (void *)&g_sites_rematch              , 0x262cu,     1u }, /* OVL032 */
    { (void *)&g_guide_avail                , 0x262du,     1u }, /* OVL032 */
    { (void *)&g_finster_anger              , 0x262eu,     1u }, /* OVL032 */
    { (void *)&g_game_finished2             , 0x262fu,     1u }, /* OVL032 */
    { (void *)&g_diary_new_items            , 0x2630u,     1u }, /* OVL032 */
    { (void *)&g_low_stone_delivered        , 0x2631u,     1u }, /* OVL032 */
    { (void *)&g_low_stone_days             , 0x2632u,     1u }, /* OVL032 */
    { (void *)&g_voyage_done                , 0x2633u,     1u }, /* OVL032 */
    { (void *)&g_elsurion_met               , 0x2634u,     1u }, /* OVL032 */
    { (void *)&g_dng13_smoke_timer          , 0x2635u,     1u }, /* OVL032 */
    { (void *)&g_dng10_countdown            , 0x2636u,     1u }, /* OVL032 */
    { (void *)&g_dng11_floor_collapsed      , 0x2637u,     1u }, /* OVL032 */
    { (void *)&g_binge_hole1_looted         , 0x2638u,     1u }, /* OVL032 */
    { (void *)&g_binge_hole2_trapped        , 0x2639u,     1u }, /* OVL032 */
    { (void *)&g_dng10_tools_found          , 0x263au,     1u }, /* OVL032 */
    { (void *)&g_dng10_parchment_taken      , 0x263bu,     1u }, /* OVL032 */
    { (void *)&g_binge_torches_lit          , 0x263cu,     1u }, /* OVL032 */
    { (void *)&g_binge_shaft_stone_ap       , 0x263du,     1u }, /* OVL032 */
    { (void *)&g_dng10_flask_taken          , 0x263eu,     1u }, /* OVL032 */
    { (void *)&g_dng12_pit_side             , 0x263fu,     1u }, /* OVL032 */
    { (void *)&g_port_haggled               , 0x2640u,     1u }, /* OVL032 */
    { (void *)&g_port_price                 , 0x2641u,     1u }, /* OVL032 */
    { (void *)&g_dng12_cart_seen            , 0x2642u,     1u }, /* OVL032 */
    { (void *)&g_dng12_lever                , 0x2643u,     1u }, /* OVL032 */
    { (void *)&g_dng10_ring_taken           , 0x2644u,     1u }, /* OVL032 */
    { (void *)&g_sacrilege                  , 0x2645u,     1u }, /* OVL032 */
    { (void *)&g_low_burglary_result        , 0x2646u,     1u }, /* OVL032 */
    { (void *)&g_dng05_supplies_taken       , 0x2647u,     1u }, /* OVL032 */
    { (void *)&g_dng05_recipe_taken         , 0x2648u,     1u }, /* OVL032 */
    { (void *)g_unused_2649                 , 0x2649u,     1u }, /* OVL032 */
    { (void *)&g_dng05_pit_seen             , 0x264au,     1u }, /* OVL032 */
    { (void *)&g_dng05_switch_0114          , 0x264bu,     1u }, /* OVL032 */
    { (void *)&g_sites_vision_ok            , 0x264cu,     1u }, /* OVL032 */
    { (void *)&g_sites_vision_failed        , 0x264du,     1u }, /* OVL032 */
    { (void *)&g_sites_night_met            , 0x264eu,     1u }, /* OVL032 */
    { (void *)&g_trnd_once1_done            , 0x264fu,     1u }, /* OVL032 */
    { (void *)&g_trnd_once2_done            , 0x2650u,     1u }, /* OVL032 */
    { (void *)&g_trnd_twice_state           , 0x2651u,     1u }, /* OVL032 */
    { (void *)&g_dng05_finale_seen          , 0x2652u,     1u }, /* OVL032 */
    { (void *)&g_dng05_bolt_trap_armed      , 0x2653u,     1u }, /* OVL032 */
    { (void *)&g_dng05_offering_made        , 0x2654u,     1u }, /* OVL032 */
    { (void *)&g_dng05_skeletons_beaten     , 0x2655u,     1u }, /* OVL032 */
    { (void *)&g_dng05_scroll_taken         , 0x2656u,     1u }, /* OVL032 */
    { (void *)&g_dng05_riddle_solved        , 0x2657u,     1u }, /* OVL032 */
    { (void *)&g_dng05_puzzle_won           , 0x2658u,     1u }, /* OVL032 */
    { (void *)&g_dng05_door_070d_found      , 0x2659u,     1u }, /* OVL032 */
    { (void *)&g_dng05_door_0402_found      , 0x265au,     1u }, /* OVL032 */
    { (void *)&g_dng05_door_1106_found      , 0x265bu,     1u }, /* OVL032 */
    { (void *)&g_dng06_noise                , 0x265cu,     1u }, /* OVL032 */
    { (void *)&g_dng06_alarm                , 0x265du,     1u }, /* OVL032 */
    { (void *)&g_dng07_noise                , 0x265eu,     1u }, /* OVL032 */
    { (void *)&g_dng07_alarm                , 0x265fu,     1u }, /* OVL032 */
    { (void *)&g_dng08_noise                , 0x2660u,     1u }, /* OVL032 */
    { (void *)&g_dng08_alarm                , 0x2661u,     1u }, /* OVL032 */
    { (void *)g_unused_2662                 , 0x2662u,     2u }, /* OVL032 */
    { (void *)&g_dng06_rack_done            , 0x2664u,     1u }, /* OVL032 */
    { (void *)&g_dng06_orcs_1b04            , 0x2665u,     1u }, /* OVL032 */
    { (void *)&g_dng06_helmet_taken         , 0x2666u,     1u }, /* OVL032 */
    { (void *)&g_dng06_statue_looted        , 0x2667u,     1u }, /* OVL032 */
    { (void *)&g_dng06_statue_destroyed     , 0x2668u,     1u }, /* OVL032 */
    { (void *)&g_dng06_statue_ignored       , 0x2669u,     1u }, /* OVL032 */
    { (void *)&g_dng06_hornets_destroyed    , 0x266au,     1u }, /* OVL032 */
    { (void *)&g_dng06_pit_1305             , 0x266bu,     1u }, /* OVL032 */
    { (void *)&g_dng06_pit_1519             , 0x266cu,     1u }, /* OVL032 */
    { (void *)&g_dng06_rockfall1            , 0x266du,     1u }, /* OVL032 */
    { (void *)&g_dng06_rockfall2            , 0x266eu,     1u }, /* OVL032 */
    { (void *)&g_dng06_skeleton_dug         , 0x266fu,     1u }, /* OVL032 */
    { (void *)&g_dng06_beer_drunk           , 0x2670u,     1u }, /* OVL032 */
    { (void *)&g_dng06_dagger_taken         , 0x2671u,     1u }, /* OVL032 */
    { (void *)&g_dng06_dogs_state           , 0x2672u,     1u }, /* OVL032 */
    { (void *)&g_dng06_tripwire_seen        , 0x2673u,     1u }, /* OVL032 */
    { (void *)&g_dng06_tripwire_hit         , 0x2674u,     1u }, /* OVL032 */
    { (void *)&g_dng06_hay_state            , 0x2675u,     1u }, /* OVL032 */
    { (void *)&g_dng06_latrine_seen         , 0x2676u,     1u }, /* OVL032 */
    { (void *)&g_dng06_hall_seen            , 0x2677u,     1u }, /* OVL032 */
    { (void *)&g_dng06_beer_drained         , 0x2678u,     1u }, /* OVL032 */
    { (void *)&g_dng06_ogre_beer_tasted     , 0x2679u,     1u }, /* OVL032 */
    { (void *)&g_dng06_stew_eaten           , 0x267au,     1u }, /* OVL032 */
    { (void *)&g_dng06_orcs_0e10            , 0x267bu,     1u }, /* OVL032 */
    { (void *)&g_dng06_bolt_wall_armed      , 0x267cu,     1u }, /* OVL032 */
    { (void *)&g_dng06_beetle_pit_armed     , 0x267du,     1u }, /* OVL032 */
    { (void *)&g_dng06_beetle_pit_fallen    , 0x267eu,     1u }, /* OVL032 */
    { (void *)&g_dng07_dogs_door_2c         , 0x267fu,     1u }, /* OVL032 */
    { (void *)&g_dng07_dogs_door_2e         , 0x2680u,     1u }, /* OVL032 */
    { (void *)&g_dng07_kennel1_searched     , 0x2681u,     1u }, /* OVL032 */
    { (void *)&g_dng07_kennel2_searched     , 0x2682u,     1u }, /* OVL032 */
    { (void *)&g_dng07_guards_beaten        , 0x2683u,     1u }, /* OVL032 */
    { (void *)&g_dng07_shelf_searched       , 0x2684u,     1u }, /* OVL032 */
    { (void *)&g_dng07_shelf_hole_found     , 0x2685u,     1u }, /* OVL032 */
    { (void *)&g_dng07_orcs_1606            , 0x2686u,     1u }, /* OVL032 */
    { (void *)&g_dng07_orcs_0d08            , 0x2687u,     1u }, /* OVL032 */
    { (void *)&g_dng07_orcs_100b            , 0x2688u,     1u }, /* OVL032 */
    { (void *)&g_dng07_orcs_140b            , 0x2689u,     1u }, /* OVL032 */
    { (void *)&g_dng07_orcs_120f            , 0x268au,     1u }, /* OVL032 */
    { (void *)&g_dng07_store_seen           , 0x268bu,     1u }, /* OVL032 */
    { (void *)&g_dng07_statue_scraped       , 0x268cu,     1u }, /* OVL032 */
    { (void *)&g_dng07_rockfall1            , 0x268du,     1u }, /* OVL032 */
    { (void *)&g_dng07_rockfall2            , 0x268eu,     1u }, /* OVL032 */
    { (void *)&g_dng07_pit1_fallen          , 0x268fu,     1u }, /* OVL032 */
    { (void *)&g_dng07_maggots_done         , 0x2690u,     1u }, /* OVL032 */
    { (void *)&g_dng07_cave_seen            , 0x2691u,     1u }, /* OVL032 */
    { (void *)&g_dng07_junk_finds           , 0x2692u,     1u }, /* OVL032 */
    { (void *)&g_dng07_beetles_1b15         , 0x2693u,     1u }, /* OVL032 */
    { (void *)&g_dng07_beetles_1d17         , 0x2694u,     1u }, /* OVL032 */
    { (void *)&g_dng07_beetles_181c         , 0x2695u,     1u }, /* OVL032 */
    { (void *)&g_dng07_feed_room_seen       , 0x2696u,     1u }, /* OVL032 */
    { (void *)&g_dng07_orcs_0a0d            , 0x2697u,     1u }, /* OVL032 */
    { (void *)&g_dng07_latrine_seen         , 0x2698u,     1u }, /* OVL032 */
    { (void *)&g_dng07_orcs_050f            , 0x2699u,     1u }, /* OVL032 */
    { (void *)&g_dng07_orcs_0305            , 0x269au,     1u }, /* OVL032 */
    { (void *)&g_dng07_pit2_fallen          , 0x269bu,     1u }, /* OVL032 */
    { (void *)&g_dng07_pit3_fallen          , 0x269cu,     1u }, /* OVL032 */
    { (void *)&g_dng07_orcs_0319            , 0x269du,     1u }, /* OVL032 */
    { (void *)&g_dng07_stew_eaten           , 0x269eu,     1u }, /* OVL032 */
    { (void *)&g_dng07_kitchen_looted       , 0x269fu,     1u }, /* OVL032 */
    { (void *)&g_dng07_straw_amulet         , 0x26a0u,     1u }, /* OVL032 */
    { (void *)&g_dng07_corpse_sword_taken   , 0x26a1u,     1u }, /* OVL032 */
    { (void *)&g_dng07_web_131a             , 0x26a2u,     1u }, /* OVL032 */
    { (void *)&g_dng07_web_1321             , 0x26a3u,     1u }, /* OVL032 */
    { (void *)&g_dng07_spiders_1321         , 0x26a4u,     1u }, /* OVL032 */
    { (void *)&g_dng07_web_1022             , 0x26a5u,     1u }, /* OVL032 */
    { (void *)&g_dng07_spiders_1022         , 0x26a6u,     1u }, /* OVL032 */
    { (void *)&g_dng07_web_1024             , 0x26a7u,     1u }, /* OVL032 */
    { (void *)&g_dng07_spiders_0e25         , 0x26a8u,     1u }, /* OVL032 */
    { (void *)&g_dng07_web_0e25             , 0x26a9u,     1u }, /* OVL032 */
    { (void *)&g_dng07_web_0922             , 0x26aau,     1u }, /* OVL032 */
    { (void *)&g_dng07_spiders_0922         , 0x26abu,     1u }, /* OVL032 */
    { (void *)&g_dng07_web_0720             , 0x26acu,     1u }, /* OVL032 */
    { (void *)&g_dng07_web_0320             , 0x26adu,     1u }, /* OVL032 */
    { (void *)&g_dng07_spiders_0320         , 0x26aeu,     1u }, /* OVL032 */
    { (void *)&g_dng07_web_0927             , 0x26afu,     1u }, /* OVL032 */
    { (void *)&g_dng07_spiders_0927         , 0x26b0u,     1u }, /* OVL032 */
    { (void *)&g_dng07_web_032b             , 0x26b1u,     1u }, /* OVL032 */
    { (void *)&g_dng07_spiders_032b         , 0x26b2u,     1u }, /* OVL032 */
    { (void *)&g_dng07_nest_destroyed       , 0x26b3u,     1u }, /* OVL032 */
    { (void *)&g_dng07_rockfall3            , 0x26b4u,     1u }, /* OVL032 */
    { (void *)&g_dng07_rockfall4            , 0x26b5u,     1u }, /* OVL032 */
    { (void *)&g_dng07_niche_looted         , 0x26b6u,     1u }, /* OVL032 */
    { (void *)&g_dng07_amulet_found         , 0x26b7u,     1u }, /* OVL032 */
    { (void *)&g_dng08_smithy_seen          , 0x26b8u,     1u }, /* OVL032 */
    { (void *)&g_dng08_storeroom_shown      , 0x26b9u,     1u }, /* OVL032 */
    { (void *)&g_dng08_banner_torn          , 0x26bau,     1u }, /* OVL032 */
    { (void *)&g_dng08_hatchet_taken        , 0x26bbu,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_0d04            , 0x26bcu,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_0e06            , 0x26bdu,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_0a06            , 0x26beu,     1u }, /* OVL032 */
    { (void *)&g_dng08_ogres_1603           , 0x26bfu,     1u }, /* OVL032 */
    { (void *)g_unused_26c0                 , 0x26c0u,     1u }, /* OVL032 */
    { (void *)&g_low_vindaria_known         , 0x26c1u,     1u }, /* OVL032 */
    { (void *)g_unused_26c2                 , 0x26c2u,     1u }, /* OVL032 */
    { (void *)&g_binge_hole3_looted         , 0x26c3u,     1u }, /* OVL032 */
    { (void *)&g_dng10_junk_found           , 0x26c4u,     1u }, /* OVL032 */
    { (void *)&g_dng16_plate_found          , 0x26c5u,     1u }, /* OVL032 */
    { (void *)&g_dng16_plate_lifted         , 0x26c6u,     1u }, /* OVL032 */
    { (void *)&g_dng16_pit_trap             , 0x26c7u,     1u }, /* OVL032 */
    { (void *)&g_dng16_casket_taken         , 0x26c8u,     1u }, /* OVL032 */
    { (void *)&g_dng16_double_key_state     , 0x26c9u,     1u }, /* OVL032 */
    { (void *)&g_dng10_vision_ap            , 0x26cau,     1u }, /* OVL032 */
    { (void *)&g_dng11_lever_on             , 0x26cbu,     1u }, /* OVL032 */
    { (void *)&g_dng11_plate_down           , 0x26ccu,     1u }, /* OVL032 */
    { (void *)&g_dng11_trap1_found          , 0x26cdu,     1u }, /* OVL032 */
    { (void *)&g_dng11_trap2_found          , 0x26ceu,     1u }, /* OVL032 */
    { (void *)&g_dng11_block_down           , 0x26cfu,     1u }, /* OVL032 */
    { (void *)g_unused_26d0                 , 0x26d0u,     1u }, /* OVL032 */
    { (void *)&g_dng11_winch_state          , 0x26d1u,     1u }, /* OVL032 */
    { (void *)&g_dng13_wheel_turned         , 0x26d2u,     1u }, /* OVL032 */
    { (void *)&g_dng14_plate_tried          , 0x26d3u,     1u }, /* OVL032 */
    { (void *)&g_dng11_plate_seen           , 0x26d4u,     1u }, /* OVL032 */
    { (void *)&g_dng11_trap3_found          , 0x26d5u,     1u }, /* OVL032 */
    { (void *)&g_dng14_plate_tries          , 0x26d6u,     1u }, /* OVL032 */
    { (void *)&g_dng11_trap2_fired          , 0x26d7u,     1u }, /* OVL032 */
    { (void *)&g_dng11_trap3_fired          , 0x26d8u,     1u }, /* OVL032 */
    { (void *)&g_place083_attacked          , 0x26d9u,     1u }, /* OVL032 */
    { (void *)&g_binge_anvil_hours          , 0x26dau,     1u }, /* OVL032 */
    { (void *)g_unused_26db                 , 0x26dbu,     1u }, /* OVL032 */
    { (void *)&g_dng11_wardrobe_seen        , 0x26dcu,     1u }, /* OVL032 */
    { (void *)&g_dng14_waterskin_found      , 0x26ddu,     1u }, /* OVL032 */
    { (void *)&g_dng14_plate_lifted         , 0x26deu,     1u }, /* OVL032 */
    { (void *)&g_dng11_kobold_met           , 0x26dfu,     1u }, /* OVL032 */
    { (void *)&g_dng11_riddle_solved        , 0x26e0u,     1u }, /* OVL032 */
    { (void *)&g_mine_cart_pos              , 0x26e1u,     1u }, /* OVL032 */
    { (void *)&g_dng16_plates_dir           , 0x26e2u,     1u }, /* OVL032 */
    { (void *)&g_dng16_grid_moved           , 0x26e3u,     1u }, /* OVL032 */
    { (void *)&g_dng16_hero_wet             , 0x26e4u,     1u }, /* OVL032 */
    { (void *)&g_dng12_beams_done           , 0x26e5u,     1u }, /* OVL032 */
    { (void *)&g_dng12_beams_seen           , 0x26e6u,     1u }, /* OVL032 */
    { (void *)&g_dng12_debris_ap            , 0x26e7u,     1u }, /* OVL032 */
    { (void *)&g_ingerimm_verse_read        , 0x26e8u,     1u }, /* OVL032 */
    { (void *)&g_tev_lowangen_key_noted     , 0x26e9u,     1u }, /* OVL032 */
    { (void *)&g_ambush_count               , 0x26eau,     1u }, /* OVL032 */
    { (void *)&g_dng10_chest_opened         , 0x26ebu,     1u }, /* OVL032 */
    { (void *)&g_dng12_skeleton_found       , 0x26ecu,     1u }, /* OVL032 */
    { (void *)&g_parchments_read            , 0x26edu,     1u }, /* OVL032 */
    { (void *)&g_topic_gavron               , 0x26eeu,     1u }, /* OVL032 */
    { (void *)&g_dng16_chest_looted         , 0x26efu,     1u }, /* OVL032 */
    { (void *)&g_dng04_village_thanked      , 0x26f0u,     1u }, /* OVL032 */
    { (void *)&g_dng10_silver_key_in_lock   , 0x26f1u,     1u }, /* OVL032 */
    { (void *)&g_dng16_sword_done           , 0x26f2u,     1u }, /* OVL032 */
    { (void *)&g_finale_done                , 0x26f3u,     1u }, /* OVL032 */
    { (void *)&g_dng10_gold_key_in_lock     , 0x26f4u,     1u }, /* OVL032 */
    { (void *)&g_quest_anim_seen            , 0x26f5u,     1u }, /* OVL032 */
    { (void *)&g_guest_lock_t2              , 0x26f6u,     1u }, /* OVL032 */
    { (void *)&g_guest_lock_t19             , 0x26f7u,     1u }, /* OVL032 */
    { (void *)&g_guest_lock_t23             , 0x26f8u,     1u }, /* OVL032 */
    { (void *)&g_guest_lock_t24             , 0x26f9u,     1u }, /* OVL032 */
    { (void *)&g_dng14_in_pool              , 0x26fau,     1u }, /* OVL032 */
    { (void *)&g_dng12_stash28_empty        , 0x26fbu,     1u }, /* OVL032 */
    { (void *)&g_dng11_ramp_falls           , 0x26fcu,     1u }, /* OVL032 */
    { (void *)&g_dng13_ambush_won           , 0x26fdu,     1u }, /* OVL032 */
    { (void *)&g_sites_night_done           , 0x26feu,     1u }, /* OVL032 */
    { (void *)g_unused_26ff                 , 0x26ffu,     1u }, /* OVL032 */
    { (void *)&g_dng16_plate_steps          , 0x2700u,     1u }, /* OVL032 */
    { (void *)&g_dng16_sign_read            , 0x2701u,     1u }, /* OVL032 */
    { (void *)&g_dng16_sacrilege            , 0x2702u,     1u }, /* OVL032 */
    { (void *)&g_dng11_ramp_armed           , 0x2703u,     1u }, /* OVL032 */
    { (void *)&g_dng16_stone_taken          , 0x2704u,     1u }, /* OVL032 */
    { (void *)&g_dng16_forging              , 0x2705u,     1u }, /* OVL032 */
    { (void *)&g_dng16_golem_fought         , 0x2706u,     1u }, /* OVL032 */
    { (void *)&g_dng16_medallion_given      , 0x2707u,     1u }, /* OVL032 */
    { (void *)&g_dng16_forge_timer          , 0x2708u,     1u }, /* OVL032 */
    { (void *)&g_dng16_wall_timer           , 0x2709u,     1u }, /* OVL032 */
    { (void *)&g_dng16_artifact_traded      , 0x270au,     1u }, /* OVL032 */
    { (void *)&g_dng16_leather_taken        , 0x270bu,     1u }, /* OVL032 */
    { (void *)&g_sites_explored             , 0x270cu,     1u }, /* OVL032 */
    { (void *)g_unused_270d                 , 0x270du,     2u }, /* OVL032 */
    { (void *)&g_low_arrival_due            , 0x270fu,     1u }, /* OVL032 */
    { (void *)&g_dng08_shaman_room_shown    , 0x2710u,     1u }, /* OVL032 */
    { (void *)&g_dng08_mummy_fought         , 0x2711u,     1u }, /* OVL032 */
    { (void *)&g_dng08_sarcophagus_opened   , 0x2712u,     1u }, /* OVL032 */
    { (void *)&g_dng08_whirlweed_found      , 0x2713u,     1u }, /* OVL032 */
    { (void *)&g_dng08_hall_0c15            , 0x2714u,     1u }, /* OVL032 */
    { (void *)&g_dng08_pit_0e15             , 0x2715u,     1u }, /* OVL032 */
    { (void *)&g_dng08_pit_armed            , 0x2716u,     1u }, /* OVL032 */
    { (void *)&g_dng08_collar_131c          , 0x2717u,     1u }, /* OVL032 */
    { (void *)&g_dng08_collar_0d10          , 0x2718u,     1u }, /* OVL032 */
    { (void *)&g_dng08_orc_veteran_1907     , 0x2719u,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_1e19            , 0x271au,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_1e15            , 0x271bu,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_1a11            , 0x271cu,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_1516            , 0x271du,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_121b            , 0x271eu,     1u }, /* OVL032 */
    { (void *)&g_dng08_ogres_0e19           , 0x271fu,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_1113            , 0x2720u,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_1110            , 0x2721u,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_0d0f            , 0x2722u,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_0f0c            , 0x2723u,     1u }, /* OVL032 */
    { (void *)&g_dng08_grain_1e0e           , 0x2724u,     1u }, /* OVL032 */
    { (void *)&g_dng08_grain_1e0f           , 0x2725u,     1u }, /* OVL032 */
    { (void *)&g_dng08_grain_1e10           , 0x2726u,     1u }, /* OVL032 */
    { (void *)&g_dng08_grain_1e0d           , 0x2727u,     1u }, /* OVL032 */
    { (void *)&g_dng13_stash29_empty        , 0x2728u,     1u }, /* OVL032 */
    { (void *)&g_dng08_goulash_eaten        , 0x2729u,     1u }, /* OVL032 */
    { (void *)&g_dng08_straw_0612           , 0x272au,     1u }, /* OVL032 */
    { (void *)&g_dng08_hall_0415            , 0x272bu,     1u }, /* OVL032 */
    { (void *)&g_dng08_trap_0213_defused    , 0x272cu,     1u }, /* OVL032 */
    { (void *)&g_dng08_lever                , 0x272du,     1u }, /* OVL032 */
    { (void *)&g_dng08_hall_fight           , 0x272eu,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_0511            , 0x272fu,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_020d            , 0x2730u,     1u }, /* OVL032 */
    { (void *)&g_dng08_orcs_0309            , 0x2731u,     1u }, /* OVL032 */
    { (void *)&g_dng02_druids_1c05          , 0x2732u,     1u }, /* OVL032 */
    { (void *)&g_dng02_cultists_0802        , 0x2733u,     1u }, /* OVL032 */
    { (void *)&g_dng02_cultists_0402        , 0x2734u,     1u }, /* OVL032 */
    { (void *)&g_dng02_cultists_020b        , 0x2735u,     1u }, /* OVL032 */
    { (void *)&g_dng02_cultists_070d        , 0x2736u,     1u }, /* OVL032 */
    { (void *)&g_dng02_cultists_041c        , 0x2737u,     1u }, /* OVL032 */
    { (void *)&g_dng02_sabretooth_0b17      , 0x2738u,     1u }, /* OVL032 */
    { (void *)&g_dng02_sabretooth_0f0c      , 0x2739u,     1u }, /* OVL032 */
    { (void *)&g_dng02_sabretooth_0f1c      , 0x273au,     1u }, /* OVL032 */
    { (void *)&g_dng02_cave_spiders_150c    , 0x273bu,     1u }, /* OVL032 */
    { (void *)&g_dng02_amulet1_timer        , 0x273cu,     1u }, /* OVL032 */
    { (void *)&g_dng02_amulet2_timer        , 0x273du,     1u }, /* OVL032 */
    { (void *)&g_dng02_amulet3_timer        , 0x273eu,     1u }, /* OVL032 */
    { (void *)&g_dng02_amulet4_timer        , 0x273fu,     1u }, /* OVL032 */
    { (void *)&g_dng02_cultists_171d        , 0x2740u,     1u }, /* OVL032 */
    { (void *)&g_dng02_cultists_1c1b        , 0x2741u,     1u }, /* OVL032 */
    { (void *)&g_dng02_step_toggle          , 0x2742u,     1u }, /* OVL032 */
    { (void *)&g_jail_freed                 , 0x2743u,     1u }, /* OVL032 */
    { (void *)&g_mugging_state              , 0x2744u,     1u }, /* OVL032 */
    { (void *)&g_gashok_arrested            , 0x2745u,     1u }, /* OVL032 */
    { (void *)&g_blackmage_done             , 0x2746u,     1u }, /* OVL032 */
    { (void *)&g_pending_msg                , 0x2747u,     1u }, /* OVL032 */
    { (void *)&g_dng05_mummies_0e1d         , 0x2748u,     1u }, /* OVL032 */
    { (void *)&g_dng05_mummies_091f         , 0x2749u,     1u }, /* OVL032 */
    { (void *)&g_dng05_mummies_021e         , 0x274au,     1u }, /* OVL032 */
    { (void *)g_unused_274b                 , 0x274bu,     1u }, /* OVL032 */
    { (void *)&g_dng05_priests_0819         , 0x274cu,     1u }, /* OVL032 */
    { (void *)&g_dng05_priests_0a0e         , 0x274du,     1u }, /* OVL032 */
    { (void *)&g_dng05_priests_0a0a         , 0x274eu,     1u }, /* OVL032 */
    { (void *)g_unused_274f                 , 0x274fu,     2u }, /* OVL032 */
    { (void *)&g_dng05_skeletons_020e       , 0x2751u,     1u }, /* OVL032 */
    { (void *)g_unused_2752                 , 0x2752u,     1u }, /* OVL032 */
    { (void *)&g_dng05_priests_1c0d         , 0x2753u,     1u }, /* OVL032 */
    { (void *)&g_dng05_priests_1501         , 0x2754u,     1u }, /* OVL032 */
    { (void *)&g_sites_reward               , 0x2755u,     1u }, /* OVL032 */
    { (void *)&g_guide_count                , 0x2756u,     1u }, /* OVL032 */
    { (void *)&g_npc8_days                  , 0x2757u,     1u }, /* OVL032 */
    { (void *)&g_gavron_chase               , 0x2758u,     1u }, /* OVL032 */
    { (void *)g_unused_2759                 , 0x2759u,     1u }, /* OVL032 */
    { (void *)&g_low_bridge_y               , 0x275au,     1u }, /* OVL032 */
    { (void *)&g_dng04_mage_killed          , 0x275bu,     1u }, /* OVL032 */
    { (void *)&g_dng07_prison_event         , 0x275cu,     1u }, /* OVL032 */
    { (void *)&g_dng07_prison_days          , 0x275du,     1u }, /* OVL032 */
    { (void *)&g_dng07_talk1_done           , 0x275eu,     1u }, /* OVL032 */
    { (void *)&g_dng07_talk2_done           , 0x275fu,     1u }, /* OVL032 */
    { (void *)&g_low_dragan_task3           , 0x2760u,     1u }, /* OVL032 */
    { (void *)&g_dng04_monster_fought       , 0x2761u,     1u }, /* OVL032 */
    { (void *)&g_finster_refuse_msg         , 0x2762u,     1u }, /* OVL032 */
    { (void *)&g_dng01_grate_first          , 0x2763u,     1u }, /* OVL032 */
    { (void *)&g_dng01_hooks_held           , 0x2764u,     1u }, /* OVL032 */
    { (void *)&g_dng01_skull_state          , 0x2765u,     1u }, /* OVL032 */
    { (void *)&g_tjolmar_vault              , 0x2766u,     1u }, /* OVL032 */
    { (void *)&g_dng01_thorn_tree_state     , 0x2767u,     1u }, /* OVL032 */
    { (void *)&g_dng01_branch_state         , 0x2768u,     1u }, /* OVL032 */
    { (void *)&g_dng01_cultists_0806        , 0x2769u,     1u }, /* OVL032 */
    { (void *)&g_dng01_cultists_0e06        , 0x276au,     1u }, /* OVL032 */
    { (void *)&g_dng01_troll_fought         , 0x276bu,     1u }, /* OVL032 */
    { (void *)&g_dng01_ice_state            , 0x276cu,     1u }, /* OVL032 */
    { (void *)&g_dng01_hesinde_done         , 0x276du,     1u }, /* OVL032 */
    { (void *)&g_dng01_ghouls_fought        , 0x276eu,     1u }, /* OVL032 */
    { (void *)&g_dng01_illusion_wall2       , 0x276fu,     1u }, /* OVL032 */
    { (void *)&g_dng01_illusion_wall1       , 0x2770u,     1u }, /* OVL032 */
    { (void *)&g_dng01_cave_spiders_1808    , 0x2771u,     1u }, /* OVL032 */
    { (void *)g_unused_2772                 , 0x2772u,     1u }, /* OVL032 */
    { (void *)&g_phex_finale_done           , 0x2773u,     1u }, /* OVL032 */
    { (void *)&g_dng05_poltergeist          , 0x2774u,     1u }, /* OVL032 */
    { (void *)&g_dng05_skeletons_1214       , 0x2775u,     1u }, /* OVL032 */
    { (void *)&g_dng05_skeletons_1a1d       , 0x2776u,     1u }, /* OVL032 */
    { (void *)&g_dng05_skeletons_1b1e       , 0x2777u,     1u }, /* OVL032 */
    { (void *)&g_dng05_skeletons_1c1d       , 0x2778u,     1u }, /* OVL032 */
    { (void *)&g_dng05_skeletons_1b1c       , 0x2779u,     1u }, /* OVL032 */
    { (void *)&g_dng05_wall_1a1b_open       , 0x277au,     1u }, /* OVL032 */
    { (void *)&g_lock_trap_off              , 0x277bu,     1u }, /* OVL032 */
    { (void *)&g_dng08_copper_disc_taken    , 0x277cu,     1u }, /* OVL032 */
    { (void *)&g_dng06_orc_veteran_1d02     , 0x277du,     1u }, /* OVL032 */
    { (void *)&g_dng06_orcs_1503            , 0x277eu,     1u }, /* OVL032 */
    { (void *)&g_dng06_orcs_0f03            , 0x277fu,     1u }, /* OVL032 */
    { (void *)&g_dng06_orcs_0402            , 0x2780u,     1u }, /* OVL032 */
    { (void *)&g_dng06_orcs_1009            , 0x2781u,     1u }, /* OVL032 */
    { (void *)&g_dng07_brandy_taken         , 0x2782u,     1u }, /* OVL032 */
    { (void *)&g_dng08_latrine_armed        , 0x2783u,     1u }, /* OVL032 */
    { (void *)&g_dng03_thief_pending        , 0x2784u,     1u }, /* OVL032 */
    { (void *)&g_dng03_sorcerer_0e0c        , 0x2785u,     1u }, /* OVL032 */
    { (void *)&g_dng03_stone_stolen         , 0x2786u,     1u }, /* OVL032 */
    { (void *)&g_final_fight_won            , 0x2787u,     1u }, /* OVL032 */
    { (void *)&g_dng01_cave_spiders_0101    , 0x2788u,     1u }, /* OVL032 */
    { (void *)&g_dng06_straw_1502           , 0x2789u,     1u }, /* OVL032 */
    { (void *)&g_dng06_straw_0d02           , 0x278au,     1u }, /* OVL032 */
    { (void *)&g_dng06_straw_100b           , 0x278bu,     1u }, /* OVL032 */
    { (void *)&g_dng06_straw_0a08           , 0x278cu,     1u }, /* OVL032 */
    { (void *)&g_dng06_straw_0b08           , 0x278du,     1u }, /* OVL032 */
    { (void *)&g_dng06_straw_0f0a           , 0x278eu,     1u }, /* OVL032 */
    { (void *)&g_sfight_seen                , 0x278fu,     1u }, /* OVL032 */
    { (void *)&g_dng06_ogre_weapons_smashed , 0x2790u,     1u }, /* OVL032 */
    { (void *)&g_tev082_thicket_done        , 0x2791u,     1u }, /* OVL032 */
    { (void *)&g_dng07_was_prisoner         , 0x2792u,     1u }, /* OVL032 */
    { (void *)&g_sfight_won                 , 0x2793u,     1u }, /* OVL032 */
    { (void *)&g_low_tunnel_arrival         , 0x2794u,     1u }, /* OVL032 */
    { (void *)&g_travel_state               , 0x2795u,     1u }, /* OVL032 */
    { (void *)&g_low_brooch_taken           , 0x2796u,     1u }, /* OVL032 */
    { (void *)&g_salamander_stolen          , 0x2797u,     1u }, /* OVL032 */
    { (void *)&g_dng11_air_counter          , 0x2798u,     1u }, /* OVL032 */
    { (void *)&g_reward_given               , 0x2799u,     1u }, /* OVL032 */
    { (void *)&g_reward2_given              , 0x279au,     1u }, /* OVL032 */
    { (void *)g_unused_279b                 , 0x279bu,    39u }, /* OVL032 */
    { (void *)g_save_play_end               , 0x27c2u,     1u }, /* OVL032 */
    { (void *)g_save_glob                   , 0x27c3u,     1u }, /* OVL032 */
    { (void *)&g_inn_room                   , 0x27c4u,     1u }, /* OVL032 */
    { (void *)g_group_sizes                 , 0x27c5u,     6u }, /* OVL032 */
    { (void *)g_god_favor                   , 0x27cbu,     8u }, /* OVL032 */
    { (void *)&g_god_favor_rondra           , 0x27d3u,     4u }, /* OVL032 */
    { (void *)g_unused_27d7                 , 0x27d7u,     8u }, /* OVL032 */
    { (void *)&g_god_favor_boron            , 0x27dfu,     4u }, /* OVL032 */
    { (void *)&g_god_favor_hesinde          , 0x27e3u,     4u }, /* OVL032 */
    { (void *)g_unused_27e7                 , 0x27e7u,     4u }, /* OVL032 */
    { (void *)&g_god_favor_tsa              , 0x27ebu,     4u }, /* OVL032 */
    { (void *)g_god_favor_9_10              , 0x27efu,     8u }, /* OVL032 */
    { (void *)&g_god_favor_ingerimm         , 0x27f7u,     4u }, /* OVL032 */
    { (void *)g_unused_27fb                 , 0x27fbu,    12u }, /* OVL032 */
    { (void *)&g_npc_months                 , 0x2807u,     1u }, /* OVL032 */
    { (void *)g_loc_saved_flag              , 0x2808u,     1u }, /* OVL032 */
    { (void *)&g_nr_groups                  , 0x2809u,     2u }, /* OVL032 */
    { (void *)&g_group_no                   , 0x280bu,     2u }, /* OVL032 */
    { (void *)&g_inn_days                   , 0x280du,     2u }, /* OVL032 */
    { (void *)&g_inn_town                   , 0x280fu,     2u }, /* OVL032 */
    { (void *)&g_inn_loc                    , 0x2811u,     2u }, /* OVL032 */
    { (void *)&g_inn_price                  , 0x2813u,     2u }, /* OVL032 */
    { (void *)&g_camp_food_mod              , 0x2815u,     2u }, /* OVL032 */
    { (void *)&g_camp_herb_mod              , 0x2817u,     2u }, /* OVL032 */
    { (void *)&g_rest_mod                   , 0x2819u,     2u }, /* OVL032 */
    { (void *)g_absent_hero1                , 0x281bu,    16u }, /* OVL032 */
    { (void *)g_absent_hero2                , 0x282bu,    16u }, /* OVL032 */
    { (void *)&g_dng06_alarm_delay          , 0x283bu,     4u }, /* OVL032 */
    { (void *)&g_loc_id                     , 0x283fu,     2u }, /* OVL032 */
    { (void *)&g_loc_name                   , 0x2841u,     2u }, /* OVL032 */
    { (void *)&g_dng10_altar_coins          , 0x2843u,     4u }, /* OVL032 */
    { (void *)&g_dng16_chest_coins          , 0x2847u,     4u }, /* OVL032 */
    { (void *)g_unused_284b                 , 0x284bu,    20u }, /* OVL032 */
    { (void *)&g_exit_dir                   , 0x285fu,     2u }, /* OVL032 */
    { (void *)g_house_hint_seen             , 0x2861u,    34u }, /* OVL032 */
    { (void *)&g_area_type                  , 0x2883u,     1u }, /* OVL032 */
    { (void *)&g_dng01_hook_hero            , 0x2884u,     4u }, /* OVL032 */
    { (void *)g_camp_job_names              , 0x2888u,    96u }, /* OVL032 */
    { (void *)&g_camp_herb_hours            , 0x28e8u,     1u }, /* OVL032 */
    { (void *)&g_dng05_offered_ducats       , 0x28e9u,     2u }, /* OVL032 */
    { (void *)&g_dng05_poltergeist_ticks    , 0x28ebu,     2u }, /* OVL032 */
    { (void *)g_save_glob_end               , 0x28edu,     1u }, /* OVL032 */
    { (void *)g_save_mons                   , 0x28eeu,     1u }, /* OVL032 */
    { (void *)g_mon_known                   , 0x28efu,   123u }, /* OVL032 */
    { (void *)g_save_mons_end               , 0x296au,     1u }, /* OVL032 */
    { (void *)g_save_locs                   , 0x296bu,     1u }, /* OVL032 */
    { (void *)g_smith_repairs               , 0x296cu,   108u }, /* OVL032 */
    { (void *)g_temple_desecrated           , 0x29d8u,    29u }, /* OVL032 */
    { (void *)g_inn_closed                  , 0x29f5u,    27u }, /* OVL032 */
    { (void *)g_tavern_banned               , 0x2a10u,    26u }, /* OVL032 */
    { (void *)g_smith_banned                , 0x2a2au,    21u }, /* OVL032 */
    { (void *)g_shop_banned                 , 0x2a3fu,    24u }, /* OVL032 */
    { (void *)&g_peddler_banned             , 0x2a57u,     1u }, /* OVL032 */
    { (void *)&g_peddler3_banned            , 0x2a58u,     1u }, /* OVL032 */
    { (void *)g_lodging_banned              , 0x2a59u,     5u }, /* OVL032 */
    { (void *)g_inn_has_room                , 0x2a5eu,     4u }, /* OVL032 */
    { (void *)g_inn_room_rolls              , 0x2a62u,     1u }, /* OVL032 */
    { (void *)g_shop_sells_food             , 0x2a63u,     8u }, /* OVL032 */
    { (void *)g_shop_food_rolls             , 0x2a6bu,     5u }, /* OVL032 */
    { (void *)&g_shed_free_today            , 0x2a70u,     1u }, /* OVL032 */
    { (void *)&g_dragan_room_today          , 0x2a71u,     1u }, /* OVL032 */
    { (void *)g_market_rolls_head           , 0x2a72u,     2u }, /* OVL032 */
    { (void *)g_markets                     , 0x2a74u,    33u }, /* OVL032 */
    { (void *)g_unused_2a95                 , 0x2a95u,     9u }, /* OVL032 */
    { (void *)g_save_locs_end               , 0x2a9eu,     1u }, /* OVL032 */
    { (void *)g_items_plural2               , 0x2a9fu,    58u }, /* OVL032 */
    { (void *)g_items_plural                , 0x2ad9u,    26u }, /* OVL032 */
    { (void *)g_forbidden_items_01          , 0x2af3u,    60u }, /* OVL032 */
    { (void *)g_forbidden_items_02          , 0x2b2fu,    64u }, /* OVL032 */
    { (void *)g_forbidden_items_03          , 0x2b6fu,    34u }, /* OVL032 */
    { (void *)g_forbidden_items_04          , 0x2b91u,    62u }, /* OVL032 */
    { (void *)g_forbidden_items_05          , 0x2bcfu,    46u }, /* OVL032 */
    { (void *)g_forbidden_items_06          , 0x2bfdu,    42u }, /* OVL032 */
    { (void *)g_forbidden_items_07          , 0x2c27u,   164u }, /* OVL032 */
    { (void *)g_forbidden_items_08          , 0x2ccbu,   176u }, /* OVL032 */
    { (void *)g_forbidden_items_09          , 0x2d7bu,   184u }, /* OVL032 */
    { (void *)g_forbidden_items_10          , 0x2e33u,    60u }, /* OVL032 */
    { (void *)g_forbidden_items_11          , 0x2e6fu,    62u }, /* OVL032 */
    { (void *)g_forbidden_items_12          , 0x2eadu,    62u }, /* OVL032 */
    { (void *)g_ranged_weapons_table        , 0x2f1bu,    96u }, /* OVL032 */
    { (void *)g_weapons_table               , 0x2f7bu,   624u }, /* OVL032 */
    { (void *)g_armors_table                , 0x31ebu,    54u }, /* OVL032 */
    { (void *)g_unused_3221                 , 0x3221u,     1u }, /* OVL032 */
    { (void *)g_usable_table                , 0x3222u,    57u }, /* OVL032 */
    { (void *)g_unused_325b                 , 0x325bu,     2u }, /* OVL032 */
    { (void *)g_poison_potions              , 0x325du,    20u }, /* OVL032 */
    { (void *)g_items_ask_use               , 0x3271u,    14u }, /* OVL032 */
    { (void *)g_herbs_toxic                 , 0x327fu,    10u }, /* OVL032 */
    { (void *)g_herbs_uneatable             , 0x3289u,    14u }, /* OVL032 */
    { (void *)g_elixir_potions              , 0x3297u,    16u }, /* OVL032 */
    { (void *)g_bad_elixirs                 , 0x32a7u,    16u }, /* OVL032 */
    { (void *)g_items_attack                , 0x32b7u,    14u }, /* OVL032 */
    { (void *)&g_teleport_spin              , 0x32c5u,     2u }, /* OVL032 */
    { (void *)&g_vis_start                  , 0x32c7u,     2u }, /* OVL032 */
    { (void *)&g_ems_cur_page               , 0x32c9u,     2u }, /* OVL032 */
    { (void *)&g_clip_rect                  , 0x3303u,     8u }, /* OVL032 */
    { (void *)&g_mouse_range                , 0x330bu,     8u }, /* OVL032 */
    { (void *)g_cursor_bmp                  , 0x3313u,    10u }, /* OVL032 */
    { (void *)g_cursor_bg_bmp               , 0x331du,    10u }, /* OVL032 */
    { (void *)g_3327                        , 0x3327u,    10u }, /* OVL032 */
    { (void *)g_3331                        , 0x3331u,    16u }, /* OVL032 */
    { (void *)g_gui_buttons_pos             , 0x3341u,    36u }, /* OVL032 */
    { (void *)g_gui_buttons_rest            , 0x3365u,   280u }, /* OVL032 */
    { (void *)g_fig_mouse_areas             , 0x347du,    20u }, /* OVL032 */
    { (void *)g_portrait_x                  , 0x3491u,    14u }, /* OVL032 */
    { (void *)g_dlg_areas                   , 0x349fu,   140u }, /* OVL032 */
    { (void *)g_action_table_merchant       , 0x352bu,    20u }, /* OVL032 */
    { (void *)g_disease_prices              , 0x353fu,    16u }, /* OVL032 */
    { (void *)g_disease_delays              , 0x354fu,    16u }, /* OVL032 */
    { (void *)g_poison_prices               , 0x355fu,    20u }, /* OVL032 */
    { (void *)g_poison_delays               , 0x3573u,    20u }, /* OVL032 */
    { (void *)g_dsa_version_string          , 0x3593u,    18u }, /* OVL032 */
    { (void *)&g_version_a                  , 0x35a5u,     1u }, /* OVL032 */
    { (void *)&g_version_b                  , 0x35a6u,     1u }, /* OVL032 */
    { (void *)&g_version_c                  , 0x35a7u,     1u }, /* OVL032 */
    { (void *)&g_version_d                  , 0x35a8u,     1u }, /* OVL032 */
    { (void *)g_portrait_marks              , 0x35a9u,    28u }, /* OVL032 */
    { (void *)g_unused_35c5                 , 0x35c5u,     1u }, /* OVL032 */
    { (void *)g_file_main                   , 0x35c6u,    56u }, /* OVL032 */
    { (void *)g_file_3                      , 0x35feu,    92u }, /* OVL032 */
    { (void *)g_file_fx                     , 0x365au,    56u }, /* OVL032 */
    { (void *)g_arc_fx                      , 0x3692u,    92u }, /* OVL032 */
    { (void *)g_file_speech                 , 0x36eeu,    56u }, /* OVL032 */
    { (void *)g_file_raw                    , 0x3726u,    56u }, /* OVL032 */
    { (void *)g_file_cd                     , 0x375eu,    56u }, /* OVL032 */
    { (void *)g_file_speech_ext             , 0x3796u,    56u }, /* OVL032 */
    { (void *)g_cd_cache                    , 0x37ceu,    22u }, /* OVL032 */
    { (void *)g_font                        , 0x37e4u,    16u }, /* OVL032 */
    { (void *)&g_font_dst                   , 0x37f4u,     4u }, /* OVL032 */
    { (void *)g_font_rest                   , 0x37f8u,    39u }, /* OVL032 */
    { (void *)&g_font_tab0                  , 0x381fu,     2u }, /* OVL032 */
    { (void *)g_font_tabs_rest              , 0x3821u,    12u }, /* OVL032 */
    { (void *)g_snd_music                   , 0x382du,    36u }, /* OVL032 */
    { (void *)g_unused_3851                 , 0x3851u,    41u }, /* OVL032 */
    { (void *)&g_music_off                  , 0x387au,     2u }, /* OVL032 */
    { (void *)&g_music_ready                , 0x387cu,     2u }, /* OVL032 */
    { (void *)g_unused_387e                 , 0x387eu,    20u }, /* OVL032 */
    { (void *)&g_music_id_req               , 0x3892u,     2u }, /* OVL032 */
    { (void *)g_unused_3894                 , 0x3894u,    34u }, /* OVL032 */
    { (void *)g_snd_fx                      , 0x38b6u,    77u }, /* OVL032 */
    { (void *)&g_speech_off                 , 0x3903u,     2u }, /* OVL032 */
    { (void *)&g_sound_ready                , 0x3905u,     2u }, /* OVL032 */
    { (void *)g_snd_fx_rest                 , 0x3907u,    20u }, /* OVL032 */
    { (void *)&g_speech_pos                 , 0x391bu,     2u }, /* OVL032 */
    { (void *)g_unused_391d                 , 0x391du,    34u }, /* OVL032 */
    { (void *)&g_sheet_enabled              , 0x393fu,     2u }, /* OVL032 */
    { (void *)&g_portrait_drag              , 0x3941u,     2u }, /* OVL032 */
    { (void *)g_unused_3943                 , 0x3943u,     2u }, /* OVL032 */
    { (void *)&g_paused                     , 0x3945u,     2u }, /* OVL032 */
    { (void *)&g_temple_visited             , 0x3947u,     2u }, /* OVL032 */
    { (void *)&g_temple_visited2            , 0x3949u,     2u }, /* OVL032 */
    { (void *)&g_game_state                 , 0x394bu,     2u }, /* OVL032 */
    { (void *)g_unused_394d                 , 0x394du,     2u }, /* OVL032 */
    { (void *)&g_fight_xms                  , 0x394fu,     4u }, /* OVL032 */
    { (void *)&g_input_preset               , 0x3953u,     2u }, /* OVL032 */
    { (void *)&g_hero_excluded              , 0x3955u,     1u }, /* OVL032 */
    { (void *)&g_sheet_hunger_shown         , 0x3956u,     1u }, /* OVL032 */
    { (void *)&g_sheet_thirst_shown         , 0x3957u,     1u }, /* OVL032 */
    { (void *)&g_dialog_depth               , 0x3958u,     2u }, /* OVL032 */
    { (void *)&g_loaded_dungeon             , 0x395au,     1u }, /* OVL032 */
    { (void *)&g_loaded_town                , 0x395bu,     1u }, /* OVL032 */
    { (void *)g_loc_exit_mark               , 0x395cu,     1u }, /* OVL032 */
    { (void *)&g_fight_screen_on            , 0x395du,     1u }, /* OVL032 */
    { (void *)&g_fight_flag                 , 0x395eu,     1u }, /* OVL032 */
    { (void *)&g_fight_end                  , 0x395fu,     2u }, /* OVL032 */
    { (void *)&g_timer_ticks                , 0x3961u,     4u }, /* OVL032 */
    { (void *)&g_timer_minutes              , 0x3965u,     4u }, /* OVL032 */
    { (void *)&g_timer_5min                 , 0x3969u,     4u }, /* OVL032 */
    { (void *)&g_timer_15min                , 0x396du,     4u }, /* OVL032 */
    { (void *)&g_timer_hours                , 0x3971u,     4u }, /* OVL032 */
    { (void *)&g_timer_4hours               , 0x3975u,     4u }, /* OVL032 */
    { (void *)&g_timer_days                 , 0x3979u,     4u }, /* OVL032 */
    { (void *)&g_timer_months               , 0x397du,     4u }, /* OVL032 */
    { (void *)&g_timer_years                , 0x3981u,     4u }, /* OVL032 */
    { (void *)&g_at_1000                    , 0x3985u,     2u }, /* OVL032 */
    { (void *)&g_at_0800                    , 0x3987u,     2u }, /* OVL032 */
    { (void *)&g_at_1400                    , 0x3989u,     2u }, /* OVL032 */
    { (void *)&g_at_2000_2200               , 0x398bu,     2u }, /* OVL032 */
    { (void *)&g_at_0600_0900               , 0x398du,     2u }, /* OVL032 */
    { (void *)&g_idle_ticks                 , 0x398fu,     2u }, /* OVL032 */
    { (void *)&g_saver_due                  , 0x3991u,     2u }, /* OVL032 */
    { (void *)&g_loc_herb                   , 0x3993u,     1u }, /* OVL032 */
    { (void *)&g_resting                    , 0x3994u,     2u }, /* OVL032 */
    { (void *)&g_leave_location             , 0x3996u,     2u }, /* OVL032 */
    { (void *)&g_speech_ext_avail           , 0x3998u,     2u }, /* OVL032 */
    { (void *)&g_cd_music                   , 0x399au,     2u }, /* OVL032 */
    { (void *)&g_disk_free                  , 0x399cu,     4u }, /* OVL032 */
    { (void *)g_unused_39a0                 , 0x39a0u,     4u }, /* OVL032 */
    { (void *)&g_mem_too_low                , 0x39a4u,     2u }, /* OVL032 */
    { (void *)&g_first_start                , 0x39a6u,     2u }, /* OVL032 */
    { (void *)&g_disk_needed                , 0x39a8u,     4u }, /* OVL032 */
    { (void *)g_str_temp_dir                , 0x39acu,     9u }, /* OVL032 */
    { (void *)g_39b5                        , 0x39b5u,    11u }, /* OVL032 */
    { (void *)g_39c0                        , 0x39c0u,    10u }, /* OVL032 */
    { (void *)g_39ca                        , 0x39cau,    13u }, /* OVL032 */
    { (void *)g_39d7                        , 0x39d7u,    12u }, /* OVL032 */
    { (void *)g_39e3                        , 0x39e3u,    11u }, /* OVL032 */
    { (void *)g_39ee                        , 0x39eeu,    12u }, /* OVL032 */
    { (void *)g_39fa                        , 0x39fau,     9u }, /* OVL032 */
    { (void *)g_3a03                        , 0x3a03u,     8u }, /* OVL032 */
    { (void *)g_3a0b                        , 0x3a0bu,     7u }, /* OVL032 */
    { (void *)g_3a12                        , 0x3a12u,     6u }, /* OVL032 */
    { (void *)g_3a18                        , 0x3a18u,    11u }, /* OVL032 */
    { (void *)g_3a23                        , 0x3a23u,    10u }, /* OVL032 */
    { (void *)g_3a2d                        , 0x3a2du,     8u }, /* OVL032 */
    { (void *)g_3a35                        , 0x3a35u,     7u }, /* OVL032 */
    { (void *)g_3a3c                        , 0x3a3cu,    11u }, /* OVL032 */
    { (void *)g_3a47                        , 0x3a47u,    12u }, /* OVL032 */
    { (void *)g_3a53                        , 0x3a53u,     6u }, /* OVL032 */
    { (void *)g_str_files_low               , 0x3a59u,    76u }, /* OVL032 */
    { (void *)g_str_version                 , 0x3aa5u,    31u }, /* OVL032 */
    { (void *)g_str_lastgame_3ac4           , 0x3ac4u,     9u }, /* OVL032 */
    { (void *)g_str_lastgame_3acd           , 0x3acdu,     9u }, /* OVL032 */
    { (void *)g_str_gam_3ad6                , 0x3ad6u,     5u }, /* OVL032 */
    { (void *)g_str_gam_3adb                , 0x3adbu,     5u }, /* OVL032 */
    { (void *)g_str_lastgame_3ae0           , 0x3ae0u,     9u }, /* OVL032 */
    { (void *)g_str_mem_low1                , 0x3ae9u,    19u }, /* OVL032 */
    { (void *)g_str_mem_low2                , 0x3afcu,    37u }, /* OVL032 */
    { (void *)g_str_mem_low3                , 0x3b21u,    55u }, /* OVL032 */
    { (void *)g_str_mem_low4                , 0x3b58u,    67u }, /* OVL032 */
    { (void *)g_str_mem_low5                , 0x3b9bu,    71u }, /* OVL032 */
    { (void *)g_str_mem_low6                , 0x3be2u,    71u }, /* OVL032 */
    { (void *)g_str_mem_low7                , 0x3c29u,    44u }, /* OVL032 */
    { (void *)g_str_drive_root              , 0x3c55u,     4u }, /* OVL032 */
    { (void *)g_str_all_3c59                , 0x3c59u,     5u }, /* OVL032 */
    { (void *)g_temple_miracle_bonus        , 0x3c5eu,    15u }, /* OVL033 */
    { (void *)g_temple_miracle_dice         , 0x3c6du,    15u }, /* OVL033 */
    { (void *)g_temple_god_anim             , 0x3c7cu,    16u }, /* OVL033 */
    { (void *)&g_text_block                 , 0x3c8cu,     2u }, /* OVL034 */
    { (void *)&g_hero_ptrs_null             , 0x3c8eu,    28u }, /* OVL034 */
    { (void *)&g_hero_ptrs_null2            , 0x3caau,    28u }, /* OVL034 */
    { (void *)g_sel_areas                   , 0x3cc6u,   202u }, /* OVL034 */
    { (void *)&g_options_shown              , 0x3d8eu,     2u }, /* OVL035 */
    { (void *)g_cl_finale                   , 0x3d90u,    22u }, /* OVL035 */
    { (void *)g_cl_finale_speech            , 0x3da6u,    18u }, /* OVL035 */
    { (void *)g_cl_finale_st                , 0x3db8u,     4u }, /* OVL035 */
    { (void *)g_finale_script               , 0x3dc6u,   168u }, /* OVL036 */
    { (void *)&g_save_count                 , 0x3e70u,     4u }, /* OVL037 */
    { (void *)&g_file_last_id               , 0x3e74u,     2u }, /* OVL037 */
    { (void *)g_tmap_areas                  , 0x3e86u,   100u }, /* OVL038 */
    { (void *)g_cl_travel                   , 0x3eeau,   102u }, /* OVL038 */
    { (void *)g_cl_travel_st                , 0x3f50u,    14u }, /* OVL038 */
    { (void *)g_cl_intro                    , 0x3f5eu,    30u }, /* OVL039 */
    { (void *)g_cl_intro_speech             , 0x3f7cu,    14u }, /* OVL039 */
    { (void *)g_cl_intro_st                 , 0x3f8au,     4u }, /* OVL039 */
    { (void *)g_intro_scenes                , 0x3f8eu,    48u }, /* OVL039 */
    { (void *)g_cl_anim3                    , 0x3fc0u,    32u }, /* OVL040 */
    { (void *)g_cl_anim3_speech             , 0x3fe0u,    38u }, /* OVL040 */
    { (void *)g_cl_anim3_st                 , 0x4006u,     6u }, /* OVL040 */
    { (void *)&g_anim3_lines                , 0x400cu,    96u }, /* OVL040 */
    { (void *)g_cl_outro1                   , 0x4072u,    12u }, /* OVL041 */
    { (void *)g_cl_outro1_speech            , 0x407eu,    36u }, /* OVL041 */
    { (void *)g_cl_outro_st                 , 0x40a2u,     8u }, /* OVL041 */
    { (void *)g_sndsetup_areas_sel          , 0x4272u,    40u }, /* OVL043 */
    { (void *)g_sndsetup_areas_input        , 0x429au,    30u }, /* OVL043 */
    { (void *)g_sndsetup_areas_key          , 0x42b8u,    22u }, /* OVL043 */
    { (void *)g_temple_locs_praios          , 0x49c0u,     4u }, /* OVL044 */
    { (void *)g_temple_locs_rondra          , 0x49c4u,     2u }, /* OVL044 */
    { (void *)g_temple_locs_efferd          , 0x49c6u,     1u }, /* OVL044 */
    { (void *)g_temple_locs_travia          , 0x49c7u,     3u }, /* OVL044 */
    { (void *)g_temple_locs_boron           , 0x49cau,     4u }, /* OVL044 */
    { (void *)g_temple_locs_hesinde         , 0x49ceu,     3u }, /* OVL044 */
    { (void *)g_temple_locs_firun           , 0x49d1u,     3u }, /* OVL044 */
    { (void *)g_temple_locs_tsa             , 0x49d4u,     4u }, /* OVL044 */
    { (void *)g_temple_locs_phex            , 0x49d8u,     3u }, /* OVL044 */
    { (void *)g_temple_locs_peraine         , 0x49dbu,     3u }, /* OVL044 */
    { (void *)g_temple_locs_ingerimm        , 0x49deu,     5u }, /* OVL044 */
    { (void *)g_temple_locs_rahja           , 0x49e3u,     4u }, /* OVL044 */
    { (void *)g_temple_locs_swafnir         , 0x49e7u,     1u }, /* OVL044 */
    { (void *)g_temple_locs_ifirn           , 0x49e8u,     2u }, /* OVL044 */
    { (void *)&g_temple                     , 0x4a26u,     4u }, /* OVL044 */
    { (void *)g_town_occl_04                , 0x4a3cu,    30u }, /* OVL046 */
    { (void *)g_town_occl_05                , 0x4a5au,    30u }, /* OVL046 */
    { (void *)g_town_objs_1                 , 0x4a78u,    75u }, /* OVL046 */
    { (void *)g_town_objs_2                 , 0x4ac3u,    81u }, /* OVL046 */
    { (void *)g_town_objs_6                 , 0x4b14u,    72u }, /* OVL046 */
    { (void *)g_town_objs_3                 , 0x4b5cu,    42u }, /* OVL046 */
    { (void *)g_town_objs_4                 , 0x4b86u,    87u }, /* OVL046 */
    { (void *)g_town_objs_5                 , 0x4bddu,    90u }, /* OVL046 */
    { (void *)g_cl_town                     , 0x4ca3u,    34u }, /* OVL046 */
    { (void *)g_cl_town_st                  , 0x4cc5u,     6u }, /* OVL046 */
    { (void *)g_cl_town1                    , 0x4ccbu,    14u }, /* OVL046 */
    { (void *)g_cl_town1_st                 , 0x4cd9u,     2u }, /* OVL046 */
    { (void *)g_cl_town2                    , 0x4cdbu,    16u }, /* OVL046 */
    { (void *)g_cl_town2_st                 , 0x4cebu,     2u }, /* OVL046 */
    { (void *)g_cl_town3                    , 0x4cedu,    16u }, /* OVL046 */
    { (void *)g_cl_town3_st                 , 0x4cfdu,     2u }, /* OVL046 */
    { (void *)g_cl_town4                    , 0x4cffu,    20u }, /* OVL046 */
    { (void *)g_cl_town4_st                 , 0x4d13u,     4u }, /* OVL046 */
    { (void *)g_cl_town5                    , 0x4d17u,    14u }, /* OVL046 */
    { (void *)g_cl_town5_st                 , 0x4d25u,     2u }, /* OVL046 */
    { (void *)g_cl_town6                    , 0x4d27u,    14u }, /* OVL046 */
    { (void *)g_cl_town6_st                 , 0x4d35u,     3u }, /* OVL046 */
    { (void *)g_item_combos                 , 0x4dd8u,   480u }, /* OVL048 */
    { (void *)g_recipe_items                , 0x4fb8u,    28u }, /* OVL048 */
    { (void *)g_recipe_outcomes             , 0x4fd4u,    28u }, /* OVL048 */
    { (void *)g_keyring_keys                , 0x4ff0u,    52u }, /* OVL048 */
    { (void *)&g_weapons_no_poison          , 0x5024u,    14u }, /* OVL048 */
    { (void *)&g_item_anim_snd              , 0x5032u,     2u }, /* OVL048 */
    { (void *)&g_item_anim_color            , 0x5034u,     4u }, /* OVL048 */
    { (void *)g_conv_areas                  , 0x5048u,   260u }, /* OVL049 */
    { (void *)g_str_ace                     , 0x514eu,     4u }, /* OVL050 */
    { (void *)g_str_ace2                    , 0x5152u,     6u }, /* OVL050 */
    { (void *)g_amap_areas                  , 0x5284u,    70u }, /* OVL052 */
    { (void *)g_amap_handler_areas          , 0x52cau,    70u }, /* OVL052 */
    { (void *)&g_reveal_by_spell            , 0x5310u,     2u }, /* OVL052 */
    { (void *)g_walk_default                , 0x5322u,    27u }, /* OVL053 */
    { (void *)g_walk_town5                  , 0x533du,    13u }, /* OVL053 */
    { (void *)g_walk_town4                  , 0x534au,    14u }, /* OVL053 */
    { (void *)g_walk_dng_01                 , 0x5358u,    49u }, /* OVL053 */
    { (void *)g_walk_dng_02                 , 0x5389u,    11u }, /* OVL053 */
    { (void *)g_walk_dng_03                 , 0x5394u,    16u }, /* OVL053 */
    { (void *)g_walk_dng_04                 , 0x53a4u,    27u }, /* OVL053 */
    { (void *)g_walk_dng_05                 , 0x53bfu,    22u }, /* OVL053 */
    { (void *)g_walk_dng_06                 , 0x53d5u,    36u }, /* OVL053 */
    { (void *)g_walk_dng_07                 , 0x53f9u,    36u }, /* OVL053 */
    { (void *)g_walk_dng_08                 , 0x541du,    36u }, /* OVL053 */
    { (void *)g_walk_town3                  , 0x5441u,    48u }, /* OVL053 */
    { (void *)g_walk_dng_10                 , 0x5471u,    17u }, /* OVL053 */
    { (void *)g_walk_dng_11                 , 0x5482u,    16u }, /* OVL053 */
    { (void *)g_walk_dng_12                 , 0x5492u,    22u }, /* OVL053 */
    { (void *)g_walk_dng_13                 , 0x54a8u,    18u }, /* OVL053 */
    { (void *)g_walk_dng_14                 , 0x54bau,    20u }, /* OVL053 */
    { (void *)g_walk_dng_16                 , 0x54ceu,    17u }, /* OVL053 */
    { (void *)g_str_der                     , 0x555cu,     4u }, /* OVL056 */
    { (void *)g_str_die                     , 0x5560u,     4u }, /* OVL056 */
    { (void *)g_str_das                     , 0x5564u,     4u }, /* OVL056 */
    { (void *)g_str_des                     , 0x5568u,     4u }, /* OVL056 */
    { (void *)g_str_den                     , 0x556cu,     4u }, /* OVL056 */
    { (void *)g_str_dem                     , 0x5570u,     4u }, /* OVL056 */
    { (void *)g_str_ein                     , 0x5574u,     4u }, /* OVL056 */
    { (void *)g_str_eine                    , 0x5578u,     5u }, /* OVL056 */
    { (void *)g_str_eines                   , 0x557du,     6u }, /* OVL056 */
    { (void *)g_str_einer                   , 0x5583u,     6u }, /* OVL056 */
    { (void *)g_str_einen                   , 0x5589u,     6u }, /* OVL056 */
    { (void *)g_str_eine2                   , 0x558fu,     5u }, /* OVL056 */
    { (void *)g_str_einem                   , 0x5594u,     6u }, /* OVL056 */
    { (void *)g_str_no_article              , 0x559au,     1u }, /* OVL056 */
    { (void *)g_str_von                     , 0x559bu,     4u }, /* OVL056 */
    { (void *)g_grammar_def_table           , 0x55dbu,    48u }, /* OVL056 */
    { (void *)g_grammar_indef_table         , 0x560bu,    96u }, /* OVL056 */
    { (void *)g_mon_no_article              , 0x5673u,     4u }, /* OVL056 */
    { (void *)&g_grammar_buf_no             , 0x5677u,     2u }, /* OVL056 */
    { (void *)&g_grammar_gender_bits        , 0x5679u,     6u }, /* OVL056 */
    { (void *)g_str_er                      , 0x567fu,     3u }, /* OVL056 */
    { (void *)g_str_sie                     , 0x5682u,     4u }, /* OVL056 */
    { (void *)g_str_es                      , 0x5686u,     3u }, /* OVL056 */
    { (void *)g_str_sein                    , 0x5689u,     5u }, /* OVL056 */
    { (void *)g_str_ihr                     , 0x568eu,     4u }, /* OVL056 */
    { (void *)g_str_ihn                     , 0x5692u,     4u }, /* OVL056 */
    { (void *)g_str_ihm                     , 0x5696u,    10u }, /* OVL056 */
    { (void *)g_str_seine                   , 0x56a0u,     6u }, /* OVL056 */
    { (void *)g_str_ihre                    , 0x56a6u,     5u }, /* OVL056 */
    { (void *)g_str_seinen                  , 0x56abu,     7u }, /* OVL056 */
    { (void *)g_str_ihren                   , 0x56b2u,     6u }, /* OVL056 */
    { (void *)g_grammar_pronoun_idx         , 0x56d4u,    24u }, /* OVL056 */
    { (void *)&g_score7_zero                , 0x56fcu,    14u }, /* OVL058 */
    { (void *)g_dng11_kobold_dest_single    , 0x575eu,     6u }, /* OVL062 */
    { (void *)g_dng11_kobold_dest_split     , 0x5764u,    12u }, /* OVL062 */
    { (void *)g_dng11_kobold_dest_far       , 0x5770u,    18u }, /* OVL062 */
    { (void *)&g_dng12_cart_hero            , 0x579au,     4u }, /* OVL063 */
    { (void *)&g_dng14_hero_ptrs_null       , 0x57b4u,    28u }, /* OVL066 */
    { (void *)g_camp_foes                   , 0x57dcu,    60u }, /* OVL067 */
    { (void *)g_gather_herbs_table          , 0x5826u,    52u }, /* OVL071 */
    { (void *)g_camp_water_season           , 0x585au,     4u }, /* OVL071 */
    { (void *)g_camp_hunt_season            , 0x585eu,     4u }, /* OVL071 */
    { (void *)g_camp_herb_season            , 0x5862u,     4u }, /* OVL071 */
    { (void *)g_camp_water_weather          , 0x5866u,     6u }, /* OVL071 */
    { (void *)g_camp_hunt_weather           , 0x586cu,     6u }, /* OVL071 */
    { (void *)g_camp_herb_weather           , 0x5872u,     6u }, /* OVL071 */
    { (void *)&g_camp_ambush_watch          , 0x5882u,     2u }, /* OVL072 */
    { (void *)g_camp_encounters             , 0x5884u,   242u }, /* OVL072 */
    { (void *)&g_name_buf_no                , 0x5974u,     2u }, /* OVL076 */
    { (void *)&g_mr_modificators            , 0x5976u,    13u }, /* OVL076 */
    { (void *)g_chest_types                 , 0x5990u,    51u }, /* OVL078 */
    { (void *)&g_quiet                      , 0x5a0eu,     2u }, /* OVL084 */
    { (void *)g_items_keep                  , 0x5a10u,     6u }, /* OVL084 */
    { (void *)g_lvl_attrib_order            , 0x5a80u,   168u }, /* OVL086 */
    { (void *)&g_lvl_skill_incs             , 0x5b28u,    12u }, /* OVL086 */
    { (void *)&g_lvl_spell_incs             , 0x5b34u,     6u }, /* OVL086 */
    { (void *)&g_lvl_incs_swap              , 0x5b3au,     6u }, /* OVL086 */
    { (void *)g_lvl_spells_witch            , 0x5b40u,     6u }, /* OVL086 */
    { (void *)g_lvl_spells_druid            , 0x5b46u,     8u }, /* OVL086 */
    { (void *)g_lvl_spells_school1          , 0x5b4eu,    22u }, /* OVL086 */
    { (void *)g_lvl_spells_school2          , 0x5b64u,    24u }, /* OVL086 */
    { (void *)g_lvl_spells_school3          , 0x5b7cu,    24u }, /* OVL086 */
    { (void *)g_lvl_spells_school4          , 0x5b94u,    24u }, /* OVL086 */
    { (void *)g_lvl_spells_school5          , 0x5bacu,    22u }, /* OVL086 */
    { (void *)g_lvl_spells_school6          , 0x5bc2u,    20u }, /* OVL086 */
    { (void *)g_lvl_spells_school7          , 0x5bd6u,    30u }, /* OVL086 */
    { (void *)g_lvl_spells_school8          , 0x5bf4u,    24u }, /* OVL086 */
    { (void *)g_lvl_spells_school9          , 0x5c0cu,    26u }, /* OVL086 */
    { (void *)g_lvl_spells_green_elf        , 0x5c4au,    14u }, /* OVL086 */
    { (void *)g_lvl_spells_sylvan_elf       , 0x5c58u,    14u }, /* OVL086 */
    { (void *)g_lvl_spells_ice_elf          , 0x5c66u,    24u }, /* OVL086 */
    { (void *)g_school_spell_ranges         , 0x5c7eu,    18u }, /* OVL088 */
    { (void *)&g_select_spell_tmpl          , 0x5ca0u,    18u }, /* OVL091 */
    { (void *)&g_spell_cols1                , 0x5cb2u,    48u }, /* OVL094 */
    { (void *)&g_spell_cols2                , 0x5ce2u,    24u }, /* OVL094 */
    { (void *)&g_skill_cols                 , 0x5d0cu,    42u }, /* OVL095 */
    { (void *)g_diary_ctrl                  , 0x605cu,     4u }, /* OVL103 */
    { (void *)g_diary_tag                   , 0x6060u,     6u }, /* OVL103 */
    { (void *)g_diary_areas                 , 0x6066u,    90u }, /* OVL103 */
    { (void *)g_fmt_diary_c                 , 0x60feu,     4u }, /* OVL104 */
    { (void *)g_fmt_diary_c2                , 0x6102u,     4u }, /* OVL104 */
    { (void *)g_fmt_diary_end               , 0x6106u,     6u }, /* OVL104 */
    { (void *)g_str_diary_dat5              , 0x6124u,    10u }, /* OVL105 */
    { (void *)g_str_diary_txt2              , 0x612eu,    16u }, /* OVL105 */
    { (void *)g_savemenu_areas              , 0x618au,   120u }, /* OVL107 */
    { (void *)g_lock_types                  , 0x6230u,    48u }, /* OVL108 */
    { (void *)g_anim_slots                  , 0x62e4u,    30u }, /* OVL116 */
    { (void *)g_anim_slots_extra            , 0x6302u,    10u }, /* OVL116 */
    { (void *)g_fig_obj_dxy                 , 0x631cu,   496u }, /* OVL131 */
    { (void *)g_fig_tail_dxy                , 0x650cu,    16u }, /* OVL131 */
    { (void *)g_fig_transparent_objs        , 0x651cu,    40u }, /* OVL131 */
    { (void *)&g_fig_origin_x               , 0x6544u,     2u }, /* OVL131 */
    { (void *)&g_fig_origin_y               , 0x6546u,     2u }, /* OVL131 */
    { (void *)g_fig_stand_frames            , 0x6548u,     4u }, /* OVL131 */
    { (void *)g_fig_hero_stand_wc4          , 0x654cu,     4u }, /* OVL131 */
    { (void *)g_fig_hero_stand_wc6          , 0x6550u,     4u }, /* OVL131 */
    { (void *)g_fig_hero_sleep_frames       , 0x6554u,     4u }, /* OVL131 */
    { (void *)g_fig_stone_frames            , 0x6558u,     4u }, /* OVL131 */
    { (void *)g_fig_step_frames             , 0x655cu,     4u }, /* OVL131 */
    { (void *)g_fig_step2_frames            , 0x6560u,     4u }, /* OVL131 */
    { (void *)g_fig_attack_frames           , 0x6564u,     4u }, /* OVL131 */
    { (void *)g_fig_parry_frames            , 0x6568u,     4u }, /* OVL131 */
    { (void *)g_fig_hero_attack_frames      , 0x656cu,    16u }, /* OVL131 */
    { (void *)g_fig_action_frames           , 0x657cu,    28u }, /* OVL131 */
    { (void *)g_fig_spell_frames            , 0x6598u,     8u }, /* OVL131 */
    { (void *)g_unused_65a0                 , 0x65a0u,     4u }, /* OVL131 */
    { (void *)g_fig_spell_hit_frames        , 0x65a4u,     4u }, /* OVL131 */
    { (void *)g_fig_hero_use_frames         , 0x65a8u,     4u }, /* OVL131 */
    { (void *)g_fig_hero_throw_frames       , 0x65acu,     4u }, /* OVL131 */
    { (void *)g_fig_hero_pose_frames        , 0x65b0u,     4u }, /* OVL131 */
    { (void *)g_twofield_mons               , 0x65b4u,    13u }, /* OVL131 */
    { (void *)g_mons_corpse_blocks          , 0x65c1u,     4u }, /* OVL131 */
    { (void *)g_gfx_humanoid                , 0x65c5u,    24u }, /* OVL131 */
    { (void *)g_gfx_undead                  , 0x65ddu,     8u }, /* OVL131 */
    { (void *)&g_fight_flee_all             , 0x65e5u,     2u }, /* OVL131 */
    { (void *)g_unused_65e7                 , 0x65e7u,     1u }, /* OVL131 */
    { (void *)g_dng01_scatter_pos           , 0x6600u,    40u }, /* OVL132 */
    { (void *)&g_dng01_scatter_init         , 0x6628u,    28u }, /* OVL132 */
    { (void *)&g_fight_timer                , 0x6644u,     2u }, /* OVL139 */
    { (void *)g_env_left_idx                , 0x6646u,     9u }, /* OVL139 */
    { (void *)g_env_right_idx               , 0x664fu,     9u }, /* OVL139 */
    { (void *)&g_fig_dirs4                  , 0x6658u,    16u }, /* OVL139 */
    { (void *)g_fight_defs_ab               , 0x6668u,   483u }, /* OVL140 */
    { (void *)g_fig_ranges                  , 0x684bu,     6u }, /* OVL140 */
    { (void *)g_fig_entry_pos               , 0x6851u,    10u }, /* OVL140 */
    { (void *)g_env_cell_offs               , 0x685bu,    24u }, /* OVL140 */
    { (void *)&g_fight_env_build            , 0x6873u,     2u }, /* OVL140 */
    { (void *)g_sfight_figs                 , 0x6882u,     1u }, /* OVL141 */
    { (void *)&g_sfight_fig1                , 0x6883u,     1u }, /* OVL141 */
    { (void *)&g_sfight_fig2                , 0x6884u,     1u }, /* OVL141 */
    { (void *)&g_sfight_fig3                , 0x6885u,     1u }, /* OVL141 */
    { (void *)g_sfight_part_x               , 0x6886u,     8u }, /* OVL141 */
    { (void *)g_sfight_part_y               , 0x688eu,     8u }, /* OVL141 */
    { (void *)&g_sfight_pos                 , 0x6896u,     2u }, /* OVL141 */
    { (void *)&g_sfight_phase               , 0x6898u,     2u }, /* OVL141 */
    { (void *)g_sfight_grid_xy              , 0x689au,   312u }, /* OVL141 */
    { (void *)g_sfight_blocked              , 0x69d2u,    26u }, /* OVL141 */
    { (void *)g_sfight_walls                , 0x69ecu,    46u }, /* OVL141 */
    { (void *)g_sfight_walls2               , 0x6a1au,    20u }, /* OVL141 */
    { (void *)g_sfight_cells                , 0x6a2eu,   202u }, /* OVL141 */
    { (void *)&g_sfight_seq_on              , 0x6af8u,     2u }, /* OVL141 */
    { (void *)&g_sfight_starts              , 0x6afau,    28u }, /* OVL141 */
    { (void *)&g_sfight_spells              , 0x6b16u,    26u }, /* OVL141 */
    { (void *)&g_sfight_spell_seq1          , 0x6b30u,    27u }, /* OVL142 */
    { (void *)&g_sfight_spell_seq2          , 0x6b4bu,    25u }, /* OVL142 */
    { (void *)&g_sfight_pic_size            , 0x6b64u,    28u }, /* OVL142 */
    { (void *)&g_sfight_pic_pos             , 0x6b80u,    28u }, /* OVL142 */
    { (void *)&g_hero_ptrs7_null            , 0x6b9cu,    28u }, /* OVL143 */
    { (void *)&g_hero_ptrs7_null2           , 0x6bb8u,    28u }, /* OVL143 */
    { (void *)&g_fig_anim_busy              , 0x6be6u,     2u }, /* OVL144 */
    { (void *)&g_fig_keys                   , 0x6be8u,    14u }, /* OVL144 */
    { (void *)&g_fig_dirs_back              , 0x6bf6u,    16u }, /* OVL144 */
    { (void *)&g_fig_dirs_search            , 0x6c06u,    16u }, /* OVL144 */
    { (void *)&g_fig_dirs_enemy             , 0x6c16u,    16u }, /* OVL149 */
    { (void *)&g_fig_dirs_hero              , 0x6c26u,    16u }, /* OVL149 */
    { (void *)&g_fig_dirs_ki                , 0x6c36u,    16u }, /* OVL155 */
    { (void *)g_af_spell_list               , 0x6c54u,    16u }, /* OVL163 */
    { (void *)g_str_dot_6c64                , 0x6c64u,     2u }, /* OVL163 */
    { (void *)&g_chargen_skill_incs         , 0x6c74u,    12u }, /* OVL167 */
    { (void *)&g_chargen_spell_incs         , 0x6c80u,     6u }, /* OVL167 */
    { (void *)&g_chargen_incs_swap          , 0x6c86u,     6u }, /* OVL167 */
    { (void *)&g_chargen_attrib_pos         , 0x6c8cu,    56u }, /* OVL173 */
    { (void *)&g_chargen_school_ranges      , 0x6cc4u,    18u }, /* OVL173 */
    { (void *)&g_chargen_type_list_init     , 0x6cd6u,    13u }, /* OVL173 */
    { (void *)g_unused_6ce3                 , 0x6ce3u,     1u }, /* OVL173 */
    { (void *)g_house_hints                 , 0x6ce4u,    52u }, /* OVL173 */
    { (void *)g_fig_msg_colors              , 0x6d18u,    24u }, /* OVL177 */
    { (void *)&g_dsa1_pics_m                , 0x6d30u,    12u }, /* OVL177 */
    { (void *)&g_dsa1_pics_f                , 0x6d3cu,    11u }, /* OVL177 */
    { (void *)g_str_roa_game                , 0x6d47u,     9u }, /* OVL177 */
    { (void *)g_dsa1_words_v1               , 0x6d50u,    16u }, /* OVL177 */
    { (void *)g_dsa1_longs_v1               , 0x6d60u,    10u }, /* OVL177 */
    { (void *)g_dsa1_words                  , 0x6d6au,    16u }, /* OVL177 */
    { (void *)g_dsa1_longs                  , 0x6d7au,    10u }, /* OVL177 */
    { (void *)&g_dsa1_slot_map              , 0x6d84u,    46u }, /* OVL177 */
    { (void *)&g_dsa1_drop_items            , 0x6db2u,    50u }, /* OVL177 */
    { (void *)&g_dsa1_slot_map_v1           , 0x6de4u,    46u }, /* OVL177 */
    { (void *)&g_dsa1_drop_items_v1         , 0x6e12u,    50u }, /* OVL177 */
    { (void *)&g_inn                        , 0x6e44u,     4u }, /* OVL187 */
    { (void *)&g_tmap_mark1_tick            , 0x6e48u,     1u }, /* OVL187 */
    { (void *)&g_tmap_mark2_tick            , 0x6e49u,     1u }, /* OVL187 */
    { (void *)&g_tmap_last_row              , 0x6e4au,     2u }, /* OVL187 */
    { (void *)g_loc_icons_loaded            , 0x6e4cu,    10u }, /* OVL189 */
    { (void *)g_npc_file_ids                , 0x6e78u,    18u }, /* OVL199 */
    { (void *)&g_sky_shade                  , 0x6e8au,     2u }, /* OVL199 */
    { (void *)&g_options_open               , 0x6e8cu,     2u }, /* OVL199 */
    { (void *)g_opt_areas                   , 0x6f7cu,   190u }, /* OVL199 */
    { (void *)g_opt_defaults                , 0x703au,    18u }, /* OVL199 */
    { (void *)&g_print_path_init            , 0x7082u,    20u }, /* OVL207 */
    { (void *)g_scene_places                , 0x70b6u,    52u }, /* OVL212 */
    { (void *)g_encounter_enemies           , 0x70eau,    86u }, /* OVL212 */
    { (void *)g_rose_dir_areas              , 0x7140u,    90u }, /* OVL214 */
    { (void *)g_rose_areas                  , 0x719au,    20u }, /* OVL214 */
    { (void *)&g_water_routes               , 0x71aeu,    84u }, /* OVL214 */
    { (void *)&g_hero_bar_areas             , 0x7204u,    80u }, /* OVL217 */
    { (void *)&g_rose_placed                , 0x7254u,     1u }, /* OVL222 */
    { (void *)g_unused_7255                 , 0x7255u,     1u }, /* OVL222 */
    { (void *)g_sel_list_areas              , 0x729au,    52u }, /* OVL225 */
    { (void *)&g_shop                       , 0x72ccu,     4u }, /* OVL228 */
    { (void *)g_merchant_items_posx         , 0x72deu,    30u }, /* OVL230 */
    { (void *)g_merchant_items_posy         , 0x72fcu,    30u }, /* OVL230 */
    { (void *)&g_smith                      , 0x731au,     4u }, /* OVL230 */
    { (void *)g_str_empty_731e              , 0x731eu,     2u }, /* OVL232 */
    { (void *)g_blaster_keys                , 0x7320u,     5u }, /* OVL232 */
    { (void *)g_blaster_bases               , 0x7325u,     8u }, /* OVL232 */
    { (void *)&g_blaster_result             , 0x736du,    24u }, /* OVL232 */
    { (void *)&g_heal_ani_frames            , 0x747au,    12u }, /* OVL233 */
    { (void *)&g_amap_teleport              , 0x7486u,     2u }, /* OVL235 */
    { (void *)&g_amap_teleported            , 0x7488u,     2u }, /* OVL235 */
    { (void *)g_analues_items               , 0x748au,   294u }, /* OVL235 */
    { (void *)&g_odem_items                 , 0x75b0u,     4u }, /* OVL235 */
    { (void *)&g_nekropathia_txt            , 0x75b4u,    18u }, /* OVL236 */
    { (void *)&g_trnd_old_width             , 0x75c8u,     2u }, /* OVL239 */
    { (void *)&g_tavern                     , 0x75d2u,     4u }, /* OVL241 */
    { (void *)g_months_spring               , 0x76bau,     4u }, /* OVL250 */
    { (void *)g_months_summer               , 0x76beu,     4u }, /* OVL250 */
    { (void *)g_months_winter               , 0x76c2u,     4u }, /* OVL250 */
    { (void *)g_sky_arc_y                   , 0x76c6u,    74u }, /* OVL250 */
    { (void *)g_unused_7710                 , 0x7710u,     2u }, /* OVL250 */
    { (void *)&g_sky_pos                    , 0x7712u,     1u }, /* OVL250 */
    { (void *)g_str_empty_7718              , 0x7718u,     1u }, /* OVL254 */
    { (void *)g_str_empty_7719              , 0x7719u,     1u }, /* OVL254 */
    { (void *)g_str_empty_771a              , 0x771au,     1u }, /* OVL254 */
    { (void *)g_str_empty_771b              , 0x771bu,     1u }, /* OVL254 */
    { (void *)g_str_empty_771c              , 0x771cu,     1u }, /* OVL254 */
    { (void *)g_unused_771d                 , 0x771du,     1u }, /* OVL254 */
    { (void *)g_conv_topic_tab              , 0x771eu,   102u }, /* OVL254 */
    { (void *)g_never_confiscated           , 0x78c6u,     6u }, /* OVL263 */
    { (void *)g_shade_segs                  , 0x85e4u,    64u }, /* OVL269 */
    { (void *)g_fog_table                   , 0x8624u,    32u }, /* OVL269 */
    { (void *)g_unused_8644                 , 0x8644u,    24u }, /* OVL269 */
    { (void *)&g_copyprot_passed            , 0x3e6eu,     2u }, /* SEG003 */
    { (void *)g_xms_handles                 , 0x43dcu,    40u }, /* SEG005 */
    { (void *)&g_xms_reserved               , 0x4404u,     2u }, /* SEG005 */
    { (void *)&g_path_init                  , 0x4406u,    80u }, /* SEG005 */
    { (void *)&g_ltx_count_init             , 0x4456u,     5u }, /* SEG005 */
    { (void *)g_fmt_col4_531c               , 0x531cu,     6u }, /* SEG008 */
    { (void *)&g_dng16_key_event1           , 0x5d46u,     4u }, /* SEG012 */
    { (void *)&g_dng16_key_event2           , 0x5d4au,     4u }, /* SEG012 */
    { (void *)&g_pick_delay                 , 0x5d4eu,     2u }, /* SEG012 */
    { (void *)&g_last_cam_cell_y            , 0x5d50u,     2u }, /* SEG012 */
    { (void *)&g_last_cam_cell_x            , 0x5d52u,     2u }, /* SEG012 */
    { (void *)&g_view_cursor_areas          , 0x5d54u,    72u }, /* SEG012 */
    { (void *)&g_cam_speed_set              , 0x5ee4u,     2u }, /* SEG013 */
    { (void *)&g_cam_bounced                , 0x5ee6u,     2u }, /* SEG013 */
    { (void *)g_func_trace                  , 0x5ee8u,    20u }, /* SEG014 */
    { (void *)&g_dev_screen_on              , 0x5efcu,     2u }, /* SEG014 */
    { (void *)g_unused_5efe                 , 0x5efeu,     2u }, /* SEG014 */
    { (void *)&g_xms_free_kb                , 0x5f00u,     2u }, /* SEG014 */
    { (void *)&g_min_stack                  , 0x5f02u,     2u }, /* SEG014 */
    { (void *)&g_dbg_raster_mouse           , 0x5f04u,     2u }, /* SEG014 */
    { (void *)&g_mem_debug                  , 0x5f06u,     2u }, /* SEG014 */
    { (void *)&g_mem_debug2                 , 0x5f08u,     2u }, /* SEG014 */
    { (void *)&g_func_trace_on              , 0x5f0au,     2u }, /* SEG014 */
    { (void *)&g_dbg_raster_timer           , 0x5f0cu,     2u }, /* SEG014 */
    { (void *)g_unused_5f0e                 , 0x5f0eu,    10u }, /* SEG014 */
    { (void *)&g_debug_txt_buf              , 0x5f18u,     4u }, /* SEG014 */
    { (void *)&g_debug_txt                  , 0x5f1cu,     4u }, /* SEG014 */
    { (void *)&g_str_crlf                   , 0x5f20u,     3u }, /* SEG014 */
    { (void *)g_edit_chars                  , 0x6288u,    90u }, /* SEG018 */
    { (void *)g_str_empty_75d8              , 0x75d8u,     6u }, /* SEG020 */
    { (void *)g_special_days                , 0x7638u,   130u }, /* SEG021 */
    { (void *)&g_anim_ticks                 , 0x78d2u,     4u }, /* SEG023 */
    { (void *)&g_fight_timer_ticks          , 0x78d6u,     2u }, /* SEG023 */
    { (void *)&g_mouse_isr_lock             , 0x78d8u,     2u }, /* SEG023 */
    { (void *)&g_random_seed                , 0x869eu,     4u }, /* SEG037 */
    { (void *)g_187a                        , 0x187au,    80u }, /* ASM */
    { (void *)g_18ca                        , 0x18cau,    20u }, /* ASM */
    { (void *)g_18de                        , 0x18deu,     4u }, /* ASM */
    { (void *)g_18e2                        , 0x18e2u,    22u }, /* ASM */
    { (void *)g_18f8                        , 0x18f8u,     4u }, /* ASM */
    { (void *)g_18fc                        , 0x18fcu,     8u }, /* ASM */
    { (void *)g_1904                        , 0x1904u,     2u }, /* ASM */
    { (void *)g_1906                        , 0x1906u,    18u }, /* ASM */
    { (void *)g_1918                        , 0x1918u,     4u }, /* ASM */
    { (void *)g_191c                        , 0x191cu,    10u }, /* ASM */
    { (void *)g_1926                        , 0x1926u,     2u }, /* ASM */
    { (void *)g_1928                        , 0x1928u,    10u }, /* ASM */
    { (void *)g_1932                        , 0x1932u,     2u }, /* ASM */
    { (void *)g_1934                        , 0x1934u,    10u }, /* ASM */
    { (void *)g_193e                        , 0x193eu,     2u }, /* ASM */
    { (void *)g_1940                        , 0x1940u,     8u }, /* ASM */
    { (void *)g_1948                        , 0x1948u,     2u }, /* ASM */
    { (void *)g_194a                        , 0x194au,    18u }, /* ASM */
    { (void *)g_195c                        , 0x195cu,     2u }, /* ASM */
    { (void *)g_195e                        , 0x195eu,     6u }, /* ASM */
    { (void *)g_1964                        , 0x1964u,     2u }, /* ASM */
};

static void dsa_apply_dgroup_tables(void) {
    const size_t n = sizeof(s_dgroup_copies) / sizeof(s_dgroup_copies[0]);
    for (size_t i = 0; i < n; ++i) {
        memcpy(s_dgroup_copies[i].dst, g_schweif_dgroup + s_dgroup_copies[i].ds_off, s_dgroup_copies[i].size);
    }
    /* Gemischte Tabellen (Datenfelder + Zeiger): Datenfelder einlesen, Zeiger erhalten */
    for (int i = 0; i < 16; ++i) {
        memcpy((unsigned char *)&g_dng_view_areas[i], g_schweif_dgroup + 0x0bcf + i * 18, 14);
    }
    memcpy((unsigned char *)&g_fli_screen, g_schweif_dgroup + 0x1522, 10);
    for (int i = 0; i < 6; ++i) {
        memcpy((unsigned char *)&g_town_view_areas[i], g_schweif_dgroup + 0x4c37 + i * 18, 14);
    }
    for (int i = 0; i < 17; ++i) {
        memcpy((unsigned char *)&g_opt_entries[i], g_schweif_dgroup + 0x6e8e + i * 14, 6);
        memcpy((unsigned char *)&g_opt_entries[i] + 10, g_schweif_dgroup + 0x6e8e + i * 14 + 10, 4);
    }
    /* Zeigertabellen auf Zeichenketten im geladenen DGROUP-Literalpool umsetzen */
    for (int i = 0; i < 4; ++i) {
        uint16_t o_mus  = *(const uint16_t *)(g_schweif_dgroup + 0x733d + i * 4);
        uint16_t o_digi = *(const uint16_t *)(g_schweif_dgroup + 0x734d + i * 4);
        uint16_t o_card = *(const uint16_t *)(g_schweif_dgroup + 0x735d + i * 4);
        g_blaster_music_drivers[i] = o_mus  ? DS_STR(o_mus)  : NULL;
        g_blaster_digi_drivers[i]  = o_digi ? DS_STR(o_digi) : NULL;
        g_blaster_card_names[i]    = o_card ? DS_STR(o_card) : NULL;
    }
    for (int i = 0; i < 7; ++i) {
        uint16_t o_kw = *(const uint16_t *)(g_schweif_dgroup + 0x784a + i * 4);
        g_script_keywords[i] = o_kw ? DS_STR(o_kw) : NULL;
    }
    for (int i = 0; i < 6; ++i) {
        uint16_t o_akw = *(const uint16_t *)(g_schweif_dgroup + 0x7866 + i * 4);
        g_script_answer_kw[i] = o_akw ? DS_STR(o_akw) : NULL;
    }
}

/* Kompakte, rein ganzzahlige MD5-Berechnung (RFC 1321) zur Pruefung von SCHWEIF.EXE */
static void schweif_md5_compute(const uint8_t *data, size_t len, char out_hex[33]) {
    static const uint8_t s[64] = {
        7,12,17,22, 7,12,17,22, 7,12,17,22, 7,12,17,22,
        5, 9,14,20, 5, 9,14,20, 5, 9,14,20, 5, 9,14,20,
        4,11,16,23, 4,11,16,23, 4,11,16,23, 4,11,16,23,
        6,10,15,21, 6,10,15,21, 6,10,15,21, 6,10,15,21
    };
    static const uint32_t K[64] = {
        0xd76aa478u, 0xe8c7b756u, 0x242070dbu, 0xc1bdceeeu,
        0xf57c0fafu, 0x4787c62au, 0xa8304613u, 0xfd469501u,
        0x698098d8u, 0x8b44f7afu, 0xffff5bb1u, 0x895cd7beu,
        0x6b901122u, 0xfd987193u, 0xa679438eu, 0x49b40821u,
        0xf61e2562u, 0xc040b340u, 0x265e5a51u, 0xe9b6c7aau,
        0xd62f105du, 0x02441453u, 0xd8a1e681u, 0xe7d3fbc8u,
        0x21e1cde6u, 0xc33707d6u, 0xf4d50d87u, 0x455a14edu,
        0xa9e3e905u, 0xfcefa3f8u, 0x676f02d9u, 0x8d2a4c8au,
        0xfffa3942u, 0x8771f681u, 0x6d9d6122u, 0xfde5380cu,
        0xa4beea44u, 0x4bdecfa9u, 0xf6bb4b60u, 0xbebfbc70u,
        0x289b7ec6u, 0xeaa127fau, 0xd4ef3085u, 0x04881d05u,
        0xd9d4d039u, 0xe6db99e5u, 0x1fa27cf8u, 0xc4ac5665u,
        0xf4292244u, 0x432aff97u, 0xab9423a7u, 0xfc93a039u,
        0x655b59c3u, 0x8f0ccc92u, 0xffeff47du, 0x85845dd1u,
        0x6fa87e4fu, 0xfe2ce6e0u, 0xa3014314u, 0x4e0811a1u,
        0xf7537e82u, 0xbd3af235u, 0x2ad7d2bbu, 0xeb86d391u
    };
    uint32_t a0 = 0x67452301u, b0 = 0xefcdab89u, c0 = 0x98badcfeu, d0 = 0x10325476u;
    size_t new_len = ((len + 8) / 64 + 1) * 64;
    uint8_t *msg = (uint8_t *)calloc(new_len, 1);
    if (!msg) { out_hex[0] = '\0'; return; }
    memcpy(msg, data, len);
    msg[len] = 0x80;
    uint64_t bits = (uint64_t)len * 8u;
    memcpy(msg + new_len - 8, &bits, 8);

    for (size_t offset = 0; offset < new_len; offset += 64) {
        uint32_t M[16];
        for (int j = 0; j < 16; ++j) {
            M[j] = (uint32_t)msg[offset + j * 4 + 0]
                 | ((uint32_t)msg[offset + j * 4 + 1] << 8)
                 | ((uint32_t)msg[offset + j * 4 + 2] << 16)
                 | ((uint32_t)msg[offset + j * 4 + 3] << 24);
        }
        uint32_t A = a0, B = b0, C = c0, D = d0;
        for (uint32_t i = 0; i < 64; ++i) {
            uint32_t F, g;
            if (i < 16)      { F = (B & C) | ((~B) & D); g = i; }
            else if (i < 32) { F = (D & B) | ((~D) & C); g = (5u * i + 1u) % 16u; }
            else if (i < 48) { F = B ^ C ^ D;            g = (3u * i + 5u) % 16u; }
            else             { F = C ^ (B | (~D));       g = (7u * i) % 16u; }
            F = F + A + K[i] + M[g];
            A = D;
            D = C;
            C = B;
            uint32_t rot = s[i];
            B = B + ((F << rot) | (F >> (32u - rot)));
        }
        a0 += A; b0 += B; c0 += C; d0 += D;
    }
    free(msg);
    uint8_t digest[16];
    uint32_t st[4] = { a0, b0, c0, d0 };
    for (int i = 0; i < 4; ++i) {
        digest[i * 4 + 0] = (uint8_t)(st[i] & 0xFFu);
        digest[i * 4 + 1] = (uint8_t)((st[i] >> 8) & 0xFFu);
        digest[i * 4 + 2] = (uint8_t)((st[i] >> 16) & 0xFFu);
        digest[i * 4 + 3] = (uint8_t)((st[i] >> 24) & 0xFFu);
    }
    for (int i = 0; i < 16; ++i) {
        snprintf(out_hex + i * 2, 3, "%02x", digest[i]);
    }
    out_hex[32] = '\0';
}

#ifdef _WIN32
extern "C" int __stdcall MessageBoxA(void *hWnd, const char *lpText, const char *lpCaption, unsigned int uType);
#endif

static FILE *schweif_try_open_exe_candidate(const char *path, char *resolved_out, size_t resolved_sz) {
    if (!path || !path[0]) return NULL;
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s", path);
#ifndef _WIN32
    for (char *p = buf; *p; ++p) if (*p == '\\') *p = '/';
#ifndef __EMSCRIPTEN__
    dsa_resolve_existing_ci_posix(buf, sizeof(buf));
#endif
#endif
    FILE *fp = fopen(buf, "rb");
    if (fp && resolved_out && resolved_sz > 0) {
        snprintf(resolved_out, resolved_sz, "%s", buf);
    }
    return fp;
}

static void schweif_fail_user_exe(const char *msg, int headless) {
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
#ifdef _WIN32
    const char *hid = getenv("DSA_HIDDEN");
    const char *aut = getenv("DSA_PORT_AUTOTEST");
    if (!headless && !(hid && hid[0] == '1') && !(aut && aut[0] == '1')) {
        MessageBoxA(NULL, msg, "DSA - Sternenschweif: Originaldatei benoetigt", 0x10u);
    }
#else
    (void)headless;
#endif
    exit(2);
}

static void schweif_load_tables_from_user_exe(const char *gamedir, int gamedir_explicit, int headless) {
    const size_t EXPECTED_EXE_SIZE  = 1387552u;
    const char  *EXPECTED_EXE_MD5   = "da3b59ce86a82e4b8bb14d36ed37d4d0";
    const size_t DGROUP_FILE_OFFSET = 0x026EB0u;
    const size_t DGROUP_BYTE_SIZE   = 0x008B06u;

    char chosen_path[1024] = "";
    FILE *fp = NULL;

    const char *env_exe = getenv("SCHWEIF_EXE");
    if (env_exe && env_exe[0]) {
        fp = schweif_try_open_exe_candidate(env_exe, chosen_path, sizeof(chosen_path));
    } else {
        char cands[12][1024];
        int n_cands = 0;
        if (gamedir && gamedir[0]) {
#ifdef _WIN32
            snprintf(cands[n_cands++], sizeof(cands[0]), "%s\\SCHWEIF.EXE", gamedir);
            snprintf(cands[n_cands++], sizeof(cands[0]), "%s\\DATA\\SCHWEIF.EXE", gamedir);
#else
            snprintf(cands[n_cands++], sizeof(cands[0]), "%s/SCHWEIF.EXE", gamedir);
            snprintf(cands[n_cands++], sizeof(cands[0]), "%s/DATA/SCHWEIF.EXE", gamedir);
#endif
        }
        if (!gamedir_explicit) {
            snprintf(cands[n_cands++], sizeof(cands[0]), "SCHWEIF.EXE");
            snprintf(cands[n_cands++], sizeof(cands[0]), "DATA/SCHWEIF.EXE");
            snprintf(cands[n_cands++], sizeof(cands[0]), "../SCHWEIF.EXE");
            snprintf(cands[n_cands++], sizeof(cands[0]), "../Original/SCHWEIF.EXE");
            snprintf(cands[n_cands++], sizeof(cands[0]), "../../Original/SCHWEIF.EXE");
            if (g_build_dir[0]) {
                snprintf(cands[n_cands++], sizeof(cands[0]), "%s/SCHWEIF.EXE", g_build_dir);
                snprintf(cands[n_cands++], sizeof(cands[0]), "%s/../SCHWEIF.EXE", g_build_dir);
                snprintf(cands[n_cands++], sizeof(cands[0]), "%s/../Original/SCHWEIF.EXE", g_build_dir);
                snprintf(cands[n_cands++], sizeof(cands[0]), "%s/../../Original/SCHWEIF.EXE", g_build_dir);
            }
        }
        for (int i = 0; i < n_cands && !fp; ++i) {
            fp = schweif_try_open_exe_candidate(cands[i], chosen_path, sizeof(chosen_path));
        }
    }

    if (!fp) {
        char msg[1536];
        snprintf(msg, sizeof(msg),
            "Fehler: SCHWEIF.EXE wurde im Spielverzeichnis nicht gefunden (deutsche GOG-Fassung benoetigt).\n"
            "Suchort: %s\n"
            "Erwartet wird die Originaldatei SCHWEIF.EXE (Version C1.02 de, 1.387.552 Bytes, MD5 %s).",
            (env_exe && env_exe[0]) ? env_exe : ((gamedir && gamedir[0]) ? gamedir : "."),
            EXPECTED_EXE_MD5);
        schweif_fail_user_exe(msg, headless);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long fsz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fsz != (long)EXPECTED_EXE_SIZE) {
        fclose(fp);
        char msg[1536];
        snprintf(msg, sizeof(msg),
            "Fehler: SCHWEIF.EXE hat eine falsche Dateigroesse (deutsche GOG-Fassung benoetigt).\n"
            "Datei: %s (%ld Bytes)\n"
            "Erwartet: SCHWEIF.EXE Version C1.02 de (1.387.552 Bytes, MD5 %s).",
            chosen_path, fsz, EXPECTED_EXE_MD5);
        schweif_fail_user_exe(msg, headless);
        return;
    }

    uint8_t *exe_buf = (uint8_t *)malloc(EXPECTED_EXE_SIZE);
    if (!exe_buf || fread(exe_buf, 1, EXPECTED_EXE_SIZE, fp) != EXPECTED_EXE_SIZE) {
        fclose(fp);
        free(exe_buf);
        schweif_fail_user_exe("Fehler: SCHWEIF.EXE konnte nicht vollstaendig gelesen werden (deutsche GOG-Fassung benoetigt).", headless);
        return;
    }
    fclose(fp);

    char actual_md5[33];
    schweif_md5_compute(exe_buf, EXPECTED_EXE_SIZE, actual_md5);
    if (strcmp(actual_md5, EXPECTED_EXE_MD5) != 0) {
        free(exe_buf);
        char msg[1536];
        snprintf(msg, sizeof(msg),
            "Fehler: SCHWEIF.EXE hat eine falsche Pruefsumme / Version (deutsche GOG-Fassung benoetigt).\n"
            "Datei: %s (MD5 %s)\n"
            "Erwartet: SCHWEIF.EXE Version C1.02 de (1.387.552 Bytes, MD5 %s).",
            chosen_path, actual_md5, EXPECTED_EXE_MD5);
        schweif_fail_user_exe(msg, headless);
        return;
    }

    memcpy(g_schweif_dgroup, exe_buf + DGROUP_FILE_OFFSET, DGROUP_BYTE_SIZE);
    free(exe_buf);

    dsa_apply_dgroup_tables();
}
