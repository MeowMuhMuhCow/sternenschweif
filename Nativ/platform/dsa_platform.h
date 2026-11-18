/*
 * dsa_platform.h - Gemeinsame Plattformschicht der Nordlandtrilogie-Ports
 * fuer DSA 2: Sternenschweif.
 */
#ifndef DSA_PLATFORM_H
#define DSA_PLATFORM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DSA_SCREEN_W 320
#define DSA_SCREEN_H 200
#define DSA_VRAM_SIZE (DSA_SCREEN_W * DSA_SCREEN_H)

/* Initialisierung & Shutdown */
int  dsa_platform_init(const char *title, int scale);
void dsa_platform_shutdown(void);

/* Grafik (320x200x8 Framebuffer + 256-Farben-DAC-Palette mit 6-Bit-Werten 0..63) */
uint8_t *dsa_platform_get_vram(void);
void dsa_platform_set_palette(int start, int count, const uint8_t *rgb6);
void dsa_platform_get_palette(int start, int count, uint8_t *rgb6);
void dsa_platform_present(void);
void dsa_platform_wait_vsync(void);

/* Eingabe & Events */
int  dsa_platform_poll_events(void);
void dsa_platform_get_mouse(int *x, int *y, int *buttons);
void dsa_platform_set_mouse_pos(int x, int y);
void dsa_platform_consume_clicks(int *left_clicks, int *right_clicks);

/* Tastatur: liefert 16-Bit BIOS-Keycode (Low-Byte = ASCII, High-Byte = DOS-Scancode) */
int  dsa_platform_check_key(void);
int  dsa_platform_get_key(void);
int  dsa_platform_get_mods(void);
int  dsa_platform_get_arrow_mask(void);

/* Zeitgeber (18,2 Hz PIT-Tick-Emulation & Millisekunden) */
uint32_t dsa_platform_get_ticks_18hz(void);
uint32_t dsa_platform_get_ticks_ms(void);
void     dsa_platform_delay_ms(uint32_t ms);

/* Optionaler Hook, der bei jedem Event-Poll aufgerufen wird (fuer Timer/Maus-Emulation) */
void dsa_platform_set_tick_hook(void (*hook)(void));

/* Dateizugriff relativ zur GOG-/Originalinstallation */
void        dsa_platform_set_gamedir(const char *path);
const char *dsa_platform_get_gamedir(void);
int         dsa_platform_resolve_path(const char *dos_path, char *out_path, size_t out_size);

/* CD-Audio: MUSIC/TrackNN.ogg der GOG-Fassung (stb_vorbis, waveOut) */
int      dsa_platform_play_cd_track(int track_no);
void     dsa_platform_stop_cd_audio(void);
int      dsa_platform_cd_is_playing(void);
void     dsa_platform_set_music_volume(int vol_0_100);
int      dsa_platform_get_music_volume(void);
int      dsa_platform_play_voc(const unsigned char *voc_data, uint32_t voc_len);
int      dsa_platform_submit_voc_chunk(const unsigned char *data, uint32_t len);
int      dsa_platform_voc_is_playing(void);
void     dsa_platform_stop_voc(void);
void     dsa_platform_set_sfx_volume(int vol_0_100);
int      dsa_platform_get_sfx_volume(void);
int      dsa_platform_voc_plays(void);
void     dsa_platform_ton_log(const char *msg);
void     dsa_log_abspann(const char *msg);

/* W4: Eingabe-Aufzeichnung & Wiedergabe (--record / --replay) */
void     dsa_platform_set_record_file(const char *path);
void     dsa_platform_set_replay_file(const char *path);
uint32_t dsa_platform_init_seed(uint32_t default_seed);
uint32_t dsa_platform_get_frame_count(void);
void     dsa_platform_save_bmp(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* DSA_PLATFORM_H */
