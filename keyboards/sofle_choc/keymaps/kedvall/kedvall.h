// Copyright 2026 Kanyon
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Shared between keymap.c and oled.c.
#pragma once
#include QMK_KEYBOARD_H

enum layers {
    _QWERTY,
    _COLEMAK,
    _NAV,    // red
    _SYM,    // blue
    _ADJUST, // NAV + SYM held together
};

enum custom_keycodes {
    OLED_NXT = QK_USER, // cycle the right screen: Luna -> Bongo Cat -> WPM -> cheat sheet
};

#ifdef OLED_ENABLE
void oled_post_init_user(void);    // call from keyboard_post_init_user
void oled_housekeeping_user(void); // call from housekeeping_task_user
void oled_next_mode(void);
void oled_space_pressed(void);     // makes Luna jump
#endif
