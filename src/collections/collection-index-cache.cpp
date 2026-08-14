#include "ifb-collections.hpp"
#include "ifb-types.hpp"
#include <cassert>

namespace ifb {

    //////////////////////////
    // NOTE(SAM):
    // true = used
    // false = free
    //////////////////////////

    void
    index_cache::memory_init(
        const u32 capacity,
        bool*     index_array) {

        assert(capacity != 0 && index_array != NULL); 
        _capacity = capacity;
        _index = index_array;

        reset();
    }

    void
    index_cache::reset(void) {

        for (
            u32 i = 0;
            i < _capacity;
            ++i) {

            _index[i] = false;
        }
    }

    void
    index_cache::stack_init(
        const u32 capacity,
        stack& s) {

        _capacity = capacity;
        _index    = s.push_struct<bool>(capacity);
        assert(_index);
    }

    void 
    index_cache::set_index_free(
        const u32 index) {
    
        validate();
        assert(index < _capacity);

        _index[index] = false;
    }

    void 
    index_cache::set_index_used(
        const u32 index) {

        validate();
        
        _index[index] = true;
    }

    u32
    index_cache::get_next_free(
        void) const {

        validate();

        u32 index = INVALID_INDEX;
        for (
            u32 i = 0;
            i < _capacity;
            ++i
        ) {
            if (!_index[i]) {
                _index[i] = true;
                index = i;
                break;
            }
        }

        return(index);
    }
    
    u32
    index_cache::count_used(
        void) const {
    
        validate();

        u32 count = 0;
        for (
            u32 i = 0;
            i < _capacity;
            ++i) {

           if (_index[i]) ++count; 
        }

        return(count);
    }

    u32
    index_cache::count_free(
        void) const {
    
        validate();

        u32 count = 0;
        for (
            u32 i = 0;
            i < _capacity;
            ++i) {

           if (!_index[i]) ++count; 
        }

        return(count);
    }

    u32
    index_cache::capacity(
        void) const {

        validate();
        return(_capacity);
    } 

    bool
    index_cache::is_index_free(
        const u32 index) const {
    
        validate();
        assert(index < _capacity);
        return(!_index[index]);
    }
};
