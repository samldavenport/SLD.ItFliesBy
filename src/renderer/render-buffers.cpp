#pragma once

#include "ifb-config.hpp"
#include "quad.hpp"
#include "renderer.hpp"
#include "eng-internal.hpp"
#include "renderer-internal.hpp"

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
        quad_buf_vtx->data     = (byte*)renderer_context_memory_alloc(quad_buf_vtx->capacity);
        assert(quad_buf_vtx->data != NULL);

        //---------------------
        // quad elmnt buffer
        //---------------------

        auto* quad_buf_elmnt = buffers->quad_elements;
        quad_buf_elmnt->capacity = cfg.quad_capacity * sizeof(u32) * 6; 
        quad_buf_elmnt->size     = 0; 
        quad_buf_elmnt->stride   = 6; 
        quad_buf_elmnt->data     = (u32*)renderer_context_memory_alloc(quad_buf_elmnt->capacity);
        assert(quad_buf_elmnt->data != NULL);
        for (
            u32 elmnt_index = 0;
                elmnt_index < cfg.quad_capacity;
              ++elmnt_index) {

            const u32 offset   = (elmnt_index * 4);
            auto*     elements = &quad_buf_elmnt->data[elmnt_index];
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
        tile_buf_vtx->capacity = cfg.map_tile_capacity;
        tile_buf_vtx->size     = 0;
        tile_buf_vtx->stride   = sizeof(u32);
        tile_buf_vtx->data     = (byte*)renderer_context_memory_alloc(tile_buf_vtx->capacity);
        assert(tile_buf_vtx->data != NULL);
    }
    
    IFB_INTERNAL void
    render_buffers_render_quads(
        render_buffers* buffers,
        render_lists*   lists) {
    
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
        renderer_camera_get_orientation(cam_ori);

        // render the quads
        // if nothing was rendered, return
        quad_buf_vtx->size = quad_render_buffer(
            quad_buf_vtx->data,
            quad_buf_vtx->capacity,
            quad_list,
            cam_ori
        );
        if (quad_buf_vtx->size == 0) return;

        // update the element count
        const u32 quad_count = quad_buf_vtx->count();
        quad_buf_elmnt->size = quad_count * quad_buf_elmnt->stride;
        assert(quad_buf_elmnt->size <= quad_buf_elmnt->capacity);
        assert(quad_count           == quad_buf_elmnt->count());
    }

    IFB_INTERNAL void
    render_buffers_render_tiles(
        render_buffers* buffers,
        render_lists*   lists) {

    }
};
