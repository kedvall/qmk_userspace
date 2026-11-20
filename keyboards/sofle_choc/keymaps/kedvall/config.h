/* Copyright 2023 Brian Low
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

// Enabling this option changes the startup behavior to listen for an
// active USB communication to delegate which part is master and which
// is slave. With this option enabled and theres’s USB communication,
// then that half assumes it is the master, otherwise it assumes it
// is the slave.
//
// I've found this helps with some ProMicros where the slave does not boot
#define SPLIT_USB_DETECT

#define RGB_MATRIX_SLEEP     // turn off effects when suspended
#define SPLIT_TRANSPORT_MIRROR             // If LED_MATRIX_KEYPRESSES or LED_MATRIX_KEYRELEASES is enabled, you also will want to enable SPLIT_TRANSPORT_MIRROR
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 100  // limits maximum brightness of LEDs (max 255). Higher may cause the controller to crash.

// ---- Thumb keys: tap for Enter/Space, hold for NAV/SYM ----
// Held longer than this = layer; shorter = Enter/Space.
#define TAPPING_TERM 175
// Also counts as held if another key is pressed AND released while the thumb
// key is still down (e.g. hold SYM, tap 1, release SYM -> "!"), even if quicker
// than TAPPING_TERM. Normal typing rolls (Space released first) stay as Space.
#define PERMISSIVE_HOLD

// ---- OLEDs (see oled.c) ----
// Screens stay on for 30 min after the last keypress. They also switch off
// whenever the Mac sleeps (USB suspend), so this only matters while it's awake.
#define OLED_TIMEOUT 1800000

// Share state with the half that isn't plugged in, so its screen can show it.
#define SPLIT_LAYER_STATE_ENABLE // current layer + QWERTY/Colemak
#define SPLIT_MODS_ENABLE        // held Cmd/Opt/Ctrl/Shift
#define SPLIT_LED_STATE_ENABLE   // Caps Lock
#define SPLIT_WPM_ENABLE         // typing speed (Luna, Bongo Cat, WPM graph)
#define SPLIT_OLED_ENABLE        // both screens wake/sleep together
#define SPLIT_TRANSACTION_IDS_USER USER_SYNC_OLED // our own data: screen mode + Luna jumps

// ---- HW-467AB controller workaround ----
// The clone controller's GP26-GP29 pads are faulty, so those four lines are hand-wired to other GPIOs on both halves.
// Raw GPIO numbers are used because the Pro Micro pin names have no equivalent for them under CONVERT_TO=rp2040_ce.
// Matrix: F6 (GP27) -> GP15, F7 (GP26) -> GP16
#undef MATRIX_COL_PINS
#define MATRIX_COL_PINS { 15U, 16U, B1, B3, B2, B6 }

// Encoders: F5 (GP28) -> GP14, F4 (GP29) -> GP13. The right half's A/B are swapped relative to the left in
// keyboard.json, so it needs its own defines to follow the same rewire.
#undef ENCODER_A_PINS
#undef ENCODER_B_PINS
#undef ENCODER_A_PINS_RIGHT
#undef ENCODER_B_PINS_RIGHT
#define ENCODER_A_PINS { 14U }
#define ENCODER_B_PINS { 13U }
#define ENCODER_A_PINS_RIGHT { 13U }
#define ENCODER_B_PINS_RIGHT { 14U }
