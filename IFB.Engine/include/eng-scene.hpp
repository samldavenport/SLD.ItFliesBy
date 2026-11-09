#ifndef ENG_SCENE_HPP
#define ENG_SCENE_HPP

#include "ifb.hpp"

namespace ifb {

    struct scene_mngr;

    struct scene {
        hnd_arena    h_arena;
        hnd_map      h_map; 
        entity_list* entt_list;
        const cchar* parallax_front;
        const cchar* parallax_mid;
        const cchar* parallax_back;
    };

    IFB_INTERNAL scene_mngr* scene_mngr_create       (void);
    IFB_INTERNAL void        scene_mngr_init         (void);

    IFB_INTERNAL hnd_scene   scene_create            (const cchar* name);
    IFB_INTERNAL void        scene_destroy           (const hnd_scene scene);    
    IFB_INTERNAL void        scene_reset             (const hnd_scene scene);    
    IFB_INTERNAL void*       scene_alloc             (const hnd_scene scene, const u32 size);
    IFB_INTERNAL entity_id   scene_create_entity     (const hnd_scene, const cchar* name, const cmpnt_type_u32 cmpnt_mask); 
    IFB_INTERNAL entity_id   scene_get_entity        (const hnd_scene, const cchar* name);
};

#endif //ENG_SCENE_HPP
