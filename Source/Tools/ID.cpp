//
//  ID.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/11/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "ID.h"

using namespace solar_core;


long UID::id_counter = 1; //zero is reserved as default uid
bool UID::FLAG_END_INITIALIZATION = false;
std::unordered_set<long> UID::used_ids;
bool UID::FLAG_NEW_MODE = true;


bool
UID::is_used_id(std::string id_)
{
    return (used_ids.find(std::stol(id_)) != used_ids.end());
}


//simple long as an id

UID
UID::get_next_id()
{
    UID id;
    
    ++UID::id_counter;
    
    if (!FLAG_END_INITIALIZATION)
    {
        while (used_ids.find(UID::id_counter) != used_ids.end())
        {
            ++UID::id_counter;
        };
        used_ids.insert(UID::id_counter);
    };
    
    id.id = id_counter;
    id.id_string = std::to_string(id.id);
    
    return id;
}



UID
UID::get_uid(std::string str_)
{
    //create uid from string
    UID uid;
    
    uid.id_string = str_;
    uid.id = std::stol(str_);
    
    //add to used ids
    used_ids.insert(uid.id);
    
    return uid;
}


