#pragma once

#include "eng-internal.hpp"

#include "eng-stack.cpp"

namespace ifb {

    struct keyboard_input {
        union {
            struct {
                u32 group_1;
                u32 group_2;
                u32 group_3;
                u32 group_4;
            };
            u32 groups[4];
        };
    } static * _keyboard;

    struct keyboard_code {
        union {
            input_keycode keycode;
            struct {
                byte index;
                byte group;
            };
        };
    };
    
    IFB_ENG_INTERNAL keyboard_input*
    keyboard_input_create(
        void) {

        _keyboard = global_alloc<keyboard_input>();
        assert(_keyboard != NULL);
        return(_keyboard);
    }
   
    IFB_ENG_INTERNAL void 
    keyboard_input_reset(
        void) {

        assert(_keyboard != NULL);
        _keyboard->group_1 = 0;
        _keyboard->group_2 = 0;
        _keyboard->group_3 = 0;
    }
    
    IFB_ENG_INTERNAL void 
    keyboard_set_key_up(
        const input_keycode kc) {

        assert(_keyboard != NULL);
   
        keyboard_code code;
        code.keycode = kc;

        assert(code.group <= 4);
        assert(code.index <= 32);

        _keyboard->groups[code.group] &= ~(1 << code.index);
    }
    
    IFB_ENG_INTERNAL void 
    keyboard_set_key_down(
        const input_keycode kc) {

        assert(_keyboard != NULL);
   
        keyboard_code code;
        code.keycode = kc;

        assert(code.group <= 4);
        assert(code.index <= 32);
        
        _keyboard->groups[code.group] |= (1 << code.index);
    }
    
    IFB_ENG_INTERNAL bool
    keyboard_is_key_down(
        const input_keycode kc) {

        assert(_keyboard != NULL);

        keyboard_code code;
        code.keycode = kc;

        assert(code.group <= 4);
        assert(code.index <= 32);

        const bool is_set = (( _keyboard->groups[code.group] >> code.index) & 1) == 1; 
        return(is_set);
    }
};
