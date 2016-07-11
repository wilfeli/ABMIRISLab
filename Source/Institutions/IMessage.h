//
//  IMessage.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/29/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__IMessage__
#define __ABMSolar__IMessage__


#include "Tools/ExternalIncludes.h"


#include "Tools/IParameters.h"


namespace solar_core
{
    
class SEI;
class PVDesign;
    

    
class MesPayment{
public:
    
    MesPayment(double payment_):q(payment_){}
    
    double q = 0.0;
protected:
};
    
    
    
class MesFinance
{
public:
    std::vector<double> schedule_payments;
    EParamTypes state_payments;
protected:
};
    
    
    
    class MesSellOrder
    {
    public:
        MesSellOrder(std::string item_, double qn_, SEI* buyer_):item(item_), qn(qn_), buyer(buyer_){};
        std::string item;
        double qn;
        SEI* buyer;
    protected:
    };
    
    
class MesDesign
{
public:
    MesDesign(std::shared_ptr<PVDesign> design_): design(design_){}
    std::shared_ptr<PVDesign> design;
protected:
};
    
    
    
/**
 
 General information about SEI
 
 */
class MesMarketingSEI
{
public:
    MesMarketingSEI(SEI* agent_, const EParamTypes& sei_type_):agent(agent_), sei_type(sei_type_){}
    SEI* agent; /*!< agent whose marketing information is presented */
    EParamTypes sei_type; /*!< depending on the type will have online quotes or not */
    
};



/**
 
 Message with the quote
 
 
 @DevStage2 think, for now this message is used in the first stage of quoting. If decide to use in the next stage might have to add field to indicate from what stage of quoting this message is. Alternative is to have separate classes for different quotes from different stages. Actually like it more. Need to have base message class to avoid multiple containers for them. But at the same time it might be actually more efficient to have multiple containers for different stages of quoting. It will make search and compare much quicker...
 
 
 
 */
class MesMarketingSEIOnlineQuote
{
public:
    std::map<EParamTypes, double> params; /*!< parameters of a quote */
    
    
    
    
};
    
/**
 
 Message with the quote for preliminary stage, separate class for now
 
 
 */
class MesMarketingSEIPreliminaryQuote
{
public:
    std::map<EParamTypes, double> params; /*!< parameters of a quote */
    
    
};
    
    
/**
 
 Returns basic parameters of HO, is used in online quote process
 
 
 @DevStage3 if decide to have agent specific messages might later collapse them all into the same class
 
 
 */
class MesStateBaseHO
{
public:
    std::map<EParamTypes, double> params; /*!< basic parameters of HO */
    
};
    
    
    
} //end namespace solar_core

#endif /* defined(__ABMSolar__IMessage__) */
