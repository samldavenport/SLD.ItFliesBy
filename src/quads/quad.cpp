#pragma once

#include "quad.hpp"
#include "entity.cpp"
#include "ifb-collections.hpp"
#include "ifb-entities.hpp"
#include "component.hpp"
#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "particle-emitter.cpp"
#include "sprite-mngr.cpp"

namespace ifb {

    IFB_INTERNAL bool 
    quad_lookup_by_id(
        quad_entity&    q,
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);

        entity e;
        const bool found_entity = (
            entity_lookup_by_id(e, id) &&
            e.archetype.has_all(ENTITY_ARCHETYPE_SPRITE)
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
        cmpnt_lookup_sprite   (q.index_sparse, q.sprite);

        q.dims.width  = q_.width;
        q.dims.height = q_.height;

        return(true);
    }

    IFB_INTERNAL void
    quad_update(
	const quad_entity& q) {

        assert(
            q.archetype.has_all(ENTITY_ARCHETYPE_SPRITE) &&
            q.id  != ENTITY_ID_INVALID                 &&
            q.tag != NULL 
        );

        cmpnt_quad q_;
        q_.width  = q.dims.width;
        q_.height = q.dims.height;
        cmpnt_update_position (q.index_sparse, q.pos);
        cmpnt_update_color    (q.index_sparse, q.color);
        cmpnt_update_quad     (q.index_sparse, q_);
        cmpnt_update_sprite   (q.index_sparse, q.sprite);
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
            e.archetype.has_all(ENTITY_ARCHETYPE_SPRITE)
        );

        return(does_exist);
    }

    // distance from the camera plane to the quad's center, farther is larger
    IFB_INTERNAL f32
    quad_depth(
        const quad_vertices& vertices,
        const orientation&   camera_orientation) {

        const vec3 center = {
            (vertices.top_right.position.x + vertices.bottom_left.position.x) * 0.5f,
            (vertices.top_right.position.y + vertices.bottom_left.position.y) * 0.5f,
            (vertices.top_right.position.z + vertices.bottom_left.position.z) * 0.5f
        };

        const f32 depth = vec3_dot(center, camera_orientation.forward);
        return(depth);
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

        // the depth test always passes, so the quads are sorted back to front
        // the order barely changes between frames, so an insertion sort is cheap
        for (
            u32 sort_index = 1;
                sort_index < vertex_index;
              ++sort_index) {

            const quad_vertices sort_quad  = vertex_buffer[sort_index];
            const f32           sort_depth = quad_depth(sort_quad, camera_orientation);

            u32 insert_index = sort_index;
            while (
                insert_index > 0 &&
                quad_depth(vertex_buffer[insert_index - 1], camera_orientation) < sort_depth) {

                vertex_buffer[insert_index] = vertex_buffer[insert_index - 1];
                --insert_index;
            }
            vertex_buffer[insert_index] = sort_quad;
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

            // quads are centered on their position, but pivot around the bottom edge
            // so the base stays on the ground and only the top tilts with the camera
            const vec3 center = {
                q.pos.x + up.x,
                q.pos.y + up.y - offset_height,
                q.pos.z + up.z
            };

            sprite_info info;
            const bool did_get_sprite_info = sprite_mngr_get_sprite_info_by_index(
                q.sprite.h_atlas,
                q.sprite.i_sprite,
                info
            );

            // the sprite's rect in the atlas, normalized
            // the atlas isn't flipped on upload, so the top row is v = 0
            // with no sprite, the quad is a solid color
            const f32 textured = did_get_sprite_info ? 1.0f : 0.0f;
            f32 u_left   = 0.0f;
            f32 u_right  = 1.0f;
            f32 v_top    = 0.0f;
            f32 v_bottom = 1.0f;
            if (did_get_sprite_info) {
                const f32 atlas_width  = (f32)info.atlas_width;
                const f32 atlas_height = (f32)info.atlas_height;
                u_left   = (f32)(info.pixel_x)                     / atlas_width;
                u_right  = (f32)(info.pixel_x + info.pixel_width)  / atlas_width;
                v_top    = (f32)(info.pixel_y)                     / atlas_height;
                v_bottom = (f32)(info.pixel_y + info.pixel_height) / atlas_height;
            }

            // top right
            vertices.top_right.position.x    = center.x + right.x + up.x;
            vertices.top_right.position.y    = center.y + right.y + up.y;
            vertices.top_right.position.z    = center.z + right.z + up.z;
            vertices.top_right.color.r       = color.r;
            vertices.top_right.color.g       = color.g;
            vertices.top_right.color.b       = color.b;
            vertices.top_right.color.a       = color.a;
            vertices.top_right.text_coords.x = u_right;
            vertices.top_right.text_coords.y = v_top;
            vertices.top_right.textured      = textured;

            // bottom right
            vertices.bottom_right.position.x = center.x + right.x - up.x;
            vertices.bottom_right.position.y = center.y + right.y - up.y;
            vertices.bottom_right.position.z = center.z + right.z - up.z;
            vertices.bottom_right.color.r    = color.r;
            vertices.bottom_right.color.g    = color.g;
            vertices.bottom_right.color.b    = color.b;
            vertices.bottom_right.color.a    = color.a;
            vertices.bottom_right.text_coords.x = u_right;
            vertices.bottom_right.text_coords.y = v_bottom;
            vertices.bottom_right.textured      = textured;

            // bottom left
            vertices.bottom_left.position.x  = center.x - right.x - up.x;
            vertices.bottom_left.position.y  = center.y - right.y - up.y;
            vertices.bottom_left.position.z  = center.z - right.z - up.z;
            vertices.bottom_left.color.r     = color.r;
            vertices.bottom_left.color.g     = color.g;
            vertices.bottom_left.color.b     = color.b;
            vertices.bottom_left.color.a     = color.a;
            vertices.bottom_left.text_coords.x  = u_left;
            vertices.bottom_left.text_coords.y  = v_bottom;
            vertices.bottom_left.textured       = textured;

            // top left
            vertices.top_left.position.x     = center.x - right.x + up.x;
            vertices.top_left.position.y     = center.y - right.y + up.y;
            vertices.top_left.position.z     = center.z - right.z + up.z;
            vertices.top_left.color.r        = color.r;
            vertices.top_left.color.g        = color.g;
            vertices.top_left.color.b        = color.b;
            vertices.top_left.color.a        = color.a;
            vertices.top_left.text_coords.x     = u_left;
            vertices.top_left.text_coords.y     = v_top;
            vertices.top_left.textured          = textured;
        }

        return(quad_is_valid);
    }
};
