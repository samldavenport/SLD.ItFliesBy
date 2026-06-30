#ifndef IFB_COLLECTIONS_HPP
#define IFB_COLLECTIONS_HPP

#include <sld.hpp>

#include "ifb-types.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STACK
    //--------------------------------------------------------------------

    class IFB_API stack {
    
    private:

        memory _mem;
        u32    _pos;
        u32    _save;

    public:

        // static methods
        static u32 memory_requirement (const u32 size);
        static u32 memory_init        (const u32 size, memory& mem);

        // instance methods
        void  validate   (void) const;
        u32   size_total (void) const;
        u32   size_used  (void) const;
        u32   size_free  (void) const;
        void* head       (void) const;
        void* tail       (void) const;
        void* save       (void);
        void  reset      (void);
        void* push       (const u32   size);
        void  pull       (const u32   size);
        void  revert     (const void* save);
    };

    //--------------------------------------------------------------------
    // SPARSE SET
    //--------------------------------------------------------------------

    template<typename t>
    class IFB_API sparse_set {

    private:

        struct {
            struct {
                u32* sparse_index;
                u32* hash;
            } dense;
            struct {
                u32* dense_index;
                t*   val;
            } sparse;
        } _data;

        struct {
            u32 sparse;
            u32 dense;
        } _capacity;

        u32 _key_size;
        u32 _count;

    public:

        // static methods
        static u32  memory_requirement (const u32 capacity, const u32 key_size, const f32 max_load_p100);
        static void memory_init        (const u32 capacity, const u32 key_size, const f32 max_load_p100, memory& mem);

        // instance methods
        u32  capacity_sparse (void) const;
        u32  capacity_dense  (void) const;
        u32  key_size        (void) const;
        u32  count           (void) const;
        void validate        (void) const;
        void reset           (void);
        bool insert          (const cchar* key, const t& val);
        void update          (const cchar* key, const t& val);
        bool lookup          (const cchar* key, t&       val) const;
        void remove          (const cchar* key);
    };

    
};

#endif //IFB_COLLECTIONS_HPP