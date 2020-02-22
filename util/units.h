//File: units.h
//Brief: Define base working units for this analysis.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//util includes
#include "util/vector.h"

//BaseUnits includes
#include "units/units.h"

//yaml-cpp includes
#include "yaml-cpp/yaml.h"

#ifndef NEUTRON_UNITS_H
#define NEUTRON_UNITS_H

//Set up YAML bindings for units.  For now, require that unit name matches exactly.
//I learned to do this from https://github.com/jbeder/yaml-cpp/wiki/Tutorial
#define ADD_YAML_TO_UNIT(UNIT)\
  namespace YAML\
  {\
    template <>\
    struct convert<UNIT>\
    {\
      static Node encode(const UNIT& rhs)\
      {\
        Node result;\
        result = rhs.in<UNIT>();\
        result.SetTag(#UNIT);\
        return result;\
      }\
\
      static bool decode(const Node& node, UNIT& value)\
      {\
        if(!node.IsScalar()) return false;\
\
        /*Require that tag matches UNIT's name*/\
        if(node.Tag() != std::string("!" #UNIT)) return false;\
\
        value = UNIT(node.as<double>());\
        return true;\
      }\
    };\
  }\

#define REGISTER_UNIT_NAME(UNIT)\
  template <>\
  struct unit_attributes<UNIT>\
  {\
    constexpr auto name = #UNIT;\
  };

#define DECLARE_UNIT_WITH_YAML(UNIT)\
  DECLARE_UNIT(UNIT)\
  ADD_YAML_TO_UNIT(UNIT)

#define DELCARE_RELATED_UNIT_WITH_YAML(NEW_UNIT, BASE_UNIT, NUM, DENOM)\
  DECLARE_RELATED_UNIT(NEW_UNIT, BASE_UNIT, NUM, DENOM)\
  ADD_YAML_TO_UNIT(NEW_UNIT)

#define DECLARE_UNIT_WITH_TYPE_AND_YAML(UNIT, TYPE)\
  DECLARE_UNIT_WITH_TYPE(UNIT, TYPE)\
  ADD_YAML_TO_UNIT(UNIT)

//Define base units using my new macros
DECLARE_UNIT_WITH_YAML(GeV)
DELCARE_RELATED_UNIT_WITH_YAML(MeV, GeV, 1, 1000)

DECLARE_UNIT_WITH_YAML(mm)
DELCARE_RELATED_UNIT_WITH_YAML(cm, mm, 10, 1)

DECLARE_UNIT_WITH_YAML(ns)

DECLARE_UNIT_WITH_YAML(radians)
DELCARE_RELATED_UNIT_WITH_YAML(degrees, radians, 31415926535897932, 1800000000000000000) //1800000000000000000, 31415926535897932)

DECLARE_UNIT_WITH_YAML(events) //Base events on double so I can reweight

DECLARE_UNIT_WITH_YAML(neutrons)

#endif //NEUTRON_UNITS_H
