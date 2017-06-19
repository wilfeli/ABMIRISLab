//
//  IParameters.h
//  ABMSolar
//
//  Created by Ekaterina Sinitskaya on 2/24/16.
//  Copyright (c) 2016 IRIS Lab. All rights reserved.
//

#pragma once
#ifndef ABMSolar_SerializeRJ_h
#define ABMSolar_SerializeRJ_h

#include "Tools/ExternalIncludes.h"
#include "Tools/IParameters.h"
#include "Tools/IRandom.h"





namespace serialize
{

	using namespace solar_core;


	template<class T>
	class GetNodeValue
	{
	public:
		static T get_value(const DocumentNode& node_)
		{
			//assume that it could only be non-string here
			if (node_.IsNumber())
			{
				return static_cast<T>(node_.GetDouble());
			}
			else
			{
				//will through most likely, but it should be dead end anyway
				return T();
			};

		}
	};


	//template specialization
	template<>
	class GetNodeValue<std::string>
	{
	public:
		static std::string get_value(const DocumentNode& node_)
		{
			return std::string(node_.GetString());
		}
	};



	template<class T, class Enable = void>
	class DeserializeRJContainer
	{
	public:

		static void deserialize(const DocumentNode& pt, std::vector<T>& r)
		{
			for (auto& item : pt.GetArray())
			{
				try
				{
					//get value from GenericValue
					r.push_back(GetNodeValue<T>::get_value(item));
				}
				catch (const std::exception&)
				{
					if (GetNodeValue<std::string>::get_value(item) == "inf")
					{
						r.push_back(std::numeric_limits<T>::infinity());
					};
				}
				catch (...)
				{
					if (GetNodeValue<std::string>::get_value(item) == "inf")
					{
						r.push_back(std::numeric_limits<T>::infinity());
					};
				}

			};
		}

		static void deserialize(const DocumentNode& pt, std::deque<T>& r)
		{
			for (auto& item : pt)
			{
				r.push_back(GetNodeValue<T>::get_value(item));
			};
		}
	};


	template<class T>
	void deserialize(const DocumentNode& pt, std::vector<T>& r) { DeserializeRJContainer<T>::deserialize(pt, r); }



	/**

	General template for serializatio of a map
	*/
	template<class K, class V, class Enable = void>
	class DeserializeRJMap
	{
	public:
		static void deserialize(const DocumentNode& pt, std::map<K, V>&  r)
		{
			K key;
			V value;

			for (const auto& node : pt.GetArray())
			{
				for (const auto& item : node.GetObject())
				{
					DeserializeValue<K>::deserialize_value(std::string(item.name.GetString()), key);
					std::stringstream ss;

					ss << item.value.GetString();

					DeserializeValue<V>::deserialize_value(ss, value);

					r.insert(std::make_pair(key, value));
				};
			};
		}



	};





	template<class K, class V>
	class DeserializeRJMap<K, std::shared_ptr<V>>
	{
	public:
		static void deserialize(const DocumentNode& pt, std::map<K, std::shared_ptr<V>>&  r)
		{
			K key;
			std::shared_ptr<V> value;

			//node is of the form "" : { "key": "value"}
			for (const auto& node : pt.GetArray())
			{
				for (const auto& item : node.GetObject())
				{
					DeserializeValue<K>::deserialize_value(std::string(item.name.GetString()), key);

					//create new object passing in PropertyTree
					value = V::deserialize(item.value);

					r.insert(std::make_pair(key, value));
				};
			};
		}
	};




	/**

	Deserialize into map


	*/
	template<typename K, typename V>
	void deserialize(const DocumentNode& pt, std::map<K, V>&  r)
	{
		return DeserializeRJMap<K, V>::deserialize(pt, r);
	};




} //end of namespace serialize
#endif
