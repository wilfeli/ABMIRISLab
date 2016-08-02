//
//  ParsingTools.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__ParsingTools__
#define __ABMSolar__ParsingTools__

#include "Tools/ExternalIncludes.h"
#include "Tools/Serialize.h"

namespace solar_core
{
    namespace tools
    {
        void parse_model_file(std::string path_to_file, std::map<std::string, std::string>& parsed_model);
        
        
        template <class T>
        void parse_csv_file(std::string path_to_file, std::vector<std::vector<T>>& parsed_file)
        {
            //read path to the saved w file
            std::ifstream in_file(path_to_file);
            std::string s;
//            const std::regex re{"((?:[^\\\\,]|\\\\.)*?)(?:,|$)"};
//            const std::regex re{"(\d+)(,\s*\d+)*"};
            const std::regex re("[\\s,]+");
            auto n = 0;
            
            //skip first line
            std::getline(in_file, s);
            
            //extract information from .model file
            while (std::getline(in_file, s))
            {
                parsed_file.push_back(std::vector<T>{});
                const std::sregex_token_iterator end;
                
                //skip first column
                
                
                for (std::sregex_token_iterator iter(s.begin(), s.end(), re, -1); iter != end; ++iter)
                {
                    
//#ifdef DEBUG
//                    if (n < 20)
//                    {
//                        std::cout << iter->str() << std::endl;
//                        ++n;
//                    };
//                    
//#endif
                    
                    parsed_file.back().push_back(serialize::DeserializeValue<T>::deserialize_value(*iter));
                };
                
            };
            
            
            
//#ifdef DEBUG
//            
//            for (auto i = 0; i < 10; ++i)
//            {
//                for (auto j = 0; j < parsed_file[i].size(); ++j)
//                {
//                    std::cout << parsed_file[i][j] << " ";
//                };
//                
//                std::cout << std::endl;
//            };
//            
//#endif
            
            
            
        }
        
        
        
        
    } //tools
    
} //solar_core


#endif /* defined(__ABMSolar__ParsingTools__) */
