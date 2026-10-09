#pragma once

#include "quad.hpp"
#include "entity.cpp"
#include "ifb-collections.hpp"
#include "ifb-entity.hpp"
#include "component.hpp"
#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "particle-emitter.cpp"

namespace ifb {

    IFB_INTERNAL bool 
    quad_lookup_by_id(
        quad_entity&    q,
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);

        entity e;
        const bool found_entity = (
            entity_lookup_by_id(e, id) &&
            e.archetype.has_all(ENTITY_ARCHETYPE_QUAD)
        );

        if (!found_entity) {
            return(false);
        }

        q.tag          = e.tag;
        q.id           = e.id;
        q.archetype    = e.archetype;
        q.index_sparse = e.index_sparse;
        q.index_dense  = e.index_dense;

        cmpnt_quad q_;
        cmpnt_lookup_position (q.index_sparse, q.pos);
        cmpnt_lookup_color    (q.index_sparse, q.color);
        cmpnt_lookup_quad     (q.index_sparse, q_);

        q.dims.width  = q_.width;
        q.dims.height = q_.height;

        return(true);
    }

    IFB_INTERNAL void
    quad_update(
	const quad_entity& q) {

        assert(
            q.archetype.has_all(ENTITY_ARCHETYPE_QUAD) &&
            q.id  != ENTITY_ID_INVALID                 &&
            q.tag != NULL 
        );

        cmpnt_quad q_;
        q_.width  = q.dims.width;
        q_.height = q.dims.height;
        cmpnt_update_position (q.index_sparse, q.pos);
        cmpnt_update_color    (q.index_sparse, q.color);
        cmpnt_update_quad     (q.index_sparse, q_);
    }

    IFB_INTERNAL void
    quad_update(
        const entity_id   id,
        const cmpnt_quad& q) {

        assert(id != ENTITY_ID_INVALID);

        const u32 sparse_index = entity_lookup_sparse_index(id);
        assert(sparse_index != INVALID_INDEX);
        
        cmpnt_update_quad     (sparse_index, q);
    }

    IFB_INTERNAL bool
    quad_does_exist(
        const entity_id id) {

        entity e;
        const bool does_exist = (
            entity_lookup_by_id(e, id) &&
            e.archetype.has_all(ENTITY_ARCHETYPE_QUAD)
        );

        return(does_exist);
    }

    IFB_INTERNAL u32
    quad_render_buffer(
        byte*               buffer_data,
        const u32           buffer_size,
        const entity_list*  quad_list,
        const orientation&  camera_orientation) {

        assert(buffer_data != NULL);
        assert(buffer_size != 0);
        assert(quad_list   != NULL);

        // make sure we have entities to render
        const u32 count_entities = entity_list_count(quad_list);  
        if (count_entities == 0) return(0);

        // calculate the size needed to render the buffer,
        // and the size we actually have
        const u32 count_vertices = count_entities * 4;
        const u32 render_size    = count_vertices * sizeof(quad_vertex); 
        assert(buffer_size >= render_size);

        // render the quads
        u32  vertex_index = 0;
        auto vertex_buffer   = (quad_vertices*)buffer_data;
        for (
            u32 quad_index = 0;
                quad_index < count_entities;
              ++quad_index) {

            const entity_id quad_id       = entity_list_index(quad_list, quad_index);  
            quad_vertices&  quad_vertices = vertex_buffer[vertex_index];
            if (quad_render(quad_vertices, quad_id, camera_orientation)) {
                vertex_index++;
            }
        }

        return(render_size);
    }
    
    IFB_INTERNAL bool 
    quad_render(
        quad_vertices&     vertices,
        const entity_id    id,
        const orientation& camera_orientation) {

        assert(id != INVALID_ID);

        quad_entity q;
        const bool quad_is_valid = quad_lookup_by_id(q, id); 
        if (quad_is_valid) {

            const color_rgba_f32 color         = color_rgba_f32(q.color.hex);
            const f32            offset_width  = q.dims.width  * 0.5f;
            const f32            offset_height = q.dims.height * 0.5f;

            // Keep the quad's horizontal axis locked to world X.
            const vec3 quad_right = { 1.0f, 0.0f, 0.0f };

            // Only consider the camera's pitch.
            // Remove the X component so camera yaw has no effect.
            vec3 pitch_forward = {
                0.0f,
                camera_orientation.forward.y,
                camera_orientation.forward.z
            };

            pitch_forward = vec3_normalize(pitch_forward);

            // Construct an up vector perpendicular to the pitch direction.
            const vec3 quad_up = {
                0.0f,
               -pitch_forward.z,
                pitch_forward.y
            };

            const vec3 right = vec3_scalar_multiply(quad_right, offset_width);
            const vec3 up    = vec3_scalar_multiply(quad_up,    offset_height);

            // top right
            vertices.top_right.position.x    = q.pos.x + right.x + up.x;
            vertices.top_right.position.y    = q.pos.y + right.y + up.y;
            vertices.top_right.position.z    = q.pos.z + right.z + up.z;
            vertices.top_right.color.r       = color.r;
            vertices.top_right.color.g       = color.g;
            vertices.top_right.color.b       = color.b;
            vertices.top_right.color.a       = color.a;

            // bottom right
            vertices.bottom_right.position.x = q.pos.x + right.x - up.x;
            vertices.bottom_right.position.y = q.pos.y + right.y - up.y;
            vertices.bottom_right.position.z = q.pos.z + right.z - up.z;
            vertices.bottom_right.color.r    = color.r;
            vertices.bottom_right.color.g    = color.g;
            vertices.bottom_right.color.b    = color.b;
            vertices.bottom_right.color.a    = color.a;

            // bottom left
            vertices.bottom_left.position.x  = q.pos.x - right.x - up.x;
            vertices.bottom_left.position.y  = q.pos.y - right.y - up.y;
            vertices.bottom_left.position.z  = q.pos.z - right.z - up.z;
            vertices.bottom_left.color.r     = color.r;
            vertices.bottom_left.color.g     = color.g;
            vertices.bottom_left.color.b     = color.b;
            vertices.bottom_left.color.a     = color.a;

            // top left
            vertices.top_left.position.x     = q.pos.x - right.x + up.x;
            vertices.top_left.position.y     = q.pos.y - right.y + up.y;
            vertices.top_left.position.z     = q.pos.z - right.z + up.z;
            vertices.top_left.color.r        = color.r;
            vertices.top_left.color.g        = color.g;
            vertices.top_left.color.b        = color.b;
            vertices.top_left.color.a        = color.a;
        }

        return(quad_is_valid);
    }
};
