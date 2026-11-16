#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

#include <cassert>
#include <math.h>
#include <stdio.h>

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr const cchar* GAME_PHYS_TEST_MAP_NAME   = "PHYS-TEST";
    static constexpr f32          GAME_PHYS_TEST_TILE_SIZE  = config::map_tile_unit_size;

    // the quads collide with everything, the walls only with the quads, so the walls never pair up with each other
    static constexpr u32 GAME_PHYS_TEST_MASK_QUAD = bit_value(0);
    static constexpr u32 GAME_PHYS_TEST_MASK_WALL = bit_value(1);

    // the walls are tall, so a quad's shortest way out of one is always sideways, never over the top
    static constexpr f32 GAME_PHYS_TEST_WALL_HEIGHT = 1.0f;

    // the quads spawn on a grid, so none of them start out overlapping
    // a cell is this much bigger than a quad, and there are never more cells than this
    static constexpr f32 GAME_PHYS_TEST_CELL_SCALE    = 1.5f;
    static constexpr u32 GAME_PHYS_TEST_CELL_CAPACITY = 8192;

    // the camera looks down on the arena at this angle from the floor, and leaves this much room around it
    static constexpr f32 GAME_PHYS_TEST_CAMERA_PITCH_DEG = 55.0f;
    static constexpr f32 GAME_PHYS_TEST_CAMERA_MARGIN    = 1.1f;

    // below this a quad counts as stopped
    static constexpr f32 GAME_PHYS_TEST_SPEED_STOPPED = 0.001f;

    static const cchar* const GAME_PHYS_TEST_QUAD_SPRITES[] = {
        "TILE-AQUA-1",   "TILE-BLUE-1",   "TILE-GREEN-1",  "TILE-ORANGE-1",
        "TILE-PURPLE-1", "TILE-RED-1",    "TILE-YELLOW-1", "TILE-AQUA-2",
        "TILE-BLUE-2",   "TILE-GREEN-2",  "TILE-ORANGE-2", "TILE-PURPLE-2",
        "TILE-RED-2",    "TILE-YELLOW-2",
    };
    static constexpr u32 GAME_PHYS_TEST_QUAD_SPRITE_COUNT = sizeof(GAME_PHYS_TEST_QUAD_SPRITES) / sizeof(GAME_PHYS_TEST_QUAD_SPRITES[0]);

    enum game_phys_test_wall_e {
        game_phys_test_wall_e_north = 0,
        game_phys_test_wall_e_south = 1,
        game_phys_test_wall_e_west  = 2,
        game_phys_test_wall_e_east  = 3,
        game_phys_test_wall_e_count = 4
    };

    //--------------------------------------------------------------------
    // STATE
    //--------------------------------------------------------------------

    // the physics test scene's state, in its scene memory, NULL while it isn't the active scene
    // the applied values are what the quads have now, the settings are what the gui wants
    struct game_scene_physics {
        hnd_scene      h_scene;
        game_map       map;
        game_phys_test test;
        entity_id      walls [game_phys_test_wall_e_count];
        entity_id      quads [GAME_PHYS_TEST_QUAD_CAPACITY];
        u32            quad_count;
        f32            restitution_applied;
        f32            mass_min_applied;
        f32            mass_max_applied;
        bool           is_energy_start_pending; // the next readout's energy becomes the start
        cmpnt_position interior_min;            // the floor between the walls, in world space
        cmpnt_position interior_max;
        vec3           camera_origin_prev;      // the camera before the scene entered, it's put back on exit
        vec3           camera_target_prev;
    } static * _scene_physics;

    inline bool game_scene_physics_spawn_walls     (game_scene_physics* phys);
    inline void game_scene_physics_spawn_quads     (game_scene_physics* phys);
    inline void game_scene_physics_destroy_quads   (game_scene_physics* phys);
    inline void game_scene_physics_apply_live      (game_scene_physics* phys);
    inline void game_scene_physics_update_readout  (game_scene_physics* phys);
    inline void game_scene_physics_frame_camera    (game_scene_physics* phys);

    //--------------------------------------------------------------------
    // SCENE PROCS
    //--------------------------------------------------------------------

    // builds the arena map, walls it in, and fills it with quads
    // the scene is bound, so everything created here belongs to it, the map too
    IFB_GAME void
    game_scene_physics_enter(
        game_context*   ctx,
        const hnd_scene h_scene) {

        assert(ctx);
        assert(h_scene != INVALID_HANDLE);
        assert(_scene_physics == NULL);

        auto phys = (game_scene_physics*)eng_scene_alloc(h_scene, sizeof(game_scene_physics));
        assert(phys != NULL);
        memset(phys, 0, sizeof(game_scene_physics));

        _scene_physics = phys;
        phys->h_scene  = h_scene;
        ctx->map       = &phys->map;
        ctx->phys_test = &phys->test;

        // the defaults, a few hundred quads at a good clip, bouncing forever
        game_phys_test_settings& settings = phys->test.settings;
        settings.quad_count           = 200;
        settings.quad_size            = 0.05f;
        settings.speed_min            = 0.5f;
        settings.speed_max            = 1.5f;
        settings.restitution          = 1.0f;
        settings.mass_min             = 1.0f;
        settings.mass_max             = 1.0f;
        settings.is_respawn_requested = false;

        // one chunk is the whole arena, physics only pairs entities in the same chunk
        // the floor is gray, and the ring of tiles around the edge is wood, where the walls are
        const hnd_map h_map = eng_map_create(GAME_PHYS_TEST_MAP_NAME, GAME_PHYS_TEST_ARENA_ROWS, GAME_PHYS_TEST_ARENA_COLS);
        assert(h_map != INVALID_HANDLE);
        phys->map.eng_hnd = h_map;

        const u32 chunk_index = eng_map_chunk_create(h_map, GAME_PHYS_TEST_ARENA_ROWS, GAME_PHYS_TEST_ARENA_COLS, 0, 0, map_color_e_gray_light);
        (void)eng_map_chunk_set_sprite(h_map, chunk_index, "CORE-SPRITES", "TILE-GRAY-2");
        for (u32 row = 0; row < GAME_PHYS_TEST_ARENA_ROWS; ++row) {
            for (u32 col = 0; col < GAME_PHYS_TEST_ARENA_COLS; ++col) {
                const bool is_edge = (
                    row == 0 || row == GAME_PHYS_TEST_ARENA_ROWS - 1 ||
                    col == 0 || col == GAME_PHYS_TEST_ARENA_COLS - 1
                );
                if (is_edge) {
                    (void)eng_map_set_tile_sprite(h_map, row, col, "CORE-SPRITES", "WOOD");
                }
            }
        }

        // the floor inside the walls, the quads have to stay in here
        cmpnt_map_coords coords_min = { h_map, 1.0f,                                     0.0f, 1.0f                                     };
        cmpnt_map_coords coords_max = { h_map, (f32)(GAME_PHYS_TEST_ARENA_COLS - 1), 0.0f, (f32)(GAME_PHYS_TEST_ARENA_ROWS - 1) };
        cmpnt_position   pos_a;
        cmpnt_position   pos_b;
        const bool did_get_interior = (
            eng_map_get_pos_from_coords(coords_min, pos_a) &&
            eng_map_get_pos_from_coords(coords_max, pos_b)
        );
        assert(did_get_interior);
        (void)did_get_interior;
        phys->interior_min = { fminf(pos_a.x, pos_b.x), pos_a.y, fminf(pos_a.z, pos_b.z) };
        phys->interior_max = { fmaxf(pos_a.x, pos_b.x), pos_a.y, fmaxf(pos_a.z, pos_b.z) };

        const bool did_spawn_walls = game_scene_physics_spawn_walls(phys);
        assert(did_spawn_walls);
        (void)did_spawn_walls;

        game_scene_physics_spawn_quads(phys);

        // the camera is put back the way the last scene left it when this one exits
        phys->camera_origin_prev = eng_camera_get_origin();
        phys->camera_target_prev = eng_camera_get_target();
        game_scene_physics_frame_camera(phys);
    }

    // the engine skips the scene's entities while it isn't updating or rendering,
    // so this only has to keep the test's own logic in step
    IFB_GAME void
    game_scene_physics_update_and_render(
        game_context* ctx) {

        assert(ctx);
        assert(_scene_physics);

        auto phys = _scene_physics;

        const hnd_scene h_scene = phys->h_scene;
        if (!eng_scene_is_updating(h_scene) &&
            !eng_scene_is_rendering(h_scene)) {
            return;
        }

        // the window asked for new quads
        game_phys_test_settings& settings = phys->test.settings;
        if (settings.is_respawn_requested) {
            settings.is_respawn_requested = false;
            game_scene_physics_destroy_quads (phys);
            game_scene_physics_spawn_quads   (phys);
        }

        game_scene_physics_apply_live     (phys);
        game_scene_physics_frame_camera   (phys);
        game_scene_physics_update_readout (phys);

        (void)eng_map_render(phys->map.eng_hnd);
        for (
            u32 i_quad = 0;
                i_quad < phys->quad_count;
              ++i_quad) {

            (void)eng_entity_render(phys->quads[i_quad]);
        }
    }

    // the engine already released everything the scene owned: its entities, its map, and its memory
    // the camera is the engine's, so it's put back for the next scene
    IFB_GAME void
    game_scene_physics_exit(
        game_context* ctx) {

        assert(ctx);
        assert(_scene_physics);

        eng_camera_set_origin(_scene_physics->camera_origin_prev);
        eng_camera_set_target(_scene_physics->camera_target_prev);

        ctx->phys_test = NULL;
        _scene_physics = NULL;
    }

    //--------------------------------------------------------------------
    // INTERNAL
    //--------------------------------------------------------------------

    // a wall is an invisible, immovable box over one side of the ring of wood tiles
    // its center is on the map, so physics pairs it with the quads in the arena's chunk
    inline entity_id
    game_scene_physics_spawn_wall(
        const cchar*  tag,
        const hnd_map h_map,
        const f32     center_col,
        const f32     center_row,
        const f32     count_cols,
        const f32     count_rows) {

        const entity_id id = eng_entity_create(tag);
        if (id == ENTITY_ID_INVALID) return(ENTITY_ID_INVALID);

        cmpnt_map_coords coords;
        coords.h_map = h_map;
        coords.col_x = center_col;
        coords.lvl_y = 0.0f;
        coords.row_z = center_row;

        cmpnt_bounds bounds;
        bounds.x      = 0.0f;
        bounds.y      = 0.0f;
        bounds.width  = count_cols * GAME_PHYS_TEST_TILE_SIZE;
        bounds.height = GAME_PHYS_TEST_WALL_HEIGHT;
        bounds.depth  = count_rows * GAME_PHYS_TEST_TILE_SIZE;

        cmpnt_collider collider;
        collider.proc               = NULL;
        collider.mask_category      = GAME_PHYS_TEST_MASK_WALL;
        collider.mask_collides_with = GAME_PHYS_TEST_MASK_QUAD;
        collider.is_trigger         = false;
        collider.restitution        = 0.0f; // the quads bring their own bounce

        bool did_spawn = true;
        did_spawn &= eng_entity_add_components(id,
            cmpnt_type_e_position   |
            cmpnt_type_e_map_coords |
            cmpnt_type_e_bounds     |
            cmpnt_type_e_collider   |
            cmpnt_type_e_immovable
        );
        did_spawn &= eng_cmpnt_update_map_coords (id, coords); // moves the position onto the coordinates
        did_spawn &= eng_cmpnt_update_bounds     (id, bounds);
        did_spawn &= eng_cmpnt_update_collider   (id, collider);

        if (!did_spawn) {
            eng_entity_destroy_by_id(id);
            return(ENTITY_ID_INVALID);
        }
        return(id);
    }

    inline bool
    game_scene_physics_spawn_walls(
        game_scene_physics* phys) {

        const hnd_map h_map = phys->map.eng_hnd;
        const f32     cols  = (f32)GAME_PHYS_TEST_ARENA_COLS;
        const f32     rows  = (f32)GAME_PHYS_TEST_ARENA_ROWS;

        // north and south run the whole width, west and east fit between them
        phys->walls[game_phys_test_wall_e_north] = game_scene_physics_spawn_wall("PHYS-WALL-N", h_map, cols * 0.5f, 0.5f,        cols, 1.0f);
        phys->walls[game_phys_test_wall_e_south] = game_scene_physics_spawn_wall("PHYS-WALL-S", h_map, cols * 0.5f, rows - 0.5f, cols, 1.0f);
        phys->walls[game_phys_test_wall_e_west]  = game_scene_physics_spawn_wall("PHYS-WALL-W", h_map, 0.5f,        rows * 0.5f, 1.0f, rows - 2.0f);
        phys->walls[game_phys_test_wall_e_east]  = game_scene_physics_spawn_wall("PHYS-WALL-E", h_map, cols - 0.5f, rows * 0.5f, 1.0f, rows - 2.0f);

        for (u32 i_wall = 0; i_wall < game_phys_test_wall_e_count; ++i_wall) {
            if (phys->walls[i_wall] == ENTITY_ID_INVALID) return(false);
        }
        return(true);
    }

    // a random mass between the settings' min and max, as an inverse mass
    inline f32
    game_scene_physics_random_inv_mass(
        const game_phys_test_settings& settings) {

        const f32 mass_min = fmaxf(settings.mass_min, 0.01f);
        const f32 mass_max = fmaxf(settings.mass_max, mass_min);
        const f32 mass     = eng_random_f32_range(mass_min, mass_max);
        return(1.0f / mass);
    }

    // fills the floor with quads from the settings, each on its own cell of a grid so none of them overlap
    // stops early if there aren't enough cells, or the engine runs out of entities
    inline void
    game_scene_physics_spawn_quads(
        game_scene_physics* phys) {

        assert(phys->quad_count == 0);

        game_phys_test_settings& settings = phys->test.settings;
        const hnd_map            h_map    = phys->map.eng_hnd;

        // keep the settings sane, the gui's sliders already do, but the defaults are set here too
        if (settings.quad_count > GAME_PHYS_TEST_QUAD_CAPACITY) settings.quad_count = GAME_PHYS_TEST_QUAD_CAPACITY;
        if (settings.quad_size  < 0.005f)                       settings.quad_size  = 0.005f;
        if (settings.speed_max  < settings.speed_min)           settings.speed_max  = settings.speed_min;

        // the grid over the floor between the walls, in tiles
        const f32 interior_cols = (f32)(GAME_PHYS_TEST_ARENA_COLS - 2);
        const f32 interior_rows = (f32)(GAME_PHYS_TEST_ARENA_ROWS - 2);
        f32       cell_tiles    = (settings.quad_size * GAME_PHYS_TEST_CELL_SCALE) / GAME_PHYS_TEST_TILE_SIZE;
        u32       cells_x       = (u32)(interior_cols / cell_tiles);
        u32       cells_z       = (u32)(interior_rows / cell_tiles);

        // tiny quads would make too many cells, so the cells grow until they fit
        if (cells_x * cells_z > GAME_PHYS_TEST_CELL_CAPACITY) {
            cell_tiles = sqrtf((interior_cols * interior_rows) / (f32)GAME_PHYS_TEST_CELL_CAPACITY);
            cells_x    = (u32)(interior_cols / cell_tiles);
            cells_z    = (u32)(interior_rows / cell_tiles);
            while (cells_x * cells_z > GAME_PHYS_TEST_CELL_CAPACITY) {
                cell_tiles *= 1.01f;
                cells_x     = (u32)(interior_cols / cell_tiles);
                cells_z     = (u32)(interior_rows / cell_tiles);
            }
        }

        const u32 cell_count = cells_x * cells_z;
        u32       count      = settings.quad_count;
        if (count > cell_count) count = cell_count;

        // pick count of the cells at random, the first count of a partial shuffle
        static u16 cells[GAME_PHYS_TEST_CELL_CAPACITY];
        for (u32 i_cell = 0; i_cell < cell_count; ++i_cell) {
            cells[i_cell] = (u16)i_cell;
        }

        // centered on the floor, the leftover space is split on both sides
        const f32 margin_col = 1.0f + ((interior_cols - (cells_x * cell_tiles)) * 0.5f);
        const f32 margin_row = 1.0f + ((interior_rows - (cells_z * cell_tiles)) * 0.5f);

        cmpnt_collider collider;
        collider.proc               = NULL;
        collider.mask_category      = GAME_PHYS_TEST_MASK_QUAD;
        collider.mask_collides_with = PHYS_CLLSN_MASK_ALL;
        collider.is_trigger         = false;
        collider.restitution        = settings.restitution;

        const f32 size = settings.quad_size;

        cmpnt_bounds bounds;
        bounds.x      = 0.0f;
        bounds.y      = 0.0f;
        bounds.width  = size;
        bounds.height = size;
        bounds.depth  = size;

        for (
            u32 i_quad = 0;
                i_quad < count;
              ++i_quad) {

            const u32 i_pick = i_quad + (eng_random_u32() % (cell_count - i_quad));
            const u16 cell   = cells[i_pick];
            cells[i_pick]    = cells[i_quad];
            cells[i_quad]    = cell;

            const u32 cell_x = cell % cells_x;
            const u32 cell_z = cell / cells_x;

            atype_physics_sprite quad = {};
            quad.map_coords.h_map = h_map;
            quad.map_coords.col_x = margin_col + ((cell_x + 0.5f) * cell_tiles);
            quad.map_coords.lvl_y = 0.0f;
            quad.map_coords.row_z = margin_row + ((cell_z + 0.5f) * cell_tiles);
            if (!eng_map_get_pos_from_coords(quad.map_coords, quad.position)) break;

            // quads are centered on their position, lift by half the height so it sits on the floor
            quad.position.y += size * 0.5f;

            // off in a random direction across the floor
            const f32 angle = eng_random_f32_range(0.0f, 6.2831853f);
            const f32 speed = eng_random_f32_range(settings.speed_min, settings.speed_max);
            quad.velocity   = { cosf(angle) * speed, 0.0f, sinf(angle) * speed };

            // no drag, and no top speed, so the only thing that changes their speed is hitting something
            quad.color.hex          = 0xFFFFFFFF;
            quad.quad.width         = size;
            quad.quad.height        = size;
            quad.inv_mass.normal_val = game_scene_physics_random_inv_mass(settings);
            quad.drag.normal_val    = 1.0f;
            quad.term_velocity      = { 0.0f, 0.0f, 0.0f };

            cchar tag[TAG_SIZE];
            (void)snprintf(tag, sizeof(tag), "PHYS-QUAD-%04u", i_quad);

            const entity_id id = eng_entity_create(tag);
            if (id == ENTITY_ID_INVALID) break;

            bool did_spawn = true;
            did_spawn &= eng_entity_add_components       (id, ARCHETYPE_PHYSICS_SPRITE | cmpnt_type_e_bounds | cmpnt_type_e_collider);
            did_spawn &= eng_atype_update_physics_sprite (id, quad);
            did_spawn &= eng_entity_set_sprite           (id, "CORE-SPRITES", GAME_PHYS_TEST_QUAD_SPRITES[i_quad % GAME_PHYS_TEST_QUAD_SPRITE_COUNT]);
            did_spawn &= eng_cmpnt_update_bounds         (id, bounds);
            did_spawn &= eng_cmpnt_update_collider       (id, collider);

            if (!did_spawn) {
                eng_entity_destroy_by_id(id);
                break;
            }

            phys->quads[phys->quad_count] = id;
            ++phys->quad_count;
        }

        phys->restitution_applied     = settings.restitution;
        phys->mass_min_applied        = settings.mass_min;
        phys->mass_max_applied        = settings.mass_max;
        phys->is_energy_start_pending = true;
    }

    inline void
    game_scene_physics_destroy_quads(
        game_scene_physics* phys) {

        for (
            u32 i_quad = 0;
                i_quad < phys->quad_count;
              ++i_quad) {

            (void)eng_entity_destroy_by_id(phys->quads[i_quad]);
            phys->quads[i_quad] = ENTITY_ID_INVALID;
        }
        phys->quad_count = 0;
    }

    // the restitution and masses change the quads that are already flying
    inline void
    game_scene_physics_apply_live(
        game_scene_physics* phys) {

        const game_phys_test_settings& settings = phys->test.settings;

        if (settings.restitution != phys->restitution_applied) {
            for (u32 i_quad = 0; i_quad < phys->quad_count; ++i_quad) {
                cmpnt_collider collider;
                if (!eng_cmpnt_lookup_collider(phys->quads[i_quad], collider)) continue;
                collider.restitution = settings.restitution;
                (void)eng_cmpnt_update_collider(phys->quads[i_quad], collider);
            }
            phys->restitution_applied = settings.restitution;
        }

        if (settings.mass_min != phys->mass_min_applied ||
            settings.mass_max != phys->mass_max_applied) {
            for (u32 i_quad = 0; i_quad < phys->quad_count; ++i_quad) {
                (void)eng_cmpnt_update_inv_mass(phys->quads[i_quad], game_scene_physics_random_inv_mass(settings));
            }
            phys->mass_min_applied = settings.mass_min;
            phys->mass_max_applied = settings.mass_max;

            // the energy changes with the masses, so it starts over
            phys->is_energy_start_pending = true;
        }
    }

    inline void
    game_scene_physics_update_readout(
        game_scene_physics* phys) {

        game_phys_test_readout& readout = phys->test.readout;
        readout.quad_count        = phys->quad_count;
        readout.quad_count_moving = 0;
        readout.speed_avg         = 0.0f;
        readout.speed_max         = 0.0f;
        readout.kinetic_energy    = 0.0f;
        readout.escaped_count     = 0;

        // a quad past the inside face of a wall by more than its own size got through it
        const f32 slack = phys->test.settings.quad_size;

        f32 speed_sum = 0.0f;
        for (
            u32 i_quad = 0;
                i_quad < phys->quad_count;
              ++i_quad) {

            const entity_id id = phys->quads[i_quad];

            cmpnt_velocity vel;
            cmpnt_position pos;
            f32            inv_mass = 0.0f;
            if (!eng_cmpnt_lookup_velocity (id, vel))      continue;
            if (!eng_cmpnt_lookup_position (id, pos))      continue;
            if (!eng_cmpnt_lookup_inv_mass (id, inv_mass)) continue;

            const f32 speed_sq = (vel.x * vel.x) + (vel.y * vel.y) + (vel.z * vel.z);
            const f32 speed    = sqrtf(speed_sq);
            speed_sum += speed;
            if (speed > readout.speed_max)             readout.speed_max = speed;
            if (speed > GAME_PHYS_TEST_SPEED_STOPPED)  ++readout.quad_count_moving;
            if (inv_mass > 0.0f)                       readout.kinetic_energy += (0.5f * speed_sq) / inv_mass;

            const bool is_escaped = (
                pos.x < phys->interior_min.x - slack || pos.x > phys->interior_max.x + slack ||
                pos.z < phys->interior_min.z - slack || pos.z > phys->interior_max.z + slack
            );
            if (is_escaped) ++readout.escaped_count;
        }

        if (phys->quad_count > 0) {
            readout.speed_avg = speed_sum / (f32)phys->quad_count;
        }

        if (phys->is_energy_start_pending) {
            readout.kinetic_energy_start  = readout.kinetic_energy;
            phys->is_energy_start_pending = false;
        }
    }

    // looks down at the middle of the arena, from far enough back to see all of it
    // every frame, so the arena stays framed when the window is resized
    inline void
    game_scene_physics_frame_camera(
        game_scene_physics* phys) {

        const f32 tile        = GAME_PHYS_TEST_TILE_SIZE;
        const f32 half_width  = ((f32)GAME_PHYS_TEST_ARENA_COLS * tile) * 0.5f;
        const f32 half_depth  = ((f32)GAME_PHYS_TEST_ARENA_ROWS * tile) * 0.5f;
        const f32 center_x    = (phys->interior_min.x + phys->interior_max.x) * 0.5f;
        const f32 center_z    = (phys->interior_min.z + phys->interior_max.z) * 0.5f;
        const f32 floor_y     = phys->interior_min.y;

        u32 screen_width  = 0;
        u32 screen_height = 0;
        eng_draw_get_screen_size(screen_width, screen_height);
        const f32 aspect = (screen_width > 0 && screen_height > 0) ? ((f32)screen_width / (f32)screen_height) : (16.0f / 9.0f);

        const f32 deg_to_rad = 3.14159265f / 180.0f;
        const f32 tan_half_y = tanf(eng_camera_get_fov() * deg_to_rad * 0.5f);
        const f32 tan_half_x = tan_half_y * aspect;
        const f32 pitch      = GAME_PHYS_TEST_CAMERA_PITCH_DEG * deg_to_rad;

        // far enough that the width fits across, and the depth, tilted away from the camera, fits up and down
        const f32 dist_for_width = half_width / tan_half_x;
        const f32 dist_for_depth = (half_depth * sinf(pitch) / tan_half_y) + (half_depth * cosf(pitch));
        const f32 dist           = fmaxf(dist_for_width, dist_for_depth) * GAME_PHYS_TEST_CAMERA_MARGIN;

        // the game's camera sits toward +z, looking back along -z, this one does too, just higher up
        const vec3 target = { center_x, floor_y, center_z };
        const vec3 origin = {
            center_x,
            floor_y  + (dist * sinf(pitch)),
            center_z + (dist * cosf(pitch))
        };
        eng_camera_set_target(target);
        eng_camera_set_origin(origin);
    }
};
