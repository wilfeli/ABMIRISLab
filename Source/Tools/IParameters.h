////Kelley reviewed 4/24/16 see lines 75, 88, 113, 250, 296, 349
//  IParameters.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/24/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//


#pragma warning (push)
#pragma warning (disable : 4068) /* disable unknown pragma warnings */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"




#ifndef ABMSolar_IParameters_h
#define ABMSolar_IParameters_h

#include "Tools/ExternalIncludes.h"
#include <boost/property_tree/ptree.hpp>
#include <Eigen/Dense>

#ifdef ABMS_DEBUG_MODE

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
        const double NUMBER_DAYS_IN_TICK = 365.25;
        const double NUMBER_TICKS_IN_YEAR = 52.0;
        const int LABOR_UNITS_PER_TICK = 40;
        const std::size_t POOL_SIZE = 100000;
        const int NUMBER_WATTS_IN_KILOWATT = 1000;
        const double NUMBER_SQM_IN_SQF = 0.09290304;
        const int NUMBER_AGENT_TYPES_LIFE = 6; /*!< number of agents that update in W::life(), hh, sei, sem, g, market, utility */
        const int NUMBER_AGENT_TYPES_LIFE_EE = 3; /*!< number of agents that update in WEE::life(), hh, sei, sem  */
        const int N_BETA_SEI_WM = 5;
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
        

        /**  Number of humans in a Homeowner */
        N_H,
        
        
        /** Credit score */
        CreditScore,
        
        
        /** Average demand for electricity */
        AverageElectricityDemand,
        
        
        /** For @wp2 average level of solar radiation */
        AverageSolarIrradiation,
        
        
        /** For @wp2 average labor intencity of permitting in labor*hours */
        AveragePermitDifficulty,
        
        
        /** Consumption of electricity - monthly KWh */
        ElectricityConsumption,
        
        
        /** Projected growth rates */
        AverageElectricityDemandGrowthRate,
        
        
        /** Adjustment for RECS dataset from 2009 to 2015 */
        AverageElectricityDemandHistoricalGrowth,
        
        
		/** Adjustment for baseline efficiencies to test different scenarios  */
		ScenarioEfficiencyAdjustment,

        /** Adjustment for income level for the simulated state relative to US median income */
        MedianIncomeToUSCoefficient,
        
        
        /** Average loan length */
        AverageLoanLength,
        
        
        /** Interest rate on a loan */
        AverageInterestRateLoan,
        
        
        /** Number of Homeowners to generate */
        N_HO,
        
        /** Number of installers to generate */
        N_SEI,
        
        /** Number of large installers to generate */
        N_SEILarge,
        
        /** Number of manufactures to generate */
        N_SEM,
        
        /** Number of pv module manufactures to generate */
        N_SEMPVProducer,
        
        /** Number of inverter manufactures to generate */
        N_SEMInverterProducer,
        
        
        /** Number of homeowners that are interested in installing */
        N_HOMarketingStateHighlyInterested,
        
        /** Number of potential buyers */
        TotalPVMarketSize,
        
        
        /** Historical penetration level */
        PenetrationLevel,
        
        
        /** Labor price for the installers - qualified labor */
        LaborPrice,
        
        /** Electricity Bill */
        ElectricityBill,
        
        
        /** Roof size */
        RoofSize,
        
        
        /** Roof age */
        RoofAge,
        
        
        /**  Globally set warranty length of PV modules */
        PVModuleWarrantyLength,
        
        
        /** Homeowner non-compensatory decisions: ratings screening rule */
        HONCDecisionSEIRating,
        
        
        /** Homeowner non-compensatory decisions: total price */
        HONCDecisionTotalPrice,
        
        
        /** Homeowner installer decision: utility of other options */
        HOSEIDecisionUtilityNone,
        
        
        /** Resulting estimate of the utility for an option */
        HOSEIDecisionEstimatedUtility,
        
        /** Decision on net savings */
        HOSEIDecisionEstimatedNetSavings,
        
        /** Decisions on the length of the project */
        HOSEIDecisionTotalProjectTime,
        
        
        /**  Utiilty of alternative */
        HODesignDecisionUtilityNone,
        
        /** Decision on panels efficiency */
        HODesignDecisionPanelEfficiency,
        
        /** Is property of a specific panel */
        HODesignDecisionPanelVisibility,
        
        /** Decision on inverter type */
        HODesignDecisionInverterType,
        
        /** Is property of a design as a whole */
        HODesignDecisionFailures,
        
        /** Is property of a design as a whole. Calculated from expected produced enegry */
        HODesignDecisionCO2,
        
        /** Decisions on savings of the whole istallation */
        HODesignDecisionEstimatedNetSavings,
        
        /** Resulting estimate of the utility for an option */
        HODesignDecisionEstimatedUtility,
        
        
        /** Types of decisions */
        HONCDecision,
        HOSEIDecision,
        HODesignDecision, 
        
        
        
        
        
        
        
		/** If Homeowner is very interested in SP */
        HOMarketingStateHighlyInterested,
        
        
        /** If Homeowner is just interested in SP and ready to ask for quotes */
        HOMarketingStateInterested,
        
        
        /** If Homeowner is not interested in installing SP */
        HOMarketingStateNotInterested,
        
        
        /*!< If Homeowner in other stages and not open to new marketing */
        HOMarketingStateNotAccepting,

        
        /** If Homeowner is already installed */
        HOStateCommitedToInstallation,
        
        
        
        /** If Homeowner is finished installing */
        HOStateInterconnected,
        
        
        /** If Homeowner decided to drop out at SEI conjoint stage  */
        HOStateDroppedOutSEIStage,
        
        
        /** If Homeowner decided to drop out at NC Decision stage  */
        HOStateDroppedOutNCDecStage,


        /** If Homeowner decided to drop out at Design stage  */
        HOStateDroppedOutDesignStage,
        
        
        /** State of a quoting stage for HO: actively requesting information */
        HOStateActiveQuoting,
        
        
        /** State of a quoting stage for HO: not requesting quotes, might be analysing them or committed to the project */
        HOStateInactiveQuoting,
        
        
        /** State of a quoting stage for HO: waits for online quotes to be provided */
        HOStateWaitingOnOnlineQuotes,
        
        
        /** State of a quoting stage for HO: waiting for preliminary quotes */
        HOStateWaitingOnPreliminaryQuotes,
        
        
        /** State of a quoting stage for HO: waiting for designs */
        HOStateWaitingOnDesigns,
        
        
        /** State of a quoting stage for HO: waiting for permitting */
        HOStateEvaluatedDesigns,
        
        
        /** State of a quoting stage for HO: decision on reroofing old roof */
        HODecisionReroof,
        
        
        /** Maximum number of visits per unit of time for HO */
        HOMaxNVisitsPerTimeUnit,
        
        
        /** Thetas for decisions: decision on preliminary quotes */
        HODecPreliminaryQuote,
        
        
        /** Number of active agents per tick, used in data collection */
        HONumberActiveAgents,
        
        
        
        
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
        RequiredHOReroof,
        
        /** State of a Project: Homeowner agreed to reroof thus waiting for reroofing */
        WaitingHOReroof,
        
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
        
        /** Preliminary size of a system */
        PreliminaryQuoteDCSize,
        
        
        /** Preliminary estimation of savings based on utility bill after site visit */
        PreliminaryQuoteEstimatedSavings,
        
        
        /** Preliminary estimation of savings based on utility bill after site visit, net expression */
        PreliminaryQuoteEstimatedNetSavings,
        
        


        
        /** Estimated total project time for the location - general estimate */
        PreliminaryQuoteTotalProjectTime,
        
        
        
        
        /**  For testing purposes, coef in demand equation  */
        EstimatedDemandCoefficientNCDec,
        
        
        /** Industry standard price per watt that is used in estimating installation cost */
        EstimatedPricePerWatt,
        
        
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
        
        
        /** Convertion from kWh into acres of forest for CO2 */
        EnergyToCO2,
        
        
        /** Length of a definition for degradation rate */
        DegradationDefinitionLength,
        
        
        
        /** Small SEI agent - such as mom and pop shop */
        SEISmall,
        
        
        /** Large SEI */
        SEILarge,
        
        
        /** Parameters of a SEI: rating */
        SEIRating,
        
        
        /** Parameters of a SEI: type of interaction */
        SEIInteractionType,
        
        
        /** Parameters of a SEI: equipment type*/
        SEIEquipmentType,
        
        
        /** Parameters of a SEI: lead in time before project could moved into permitting phase*/
        SEILeadInProjectTime,
        
        
        /** Parameters of a SEI: type of warranty*/
        SEIWarranty,

        
        /** Parameters of a SEI, such as processing time before preliminary quote is formed after site visit */
        SEIProcessingTimeRequiredForPreliminaryQuote,
        
        /** Parameters of a SEI, such as processing time before action is taken to schedule first site visit */
        SEIProcessingTimeRequiredForSchedulingFirstSiteVisit,
        
        /** Parameters of a SEI, processing time before design is created after project is accepted by HO */
        SEIProcessingTimeRequiredForDesign,
        
        /** Maximum number of visits per unit of time for SEI */
        SEIMaxNVisitsPerTimeUnit,
        
        /** Maximum number of installation per unit of time for SEI */
        SEIMaxNInstallationsPerTimeUnit,
        
        /** Maximum age of the roof to accept for installation */
        SEIMaxRoofAge,

        /** Parameters of a SEI, frequency of revising designs */
        SEIFrequencyUpdateDesignTemplates,
        
        /** Parameters of a SEI, frequency of updating pricing */
        SEIFrequencyDecPrice,
        
        /**  If use customer specific consumption data in preliminary quotes */
        SEIAverageDemandInPreliminaryQuote,
        
        
		/*Could you also use an enumerated class for high efficiency, mid and low efficiency design?*/

        /** High efficiency choice of a panel */
        SEIHighEfficiencyDesign,
        
        
        /** Mid range efficiency choice of a panel */
        SEIMidEfficiencyDesign,
        
        
        /** Low efficiency choice of a panel */
        SEILowEfficiencyDesign,
        
    
        
        SEITimeLUForDesign,
        
        SEITimeLUForMarketing,
        
        SEITimeLUForPermit,
        
        SEITimeLUForAdministration,
        
        
        
        
        
        
        
        
        
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
        
        /** SEM parameter: Markup to the price for the increased efficiency */
        SEMPriceMarkupEfficiency,
        
        /** SEM parameter: Learning rate for production of new panels, basically price decrease rate */
        SEMLearningPrice,
        
        
        /** Inverter or PV producer  */
        SEMPVProducer,
        SEMInverterProducer,
        
        
        
        /** Base Prices per type */
        SEMCentralInverterBasePrice,
        SEMMicroInverterBasePrice,
        
        
        
        /** Failure rates for PV module, failure rate per year! */
        SEMFailureRatePVModule,
        
        /** Failure rates for central inverter, failure rate per year! */
        SEMFailureRateInverterCentral,
        
        /** Failure rates for power optimizer, failure rate per year! */
        SEMFailureRateInverterPowerOptimizer,
        
        /** Failure rates for micro inverter, failure rate per year! */
        SEMFailureRateInverterMicro,
        
        
        
        
        /** G parameters: processing time before visit is scheduled */
        GProcessingTimeRequiredForSchedulingPermitVisit,
        
        /** G parameters: maximum number of visits per time unit */
        GMaxNVisitsPerTimeUnit,
        
        /** G parameters: time to process permit */
        GProcessingTimeRequiredForProcessingPermit,
        
        /** G parameters: processing time for granting installation permit */
        GProcessingTimeRequiredForGrantingPermitForInstallation,
        
        
        GFederalTaxIncentive,
        
        
        /** Utility parameters: time to investigate request for interconnection */
        UtilityProcessingTimeRequiredForPermit,
        
        /** Utility parameters: Max capacity of the grid */
        UtilityMaxCapacity,
        
        /** Utility parameters: Current capacity of the grid */
        UtilityCurrentCapacity,
        
        
        /** Marketing part: number of HO to draw per unit of time to push marketing information */
        MarketingMaxNToDrawPerTimeUnit,
        
        
        /** Market, maximum distance to accept sei offer */
        MarketingSEIMaxDistance,
        
        
        
        
        /** State of Payments: all payments on time */
        PaymentsOnTime,
        
        

        
        
        /** Technology block: inverters technology type standard */
        TechnologyInverterCentral,
        
        
        /** Technology block: inverters technology type micro */
        TechnologyInverterMicro,
        
        
        /** Technology block: inverters technology type micro */
        TechnologyInverterPowerOptimizer,
        
        
        /** Number of H to draw per tick for EE model */
        WHMaxNToDrawPerTimeUnit,
        
        
        
        
        /** Empty enum for completeness */
        None
        
    };
    
    enum class EConstraintParams: int64_t
    {
        
        /** Maximum number of ticks to collect quotes (online) */
        MaxNTicksToCollectQuotes, //Can you explain what this variable means?
        
        
        /** Maximum number of open projects to consider */
        MaxNOpenProjectsHO,
        
        
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
        
        
        
        
        
        /** Empty enum for completeness */
        None,
    };
    
    
    
    
    enum class ERandomParams: int64_t
    {
        /** continuous distribution discretized into bins */
        ContinuousDiscretized,
        
        /** discrete distribution to begin with */
        Discrete,
        
        /** truncated normal */
        N_trunc,
         
        /** normal */
        N,
         
        /** uniform distribution  */
        u,
        
        /**  uniform int distribution */
        u_int,
        
        
        /** draw from the big joint distribution from external sources */
        custom,
        
        
        /** Empty enum for completeness */
        None
    };
    
    
    
    /** All in terms of efficiency only  */
    enum class ESEIEquipmentType: int64_t
    {
        
        /**  Least advanced */
        Traditional = 0,
        
        /**  Mid range */
        Standard = 1,
        
        /**  Most advanced */
        CuttingEdge = 2,
        
        /** Empty enum for completeness  */
        None = -1
    };
    
    
    
    enum class ESEIInverterType: int64_t
    {
        
        Central = 0,
        
        
        PoweOptimizer = 1,
        
        
        Micro = 2,
        
        
        
        /** Empty enum for completeness  */
        None = -1
        
    };
    
    
    
    
    enum class EDecParams: int64_t
    {
        Reputation_i,
        
        Reputation,
        
        Price_i,
        
        Price,
        
        irr,
        
        irr_i,
        
        NewTechnology,
        
        CurrentTechnology,
        
        Share,
        
        None
        
    };
    
    
    
    
    typedef int64_t TimeUnit;
    typedef boost::property_tree::ptree PropertyTree; //container for parameters, properties
    typedef Eigen::MatrixXd ThetaType; //one of containers for parameters
    typedef Eigen::Matrix<double, constants::N_BETA_SEI_WM, 1> SEIWMDataType; //one of containers for parameters
    typedef Eigen::Matrix<double, constants::N_BETA_SEI_WM, constants::N_BETA_SEI_WM> SEIWMMatrixd;
    
    typedef std::underlying_type<EParamTypes>::type EParamTypes_type;
    
    std::ostream &operator<<(std::ostream& is, const EParamTypes & item);
    std::istream& operator>> (std::istream&  os, EParamTypes & item);
    
    
    
    
    class EnumFactory{
    public:
        static EParamTypes ToEParamTypes(std::string param_type)
        {
            //case insensitive
            std::transform(param_type.begin(), param_type.end(), param_type.begin(), ::tolower);
            
            if (param_type == "eparamtypes::income")
            {
                return EParamTypes::Income;
            }
            else if (param_type == "eparamtypes::n_h")
            {
                return EParamTypes::N_H;
            }
            else if (param_type == "eparamtypes::energytoco2")
            {
                return EParamTypes::EnergyToCO2;
            }
            else if (param_type == "eparamtypes::pvmodulewarrantylength")
            {
                return EParamTypes::PVModuleWarrantyLength;
            }
            else if (param_type == "eparamtypes::hodecpreliminaryquote")
            {
                return EParamTypes::HODecPreliminaryQuote;
            }
            else if (param_type == "eparamtypes::creditscore")
            {
                return EParamTypes::CreditScore;
            }
            else if (param_type == "eparamtypes::electricitybill")
            {
                return EParamTypes::ElectricityBill;
            }
			else if (param_type == "eparamtypes::scenarioefficiencyadjustment") 
			{
				return EParamTypes::ScenarioEfficiencyAdjustment;
			}
            else if (param_type == "eparamtypes::averageelectricitydemand")
            {
                return EParamTypes::AverageElectricityDemand;
            }
            else if (param_type == "eparamtypes::electricitypriceucsupply")
            {
                return EParamTypes::ElectricityPriceUCSupply;
            }
            else if (param_type == "eparamtypes::averageelectricitydemandgrowthrate")
            {
                return EParamTypes::AverageElectricityDemandGrowthRate;
            }
            else if (param_type == "eparamtypes::averageelectricitydemandhistoricalgrowth")
            {
                return EParamTypes::AverageElectricityDemandHistoricalGrowth;
            }
            else if (param_type == "eparamtypes::medianincometouscoefficient")
            {
                return EParamTypes::MedianIncomeToUSCoefficient;
            }
            else if (param_type == "eparamtypes::penetrationlevel")
            {
                return EParamTypes::PenetrationLevel;
            }
            else if (param_type == "eparamtypes::homaxnvisitspertimeunit")
            {
                return EParamTypes::HOMaxNVisitsPerTimeUnit;
            }
            else if (param_type == "eparamtypes::homarketingstatehighlyinterested")
            {
                return EParamTypes::HOMarketingStateHighlyInterested;
            }
            else if (param_type == "eparamtypes::honcdecisionseirating")
            {
                return EParamTypes::HONCDecisionSEIRating;
            }
            else if (param_type == "eparamtypes::honcdecisiontotalprice")
            {
                return EParamTypes::HONCDecisionTotalPrice;
            }
            else if (param_type == "eparamtypes::hoseidecisionutilitynone")
            {
                return EParamTypes::HOSEIDecisionUtilityNone;
            }
            else if (param_type == "eparamtypes::hodesigndecisionco2")
            {
                return EParamTypes::HODesignDecisionCO2;
            }
            else if (param_type == "eparamtypes::hodesigndecisionfailures")
            {
                return EParamTypes::HODesignDecisionFailures;
            }
            else if (param_type == "eparamtypes::hoseidecisiontotalprojecttime")
            {
                return EParamTypes::HOSEIDecisionTotalProjectTime;
            }
            else if (param_type == "eparamtypes::hoseidecisionestimatednetsavings")
            {
                return EParamTypes::HOSEIDecisionEstimatedNetSavings;
            }
            else if (param_type == "eparamtypes::hodesigndecisionestimatednetsavings")
            {
                return EParamTypes::HODesignDecisionEstimatedNetSavings;
            }
            else if (param_type == "eparamtypes::hodesigndecisioninvertertype")
            {
                return EParamTypes::HODesignDecisionInverterType;
            }
            else if (param_type == "eparamtypes::hodesigndecisionutilitynone")
            {
                return EParamTypes::HODesignDecisionUtilityNone;
            }
            else if (param_type == "eparamtypes::hodesigndecisionpanelvisibility")
            {
                return EParamTypes::HODesignDecisionPanelVisibility;
            }
            else if (param_type == "eparamtypes::hodesigndecisionpanelefficiency")
            {
                return EParamTypes::HODesignDecisionPanelEfficiency;
            }
            else if (param_type == "eparamtypes::hostateinactivequoting")
            {
                return EParamTypes::HOStateInactiveQuoting;
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
            else if (param_type == "eparamtypes::seifrequencydecprice")
            {
                return EParamTypes::SEIFrequencyDecPrice;
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
            else if (param_type == "eparamtypes::seiaveragedemandinpreliminaryquote")
            {
                return EParamTypes::SEIAverageDemandInPreliminaryQuote;
            }
            else if (param_type == "eparamtypes::estimateddemandcoefficientncdec")
            {
                return EParamTypes::EstimatedDemandCoefficientNCDec;
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
            else if (param_type == "eparamtypes::seitimeluformarketing")
            {
                return EParamTypes::SEITimeLUForMarketing;
            }
            else if (param_type == "eparamtypes::seitimelufordesign")
            {
                return EParamTypes::SEITimeLUForDesign;
            }
            else if (param_type == "eparamtypes::seitimeluforpermit")
            {
                return EParamTypes::SEITimeLUForPermit;
            }
            else if (param_type == "eparamtypes::seitimeluforadministration")
            {
                return EParamTypes::SEITimeLUForAdministration;
            }
            else if (param_type == "eparamtypes::seiinteractiontype")
            {
                return EParamTypes::SEIInteractionType;
            }
            else if (param_type == "eparamtypes::seileadinprojecttime")
            {
                return EParamTypes::SEILeadInProjectTime;
            }
            else if (param_type == "eparamtypes::seiwarranty")
            {
                return EParamTypes::SEIWarranty;
            }
            else if (param_type == "eparamtypes::seiequipmenttype")
            {
                return EParamTypes::SEIEquipmentType;
            }
            else if (param_type == "eparamtypes::seirating")
            {
                return EParamTypes::SEIRating;
            }
            else if (param_type == "eparamtypes::n_ho")
            {
                return EParamTypes::N_HO;
            }
            else if (param_type == "eparamtypes::n_sei")
            {
                return EParamTypes::N_SEI;
            }
            else if (param_type == "eparamtypes::n_seilarge")
            {
                return EParamTypes::N_SEILarge;
            }
            else if (param_type == "eparamtypes::n_sem")
            {
                return EParamTypes::N_SEM;
            }
            else if (param_type == "eparamtypes::n_homarketingstatehighlyinterested")
            {
                return EParamTypes::N_HOMarketingStateHighlyInterested;
            }
            else if (param_type == "eparamtypes::totalpvmarketsize")
            {
                return EParamTypes::TotalPVMarketSize;
            }
            else if (param_type == "eparamtypes::laborprice")
            {
                return EParamTypes::LaborPrice;
            }
            else if (param_type == "eparamtypes::averagesolarirradiation")
            {
                return EParamTypes::AverageSolarIrradiation;
            }
            else if (param_type == "eparamtypes::averagepermitdifficulty")
            {
                return EParamTypes::AveragePermitDifficulty;
            }
            else if (param_type == "eparamtypes::electricityconsumption")
            {
                return EParamTypes::ElectricityConsumption;
            }
            else if (param_type == "eparamtypes::averageloanlength")
            {
                return EParamTypes::AverageLoanLength;
            }
            else if (param_type == "eparamtypes::averageinterestrateloan")
            {
                return EParamTypes::AverageInterestRateLoan;
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
            else if (param_type == "eparamtypes::gfederaltaxincentive")
            {
                return EParamTypes::GFederalTaxIncentive;
            }
            else if (param_type == "eparamtypes::marketingseimaxdistance")
            {
                return EParamTypes::MarketingSEIMaxDistance;
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
            else if (param_type == "eparamtypes::semlearningprice")
            {
                return EParamTypes::SEMLearningPrice;
            }
            else if (param_type == "eparamtypes::semfailureratepvmodule")
            {
                return EParamTypes::SEMFailureRatePVModule;
            }
            else if (param_type == "eparamtypes::semfailurerateinvertercentral")
            {
                return EParamTypes::SEMFailureRateInverterCentral;
            }
            else if (param_type == "eparamtypes::semfailurerateinverterpoweroptimizer")
            {
                return EParamTypes::SEMFailureRateInverterPowerOptimizer;
            }
            else if (param_type == "eparamtypes::semfailurerateinvertermicro")
            {
                return EParamTypes::SEMFailureRateInverterMicro;
            }
            else if (param_type == "eparamtypes::semcentralinverterbaseprice")
            {
                return EParamTypes::SEMCentralInverterBasePrice;
            }
            else if (param_type == "eparamtypes::semmicroinverterbaseprice")
            {
                return EParamTypes::SEMMicroInverterBasePrice;
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
            else if (param_type == "eparamtypes::gprocessingtimerequiredforgrantingpermitforinstallation")
            {
                return EParamTypes::GProcessingTimeRequiredForGrantingPermitForInstallation;
            }
            else if (param_type == "eparamtypes::whmaxntodrawpertimeunit")
            {
                return EParamTypes::WHMaxNToDrawPerTimeUnit;
            }
            else if (param_type == "eparamtypes::none")
            {
                return EParamTypes::None;
            }
            else
            {
#ifdef ABMS_DEBUG_MODE
                std::cout << param_type;
                throw std::runtime_error("missing conversion");
#endif
                return EParamTypes::None;
            };
        }
        
        
        
        static std::string FromEParamTypes(EParamTypes param_)
        {
            if (param_ == EParamTypes::HOMarketingStateHighlyInterested)
            {
                return "EParamTypes::HOMarketingStateHighlyInterested";
            }
            else if (param_ == EParamTypes::HONumberActiveAgents)
            {
                return "EParamTypes::HONumberActiveAgents";
            }
            else if (param_ == EParamTypes::HOMarketingStateInterested)
            {
                return "EParamTypes::HOMarketingStateInterested";
            }
            else if (param_ == EParamTypes::HOStateDroppedOutSEIStage)
            {
                return "EParamTypes::HOStateDroppedOutSEIStage";
            }
            else if (param_ == EParamTypes::HOStateDroppedOutNCDecStage)
            {
                return "EParamTypes::HOStateDroppedOutNCDecStage";
            }
            else if (param_ == EParamTypes::HOStateWaitingOnOnlineQuotes)
            {
                return "EParamTypes::HOStateWaitingOnOnlineQuotes";
            }
            else if (param_ == EParamTypes::HOStateWaitingOnPreliminaryQuotes)
            {
                return "EParamTypes::HOStateWaitingOnPreliminaryQuotes";
            }
            else if (param_ == EParamTypes::HOStateDroppedOutDesignStage)
            {
                return "EParamTypes::HOStateDroppedOutDesignStage";
            }
            else if (param_ == EParamTypes::HOStateWaitingOnDesigns)
            {
                return "EParamTypes::HOStateWaitingOnDesigns";
            }
            else if (param_ == EParamTypes::HOStateCommitedToInstallation)
            {
                return "EParamTypes::HOStateCommitedToInstallation";
            }
            else if (param_ == EParamTypes::HOStateInterconnected)
            {
                return "EParamTypes::HOStateInterconnected";
            }
            else if (param_ == EParamTypes::HOMarketingStateNotInterested)
            {
                return "EParamTypes::HOMarketingNotInterested";
            }
            else if (param_ == EParamTypes::HOMarketingStateNotAccepting)
            {
                return "EParamTypes::HOMarketingStateNotAccepting";
            }
            else if (param_ == EParamTypes::HODecPreliminaryQuote)
            {
                return "EParamTypes::HODecPreliminaryQuote";
            }
            else if (param_ == EParamTypes::RequestedOnlineQuote)
            {
                return "EParamTypes::RequestedOnlineQuote";
            }
            else if (param_ == EParamTypes::RequestedPreliminaryQuote)
            {
                return "EParamTypes::RequestedPreliminaryQuote";
            }
            else if (param_ == EParamTypes::ProvidedOnlineQuote)
            {
                return "EParamTypes::ProvidedOnlineQuote";
            }
            else if (param_ == EParamTypes::ProvidedPreliminaryQuote)
            {
                return "EParamTypes::ProvidedPreliminaryQuote";
            }
            else if (param_ == EParamTypes::ScheduledFirstSiteVisit)
            {
                return "EParamTypes::ScheduledFirstSiteVisit";
            }
            else if (param_ == EParamTypes::CollectedInfFirstSiteVisit)
            {
                return "EParamTypes::CollectedInfFirstSiteVisit";
            }
            else if (param_ == EParamTypes::RequiredHOReroof)
            {
                return "EParamTypes::RequiredHOReroof";
            }
            else if (param_ == EParamTypes::WaitingHOReroof)
            {
                return "EParamTypes::WaitingHOReroof";
            }
            else if (param_ == EParamTypes::AcceptedPreliminaryQuote)
            {
                return "EParamTypes::AcceptedPreliminaryQuote";
            }
            else if (param_ == EParamTypes::DraftedDesign)
            {
                return "EParamTypes::DraftedDesign";
            }
            else if (param_ == EParamTypes::AcceptedDesign)
            {
                return "EParamTypes::AcceptedDesign";
            }
            else if (param_ == EParamTypes::RequestedPermit)
            {
                return "EParamTypes::RequestedPermit";
            }
            else if (param_ == EParamTypes::ScheduledPermitVisit)
            {
                return "EParamTypes::ScheduledPermitVisit";
            }
            else if (param_ == EParamTypes::CollectedInfPermitVisit)
            {
                return "EParamTypes::CollectedInfPermitVisit";
            }
            else if (param_ == EParamTypes::ScheduledInstallation)
            {
                return "EParamTypes::ScheduledInstallation";
            }
            else if (param_ == EParamTypes::ScheduleInstallation)
            {
                return "EParamTypes::ScheduleInstallation";
            }
            else if (param_ == EParamTypes::PendingMaterials)
            {
                return "EParamTypes::PendingMaterials";
            }
            else if (param_ == EParamTypes::Installed)
            {
                return "EParamTypes::Installed";
            }
            else if (param_ == EParamTypes::RequestedPermitForInstallation)
            {
                return "EParamTypes::RequestedPermitForInstallation";
            }
            else if (param_ == EParamTypes::GrantedPermitForInstallation)
            {
                return "EParamTypes::GrantedPermitForInstallation";
            }
            else if (param_ == EParamTypes::RequestedInspectionAfterInstallation)
            {
                return "EParamTypes::RequestedInspectionAfterInstallation";
            }
            else if (param_ == EParamTypes::PassedInspectionAfterInstallation)
            {
                return "EParamTypes::PassedInspectionAfterInstallation";
            }
            else if (param_ == EParamTypes::RequestedPermitForInterconnection)
            {
                return "EParamTypes::RequestedPermitForInterconnection";
            }
            else if (param_ == EParamTypes::GrantedPermitForInterconnection)
            {
                return "EParamTypes::GrantedPermitForInterconnection";
            }
            else if (param_ == EParamTypes::ClosedProject)
            {
                return "EParamTypes::ClosedProject";
            }
            else if (param_ == EParamTypes::SEISmall)
            {
                return "EParamTypes::SEISmall";
            }
            else if (param_ == EParamTypes::SEILarge)
            {
                return "EParamTypes::SEILarge";
            }
            else if (param_ == EParamTypes::None)
            {
                return "EParamTypes::None";
            }
            else
            {
#ifdef ABMS_DEBUG_MODE
                throw std::runtime_error("missing conversion");
#endif
                return "EParamTypes::None";
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
            else if (param_type == "econstraintparams::maxnopenprojectsho")
            {
                return EConstraintParams::MaxNOpenProjectsHO;
            }
            else if (param_type == "econstraintparams::semmaxlengthrecordhistory")
            {
                return EConstraintParams::SEMMaxLengthRecordHistory;
            }
            else
            {
#ifdef ABMS_DEBUG_MODE
                std::cout << param_type;
                throw std::runtime_error("missing conversion");
#endif
                return EConstraintParams::None;
            };

        }
        
        
        
        
        
        static ESEIInverterType ToESEIInverterType(std::string param_)
        {
            //case insensitive
            std::transform(param_.begin(), param_.end(), param_.begin(), ::tolower);
            
            if (param_ == "eseiinvertertype::central")
            {
                return ESEIInverterType::Central;
            }
            else if (param_ == "single-phase string inverter")
            {
                return ESEIInverterType::Central;
            }
            else if (param_ == "string inverter")
            {
                return ESEIInverterType::Central;
            }
            else if (param_ == "eseiinvertertype::poweoptimizer")
            {
                return ESEIInverterType::PoweOptimizer;
            }
            else if (param_ == "eseiinvertertype::micro")
            {
                return ESEIInverterType::Micro;
            }
            else if (param_ == "microinverter")
            {
                return ESEIInverterType::Micro;
            }
            else
            {
#ifdef ABMS_DEBUG_MODE
                std::cout << param_;
                throw std::runtime_error("missing conversion");
#endif
                return ESEIInverterType::None;
            };

        }
        
        
    };
    
    
    
    
    
    
    
    
    
    
    

} //end of namespace solar_core


#pragma clang diagnostic pop
#pragma warning (pop)

#endif
