//
//  SEI.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/22/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_SEI_h
#define ABMSolar_SEI_h







namespace solar_core
{
    

    
/**
 
 
 
 */
class MesMarketing;
class MesMarketingQuote;
class Household;
    

/**
 
 For marketing - use marketing institute as an intermediary. SEI submits marketing information, marketing institute passes it to H based on the parameters, such as effectiveness
 
 
 
 
 @wp For choice function could try neural nets - train them, include two sets of parameters, one will be parameters of an installer and the second will be parameters of a project. The resulting node will be probability of accepting project. Estimation will be done on a database with all installed solar panels. It is equivalent to density estimation using NN. Could also use PCA when the estimated component is assumed to be profit from the project. Could use Python with sci-kit or check TensorFlow. Once estimation is done - could use it as a simple linear function to produce estimation of profit - and thus estimation for the chance of accepting project. Basically dataset will produce profit estimation in some form, should still work even if have only accepted projects. For new project - will get profit estimation. 
 
 
 
 */
class SEI
{
public:
    std::shared_ptr<MesMarketingQuote> get_quote(Household* agent_in); /*!< @DevStage2 think about transforming this call into interface based one, with agent_in replaced by interface and it being virtual method from the general interface. But virtual call might be more costly and unnecessary in this case, as structure of who will be requesting quotes does not change. */
protected:
    
    
    MesMarketing* mes_marketing; /*!< is created with agent and updated to reflect new marketing policy */

    
    
    

};
} //end namespace solar_core




#endif
