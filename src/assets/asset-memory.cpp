#pragma once

#include "asset.hpp"
#include "asset-internal.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-reservation.cpp"

namespace ifb {

    
    //--------------------------------------------------------------------
    // ASSET PARTITIONING
    //--------------------------------------------------------------------

    struct asset_memory_partition {
        hnd_asset* array_asset;
        u64*       array_timestamp_last_accessed;
        u32*       array_sizes;
    };

    struct asset_memory {
        reservation*            res;
        asset_memory_partition* partition_table[IFB_CONFIG_ASSET_PARTITION_COUNT];
    };

    inline asset_memory_partition* asset_memory_partition_create (reservation* res); 

    IFB_INTERNAL asset_memory*
    asset_memory_create (
        void) {

        auto mem = (asset_memory*)global_alloc(sizeof(asset_memory));
        assert(mem);

        return(mem);
    }

    IFB_INTERNAL void
    asset_memory_init(
        reservation* res) {

        assert(res);
    
        
    }

    inline asset_memory_partition*
    asset_memory_partition_create(
        reservation* res) {

        assert(res);

        const auto& cfg = config_instance();

        const addr addr_partition = (addr)reservation_push_bytes(res, cfg.asset_partition_size);
        assert(addr_partition);

        const addr addr_hnd              = addr_partition        = sizeof(asset_memory_partition);  
        const addr addr_ts_last_accessed = addr_hnd              = (cfg.asset_partition_block_capacity * sizeof(hnd_asset));  
        const addr addr_sizes            = addr_ts_last_accessed = (cfg.asset_partition_block_capacity * sizeof(u64)); 

        auto partition = (asset_memory_partition*)addr_partition;
        partition->array_asset                   = (array_hnd*)addr_hnd; 
        partition->array_timestamp_last_accessed =       (u64*)addr_ts_last_accessed; 
        partition->array_sizes                   =       (u32*)addr_sizes;

        return(partition);
    }
};
