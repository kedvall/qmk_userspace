// Copyright 2026 Kanyon
// SPDX-License-Identifier: GPL-2.0-or-later
//
// OLED screens (128x32 each, horizontal):
//   Left  = status panel: big layer name, QWERTY/Colemak, Caps, held modifiers.
//   Right = one of four modes, cycled with OLED_NXT on the ADJUST layer and
//           saved to EEPROM: Luna -> Bongo Cat -> WPM graph -> cheat sheet.
//           While NAV/SYM/ADJUST is held, it always shows that layer's cheat sheet.
//
// Only the half plugged into USB (the "master") runs the keymap. QMK's SPLIT_*
// options in config.h copy layer/mods/caps/WPM to the other half; the screen
// mode and Luna's jumps are sent over with a small custom transaction below.
//
// Drawing goes into our own 512-byte framebuffer, which is then copied into
// QMK's OLED buffer. QMK only re-sends bytes that changed, so redrawing
// everything every frame is cheap.
// Included from keymap.c (needs its layer names), not compiled on its own.
#include <string.h>
#include "transactions.h"
#include "oled_font.h"
#include "luna_frames.h"
#include "bongo_frames.h"

// ---------------------------------------------------------------------------
// Modes, saved settings, and data shared between halves
// ---------------------------------------------------------------------------

enum oled_mode { MODE_LUNA, MODE_BONGO, MODE_WPM, MODE_CHEAT, MODE_COUNT };

// QMK reserves 4 bytes of EEPROM for the keymap's own settings.
typedef union {
    uint32_t raw;
    struct {
        uint8_t oled_mode;
    };
} user_config_t;

static user_config_t user_config;

// What the master sends to the other half. Layer, mods, caps and WPM are
// already synced by QMK; this is just the extra state that's ours.
typedef struct {
    uint8_t mode;
    uint8_t jump_count; // bumped on every Space press
    bool    screen_on;  // whether the master's screen is on
} oled_sync_t;

static oled_sync_t sync_state;
static uint32_t    jump_timer;

static void sync_slave_handler(uint8_t in_len, const void *in_data, uint8_t out_len, void *out_data) {
    if (in_len == sizeof(sync_state)) memcpy(&sync_state, in_data, sizeof(sync_state));
}

void eeconfig_init_user(void) { // runs when EEPROM is reset/initialised
    user_config.raw = 0;
    eeconfig_update_user(user_config.raw);
}

void oled_post_init_user(void) {
    user_config.raw = eeconfig_read_user();
    if (user_config.oled_mode >= MODE_COUNT) user_config.oled_mode = MODE_LUNA;
    sync_state.mode = user_config.oled_mode;
    transaction_register_rpc(USER_SYNC_OLED, sync_slave_handler);
}

void oled_next_mode(void) {
    sync_state.mode       = (sync_state.mode + 1) % MODE_COUNT;
    user_config.oled_mode = sync_state.mode;
    eeconfig_update_user(user_config.raw);
}

void oled_space_pressed(void) {
    sync_state.jump_count++;
}

// WPM history for the graph: one sample per second, kept on both halves.
#define WPM_SAMPLES 84
static uint8_t wpm_history[WPM_SAMPLES];
static uint8_t wpm_head; // index of the oldest sample

void oled_housekeeping_user(void) {
    // Master: send our state to the other half when it changes (and once a second anyway).
    if (is_keyboard_master()) {
        static oled_sync_t last_sent;
        static uint32_t    last_sync;
        sync_state.screen_on = is_oled_on();
        if (memcmp(&sync_state, &last_sent, sizeof(sync_state)) != 0 || timer_elapsed32(last_sync) > 1000) {
            if (transaction_rpc_send(USER_SYNC_OLED, sizeof(sync_state), &sync_state)) {
                last_sent = sync_state;
                last_sync = timer_read32();
            }
        }
    } else {
        // Other half: after a long sleep this screen sometimes stays dark when the
        // master's comes back, and only a reset fixed it. So whenever the master's
        // screen turns on, re-send the screen's full setup instead of just "display
        // on", and keep retrying every 2 s if this screen still reports off.
        static bool     master_was_on;
        static uint32_t last_reinit;
        if (sync_state.screen_on && (!master_was_on || (!is_oled_on() && timer_elapsed32(last_reinit) > 2000))) {
            oled_init(OLED_ROTATION_0);
            last_reinit = timer_read32();
        }
        master_was_on = sync_state.screen_on;
    }

    // Both halves: start a jump when the jump counter changes.
    static uint8_t jumps_seen;
    if (sync_state.jump_count != jumps_seen) {
        jumps_seen = sync_state.jump_count;
        jump_timer = timer_read32();
    }

    static uint32_t sample_timer;
    if (timer_elapsed32(sample_timer) >= 1000) {
        sample_timer          = timer_read32();
        wpm_history[wpm_head] = get_current_wpm();
        wpm_head              = (wpm_head + 1) % WPM_SAMPLES;
    }
}

// ---------------------------------------------------------------------------
// Drawing helpers. Screen = 128 columns x 4 "pages"; each page is 8 pixels
// tall and each byte is one 1x8 column (bit 0 = top pixel).
// ---------------------------------------------------------------------------

#define SCREEN_W 128
#define PAGES 4
static uint8_t fb[SCREEN_W * PAGES];

// Key icons living in oled_font.h (0x18-0x1F are standard code page 437 arrows)
#define G_CMD 0x01
#define G_OPT 0x02
#define G_CTL 0x03
#define G_SFT 0x04
#define G_CAPS 0x05
#define G_BSPC 0x06
#define G_ESC 0x0B
#define G_DEL 0x0E
#define G_TAB 0x0F
#define G_UP 0x18
#define G_DOWN 0x19
#define G_RGHT 0x1A
#define G_LEFT 0x1B
#define G_PGUP 0x1E // ▲
#define G_PGDN 0x1F // ▼
#define G_PLAY 0x10 // ►
#define G_PREV 0x11 // ◄
#define INV 0x80    // cheat sheet: draw this cell inverted (white box)

static void put_col(int x, int page, uint8_t bits) {
    if (x >= 0 && x < SCREEN_W && page >= 0 && page < PAGES) fb[page * SCREEN_W + x] = bits;
}

static void or_pixel(int x, int y) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < PAGES * 8) fb[(y / 8) * SCREEN_W + x] |= 1 << (y % 8);
}

static uint8_t glyph_col(uint8_t c, uint8_t i) {
    return pgm_read_byte(&oled_font[(c & 0x7F) * 6 + i]);
}

static void draw_char(int x, int page, uint8_t c, bool inverted) {
    for (uint8_t i = 0; i < 6; i++) {
        uint8_t b = glyph_col(c, i);
        put_col(x + i, page, inverted ? ~b : b);
    }
}

static int draw_str(int x, int page, const char *s, bool inverted) {
    for (; *s; s++, x += 6) draw_char(x, page, (uint8_t)*s, inverted);
    return x;
}

// Text in a box: inverted with a one-pixel margin when `on`, plain otherwise.
static void draw_label(int x, int page, const char *s, bool on) {
    put_col(x, page, on ? 0xFF : 0);
    int end = draw_str(x + 1, page, s, on);
    put_col(end, page, on ? 0xFF : 0);
}

// Double-size character (12x16), spanning `page` and `page + 1`.
static void draw_char2x(int x, int page, uint8_t c) {
    for (uint8_t i = 0; i < 6; i++) {
        uint8_t  b    = glyph_col(c, i);
        uint16_t tall = 0;
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (b & (1 << bit)) tall |= 3u << (bit * 2);
        }
        for (uint8_t d = 0; d < 2; d++) {
            put_col(x + i * 2 + d, page, tall & 0xFF);
            put_col(x + i * 2 + d, page + 1, tall >> 8);
        }
    }
}

static void draw_str2x(int x, int page, const char *s) {
    for (; *s; s++, x += 12) draw_char2x(x, page, (uint8_t)*s);
}

// Copy a sprite stored page by page (w columns x `pages` pages).
static void blit_P(int x, int page, int w, int pages, const uint8_t *data) {
    for (int p = 0; p < pages; p++) {
        for (int i = 0; i < w; i++) put_col(x + i, page + p, pgm_read_byte(&data[p * w + i]));
    }
}

static void u8_to_str(uint8_t n, char out[4]) {
    out[0] = n >= 100 ? '0' + n / 100 : ' ';
    out[1] = n >= 10 ? '0' + (n / 10) % 10 : ' ';
    out[2] = '0' + n % 10;
    out[3] = '\0';
}

// ---------------------------------------------------------------------------
// Left screen: status panel
//
//   NAV            QWERTY
//   (2x size)        CAPS
//
//   ⌘cmd  ⌥opt  ⌃ctl  ⇧sft     <- boxed while held
// ---------------------------------------------------------------------------

static void render_status(void) {
    const char *layer_name;
    switch (get_highest_layer(layer_state)) {
        case _NAV:    layer_name = "NAV"; break;
        case _SYM:    layer_name = "SYM"; break;
        case _ADJUST: layer_name = "ADJUST"; break;
        case _CORNE:  layer_name = "CORNE"; break;
        default:      layer_name = "BASE"; break;
    }
    draw_str2x(0, 0, layer_name);

    const char *base = get_highest_layer(default_layer_state) == _COLEMAK ? "COLEMAK" : "QWERTY";
    draw_str(SCREEN_W - (int)strlen(base) * 6, 0, base, false);

    if (host_keyboard_led_state().caps_lock) draw_label(SCREEN_W - 4 * 6 - 2, 1, "CAPS", true);

    static const struct {
        uint8_t mask;
        char    label[5];
    } mods[] = {
        {MOD_MASK_GUI, {G_CMD, 'c', 'm', 'd', 0}},
        {MOD_MASK_ALT, {G_OPT, 'o', 'p', 't', 0}},
        {MOD_MASK_CTRL, {G_CTL, 'c', 't', 'l', 0}},
        {MOD_MASK_SHIFT, {G_SFT, 's', 'f', 't', 0}},
    };
    uint8_t held = get_mods() | get_oneshot_mods();
    for (uint8_t i = 0; i < 4; i++) draw_label(i * 32 + 2, 3, mods[i].label, held & mods[i].mask);
}

// ---------------------------------------------------------------------------
// Right screen: cheat sheets. One cell per key, 4 rows x 12 keys, laid out like
// the keyboard (6 left, gap, 6 right). ' ' = nothing/transparent.
// Keep these in sync with keymap.c when you change a layer.
// ---------------------------------------------------------------------------

typedef struct {
    uint8_t     cells[4][12];
    const char *top_left;  // optional text instead of row 0 (for F-keys)
    const char *top_right;
} cheat_sheet_t;

static const cheat_sheet_t nav_sheet = {{
    {' ', ' ',   ' ',   ' ',   ' ',   ' ',        ' ', ' ',    ' ',    ' ',    ' ',    ' '},
    {' ', ' ',   ' ',   ' ',   ' ',   ' ',        ' ', ' ',    ' ',    ' ',    ' ',    G_BSPC},
    {' ', G_OPT, G_CTL, G_SFT, G_CMD, G_CAPS,     ' ', G_LEFT, G_DOWN, G_UP,   G_RGHT, G_DEL},
    {' ', ' ',   ' ',   ' ',   ' ',   'B' | INV,  ' ', ' ',    G_PGDN, G_PGUP, ' ',    ' '},
}, NULL, NULL};

static const cheat_sheet_t sym_sheet = {{
    {' ',   ' ', ' ', ' ', ' ', ' ',              ' ', ' ', ' ', ' ', ' ', ' '},
    {'`',   '1', '2', '3', '4', '5',              '6', '7', '8', '9', '0', G_BSPC},
    {' ',   '!', '@', '#', '$', '%',              '^', '&', '*', '(', ')', '|'},
    {' ',   '=', '-', '+', '{', '}',              '[', ']', ';', ':', '\\', ' '},
}, "F12 F1-F5", "F6-F11"};

// Boxed letters are "special": Boot, QWERTY, Colemak, OLED mode, c = Corne mode on/off.
// RGB: * on/off, H/h hue, S/s saturation, B/b brightness, >/< effect.
static const cheat_sheet_t adjust_sheet = {{
    {' ',       ' ', ' ',       ' ',       ' ',       ' ',     ' ', ' ',    ' ',    ' ',    ' ', ' '},
    {'!' | INV, ' ', 'Q' | INV, 'C' | INV, 'O' | INV, 'c' | INV, ' ', ' ',    ' ',    ' ',    ' ', ' '},
    {'*',       'H', 'S',       'B',       '>',       ' ',     ' ', '-',    'x',    '+',    ' ', ' '},
    {' ',       'h', 's',       'b',       '<',       ' ',     ' ', G_PREV, 'p',    G_PLAY, ' ', ' '},
}, NULL, NULL};

static int cheat_cell_x(uint8_t col) {
    return col < 6 ? 2 + col * 10 : 70 + (col - 6) * 10;
}

static void render_cheat(uint8_t layer) {
    const cheat_sheet_t *sheet = layer == _SYM ? &sym_sheet : layer == _ADJUST ? &adjust_sheet : &nav_sheet;
    for (uint8_t row = 0; row < 4; row++) {
        if (row == 0 && sheet->top_left) {
            draw_str(2, 0, sheet->top_left, false);
            draw_str(70, 0, sheet->top_right, false);
            continue;
        }
        for (uint8_t col = 0; col < 12; col++) {
            uint8_t c = sheet->cells[row][col];
            if (c == ' ') {
                put_col(cheat_cell_x(col) + 2, row, 0x10); // faint dot marks the key position
                continue;
            }
            bool inverted = c & INV;
            int  x        = cheat_cell_x(col);
            if (inverted) put_col(x - 1, row, 0xFF);
            draw_char(x, row, c & 0x7F, inverted);
        }
    }
}

// ---------------------------------------------------------------------------
// Right screen: Luna (sits < 10 wpm, walks < 40, runs above; sneaks while Ctrl
// is held; barks with Caps Lock on; jumps on Space).
// ---------------------------------------------------------------------------

static void render_luna(void) {
    static uint32_t frame_timer;
    static uint8_t  frame;
    if (timer_elapsed32(frame_timer) > 200) {
        frame_timer = timer_read32();
        frame ^= 1;
    }

    uint8_t        wpm = get_current_wpm();
    const uint8_t *sprite;
    if (host_keyboard_led_state().caps_lock) {
        sprite = luna_bark[frame];
    } else if (get_mods() & MOD_MASK_CTRL) {
        sprite = luna_sneak[frame];
    } else if (wpm <= 10) {
        sprite = luna_sit[frame];
    } else if (wpm <= 40) {
        sprite = luna_walk[frame];
    } else {
        sprite = luna_run[frame];
    }

    bool jumping = timer_elapsed32(jump_timer) < 250;
    blit_P(48, jumping ? 0 : 1, LUNA_W, LUNA_PAGES, sprite);

    char n[4];
    u8_to_str(wpm, n);
    draw_str(SCREEN_W - 7 * 6, 0, n, false);
    draw_str(SCREEN_W - 3 * 6, 0, "wpm", false);
}

// ---------------------------------------------------------------------------
// Right screen: Bongo Cat (idles when slow, taps faster the faster you type).
// ---------------------------------------------------------------------------

static void render_bongo(void) {
    static uint32_t frame_timer;
    static uint8_t  idle_frame, tap_frame;
    uint8_t         wpm = get_current_wpm();
    const uint8_t  *frame;

    if (wpm <= 10) {
        if (timer_elapsed32(frame_timer) > 300) {
            frame_timer = timer_read32();
            idle_frame  = (idle_frame + 1) % 5;
        }
        frame = bongo_idle[idle_frame];
    } else if (wpm < 20) {
        frame = bongo_prep[0];
    } else {
        uint32_t duration = wpm * 5 / 2 < 350 ? 450 - wpm * 5 / 2 : 100;
        if (timer_elapsed32(frame_timer) > duration) {
            frame_timer = timer_read32();
            tap_frame ^= 1;
        }
        frame = bongo_tap[tap_frame];
    }
    blit_P(0, 0, SCREEN_W, PAGES, frame);
}

// ---------------------------------------------------------------------------
// Right screen: WPM number + bar graph of the last ~84 seconds.
// ---------------------------------------------------------------------------

static void render_wpm(void) {
    uint8_t wpm = get_current_wpm(), peak = 0;
    for (uint8_t i = 0; i < WPM_SAMPLES; i++) {
        if (wpm_history[i] > peak) peak = wpm_history[i];
    }

    char n[4];
    draw_str(0, 0, "WPM", false);
    u8_to_str(wpm, n);
    draw_str2x(0, 1, n);
    u8_to_str(peak, n);
    draw_str(0, 3, "max", false);
    draw_str(18, 3, n, false);

    // Graph on the right, scaled to at least 60 wpm tall.
    const int gx = SCREEN_W - WPM_SAMPLES, gh = PAGES * 8;
    uint8_t   scale = peak > 60 ? peak : 60;
    for (uint8_t i = 0; i < WPM_SAMPLES; i++) {
        uint8_t v = wpm_history[(wpm_head + i) % WPM_SAMPLES]; // oldest -> newest
        int     h = (int)v * gh / scale;
        for (int y = 0; y < h; y++) or_pixel(gx + i, gh - 1 - y);
    }
    or_pixel(gx - 2, 0); // little axis ticks top/bottom
    or_pixel(gx - 2, gh - 1);
}

// ---------------------------------------------------------------------------

static void render_right(void) {
    uint8_t layer = get_highest_layer(layer_state);
    if (layer >= _NAV) {
        render_cheat(layer); // holding a layer always shows its cheat sheet
        return;
    }
    switch (sync_state.mode) {
        case MODE_LUNA:  render_luna(); break;
        case MODE_BONGO: render_bongo(); break;
        case MODE_WPM:   render_wpm(); break;
        case MODE_CHEAT: render_cheat((timer_read32() / 3000) % 2 ? _SYM : _NAV); break; // alternate every 3 s
    }
}

bool oled_task_user(void) {
    memset(fb, 0, sizeof(fb));
    if (is_keyboard_left()) {
        render_status();
    } else {
        render_right();
    }
    for (uint16_t i = 0; i < sizeof(fb); i++) oled_write_raw_byte(fb[i], i);
    return false; // skip the keyboard's default logo
}
