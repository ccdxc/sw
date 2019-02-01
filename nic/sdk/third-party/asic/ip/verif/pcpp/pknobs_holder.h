#ifndef _PKNOBS_HOLDER_H
#define _PKNOBS_HOLDER_H

#include "pknobs.h"
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <map>

using namespace boost;
using namespace std;
using namespace pknobs;

//+KnobDef=firstKnob
//+firstKnob/type=RKnob
//
//+KnobDef=secondKnob
//+secondKnob/type=RRknob
//+secondKnob/min=100
//+secondKnob/max=200
//
//+KnobDef=thirdKnob
//+thirdKnob/type=WLKnob
//+thirdKnob/numKnobs=2
//+thirdKnob/0/knob=firstKnob
//+thirdKnob/1/knob=secondKnob
//+thirdKnob/[01]/weight=10



class PKnobsHolder {
private:
  static map<string,PKnobsHolder *> knobholder_map;
  string            prefix;
  map<string,Knob*> knob_map;
public:
  PKnobsHolder() : prefix("") {}
  PKnobsHolder(const string & n) : prefix(n) { knobholder_map[n]= this;}
  static PKnobsHolder * get_knobholder(const string &n) {
    auto i = knobholder_map.find(n);
    if (i == knobholder_map.end())
        return nullptr;
    else
        return i->second;
  }

  bool setKnob(const string & name, const Knob & k){
    map<string,Knob*>::iterator ii = knob_map.find(name);
    if (ii == knob_map.end()){
      knob_map[name] = k.clone();
      knob_map[name]->setName(prefix+name);
      return true;
    } else {
      return false;
    }
  }
  bool addKnob(const string & name,Knob * k){
    map<string,Knob*>::iterator ii = knob_map.find(name);
    if (ii == knob_map.end()){
      knob_map[name] = k;
      knob_map[name]->setName(prefix+name);
      return true;
    } else {
      delete ii->second;
      ii->second = k;
      ii->second->setName(prefix+name);
      return true;
    }
  }
  bool getKnob(const string & name, Knob* & k){
    map<string,Knob*>::iterator ii = knob_map.find(name);
    if (ii == knob_map.end()){
      k = NULL;
      return false;
    } else {
      k = ii->second;
      return true;
    }
  }
  Knob * getKnob(const string & name){
    map<string,Knob*>::iterator ii = knob_map.find(name);
    Knob * k;
    if (ii == knob_map.end()){
      k = NULL;
    } else {
      k = ii->second;
    }
    return k;
  }
  bool replaceKnob(const string & name, const Knob & k){
    map<string,Knob*>::iterator ii = knob_map.find(name);
    if (ii == knob_map.end()){
      knob_map[name] = k.clone();
      knob_map[name]->setName(prefix+name);
      return false;
    } else {
      delete ii->second;
      ii->second = k.clone();
      ii->second->setName(prefix+name);
      return true;
    }
  }
  ~PKnobsHolder(){
    for (map<string,Knob*>::iterator it=knob_map.begin(); it!=knob_map.end(); ++it){
       delete it->second;
    }
  }

  static void clear() {
    for (map<string,PKnobsHolder *>::iterator it = knobholder_map.begin(); it != knobholder_map.end(); it++) {
        delete it->second;
    }
    knobholder_map.clear();
  }
  static void clear(std::string prefix) {
    auto it = knobholder_map.begin();
    while (it != knobholder_map.end()) {
      if (it->first.find(prefix) == 0) {
	delete it->second;
	it = knobholder_map.erase(it);
      } else {
	it++;
      }
    }
  }
};


#endif
