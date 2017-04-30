//
//  Simulation.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 6/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/Simulation.h"
#include "Tools/Serialize.h"
#include "Tools/ParsingTools.h"
#include "Tools/IRandom.h"

using namespace solar_core;


tools::EmpiricalMVD::~EmpiricalMVD()
{
    for (auto& dist: mvd)
    {
        delete dist;
    };
}


tools::EmpiricalMVD* tools::create_joint_distribution(std::string path_to_scheme, std::string path_to_data)
{
    //read file
    //MARK: cont. change to the pointer
    std::vector<std::vector<double>>* parsed_file = new std::vector<std::vector<double>>();
    std::string path_to_file;
    
    tools::parse_csv_file(path_to_data, parsed_file);
    
    
    //read file with specification
    //assume each high level has a name for the variable in joint distribution
    //iterate over them, get N_BINS,
    
    PropertyTree pt;
    read_json(path_to_scheme, pt);
    
    EmpiricalMVD* e_dist = new EmpiricalMVD();
    
    std::vector<long> N_BINS;
    std::vector<int64_t> N_BINS_CUM;
    
    //total number of bins
    N_BINS_CUM.push_back(1);
    
    for (const auto& node: pt)
    {
        e_dist->mvd.push_back(new EmpiricalUVD());
        N_BINS.push_back(node.second.get<long>("N_BINS"));
        
        //get bin values if discrete, otherwise assume it will be just numbers
        if (node.second.get<std::string>("type") == "continuous")
        {
            e_dist->mvd.back()->type = ERandomParams::ContinuousDiscretized;
            //bin edges
            serialize::deserialize(node.second.get_child("BIN_ENDS"), e_dist->mvd.back()->bin_ends);
        }
        else
        {
            e_dist->mvd.back()->type = ERandomParams::Discrete;
        };

        //read BIN_VALUES from the file
        serialize::deserialize(node.second.get_child("values"), e_dist->mvd.back()->bin_values);
        
        for (auto i = 0; i < e_dist->mvd.back()->bin_values.size(); ++i)
        {
            e_dist->mvd.back()->bin_values_map[e_dist->mvd.back()->bin_values[i]] = i;
        };
        
        
        //read name from the file
        e_dist->mvd.back()->name = node.second.get<std::string>("name");
    };
    
    for (auto i = 0; i < N_BINS.size(); ++i)
    {
        //MARK: change to push back
        N_BINS_CUM.push_back(N_BINS_CUM[i] * N_BINS[N_BINS.size() - 1 - i]);
    };
    
    
    
    //joint distribution table
    e_dist->values = std::vector<std::vector<long>>(N_BINS_CUM.back(), std::vector<long>(N_BINS.size(), 0.0));
    
    
    for (int64_t i = 0; i < e_dist->values.size(); ++i)
    {
        for(int64_t j = 0; j < N_BINS.size(); ++j)
        {
            //
            auto N_TAILS = N_BINS_CUM[N_BINS_CUM.size() - 2 - j];
            auto i_sub = i % (N_TAILS * N_BINS[j]);
            long k = i_sub/N_TAILS;
            e_dist->values[i][j] = k; //from what bin to take values    //BIN_VALUES[j][k];
        };
    };
    
    
    //create vector of frequencies
    e_dist->freq = std::vector<long>(N_BINS_CUM.back(), 0);
    
    int64_t N_TAILS = 0;
    int64_t bin = 0;
    int64_t i = 0;
    //reverse order
    for (auto x_i:(*parsed_file))
    {
        for (auto j = 0; j < x_i.size() - 1; ++j)
        {
            N_TAILS = N_BINS_CUM[N_BINS_CUM.size() - 2 - j];
            bin = e_dist->mvd[j]->bin_values_map[x_i[j+1]];
            i += bin * N_TAILS;
        };
        e_dist->freq[i] += 1;
        i = 0;
    };
    
    //store index into actual bin values
    for (auto value_add = 0; value_add < e_dist->mvd[0]->bin_values.size(); ++value_add)
    {
        e_dist->mvd[0]->cond_values.push_back(std::vector<long>{});
        e_dist->mvd[0]->cond_values.back().push_back(value_add);
        
        //calculate conditional pmf
       calculate_pmf(e_dist->mvd[0]->cond_values, parsed_file, e_dist->mvd[0]->cond_freq, e_dist, N_BINS_CUM);
        
    };
    
    //take table from the previous one and multiply by own values
    for (auto i = 1; i < e_dist->mvd.size(); ++i)
    {
        //take previous distribution and multiply by own bin values
        for (auto value: e_dist->mvd[i-1]->cond_values)
        {
            for (auto value_add = 0 ; value_add< e_dist->mvd[i]->bin_values.size(); ++value_add)
            {
                e_dist->mvd[i]->cond_values.push_back(value);
                e_dist->mvd[i]->cond_values.back().push_back(value_add);

#ifdef ABMS_DEBUG_MODE
				if (e_dist->mvd[i]->cond_values.back().size() > 1000000) 
				{
					std::cout << "something is wrong" << std::endl;
				};
#endif
            };
        };
        
        //calculate conditional pmf
        calculate_pmf(e_dist->mvd[i]->cond_values, parsed_file, e_dist->mvd[i]->cond_freq, e_dist, N_BINS_CUM);
        
    };

    //save information about tail length
    e_dist->n_bins_cum = N_BINS_CUM;
    
    return e_dist;
}



void
tools::calculate_pmf(std::vector<std::vector<long>>& bins, std::vector<std::vector<double>>* parsed_file, std::vector<long>& freq_n, EmpiricalMVD* e_dist, std::vector<int64_t>& N_BINS_CUM)
{
    freq_n = std::vector<long>(bins.size(), 0);
    auto i_pos = 0;
    int64_t N_TAILS = 0;
    for (auto i = 0; i < bins.size(); ++i)
    {
        //calculate starting position for this bin
        for(auto j = 0; j < bins[i].size(); ++j)
        {
            N_TAILS = N_BINS_CUM[N_BINS_CUM.size() - 2 - j];
            i_pos += bins[i][j] * N_TAILS;
            
        };
        
        //sum over the number of tails
        for (auto k = 0; k < N_TAILS; ++k){
            freq_n[i] += e_dist->freq[i_pos + k];
        }
        
        i_pos = 0;
    };
}





std::vector<double>
tools::draw_joint_distribution(EmpiricalMVD* pmf, IRandom* rand)
{
    
    std::vector<double> x;
    std::vector<long> i_x; //indices for bin values for each drawn discrete distribution, and into bins for continuous - the same because used for collapsing cond distribution
    //initialize uniform distribution
    double u_i = 0.0;
    
    
    //go through the list of univariate distributions
    for (auto& dist:pmf->mvd)
    {
        //in indexes last value will be bin index
        //collapse conditional for already drawn values
        //returns list of indices into main list for conditional distribution
        auto cond_dist = collapse_pmf(i_x, dist, pmf);
        
        //calculate scale factor
        auto scale_factor = calculate_scale_factor(cond_dist, dist);
        
        //draw first from conditional distribution
        //create cmf - vector of end cumulative values from cond_dist
        auto cmf = create_cmf(cond_dist, scale_factor, dist);
        
        //if it is discrete - just get inverse index, cmf has value and index; and bin value from that
        //draw next uniform
        u_i = rand->ru();
        auto i = get_inverse_index(cmf, u_i);
        i_x.push_back(dist->cond_values[cond_dist[i]].back());
        if (dist->type == ERandomParams::Discrete)
        {
            x.push_back(dist->bin_values[i_x.back()]);
        }
        else if (dist->type == ERandomParams::ContinuousDiscretized)
        {
            //if it is continuous - get inverse index and inverse value from the corresponding description
            //will point to the bin to use - simple linear inverse
            //check if it is last bin and if it will include inf right end
            
            if (( i == dist->theta_bins.size() - 1) && (dist->bin_ends.back() == constants::SOLAR_INFINITY()))
            {
                x.push_back(get_inverse_value_exp(dist->theta_bins[i], u_i));
            }
            else
            {
                x.push_back(get_inverse_value(dist->theta_bins[i], u_i));
            };
        };
        
    };
    return x;
}




std::vector<long>
tools::collapse_pmf(std::vector<long>& i_x, EmpiricalUVD* dist, EmpiricalMVD* pmf)
{
    std::vector<long> cond_dist;
    bool FLAG_EQ = true;
    //pick only rows with values from there
    for (auto i = 0; i < dist->cond_values.size(); ++i)
    {
        //should still push all as will never go into the cycle in the first place
        FLAG_EQ = true;
        for (auto j = 0; j < i_x.size(); ++j)
        {
            if (dist->cond_values[i][j] != i_x[j])
            {
                FLAG_EQ = false;
                break;
            };
        };
        if (FLAG_EQ)
        {
            //will always push continious chunk because of the layout length will be N_BINS
            for (auto k = 0; k < dist->bin_values.size(); ++k)
            {
                //save position into the distiribution table
                cond_dist.push_back(i + k);
            };
            break;
        };
    };
    
    
    return cond_dist;
    
}



double
tools::calculate_scale_factor(std::vector<long>& cond_dist, EmpiricalUVD* dist)
{
    double scale_factor = 0;
    
    for (auto i:cond_dist)
    {
        scale_factor += dist->cond_freq[i];
    };
    
    
    return scale_factor;
}



std::vector<double>
tools::create_cmf(std::vector<long>& cond_dist, double scale_factor, EmpiricalUVD* dist)
{
    std::vector<double> cmf{0};
    for (auto i = 0; i < cond_dist.size(); ++i)
    {
        cmf.push_back(cmf[i] + dist->cond_freq[cond_dist[i]] / scale_factor);
    };
    
    if (dist->type == ERandomParams::ContinuousDiscretized)
    {
        dist->theta_bins.clear();
        //update theta_bins
        for (auto i = 1; i < cmf.size(); ++i)
        {
            dist->theta_bins.push_back(std::vector<double>{});
            if (( i != cmf.size() - 1) || ((i == cmf.size() - 1) && (dist->bin_ends.back() != constants::SOLAR_INFINITY())))
            {
                dist->theta_bins.back().push_back(cmf[i-1]);
                dist->theta_bins.back().push_back(dist->bin_ends[i] - dist->bin_ends[i-1]);
                dist->theta_bins.back().push_back(cmf[i] - cmf[i-1]);
                dist->theta_bins.back().push_back(dist->bin_ends[i-1]);
            }
            else
            {
                //scale it down
//                std::cout << std::endl;
                dist->theta_bins.back().push_back(dist->bin_ends[(i - 1)]);
                dist->theta_bins.back().push_back(std::exp(dist->bin_ends[(i - 1)]/dist->theta_bins.back()[0]) * (1 - cmf[i-1]));
                
            };
        };
    };
    
    
    return cmf;
    
}


long
tools::get_inverse_index(std::vector<double>& cmf, double u_i)
{
    long i = 1;
    for (; i < cmf.size(); ++i)
    {
        if ((u_i < cmf[i]) && (u_i >= cmf[i-1]))
        {
            break;
        };
    };

    
    //breaks when crossed over, so returns step back
    return i - 1;


	////here implement nlog(n) algorithm 
	//long count, step;
	//count = cmf.size();
	//long it = 0;
	//long first = 0;

	//while (count > 0) {
	//	it = first;
	//	step = count / 2;
	//	it += step;
	//	if (cmf[it] < u_i) {
	//		first = ++it;
	//		count -= step + 1;
	//	}
	//	else
	//		count = step;
	//}
	//return first - 1;


}



double
tools::get_inverse_value(std::vector<double>& theta_bin, double u_i)
{
    return (u_i - theta_bin[0])*(theta_bin[1]/theta_bin[2])+theta_bin[3];
    
}


double
tools::get_inverse_value_exp(std::vector<double>& theta_bin, double u_i)
{
    //uses exponent as a proxy
    double ret = (-std::log((1 - u_i)/theta_bin[1]) * theta_bin[0]);
    
//    if (ret > 1000000)
//    {
////        throw std::runtime_error("too big simulation value");
//    };
    
    return ret;
}


/**

Algorithm is taken from :
https://en.wikipedia.org/w/index.php?title=Spline_%28mathematics%29&oldid=288288033#Algorithm_for_computing_natural_cubic_splines

also references
https://github.com/ttk592/spline to double check logic


*/
void solar_core::tools::EmpiricalHUVD::calculate_spline_coefs()
{

	std::vector<double> x_i;
	std::vector<double> y_i;
	std::vector<double> a_i;
	std::vector<double> b_i, d_i, h_i, alpha_i = std::vector<double>(0.0);
	std::vector<double> c_i, l_i, mu_i, z_i = std::vector<double>(0.0);

	long n = 0;
	//for each label 

	//get x_i, y_i 
	for (long label_i = 0; label_i < labels.size(); ++label_i) 
	{
		//only calculate splines for keys in HO_x_i
		
		for (auto iter : HO_x_i[label_i])
		{
			x_i = HO_x_i[label_i][iter.first];
			//HO_y_i will be in HOD_distribution_scheme
			y_i = HOD_distribution_scheme[labels[label_i]][iter.first];

			n = y_i.size();

			//calculate spline points
			//1.
			a_i = y_i;

			//2.
			b_i = std::vector<double>(n - 1, 0.0);
			d_i = std::vector<double>(n - 1, 0.0);
			h_i = std::vector<double>(n - 1, 0.0);
			alpha_i = std::vector<double>(n - 1, 0.0);


			c_i = std::vector<double>(n, 0.0);
			l_i = std::vector<double>(n, 0.0);
			mu_i = std::vector<double>(n, 0.0);
			z_i = std::vector<double>(n, 0.0);


			for (long i = 0; i < n - 1; ++i)
			{
				//3.
				h_i[i] = x_i[i + 1] - x_i[i];
			};

			for (long i = 1; i < n - 1; ++i)
			{
				//4.
				alpha_i[i] = 3 / h_i[i] * (a_i[i + 1] - a_i[i]) - 3 / h_i[i - 1] * (a_i[i] - a_i[i - 1]);
			};



			l_i[0] = 1.0;

			for (long i = 1; i < n - 1; ++i)
			{
				//7.
				l_i[i] = 2 * (x_i[i + 1] - x_i[i - 1]) - h_i[i - 1] * mu_i[i - 1];
				mu_i[i] = h_i[i] / l_i[i];
				z_i[i] = (alpha_i[i] - h_i[i-1]*z_i[i-1]) / l_i[i];
			};






			//8.
			l_i[n - 1] = 1.0;
			for (long j = n - 2; j >= 0; --j)
			{
				c_i[j] = z_i[j] - mu_i[j] * c_i[j + 1];
				b_i[j] = (a_i[j + 1] - a_i[j]) / h_i[j] - h_i[j] * (c_i[j + 1] + 2 * c_i[j]) / 3;
				d_i[j] = (c_i[j+1] - c_i[j]) / (3 * h_i[j]);
			};


//#ifdef ABMS_DEBUG_MODE
//			//tests for calculations
//			if (iter.first == EParamTypes::HODesignDecisionEstimatedNetSavings)
//			{
//				for (long i = 0; i < n - 1; ++i)
//				{
//					std::cout << c_i[i] << " " << b_i[i] << " " << d_i[i] << std::endl;
//				};
//			};
//#endif


			//save into map
			//assume that map is precreated to the number of labels
			HO_coefs.push_back({});
			HO_coefs[label_i][iter.first] = std::vector<std::vector<double>>(4, std::vector<double>{});
			//a
			HO_coefs[label_i][iter.first][0] = std::vector<double>(a_i.begin(), a_i.begin() +  (n - 1));
			//b
			HO_coefs[label_i][iter.first][1] = b_i;
			//c
			HO_coefs[label_i][iter.first][2] = std::vector<double>(c_i.begin(), c_i.begin() + (n - 1));
			//d
			HO_coefs[label_i][iter.first][3] = d_i;
		};

		

	};



}

double solar_core::tools::EmpiricalHUVD::get_spline_value(EParamTypes param_, double x_i, long label_i)
{
	//
	//find position for x in the spline? 
	double y_i = 0.0;
	long n = HO_x_i[label_i][param_].size();
	std::vector<double>::const_iterator it;
	it = std::lower_bound(HO_x_i[label_i][param_].begin(), HO_x_i[label_i][param_].end(), x_i);
	int idx = std::max(int(it - HO_x_i[label_i][param_].begin()) - 1, 0);
	double h = 0.0;
	h = x_i - HO_x_i[label_i][param_][idx];

//#ifdef ABMS_DEBUG_MODE
//	std::cout << x_i << std::endl;
//	std::cout << HO_x_i[label_i][param_][0] << std::endl;
//#endif

	//interpolate to the left
	if (x_i < HO_x_i[label_i][param_][0]) 
	{
		//only x, x^2 
		//assume that S''(a)=S''(b)=0 - so d_j = 0 and thus no h^3
		y_i = HO_coefs[label_i][param_][0][0]
			+ HO_coefs[label_i][param_][1][0] * h
			+ HO_coefs[label_i][param_][2][0] * h * h;

	}
	else if (x_i > HO_x_i[label_i][param_].back())
	{
		//only x, x^2
		//only x, x^2 
		y_i = HO_coefs[label_i][param_][0][n - 1]
			+ HO_coefs[label_i][param_][1][n - 1] * h
			+ HO_coefs[label_i][param_][2][n - 1] * h * h;

	}
	else 
	{
		//all x, x^2, x^3
		y_i = HO_coefs[label_i][param_][0][idx]
			+ HO_coefs[label_i][param_][1][idx] * h
			+ HO_coefs[label_i][param_][2][idx] * h * h
			+ HO_coefs[label_i][param_][3][idx] * h * h * h;

	};

	//interpolate to the right

	//interpolate
//#ifdef ABMS_DEBUG_MODE
//	std::cout << idx << std::endl;
//	std::cout << y_i << std::endl;
//#endif


	return y_i;
}
