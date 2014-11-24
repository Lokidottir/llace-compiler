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
            negation,
            setrepeat
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
            negation,
            setrepeat
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
                    Concatination type. Concatinations are only considered such if they
                    are not immediately contained by containers.
                */
                return (!(isType(segment, types::group))
                     && !(isType(segment, types::option))
                     && !(isType(segment, types::repeat))
                     && !(RegexHelper::firstMatch("(,)",segment).empty()));
            case types::terminal:
                /*
                    Terminal type. As strings are stripped from the source before processing
                    and replaced with "str@<index>" where "index" is a positive integer, it
                    is simple to check if a segment is a terminal as the test is similar for
                    testing for container types in that the index is contained by the strings
                    "str@<" and ">".
                */
                return ((RegexHelper::firstMatch(genRegexBetweenStrings("str@<",">",true),segment) == segment));
            case types::special:
                /*
                    Special type. Checked for by seeing if it's neither a concatination or an
                    alternation and then seeing if the first and last characters are '?'
                */
                return (segment.size() > 1
                     && !(isType(segment, types::concatination))
                     && !(isType(segment, types::alternation))
                     && RegexHelper::firstMatch("(\\S)",segment) == "?"
                     && RegexHelper::lastMatch("(\\S)",segment) == "?");
            case types::identifier:
                /*
                    Identifier type. Checked for with the same regex used to pull the identifier
                    from the grammar. If it's the same as the whole segment, then it matches an
                    identifier.
                */
                return (RegexHelper::firstMatch(EBNF_REGEX_ID_LONE,segment) == segment);
            case types::negation:
                /*
                    Negation type. If the segment is neither a concatination or an alternation
                    and the first character is a '-' then it is thought of as a negation. Not
                    evaluated, but a type is declared.
                */
                return (!(isType(segment, types::concatination))
                     && !(isType(segment, types::alternation))
                     && (RegexHelper::firstMatch("(\\S)",segment) == "-"));
            case types::setrepeat:
                /*
                    Set repeat type. This type denotes a set number of repetitions for a match.
                    All checks for container-like types are made before checking for the setrepeat
                    notation.
                */
                return (!(isType(segment, types::concatination))
                     && !(isType(segment, types::alternation))
                     && !(isType(segment, types::group))
                     && !(isType(segment, types::option))
                     && !(isType(segment, types::repeat))
                     && !(RegexHelper::firstMatch("((\\*(\\s*)[0-9]+)|([0-9]+\\s*\\*))",segment).empty());
            default:
                /*
                    Unnknown type check, always return false.
                */
                return false;
        }
    }

    uint_type type(const std::string& segment) {
        /*
            Checks a segment against every declared type and returns
            the constant that represents it.
        */
        for (uint_type iter = 0; iter < types::typelist.size(); iter++) {
            if (isType(segment, types::typelist[iter])) return types::typelist[iter];
        }
        /*
            If the segment is of no type, then a code that matches no
            type is returned.
        */
        return (types::typelist[types::typelist.size() - 1]) + 1;
    }

    std::string typeStr(const std::string& segment) {
        /*
            Returns a string descriptor for the type of a segment.
        */
        if (isType(segment,types::alternation))   return "alternation";
        if (isType(segment,types::group))         return "group";
        if (isType(segment,types::option))        return "option";
        if (isType(segment,types::repeat))        return "repeat";
        if (isType(segment,types::concatination)) return "concatination";
        if (isType(segment,types::terminal))      return "terminal";
        if (isType(segment,types::special))       return "special";
        if (isType(segment,types::identifier))    return "identifier";
        if (isType(segment,types::negation))      return "negation";
        if (isType(segment,types::setrepeat))     return "setrepeat";
        else /*There is no type for the segment*/ return "notype";
    }
};
#endif
