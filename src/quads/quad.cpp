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

    inline bool 
    quad_render_buffer_add_quad(
        quad_vertex_buffer& buffer,
        const entity&       e,
        const hnd_arena     h_arena) {
        
        const bool has_quad    = e.archetype.has_any(cmpnt_type_e_quad);
        const bool has_emitter = e.archetype.has_any(cmpnt_type_e_particle_emitter);

          
    }

    inline u32
    quad_list_count_vertices(
        const entity_list* quad_list) {

        const u32              quad_count   = entity_list_count(quad_list);
        u32                    vertex_count = 0;
        entity                 e;
        cmpnt_particle_emitter emitter;
        particle_emitter_info  emitter_info;
        for (
            u32 quad_index = 0;
                quad_index < quad_count;
              ++quad_index
        ) {
            // get the next id
            const entity_id eid = entity_list_index(quad_list, quad_index);
            assert(eid != INVALID_ID);

            // look up the entity
            const bool is_valid = entity_lookup_by_id(e, eid) && e.archetype.has_any(cmpnt_type_e_quad);
        
            // if the entity has a quad, add 4 vertices
            if (is_valid) vertex_count += 4;
        }

        return(quad_count);
    }

    IFB_INTERNAL u32
    quad_render_buffer(
        quad_vertex_buffer& buffer,
        const entity_list*  quad_list,
        const orientation&  camera_orientation,
        const hnd_arena     h_arena) {

        assert(quad_list);
        assert(h_arena != INVALID_HANDLE);

        // make sure we have entities to render
        const u32 count_entities = entity_list_count(quad_list);  
        if (count_entities == 0) return(0);
      
        // count the vertices we need to render
        // we've established there's something to render,
        // so it should be non-zero
        const u32 count_vertices = quad_list_count_vertices(quad_list);
        assert(count_vertices > 0);

        // calculate the size needed to render the buffer,
        // and the size we actually have
        const u32 size_free   = arena_size_free(h_arena);
        const u32 size_needed = count_vertices * sizeof(quad_vertex); 
        const u32 size_push   = size_free >= size_needed ? size_needed : size_free; 
        if (size_push == 0) return(0);

        // allocate memory
        // we've established its available, so it should succeed 
        buffer.data.vptr = arena_push(h_arena, size_push);
        buffer.size      = size_push;
        assert(buffer.data.vptr != NULL);

        // render the quads
        for (
            u32 index = 0;
                index < count_entities;
              ++index) {

            const entity_id eid      = entity_list_index(quad_list, index);  
            quad_vertices&  vertices = buffer.data.vertices[index];
            quad_render(vertices, eid, camera_orientation);
        }

        return(count_vertices);
    }
    
    IFB_INTERNAL void
    quad_render(
        quad_vertices&     vertices,
        const entity_id    id,
        const orientation& camera_orientation) {

        assert(id != INVALID_ID);

        quad_entity q;
        assert(quad_lookup_by_id(q, id));

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
};
