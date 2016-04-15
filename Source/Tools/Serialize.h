//
//  IParameters.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/24/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#ifndef ABMSolar_Serialize_h
#define ABMSolar_Serialize_h

#include "Tools/ExternalIncludes.h"
#include "Tools/IParameters.h"
#include "Tools/IRandom.h"


namespace serialize
{

    
    
using namespace solar_core;
    
    
/**
 
 
 Service function for serialization through string stream.
 
 
 */
template <typename In>
void serialize_value(In& in_, std::string& out_)
{
    std::stringstream ss;
    
    ss << in_;
    
    out_ = ss.str();
}
    
    
    
    
    
/**
 
 General template for serialization of a map
 */
template<class K, class V, class Enable = void>
class SerializeImpl2
{
public:
    
    /**
     
     
     Serialize simple map
     
     It is going through output stream to accomodate maps with complex data types
     
     */
    static PropertyTree serialize(std::map<K, V>& container_, PropertyTree::key_type const& key)
    {
        PropertyTree pt;
        PropertyTree child;
        PropertyTree children;
        
        std::string value;
        std::string key_value;
        
        for (auto elem:container_)
        {
            serialize::serialize_value(elem.second, value);
            serialize::serialize_value(elem.first, key_value);
            
            child.put(key_value, value);
            children.push_back(std::make_pair("", child));
            child.clear();
        };
        
        pt.add_child(key, children);
        return pt;
    };
};
    
    
template<class T, class Enable = void>
class DeserializeValue
{
public:
    static void deserialize_value(const std::string& in_, T& out_)
    {
        std::stringstream ss(in_);
        ss >> out_;
    }
    
    static void deserialize_value(std::stringstream& ss, T& out_)
    {
        ss >> out_;
    }
};


template<class T>
class DeserializeValue<T, typename std::enable_if<std::is_same<T, std::string>::value>::type>
{
public:
    static void deserialize_value(const std::string& in_, T& out_)
    {
        std::stringstream ss(in_);
        out_ = ss.str();
    }
    
    static void deserialize_value(std::stringstream& ss, T& out_)
    {
        out_ = ss.str();
    }
};
    
    
    

/**
 
 General template for serializatio of a map
 */
template<class K, class V, class Enable = void>
class DeserializeMap
{
public:
    static void deserialize(const PropertyTree& pt, std::map<K, V>&  r)
    {
        K key;
        V value;
        
        for (const auto& node: pt)
        {
            for (const auto& item : node.second)
            {
                DeserializeValue<K>::deserialize_value(item.first, key);
                std::stringstream ss;
                
                ss << item.second.get_value<std::string>();
                
                DeserializeValue<V>::deserialize_value(ss, value);
                
                r.insert(std::make_pair(key, value));
            };
        };
    }
    
    
    
};
    
    
template<class K, class V>
class DeserializeMap<K, V, typename std::enable_if<std::is_pointer<V>::value>::type>
{
public:
    static void deserialize(const PropertyTree& pt, std::map<K, V>&  r)
    {
        K key;
        V value;
        
        //node is of the form "" : { "key": "value"}
        for (const auto& node: pt)
        {
            for (const auto& item : node.second)
            {
                DeserializeValue<K>::deserialize_value(item.first, key);
                
                //create new object passing in PropertyTree
                value = V::element_type::deserialize(item.second);
                
                r.insert(std::make_pair(key, value));
            };
        };
    }
};

    
/**
 
 Deserialize into map
 
 
 */
template<typename K, typename V>
void deserialize(const PropertyTree& pt, std::map<K, V>&  r)
{
    return DeserializeMap<K, V>::deserialize(pt, r);
};


//@{
/**
 
 
 Simple algorithm to solve mathematical formulas
 
 
 */
double solve_formula(std::string str_);

double evaluate_rpn(std::list<std::string>& tokens);

std::list<std::string> infixToRPN_SYAlg(const std::string& expression_);

std::vector<std::string> split_expression_SYAlg(const std::string& expression_);

bool is_parenthesis(const std::string& token_);

bool is_operator(const std::string& token_);


    
class SYAlg
{
public:
    std::map<std::string, std::pair<long, std::string>> ops;
    
    SYAlg()
    {
        std::string R = "Right";
        std::string L = "Left";
        
        //create ops
        ops["^"] = std::make_pair(4, R);
        ops["*"] = std::make_pair(3, L);
        ops["/"] = std::make_pair(3, L);
        ops["+"] = std::make_pair(2, L);
        ops["-"] = std::make_pair(2, L);
        ops["("] = std::make_pair(9, L);
        ops[")"] = std::make_pair(0, L);
    }
    
};

//@}
/**
 
 MARK: cont.
 
 */
template <class T>
T solve_str_formula(std::string& formula_, IRandom& rand_)
{
    T dest_;
    
    if (formula_.find("FORMULA::") != std::string::npos)
    {
        std::regex e("");
        e.assign("FORMULA\\u003A\\u003A");
        formula_ = std::regex_replace(formula_, e, "");
        
        //check if it is requesting random number generation
        if (formula_.find("p.d.f.") != std::string::npos)
        {
            e.assign("p.d.f.\\u003A\\u003A");
            formula_ = std::regex_replace(formula_, e, "");
            
            //parse formula
            //case of a truncated normal
            if (formula_.find("N_trunc") != std::string::npos)
            {
                double mean = std::stod(formula_.substr(formula_.find("(") + 1, formula_.find(",") - formula_.find("(") - 1));
                double sigma2 = std::stod(formula_.substr(formula_.find(",") + 1, formula_.find(",") - formula_.find(")") - 1));
                
                ///@DevStage1 change to Truncated generation
                formula_ = std::to_string((rand_.rnd() + mean) * std::pow(sigma2, 0.5));
                
            }
            else if (formula_.find("u") != std::string::npos)
            {
                double min = std::stod(formula_.substr(formula_.find("(") + 1, formula_.find(",") - formula_.find("(") - 1));
                double max = std::stod(formula_.substr(formula_.find(",") + 1, formula_.find(",") - formula_.find(")") - 1));
                
                formula_ = std::to_string(rand_.rnd() * (max - min) + min);
            };
            
        };
 
    }
    else
    {
    };
    
    //convert string into value
    DeserializeValue<T>::deserialize_value(std::to_string(solve_formula(formula_)), dest_);
    
    return dest_;
}
    

} //end of namespace serialize
#endif
