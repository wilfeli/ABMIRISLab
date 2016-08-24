//
//  SEIBL.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef __ABMSolar__SEIBL__
#define __ABMSolar__SEIBL__

#include "Agents/SEI.h"
#include "Tools/ID.h"
#include "UI/UI_Python.h"
#include <Eigen/Dense>


namespace solar_core {
    class H;
    class WEE;
}






namespace solar_core
{
    
    class TDesign
    {
    public:
        std::shared_ptr<SolarModuleBL> PV_module; /*!< is used to pull ID, efficiency, degradation */
        std::vector<double> THETA_reliability; /*!< prior is G(1, (expected time without failures = warranty)), data is exponential, posterior is \f$ G(a + n, b + \sum_{i}\left(y_{i})\right) \f$   */
        std::vector<double> THETA_complexity; /*!< prior is Normal-Inverse Gamma(), posterior will be the same, and data distribution is Normal with unknown mean and variance:  */
        double irr = 0.0; /*!< advertized rate of return for an average project */
        double p_module = 0.0; /*!< price of a module for this installer */
        double p_design = 0.0; /*!< price per watt for the design based on this module */
        double complexity_install = 16.0; /*!< in labor*hours for the current state of the project */
        double BETA_complexity_time = 1.0; /*!< discounting for learning on how to install */
        int N_panels = 0; /*!< for an average house */
        double DC_size = 0.0; /*!< for an average house */
        double AC_size = 0.0; /*!< for an average house */
    };
    
    
    
    class SEIBL: public SEI
    {
        template <class T1, class T2> friend class HelperWSpecialization;
        friend class WEE;
        friend class solar_ui::UIBL;
        friend double ::C_API_estimate_profit(HUIBL* ui_, int sei_i, double p);
        friend double ::C_API_estimate_irr(HUIBL* ui_, int sei_i, double p);
        friend double ::C_API_estimate_irr_params(HUIBL* ui_, int sei_i, double efficiency_);
        friend double ::C_API_estimate_demand(HUIBL* ui_, int sei_i, double p);
        friend double ::C_API_estimate_demand_sei_params(HUIBL* ui_, int sei_i, double p, int size_THETA, const double* THETA);
    public:
        //@{
        
        /**
         
         Initialization section
         
         */
        SEIBL(const PropertyTree& pt_, WEE* w_);
        void init(WEE* w_); /*!< not virtual because in WEE has SEIBL as pointers */
        
        //@}
        
        //@{
        /**
         
         Agent's parameters
         
         */
        std::shared_ptr<PVProjectFlat> form_design_for_params(H* agent_, std::shared_ptr<PVProjectFlat> project);
        std::vector<double> THETA_reputation; /*!< have current estimate of a reputation by onlookers. Inv-Gamma distribution, is updated based on the realized production of installations. Here use Inv-Gamma\f$\left(\alpha, \beta \right)\f$. Assumption is that \f$\beta=1\f$ and estimate \f$\alpha\f$ via method of moments */
        //@}
        
        //@{
        /**
         
         Interactions with the world
         
         */
        
        void install_project(std::shared_ptr<PVProjectFlat> project_, TimeUnit time_);
        
        
        virtual void act_tick() override;
        void act_tick_wm(); /*!< updates wm when called from W, because need synchronized update for all sei, otherwise reputation will be divergent */
        void act_tick_pre(); /*!< updates to the next cycle of sei's inners */
        
        //@}
        
        
        //@{
        /**
         
         Offered design
         
         */
        
        TDesign* dec_design; /*!< current design to offer */
        
        //@}
        
        
        
    protected:
        
        
        //@{
        /**
         
         Section with internals of an agent
         
         */
        

        
        
        std::map<UID, TDesign*>  designs;  /** have current estimates on system design parameters for different learning distributions. UID of a solar_module here */
        

        
        std::vector<double> THETA_demand; /*!< BLR estimate for demand for projects given own irr and other parameters */
        
        std::vector<double> THETA_complexity_prior; /*!< mean was estimated from {\displaystyle \nu } \nu  observations with sample mean {\displaystyle \mu _{0}} \mu _{0}; variance was estimated from {\displaystyle 2\alpha } 2\alpha  observations with sample mean {\displaystyle \mu _{0}} \mu _{0} and sum of squared deviations {\displaystyle 2\beta } 2\beta  */
        
        std::vector<double> THETA_reliability_prior{1, 25}; /*!< 1, 25 where 25 - is average warranty length */
        
        double complexity_install_prior =  80.0;

        std::vector<double> WM_time; /*!< current state of information about reputation of others, promised returns of others, average for both parameters */
        
        std::vector<double> THETA_exploration; /*!< parameters for exploration distribution. Assume that probability of switching depends on the difference in profits. Two classes - with high and low probability of switching */
        
        std::mutex lock;
        
        WEE* w;
        
        
        std::vector<std::shared_ptr<PVProjectFlat>> pvprojects; /*!< list of active and potential PV projects */
        
        
        
        //@{
        /**
         
         Decisions section
         
         */
        
        TDesign* dec_base(); /*!< decision to switch or not */
        double exploration_p(double profit_new, double profit_old); /*!< return probability to switch */
        
        
        //@}
        
        
        
        
        
        //@{
        /**
         
         Calculations of a profit
         
         */
        double est_irr_from_params(TDesign* dec_design_hat, PVProjectFlat* project, double p); /*!< estimate irr for the project */
        double NPV_purchase(PVProjectFlat* project, double irr); /*!< NPV if purchased */
        double irr_secant(PVProjectFlat* project); /*!< finds irr given project parameters */
        double max_profit_GD(TDesign* dec_design_hat, PVProjectFlat* project); /*!< finds price that would maximize profit for this design, uses gradient descent */
        double max_profit_GS(TDesign* dec_design_hat, PVProjectFlat* project); /*!< finds price that would maximize profit for this design, uses grid search */
        
        
        using OptMethod = double (SEIBL::*) (TDesign* dec_design_hat, PVProjectFlat* project);
        
        OptMethod max_profit;
        
        
        double est_profit(TDesign* dec_design_hat, PVProjectFlat* project, double p, bool debug_flag = false); /*!< estimates profit for a price */
        double est_maintenance(TDesign* dec_design_hat, std::size_t N_hat, double wage); /*!< estimates maintenance given expected parameters */
        double est_demand_from_params(TDesign* dec_design_hat, PVProjectFlat* project, double p); /*!< estimates demand for the project and price */
        double f_derivative(double epsilon, TDesign* dec_design_hat, PVProjectFlat* project, double x);
        PVProjectFlat* init_average_pvproject(TDesign* dec_design_hat, PVProjectFlat* project, double p); /*!< setup average project for profit maximization */
        
        
        Eigen::MatrixXd profit_grid;
        
        
        
        //@}
        
        
        
        
        
        //@{
        /**
         
         Parameters for BLR for estimation of demand
         
         */
        Eigen::Matrix<double, 1, constants::N_BETA_SEI_WM> X;
        Eigen::Matrix<double, 1, 1> Y;
        
        SEIWMMatrixd V_0;
        SEIWMMatrixd V_n;
        SEIWMDataType Mu_0;
        SEIWMDataType Mu_n;
        double a_0;
        double a_n;
        double b_0;
        double b_n;
        //@}
        
        
        //@{
        /**
         
         Accounting parameters
         
         */
        
        double costs_time = 0.0; /*!< period costs */
        
        
        
        //@}
        
        //@{
        /**
         
         Section with internal actions of an agent
         
         */
        
        virtual void ac_update_tick() override; /*!< update internals for the tick */
        void wm_update_external(); /*!< updates estimates for demand */
        void wm_update_internal(); /*!< updates estimates for reputation estimate */
        void projects_update(); /*!< updates projects to maintenance and their parameters */
        //@}

        
        
        
        
    };
    
    
    
    
} //end namespace solar_core



#endif /* defined(__ABMSolar__SEIBL__) */
