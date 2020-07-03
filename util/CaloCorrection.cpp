//File: CaloCorrection.h
//Brief: A CaloCorrection corrects recoil energy (without vertex box, with OD, etc.) to
//       a better estimator for total hadronic energy.  It mimics Minerva::CalorimetryUtils
//       in doing this by interpolating a multiplicative constant between a series of points.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "util/CaloCorrection.h"

//c++ includes
#include <string>
#include <cstdlib> //std::getenv()
#include <regex>
#include <fstream>
#include <stack>
#include <cassert>
#include <sstream>
#include <algorithm>
#include <iostream>

#define FORBID(KEYWORD, STATE)\
  assert(line.find("KEYWORD") == std::string::npos && KEYWORD "keyword does not make sense in the " STATE "state!");

namespace util
{
  bool foundKeyword(const std::string& line, const std::string& keyword, size_t& nextToken)
  {
    nextToken = line.find(keyword);
    if(nextToken != std::string::npos) nextToken += keyword.length();
    return nextToken != std::string::npos;
  }

  std::string& replaceEnvVars(std::string& path)
  {
    std::regex envVar(R"(\$[[:alnum:]]+)");
    std::smatch results;

    while(std::regex_search(path, results, envVar))
    {
      auto toReplace = results.str();
      for(size_t found = toReplace.find("{"); found != std::string::npos; found = toReplace.find("{")) toReplace.erase(found, 1);
      for(size_t found = toReplace.find("}"); found != std::string::npos; found = toReplace.find("}")) toReplace.erase(found, 1);

      const char* env = std::getenv(toReplace.substr(1).c_str()); //Important to skip the $
      if(!env) env = "";

      for(auto& result: results) path.replace(std::distance(path.cbegin(), result.first), std::distance(result.first, result.second), env);
    }

    return path;
  }

  std::unordered_map<std::string, CaloCorrection> CaloCorrection::parse(const std::string& caloFile)
  {
    std::unordered_map<std::string, CaloCorrection> corrections;
    auto currentCorr = corrections.end();

    //Model this parsing job as a state machine
    enum class State
    {
      Top, //When not in a BEGIN/END pair, look for BEGIN and IMPORT
      Loading //When between a BEGIN and an END, look for POLYPOINTs
    };

    //Theorem: Any recurive algorithm can be rewritten using a LIFO
    std::stack<std::string> filesLeft;
    filesLeft.push(caloFile);

    while(!filesLeft.empty())
    {
      std::ifstream currentFile(replaceEnvVars(filesLeft.top()));
      filesLeft.pop();
      //--depth; //TODO: This depth check doesn't seem correct because I keep going with the current file

      //int depth = 0;
      State parseState = State::Top;
      std::string line;
      while(std::getline(currentFile, line))
      {
        //Trim whitespace and ignore empty lines and comments
        line = line.substr(line.find_first_not_of(" "), line.length());
        if(line.empty() || line[0] == '#') continue;

        //Parser state machine.
        //Keywords have the following priorities because of how CalorimetryUtils
        //is written.
        size_t nextToken = std::string::npos;
        if(parseState == State::Top)
        {
          if(foundKeyword(line, "BEGIN", nextToken))
          {
            currentCorr = corrections.insert(std::make_pair(line.substr(nextToken), CaloCorrection())).first;
            parseState = State::Loading;
          }
          else if(foundKeyword(line, "IMPORT", nextToken)) 
          {
            filesLeft.push(line.substr(nextToken));
            //++depth; //TODO: This is just filesLeft.size(), not recursion depth.  Maybe I could use CHANGES in filesLeft.size() to track recursion depth?  When it decreases, --depth.  Keep a bool, addedFiles, and ++depth when it's true at the END of this loop.
            //if(depth > 5) throw std::runtime_error("Reached maximum recursion depth of 5 in parseCaloCorrections().  You've probably loaded a calorimetry polyline with recursive IMPORTs."); 
          }
          else
          {
            #ifndef NDEBUG
            std::cout << "In parseCaloCorrections(), ignoring a line that looks like " << line << " because it doesn't match any keywords for State::Top.\n";
            #endif //NDEBUG

            FORBID("END", "Top")
            FORBID("POLYPOINT", "Top")
          }
        }
        else if(parseState == State::Loading)
        {
          if(foundKeyword(line, "POLYPOINT", nextToken))
          {
            std::stringstream findPoint(line.substr(nextToken));
            double x, y;
            findPoint >> x >> y;
            currentCorr->second.fPoints.push_back({x, y});
          }
          else if(foundKeyword(line, "END", nextToken))
          {
            std::sort(currentCorr->second.fPoints.begin(), currentCorr->second.fPoints.end(), [](const auto& lhs, const auto& rhs) { return lhs.threshold < rhs.threshold; });
            parseState = State::Top;
          }
          else
          {
            #ifndef NDEBUG
            std::cout << "In parseCaloCorrections(), ignoring a line that looks like " << line << " because it doesn't match any keywords for State::Loading.\n";
            #endif //NDEBUG

            FORBID("BEGIN", "Loading")
            FORBID("IMPORT", "Loading")
          }
        }
        else assert(false && "Parser got into unknown state!");
      } //End loop over currentFile
    } //End while filesLeft

    return corrections;
  }

  CaloCorrection::CaloCorrection(const std::string& caloFile, const std::string& tuningName)
  {
    const auto parsed = parse(caloFile);
    const auto found = parsed.find(tuningName);
    if(found != parsed.end()) fPoints = found->second.fPoints;
    assert(found != parsed.end() && "Failed to find calorimetric correction by name!");
  }

  GeV CaloCorrection::eCorrection(const MeV rawRecoil) const
  {
    const auto upperPoint = std::lower_bound(std::next(fPoints.begin()), fPoints.end(), rawRecoil, [](const auto& point, const auto raw) { return point.threshold < raw; });
    if(upperPoint != fPoints.end())
    {
      const auto lowerPoint = std::prev(upperPoint);
      return std::max(0., upperPoint->correction + (upperPoint->correction - lowerPoint->correction) * rawRecoil.in<GeV>() / (upperPoint->threshold - lowerPoint->threshold).in<GeV>());
    }
    else return rawRecoil; //rawRecoil is after last point, so don't correct.
                           //This is both a fail-safe mechanism for very energetic
                           //neutrinos which are very rare in MINERvA's <6 GeV>
                           //beam and the Default spline.
  }

  CaloCorrection::CaloCorrection(): fPoints{}
  {
  }
}
