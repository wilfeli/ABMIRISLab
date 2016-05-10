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

namespace solar_tests{


    class WMock: public solar_core::W
    {
    public:
        WMock(std::string path_, std::string mode_): solar_core::W(path_, mode_){};
        std::vector<solar_core::Household*>& get_hhs();
    };


}
#endif /* defined(__ABMSolar__WMock__) */
