//
//  UIBL.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/29/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__UIBL__
#define __ABMSolar__UIBL__



#include "Tools/ExternalIncludes.h"


namespace solar_core
{
    class WEE;
}


namespace solar_ui
{
    
    class UIBL
    {
    public:
        UIBL(){};
        void init(solar_core::WEE* w_);
        
        
        void collect_inf();
        void save(std::string path_to_save_file_ = "");
        
    
        solar_core::WEE* w;
    protected:    
    };
    
    
    
}


#endif /* defined(__ABMSolar__UIBL__) */
