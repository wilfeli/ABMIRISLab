//
//  ParsingTools.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__ParsingTools__
#define __ABMSolar__ParsingTools__

#include <vector>
#include <map>
#include <string>

namespace solar_core
{
    namespace tools
    {
        void parse_model_file(std::string path_to_file, std::map<std::string, std::string>& parsed_model);
        
    } //tools
    
} //solar_core


#endif /* defined(__ABMSolar__ParsingTools__) */
