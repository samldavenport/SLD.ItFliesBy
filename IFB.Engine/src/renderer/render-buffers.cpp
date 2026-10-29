#pragma once

#include "ifb-config.hpp"
#include "component.hpp"
#include "ifb.hpp"
#include "eng-internal.hpp"
#include "renderer-internal.hpp"
#include "eng-collections.hpp"
#include "entity-internal.hpp"
#include "map-internal.hpp"
#include "particle-internal.hpp"
#include "quad-internal.hpp"
#include "types-internal.hpp"
#include "eng-types.hpp"

namespace ifb {

    static const f32 _grid_vertex_data[] = {
         1.0f,  1.0f, 0.0f, 
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f 
    };

    struct render_vertex_buffer {
        byte* data;
        u32   capacity;
        u32   size;
        u32   stride;

        inline u32
        count(void) {
            assert(stride != 0);
            return(size / stride);
        }
    };

    struct render_element_buffer {
        u32* data;
        u32  capacity;
        u32  size;
        u32  stride;

        inline u32
        count(void) {
            assert(stride != 0);
            return(size / stride);
        }
    };

    struct render_buffers {
        render_vertex_buffer*  grid_vertices;
        render_vertex_buffer*  quad_vertices;
        render_element_buffer* quad_elements;
        render_vertex_buffer*  tile_vertices;
    };

    IFB_INTERNAL render_buffers*
    render_buffers_create(
        void) {

        auto buffers = (render_buffers*)global_alloc(sizeof(render_buffers));
        assert(buffers);

        buffers->grid_vertices =  (render_vertex_buffer*)global_alloc(sizeof(render_vertex_buffer));
        buffers->quad_vertices =  (render_vertex_buffer*)global_alloc(sizeof(render_vertex_buffer));
        buffers->quad_elements = (render_element_buffer*)global_alloc(sizeof(render_element_buffer));
        buffers->tile_vertices =  (render_vertex_buffer*)global_alloc(sizeof(render_vertex_buffer));
       
        assert(buffers->grid_vertices);
        assert(buffers->quad_vertices);
        assert(buffers->quad_elements);
        assert(buffers->tile_vertices);
        
        return(buffers);
    }

    IFB_INTERNAL void
    render_buffers_alloc_and_init(
        render_buffers* buffers) {

        static const auto& cfg = config_instance();

        assert(buffers); 
        assert(buffers->grid_vertices);
        assert(buffers->quad_vertices);
        assert(buffers->quad_elements);
        assert(buffers->tile_vertices);

        //---------------------
        // grid buffer
        //---------------------
        auto* grid_buffer = buffers->grid_vertices;
        grid_buffer->capacity = sizeof(_grid_vertex_data); 
        grid_buffer->size     = sizeof(_grid_vertex_data); 
        grid_buffer->data     = (byte*)_grid_vertex_data;
        grid_buffer->stride   = sizeof(f32) * 3; 
    
        //---------------------
        // quad vtx buffer
        //---------------------

        auto* quad_buf_vtx = buffers->quad_vertices;
        quad_buf_vtx->capacity = cfg.quad_capacity * sizeof(quad_vertices); 
        quad_buf_vtx->size     = 0;
        quad_buf_vtx->stride   = sizeof(quad_vertex);
        quad_buf_vtx->data     = (byte*)renderer_memory_alloc(quad_buf_vtx->capacity);
        assert(quad_buf_vtx->data != NULL);

        //---------------------
        // quad elmnt buffer
        //---------------------

        auto* quad_buf_elmnt = buffers->quad_elements;
        quad_buf_elmnt->capacity = cfg.quad_capacity * QUAD_ELEMENT_DATA_SIZE;
        quad_buf_elmnt->size     = 0;
        quad_buf_elmnt->stride   = sizeof(u32);
        quad_buf_elmnt->data     = (u32*)renderer_memory_alloc(quad_buf_elmnt->capacity);
        assert(quad_buf_elmnt->data != NULL);
        for (
            u32 elmnt_index = 0;
                elmnt_index < cfg.quad_capacity;
              ++elmnt_index) {

            const u32 offset   = (elmnt_index * QUAD_VERTEX_COUNT);
            auto*     elements = &quad_buf_elmnt->data[elmnt_index * QUAD_ELEMENT_COUNT];
            elements[0] = (offset);  
            elements[1] = (offset + 1); 
            elements[2] = (offset + 3); 
            elements[3] = (offset + 1); 
            elements[4] = (offset + 2); 
            elements[5] = (offset + 3); 
        }

        //---------------------
        // tile vertex buffer 
        //---------------------

        auto* tile_buf_vtx = buffers->tile_vertices;
        tile_buf_vtx->capacity = cfg.map_render_buffer_size;
        tile_buf_vtx->size     = 0;
        tile_buf_vtx->stride   = sizeof(map_tile);
        tile_buf_vtx->data     = (byte*)renderer_memory_alloc(tile_buf_vtx->capacity);
        assert(tile_buf_vtx->data != NULL);
    }
    
    IFB_INTERNAL void
    render_buffers_render_quads(
        render_buffers*  buffers,
        render_lists*    lists,
        renderer_camera* camera) {
    
        assert(buffers);

        // validate the vertex buffer
        auto* quad_buf_vtx   = buffers->quad_vertices;
        assert(quad_buf_vtx);
        assert(quad_buf_vtx->capacity != 0);
        assert(quad_buf_vtx->size     <= quad_buf_vtx->capacity);
        assert(quad_buf_vtx->stride   != 0);
        assert(quad_buf_vtx->data     != NULL);
       
        // validate the element buffer
        auto* quad_buf_elmnt = buffers->quad_elements;
        assert(quad_buf_elmnt);
        assert(quad_buf_elmnt->capacity != 0);
        assert(quad_buf_elmnt->data     != NULL);

        // reset the buffers
        quad_buf_vtx->size   = 0;
        quad_buf_elmnt->size = 0;
   
        // get the quad list
        const entity_list* quad_list = render_lists_get_quads(lists); 
        assert(quad_list); 

        // get the camera orientation
        orientation cam_ori;
        renderer_camera_get_orientation(camera, cam_ori);

        // the depth test always passes, so whatever renders last is in front
        // the quads go first so the particles are on top of them
        if (entity_list_count(quad_list) != 0) {
            quad_buf_vtx->size = quad_render_buffer(
                quad_buf_vtx->data,
                quad_buf_vtx->capacity,
                quad_list,
                cam_ori
            );
        }

        // the particles are quads too, they render into what is left of the buffer
        // if the buffer is full, the rest of the particles aren't rendered
        const entity_list* emitter_list  = render_lists_get_particle_emitters(lists);
        const u32          emitter_count = entity_list_count(emitter_list);
        for (
            u32 emitter_index = 0;
                emitter_index < emitter_count;
              ++emitter_index) {

            entity e;
            const entity_id emitter_id = entity_list_index(emitter_list, emitter_index);
            if (!entity_lookup_by_id(e, emitter_id)) continue;

            cmpnt_particle_emitter emitter;
            cmpnt_lookup_particle_emitter(e.index_sparse, emitter);
            if (emitter.hnd == INVALID_HANDLE)                continue;
            if (quad_buf_vtx->size == quad_buf_vtx->capacity) break;

            quad_buf_vtx->size += particle_emitter_render_quads(
                emitter.hnd,
                &quad_buf_vtx->data[quad_buf_vtx->size],
                quad_buf_vtx->capacity - quad_buf_vtx->size,
                cam_ori
            );
        }

        // if nothing was rendered, return
        if (quad_buf_vtx->size == 0) return;

        // update the element count
        const u32 quad_count = quad_buf_vtx->count() / QUAD_VERTEX_COUNT;
        quad_buf_elmnt->size = quad_count * QUAD_ELEMENT_DATA_SIZE;
        assert(quad_buf_elmnt->size <= quad_buf_elmnt->capacity);
}

    IFB_INTERNAL void
    render_buffers_render_tiles(
        render_buffers* buffers,
        render_lists*   lists) {

        assert(buffers);

        // validate the vertex buffer
        auto* tile_buf_vtx = buffers->tile_vertices;
        assert(tile_buf_vtx);
        assert(tile_buf_vtx->capacity != 0);
        assert(tile_buf_vtx->stride   != 0);
        assert(tile_buf_vtx->data     != NULL);

        // reset the buffer
        tile_buf_vtx->size = 0;

        // if there's no map, there's nothing to render
        const hnd_map h_map = render_lists_get_map(lists);
        if (h_map == INVALID_HANDLE) return;

        // render the tiles
        tile_buf_vtx->size = map_render_buffer(
            tile_buf_vtx->data,
            tile_buf_vtx->capacity,
            h_map
        );
        assert(tile_buf_vtx->size <= tile_buf_vtx->capacity);
    }
};
