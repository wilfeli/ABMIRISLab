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
    
    
    template<typename T>
    class is_vector{
    public:
        static const bool value = false;
    };
    
    template <typename T>
    class is_vector<std::vector<T>>
    {
    public:
        static const bool value = true;
    };
    
    
    
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
     
     General template for serializatio of a vector or deque of objects. Code for vector and deque is exactly the same
     
     */
    template<class T, class Enable = void>
    class SerializeContainer
    {
    public:
        
        static PropertyTree serialize(std::vector<T>& container_, PropertyTree::key_type const& key)
        {
            PropertyTree pt;
            PropertyTree child;
            PropertyTree children;
            
            for (auto elem:container_)
            {
                child.put("", elem);
                children.push_back(std::make_pair("", child));
            };
            
            pt.add_child(key, children);
            return pt;
        }
        
        
        static PropertyTree serialize(std::deque<T>& container_, PropertyTree::key_type const& key)
        {
            PropertyTree pt;
            PropertyTree child;
            PropertyTree children;
            
            for (auto elem:container_)
            {
                child.put("", elem);
                children.push_back(std::make_pair("", child));
            };
            
            pt.add_child(key, children);
            return pt;
        }
        
    };
    
    
    
    /**
     
     General template for serialization of a map
     */
    template<class K, class V, class Enable = void>
    class SerializeMap
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
    
    
    //here could add serialization of complex maps
    template<class K, class V>
    class SerializeMap<K, V, typename std::enable_if<is_vector<V>::value>::type>
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
            
            typedef typename V::value_type ValueType;
            
            
            for (auto elem:container_)
            {
                serialize::serialize_value(elem.first, key_value);
                child.put_child(key_value, SerializeContainer<ValueType>::serialize(elem.second, "dummy").get_child("dummy"));
                children.push_back(std::make_pair("", child));
                child.clear();
            };
            
            
            pt.add_child(key, children);
            
            return pt;
        }
    };
    
    
    
    /**
     
     
     
     Wrapper for call to serialize vector, deque, map. Does not change.
     
     */
    
    template<class T>
    PropertyTree serialize(std::vector<T>& container_, PropertyTree::key_type const& key) { return SerializeContainer<T>::serialize(container_, key); }
    
    template<class T>
    PropertyTree serialize(std::deque<T>& container_, PropertyTree::key_type const& key) { return SerializeContainer<T>::serialize(container_, key); }
    
    template <typename K, typename V>
    PropertyTree serialize(std::map<K, V>& container_, PropertyTree::key_type const& key){ return SerializeMap<K, V>::serialize(container_, key);}
    
    
    template<class T, class Enable = void>
    class DeserializeValue
    {
    public:
        static void deserialize_value(const std::string& in_, T& out_)
        {
            std::stringstream ss(in_);
            ss >> out_;
        }
        
        
        static T deserialize_value(const std::string& in_)
        {
            T out_;
            std::stringstream ss(in_);
            ss >> out_;
            return out_;
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
        
        static T deserialize_value(const std::string& in_)
        {
            T out_ = in_;
            return out_;
        }
        
        
    };
    
    
    template<class T, class Enable = void>
    class DeserializeContainer
    {
    public:
        
        static void deserialize(const PropertyTree& pt, std::vector<T>& r)
        {
            for (auto& item : pt)
            {
				try
				{
					r.push_back(item.second.get_value<T>());
				}
				catch (const std::exception&)
				{
					if (item.second.get_value<std::string>() == "inf")
					{
						r.push_back(std::numeric_limits<T>::infinity());
					};
				}
				catch (...) 
				{
					if (item.second.get_value<std::string>() == "inf")
					{
						r.push_back(std::numeric_limits<T>::infinity());
					};
				}


				//try {
				//	r.push_back(item.second.get_value<T>());
				//{
				//catch (...) {
				//	//check if it is infinity value
				//	if (item.second.get_value<std::string>() == "inf") 
				//	{
				//		r.push_back(std::numeric_limits<T>::infinity());
				//	};
				//};
            };
        }
        
        static void deserialize(const PropertyTree& pt, std::deque<T>& r)
        {
            for (auto& item : pt)
            {
                r.push_back(item.second.get_value<T>());
            };
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
    
    
    
    template<class K, class V>
    class DeserializeMap<K, std::shared_ptr<V>>
    {
    public:
        static void deserialize(const PropertyTree& pt, std::map<K, std::shared_ptr<V>>&  r)
        {
            K key;
            std::shared_ptr<V> value;
            
            //node is of the form "" : { "key": "value"}
            for (const auto& node: pt)
            {
                for (const auto& item : node.second)
                {
                    DeserializeValue<K>::deserialize_value(item.first, key);
                    
                    //create new object passing in PropertyTree
                    value = V::deserialize(item.second);
                    
                    r.insert(std::make_pair(key, value));
                };
            };
        }
    };
    
    
    
    /**
     
     
     */
    template<class K, class V>
    class DeserializeMap<K, V, typename std::enable_if<is_vector<V>::value>::type>
    {
    public:
        static void deserialize(const PropertyTree& pt, std::map<K, V>& r)
        {
            K key{};
            V value{};
            typedef typename V::value_type ValueType;
            
            for (auto& node: pt)
            {
                for (auto& item : node.second)
                {
                    DeserializeValue<K>::deserialize_value(item.first, key);
                    DeserializeContainer<ValueType>::deserialize(item.second, value);
                    r.insert(std::make_pair(key, value));
                    value.clear();
                };
            };
        };
        
    };
    
    
    template<class T>
    void deserialize(const PropertyTree& pt, std::vector<T>& r) { DeserializeContainer<T>::deserialize(pt, r); }
    
    
    template<class T>
    void deserialize(const PropertyTree& pt, std::deque<T>& r) { DeserializeContainer<T>::deserialize(pt, r); }
    
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
    T solve_str_formula(const std::string& formula_, IRandom& rand_)
    {
        T dest_;
        std::string formula = formula_;
        
        if (formula.find("FORMULA::") != std::string::npos)
        {
            std::regex e("");
            e.assign("FORMULA\\u003A\\u003A");
            formula = std::regex_replace(formula, e, "");
            
            //check if it is requesting random number generation
            if (formula.find("p.d.f.") != std::string::npos)
            {
                e.assign("p.d.f.\\u003A\\u003A");
                formula = std::regex_replace(formula, e, "");
                
                //parse formula
                //case of a truncated normal
                if (formula.find("N_trunc") != std::string::npos)
                {
                    double mean = std::stod(formula.substr(formula.find("(") + 1, formula.find(",") - formula.find("(") - 1));
                    double sigma2 = std::stod(formula.substr(formula.find(",") + 1, formula.find(",") - formula.find(")") - 1));
                    
                    ///@DevStage2 change to better Truncated generation, now is simple rejection algorithm
                    formula = std::to_string(std::max(rand_.rnd() * std::pow(sigma2, 0.5) + mean, 0.0));
                    
                }
                ///careful here - will find both u and u_int
                else if (formula.find("u") != std::string::npos)
                {
                    double min = std::stod(formula.substr(formula.find("(") + 1, formula.find(",") - formula.find("(") - 1));
                    double max = std::stod(formula.substr(formula.find(",") + 1, formula.find(",") - formula.find(")") - 1));
                    
                    formula = std::to_string(rand_.ru() * (max - min) + min);
                }
                else if (formula.find("CUSTOM") != std::string::npos)
                {
                    e.assign("CUSTOM");
                    formula = std::regex_replace(formula, e, "");
                    //MARK: cont. add log here 
                };
                
            };
            
        }
        else
        {
        };
        
        //convert string into value
        DeserializeValue<T>::deserialize_value(std::to_string(solve_formula(formula)), dest_);
        
        return dest_;
    }
    
    class ParsedDist
    {
    public:
        ParsedDist() = default;
        ERandomParams type = ERandomParams::None;
        bool valid_dist = false;
        std::vector<double> params;
    };
    
    
    
    ParsedDist create_dist_from_formula(const std::string& formula_, solar_core::IRandom* rand_);
    
    
    
    
} //end of namespace serialize
#endif
