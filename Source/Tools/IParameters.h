//
//  IParameters.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/24/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_IParameters_h
#define ABMSolar_IParameters_h

#include "Tools/ExternalIncludes.h"
#include <boost/property_tree/ptree.hpp>


namespace solar_core
{
    
    namespace constants
    {
        const int WAIT_MILLISECONDS_ERROR = 100;
        const int WAIT_MILLISECONDS_LIFE_TICK = 100;
        const int WAIT_MILLISECONDS_DATA_REQUEST = 2000;
        const int WAIT_MILLISECONDS_MARKET_CYCLE = 1000;
        const int WAIT_MILLISECONDS_UIW_PAUSE = 100;
        const int WAIT_CYCLES_VIEW_REQUEST = 10;
        const int NUMBER_AGENT_TYPES_LIFE = 4; /*!< number of agents that update in W::life() */
        static constexpr double SOLAR_INFINITY() {return std::numeric_limits<double>::infinity();}; /*!< could use INFINITY macro constant from <cmath>, but it will be float infinity. see http://en.cppreference.com/w/cpp/header/cmath */
        static constexpr double SOLAR_NAN() {return std::numeric_limits<double>::quiet_NaN();};
        // ... other related constants
        
    } // namespace constants

    
    
    
    
    
    
    
    
    
    /**
     
     @DevStage1 add factories enum<->std::string
     
     @DevStage2 think about splitting enum into multiple enums
     
     */
    enum class EParamTypes: int64_t
    {
        /** Average yearly income for hh */
        Income,
        

        /**  Number of humans in a household */
        N_H,
        
        
        /** Credit score */
        CreditScore,
        
        
        /** Electricity Bill */
        ElectricityBill,
        
        
        /** Roof size */
        RoofSize,
        
        /** Roof age */
        RoofAge,
        
        
        /** If Household is very interested in SP */
        HHMarketingStateHighlyInterested,
        
        
        /** If Household is just interested in SP and ready to ask for quotes */
        HHMarketingStateInterested,
        
        
        /** If Household is not interested in installing SP */
        HHMarketingNotInterested,
        
        
        /** State of a quoting stage for HH: actively requesting information */
        ActiveQuoting,
        
        
        /** State of a quoting stage for HH: not requesting quotes, might be analysing them or committed to the project */
        InactiveQuoting,
        
        
        /** State of a quoting stage for HH: decision on reroofing old roof */
        HHDecisionReroof,
        
        
        /** Maximum number of visits per unit of time for HH */
        HHMaxNVisitsPerTimeUnit,
        
        
        /** Thetas for decisions: decision on preliminary quotes */
        HHDecPreliminaryQuote,
        
        
        /** State of a Project: preliminary quotes has been requested via online */
        RequestedOnlineQuote,
        
        /** State of a Project: preliminary quotes need to been requested via phone */
        RequestPreliminaryQuote,
        
        /** State of a Project: preliminary quotes has been requested via phone */
        RequestedPreliminaryQuote,
        
        /** State of a Project: preliminary quotes has been provided via online */
        ProvidedOnlineQuote,
        
        /** State of a Project: preliminary quotes has been provided via site visit */
        ProvidedPreliminaryQuote,
        
        /** State of a Project: site visit is scheduled */
        ScheduledFirstSiteVisit,
        
        /** State of a Project: collected information after first site visit */
        CollectedInfFirstSiteVisit,
        
        /** State of a Project: Roof needs to be changed */
        RequiredHHReroof,
        
        /** State of a Project: Homeowner agreed to reroof thus waiting for reroofing */
        WaitingHHReroof,
        
        /** State of a Project: project is accepted for further development*/
        AcceptedPreliminaryQuote,
        
        /** State of a Project: created design for the project*/
        DraftedDesign,
        
        /** State of a Project: accepted design for the project*/
        AcceptedDesign,
        
        /** State of a Project: requested permit for the project*/
        RequestedPermit,
        
        /** State of a Project: granted permit for the project*/
        GrantedPermit,
        
        /** State of a Project: scheduled installation */
        ScheduledInstallation,
        
        /** State of a Project: project is installed */
        Installed,
        
        /** State of a Project: project closed for any reason */
        ClosedProject,
        
        
        
        
        /** Estimated price of installation after the online quote was formed */
        OnlineQuotePrice,
        
        
        /** Preliminary estimation of savings based on utility bill for online quote */
        OnlineQuoteEstimatedSavings,
        
        
        /** Estimated price of installation after the preliminary quote with site visit was made*/
        PreliminaryQuotePrice,
        
        
        /** Preliminary estimation of savings based on utility bill after site visit */
        PreliminaryQuoteEstimatedSavings,
        
        
        /** Industry standard price per watt that is used in estimating installation cost */
        EstimatedPricePerWatt,
        
        
        /** Industry standard PV price, used in online estimation */
        AveragePVPrice,
        
        
        /** Industry standard PV price, used in online estimation */
        AveragePVCapacity,
        
        
        /** Electricity price per watt for the demand of electricity from the utility company (UC) */
        ElectricityPriceUCDemand,
        
        
        /** Electricity price per watt for the supply of electricity to the utility company (UC) */
        ElectricityPriceUCSupply,
        
        
        /** Expected inflation rate over the next 20 years */
        InflationRate,
        
        
        /** DC to AC loss */
        DCtoACLoss,
        
        
        
        
        
        
        
        /** Small SEI agent - such as mom and pop shop */
        SEISmall,
        
        
        /** Large SEI */
        SEILarge,

        
        /** Parameters of a SEI, such as processing time before preliminary quote is formed after site visit */
        SEIProcessingTimeRequiredForPreliminaryQuote,
        
        /** Parameters of a SEI, such as processing time before action is taken to schedule first site visit */
        SEIProcessingTimeRequiredForSchedulingFirstSiteVisit,
        
        /** Parameters of a SEI, processing time before design is created after project is accepted by HH */
        SEIProcessingTimeRequiredForDesign,
        
        /** Maximum number of visits per unit of time for SEI */
        SEIMaxNVisitsPerTimeUnit,
        
        /** Maximum number of installation per unit of time for SEI */
        SEIMaxNInstallationsPerTimeUnit,
        
        /** Maximum age of the roof to accept for installation */
        SEIMaxRoofAge,

        /** Parameters of a SEI, frequency of revising designs */
        SEIFrequencyUpdateDesignTemplates,
        
        
        /** High efficiency choice of a panel */
        SEIHighEfficiencyDesign,
        
        
        /** Mid range efficiency choice of a panel */
        SEIMidEfficiencyDesign,
        
        
        /** Low efficiency choice of a panel */
        SEILowEfficiencyDesign,
        
        
        
        
        
        
        /** State of Payments: all payments on time */
        PaymentsOnTime,
        
        
        
        /** Empty enum for completeness */
        None
        
    };
    
    enum class EConstraintParams: int64_t
    {
        
        /** Maximum number of ticks to collect quotes (online) */
        MaxNTicksToCollectQuotes,
        
        
        /** Maximum number of open projects to consider */
        MaxNOpenProjectsHH,
        
        
        /** Maximum number of preliminary quotes to request from received online quotes */
        MaxNRequestedPreliminaryFromOnlineQuotes,
        
        
        /** Minimum number of preliminary quotes to consider */
        MinNReceivedPreliminaryQuotes,
        
        
        /** Minimum number of designs to consider */
        MinNReceivedDesings,
        
        
        /** Maximum waiting time before the visit to get preliminary quote is made */
        MaxLengthWaitPreliminaryQuote,
        
        
        /** Maximum forecasting horizon for installations */
        MaxLengthPlanInstallations,
        
        None,
    };
    
    
    typedef int64_t TimeUnit;
    typedef boost::property_tree::ptree PropertyTree; //container for parameters, properties
    
    
    
    typedef std::underlying_type<EParamTypes>::type EParamTypes_type;
    
    std::ostream &operator<<(std::ostream& is, const EParamTypes & item);
    std::istream& operator>> (std::istream&  os, EParamTypes & item);

} //end of namespace solar_core
#endif
