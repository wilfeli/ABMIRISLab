//
//  Serialize.cpp
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 4/11/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#include "Tools/ExternalIncludes.h"
#include "Tools/Serialize.h"


bool
serialize::is_parenthesis(const std::string& token_)
{
    return (token_ == "(" || token_ == ")");
}


bool
serialize::is_operator(const std::string& token_)
{
    return (token_ == "+" || token_ == "-" || token_ == "*" || token_ == "/" || token_ == "^");
}





std::vector<std::string>
serialize::split_expression_SYAlg(const std::string& expression_)
{
    std::vector<std::string> tokens_;
    std::string str_ = "";
    
    
    for (auto c:expression_)
    {
        const std::string token(1, c);
        if ( is_operator(token) || is_parenthesis(token) )
        {
            if ( !str_.empty() )
            {
                tokens_.push_back(str_);
            };
            str_ = "";
            
            if ((token == "-") && ((tokens_.size() == 0) || is_operator(tokens_.back()) || (is_parenthesis(tokens_.back()))))
            {
                str_.append(token);
            }
            else
            {
                tokens_.push_back(token);
            };
        }
        else
        {
            // Append the numbers
            if ( !token.empty() && (token != " ") )
            {
                str_.append(token);
            }
            else
            {
                if (str_ != "")
                {
                    tokens_.push_back(str_);
                    str_ = "";
                };
            };
        };
    };
    
    
    //flash last string
    if (str_ != "")
    {
        tokens_.push_back(str_);
        str_ = "";
    };
    
    return tokens_;
}



std::list<std::string>
serialize::infixToRPN_SYAlg(const std::string& expression_)
{
    std::list<std::string> outq;
    std::stack<std::string> stack;
    serialize::SYAlg alg;
    
    
    std::string R = "Right";
    std::string L = "Left";
    std::string RPAREN = ")";
    std::string LPAREN = "(";
    
    auto tokens = split_expression_SYAlg(expression_);
    
    //    for (std::string token:tokens)
    for (auto iter = tokens.begin(); iter != tokens.end(); ++iter)
    {
        auto token = *iter;
        if (!is_parenthesis(token) && !(is_operator(token)))
        {
            outq.push_back(token);
        }
        else
        {
            auto token1 = token;
            auto p1 = alg.ops[token1].first;
            auto a1 = alg.ops[token1].second;
            
            while (stack.size())
            {
                auto token2 = stack.top();
                auto p2 = alg.ops[token2].first;
                auto a2 = alg.ops[token2].second;
                
                if (((a1 == L) && (p1 <= p2)) || ((a1 == R) && (p1 < p2)))
                {
                    if ( token1 != RPAREN)
                    {
                        if ( token2 != LPAREN)
                        {
                            stack.pop();
                            outq.push_back(token2);
                        }
                        else
                        {
                            break;
                            
                        };
                    }
                    else
                    {
                        if ( token2 != LPAREN)
                        {
                            stack.pop();
                            outq.push_back(token2);
                        }
                        else
                        {
                            stack.pop();
                            break;
                        };
                        
                    };
                }
                else
                {
                    break;
                };
            };
            
            if ( token1 != RPAREN)
            {
                stack.push(token1);
            };
        };
    };
    
    //drain stack
    while (stack.size())
    {
        outq.push_back(stack.top());
        stack.pop();
    };
    
    return outq;
}



/**
 
 
 Evaluate RPN
 
 */
double
serialize::evaluate_rpn(std::list<std::string>& tokens)
{
    std::stack<double> stack;
    
    for (std::string token:tokens)
    {
        if (is_operator(token))
        {
            //evaluate stack
            double secondOperand = stack.top();
            stack.pop();
            double firstOperand = stack.top();
            stack.pop();
            if (token == "*")
            {
                stack.push(firstOperand * secondOperand);
            }
            else if (token == "/")
            {
                stack.push(firstOperand / secondOperand);
            }
            else if (token == "-")
            {
                stack.push(firstOperand - secondOperand);
            }
            else if (token == "+")
            {
                stack.push(firstOperand + secondOperand);
            }
            else if (token == "^")
            {
                stack.push(std::pow(firstOperand, secondOperand));
            }
            else
            { //just in case
            }
        }
        else
        {
            stack.push(std::stod(token));
        };
    };
    
    return stack.top();
}




/**
 
 
 Simple algorithm to solve mathematical formulas
 
 
 */
double
serialize::solve_formula(std::string str_)
{
    //algortithm to solve formula
    auto rpn = infixToRPN_SYAlg(str_);
    auto result = evaluate_rpn(rpn);
    
    return result;
}




/**
 
 Parses distribution
 
 */


serialize::ParsedDist
serialize::create_dist_from_formula(const std::string& formula_, solar_core::IRandom* rand_)
{
    serialize::ParsedDist dist;
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
                
                
                dist.params.push_back(mean);
                dist.params.push_back(sigma2);
                dist.type = solar_core::ERandomParams::N_trunc;
                dist.valid_dist = true;
                
                
            }
            ///careful here - will find both u and u_int
            else if (formula.find("u") != std::string::npos)
            {
                double min = std::stod(formula.substr(formula.find("(") + 1, formula.find(",") - formula.find("(") - 1));
                double max = std::stod(formula.substr(formula.find(",") + 1, formula.find(",") - formula.find(")") - 1));
                
                dist.params.push_back(min);
                dist.params.push_back(max);
                
                if (formula.find("u_int") != std::string::npos)
                {
                    dist.type = solar_core::ERandomParams::u_int;
                }
                else
                {
                    dist.type = solar_core::ERandomParams::u;
                };
                dist.valid_dist = true;
                
            }
            else if (formula.find("CUSTOM") != std::string::npos)
            {
                dist.type = solar_core::ERandomParams::custom;
                dist.valid_dist = true;
            };
            
        };
        
    }
    else
    {
    };
    
    
    return dist;
}






