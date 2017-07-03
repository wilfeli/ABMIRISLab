import numpy as np
import math 
import json
import pandas as pd

'''

Installer fixes price at some level, assume some costs
Estimate how much could sell at that price - given different rules

Run model over actual distribution of people and see how many install

Technical - calculating parameters of a system

Manufacturer is removed, replaced with fixed offering of PV panel 



TODO:
Fix parameters for manufacturer - precan them
save h generation
create utilities for sei and system
fix parameters for sei and system utility estimation

read json - check in jupyter format and recreate splines algorithm 




'''


from enum import Enum
class EParamTypes(Enum):
    SEIInteractionType = auto()
    SEIRating = auto()
    SEIEquipmentType = auto()
    HOSEIDecisionTotalProjectTime = auto()
    SEIWarranty = auto()
    HOSEIDecisionEstimatedNetSavings = auto()
    HODesignDecisionEstimatedNetSavings = auto()
    HODesignDecisionCO2 = auto()
    HODesignDecisionPanelVisibility = auto()
    HODesignDecisionInverterType = auto()
    HODesignDecisionPanelEfficiency = auto()
    HODesignDecisionFailures = auto()




'''

System to offer

'''
class PVPanel:

    def __init__(self, **kwargs):
        self.efficiency = 0.1576
        self.width = 991
        self.height = 1665
        self.degradation =  math.exp(math.log((1 - 0.1))/10);
        self.visibility = 0 

        
        return super().__init__(**kwargs)

class PVSystem:
    def __init__(self, **kwargs):
        self.n_panels = 0.0;
        self.price = 0.0
        self.savings = 0.0


        return super().__init__(**kwargs)

class Settings:
    def __init__(self, **kwargs):
        self.p_electricity_d = 0.15
        self.p_electricity_s = 0.15
        self.house_to_roof_size = 0.5 * 0.25
        self.demand_share = 1.0
        self.solar_irradiation = 5.0
        self.dc_to_ac = 0.8
        self.tax_incentive = 0.3
        self.interest_rate = 0.06
        self.inflation = 0.02
        self.EnergyToCO2 = 0.535
        self.SEMFailureRatePVModule = 0.003
        self.SEMFailureRateInverterCentral = 0.1 
        self.SEMFailureRateInverterCentral = 0.0035


        return super().__init__(**kwargs)





class Model:

    #main loop of the model
    def run_model(self):

        #pull hs for system installation
        hs = np.random.randint(size=(len(self.hs)))

        for i in range(len(self.seis)):
            self.seis[i].offer_systems(hs)

        
        #random part of homeowners calls him to get quotes
        

        #quotes are formed 
        



    #need utility functions to get utility values 

    #need profit max to get profit max estimation 

    #need pv system calculation


    def tearup_model(self):

        global settings = Settings()
        global tools = MathTools()

        global NUMBER_DAYS_IN_MONTH = 30.4375
        global NUMBER_DAYS_IN_YEAR = 365.25

        self.systems = []

        #read file with csv data
        df_full = pd.read_csv("../../ABMIRISLab/Source/Examples/BaseModel/homeowners.csv", low_memory=False)

        #here setting up simulation will go 
        #need distribution of homeowners
        #create homeowners
        self.hs = []
        for i in range(100):
            roof_size = settings.house_to_roof_size * df_full.values[i][0] 
            el_d = df_full.values[i][5]/NUMBER_DAYS_IN_YEAR * NUMBER_DAYS_IN_MONTH
            el_bill = el_d * settings.p_electricity_d

            self.hs.append(Homeowner(roof_size, el_d, el_bill, self))
            



        #need few installers? - one for now with fixed parameters 
        #fixed pv panel
        #fixed number of projects per time unit
        self.seis = []
        self.seis.append(SEI(self))



        #read parameters for preferences 
        settings.THETA_SEI = {}
        settings.THETA_DESIGN = {}

        with open('../../ABMIRISLab/Source/Examples/BaseModel/ho-installerdecisions.json') as infile:
            data_raw = json.load(infile)
            settings.THETA_SEI = data_raw['type1']['scheme']

        with open('../../ABMIRISLab/Source/Examples/BaseModel/ho-designdecisions.json') as infile:
            data_raw = json.load(infile)
            settings.THETA_DESIGN = data_raw['type1']['scheme']


        

    

'''

Will have roof size, house size, electricity bill

'''
class Homeowner:

    def __init__(self, name= '', roof_size=0.0, el_d = 0.0, el_bill = 0.0, model = NoneType):
        """ Create Homeowner """
        self.name = name
        #demand daily
        self.el_d = el_bill/settings.p_electricity_d/NUMBER_DAYS_IN_MONTH
        #demand in money terms, monthly
        self.el_bill = el_bill
        self.roof_size = roof_size
        self.model = model



    def utility_sei(project):
        #estimate utility for sei 
        #need splines 
        utility = 0.0
        x_i = 0.0
        y_i = 0.0
        param = None
        THETA = settings.THETA_SEI



        #here installers are evaluated
        #preliminary quote will have general savings estimation
        #collect all parameters for decisions
        utility = utility + THETA['EParamTypes::SEIRating'][project.sei.params[EParamTypes.SEIRating] - 3]

        #discrete variables have position in the vector of coefficients as their value. Fragile.
        utility = utility + THETA['EParamTypes::SEIInteractionType'][project.sei.params[EParamTypes.SEIInteractionType]]


        #number inside is type of equipment - directly corresponds to the position in the vector
        utility = utility + THETA['EParamTypes::SEIEquipmentType'][project.sei.params[EParamTypes.SEIEquipmentType]]

        #estimated project total time
        #LeadIn time if fixed for each installer and is an estimation
        #Permitting time depends on the location, but is an estimate
        NUMBER_TICKS_IN_MONTH = 4
        x_i = project.TotalProjectTime / NUMBER_TICKS_IN_MONTH
        param = EParamTypes.HOSEIDecisionTotalProjectTime
        y_i = tools.get_spline_value(param, x_i)

        #it is part worth, so no multiplication here
        utility = utility + y_i

        #warranty
        #MARK: CAREFULL Warranty is in month in SEI definition and in years in conjoint
        warranty_map = {{5.0, 0.0}, {15.0, 1.0}, {25.0, 2.0}};
        utility = utility + THETA['EParamTypes::SEIWarranty'][warranty_map[project.sei.params[EParamTypes.SEIWarranty]]]


        #savings are estimated for an average homeowner
        x_i = project.total_net_savings
        param = EParamTypes.HOSEIDecisionEstimatedNetSavings
        y_i = tools.get_spline_value(param, x_i)
        utility = utility + y_i



    def utility_system(project):
        utility = 0.0
        x_i = 0.0
        y_i = 0.0

        THETA = settings.THETA_DESIGN

        #efficiency
        x_i = project.pvpanel.efficiency
        param = EParamTypes.HODesignDecisionPanelEfficiency
        y_i = get_spline_value(param, x_i)
        utility = utility + y_i

        #visibility
        utility = utility + THETA['EParamTypes::HODesignDecisionPanelVisibility'][project.vpanel.visibility]

        #inverter type
        utility = utility + THETA['EParamTypes::HODesignDecisionInverterType'][project.inverter]

        #number of failures
        #here need 5 years because it is what is asked in the survey
        x_i = 5 * project.failure_rate
        param = EParamTypes.HODesignDecisionFailures
        y_i = get_spline_value(param, x_i)
        utility += y_i

	
        #emmision levels
        x_i = project.co2_equivalent
        param = EParamTypes.HODesignDecisionCO2
        y_i = get_spline_value(param, x_i)
        utility = utility + y_i
	

        #savings are estimated for an average homeowner
        x_i = project.total_net_savings
        param = EParamTypes.HODesignDecisionEstimatedNetSavings
        y_i = get_spline_value(param, x_i)
        utility + utility + y_i



	
        return utility



    def offer_system(system):
        install_FLAG = 0
        #check utility compared to none
        #for installer
        res  = utility_sei(system)
        error = -math.log(1 / np.random.uniform() - 1)

        #generate error
        if (res + error > settings.THETA_SEI['EParamTypes::HOSEIDecisionUtilityNone']):
            res = utility_system(system)
            error = -math.log(1 / np.random.uniform() - 1)
            if (res + error > settings.THETA_SEI['EParamTypes::HODesignDecisionUtilityNone']):
                install_FLAG = 1

            
            

        #for system
        #return yes or no  no = 0, yes = 1
        return install_FLAG



'''

params for conjoint estimates
markup 

'''
class SEI:
    
    def __init__(self, model):
        self.markup = 1.0
        self.pvpanel = PVPanel()
        self.price_per_watt = 3.5
        self.params = {}
        self.params[EParamTypes.SEIEquipmentType] = 1
        self.params[EParamTypes.SEIInteractionType] = 1
        self.params[EParamTypes.SEIRating] = 3
        self.params[EParamTypes.HOSEIDecisionTotalProjectTime] = 8 #in ticks
        self.params[EParamTypes.SEIWarranty] = 25
        self.model = model


        return super().__init__(**kwargs)


    def offer_systems(self, hs):
        #offer each hs system 
        for h in hs:
            res, system = offer_system(h) 

            if res:
                model.systems.append(system.copy())
                

    '''
        h - Homeowner
    '''
    def offer_system(h):
        #system
        system = PVSystem()
        #number of panels
        system.n_panels = h.el_d * settings.demand_share / ((settings.solar_irradiation) * self.pvpanel.efficiency \
			* (self.pvpanel.length * self.pvpanel.width/1000000) * (settings.dc_to_ac))
        system.dc_size = system.n_panels * self.pvpanel.efficiency * self.pvpanel.length * self.pvpanel.width;
        
        #financial part
        system.demand = h.el_d
        system.total_costs = system.dc_size * self.price_per_watt

        #technical side
        system.pvpanel = self.pvpanel 
        system.inverter = 0 #0 for central or 2 micro

        system.failure_rate = failure_rate(system)

        


        #savigns
        savings_system(system)


        #offer system 
        system.sei = self



        res = h.offer_system(system)

        return res, system


    def failure_rate(design):
        design.failure_rate = 0.0
        design.failure_rate = design.failure_rate + settings.SEMFailureRatePVModule * design.N_PANELS
        
        if (design.inverter == 0):
            design.failure_rate = design.failure_rate + settings.SEMFailureRateInverterCentral
 
        if (desing.inverter == 2):
            design.failure_rate += design.failure_rate + settings. SEMFailureRateInverterMicro * design.N_PANELS


    def savings_system(system):
        inflation = settings.inflation
        CPI = 1.0
        AC_size_t = system.dc_size * settings.dc_to_ac
            
        degradation_t = system.pvpanel.degradation
        production_t = 0.0
    
        consumption_t = demand_ * NUMBER_DAYS_IN_YEAR

        loan_amount = system.total_costs * (1 - settings.tax_incentive)
        interest_rate_loan = settings.interest_rate
        warranty_length = 25
        
        loan_length = warranty_length
    
        NUMBER_MONTHS_IN_YEAR = 12
        N_loan = loan_length * NUMBER_MONTHS_IN_YEAR
        loan_annuity = (interest_rate_loan/NUMBER_MONTHS_IN_YEAR)/(1 - math.pow((1 + interest_rate_loan/NUMBER_MONTHS_IN_YEAR), - N_loan))*loan_amount
        total_production = 0.0
        potential_energy_costs = 0.0
        realized_energy_income = 0.0

        for i in range(warranty_length):
            production_t = AC_size_t * settings.solar_irradiation * NUMBER_DAYS_IN_YEAR
            total_production += production_t
            potential_energy_costs += consumption_t * CPI * settings.p_electricity_d
            realized_energy_income += production_t * CPI * settings.p_electricity_s
            AC_size_t = AC_size_t * degradation_t
            CPI = CPI * (1 + inflation)

        system.total_net_savings = (realized_energy_income - loan_annuity * N_loan)/potential_energy_costs
    

        system.co2_equivalent = total_production/warranty_length * settings.EnergyToCO2/1000
    







class MathTools:
    def __init__(self, **kwargs):
        self.label_i = 'type1'
        self.HO_coefs = {self.label_i:{}}
        self.HO_x_i = {self.label_i:{}}

        return super().__init__(**kwargs)

    def get_spline_value(param_, x_i):

	    #find position for x in the spline? 
	    y_i = 0.0
	    n = HO_x_i[label_i][param_].size()

        x_i_min = [n for n in HO_x_i[label_i][param_] if n < x_i]
        h = x_i - max(x_i_min, 0.0)

	    #interpolate to the left
	    if (x_i < HO_x_i[label_i][param_][0]):
		    #only x, x^2 
		    #assume that S''(a)=S''(b)=0 - so d_j = 0 and thus no h^3
		    y_i = HO_coefs[label_i][param_][0][0] \
			    + HO_coefs[label_i][param_][1][0] * h \ 
			    + HO_coefs[label_i][param_][2][0] * h * h \


	    if (x_i > HO_x_i[label_i][param_].back()):

		    #only x, x^2
		    #only x, x^2 
		    y_i = HO_coefs[label_i][param_][0][n - 1] \
			    + HO_coefs[label_i][param_][1][n - 1] * h \
			    + HO_coefs[label_i][param_][2][n - 1] * h * h \
	    else
		    #all x, x^2, x^3
		    y_i = HO_coefs[label_i][param_][0][idx] \
			    + HO_coefs[label_i][param_][1][idx] * h \
			    + HO_coefs[label_i][param_][2][idx] * h * h \
			    + HO_coefs[label_i][param_][3][idx] * h * h * h \





	    return y_i
        




    def calculate_splines():

        label_i = self.label_i
        
        with open('../../ABMIRISLab/Source/Examples/BaseModel/ho-installerdecisions.json') as infile:
            data_raw = json.load(infile)

            HO_x_i = data_raw[label_i]['spline_points']
            HOD_distribution_scheme = data_raw[label_i]['scheme']

            self.calculate_splines(HO_x_i, HOD_distribution_scheme)
            self.HO_x_i = HO_x_i

        with open('../../ABMIRISLab/Source/Examples/BaseModel/ho-designdecisions.json') as infile:
            data_raw = json.load(infile)

            HO_x_i = data_raw[label_i]['spline_points']
            HOD_distribution_scheme = data_raw[label_i]['scheme']

            self.calculate_splines(HO_x_i, HOD_distribution_scheme)
            self.HO_x_i.update(HO_x_i)



    def calculate_splines(HO_x_i, HOD_distribution_scheme):      
	    x_i = []
	    y_i = []
	    a_i = []
	    b_i, d_i, h_i, alpha_i = [0.0]
	    c_i, l_i, mu_i, z_i = [0.0]

	    n = 0
	    #for each label 
	    #get x_i, y_i 
        #assume one label


		    #only calculate splines for keys in HO_x_i
		
		for key, value in HO_x_i:
			x_i = value
			#HO_y_i will be in HOD_distribution_scheme
			y_i = HOD_distribution_scheme[key]

			n = y_i.size()

			#calculate spline points
			#1.
			a_i = y_i

			#2.
			b_i = [0.0] * (n - 1)
			d_i = [0.0] * (n - 1)
			h_i = [0.0] * (n - 1)
			alpha_i = [0.0] * (n - 1)


			c_i = [0.0] * (n)
			l_i = [0.0] * (n)
			mu_i = [0.0] * (n)
			z_i = [0.0] * (n)


			for i in range(n-1):   
				#3.
				h_i[i] = x_i[i + 1] - x_i[i]


			for i in range(1, n-1, 1): 
			    #4.
				alpha_i[i] = 3 / h_i[i] * (a_i[i + 1] - a_i[i]) - 3 / h_i[i - 1] * (a_i[i] - a_i[i - 1])



			l_i[0] = 1.0

			for i in range(1, n-1, 1): 
				#7.
				l_i[i] = 2 * (x_i[i + 1] - x_i[i - 1]) - h_i[i - 1] * mu_i[i - 1]
				mu_i[i] = h_i[i] / l_i[i]
				z_i[i] = (alpha_i[i] - h_i[i-1]*z_i[i-1]) / l_i[i]






			#8.
			l_i[n - 1] = 1.0
			for j in range(n-2, -1, -1): 
				c_i[j] = z_i[j] - mu_i[j] * c_i[j + 1]
				b_i[j] = (a_i[j + 1] - a_i[j]) / h_i[j] - h_i[j] * (c_i[j + 1] + 2 * c_i[j]) / 3
				d_i[j] = (c_i[j+1] - c_i[j]) / (3 * h_i[j])



			#save into map
			#assume that map is precreated to the number of labels
			HO_coefs[label_i][key] = [[] * 4]

			#a
			HO_coefs[label_i][key][0] = a_i[0:n-1]
			#b
			HO_coefs[label_i][key][1] = b_i
			#c
			HO_coefs[label_i][key][2] = c_i[0:n-1]
			#d
			HO_coefs[label_i][key][3] = d_i
		    

		








#here calling model? 
#or calling from jupyter? 