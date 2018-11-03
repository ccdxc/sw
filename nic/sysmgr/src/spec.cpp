#include <string>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>

#include "spec.hpp"

namespace pt = boost::property_tree;

static std::string name_from_obj(pt::ptree obj)
{
   return obj.get<std::string>("name");
}

static std::string command_from_obj(pt::ptree obj)
{
   return obj.get<std::string>("command");
}

static int flags_from_obj(pt::ptree obj)
{
   int flags = 0;
   
   for (auto flag: obj.get_child("flags")) {
      if (boost::iequals(flag.second.data(), "restartable")) {
	 flags |= RESTARTABLE;
      } else if (boost::iequals(flag.second.data(), "no_watchdog")) {
	 flags |= NO_WATCHDOG;
      } else if (boost::iequals(flag.second.data(), "non_critical")) {
	 flags |= NON_CRITICAL;
      } else {
	 throw std::runtime_error("Unknown flag: " + flag.second.data());
      }
   }
   return flags;
}

static std::vector<std::string> dependencies_from_obj(pt::ptree obj)
{
   std::vector<std::string> dependencies;

   for (auto dep: obj.get_child("dependencies")) {
      dependencies.push_back(dep.second.data());
   }

   return dependencies;
}

static Spec spec_from_obj(pt::ptree obj)
{
   return Spec(name_from_obj(obj),
	       flags_from_obj(obj),
	       command_from_obj(obj),
	       dependencies_from_obj(obj));
}

vector<Spec> specs_from_json(const char *filename)
{
   pt::ptree root;
   vector<Spec> specs;

   read_json(filename, root); // throws exception if it fails

   for (auto process: root) {
      specs.push_back(spec_from_obj(process.second));
   }

   return specs;
}

