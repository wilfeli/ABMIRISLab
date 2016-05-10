//
//  ParsingTools.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include <iostream>
#include <fstream>


#include "Tools/ParsingTools.h"



using namespace solar_core;

void
tools::parse_model_file(std::string path_to_file, std::map<std::string, std::string>& parsed_model)
{
    //read path to the saved w file
    std::ifstream in_file(path_to_file);
    std::string s, key, value;
    
    //extract information from .model file
    while (std::getline(in_file, s))
    {
        // Extract the key value
        std::string::size_type begin = s.find_first_not_of( " \f\t\v" );
        std::string::size_type end = s.find( '=', begin );
        key = s.substr( begin, end - begin );
        
        // (No leading or trailing whitespace allowed)
        key.erase(key.find_last_not_of( " \f\t\v" ) + 1 );
        
        // No blank keys allowed
        if (key.empty()) continue;
        
        // Extract the value (no leading or trailing whitespace allowed)
        begin = s.find_first_not_of( " \f\n\r\t\v", end + 1 );
        end   = s.find_last_not_of(  " \f\n\r\t\v" ) + 1;
        
        value = s.substr( begin, end - begin );
        parsed_model[key] = value;
    };
    
    in_file.close();
}
