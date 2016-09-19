//
//  WMock.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 5/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__WMock__
#define __ABMSolar__WMock__

#include "UI/W.h"
#include "UI/HelperW.h"

namespace solar_tests{


    class WMock: public solar_core::W
    {

    public:
        WMock(std::string path_, solar_core::HelperW* helper_, std::string mode_): solar_core::W(path_, helper_, mode_){};
        std::vector<solar_core::Homeowner*>& get_hos();
        std::vector<solar_core::SEI*>& get_seis();
        
        static WMock* create(std::string path_, std::string mode_);
        
    protected:
        
    };


}
#endif /* defined(__ABMSolar__WMock__) */
