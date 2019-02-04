#ifndef _PKNOBS_READER_H
#define _PKNOBS_READER_H

#include <stdarg.h>
#include <map>

#include "pknobs_holder.h"
#include "pknobs.h"

using namespace boost;
using namespace std;
using namespace pknobs;

//
// reads sknobs database and returns the knobs
//
class PKnobsReader : public PKnobsHolder {
private:
  static bool getSknobsValue(string const & key_s, pknobs::u64 & value);
  static bool getSknobsString(string const & key_s, string & value_s);
  bool getKnobDefinition(string const & knob_name
  											,string const & knob_def_s
  											,string & knob_def_value_s);
  bool buildKnob(string const & path, string const & knob_name, Knob * & k, int count);
public:
  PKnobsReader() : PKnobsHolder () {};
  PKnobsReader(const string & n) : PKnobsHolder(n) {}

  void parseKnobFile(string const & path);
  static PKnobsHolder * access(const string & name);
  static uint64_t evalKnob(const string & name);
  static bool knobExists(const string & name);
  static Knob * findKnob(const string & name);
};

#endif
