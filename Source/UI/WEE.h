//
//  WEE.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__WEE__
#define __ABMSolar__WEE__


#include "UI/W.h"

namespace solar_core {


class WEE: public W
{
public:
    virtual void life_hhs() overwrite; /*!< life of households */
protected:
    std::vector<std::shared_ptr<PVProjectFlat>> pool_projects;
    std::size_t i_pool_projects;
    
};

} //end namespace solar_core


#endif /* defined(__ABMSolar__WEE__) */
