//
//  IAgent.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/29/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__IAgent__
#define __ABMSolar__IAgent__


namespace solar_core
{

class IAgent
{
public:
    virtual void ac_inf_marketing_sei() = 0; /*!< action to request information from SEI when initiative is given from the W */

    
};



} //end of solar_core namespace






#endif /* defined(__ABMSolar__IAgent__) */
