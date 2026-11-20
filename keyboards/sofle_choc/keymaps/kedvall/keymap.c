// Copyright 2026 Kanyon
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Starting point: the Sofle Choc build-guide default layout (macOS flavour),
// plus a Colemak base layer. Red layer = NAV (left thumb), blue layer = SYM
// (right thumb), hold both = ADJUST.
#include QMK_KEYBOARD_H

enum layers {
    _QWERTY,
    _COLEMAK,
    _CORNE,  // optional overlay on either base: Corne-size practice (toggle on ADJUST)
    _NAV,    // red
    _SYM,    // blue
    _ADJUST, // NAV + SYM held together
};

enum custom_keycodes {
    OLED_NXT = QK_USER, // cycle the right screen: Luna -> Bongo Cat -> WPM -> cheat sheet
    CORNE,              // Corne mode on/off (saved, so it survives unplugging)
};

#include "oled.c" // screens; uses the layer names above

// Inner thumb keys: tap for Enter / Space, hold for the layer.
#define NAV_ENT LT(_NAV, KC_ENT)
#define SYM_SPC LT(_SYM, KC_SPC)
#define QWERTY  PDF(_QWERTY)   // persistent default layer (saved to EEPROM)
#define COLEMAK PDF(_COLEMAK)

#define TMUX    C(KC_B)       // tmux prefix (Ctrl-B)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 * QWERTY
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |  `   |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |  `   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | ESC  |   Q  |   W  |   E  |   R  |   T  |                    |   Y  |   U  |   I  |   O  |   P  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   A  |   S  |   D  |   F  |   G  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|  Play |    | Play  |------+------+------+------+------+------|
 * |LShift|   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            | Ctrl | Opt  | Cmd  |NAV/Ent| /Enter /       \Space \  |SYM/Spc| Cmd | Opt  | Ctrl |
 *            `----------------------------------'           '------''---------------------------'
 */
[_QWERTY] = LAYOUT(
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                        KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_GRV,
    KC_ESC,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                        KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC,
    KC_TAB,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                        KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_MPLY,   KC_MPLY, KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
                      KC_LCTL, KC_LOPT, KC_LCMD, NAV_ENT, KC_ENT,    KC_SPC,  SYM_SPC, KC_RCMD, KC_ROPT, KC_RCTL
),
/*
 * COLEMAK
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |  `   |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |  `   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | ESC  |   Q  |   W  |   F  |   P  |   G  |                    |   J  |   L  |   U  |   Y  |   ;  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   A  |   R  |   S  |   T  |   D  |-------.    ,-------|   H  |   N  |   E  |   I  |   O  |  '   |
 * |------+------+------+------+------+------|  Play |    | Play  |------+------+------+------+------+------|
 * |LShift|   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   K  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            | Ctrl | Opt  | Cmd  |NAV/Ent| /Enter /       \Space \  |SYM/Spc| Cmd | Opt  | Ctrl |
 *            `----------------------------------'           '------''---------------------------'
 */
[_COLEMAK] = LAYOUT(
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                        KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_GRV,
    KC_ESC,  KC_Q,    KC_W,    KC_F,    KC_P,    KC_G,                        KC_J,    KC_L,    KC_U,    KC_Y,    KC_SCLN, KC_BSPC,
    KC_TAB,  KC_A,    KC_R,    KC_S,    KC_T,    KC_D,                        KC_H,    KC_N,    KC_E,    KC_I,    KC_O,    KC_QUOT,
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_MPLY,   KC_MPLY, KC_K,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
                      KC_LCTL, KC_LOPT, KC_LCMD, NAV_ENT, KC_ENT,    KC_SPC,  SYM_SPC, KC_RCMD, KC_ROPT, KC_RCTL
),
/*
 * CORNE mode - laid over QWERTY/Colemak to practise on a Corne-size board (3 rows x 6 columns + 3 thumbs per side).
 * Letters come from the base layer underneath; the number row and two outer thumb keys per side are off.
 * Numbers/backtick are on SYM. Knobs still work.
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |  --  |  --  |  --  |  --  |  --  |  --  |                    |  --  |  --  |  --  |  --  |  --  |  --  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |      |      |      |      |      |                    |      |      |      |      |      | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Ctrl |      |      |      |      |      |-------.    ,-------|      |      |      |      |      |  '   |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * |Shift |      |      |      |      |      |-------|    |-------|      |      |      |      |      | Esc  |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            |  --  |  --  | Cmd  |NAV/Ent| / Opt  /       \ Cmd  \  |SYM/Spc| Bspc|  --  |  --  |
 *            `----------------------------------'           '------''---------------------------'
 */
[_CORNE] = LAYOUT(
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    KC_TAB,  _______, _______, _______, _______, _______,                     _______, _______, _______, _______, _______, _______,
    KC_LCTL, _______, _______, _______, _______, _______,                     _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______, _______, KC_ESC,
                      XXXXXXX, XXXXXXX, KC_LCMD, _______, KC_LOPT,   KC_RCMD, _______, KC_BSPC, XXXXXXX, XXXXXXX
),
/*
 * NAV (red) - hold left thumb
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |      |      |      |      |      |      |                    |      |      |      |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Esc  |      |      |      |      |      |                    |      |      |      |      |      | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  | Opt  | Ctrl |Shift | Cmd  | Caps |-------.    ,-------|      | Left | Down |  Up  | Right| Del  |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * |Shift |      |      |      |      | ^B   |-------|    |-------|      |      | PgDn | PgUp |      | Shift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            |      |      |      |      | /       /       \      \  |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */
[_NAV] = LAYOUT(
    _______, _______, _______, _______, _______, _______,                     _______, _______, _______, _______, _______, _______,
    _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, _______,
    _______, KC_LOPT, KC_LCTL, KC_LSFT, KC_LCMD, KC_CAPS,                     KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, XXXXXXX, KC_DEL,
    _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, TMUX,    _______,   _______, XXXXXXX, KC_PGDN, KC_PGUP, XXXXXXX, XXXXXXX, _______,
                      _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______
),
/*
 * SYM (blue) - hold right thumb
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | F12  |  F1  |  F2  |  F3  |  F4  |  F5  |                    |  F6  |  F7  |  F8  |  F9  | F10  | F11  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |  `   |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   !  |   @  |   #  |   $  |   %  |-------.    ,-------|   ^  |   &  |   *  |   (  |   )  |   |  |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * | Shift|  =   |  -   |  +   |   {  |   }  |-------|    |-------|   [  |   ]  |   ;  |   :  |   \  | Shift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            |      |      |      |      | /       /       \      \  |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */
[_SYM] = LAYOUT(
    KC_F12,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                       KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                        KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    _______,
    _______, KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC,                     KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_PIPE,
    _______, KC_EQL,  KC_MINS, KC_PLUS, KC_LCBR, KC_RCBR, _______,   _______, KC_LBRC, KC_RBRC, KC_SCLN, KC_COLN, KC_BSLS, _______,
                      _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______
),
/*
 * ADJUST - hold NAV + SYM
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |      |      |      |      |      |      |                    |      |      |      |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Boot |      |QWERTY|COLMAK| OLED |CORNE |                    |      |      |      |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |RGBTog| Hue+ | Sat+ | Bri+ | Mode+|      |-------.    ,-------|      | Vol- | Mute | Vol+ |      |      |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * |      | Hue- | Sat- | Bri- | Mode-|      |-------|    |-------|      | Prev | Play | Next |      |      |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            |      |      |      |      | /       /       \      \  |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */
[_ADJUST] = LAYOUT(
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    QK_BOOT, XXXXXXX, QWERTY,  COLEMAK, OLED_NXT,CORNE,                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    RM_TOGG, RM_HUEU, RM_SATU, RM_VALU, RM_NEXT, XXXXXXX,                     XXXXXXX, KC_VOLD, KC_MUTE, KC_VOLU, XXXXXXX, XXXXXXX,
    XXXXXXX, RM_HUED, RM_SATD, RM_VALD, RM_PREV, XXXXXXX, _______,   _______, XXXXXXX, KC_MPRV, KC_MPLY, KC_MNXT, XXXXXXX, XXXXXXX,
                      _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______
),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case OLED_NXT:
            if (record->event.pressed) oled_next_mode();
            return false; // handled, nothing to send to the computer
        case CORNE:
            if (record->event.pressed) {
                layer_invert(_CORNE);
                user_config.corne = layer_state_is(_CORNE);
                eeconfig_update_user(user_config.raw);
            }
            return false;
        case KC_SPC:
            if (record->event.pressed) oled_space_pressed();
            return true; // still type the space
        case SYM_SPC:
            if (record->event.pressed && record->tap.count) oled_space_pressed(); // only when tapped
            return true;
    }
    return true;
}

void keyboard_post_init_user(void) {
    oled_post_init_user(); // also loads user_config from EEPROM
    if (user_config.corne) layer_on(_CORNE);
}

void housekeeping_task_user(void) {
    oled_housekeeping_user();
}

// Holding both NAV and SYM turns on ADJUST.
layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, _NAV, _SYM, _ADJUST);
}

#if defined(ENCODER_MAP_ENABLE)
// Left knob: volume. Right knob: scroll like a mouse wheel; hold NAV to make it
// prev/next track instead. (One entry per layer required.)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_QWERTY]  = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(MS_WHLU, MS_WHLD) },
    [_COLEMAK] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(MS_WHLU, MS_WHLD) },
    [_CORNE]   = { ENCODER_CCW_CW(_______, _______), ENCODER_CCW_CW(_______, _______) },
    [_NAV]     = { ENCODER_CCW_CW(_______, _______), ENCODER_CCW_CW(KC_MPRV, KC_MNXT) },
    [_SYM]     = { ENCODER_CCW_CW(_______, _______), ENCODER_CCW_CW(_______, _______) },
    [_ADJUST]  = { ENCODER_CCW_CW(RM_VALD, RM_VALU), ENCODER_CCW_CW(RM_SPDD, RM_SPDU) },
};
#endif
