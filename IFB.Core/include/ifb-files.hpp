#ifndef IFB_FILES_HPP
#define IFB_FILES_HPP

#include "ifb-config.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES 
    //--------------------------------------------------------------------

    struct file_path;

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------

   struct file_path {
        cchar cstr[IFB_CONFIG_FILE_PATH_SIZE];
    };

};

#endif //IFB_FILES_HPP
