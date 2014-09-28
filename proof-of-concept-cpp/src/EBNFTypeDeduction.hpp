#ifndef EBNF_TYPE_DEDUCTION_HPP
#define EBNF_TYPE_DEDUCTION_HPP
#include <string>
#include "RegexHelpers.hpp"


namespace EvalEBNF {
	namespace types {
		enum Types {
			alternation,
			group,
			option,
			repeat,
			concatination,
			terminal,
			special,
			identifier,
			negation
		};
		const std::vector<uint_type> typelist = {
			alternation,
			group,
			option,
			repeat,
			concatination,
			terminal,
			special,
			identifier,
			negation
		};
	};
	bool isType(const std::string& segment, uint_type type) {
		/*
			Main type deduction happens here, with each type requiring a number
		*/
		if (segment.empty()) return false;
		switch (type) {
			case types::alternation:
				/*
					Alternation type, is only such if it is not within a container
					or a concatination, as those are able to contain alternations.
				*/
				return (!(isType(segment,types::group))
					 && !(isType(segment,types::option))
					 && !(isType(segment,types::repeat))
					 && !(isType(segment,types::concatination))
					 && !(RegexHelper::firstMatch("(\\|)",segment).empty()));
			case types::group:
				/*
					Container types. A segment is only considered to be a container
					type if the first match for that container is equal to the entire
					original segment.
				*/
				return (RegexHelper::firstMatch(genRegexBetweenStrings("(",")",true),segment) == segment);
			case types::option:
				return (RegexHelper::firstMatch(genRegexBetweenStrings("[","]",true),segment) == segment);
			case types::repeat:
				return (RegexHelper::firstMatch(genRegexBetweenStrings("{","}",true),segment) == segment);
			case types::concatination:
				/*
					
				*/
				return (!(isType(segment, types::group))
					 && !(isType(segment, types::option))
					 && !(isType(segment, types::repeat))
					 && !(RegexHelper::firstMatch("(,)",segment).empty()));
			case types::terminal:
				return ((RegexHelper::firstMatch(genRegexBetweenStrings("str@<",">",true),segment) == segment));
			case types::special:
				return (segment.size() > 1
					 && !(isType(segment, types::concatination))
					 && !(isType(segment, types::alternation))
					 && RegexHelper::firstMatch("(\\S)",segment) == "?"
					 && RegexHelper::lastMatch("(\\S)",segment) == "?");
			case types::identifier:
				return (RegexHelper::firstMatch(EBNF_REGEX_ID_LONE,segment) == segment);
			case types::negation:
				return (!(isType(segment, types::concatination))
					 && !(isType(segment, types::alternation))
					 && (RegexHelper::firstMatch("(\\S)",segment) == "-"));
			default:
				return false;
		}
	}
	
	uint_type type(const std::string& segment) {
		for (uint_type iter = 0; iter < types::typelist.size(); iter++) {
			if (isType(segment, types::typelist[iter])) return types::typelist[iter];
		}
		return (*types::typelist.end()) + 1;
	}
	
	std::string typeStr(const std::string& segment) {
		/*
			Returns a string descriptor for the type of a segment.
		*/
		if (isType(segment,types::alternation))   return "alternation";
		if (isType(segment,types::group))   	  return "group";
		if (isType(segment,types::option))  	  return "option";
		if (isType(segment,types::repeat))  	  return "repeat";
		if (isType(segment,types::concatination)) return "concatination";
		if (isType(segment,types::terminal))	  return "terminal";
		if (isType(segment,types::special)) 	  return "special";
		if (isType(segment,types::identifier))    return "identifier";
		if (isType(segment,types::negation))	  return "negation";
		else									  return "notype";
	}
};
#endif
