// ==========================================================================
//                    DP-Based Optimal Threshold Computation
// ==========================================================================
// Copyright (c) 2014-2015, Enrico Siragusa, FU Berlin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Enrico Siragusa or the FU Berlin nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ENRICO SIRAGUSA OR THE FU BERLIN BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ==========================================================================
// Author: Enrico Siragusa <enrico.siragusa@fu-berlin.de>
// ==========================================================================

// ============================================================================
// Prerequisites
// ============================================================================

// ----------------------------------------------------------------------------
// SeqAn headers
// ----------------------------------------------------------------------------

#include <seqan/sequence.h>
#include <seqan/index.h>
#include <seqan/arg_parse.h>

using namespace seqan;

// ============================================================================
// Classes
// ============================================================================

// ----------------------------------------------------------------------------
// Class Options
// ----------------------------------------------------------------------------

struct Options
{
    typedef std::string             TString;
    typedef std::vector<TString>    TList;

    enum Algorithm
    {
        ALGO_LEMMA, ALGO_EXACT, ALGO_APX
    };

    CharString      shape;
    unsigned        length;
    unsigned        errors;
    bool            edit;

    Algorithm       algorithm;
    TList           algorithmList;

    Options() :
        edit(false)
    {
        algorithmList.push_back("lemma");
        algorithmList.push_back("exact");
        algorithmList.push_back("apx");
    }
};

// ============================================================================
// Functions
// ============================================================================

template <typename TOption, typename TString, typename TOptionsList>
void getOptionEnum(TOption & option,
                   TString const & optionStr,
                   TOptionsList & optionsList)
{
    typedef typename Iterator<TOptionsList, Standard>::Type TOptionsIterator;

    TOptionsIterator optionsBegin = begin(optionsList, Standard());
    TOptionsIterator optionsEnd = end(optionsList, Standard());

    TOptionsIterator optionPos = std::find(optionsBegin, optionsEnd, optionStr);

    option = (optionPos != optionsEnd) ? TOption(optionPos - optionsBegin) : TOption();
}

template <typename TOption, typename TString, typename TOptionsList>
void getOptionValue(TOption & option,
                    ArgumentParser const & parser,
                    TString const & optionName,
                    TOptionsList & optionsList)
{
    typedef typename Value<TOptionsList>::Type              TOptionString;

    TOptionString optionStr;
    getOptionValue(optionStr, parser, optionName);

    return getOptionEnum(option, optionStr, optionsList);
}

// ----------------------------------------------------------------------------
// Function setupArgumentParser()
// ----------------------------------------------------------------------------

template <typename TOptions>
inline void setupArgumentParser(ArgumentParser & parser, TOptions const & options)
{
    setAppName(parser, "shape_threshold");
    setShortDescription(parser, "Compute the minimum threshold of a given q-gram shape");
    setCategory(parser, "Approximate string matching");

    addUsageLine(parser, "[\\fIOPTIONS\\fP] <\\fISHAPE\\fP> <\\fILENGTH\\fP> <\\fIERRORS\\fP>");

    addArgument(parser, ArgParseArgument(ArgParseArgument::STRING));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INTEGER));
    addArgument(parser, ArgParseArgument(ArgParseArgument::INTEGER));

    setMinValue(parser, 1, "10");
    setMaxValue(parser, 1, "100");
    setMinValue(parser, 2, "0");
    setMaxValue(parser, 2, "10");

    addOption(parser, ArgParseOption("a", "algorithm", "Algorithm to use.", ArgParseOption::STRING));
    setValidValues(parser, "algorithm", options.algorithmList);
    setDefaultValue(parser, "algorithm", options.algorithmList[options.algorithm]);

    addOption(parser, ArgParseOption("e", "edit", "Use edit distance. Default: Hamming distance."));
}

// ----------------------------------------------------------------------------
// Function parseCommandLine()
// ----------------------------------------------------------------------------

template <typename TOptions>
ArgumentParser::ParseResult
inline parseCommandLine(TOptions & options, ArgumentParser & parser, int argc, char const ** argv)
{
    ArgumentParser::ParseResult res = parse(parser, argc, argv);

    if (res != ArgumentParser::PARSE_OK)
        return res;

    getArgumentValue(options.shape, parser, 0);
    getArgumentValue(options.length, parser, 1);
    getArgumentValue(options.errors, parser, 2);
    getOptionValue(options.algorithm, parser, "algorithm", options.algorithmList);
    getOptionValue(options.edit, parser, "edit");

    return ArgumentParser::PARSE_OK;
}

// ----------------------------------------------------------------------------
// Function computeThreshold()
// ----------------------------------------------------------------------------

template <typename TShape, typename TAlgorithm>
inline int computeThreshold(Options const & options, TShape const & shape, TAlgorithm const & algorithm)
{
    if (options.edit)
        return qgramThreshold(shape, options.length, options.errors, EditDistance(), algorithm);
    else
        return qgramThreshold(shape, options.length, options.errors, HammingDistance(), algorithm);
}

template <typename TShape>
inline int computeThreshold(Options const & options, TShape const & shape)
{
    switch (options.algorithm)
    {
    case Options::ALGO_LEMMA:
        return computeThreshold(options, shape, ThreshQGramLemma());
    case Options::ALGO_EXACT:
        return computeThreshold(options, shape, ThreshExact());
    case Options::ALGO_APX:
        return computeThreshold(options, shape, ThreshHeuristic());
    default:
        throw RuntimeError("Unsupported algorithm");
    }
}

inline int computeThreshold(Options const & options)
{
    Shape<Dna, SimpleShape>     contiguous;
    Shape<Dna, GenericShape>    gapped;

    if (stringToShape(contiguous, options.shape))
        return computeThreshold(options, contiguous);
    else if (stringToShape(gapped, options.shape))
        return computeThreshold(options, gapped);
    else
        throw RuntimeError("Unsupported q-gram shape");
}

// ----------------------------------------------------------------------------
// Function main()
// ----------------------------------------------------------------------------

int main(int argc, char const ** argv)
{
    ArgumentParser parser;
    Options options;
    setupArgumentParser(parser, options);

    ArgumentParser::ParseResult res = parseCommandLine(options, parser, argc, argv);

    if (res != ArgumentParser::PARSE_OK)
        return res;

    try
    {
        double start = sysTime();
        int threshold = computeThreshold(options);
        double finish = sysTime();

        std::cout << threshold  << '\t';
        std::cout << std::fixed << finish - start << std::endl;
    }
    catch (Exception const & e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
