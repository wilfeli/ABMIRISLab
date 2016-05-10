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
#include "ExternalTools/boost_1_57_0/boost/filesystem.hpp"
#include "UI/W.h"





using namespace solar_core;




namespace solar_tests
{
    
    
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
            w = new WMock(path_to_model_file.string(), "NEW");
            
            //initialize world
            w->init();
            
            
            //create ui
            ui = new solar_ui::UIW(w);
            
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
            //now DevStage1 does nothing, later will clean agents
            
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
            return w->hhs[i];
        };
        
        
        
        
        
        
        
        static WMock* w;
        static solar_ui::UIW* ui;
        
        
    };
    
    WMock* WTest::w = nullptr;
    solar_ui::UIW* WTest::ui = nullptr;
    
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
        
        auto pdf_i = boost::uniform_int<uint64_t>(0, get_hhs().size() - 1);
        auto rng_i = boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t>>(w->rand->rng, pdf_i);
        
        auto project = std::make_shared<PVProject>();
        SEIMock seimock = *get_seis()[0];
        project->sei = seimock;
        project->agent = get_hhs()[rng_i()];
        double demand = 300;
        double solar_radiation = 5;
        double permit_difficulty = 2;
        double project_percentage = 0.8;
        auto design = PVDesign();
        auto iter = project->sei->dec_solar_modules.begin();
        
        
        project->sei->form_design_from_param(project, demand, solar_radiation, permit_difficulty, project_percentage, iter, design);
        
        
        
        EXPECT_EQ(design.N_PANELS, 1.0);
        
        
        
        
    }
    
    
    
    
    
    
    

} //end solar_tests namespace




int main(int argc, char * argv[])
{
    
    static boost::filesystem::path path_to_model_file(argv[1]);
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
















