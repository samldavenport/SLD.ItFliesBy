#pragma once

#include "asset.hpp"
#include "asset-internal.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-reservation.cpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    enum asset_memory_size_class_e {
        asset_memory_size_class_e_small  = 0,
        asset_memory_size_class_e_medium = 1,
        asset_memory_size_class_e_large  = 2,
        asset_memory_size_class_e_count  = 3
    };

    enum asset_memory_block_flag_e {
        asset_memory_block_flag_e_none            = 0,
        asset_memory_block_flag_e_marked_for_free = (1 << 0)
    };

    struct asset_memory_size_class {
        u32 partition_count;
        u32 data_size;
        u32 block_capacity;
    };

    struct asset_memory_partition {
        struct {
            u64*       array_timestamp_last_accessed;
            hnd_asset* array_hnd;
            u32*       array_sizes;
            u32*       array_offsets;
            u8*        array_flags;
            u8*        array_pin_counts;
        } header;
        u32  data_size;
        u32  block_capacity;
        u32  offset_end;        // first free byte in the data region, everything after it is free
        u32  bytes_used;        // aligned size of all live blocks, (offset_end - bytes_used) is the space lost to gaps
        u32  count_blocks_free;
        u32  count_blocks_pinned;
        addr data;
    };

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    // size classes must be ordered from smallest to largest
    constexpr asset_memory_size_class ASSET_MEMORY_SIZE_CLASSES[asset_memory_size_class_e_count] = {
        { config::asset_small_partition_count,  config::asset_small_data_size,  config::asset_small_block_capacity  },
        { config::asset_medium_partition_count, config::asset_medium_data_size, config::asset_medium_block_capacity },
        { config::asset_large_partition_count,  config::asset_large_data_size,  config::asset_large_block_capacity  }
    };

    constexpr u32 ASSET_MEMORY_ALIGNMENT     = 16;
    constexpr u32 ASSET_MEMORY_PIN_COUNT_MAX = 0xFF;

    constexpr u32
    asset_memory_align(
        const u32 size) {

        return((size + (ASSET_MEMORY_ALIGNMENT - 1)) & ~(ASSET_MEMORY_ALIGNMENT - 1));
    }

    // the timestamp array comes first so it stays 8 byte aligned regardless of block capacity
    constexpr u32
    asset_memory_partition_header_size(
        const u32 block_capacity) {

        return(asset_memory_align(
            sizeof(asset_memory_partition) +
            (
                sizeof(u64)       +
                sizeof(hnd_asset) +
                sizeof(u32)       +
                sizeof(u32)       +
                sizeof(u8)        +
                sizeof(u8)
            ) * block_capacity
        ));
    }

    // partitions are stored in the table grouped by size class, smallest first
    constexpr u32
    asset_memory_size_class_first_partition(
        const u32 size_class) {

        u32 first = 0;
        for (u32 index = 0; index < size_class; ++index) {
            first += ASSET_MEMORY_SIZE_CLASSES[index].partition_count;
        }
        return(first);
    }

    constexpr u32
    asset_memory_block_capacity_max(
        void) {

        u32 max = 0;
        for (u32 index = 0; index < asset_memory_size_class_e_count; ++index) {
            const u32 capacity = ASSET_MEMORY_SIZE_CLASSES[index].block_capacity;
            if (capacity > max) max = capacity;
        }
        return(max);
    }

    constexpr u32 ASSET_MEMORY_PARTITION_COUNT      = asset_memory_size_class_first_partition(asset_memory_size_class_e_count);
    constexpr u32 ASSET_MEMORY_BLOCK_CAPACITY_MAX   = asset_memory_block_capacity_max();

    constexpr bool
    asset_memory_size_classes_are_valid(
        void) {

        for (u32 index = 0; index < asset_memory_size_class_e_count; ++index) {

            const asset_memory_size_class& size_class = ASSET_MEMORY_SIZE_CLASSES[index];

            if (size_class.partition_count == 0)                                           return(false);
            if (size_class.block_capacity  == 0)                                           return(false);
            if (size_class.block_capacity  >  INVALID_U16)                                 return(false);
            if (size_class.data_size       != asset_memory_align(size_class.data_size))    return(false);
            if (index > 0 && size_class.data_size <= ASSET_MEMORY_SIZE_CLASSES[index - 1].data_size) return(false);
        }
        return(true);
    }

    static_assert(
        asset_memory_size_classes_are_valid(),
        "asset size classes must be non-empty, aligned, have block capacities that fit in a u16, and be ordered smallest to largest"
    );
    static_assert(
        ASSET_MEMORY_PARTITION_COUNT <= INVALID_U16,
        "asset partition indices must fit in asset_memory_block"
    );
    static_assert(
        ASSET_MEMORY_SIZE_CLASSES[asset_memory_size_class_e_count - 1].data_size >= config::asset_size_max,
        "the largest asset size class must fit the max asset size"
    );
    static_assert(
        sizeof(asset_memory_partition) % sizeof(u64) == 0,
        "asset partition timestamps must be 8 byte aligned"
    );
    static_assert(
        config::asset_cleanup_interval_frames > 0,
        "asset cleanup interval must be at least one frame"
    );

    struct asset_memory {
        reservation*            res;
        u64                     frame_current;
        asset_memory_partition* partition_table[ASSET_MEMORY_PARTITION_COUNT];
    };

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    inline asset_memory_partition* asset_memory_partition_create              (reservation* res, const asset_memory_size_class& size_class);
    inline u32                     asset_memory_partition_find_free_block     (const asset_memory_partition* partition);
    inline void                    asset_memory_partition_free_block          (asset_memory_partition* partition, const u32 block_index);
    inline void                    asset_memory_partition_compact             (asset_memory_partition* partition);
    inline u32                     asset_memory_size_class_for                (const u32 size_aligned);
    inline u32                     asset_memory_find_partition_with_tail      (const asset_memory* mem, const u32 size_aligned);
    inline u32                     asset_memory_find_partition_after_compact  (const asset_memory* mem, const u32 size_aligned);
    inline asset_memory_partition* asset_memory_resolve                       (const asset_memory* mem, const asset_memory_block& block, const hnd_asset hnd);
    inline void                    asset_memory_cleanup                       (asset_memory* mem);
    inline bool                    asset_memory_partition_block_is_pinned     (const asset_memory_partition* partition, const u32 block_index);
    inline u64                     asset_memory_partition_eviction_key        (const asset_memory_partition* partition, const u32 block_index);
    inline u32                     asset_memory_partition_collect_evictable   (const asset_memory_partition* partition, const u64 frame_current, u16* array_evictable);
    inline bool                    asset_memory_partition_plan_eviction       (const asset_memory_partition* partition, const u16* array_evictable, const u32 count_evictable, const u32 size_aligned, u32& count_evict, u64& key_newest);
    inline u32                     asset_memory_evict_for                     (asset_memory* mem, const u32 size_aligned);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL asset_memory*
    asset_memory_create (
        void) {

        auto mem = (asset_memory*)global_alloc(sizeof(asset_memory));
        assert(mem);

        (void)memset(mem, 0, sizeof(asset_memory));

        return(mem);
    }

    IFB_INTERNAL void
    asset_memory_init(
        asset_memory* mem,
        reservation*  res) {

        assert(mem);
        assert(res);

        mem->res           = res;
        mem->frame_current = 0;

        u32 partition_index = 0;
        for (
            u32 size_class_index = 0;
                size_class_index < asset_memory_size_class_e_count;
              ++size_class_index) {

            const asset_memory_size_class& size_class = ASSET_MEMORY_SIZE_CLASSES[size_class_index];

            for (
                u32 count = 0;
                    count < size_class.partition_count;
                  ++count) {

                mem->partition_table[partition_index] = asset_memory_partition_create(res, size_class);
                assert(mem->partition_table[partition_index] != NULL);
                ++partition_index;
            }
        }

        assert(partition_index == ASSET_MEMORY_PARTITION_COUNT);
    }

    IFB_INTERNAL void
    asset_memory_update(
        asset_memory* mem,
        const u64     frame) {

        assert(mem);
        assert(frame >= mem->frame_current);

        mem->frame_current = frame;

        if (frame % config::asset_cleanup_interval_frames == 0) {
            asset_memory_cleanup(mem);
        }
    }

    IFB_INTERNAL bool
    asset_memory_alloc(
        asset_memory*       mem,
        const hnd_asset     hnd,
        const u32           size,
        asset_memory_block& block) {

        assert(mem);
        assert(hnd != INVALID_HANDLE);

        if (size == 0 || size > config::asset_size_max) return(false);
        const u32 size_aligned = asset_memory_align(size);

        // first, look for a partition with enough room at the end of its data
        // if there isn't one, look for a partition that will have enough room once it's compacted
        // if there still isn't one, evict the least recently used blocks to make room
        u32 partition_index = asset_memory_find_partition_with_tail(mem, size_aligned);
        if (partition_index == INVALID_INDEX) {

            partition_index = asset_memory_find_partition_after_compact(mem, size_aligned);
            if (partition_index != INVALID_INDEX) {
                asset_memory_partition_compact(mem->partition_table[partition_index]);
            }
        }
        if (partition_index == INVALID_INDEX) {

            partition_index = asset_memory_evict_for(mem, size_aligned);
            if (partition_index == INVALID_INDEX) return(false);
        }

        asset_memory_partition* partition = mem->partition_table[partition_index];
        assert(partition->data_size - partition->offset_end >= size_aligned);

        // find an unused block in the partition
        const u32 block_index = asset_memory_partition_find_free_block(partition);
        assert(block_index != INVALID_INDEX);

        // bump allocate from the end of the partition
        partition->header.array_timestamp_last_accessed[block_index] = mem->frame_current;
        partition->header.array_hnd                    [block_index] = hnd;
        partition->header.array_sizes                  [block_index] = size;
        partition->header.array_offsets                [block_index] = partition->offset_end;
        partition->header.array_flags                  [block_index] = asset_memory_block_flag_e_none;
        partition->header.array_pin_counts             [block_index] = 0;
        partition->offset_end        += size_aligned;
        partition->bytes_used        += size_aligned;
        partition->count_blocks_free -= 1;

        block.partition_index = (u16)partition_index;
        block.block_index     = (u16)block_index;
        return(true);
    }

    IFB_INTERNAL void
    asset_memory_free(
        asset_memory*             mem,
        const asset_memory_block& block) {

        assert(mem);
        assert(block.partition_index < ASSET_MEMORY_PARTITION_COUNT);

        asset_memory_partition* partition = mem->partition_table[block.partition_index];
        assert(partition);

        asset_memory_partition_free_block(partition, block.block_index);
    }

    IFB_INTERNAL bool
    asset_memory_mark_for_free(
        asset_memory*             mem,
        const asset_memory_block& block,
        const hnd_asset           hnd) {

        asset_memory_partition* partition = asset_memory_resolve(mem, block, hnd);
        if (!partition) return(false);

        partition->header.array_flags[block.block_index] |= asset_memory_block_flag_e_marked_for_free;
        return(true);
    }

    // returns NULL if the block no longer belongs to this asset
    // the address is only valid until the next call to asset_memory_update or asset_memory_alloc,
    // either of which can compact the partition and move the data
    IFB_INTERNAL addr
    asset_memory_get_data(
        asset_memory*             mem,
        const asset_memory_block& block,
        const hnd_asset           hnd) {

        asset_memory_partition* partition = asset_memory_resolve(mem, block, hnd);
        if (!partition) return(NULL);

        partition->header.array_timestamp_last_accessed[block.block_index] = mem->frame_current;

        const addr data = partition->data + partition->header.array_offsets[block.block_index];
        return(data);
    }

    // returns NULL if the block no longer belongs to this asset
    // while pinned, the block won't be moved, expired, or freed, so the address stays valid until it's unpinned
    // pins are counted, every call to asset_memory_pin needs a matching call to asset_memory_unpin
    IFB_INTERNAL addr
    asset_memory_pin(
        asset_memory*             mem,
        const asset_memory_block& block,
        const hnd_asset           hnd) {

        asset_memory_partition* partition = asset_memory_resolve(mem, block, hnd);
        if (!partition) return(NULL);

        u8& pin_count = partition->header.array_pin_counts[block.block_index];
        assert(pin_count < ASSET_MEMORY_PIN_COUNT_MAX);

        if (pin_count == 0) {
            partition->count_blocks_pinned += 1;
        }
        pin_count += 1;

        partition->header.array_timestamp_last_accessed[block.block_index] = mem->frame_current;

        const addr data = partition->data + partition->header.array_offsets[block.block_index];
        return(data);
    }

    IFB_INTERNAL void
    asset_memory_unpin(
        asset_memory*             mem,
        const asset_memory_block& block,
        const hnd_asset           hnd) {

        asset_memory_partition* partition = asset_memory_resolve(mem, block, hnd);
        assert(partition);

        u8& pin_count = partition->header.array_pin_counts[block.block_index];
        assert(pin_count > 0);
        assert(partition->count_blocks_pinned > 0);

        pin_count -= 1;
        if (pin_count == 0) {
            partition->count_blocks_pinned -= 1;
        }

        // count the unpin as an access, so a block that was pinned for a long time doesn't expire right away
        partition->header.array_timestamp_last_accessed[block.block_index] = mem->frame_current;
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    inline asset_memory_partition*
    asset_memory_partition_create(
        reservation*                   res,
        const asset_memory_size_class& size_class) {

        assert(res);

        const u32 size_header    = asset_memory_partition_header_size(size_class.block_capacity);
        const u32 size_partition = size_header + size_class.data_size;

        const addr addr_partition = (addr)reservation_push_bytes(res, size_partition);
        assert(addr_partition);

        // calculate addresses
        const addr addr_ts_last_accessed = addr_partition        + sizeof(asset_memory_partition);
        const addr addr_hnd              = addr_ts_last_accessed + (size_class.block_capacity * sizeof(u64));
        const addr addr_sizes            = addr_hnd              + (size_class.block_capacity * sizeof(hnd_asset));
        const addr addr_offsets          = addr_sizes            + (size_class.block_capacity * sizeof(u32));
        const addr addr_flags            = addr_offsets          + (size_class.block_capacity * sizeof(u32));
        const addr addr_pin_counts       = addr_flags            + (size_class.block_capacity * sizeof(u8));
        const addr addr_data             = addr_partition        + size_header;

        // cast pointers
        auto partition = (asset_memory_partition*)addr_partition;
        partition->header.array_timestamp_last_accessed =       (u64*)addr_ts_last_accessed;
        partition->header.array_hnd                     = (hnd_asset*)addr_hnd;
        partition->header.array_sizes                   =       (u32*)addr_sizes;
        partition->header.array_offsets                 =       (u32*)addr_offsets;
        partition->header.array_flags                   =        (u8*)addr_flags;
        partition->header.array_pin_counts              =        (u8*)addr_pin_counts;
        partition->data                                 = addr_data;
        partition->data_size                            = size_class.data_size;
        partition->block_capacity                       = size_class.block_capacity;
        partition->offset_end                           = 0;
        partition->bytes_used                           = 0;
        partition->count_blocks_free                    = size_class.block_capacity;
        partition->count_blocks_pinned                  = 0;

        // set all handles to invalid
        (void)memset(
            (void*)addr_hnd,
            0xFF,
            size_class.block_capacity * sizeof(hnd_asset)
        );

        // clear all flags and pin counts
        (void)memset(
            (void*)addr_flags,
            0,
            size_class.block_capacity * (sizeof(u8) + sizeof(u8))
        );

        return(partition);
    }

    inline u32
    asset_memory_partition_find_free_block(
        const asset_memory_partition* partition) {

        assert(partition);

        if (partition->count_blocks_free == 0) return(INVALID_INDEX);

        for (
            u32 block_index = 0;
                block_index < partition->block_capacity;
              ++block_index) {

            if (partition->header.array_hnd[block_index] == INVALID_HANDLE) {
                return(block_index);
            }
        }

        return(INVALID_INDEX);
    }

    inline void
    asset_memory_partition_free_block(
        asset_memory_partition* partition,
        const u32               block_index) {

        assert(partition);
        assert(block_index < partition->block_capacity);
        assert(partition->header.array_hnd[block_index] != INVALID_HANDLE);
        assert(!asset_memory_partition_block_is_pinned(partition, block_index));

        const u32 size_aligned = asset_memory_align(partition->header.array_sizes[block_index]);
        const u32 offset       = partition->header.array_offsets[block_index];
        assert(partition->bytes_used >= size_aligned);

        // if this was the last block, we can reclaim its space now
        // otherwise, it leaves a gap until the partition is compacted
        if (offset + size_aligned == partition->offset_end) {
            partition->offset_end = offset;
        }

        partition->header.array_timestamp_last_accessed[block_index] = 0;
        partition->header.array_hnd                    [block_index] = INVALID_HANDLE;
        partition->header.array_sizes                  [block_index] = 0;
        partition->header.array_offsets                [block_index] = 0;
        partition->header.array_flags                  [block_index] = asset_memory_block_flag_e_none;
        partition->header.array_pin_counts             [block_index] = 0;
        partition->bytes_used        -= size_aligned;
        partition->count_blocks_free += 1;
    }

    inline void
    asset_memory_partition_compact(
        asset_memory_partition* partition) {

        assert(partition);
        assert(partition->block_capacity <= ASSET_MEMORY_BLOCK_CAPACITY_MAX);

        // nothing to do if there are no gaps
        if (partition->offset_end == partition->bytes_used) return;

        // collect the live blocks, sorted by offset
        u16 array_live[ASSET_MEMORY_BLOCK_CAPACITY_MAX];
        u32 count_live = 0;
        for (
            u32 block_index = 0;
                block_index < partition->block_capacity;
              ++block_index) {

            if (partition->header.array_hnd[block_index] == INVALID_HANDLE) continue;

            const u32 offset = partition->header.array_offsets[block_index];
            u32       insert = count_live;
            while (insert > 0 && partition->header.array_offsets[array_live[insert - 1]] > offset) {
                array_live[insert] = array_live[insert - 1];
                --insert;
            }
            array_live[insert] = (u16)block_index;
            ++count_live;
        }

        // slide each block down to the end of the previous one
        // blocks are moved in offset order, so a move never overwrites a block that hasn't moved yet
        // pinned blocks stay where they are, and the blocks after them pack down to the end of the pinned block
        // a block only ever moves down, so it can never be moved on top of a pinned block
        u32 offset_next = 0;
        for (
            u32 live_index = 0;
                live_index < count_live;
              ++live_index) {

            const u32 block_index  = array_live[live_index];
            const u32 offset_curr  = partition->header.array_offsets[block_index];
            const u32 size_aligned = asset_memory_align(partition->header.array_sizes[block_index]);
            assert(offset_curr >= offset_next);

            if (asset_memory_partition_block_is_pinned(partition, block_index)) {
                offset_next = offset_curr + size_aligned;
                continue;
            }

            if (offset_curr != offset_next) {
                (void)memmove(
                    (void*)(partition->data + offset_next),
                    (void*)(partition->data + offset_curr),
                    partition->header.array_sizes[block_index]
                );
                partition->header.array_offsets[block_index] = offset_next;
            }

            offset_next += size_aligned;
        }

        // with no pinned blocks, everything packs down with no gaps
        // otherwise, there can still be gaps in front of pinned blocks
        assert(offset_next >= partition->bytes_used);
        assert(partition->count_blocks_pinned > 0 || offset_next == partition->bytes_used);
        partition->offset_end = offset_next;
    }

    inline u32
    asset_memory_size_class_for(
        const u32 size_aligned) {

        for (
            u32 size_class_index = 0;
                size_class_index < asset_memory_size_class_e_count;
              ++size_class_index) {

            if (size_aligned <= ASSET_MEMORY_SIZE_CLASSES[size_class_index].data_size) {
                return(size_class_index);
            }
        }

        return(INVALID_INDEX);
    }

    // start with the smallest size class that fits, and fall back to larger classes if it's full
    inline u32
    asset_memory_find_partition_with_tail(
        const asset_memory* mem,
        const u32           size_aligned) {

        assert(mem);

        const u32 size_class_first = asset_memory_size_class_for(size_aligned);
        if (size_class_first == INVALID_INDEX) return(INVALID_INDEX);

        const u32 partition_index_start = asset_memory_size_class_first_partition(size_class_first);

        for (
            u32 partition_index = partition_index_start;
                partition_index < ASSET_MEMORY_PARTITION_COUNT;
              ++partition_index) {

            const asset_memory_partition* partition = mem->partition_table[partition_index];
            assert(partition);

            const u32  size_tail    = partition->data_size - partition->offset_end;
            const bool is_available = (
                size_tail                    >= size_aligned &&
                partition->count_blocks_free >  0
            );

            if (is_available) return(partition_index);
        }

        return(INVALID_INDEX);
    }

    // same search order as asset_memory_find_partition_with_tail, but counts the space lost to gaps
    // among the partitions that fit, picks the one with the least to move
    // partitions with pinned blocks are skipped, since compacting them isn't guaranteed to close every gap
    inline u32
    asset_memory_find_partition_after_compact(
        const asset_memory* mem,
        const u32           size_aligned) {

        assert(mem);

        const u32 size_class_first = asset_memory_size_class_for(size_aligned);
        if (size_class_first == INVALID_INDEX) return(INVALID_INDEX);

        const u32 partition_index_start = asset_memory_size_class_first_partition(size_class_first);

        u32 partition_index_best = INVALID_INDEX;
        u32 bytes_used_best      = INVALID_U32;
        for (
            u32 partition_index = partition_index_start;
                partition_index < ASSET_MEMORY_PARTITION_COUNT;
              ++partition_index) {

            const asset_memory_partition* partition = mem->partition_table[partition_index];
            assert(partition);

            const u32  size_free    = partition->data_size - partition->bytes_used;
            const bool is_available = (
                size_free                      >= size_aligned &&
                partition->count_blocks_free   >  0            &&
                partition->count_blocks_pinned == 0
            );

            if (is_available && partition->bytes_used < bytes_used_best) {
                partition_index_best = partition_index;
                bytes_used_best      = partition->bytes_used;
            }
        }

        return(partition_index_best);
    }

    inline asset_memory_partition*
    asset_memory_resolve(
        const asset_memory*       mem,
        const asset_memory_block& block,
        const hnd_asset           hnd) {

        assert(mem);
        assert(hnd != INVALID_HANDLE);

        if (block.partition_index >= ASSET_MEMORY_PARTITION_COUNT) return(NULL);

        asset_memory_partition* partition = mem->partition_table[block.partition_index];
        assert(partition);

        if (block.block_index >= partition->block_capacity)        return(NULL);
        if (partition->header.array_hnd[block.block_index] != hnd) return(NULL);

        return(partition);
    }

    // frees every block that was marked for free or hasn't been accessed in asset_expire_frames,
    // then compacts the partition with the most space lost to gaps
    // pinned blocks are never freed, a pinned block that's marked for free is freed on the first cleanup after it's unpinned
    inline void
    asset_memory_cleanup(
        asset_memory* mem) {

        assert(mem);

        u32 partition_index_worst = INVALID_INDEX;
        u32 size_gaps_worst       = 0;

        for (
            u32 partition_index = 0;
                partition_index < ASSET_MEMORY_PARTITION_COUNT;
              ++partition_index) {

            asset_memory_partition* partition = mem->partition_table[partition_index];
            assert(partition);

            for (
                u32 block_index = 0;
                    block_index < partition->block_capacity;
                  ++block_index) {

                if (partition->header.array_hnd[block_index] == INVALID_HANDLE)       continue;
                if (asset_memory_partition_block_is_pinned(partition, block_index)) continue;

                const u64  frames_idle  = mem->frame_current - partition->header.array_timestamp_last_accessed[block_index];
                const bool is_marked    = (partition->header.array_flags[block_index] & asset_memory_block_flag_e_marked_for_free) != 0;
                const bool is_expired   = frames_idle >= config::asset_expire_frames;

                if (is_marked || is_expired) {
                    asset_memory_partition_free_block(partition, block_index);
                }
            }

            const u32 size_gaps = partition->offset_end - partition->bytes_used;
            if (size_gaps > size_gaps_worst) {
                partition_index_worst = partition_index;
                size_gaps_worst       = size_gaps;
            }
        }

        if (partition_index_worst != INVALID_INDEX) {
            asset_memory_partition_compact(mem->partition_table[partition_index_worst]);
        }
    }

    inline bool
    asset_memory_partition_block_is_pinned(
        const asset_memory_partition* partition,
        const u32                     block_index) {

        assert(partition);
        assert(block_index < partition->block_capacity);

        return(partition->header.array_pin_counts[block_index] > 0);
    }

    // blocks marked for free are evicted first, then the least recently accessed
    inline u64
    asset_memory_partition_eviction_key(
        const asset_memory_partition* partition,
        const u32                     block_index) {

        assert(partition);
        assert(block_index < partition->block_capacity);

        const bool is_marked = (partition->header.array_flags[block_index] & asset_memory_block_flag_e_marked_for_free) != 0;
        return(is_marked ? 0 : partition->header.array_timestamp_last_accessed[block_index]);
    }

    // collects the blocks that can be evicted, sorted so the first block is the first to evict
    // pinned blocks and blocks accessed this frame can't be evicted
    inline u32
    asset_memory_partition_collect_evictable(
        const asset_memory_partition* partition,
        const u64                     frame_current,
        u16*                          array_evictable) {

        assert(partition);
        assert(array_evictable);

        u32 count_evictable = 0;
        for (
            u32 block_index = 0;
                block_index < partition->block_capacity;
              ++block_index) {

            if (partition->header.array_hnd[block_index] == INVALID_HANDLE)                    continue;
            if (asset_memory_partition_block_is_pinned(partition, block_index))              continue;
            if (partition->header.array_timestamp_last_accessed[block_index] == frame_current) continue;

            const u64 key    = asset_memory_partition_eviction_key(partition, block_index);
            u32       insert = count_evictable;
            while (insert > 0 && asset_memory_partition_eviction_key(partition, array_evictable[insert - 1]) > key) {
                array_evictable[insert] = array_evictable[insert - 1];
                --insert;
            }
            array_evictable[insert] = (u16)block_index;
            ++count_evictable;
        }

        return(count_evictable);
    }

    // works out how many blocks from the front of array_evictable need to be evicted to fit size_aligned
    // assumes the partition will be compacted afterwards, so it can't have any pinned blocks
    inline bool
    asset_memory_partition_plan_eviction(
        const asset_memory_partition* partition,
        const u16*                    array_evictable,
        const u32                     count_evictable,
        const u32                     size_aligned,
        u32&                          count_evict,
        u64&                          key_newest) {

        assert(partition);
        assert(array_evictable);
        assert(partition->count_blocks_pinned == 0);

        u32 size_free   = partition->data_size - partition->bytes_used;
        u32 count_free  = partition->count_blocks_free;
        count_evict     = 0;
        key_newest      = 0;

        while (size_free < size_aligned || count_free == 0) {

            if (count_evict == count_evictable) return(false);

            const u32 block_index = array_evictable[count_evict];
            size_free  += asset_memory_align(partition->header.array_sizes[block_index]);
            count_free += 1;
            key_newest  = asset_memory_partition_eviction_key(partition, block_index);
            ++count_evict;
        }

        return(true);
    }

    // evicts the least recently used blocks from the partition that can make room by evicting the oldest data,
    // then compacts it, returns the partition index or INVALID_INDEX if no partition can make room
    // same search order as asset_memory_find_partition_with_tail, and partitions with pinned blocks are skipped
    inline u32
    asset_memory_evict_for(
        asset_memory* mem,
        const u32     size_aligned) {

        assert(mem);

        const u32 size_class_first = asset_memory_size_class_for(size_aligned);
        if (size_class_first == INVALID_INDEX) return(INVALID_INDEX);

        const u32 partition_index_start = asset_memory_size_class_first_partition(size_class_first);

        u16 array_evictable[ASSET_MEMORY_BLOCK_CAPACITY_MAX];
        u32 partition_index_best = INVALID_INDEX;
        u64 key_newest_best      = 0;
        u32 count_evict_best     = 0;

        for (
            u32 partition_index = partition_index_start;
                partition_index < ASSET_MEMORY_PARTITION_COUNT;
              ++partition_index) {

            const asset_memory_partition* partition = mem->partition_table[partition_index];
            assert(partition);

            if (partition->count_blocks_pinned > 0) continue;

            const u32 count_evictable = asset_memory_partition_collect_evictable(partition, mem->frame_current, array_evictable);

            u32        count_evict = 0;
            u64        key_newest  = 0;
            const bool can_fit     = asset_memory_partition_plan_eviction(
                partition,
                array_evictable,
                count_evictable,
                size_aligned,
                count_evict,
                key_newest
            );
            if (!can_fit) continue;

            // prefer the partition where the newest block we'd evict is the oldest,
            // then the one that evicts the fewest blocks
            const bool is_better = (
                partition_index_best == INVALID_INDEX ||
                key_newest <  key_newest_best         ||
                (key_newest == key_newest_best && count_evict < count_evict_best)
            );

            if (is_better) {
                partition_index_best = partition_index;
                key_newest_best      = key_newest;
                count_evict_best     = count_evict;
            }
        }

        if (partition_index_best == INVALID_INDEX) return(INVALID_INDEX);

        // collect the blocks again for the partition we picked, and evict them
        asset_memory_partition* partition       = mem->partition_table[partition_index_best];
        const u32               count_evictable = asset_memory_partition_collect_evictable(partition, mem->frame_current, array_evictable);
        assert(count_evictable >= count_evict_best);

        for (
            u32 evict_index = 0;
                evict_index < count_evict_best;
              ++evict_index) {

            asset_memory_partition_free_block(partition, array_evictable[evict_index]);
        }

        asset_memory_partition_compact(partition);
        assert(partition->data_size - partition->offset_end >= size_aligned);
        assert(partition->count_blocks_free > 0);

        return(partition_index_best);
    }
};
