#pragma once

#include "entity.cpp"
#include "eng-component.hpp"
#include "ifb-profile.hpp"
#include "ifb.hpp"
#include "memory-arena.cpp"
#include "particle-emitter.cpp"
#include "sprite-mngr.cpp"
#include "ifb-engine.hpp"
#include "eng-entity.hpp"
#include "eng-quad.hpp"

namespace ifb {

    IFB_INTERNAL bool
    quad_lookup(
        quad_entity&  q,
        const entity& e) {

        if (!e.archetype.has_all(ARCHETYPE_SPRITE)) {
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

    IFB_INTERNAL bool
    quad_lookup_by_id(
        quad_entity&    q,
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);

        entity e;
        const bool did_find = (
            entity_lookup_by_id(e, id) &&
            quad_lookup(q, e)
        );
        return(did_find);
    }

    IFB_INTERNAL void
    quad_update(
	const quad_entity& q) {

        assert(
            q.archetype.has_all(ARCHETYPE_SPRITE) &&
            q.id  != ENTITY_ID_INVALID            &&
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

    // distance from the camera plane to the quad's center, farther is larger
    IFB_INTERNAL f32
    quad_depth(
        const quad_vertices& vertices,
        const orientation&   camera_orientation) {

        // no profile zone, this runs once per quad and the zones cost more than the math

        const vec3 center = {
            (vertices.top_right.position.x + vertices.bottom_left.position.x) * 0.5f,
            (vertices.top_right.position.y + vertices.bottom_left.position.y) * 0.5f,
            (vertices.top_right.position.z + vertices.bottom_left.position.z) * 0.5f
        };

        const f32 depth = vec3_dot(center, camera_orientation.forward);
        return(depth);
    }

    struct quad_sort_key {
        u32 key;   // the depth, as bits that sort in the order we draw
        u32 index; // where the quad is in the vertex buffer
    };

    // maps the depth to bits where a smaller value is drawn first
    // a float's bits sort like the float once negatives are flipped,
    // then everything is inverted so the farthest quad comes first
    inline u32
    quad_sort_key_from_depth(
        const f32 depth) {

        union { f32 f; u32 u; } bits;
        bits.f = depth;

        const u32 ascending = (bits.u & 0x80000000u)
            ? ~bits.u
            : (bits.u | 0x80000000u);

        return(~ascending);
    }

    // sorts the quads in the buffer back to front
    // the depths are computed once, then the keys are radix sorted and the quads are moved into place once
    // this used to be an insertion sort over the quads, which was O(n^2) since the buffer is
    // filled in entity order every frame, at 1000 quads that was ~250k steps each recomputing a depth and copying a quad
    // the radix sort is stable, so quads at the same depth keep their entity order like before
    IFB_INTERNAL void
    quad_sort_back_to_front(
        quad_vertices*     vertex_buffer,
        const u32          count,
        const orientation& camera_orientation) {

        IFB_PROFILE_FUNCTION();

        if (count < 2) return;

        const hnd_arena h_arena = arena_alloc();
        assert(h_arena != INVALID_HANDLE);
        if (h_arena == INVALID_HANDLE) return;

        auto* keys     = (quad_sort_key*)arena_push(h_arena, count * sizeof(quad_sort_key));
        auto* keys_tmp = (quad_sort_key*)arena_push(h_arena, count * sizeof(quad_sort_key));
        assert(keys != NULL && keys_tmp != NULL);
        if (keys == NULL || keys_tmp == NULL) {
            arena_free(h_arena);
            return;
        }

        // one depth per quad
        for (
            u32 index = 0;
                index < count;
              ++index) {

            keys[index].key   = quad_sort_key_from_depth(quad_depth(vertex_buffer[index], camera_orientation));
            keys[index].index = index;
        }

        // lsd radix sort, a byte at a time
        // a pass where every key has the same byte doesn't change the order, so it's skipped
        quad_sort_key* src = keys;
        quad_sort_key* dst = keys_tmp;
        for (
            u32 shift = 0;
                shift < 32;
                shift += 8) {

            u32 offsets[256] = {};
            for (
                u32 index = 0;
                    index < count;
                  ++index) {

                ++offsets[(src[index].key >> shift) & 0xFF];
            }

            if (offsets[(src[0].key >> shift) & 0xFF] == count) continue;

            u32 sum = 0;
            for (
                u32 bucket = 0;
                    bucket < 256;
                  ++bucket) {

                const u32 bucket_count = offsets[bucket];
                offsets[bucket] = sum;
                sum += bucket_count;
            }

            for (
                u32 index = 0;
                    index < count;
                  ++index) {

                const u32 bucket = (src[index].key >> shift) & 0xFF;
                dst[offsets[bucket]++] = src[index];
            }

            quad_sort_key* swap = src;
            src = dst;
            dst = swap;
        }

        // src[i].index is the quad that belongs at i, move them into place by following the cycles
        // each quad is copied once, plus one extra copy per cycle
        for (
            u32 index = 0;
                index < count;
              ++index) {

            if (src[index].index == index) continue;

            const quad_vertices first = vertex_buffer[index];
            u32 hole = index;
            for (;;) {
                const u32 from = src[hole].index;
                src[hole].index = hole;
                if (from == index) {
                    vertex_buffer[hole] = first;
                    break;
                }
                vertex_buffer[hole] = vertex_buffer[from];
                hole = from;
            }
        }

        arena_free(h_arena);
    }

    // renders the quad of every entity marked to be drawn this frame
    // returns the size of the vertex data that was written to the buffer
    // if the buffer is full, the rest of the quads aren't rendered
    IFB_INTERNAL u32
    quad_render_buffer(
        byte*               buffer_data,
        const u32           buffer_size,
        const orientation&  camera_orientation) {

        IFB_PROFILE_FUNCTION();

        assert(buffer_data != NULL);

        auto*     vertex_buffer = (quad_vertices*)buffer_data;
        const u32 quad_capacity = buffer_size / sizeof(quad_vertices);
        const u32 entity_count  = entity_mngr_get_count();
        u32       vertex_index  = 0;
        for (
            u32 entity_index = 0;
                entity_index < entity_count && vertex_index < quad_capacity;
              ++entity_index) {

            entity e;
            if (!entity_lookup_by_index_dense(e, entity_index)) continue;
            if (!entity_is_drawn(e))                            continue;

            if (quad_render(vertex_buffer[vertex_index], e, camera_orientation)) {
                vertex_index++;
            }
        }

        // the quads are sorted back to front so their transparent edges blend over the quads behind them
        quad_sort_back_to_front(vertex_buffer, vertex_index, camera_orientation);

        return(vertex_index * sizeof(quad_vertices));
    }

    IFB_INTERNAL bool
    quad_render(
        quad_vertices&     vertices,
        const entity&      e,
        const orientation& camera_orientation) {

        IFB_PROFILE_FUNCTION();

        quad_entity q;
        const bool quad_is_valid = quad_lookup(q, e); 
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
            vertices.top_right.position.x       = center.x + right.x + up.x;
            vertices.top_right.position.y       = center.y + right.y + up.y;
            vertices.top_right.position.z       = center.z + right.z + up.z;
            vertices.top_right.color.r          = color.r;
            vertices.top_right.color.g          = color.g;
            vertices.top_right.color.b          = color.b;
            vertices.top_right.color.a          = color.a;
            vertices.top_right.text_coords.x    = u_right;
            vertices.top_right.text_coords.y    = v_top;
            vertices.top_right.textured         = textured;

            // bottom right
            vertices.bottom_right.position.x    = center.x + right.x - up.x;
            vertices.bottom_right.position.y    = center.y + right.y - up.y;
            vertices.bottom_right.position.z    = center.z + right.z - up.z;
            vertices.bottom_right.color.r       = color.r;
            vertices.bottom_right.color.g       = color.g;
            vertices.bottom_right.color.b       = color.b;
            vertices.bottom_right.color.a       = color.a;
            vertices.bottom_right.text_coords.x = u_right;
            vertices.bottom_right.text_coords.y = v_bottom;
            vertices.bottom_right.textured      = textured;

            // bottom left
            vertices.bottom_left.position.y     = center.y - right.y - up.y;
            vertices.bottom_left.position.x     = center.x - right.x - up.x;
            vertices.bottom_left.position.z     = center.z - right.z - up.z;
            vertices.bottom_left.color.r        = color.r;
            vertices.bottom_left.color.g        = color.g;
            vertices.bottom_left.color.b        = color.b;
            vertices.bottom_left.color.a        = color.a;
            vertices.bottom_left.text_coords.x  = u_left;
            vertices.bottom_left.text_coords.y  = v_bottom;
            vertices.bottom_left.textured       = textured;

            // top left
            vertices.top_left.position.x        = center.x - right.x + up.x;
            vertices.top_left.position.y        = center.y - right.y + up.y;
            vertices.top_left.position.z        = center.z - right.z + up.z;
            vertices.top_left.color.r           = color.r;
            vertices.top_left.color.g           = color.g;
            vertices.top_left.color.b           = color.b;
            vertices.top_left.color.a           = color.a;
            vertices.top_left.text_coords.x     = u_left;
            vertices.top_left.text_coords.y     = v_top;
            vertices.top_left.textured          = textured;
        }

        return(quad_is_valid);
    }
};
