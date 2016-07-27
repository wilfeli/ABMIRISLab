//
//  SEMBL.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 7/9/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "UI/WEE.h"
#include "Agents/SEMBL.h"
#include "Agents/SolarPanel.h"

using namespace solar_core;


int SEMBL::N_complexity_params = 2;


/**
 
 
 Cholevsky decomposition from http://stackoverflow.com/questions/6142576/sample-from-multivariate-normal-gaussian-distribution-in-c
 
 */
SEMBL::SEMBL(const PropertyTree& pt_, W* w_): SEM(pt_, w_), mean_rw_complexity_dist(N_complexity_params, 1), sigma_rw_complexity_dist(N_complexity_params, N_complexity_params), normTransform_rw_complexity_dist(N_complexity_params, N_complexity_params), sample_ind_rw_complexity_dist(N_complexity_params, 1), sample_rw_complexity_dist(N_complexity_params, 1)
{
    
    //generate from THETA_dist_complexity
    for (auto i = 0; i < N_complexity_params * N_complexity_params; ++i)
    {
        sigma_rw_complexity_dist(i % N_complexity_params, i / N_complexity_params) = THETA_dist_complexity[N_complexity_params + i];
    };

    for (auto i = 0; i < N_complexity_params; ++i)
    {
        mean_rw_complexity_dist(i, 0) = THETA_dist_complexity[i];
    };
    
    
    Eigen::LLT<Eigen::MatrixXd> cholSolver(sigma_rw_complexity_dist);
    
    // We can only use the cholesky decomposition if
    // the covariance matrix is symmetric, pos-definite.
    // But a covariance matrix might be pos-semi-definite.
    // In that case, we'll go to an EigenSolver
    if (cholSolver.info() == Eigen::Success)
    {
        // Use cholesky solver
        normTransform_rw_complexity_dist = cholSolver.matrixL();
    }
    else
    {
        // Use eigen solver
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigenSolver(sigma_rw_complexity_dist);
        normTransform_rw_complexity_dist = eigenSolver.eigenvectors() * eigenSolver.eigenvalues().cwiseSqrt().asDiagonal();
    }
    
    
    
}




void SEMBL::add_connection(std::shared_ptr<SolarModuleBL> link)
{
    if (solar_panel_templates[EDecParams::NewTechnology] == link)
    {
        ++N_connections_new;
    }
    else
    {
        ++N_connections_current;
    };
}


void SEMBL::remove_connection(std::shared_ptr<SolarModuleBL> link)
{
    if (solar_panel_templates[EDecParams::NewTechnology] == link)
    {
        --N_connections_new;
    }
    else
    {
        --N_connections_current;
    };
}


void SEMBL::ac_update_tick()
{
    lock.lock();
    
    //update internal timer
    a_time = w->time;
    history_sales[a_time % history_sales.size()] = 0.0;
    
    
    //switch techonologies
    if (N_connections_current == 0)
    {
        solar_panel_templates[EDecParams::CurrentTechnology] = solar_panel_templates[EDecParams::NewTechnology];
        solar_panel_templates[EDecParams::NewTechnology] = nullptr;
        N_connections_current += N_connections_new;
        N_connections_new = 0;
    };
    
    lock.unlock();
}

/**
 
 
 TODO: for efficiency growth look at the historical values for the efficiency and rate of progress, that will give rate - THETA_dist_efficiency[0], variance of a progress - THETA_dist_efficiency[1], current value for the average efficiency - THETA_dist_efficiency[2].
 
 
 
 */
void SEMBL::act_tick()
{
    
    ac_update_tick();
    
    
    //if time is up - advance techonology
    //offer new panel with randomly drawn efficiency - as a random walk
    //and random reliability and complexty, ass random walks - lognormal variety
    //calibrate efficiency increase and degradation? - as a proxy to quality
    
    
    //draw new efficiency from the random walk
    //draw new reliability rate lambda from random walk
    //draw new mean and variance for complexity from random walk
    
    //assume for now they are separate distributions
    
    //new efficiency from random walk
    THETA_dist_efficiency[2] = THETA_dist_efficiency[2] * std::exp(w->rand->rnd() * std::pow(THETA_dist_efficiency[1], 0.5) + THETA_dist_efficiency[0]);
    
    //new reliability from random walk
    THETA_dist_reliability[2] = THETA_dist_reliability[2] * std::exp(w->rand->rnd() * std::pow(THETA_dist_reliability[1], 0.5) + THETA_dist_reliability[0]);
    
    //new complexity from random walk
    //generate N_complexity_params of independent N(0, 1)
    //find how to vectorize this call - for fun
    for (auto i = 0; i < N_complexity_params; ++i)
    {
        sample_ind_rw_complexity_dist[i] = w->rand->rnd();
    };
    
    sample_rw_complexity_dist = (normTransform_rw_complexity_dist * sample_ind_rw_complexity_dist).colwise() + mean_rw_complexity_dist;
    
    //update parameters for the solar_panel
    THETA_dist_complexity[N_complexity_params * 3] = sample_rw_complexity_dist(0,0);
    THETA_dist_complexity[N_complexity_params * 3 + 1] = sample_rw_complexity_dist(0,1);
    
   
    std::shared_ptr<SolarModuleBL> new_pv(new SolarModuleBL(*solar_panel_templates[EDecParams::CurrentTechnology]));
    
    //set params for new module
    new_pv->efficiency = THETA_dist_efficiency[2];
    
    
    
    //think if there is problem that reliability is updated as random log-normal walk, but prior is assumed to be Gamma. Might need to make it gamma too?
    

    
    //update uid
    new_pv->
    

    solar_panel_templates[EDecParams::NewTechnology] = new_pv;

    
    
    
}