//
//  UI.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/7/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__UI__
#define __ABMSolar__UI__

#include "Tools/ExternalIncludes.h"


namespace solar_core
{
    class W;
}


namespace solar_ui
{
    
    class UI
    {
    public:
        UI(){};
        void init(solar_core::W* w_);
        
        
        void collect_inf();
        void save(std::string path_to_save_file_ = "");
        
    protected:
        solar_core::W* w;
        
    };
    
    
    
}



#endif /* defined(__ABMSolar__UI__) */
