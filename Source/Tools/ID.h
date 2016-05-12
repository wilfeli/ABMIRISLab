//
//  ID.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/11/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__ID__
#define __ABMSolar__ID__


#include "Tools/ExternalIncludes.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace solar_core
{
    /**
     Unique ID
     Own simple id system is implemented
     */
    class UID
    {
    public:
        UID() = default;
        
        UID(UID const &id_): id(id_.id), id_string(id_.id_string){};
        
        inline std::string get_string(){return id_string;};
        static UID get_next_id();
        static UID get_uid(std::string);
        static bool is_used_id(std::string id_);
        
        static std::unordered_set<long> used_ids; /*!< list of used ids */
        static bool FLAG_END_INITIALIZATION; /*!< flag to set to true when initialization is done to save on time */
        static bool FLAG_NEW_MODE; /*!< set to true if it is new world creation, otherwise it is false if loaded from save */
        
    private:
        static long id_counter;
        long id = 0;
        std::string id_string = "0"; /*!< string representation of id */
        
        
    };
} //end solar_core namespace




#endif /* defined(__ABMSolar__ID__) */
