#pragma once

#include "win32.hpp"

namespace ifb {
    
    IFB_WIN32_INTERNAL input_keycode
    win32_input_get_keycode(
        const DWORD wparam) {

        constexpr input_keycode keycode_map[256] = {
            input_keycode_e_null,          // 0x00
            input_keycode_e_null,          // 0x01
            input_keycode_e_null,          // 0x02
            input_keycode_e_null,          // 0x03
            input_keycode_e_null,          // 0x04
            input_keycode_e_null,          // 0x05
            input_keycode_e_null,          // 0x06
            input_keycode_e_null,          // 0x07
            input_keycode_e_backspace,     // 0x08 VK_BACK
            input_keycode_e_tab,           // 0x09 VK_TAB
            input_keycode_e_null,          // 0x0A
            input_keycode_e_null,          // 0x0B
            input_keycode_e_null,          // 0x0C VK_CLEAR
            input_keycode_e_enter,         // 0x0D VK_RETURN
            input_keycode_e_null,          // 0x0E
            input_keycode_e_null,          // 0x0F
            input_keycode_e_null,          // 0x10 VK_SHIFT   
            input_keycode_e_null,          // 0x11 VK_CONTROL 
            input_keycode_e_null,          // 0x12 VK_MENU    
            input_keycode_e_null,          // 0x13 VK_PAUSE   
            input_keycode_e_caps_lock,     // 0x14 VK_CAPITAL 
            input_keycode_e_null,          // 0x15
            input_keycode_e_null,          // 0x16
            input_keycode_e_null,          // 0x17
            input_keycode_e_null,          // 0x18
            input_keycode_e_null,          // 0x19
            input_keycode_e_null,          // 0x1A
            input_keycode_e_escape,        // 0x1B VK_ESCAPE
            input_keycode_e_null,          // 0x1C
            input_keycode_e_null,          // 0x1D
            input_keycode_e_null,          // 0x1E
            input_keycode_e_null,          // 0x1F
            input_keycode_e_space,         // 0x20 VK_SPACE
            input_keycode_e_page_up,       // 0x21 VK_PRIOR
            input_keycode_e_page_down,     // 0x22 VK_NEXT
            input_keycode_e_end,           // 0x23 VK_END
            input_keycode_e_home,          // 0x24 VK_HOME
            input_keycode_e_arrow_left,    // 0x25 VK_LEFT
            input_keycode_e_arrow_up,      // 0x26 VK_UP
            input_keycode_e_arrow_right,   // 0x27 VK_RIGHT
            input_keycode_e_arrow_down,    // 0x28 VK_DOWN
            input_keycode_e_null,          // 0x29 VK_SELECT
            input_keycode_e_null,          // 0x2A VK_PRINT
            input_keycode_e_null,          // 0x2B VK_EXECUTE
            input_keycode_e_null,          // 0x2C VK_SNAPSHOT
            input_keycode_e_insert,        // 0x2D VK_INSERT
            input_keycode_e_delete,        // 0x2E VK_DELETE
            input_keycode_e_null,          // 0x2F VK_HELP
            input_keycode_e_0,             // 0x30 '0'
            input_keycode_e_1,             // 0x31 '1'
            input_keycode_e_2,             // 0x32 '2'
            input_keycode_e_3,             // 0x33 '3'
            input_keycode_e_4,             // 0x34 '4'
            input_keycode_e_5,             // 0x35 '5'
            input_keycode_e_6,             // 0x36 '6'
            input_keycode_e_7,             // 0x37 '7'
            input_keycode_e_8,             // 0x38 '8'
            input_keycode_e_9,             // 0x39 '9'
            input_keycode_e_null,          // 0x3A
            input_keycode_e_null,          // 0x3B
            input_keycode_e_null,          // 0x3C
            input_keycode_e_null,          // 0x3D
            input_keycode_e_null,          // 0x3E
            input_keycode_e_null,          // 0x3F
            input_keycode_e_null,          // 0x40
            input_keycode_e_a,             // 0x41 'A'
            input_keycode_e_b,             // 0x42 'B'
            input_keycode_e_c,             // 0x43 'C'
            input_keycode_e_d,             // 0x44 'D'
            input_keycode_e_e,             // 0x45 'E'
            input_keycode_e_f,             // 0x46 'F'
            input_keycode_e_g,             // 0x47 'G'
            input_keycode_e_h,             // 0x48 'H'
            input_keycode_e_i,             // 0x49 'I'
            input_keycode_e_j,             // 0x4A 'J'
            input_keycode_e_k,             // 0x4B 'K'
            input_keycode_e_l,             // 0x4C 'L'
            input_keycode_e_m,             // 0x4D 'M'
            input_keycode_e_n,             // 0x4E 'N'
            input_keycode_e_o,             // 0x4F 'O'
            input_keycode_e_p,             // 0x50 'P'
            input_keycode_e_q,             // 0x51 'Q'
            input_keycode_e_r,             // 0x52 'R'
            input_keycode_e_s,             // 0x53 'S'
            input_keycode_e_t,             // 0x54 'T'
            input_keycode_e_u,             // 0x55 'U'
            input_keycode_e_v,             // 0x56 'V'
            input_keycode_e_w,             // 0x57 'W'
            input_keycode_e_x,             // 0x58 'X'
            input_keycode_e_y,             // 0x59 'Y'
            input_keycode_e_z,             // 0x5A 'Z'
            input_keycode_e_super_left,    // 0x5B VK_LWIN 
            input_keycode_e_super_right,   // 0x5C VK_RWIN 
            input_keycode_e_menu,          // 0x5D VK_APPS 
            input_keycode_e_null,          // 0x5E
            input_keycode_e_null,          // 0x5F VK_SLEEP
            input_keycode_e_kp_0,          // 0x60 VK_NUMPAD0
            input_keycode_e_kp_1,          // 0x61 VK_NUMPAD1
            input_keycode_e_kp_2,          // 0x62 VK_NUMPAD2
            input_keycode_e_kp_3,          // 0x63 VK_NUMPAD3
            input_keycode_e_kp_4,          // 0x64 VK_NUMPAD4
            input_keycode_e_kp_5,          // 0x65 VK_NUMPAD5
            input_keycode_e_kp_6,          // 0x66 VK_NUMPAD6
            input_keycode_e_kp_7,          // 0x67 VK_NUMPAD7
            input_keycode_e_kp_8,          // 0x68 VK_NUMPAD8
            input_keycode_e_kp_9,          // 0x69 VK_NUMPAD9
            input_keycode_e_kp_multiply,   // 0x6A VK_MULTIPLY
            input_keycode_e_kp_add,        // 0x6B VK_ADD
            input_keycode_e_null,          // 0x6C VK_SEPARATOR
            input_keycode_e_kp_subtract,   // 0x6D VK_SUBTRACT
            input_keycode_e_kp_decimal,    // 0x6E VK_DECIMAL
            input_keycode_e_kp_divide,     // 0x6F VK_DIVIDE
            input_keycode_e_f1,            // 0x70 VK_F1
            input_keycode_e_f2,            // 0x71 VK_F2
            input_keycode_e_f3,            // 0x72 VK_F3
            input_keycode_e_f4,            // 0x73 VK_F4
            input_keycode_e_f5,            // 0x74 VK_F5
            input_keycode_e_f6,            // 0x75 VK_F6
            input_keycode_e_f7,            // 0x76 VK_F7
            input_keycode_e_f8,            // 0x77 VK_F8
            input_keycode_e_f9,            // 0x78 VK_F9
            input_keycode_e_f10,           // 0x79 VK_F10
            input_keycode_e_f11,           // 0x7A VK_F11
            input_keycode_e_f12,           // 0x7B VK_F12
            input_keycode_e_null,          // 0x7C
            input_keycode_e_null,          // 0x7D
            input_keycode_e_null,          // 0x7E
            input_keycode_e_null,          // 0x7F
            input_keycode_e_null,          // 0x80
            input_keycode_e_null,          // 0x81
            input_keycode_e_null,          // 0x82
            input_keycode_e_null,          // 0x83
            input_keycode_e_null,          // 0x84
            input_keycode_e_null,          // 0x85
            input_keycode_e_null,          // 0x86
            input_keycode_e_null,          // 0x87
            input_keycode_e_null,          // 0x88
            input_keycode_e_null,          // 0x89
            input_keycode_e_null,          // 0x8A
            input_keycode_e_null,          // 0x8B
            input_keycode_e_null,          // 0x8C
            input_keycode_e_null,          // 0x8D
            input_keycode_e_null,          // 0x8E
            input_keycode_e_null,          // 0x8F
            input_keycode_e_num_lock,      // 0x90 VK_NUMLOCK
            input_keycode_e_scroll_lock,   // 0x91 VK_SCROLL
            input_keycode_e_null,          // 0x92
            input_keycode_e_null,          // 0x93
            input_keycode_e_null,          // 0x94
            input_keycode_e_null,          // 0x95
            input_keycode_e_null,          // 0x96
            input_keycode_e_null,          // 0x97
            input_keycode_e_null,          // 0x98
            input_keycode_e_null,          // 0x99
            input_keycode_e_null,          // 0x9A
            input_keycode_e_null,          // 0x9B
            input_keycode_e_null,          // 0x9C
            input_keycode_e_null,          // 0x9D
            input_keycode_e_null,          // 0x9E
            input_keycode_e_null,          // 0x9F
            input_keycode_e_shift_left,    // 0xA0 VK_LSHIFT
            input_keycode_e_shift_right,   // 0xA1 VK_RSHIFT
            input_keycode_e_ctrl_left,     // 0xA2 VK_LCONTROL
            input_keycode_e_ctrl_right,    // 0xA3 VK_RCONTROL
            input_keycode_e_alt_left,      // 0xA4 VK_LMENU
            input_keycode_e_alt_right,     // 0xA5 VK_RMENU
            input_keycode_e_null,          // 0xA6 VK_BROWSER_BACK
            input_keycode_e_null,          // 0xA7 VK_BROWSER_FORWARD
            input_keycode_e_null,          // 0xA8 VK_BROWSER_REFRESH
            input_keycode_e_null,          // 0xA9 VK_BROWSER_STOP
            input_keycode_e_null,          // 0xAA VK_BROWSER_SEARCH
            input_keycode_e_null,          // 0xAB VK_BROWSER_FAVES
            input_keycode_e_null,          // 0xAC VK_BROWSER_HOME
            input_keycode_e_volume_mute,   // 0xAD VK_VOLUME_MUTE
            input_keycode_e_volume_down,   // 0xAE VK_VOLUME_DOWN
            input_keycode_e_volume_up,     // 0xAF VK_VOLUME_UP
            input_keycode_e_media_next,    // 0xB0 VK_MEDIA_NEXT
            input_keycode_e_media_prev,    // 0xB1 VK_MEDIA_PREV
            input_keycode_e_media_stop,    // 0xB2 VK_MEDIA_STOP
            input_keycode_e_media_play,    // 0xB3 VK_MEDIA_PLAY_PAUSE
            input_keycode_e_null,          // 0xB4 VK_LAUNCH_MAIL
            input_keycode_e_null,          // 0xB5 VK_LAUNCH_MEDIA
            input_keycode_e_null,          // 0xB6 VK_LAUNCH_APP1
            input_keycode_e_null,          // 0xB7 VK_LAUNCH_APP2
            input_keycode_e_null,          // 0xB8
            input_keycode_e_null,          // 0xB9
            input_keycode_e_semicolon,     // 0xBA VK_OEM_1 (;:) 
            input_keycode_e_equals,        // 0xBB VK_OEM_PLUS (=+) 
            input_keycode_e_comma,         // 0xBC VK_OEM_COMMA (,<) 
            input_keycode_e_minus,         // 0xBD VK_OEM_MINUS (-_) 
            input_keycode_e_period,        // 0xBE VK_OEM_PERIOD (.>) 
            input_keycode_e_slash,         // 0xBF VK_OEM_2 (/?) 
            input_keycode_e_grave,         // 0xC0 VK_OEM_3 (`~) 
            input_keycode_e_null,          // 0xC1
            input_keycode_e_null,          // 0xC2
            input_keycode_e_null,          // 0xC3
            input_keycode_e_null,          // 0xC4
            input_keycode_e_null,          // 0xC5
            input_keycode_e_null,          // 0xC6
            input_keycode_e_null,          // 0xC7
            input_keycode_e_null,          // 0xC8
            input_keycode_e_null,          // 0xC9
            input_keycode_e_null,          // 0xCA
            input_keycode_e_null,          // 0xCB
            input_keycode_e_null,          // 0xCC
            input_keycode_e_null,          // 0xCD
            input_keycode_e_null,          // 0xCE
            input_keycode_e_null,          // 0xCF
            input_keycode_e_null,          // 0xD0
            input_keycode_e_null,          // 0xD1
            input_keycode_e_null,          // 0xD2
            input_keycode_e_null,          // 0xD3
            input_keycode_e_null,          // 0xD4
            input_keycode_e_null,          // 0xD5
            input_keycode_e_null,          // 0xD6
            input_keycode_e_null,          // 0xD7
            input_keycode_e_null,          // 0xD8
            input_keycode_e_null,          // 0xD9
            input_keycode_e_null,          // 0xDA
            input_keycode_e_bracket_left,  // 0xDB VK_OEM_4  ([)
            input_keycode_e_backslash,     // 0xDC VK_OEM_5  (\|)
            input_keycode_e_bracket_right, // 0xDD VK_OEM_6  (])
            input_keycode_e_apostrophe,    // 0xDE VK_OEM_7  ('")
            input_keycode_e_null,          // 0xDF VK_OEM_8
            input_keycode_e_null,          // 0xE0
            input_keycode_e_null,          // 0xE1
            input_keycode_e_null,          // 0xE2 VK_OEM_102 (< >)
            input_keycode_e_null,          // 0xE3
            input_keycode_e_null,          // 0xE4
            input_keycode_e_null,          // 0xE5 VK_PROCESSKEY
            input_keycode_e_null,          // 0xE6
            input_keycode_e_null,          // 0xE7 VK_PACKET
            input_keycode_e_null,          // 0xE8
            input_keycode_e_null,          // 0xE9
            input_keycode_e_null,          // 0xEA
            input_keycode_e_null,          // 0xEB
            input_keycode_e_null,          // 0xEC
            input_keycode_e_null,          // 0xED
            input_keycode_e_null,          // 0xEE
            input_keycode_e_null,          // 0xEF
            input_keycode_e_null,          // 0xF0
            input_keycode_e_null,          // 0xF1
            input_keycode_e_null,          // 0xF2
            input_keycode_e_null,          // 0xF3
            input_keycode_e_null,          // 0xF4
            input_keycode_e_null,          // 0xF5
            input_keycode_e_null,          // 0xF6 VK_ATTN
            input_keycode_e_null,          // 0xF7 VK_CRSEL
            input_keycode_e_null,          // 0xF8 VK_EXSEL
            input_keycode_e_null,          // 0xF9 VK_EREOF
            input_keycode_e_null,          // 0xFA VK_PLAY
            input_keycode_e_null,          // 0xFB VK_ZOOM
            input_keycode_e_null,          // 0xFC VK_NONAME
            input_keycode_e_null,          // 0xFD VK_PA1
            input_keycode_e_null,          // 0xFE VK_OEM_CLEAR
            input_keycode_e_null           // 0xFF
        };

        const input_keycode keycode = (wparam < 256)
            ? keycode_map[(DWORD)wparam]
            : input_keycode_e_null;

        return(keycode);
    }

};