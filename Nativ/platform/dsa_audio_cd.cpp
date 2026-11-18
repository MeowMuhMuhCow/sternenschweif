/* Ton fuer den Sternenschweif-Port.
 * Standard: GOG-CD-Titel MUSIC/Track02.ogg .. Track22.ogg.
 * Dekoder: stb_vorbis (wie in der gemeinsamen Plattformschicht der Nordlandtrilogie-Ports).
 * Ausgabe: waveOut, weil dieser Port keine SDL2-Audiothek einbindet (winmm ist schon gelinkt).
 * Digitale VOC-Effekte und Sprache mischen in denselben Strom.
 * F10 bleibt die Pause des Originals. Lautstaerke: F11/F12, mit Strg die Effekte.
 * General MIDI und AdLib sind hier nicht nachgebaut.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"
}

#include "dsa_platform.h"

enum { CD_BUFS = 4, CD_FRAMES = 2048 };

static CRITICAL_SECTION g_cd_lock;
static int g_cd_lock_ready = 0;
static HWAVEOUT g_wave = NULL;
static WAVEHDR g_hdr[CD_BUFS];
static int16_t g_pcm[CD_BUFS][CD_FRAMES * 2];
static stb_vorbis *g_vorb = NULL;
static int g_channels = 2;
static int g_rate = 44100;
static int g_playing = 0;
static int g_volume = 100; /* 0..100 wie CD::volume */
static int g_sfx_volume = 100;
static int g_track = 0;
static uint32_t g_decoded_samples = 0;
static int g_peak = 0;
static int16_t *g_voc = NULL;
static uint32_t g_voc_len = 0;
static uint32_t g_voc_pos = 0;
static int g_voc_plays = 0;
static int g_pending_submit = 0;
static int g_voc_dumped = 0;
static volatile int g_wave_closing = 0;

static void ton_log(const char *msg) {
    const char *path = getenv("DSA_TON_LOG");
    if (!path || !path[0] || !msg) return;
    FILE *fp = fopen(path, "a");
    if (!fp) return;
    fprintf(fp, "%s\n", msg);
    fclose(fp);
}

extern "C" void dsa_platform_ton_log(const char *msg) {
    ton_log(msg);
}

static void cd_lock(void) {
    if (!g_cd_lock_ready) {
        InitializeCriticalSection(&g_cd_lock);
        g_cd_lock_ready = 1;
    }
    EnterCriticalSection(&g_cd_lock);
}

static void cd_unlock(void) {
    LeaveCriticalSection(&g_cd_lock);
}

static void apply_volume(int16_t *dst, const int16_t *src, int n) {
    int vol = g_volume;
    if (vol < 0) vol = 0;
    if (vol > 100) vol = 100;
    for (int i = 0; i < n; ++i) {
        int s = (int)src[i] * vol / 100;
        if (s > 32767) s = 32767;
        if (s < -32768) s = -32768;
        int a = s < 0 ? -s : s;
        if (a > g_peak) g_peak = a;
        dst[i] = (int16_t)s;
    }
}

static void fill_buffer(int16_t *dst) {
    int16_t tmp[CD_FRAMES * 2];
    int got = 0;
    if (g_vorb) {
        got = stb_vorbis_get_samples_short_interleaved(g_vorb, 2, tmp, CD_FRAMES * 2);
        if (got <= 0) {
            stb_vorbis_seek_start(g_vorb);
            got = stb_vorbis_get_samples_short_interleaved(g_vorb, 2, tmp, CD_FRAMES * 2);
        }
    }
    if (got < 0) got = 0;
    if (got < CD_FRAMES) {
        memset(tmp + got * 2, 0, (size_t)(CD_FRAMES - got) * 2 * sizeof(int16_t));
    }
    g_decoded_samples += (uint32_t)got;
    apply_volume(dst, tmp, CD_FRAMES * 2);
    if (g_voc && g_voc_pos < g_voc_len) {
        int sfx = g_sfx_volume;
        if (sfx < 0) sfx = 0;
        if (sfx > 100) sfx = 100;
        for (int i = 0; i < CD_FRAMES && g_voc_pos < g_voc_len; ++i) {
            int s = (int)g_voc[g_voc_pos++] * sfx / 100;
            int l = (int)dst[i * 2] + s;
            int r = (int)dst[i * 2 + 1] + s;
            if (l > 32767) l = 32767;
            if (l < -32768) l = -32768;
            if (r > 32767) r = 32767;
            if (r < -32768) r = -32768;
            dst[i * 2] = (int16_t)l;
            dst[i * 2 + 1] = (int16_t)r;
        }
        if (g_voc_pos >= g_voc_len) {
            free(g_voc);
            g_voc = NULL;
            g_voc_len = 0;
            g_voc_pos = 0;
        }
    }
}

static void CALLBACK wave_proc(HWAVEOUT, UINT msg, DWORD_PTR, DWORD_PTR p1, DWORD_PTR) {
    if (msg != WOM_DONE || g_wave_closing) return;
    WAVEHDR *hdr = (WAVEHDR *)p1;
    HWAVEOUT wave = NULL;
    cd_lock();
    if (!g_wave_closing && g_playing && g_wave && hdr) {
        fill_buffer((int16_t *)hdr->lpData);
        wave = g_wave;
    }
    cd_unlock();
    if (wave && !g_wave_closing)
        waveOutWrite(wave, hdr, sizeof(WAVEHDR));
}

static void close_wave(void) {
    /* Aufrufer haelt cd_lock. waveOutReset wartet auf den Rueckruf.
     * Der Rueckruf darf dabei nicht auf dieselbe Sperre warten. */
    HWAVEOUT wave = g_wave;
    stb_vorbis *vorb = g_vorb;
    g_wave = NULL;
    g_vorb = NULL;
    g_playing = 0;
    g_pending_submit = 0;
    g_wave_closing = 1;
    cd_unlock();
    if (wave) {
        waveOutReset(wave);
        for (int i = 0; i < CD_BUFS; ++i) {
            if (g_hdr[i].dwFlags & WHDR_PREPARED)
                waveOutUnprepareHeader(wave, &g_hdr[i], sizeof(WAVEHDR));
        }
        waveOutClose(wave);
    }
    if (vorb)
        stb_vorbis_close(vorb);
    cd_lock();
    g_wave_closing = 0;
}

static void ensure_wave_locked(void) {
    if (g_wave || g_playing) return;
    if (g_rate <= 0) g_rate = 44100;
    WAVEFORMATEX fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = 2;
    fmt.nSamplesPerSec = (DWORD)g_rate;
    fmt.wBitsPerSample = 16;
    fmt.nBlockAlign = 4;
    fmt.nAvgBytesPerSec = (DWORD)g_rate * 4u;
    if (waveOutOpen(&g_wave, WAVE_MAPPER, &fmt, (DWORD_PTR)wave_proc, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
        g_wave = NULL;
        g_playing = 1;
        return;
    }
    g_playing = 1;
    for (int i = 0; i < CD_BUFS; ++i) {
        memset(&g_hdr[i], 0, sizeof(g_hdr[i]));
        fill_buffer(g_pcm[i]);
        g_hdr[i].lpData = (LPSTR)g_pcm[i];
        g_hdr[i].dwBufferLength = sizeof(g_pcm[i]);
        waveOutPrepareHeader(g_wave, &g_hdr[i], sizeof(WAVEHDR));
    }
    g_pending_submit = 1;
}

static void submit_pending(void) {
    if (!g_pending_submit || !g_wave) {
        g_pending_submit = 0;
        return;
    }
    g_pending_submit = 0;
    for (int i = 0; i < CD_BUFS; ++i)
        waveOutWrite(g_wave, &g_hdr[i], sizeof(WAVEHDR));
}

static void write_probe_wav(const char *path) {
    if (!path || !path[0] || !g_vorb) return;
    FILE *fp = fopen(path, "wb");
    if (!fp) return;
    const int seconds = 2;
    const int frames = g_rate * seconds;
    const int data_bytes = frames * 2 * (int)sizeof(int16_t);
    uint8_t hdr[44];
    memset(hdr, 0, sizeof(hdr));
    memcpy(hdr, "RIFF", 4);
    uint32_t chunk = 36u + (uint32_t)data_bytes;
    memcpy(hdr + 4, &chunk, 4);
    memcpy(hdr + 8, "WAVEfmt ", 8);
    uint32_t fmt_size = 16;
    memcpy(hdr + 16, &fmt_size, 4);
    uint16_t audio_fmt = 1, ch = 2, bits = 16;
    uint32_t rate = (uint32_t)g_rate;
    uint32_t byte_rate = rate * 4u;
    uint16_t align = 4;
    memcpy(hdr + 20, &audio_fmt, 2);
    memcpy(hdr + 22, &ch, 2);
    memcpy(hdr + 24, &rate, 4);
    memcpy(hdr + 28, &byte_rate, 4);
    memcpy(hdr + 32, &align, 2);
    memcpy(hdr + 34, &bits, 2);
    memcpy(hdr + 36, "data", 4);
    uint32_t dsz = (uint32_t)data_bytes;
    memcpy(hdr + 40, &dsz, 4);
    fwrite(hdr, 1, 44, fp);
    int left = frames;
    int16_t block[1024 * 2];
    while (left > 0) {
        int n = left > 1024 ? 1024 : left;
        int got = stb_vorbis_get_samples_short_interleaved(g_vorb, 2, block, n * 2);
        if (got <= 0) break;
        apply_volume(block, block, got * 2);
        fwrite(block, sizeof(int16_t), (size_t)got * 2, fp);
        left -= got;
    }
    fclose(fp);
    stb_vorbis_seek_start(g_vorb);
}

extern "C" int dsa_platform_play_cd_track(int track_no) {
    cd_lock();
    close_wave();
    g_track = track_no;
    g_decoded_samples = 0;
    g_peak = 0;
    const char *dir = dsa_platform_get_gamedir();
    char path[1024];
    snprintf(path, sizeof(path), "%s\\MUSIC\\Track%02d.ogg", dir && dir[0] ? dir : ".", track_no);
    int err = 0;
    g_vorb = stb_vorbis_open_filename(path, &err, NULL);
    if (!g_vorb) {
        snprintf(path, sizeof(path), "%s/MUSIC/Track%02d.ogg", dir && dir[0] ? dir : ".", track_no);
        g_vorb = stb_vorbis_open_filename(path, &err, NULL);
    }
    if (!g_vorb) {
        cd_unlock();
        ton_log("cd_fehlt");
        return 0;
    }
    stb_vorbis_info info = stb_vorbis_get_info(g_vorb);
    g_rate = info.sample_rate > 0 ? (int)info.sample_rate : 44100;
    g_channels = info.channels > 0 ? info.channels : 2;
    const char *dump = getenv("DSA_DUMP_WAV");
    if (dump && dump[0]) write_probe_wav(dump);

    WAVEFORMATEX fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = 2;
    fmt.nSamplesPerSec = (DWORD)g_rate;
    fmt.wBitsPerSample = 16;
    fmt.nBlockAlign = 4;
    fmt.nAvgBytesPerSec = (DWORD)g_rate * 4;
    if (waveOutOpen(&g_wave, WAVE_MAPPER, &fmt, (DWORD_PTR)wave_proc, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
        g_wave = NULL;
        g_playing = 1; /* Datei ist offen; Wiedergabe ohne Geraet bleibt still, der Titel gilt als laufend */
        cd_unlock();
        char msg[80];
        snprintf(msg, sizeof(msg), "cd track=%d rate=%d geraet=0", track_no, g_rate);
        ton_log(msg);
        return 1;
    }
    g_playing = 1;
    for (int i = 0; i < CD_BUFS; ++i) {
        memset(&g_hdr[i], 0, sizeof(g_hdr[i]));
        fill_buffer(g_pcm[i]);
        g_hdr[i].lpData = (LPSTR)g_pcm[i];
        g_hdr[i].dwBufferLength = sizeof(g_pcm[i]);
        waveOutPrepareHeader(g_wave, &g_hdr[i], sizeof(WAVEHDR));
    }
    cd_unlock();
    for (int i = 0; i < CD_BUFS; ++i)
        waveOutWrite(g_wave, &g_hdr[i], sizeof(WAVEHDR));
    char msg[80];
    snprintf(msg, sizeof(msg), "cd track=%d rate=%d geraet=1", track_no, g_rate);
    ton_log(msg);
    return 1;
}

extern "C" void dsa_platform_stop_cd_audio(void) {
    cd_lock();
    close_wave();
    g_track = 0;
    cd_unlock();
}

extern "C" int dsa_platform_cd_is_playing(void) {
    return g_playing;
}

extern "C" void dsa_platform_set_music_volume(int vol_0_100) {
    if (vol_0_100 < 0) vol_0_100 = 0;
    if (vol_0_100 > 100) vol_0_100 = 100;
    g_volume = vol_0_100;
}

extern "C" int dsa_platform_cd_track(void) { return g_track; }
extern "C" uint32_t dsa_platform_cd_decoded(void) { return g_decoded_samples; }
extern "C" int dsa_platform_cd_peak(void) { return g_peak; }
extern "C" int dsa_platform_voc_plays(void) { return g_voc_plays; }

extern "C" void dsa_platform_set_sfx_volume(int vol_0_100) {
    if (vol_0_100 < 0) vol_0_100 = 0;
    if (vol_0_100 > 100) vol_0_100 = 100;
    g_sfx_volume = vol_0_100;
}

extern "C" int dsa_platform_get_music_volume(void) { return g_volume; }
extern "C" int dsa_platform_get_sfx_volume(void) { return g_sfx_volume; }

static uint32_t g_voc_dump_n = 0;
static uint8_t *g_voc_raw = NULL;
static uint32_t g_voc_raw_len = 0;
static uint32_t g_voc_raw_cap = 0;

static void write_voc_probe(const int16_t *pcm, uint32_t n, int rate) {
    if (!pcm || n == 0 || rate <= 0 || n <= g_voc_dump_n) return;
    const char *dump = getenv("DSA_DUMP_WAV");
    if (!dump || !dump[0]) return;
    char path[1024];
    snprintf(path, sizeof(path), "%s", dump);
    char *slash = strrchr(path, '\\');
    char *fsl = strrchr(path, '/');
    if (!slash || (fsl && fsl > slash)) slash = fsl;
    if (!slash || (size_t)(slash + 1 - path) + 14 >= sizeof(path)) return;
    memcpy(slash + 1, "voc_probe.wav", 14);
    if (n > (uint32_t)rate * 2u) n = (uint32_t)rate * 2u;
    FILE *fp = fopen(path, "wb");
    if (!fp) return;
    const int data_bytes = (int)n * 2 * (int)sizeof(int16_t);
    uint8_t hdr[44];
    memset(hdr, 0, sizeof(hdr));
    memcpy(hdr, "RIFF", 4);
    uint32_t chunk = 36u + (uint32_t)data_bytes;
    memcpy(hdr + 4, &chunk, 4);
    memcpy(hdr + 8, "WAVEfmt ", 8);
    uint32_t fmt_size = 16;
    memcpy(hdr + 16, &fmt_size, 4);
    uint16_t audio_fmt = 1, ch = 2, bits = 16;
    uint32_t srate = (uint32_t)rate;
    uint32_t byte_rate = srate * 4u;
    uint16_t align = 4;
    memcpy(hdr + 20, &audio_fmt, 2);
    memcpy(hdr + 22, &ch, 2);
    memcpy(hdr + 24, &srate, 4);
    memcpy(hdr + 28, &byte_rate, 4);
    memcpy(hdr + 32, &align, 2);
    memcpy(hdr + 34, &bits, 2);
    memcpy(hdr + 36, "data", 4);
    uint32_t dsz = (uint32_t)data_bytes;
    memcpy(hdr + 40, &dsz, 4);
    fwrite(hdr, 1, 44, fp);
    for (uint32_t i = 0; i < n; ++i) {
        int16_t s = pcm[i];
        fwrite(&s, sizeof(s), 1, fp);
        fwrite(&s, sizeof(s), 1, fp);
    }
    fclose(fp);
    g_voc_dump_n = n;
}

static int voc_decode(const unsigned char *voc_data, uint32_t voc_len,
                      int16_t **out_pcm, uint32_t *out_count, uint32_t *out_rate, int *out_peak) {
    *out_pcm = NULL;
    *out_count = 0;
    if (!voc_data || voc_len < 26) return 0;
    if (memcmp(voc_data, "Creative Voice File\x1a", 20) != 0) return 0;
    uint16_t hdr_size = (uint16_t)voc_data[20] | ((uint16_t)voc_data[21] << 8);
    if (hdr_size < 20 || hdr_size >= voc_len) return 0;
    uint32_t src_rate = 11025;
    int rate_from_type8 = 0;
    uint32_t raw_count = 0;
    uint32_t scan = hdr_size;
    while (scan < voc_len) {
        uint8_t bt = voc_data[scan];
        if (bt == 0 || scan + 4 > voc_len) break;
        uint32_t blen = (uint32_t)voc_data[scan + 1] | ((uint32_t)voc_data[scan + 2] << 8) | ((uint32_t)voc_data[scan + 3] << 16);
        if (scan + 4 + blen > voc_len) blen = voc_len - (scan + 4);
        if (bt == 8 && blen >= 4) {
            uint16_t tc = (uint16_t)voc_data[scan + 4] | ((uint16_t)voc_data[scan + 5] << 8);
            if (tc < 65535) {
                src_rate = 256000000u / (65536u - (uint32_t)tc);
                rate_from_type8 = 1;
            }
        } else if (bt == 1 && blen > 2) {
            if (!rate_from_type8) {
                uint8_t sr_div = voc_data[scan + 4];
                if (sr_div < 255) src_rate = 1000000u / (256u - (uint32_t)sr_div);
            }
            raw_count += blen - 2;
        } else if (bt == 2) {
            raw_count += blen;
        }
        scan += 4 + blen;
    }
    if (raw_count == 0 || src_rate < 1000) return 0;
    uint8_t *raw = (uint8_t *)malloc(raw_count);
    if (!raw) return 0;
    uint32_t wpos = 0;
    scan = hdr_size;
    while (scan < voc_len && wpos < raw_count) {
        uint8_t bt = voc_data[scan];
        if (bt == 0 || scan + 4 > voc_len) break;
        uint32_t blen = (uint32_t)voc_data[scan + 1] | ((uint32_t)voc_data[scan + 2] << 8) | ((uint32_t)voc_data[scan + 3] << 16);
        if (scan + 4 + blen > voc_len) blen = voc_len - (scan + 4);
        if (bt == 1 && blen > 2) {
            uint32_t n = blen - 2;
            if (wpos + n > raw_count) n = raw_count - wpos;
            memcpy(raw + wpos, voc_data + scan + 6, n);
            wpos += n;
        } else if (bt == 2 && blen > 0) {
            uint32_t n = blen;
            if (wpos + n > raw_count) n = raw_count - wpos;
            memcpy(raw + wpos, voc_data + scan + 4, n);
            wpos += n;
        }
        scan += 4 + blen;
    }
    uint32_t dec_rate = g_rate > 0 ? (uint32_t)g_rate : 44100u;
    uint32_t dec_count = (uint32_t)(((uint64_t)wpos * dec_rate) / src_rate);
    if (dec_count == 0) { free(raw); return 0; }
    int16_t *pcm = (int16_t *)malloc(dec_count * sizeof(int16_t));
    if (!pcm) { free(raw); return 0; }
    for (uint32_t i = 0; i < dec_count; ++i) {
        uint32_t src_idx = (uint32_t)(((uint64_t)i * src_rate) / dec_rate);
        if (src_idx >= wpos) src_idx = wpos - 1;
        pcm[i] = (int16_t)(((int)raw[src_idx] - 128) * 256);
    }
    free(raw);
    int peak = 0;
    for (uint32_t i = 0; i < dec_count; ++i) {
        int a = pcm[i] < 0 ? -(int)pcm[i] : (int)pcm[i];
        if (a > peak) peak = a;
    }
    *out_pcm = pcm;
    *out_count = dec_count;
    *out_rate = dec_rate;
    *out_peak = peak;
    return 1;
}

extern "C" int dsa_platform_submit_voc_chunk(const unsigned char *data, uint32_t len) {
    if (!data || len == 0) return 0;
    int fresh = (len >= 26 && memcmp(data, "Creative Voice File\x1a", 20) == 0);
    if (fresh) g_voc_raw_len = 0;
    else if (g_voc_raw_len == 0) return 0;
    if (g_voc_raw_len + len > g_voc_raw_cap) {
        uint32_t cap = g_voc_raw_cap ? g_voc_raw_cap * 2u : 65536u;
        while (cap < g_voc_raw_len + len) cap *= 2u;
        uint8_t *grown = (uint8_t *)realloc(g_voc_raw, cap);
        if (!grown) return 0;
        g_voc_raw = grown;
        g_voc_raw_cap = cap;
    }
    memcpy(g_voc_raw + g_voc_raw_len, data, len);
    g_voc_raw_len += len;
    int16_t *pcm = NULL;
    uint32_t count = 0, rate = 44100;
    int peak = 0;
    if (!voc_decode(g_voc_raw, g_voc_raw_len, &pcm, &count, &rate, &peak)) return 0;
    write_voc_probe(pcm, count, (int)rate);
    char msg[96];
    snprintf(msg, sizeof(msg), "%s samples=%u peak=%d rate=%u",
             fresh ? "voc_ok" : "voc_weiter", count, peak, rate);
    ton_log(msg);
    cd_lock();
    uint32_t keep = fresh ? 0 : g_voc_pos;
    if (g_voc) free(g_voc);
    g_voc = pcm;
    g_voc_len = count;
    g_voc_pos = keep < count ? keep : count;
    if (fresh) g_voc_plays++;
    ensure_wave_locked();
    cd_unlock();
    submit_pending();
    return 1;
}

extern "C" int dsa_platform_play_voc(const unsigned char *voc_data, uint32_t voc_len) {
    g_voc_raw_len = 0;
    return dsa_platform_submit_voc_chunk(voc_data, voc_len);
}
