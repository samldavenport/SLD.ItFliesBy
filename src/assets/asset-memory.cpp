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

    struct asset_memory_partition {
        struct {
            hnd_asset* array_hnd;
            u64*       array_timestamp_last_accessed;
            u32*       array_sizes;
            u32*       array_offsets;
        } header;
        addr data;
    };

    struct asset_memory {
        reservation*            res;
        asset_memory_partition* partition_table[IFB_CONFIG_ASSET_PARTITION_COUNT];
    };

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS 
    //--------------------------------------------------------------------

    inline asset_memory_partition* asset_memory_partition_create      (reservation* res); 
    inline u32                     asset_memory_partition_header_size (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL asset_memory*
    asset_memory_create (
        void) {

        auto mem = (asset_memory*)global_alloc(sizeof(asset_memory));
        assert(mem);

        return(mem);
    }

    IFB_INTERNAL void
    asset_memory_init(
        asset_memory* mem, 
        reservation*  res) {

        const auto& cfg = config_instance();

        for (
            u32 index = 0;
                index < cfg.asset_partition_count;
              ++index) {

            mem->partition_table[index] = asset_memory_partition_create(res);
            assert(mem->partition_table[index] != NULL);
        }

        assert(res);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS 
    //--------------------------------------------------------------------
    
    inline u32
    asset_memory_partition_header_size(
        void) {

        static const auto& cfg = config_instance();

        static const u32 partition_size_req = (
            sizeof(asset_memory_partition) +
            (
                sizeof(hnd_asset) + 
                sizeof(u64)       + 
                sizeof(u32)       + 
                sizeof(u32) 
            ) * cfg.asset_partition_block_capacity
        );
    
        return(partition_size_req);
    }
    
    inline asset_memory_partition*
    asset_memory_partition_create(
        reservation* res) {

        assert(res);

        const auto& cfg = config_instance();

        const addr addr_partition = (addr)reservation_push_bytes(res, cfg.asset_partition_size);
        assert(addr_partition);

        const addr addr_hnd              = addr_partition        + sizeof(asset_memory_partition);  
        const addr addr_ts_last_accessed = addr_hnd              + (cfg.asset_partition_block_capacity * sizeof(hnd_asset));  
        const addr addr_sizes            = addr_ts_last_accessed + (cfg.asset_partition_block_capacity * sizeof(u64)); 
        const addr addr_offsets          = addr_sizes            + (cfg.asset_partition_block_capacity * sizeof(u32));

        auto partition = (asset_memory_partition*)addr_partition;
        partition->header.array_hnd                     = (hnd_asset*)addr_hnd; 
        partition->header.array_timestamp_last_accessed =       (u64*)addr_ts_last_accessed; 
        partition->header.array_sizes                   =       (u32*)addr_sizes;
        partition->header.array_offsets                 =       (u32*)addr_offsets;

        return(partition);
    }
};
