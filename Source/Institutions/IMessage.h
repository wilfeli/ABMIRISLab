//
//  IMessage.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/29/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__IMessage__
#define __ABMSolar__IMessage__


#include <map>


#include "Tools/IParameters.h"


namespace solar_core
{
    
class SEI;


/**
 
 General information about SEI
 
 */
class MesMarketingSEI
{
public:
    SEI* sei; /*!< agent whose marketing information is presented */
    
    
};



/**
 
 Message with the quote
 
 
 */
class MesMarketingQuote
{
public:
    std::map<EParamTypes, double> params; /*!< parameters of a quote */
    
    
    
    
};
    
    

} //end namespace solar_core

#endif /* defined(__ABMSolar__IMessage__) */
