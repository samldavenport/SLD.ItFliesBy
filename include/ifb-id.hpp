#ifndef IFB_ID_HPP
#define IFB_ID_HPP

#include "sld.hpp"

namespace ifb {

    struct id32;
    struct entity_id;
    struct quad_id;
    struct file_id;

    struct id32 {
        u32 val;

        inline bool  operator== (const id32& other) const;
        inline bool  operator!= (const id32& other) const;
        inline bool  operator== (const u32& hash)   const;
        inline bool  operator!= (const u32& hash)   const;
        inline id32& operator=  (const u32& hash);
        inline id32& operator=  (const id32& id);
    };

    struct entity_id : ifb32 { };
    struct quad_id   : ifb32 { };
    struct file_id   : ifb32 { };


    //--------------------------------------------------------------------
    // ID 32 INLINE OPERATORS
    //--------------------------------------------------------------------

    inline bool
    id32::operator==(
        const id32& other) const {

        return(this->val == other.val);
    }

    inline bool
    id32::operator!=(
        const id32& other) const {

        return(this->val != other.val);
    }

    inline bool
    id32::operator==(
        const u32& val) const {

        return(this->val == val);
    }

    inline bool
    id32::operator!=(
        const u32& val) const {

        return(this->val != val);
    }

    inline id32&
    id32::operator=(
        const u32& val) {

        this->val = val;
        return(*this);
    }

    inline id32&
    id32::operator= (
        const id32& id) {

        this->val = id.val;
        return(*this);
    }

    struct entity_id : id32 { };
    struct quad_id   : id32 { };
    struct file_id   : id32 { };


};

#endif //IFB_ID_HPP