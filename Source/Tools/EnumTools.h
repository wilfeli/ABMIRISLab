#pragma once


#include <string>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq/elem.hpp>

namespace solar_core
{

/**
* Taken from here http://stackoverflow.com/questions/5093460/how-to-convert-an-enum-type-variable-to-a-string
*/
#define ABMS_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE(r, data, elem)  \
    case data::elem : return BOOST_PP_STRINGIZE(data::elem);


#define ABMS_DEFINE_ENUM_WITH_STRING_CONVERSIONS_FROMSTRING_NAME(r, data, elem)\
	BOOST_PP_CAT(BOOST_PP_SEQ_ELEM(1,data),::)elem                             \

#define ABMS_DEFINE_ENUM_WITH_STRING_CONVERSIONS_FROMSTRING_CASE(r, data, elem)\
	if (BOOST_PP_SEQ_ELEM(0, data) ==  BOOST_PP_STRINGIZE(ABMS_DEFINE_ENUM_WITH_STRING_CONVERSIONS_FROMSTRING_NAME(r, data, elem))) \
	{																		  \
		return BOOST_PP_CAT(BOOST_PP_SEQ_ELEM(1,data),::)elem;	              \
	};



#define DEFINE_ENUM_WITH_STRING_CONVERSIONS(name, enumerators)                \
    enum class name: long {                                                   \
        BOOST_PP_SEQ_ENUM(enumerators)                                        \
    };                                                                        \
                                                                              \
    inline const char* EnumToString(name v)                                   \
    {                                                                         \
        switch (v)                                                            \
        {                                                                     \
            BOOST_PP_SEQ_FOR_EACH(                                            \
                ABMS_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE,       \
                name,                                                         \
                enumerators                                                   \
            )                                                                 \
            default: return "[Unknown " BOOST_PP_STRINGIZE(name) "]";         \
        }                                                                     \
    }																		  \
																			  \
																			  \
	inline name BOOST_PP_CAT(name,FromString)(std::string v)		          \
	{																		  \
		 BOOST_PP_SEQ_FOR_EACH(                                               \
                ABMS_DEFINE_ENUM_WITH_STRING_CONVERSIONS_FROMSTRING_CASE,     \
                (v)(name),                                                    \
                enumerators                                                   \
            )                                                                 \
		return name::None;													  \
	}		


} //end namespace solar_core