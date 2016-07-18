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

    
    class PVProjectFlat;
    class SEIBL;
    class SEMBL;
    class H;
    
    

class WEE: public W
{
public:
    virtual void init() override;
    virtual void life_hos() override; /*!< life of Homeowners */
    
    
    double get_inf(EDecParams type_, SEIBL* agent_);
    
protected:
    std::vector<std::shared_ptr<PVProjectFlat>> pool_projects;
    std::size_t i_pool_projects;
    
    
    
    std::vector<H*> hos;/*!< all H agents */
    std::vector<SEIBL*> seis;/*!< all SEI agents */
    std::vector<SEMBL*> sems; /*!< all SEM */

    
    
};

} //end namespace solar_core


#endif /* defined(__ABMSolar__WEE__) */
