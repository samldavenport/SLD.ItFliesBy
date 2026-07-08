#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct memory_mngr;
    struct stack_allocator;
    struct block_allocator;
    struct global_allocator;
    
    class arena_allocator;
    class arena;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL memory_mngr* memory_mngr_create            (void);
    IFB_INTERNAL void         memory_mngr_startup           (memory& mem_reserved_arenas);
    IFB_INTERNAL void         memory_mngr_shutdown          (void);
    IFB_INTERNAL arena*       memory_mngr_arena_alloc       (void);
    IFB_INTERNAL void         memory_mngr_arena_free        (arena** a);
    IFB_INTERNAL arena*       memory_mngr_arena_from_handle (const eng_arena_handle hnd);

    IFB_INTERNAL void         block_alctr_init     (block_allocator* alctr, memory mem, const u32 granularity);
    IFB_INTERNAL void*        block_alloc          (block_allocator* alctr);
    IFB_INTERNAL void         block_free           (void* mem);


    //--------------------------------------------------------------------
    // ARENA ALLOCATOR CLASS
    //--------------------------------------------------------------------

    class arena_allocator {
        
    private:

        memory _mem;
        u32    _arena_size;
        u32    _arena_count_total;
        u32    _arena_count_free;
        struct {
            arena* free;
            arena* used;
        } _list;

        friend class arena;

    public:

        arena_allocator(void) = default;

        void   init              (memory& mem, const u32 granularity);
        void   validate          (void);
        arena* alloc             (void);
        void   free              (arena* a);
        arena* arena_from_handle (const eng_arena_handle hnd);

        inline u32 arena_size(void) { return(_arena_size); }
    };

    //--------------------------------------------------------------------
    // ARENA CLASS
    //--------------------------------------------------------------------

    class arena {

    private:

        arena_allocator* _alctr;
        arena*           _next;
        arena*           _prev;
        u32              _id;
        u32              _position;
        u32              _save;

        friend class arena_allocator;

    public:

        void   validate (void);
        void   free     (void);
        void   reset    (void);
        u32    save     (void);
        void*  push     (const u32 size);
        void   revert   (const u32 save);
        void   commit   (const u32 save);        

        template<typename t>
        t* push(const u32 count = 1);
        
        inline u32 id         (void) const { return( _id);                            }
        inline u32 size_used  (void) const { return(_position);                       }
        inline u32 size_total (void) const { return(_alctr->_arena_size);             }
        inline u32 size_free (void)  const { return(_alctr->_arena_size - _position); }
    };

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------

    struct block_memory {
        block_allocator* alctr;
        block_memory*    prev;
        block_memory*    next;
        u32              id;
    };

    struct block_allocator {
        struct {
            block_memory* free;
            block_memory* used;
        } list;
        addr start;
        u32  block_size;
        u32  block_count_total;
        u32  block_count_free;
    };

    struct memory_mngr {
        arena_allocator* arena_alctr;
    } static * _memory_mngr;
};

#endif //MEMORY_HPP