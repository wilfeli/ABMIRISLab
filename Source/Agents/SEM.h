//
//  SEM.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_SEM_h
#define ABMSolar_SEM_h

#include "Tools/IParameters.h"


namespace solar_core
{

    
class W;
    
/**
 
 @wp Research for the structure of costs 
 
 Hanwha Q CELLS, 3d quater 2015 report. http://investors.hanwha-qcells.com/releasedetail.cfm?ReleaseID=943563
 Total all-in processing costs approached US$0.39 per watt in September for in-house production

 
 
 
 
 */
class SEM
{
public:
    //@{
    /**
     
     Initializations
     
     */
    
    SEM(const PropertyTree& pt_, W* w_);
    void init(W* w_);
    
    //@}
    
};
} //end namespace solar_core


#endif
