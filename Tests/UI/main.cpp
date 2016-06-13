//
//  main.cpp
//  ABMSolar
//
// Hi there
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//


/** /!\ HERE BE DRAGONS /!\
 *
 ^    ^
 / \  //\
 |\___/|      /   \//  .\
 /O  O  \__  /    //  | \ \
 /     /  \/_/    //   |  \  \
 @___@'    \/_   //    |   \   \
 |       \/_ //     |    \    \
 |        \///      |     \     \
 _|_ /   )  //       |      \     _\
 '/,_ _ _/  ( ; -.    |    _ _\.-~        .-~~~^-.
 ,-{        _      `-.|.-~-.           .~         `.
 '/\      /                 ~-. _ .-~      .-~^-.  \
 `.   {            }                   /      \  \
 .----~-.\        \-'                 .~         \  `. \^-.
 ///.----..>    c   \             _ -~             `.  ^-`   ^-_
 ///-._ _ _ _ _ _ _}^ - - - - ~                     ~--,   .-~
 
 */


#include "gtest/gtest.h"


#include <thread>
#include <iostream>
#include <boost/filesystem.hpp>
#include "Tests/UI/WMock.h"
#include "Tests/Agents/SEIMock.h"
#include "UI/UI.h"
#include "Agents/H.h"



using namespace solar_core;




namespace solar_tests
{
    static boost::filesystem::path path_to_model_file;
    
    
    // The fixture for testing payment system
    class WTest : public ::testing::Test
    {
    protected:
        WTest()
        {
        }
        
        
        virtual ~WTest()
        {
        }
        
        
        static void SetUpTestCase()
        {
            
            EXPECT_NE(access(path_to_model_file.string().c_str(), F_OK), -1);
            
            //create world
            w = WMock::create(path_to_model_file.string(), "NEW");
            
            //initialize world
            w->init();
            
            
            //create ui
            ui = new solar_ui::UI();
            
            w->FLAG_IS_STARTED = true;
            w->FLAG_IS_STOPPED = false;
            w->FLAG_IS_PAUSED = false;

            
            //run world for 1 tick to allow for the delayed initialization to take place
            //start threads
//            std::vector<std::thread> threads;
//            void (*func)();
//            
//            func = &PSTest::WActTickHH;
//            threads.push_back(std::thread(func));
//            
//            // work for the workers to finish
//            for(auto& t : threads)
//            {
//                t.join();
//            };
            
            
            
        }
        
        
        static void WActTickHH()
        {
            for (auto& agent:w->get_hhs())
            {
                //get tick
                agent->act_tick();
            };
        };
        
        
        
        static void TearDownTestCase()
        {
            //deletes world
            //now does nothing, @DevStage2 later will clean agents
            
            delete w;
            w = nullptr;
            
            delete ui;
            ui = nullptr;
        }
        
        
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        virtual void SetUp()
        {
            // Code here will be called immediately after the constructor (right
            // before each test).
        }
        
        virtual void TearDown()
        {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }
        
        
        virtual solar_core::Household* get_hh(std::size_t i)
        {
            return w->get_hhs()[i];
        };
        
        
        
        
        //@{
        /** Main parameters  */
        
        static WMock* w;
        static solar_ui::UI* ui;
        
        //@}
        
        
        //@{
        /** Parameters to be shared between test runs */
        
        
        static double demand;
        static double solar_irradiation;
        static double permit_difficulty;
        static double project_percentage;
        
        //@}
        
        
        
    };
    
    
    //@{
    /** Initialization of static data members */
    
    
    double WTest::demand = 30;
    double WTest::solar_irradiation = 5;
    double WTest::permit_difficulty = 2;
    double WTest::project_percentage = 0.8;
    
    WMock* WTest::w = nullptr;
    solar_ui::UI* WTest::ui = nullptr;
    
    
    //@}
    
    
    /**
     
     Test that hh is created according to template
     
    */
    TEST_F(WTest, CreateHH)
    {
        //number of hh
        
        //all are different
        
        
        
    }
    
    

    
    
    TEST_F(WTest, DesignPV)
    {
        
        auto pdf_i = boost::uniform_int<uint64_t>(0, w->get_hhs().size() - 1);
        auto rng_i = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand->rng, pdf_i);
        
        auto project = std::make_shared<PVProject>();
        SEIMock* seimock = dynamic_cast<SEIMock*>(w->get_seis()[0]);
        project->sei = seimock;
        project->agent = w->get_hhs()[rng_i()];
        auto design = PVDesign();
        auto decs = seimock->get_dec_solar_modules();
        
        
        seimock->form_design_for_params(project, demand, solar_irradiation, permit_difficulty, project_percentage, *(++decs.begin()), design);
        
        
        
        EXPECT_NE(design.N_PANELS, 1.0);
        EXPECT_EQ(design.N_PANELS, 24.0);
        
        EXPECT_EQ(design.DC_size, 6.24);
        EXPECT_EQ(design.AC_size, 4.99);
        
        EXPECT_EQ(design.energy_savings_money, 34320);
        
        
        
        
    }
    
    
    TEST_F(WTest, DesignPVMoney)
    {
        auto pdf_i = boost::uniform_int<uint64_t>(0, w->get_hhs().size() - 1);
        auto rng_i = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand->rng, pdf_i);
        
        auto project = std::make_shared<PVProject>();
        SEIMock* seimock = dynamic_cast<SEIMock*>(w->get_seis()[0]);
        project->sei = seimock;
        project->agent = w->get_hhs()[rng_i()];
        auto design = PVDesign();
        auto decs = seimock->get_dec_solar_modules();
        
        
        seimock->form_design_for_params(project, demand, solar_irradiation, permit_difficulty, project_percentage, *(++decs.begin()), design);
        
        seimock->ac_estimate_savings(design, project);
        
        EXPECT_EQ(design.energy_savings_money, 34320.00);
        
        
        
    }
    
    
    TEST_F(WTest, SendMarketingInf)
    {
        //testing if marketing information is sent out
        
        
        
        
        
    }
    
    
    
    TEST_F(WTest, CreatJointDistribution)
    {
        //testing if joint distribution is calculated correctly
        
        
        
        
    }
    
    
    

} //end solar_tests namespace




int main(int argc, char * argv[])
{
    
    solar_tests::path_to_model_file = boost::filesystem::path(argv[1]);
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
















