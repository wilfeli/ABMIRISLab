//
//  IInstitute.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/29/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__IInstitute__
#define __ABMSolar__IInstitute__



namespace solar_core
{
    
class IInstitute
{
public:
    virtual ~IInstitute() = default;
    virtual void act_tick() = 0;
};
    
    
    
} //end of solar_core namespace






#endif /* defined(__ABMSolar__IInstitute__) */
