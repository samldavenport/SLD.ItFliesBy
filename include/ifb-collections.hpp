#ifndef IFB_COLLECTIONS_HPP
#define IFB_COLLECTIONS_HPP

#include <sld.hpp>

#include "ifb-types.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // ARRAY LIST
    //--------------------------------------------------------------------

    template<typename t>
    class IFB_API array_list {
    
    private:
        
        t*  _elmnt;
        u32 _capacity;
        u32 _count;

    public:

        void init          (t* elmnts, const u32 capacity);
        void validate      (void)                        const;
        u32  capacity      (void)                        const;
        u32  count         (void)                        const;
        bool index_of      (const t&  elmnt, u32& index) const;
        void reset         (void);
        t&   get           (const u32 index);
        bool add           (const t&  elmnt);
        void remove        (const t&  elmnt);
        void remove_at     (const u32 index);

        inline t& operator[] (const u32 index) {
            assert(index < count);
            return(elmnt[index]);
        }
    };

    //--------------------------------------------------------------------
    // STACK
    //--------------------------------------------------------------------

    class IFB_API stack {
    
    private:

        memory _mem;
        u32    _pos;
        u32    _save;

    public:

        // instance methods
        void  init       (memory& mem);
        void  validate   (void) const;
        u32   size_total (void) const;
        u32   size_used  (void) const;
        u32   size_free  (void) const;
        void* head       (void) const;
        void* tail       (void) const;
        u32   save       (void);
        void  reset      (void);
        void* push       (const u32 size);
        void  pull       (const u32 size);
        void  revert     (const u32 save);
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

        // instance methods
        void init            (const u32 capacity, const u32 key_size, const f32 max_load_p100, memory& mem);
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