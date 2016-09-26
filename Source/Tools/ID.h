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
    
    class UID;
    
    inline bool operator==(const UID& lhs, const UID& rhs); /* do actual comparison */
    inline bool operator!=(const UID& lhs, const UID& rhs){return !operator==(lhs,rhs);};
    inline bool operator< (const UID& lhs, const UID& rhs); /* do actual comparison */
    inline bool operator> (const UID& lhs, const UID& rhs){return  operator< (rhs,lhs);}
    inline bool operator<=(const UID& lhs, const UID& rhs){return !operator> (lhs,rhs);}
    inline bool operator>=(const UID& lhs, const UID& rhs){return !operator< (lhs,rhs);}
    
    
    
    
    /**
     Unique ID
     Own simple id system is implemented
     */
    class UID
    {
        friend bool operator==(const UID& lhs, const UID& rhs);
        friend bool operator< (const UID& lhs, const UID& rhs);
        
    public:
        /** By default creates next id */
        UID();
        
        UID(UID const &id_): id(id_.id), id_string(id_.id_string){};
        
        inline std::string get_string(){return id_string;};
        static UID get_next_id();
        static UID get_uid(std::string);
        static bool is_used_id(std::string id_);
        
        static std::unordered_set<int64_t> used_ids; /*!< list of used ids */
        static bool FLAG_END_INITIALIZATION; /*!< flag to set to true when initialization is done to save on time */
        static bool FLAG_NEW_MODE; /*!< set to true if it is new world creation, otherwise it is false if loaded from save */
        
    private:
        static int64_t id_counter;
        int64_t id = 0;
        std::string id_string = "0"; /*!< string representation of id */
        
        
    };
    
    
    
    inline bool operator==(const UID& lhs, const UID& rhs)
    {
        return (lhs.id == rhs.id);
    };
    
    
    inline bool operator< (const UID& lhs, const UID& rhs)
    {
        return (lhs.id < rhs.id);
    };
    

} //end solar_core namespace




#endif /* defined(__ABMSolar__ID__) */
