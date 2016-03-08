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
 
 
 @DevStage1 think, for now this message is used in the first stage of quoting. If decide to use in the next stage might have to add field to indicate from what stage of quoting this message is. Alternative is to have separate classes for different quotes from different stages. Actually like it more. Need to have base message class to avoid multiple containers for them. But at the same time it might be actually more efficient to have multiple containers for different stages of quoting. It will make search and compare much quicker...
 
 
 
 */
class MesMarketingSEIPreliminaryQuote
{
public:
    std::map<EParamTypes, double> params; /*!< parameters of a quote */
    
    
    
    
};
    
    
    
/**
 
 Returns basic parameters of HH, is used in online quote process
 
 
 @DevStage3 if decide to have agent specific messages might later collapse them all into the same class
 
 
 */
class MesStateBaseHH
{
public:
    std::map<EParamTypes, double> params; /*!< basic parameters of HH */
    
};
    
    
    
} //end namespace solar_core

#endif /* defined(__ABMSolar__IMessage__) */
