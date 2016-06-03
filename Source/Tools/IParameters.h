////Kelley reviewed 4/24/16 see lines 75, 88, 113, 250, 296, 349
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

#ifdef DEBUG

#include <iostream>

#endif

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
        const double NUMBER_DAYS_IN_MONTH = 30.4375;
        const double NUMBER_DAYS_IN_YEAR = 365.25;
        const int NUMBER_WATTS_IN_KILOWATT = 1000;
        const int NUMBER_AGENT_TYPES_LIFE = 5; /*!< number of agents that update in W::life(), hh, sei, sem, g, market */
        static constexpr double SOLAR_INFINITY() {return std::numeric_limits<double>::infinity();}; /*!< could use INFINITY macro constant from <cmath>, but it will be float infinity. see http://en.cppreference.com/w/cpp/header/cmath */
        static constexpr double SOLAR_NAN() {return std::numeric_limits<double>::quiet_NaN();};
        // ... other related constants
        
    } // namespace constants

    
    
    
    
    
    
    
    
    
    /**
     
     factories enum<->std::string
     
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
        
        
		/*Why couldn't we use an enum class to show different levels of interest for HHMarketingState?*/
		
		/** If Household is very interested in SP */
        HHMarketingStateHighlyInterested,
        
        
        /** If Household is just interested in SP and ready to ask for quotes */
        HHMarketingStateInterested,
        
        
        /** If Household is not interested in installing SP */
        HHMarketingNotInterested,
        
<<<<<<< HEAD
        
        /** If Household is already installed */
        HHMarketingCommitedToInstallation,

        
=======
		/*Is it possible to have a bool quoting stage active = true, inactive = false?*/

>>>>>>> 9fadf023062505cb443534457ab9d4d3cc1b7bfc
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
        
        /** State of a Project: scheduled visit to grant permit*/
        ScheduledPermitVisit,
        
        /** State of a Project: collected information for granting permit */
        CollectedInfPermitVisit,
        
        /** State of a Project: scheduled installation */
        ScheduledInstallation,
        
        /** State of a Project: need to schedule installation */
        ScheduleInstallation,
        
        /** State of a Project: need to purchase materials */
        PendingMaterials,
        
        /** State of a Project: project is installed */
        Installed,
        
        /** State of a Project: request permit to install project */
        RequestedPermitForInstallation,
        
        /** State of a Project: project could be installed */
        GrantedPermitForInstallation,
        
        /** State of a Project: request inspection after installation */
        RequestedInspectionAfterInstallation,
        
        /** State of a Project: passed inspection after installation */
        PassedInspectionAfterInstallation,
        
        /** State of a Project: request permission to connect to the grid */
        RequestedPermitForInterconnection,
        
        /** State of a Project: granted permit to connect to the grid */
        GrantedPermitForInterconnection,
        
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
        
        
        /** Industry standard solar module size, used in online estimation */
        AveragePVCapacity,
        
        
        /** Electricity price per watt for the demand of electricity from the utility company (UC) */
        ElectricityPriceUCDemand,
        
        
        /** Electricity price per watt for the supply of electricity to the utility company (UC) */
        ElectricityPriceUCSupply,
        
        
        /** Expected inflation rate over the next 20 years */
        InflationRate,
        
        
        /** DC to AC loss */
        DCtoACLoss,
        
        
        /** Length of a definition for degradation rate */
        DegradationDefinitionLength,
        
        
        
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
        
		/*Could you also use an enumerated class for high efficiency, mid and low efficiency design?*/

        /** High efficiency choice of a panel */
        SEIHighEfficiencyDesign,
        
        
        /** Mid range efficiency choice of a panel */
        SEIMidEfficiencyDesign,
        
        
        /** Low efficiency choice of a panel */
        SEILowEfficiencyDesign,
        
    
        
        /** SEM parameters: Length of a production cycle  */
        SEMFrequencyProduction,
        
        
        /** SEM parameters: Amount of solar panels produced during each cycle */
        SEMProductionQuantity,
        
        
        /** SEM parameters: Length of a research cycle */
        SEMFrequencyResearchTemplates,
        
        /** SEM parameters: Number of modifications researched per cycle*/
        SEMNSolarPanelsResearch,
        
        /** SEM parameters: Amount of percentage that efficiency goes up after research */
        SEMEfficiencyUpgradeResearch,
        
        /** SEM parameter: Frequency of making price decisions */
        SEMFrequencyPriceDecisions,
        
        /** SEM parameter: Base level of efficiency with 1.0 as price adjustment */
        SEMPriceBaseEfficiency,
        
        /** SEM parameter: Marrkup to the price for the increased efficiency */
        SEMPriceMarkupEfficiency,
        
        
        
        
        /** G parameters: processing time before visit is scheduled */
        GProcessingTimeRequiredForSchedulingPermitVisit,
        
        /** G parameters: maximum number of visits per time unit */
        GMaxNVisitsPerTimeUnit,
        
        /** G parameters: time to process permit */
        GProcessingTimeRequiredForProcessingPermit,
        
        /** G parameters: processing time for granting installation permit */
        GProcessingTimeRequiredForGrantingPermitForInstallation,
        
        /** Utility parameters: time to investigate request for interconnection */
        UtilityProcessingTimeRequiredForPermit,
        
        /** Utility parameters: Max capacity of the grid */
        UtilityMaxCapacity,
        
        /** Utility parameters: Current capacity of the grid */
        UtilityCurrentCapacity,
        
        
        /** State of Payments: all payments on time */
        PaymentsOnTime,
        
        
        
        /** Marketing part: number of HH to draw per unit of time to push marketing information */
        MarketingMaxNToDrawPerTimeUnit,
        
        
        /** Technology block: inverters technology type standard */
        TechnologyInverterStandard,
        
        
        /** Technology block: inverters technology type micro */
        TechnologyInverterMicro,
        
        
        /** Empty enum for completeness */
        None
        
    };
    
    enum class EConstraintParams: int64_t
    {
        
        /** Maximum number of ticks to collect quotes (online) */
        MaxNTicksToCollectQuotes, //Can you explain what this variable means?
        
        
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
        
        
        /** Maximum forecasting horizon for scheduling permit visits */
        MaxLengthWaitPermitVisit,
        
        
        /** Maximum length of collecting past information by SEM */
        SEMMaxLengthRecordHistory,
        
        
        
        
        
        
        None,
    };
    
    
    typedef int64_t TimeUnit;
    typedef boost::property_tree::ptree PropertyTree; //container for parameters, properties
    
    
    
    typedef std::underlying_type<EParamTypes>::type EParamTypes_type;
    
    std::ostream &operator<<(std::ostream& is, const EParamTypes & item);
    std::istream& operator>> (std::istream&  os, EParamTypes & item);
    
    
    
    
    class EnumFactory{
    public:
        static EParamTypes ToEParamTypes(std::string param_type)
        {
            //case insensitive
            std::transform(param_type.begin(), param_type.end(), param_type.begin(), ::tolower);
            
            if (param_type == "eparamtypes::income") //why do we only make income and N_H case insensitive?
            {
                return EParamTypes::Income;
            }
            else if (param_type == "eparamtypes::n_h")
            {
                return EParamTypes::N_H;
            }
            else if (param_type == "eparamtypes::hhdecpreliminaryquote")
            {
                return EParamTypes::HHDecPreliminaryQuote;
            }
            else if (param_type == "eparamtypes::creditscore")
            {
                return EParamTypes::CreditScore;
            }
            else if (param_type == "eparamtypes::electricitybill")
            {
                return EParamTypes::ElectricityBill;
            }
            else if (param_type == "eparamtypes::hhmaxnvisitspertimeunit")
            {
                return EParamTypes::HHMaxNVisitsPerTimeUnit;
            }
            else if (param_type == "eparamtypes::hhmarketingstatehighlyinterested")
            {
                return EParamTypes::HHMarketingStateHighlyInterested;
            }
            else if (param_type == "eparamtypes::inactivequoting")
            {
                return EParamTypes::InactiveQuoting;
            }
            else if (param_type == "eparamtypes::seihighefficiencydesign")
            {
                return EParamTypes::SEIHighEfficiencyDesign;
            }
            else if (param_type == "eparamtypes::seimidefficiencydesign")
            {
                return EParamTypes::SEIMidEfficiencyDesign;
            }
            else if (param_type == "eparamtypes::seilowefficiencydesign")
            {
                return EParamTypes::SEILowEfficiencyDesign;
            }
            else if (param_type == "eparamtypes::seismall")
            {
                return EParamTypes::SEISmall;
            }
            else if (param_type == "eparamtypes::seilarge")
            {
                return EParamTypes::SEILarge;
            }
            else if (param_type == "eparamtypes::averagepvcapacity")
            {
                return EParamTypes::AveragePVCapacity;
            }
            else if (param_type == "eparamtypes::estimatedpriceperwatt")
            {
                return EParamTypes::EstimatedPricePerWatt;
            }
            else if (param_type == "eparamtypes::seifrequencyupdatedesigntemplates")
            {
                return EParamTypes::SEIFrequencyUpdateDesignTemplates;
            }
            else if (param_type == "eparamtypes::seimaxninstallationspertimeunit")
            {
                return EParamTypes::SEIMaxNInstallationsPerTimeUnit;
            }
            else if (param_type == "eparamtypes::seimaxnvisitspertimeunit")
            {
                return EParamTypes::SEIMaxNVisitsPerTimeUnit;
            }
            else if (param_type == "eparamtypes::seimaxroofage")
            {
                return EParamTypes::SEIMaxRoofAge;
            }
            else if (param_type == "eparamtypes::seiprocessingtimerequiredfordesign")
            {
                return EParamTypes::SEIProcessingTimeRequiredForDesign;
            }
            else if (param_type == "eparamtypes::seiprocessingtimerequiredforpreliminaryquote")
            {
                return EParamTypes::SEIProcessingTimeRequiredForPreliminaryQuote;
            }
            else if (param_type == "eparamtypes::seiprocessingtimerequiredforschedulingfirstsitevisit")
            {
                return EParamTypes::SEIProcessingTimeRequiredForSchedulingFirstSiteVisit;
            }
            else if (param_type == "eparamtypes::gprocessingtimerequiredforprocessingpermit")
            {
                return EParamTypes::GProcessingTimeRequiredForProcessingPermit;
            }
            else if (param_type == "eparamtypes::gprocessingtimerequiredforschedulingpermitvisit")
            {
                return EParamTypes::GProcessingTimeRequiredForSchedulingPermitVisit;
            }
            else if (param_type == "eparamtypes:gmaxnvisitspertimeunit")
            {
                return EParamTypes::GMaxNVisitsPerTimeUnit;
            }
            else if (param_type == "eparamtypes::dctoacloss")
            {
                return EParamTypes::DCtoACLoss;
            }
            else if (param_type == "eparamtypes::degradationdefinitionlength")
            {
                return EParamTypes::DegradationDefinitionLength;
            }
            else if (param_type == "eparamtypes::electricitypriceucdemand")
            {
                return EParamTypes::ElectricityPriceUCDemand;
            }
            else if (param_type == "eparamtypes::inflationrate")
            {
                return EParamTypes::InflationRate;
            }
            else if (param_type == "eparamtypes::marketingmaxntodrawpertimeunit")
            {
                return EParamTypes::MarketingMaxNToDrawPerTimeUnit;
            }
            else if (param_type == "eparamtypes::semefficiencyupgraderesearch")
            {
                return EParamTypes::SEMEfficiencyUpgradeResearch;
            }
            else if (param_type == "eparamtypes::semfrequencypricedecisions")
            {
                return EParamTypes::SEMFrequencyPriceDecisions;
            }
            else if (param_type == "eparamtypes::semfrequencyproduction")
            {
                return EParamTypes::SEMFrequencyProduction;
            }
            else if (param_type == "eparamtypes::semfrequencyresearchtemplates")
            {
                return EParamTypes::SEMFrequencyResearchTemplates;
            }
            else if (param_type == "eparamtypes::semnsolarpanelsresearch")
            {
                return EParamTypes::SEMNSolarPanelsResearch;
            }
            else if (param_type == "eparamtypes::sempricebaseefficiency")
            {
                return EParamTypes::SEMPriceBaseEfficiency;
            }
            else if (param_type == "eparamtypes::sempricemarkupefficiency")
            {
                return EParamTypes::SEMPriceMarkupEfficiency;
            }
            else if (param_type == "eparamtypes::semproductionquantity")
            {
                return EParamTypes::SEMProductionQuantity;
            }
            else if (param_type == "eparamtypes::utilitycurrentcapacity")
            {
                return EParamTypes::UtilityCurrentCapacity;
            }
            else if (param_type == "eparamtypes::utilitymaxcapacity")
            {
                return EParamTypes::UtilityMaxCapacity;
            }
            else if (param_type == "eparamtypes::utilityprocessingtimerequiredforpermit")
            {
                return EParamTypes::UtilityProcessingTimeRequiredForPermit;
            }
            else
            {
#ifdef DEBUG
                std::cout << param_type;
                throw std::runtime_error("missing conversion");
#endif
                return EParamTypes::None;
            };
        }
        
        
        
        static std::string FromEParamTypes(EParamTypes param_)
        {
            if (param_ == EParamTypes::HHMarketingStateHighlyInterested)
            {
                return "EParamTypes::HHMarketingStateHighlyInterested";
            }
            else if (param_ == EParamTypes::HHMarketingStateInterested)
            {
                return "EParamTypes::HHMarketingStateInterested";
            }
            else if (param_ == EParamTypes::HHMarketingNotInterested)
            {
                return "EParamTypes::HHMarketingNotInterested";
            }
            else if (param_ == EParamTypes::HHDecPreliminaryQuote)
            {
                return "EParamTypes::HHDecPreliminaryQuote";
            }
            else if (param_ == EParamTypes::SEISmall)
            {
                return "EParamTypes::SEISmall";
            }
            else if (param_ == EParamTypes::SEILarge)
            {
                return "EParamTypes::SEILarge";
            }
            else
            {
#ifdef DEBUG
                throw std::runtime_error("missing conversion");
#endif
                return "None";
            };
        }
        
        static EConstraintParams ToEConstraintParams(std::string param_type)
        {
            //case insensitive
            std::transform(param_type.begin(), param_type.end(), param_type.begin(), ::tolower);
            
            if (param_type == "econstraintparams::maxlengthplaninstallations")
            {
                return EConstraintParams::MaxLengthPlanInstallations;
            }
            else if (param_type == "econstraintparams::maxlengthwaitpermitvisit")
            {
                return EConstraintParams::MaxLengthWaitPermitVisit;
            }
            else if (param_type == "econstraintparams::maxlengthwaitpreliminaryquote")
            {
                return EConstraintParams::MaxLengthWaitPreliminaryQuote;
            }
            else if (param_type == "econstraintparams::minnreceiveddesings")
            {
                return EConstraintParams::MinNReceivedDesings;
            }
            else if (param_type == "econstraintparams::maxntickstocollectquotes")
            {
                return EConstraintParams::MaxNTicksToCollectQuotes;
            }
            else if (param_type == "econstraintparams:minnreceivedpreliminaryquotes")
            {
                return EConstraintParams::MinNReceivedPreliminaryQuotes;
            }
            else if (param_type == "econstraintparams::maxnrequestedpreliminaryfromonlinequotes")
            {
                return EConstraintParams::MaxNRequestedPreliminaryFromOnlineQuotes;
            }
            else if (param_type == "econstraintparams::maxnopenprojectshh")
            {
                return EConstraintParams::MaxNOpenProjectsHH;
            }
            else if (param_type == "econstraintparams::semmaxlengthrecordhistory")
            {
                return EConstraintParams::SEMMaxLengthRecordHistory;
            }
            else
            {
#ifdef DEBUG
                std::cout << param_type;
                throw std::runtime_error("missing conversion");
#endif
                return EConstraintParams::None;
            };

        }
        
    };
    
    
    
    
    
    
    
    
    
    
    

} //end of namespace solar_core
#endif
