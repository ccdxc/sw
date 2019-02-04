#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <assert.h>
#include <map>
#include <vector>

#include "pknobs_reader.h"
#include "sknobs.h"
#include "LogMsg.h"

#define PKH_LOG 0 && cout
#define PKH_WARN cout << "WARNING: "
#define PKH_BLDKNOB(l) 0 && cout << std::string(l, '-')
map<string,PKnobsHolder *> PKnobsHolder::knobholder_map;

PKnobsHolder * PKnobsReader::access(const string &name)
{
    auto p = get_knobholder(name);
    if (p == nullptr) {
        auto reader = new PKnobsReader(name);
        reader->parseKnobFile(name);
        return reader;
    }
    return p;
}

bool PKnobsReader::knobExists(const string & name)
{
    auto k = findKnob(name);
    return (k != nullptr);
}

Knob * PKnobsReader::findKnob(const string &name)
{
    auto pos = name.find_last_of("/");
    string knob_key_path = "";
    string knob_key_name;
    bool found = true;
    if (pos == string::npos) {
        found = false;
    } else {
        knob_key_path = name.substr(0,pos+1);
        knob_key_name = name.substr(pos+1);
        auto p = access(knob_key_path);
        auto k = p->getKnob(knob_key_name);
        if (k != nullptr) {
            return k;
        } else {
            found = false;
        }
    }
    if (!found && knob_key_path != "") {
        // Try to look in to sknobs to see if there is a key with this
        // name.
        string real_knob_path;
        string real_knob_name;
        string knob_name;

        if (getSknobsString(name, knob_name)) {
            pos = knob_name.find_last_of("/");
            if (pos == string::npos) {
                real_knob_path = knob_key_path;
                real_knob_name = knob_name;
            } else {
                real_knob_path = knob_name.substr(0,pos+1);
                real_knob_name = knob_name.substr(pos+1);
            }
            auto p = access(real_knob_path);
            auto k = p->getKnob(real_knob_name);
            if (k != nullptr) {
                return k;
            } else {
                if (pos == string::npos) {
                   // perhaps it wasn't a relative path, but an absolute one
                   real_knob_path = "";
                   real_knob_name = knob_name;
                   auto p = access(real_knob_path);
                   auto k = p->getKnob(real_knob_name);
                   if (k != nullptr) {
                       return k;
                   } 
                }
            }
        }
    }
    return nullptr;
}


uint64_t  PKnobsReader::evalKnob(const string &name)
{
    Knob * k = findKnob(name);
    if (k) {
        return k->eval();
    } else {
	PLOG_ERR("Could not find knob: " << name << endl);
	return 0;
    }
}

void PKnobsReader::parseKnobFile(string const & path){

    PLOG_MSG("parseKnobFile with path:"<<path<<" for KnobDef"<<endl);

  vector<string> knob_def_vec;

  //
  // get the name of all knobs with the path defined
  //
  string parse_s              = path + "KnobDef";
  sknobs_iterator_p iterator  = sknobs_iterate((char *)parse_s.c_str());
  while(sknobs_iterator_next(iterator)) {
    char *t = sknobs_iterator_get_string(iterator);
    PKH_LOG << "\t found "<< t << endl;
    knob_def_vec.push_back(string(t));
  }
  //
  // now build each knob and put in the holder prefixed by path
  //
  for(vector<string>::iterator it=knob_def_vec.begin(); it != knob_def_vec.end();++it){
    Knob * k;
    PKH_LOG << "-->calling buildKnob with:"<<*it<<endl;
    if (buildKnob(path,*it,k,0)){
      PKH_LOG << "-->ending buildKnob with knob:" << *it << endl;
      addKnob(*it,k);
      assert(getKnob(*it,k));
      PKH_LOG << "added to PKnobHolder:"<<*k<<endl<<endl;
    } else {
      PLOG_ERR("KnobDef for "<< path << *it << " exists, but could not be read\n");
    }
  }
}

bool PKnobsReader::getSknobsValue(string const & key_s, pknobs::u64 & value){
	if (sknobs_exists((char*) key_s.c_str()) == 0){
            //PLOG_WARN ("Can't find "<<key_s<<" in sknobs database"<<endl);
            return false;
	} else {
		value = sknobs_get_value((char*) key_s.c_str(),0);
		return true;
	}
}

bool PKnobsReader::getSknobsString(string const & key_s, string & value_s){
	if (sknobs_exists((char*) key_s.c_str()) == 0){
            //PLOG_WARN ("Can't find "<<key_s<<" in sknobs database"<<endl);
            return false;
	} else {
		value_s = sknobs_get_string((char*) key_s.c_str(),(char*)"");
		return true;
	}
}

bool PKnobsReader::getKnobDefinition(string const & knob_name  // +knobDef=<value>
                                     ,string const & knob_def_s // +<.*/knob>=value
                                     ,string & knob_def_value_s){     // +.*/knob=<value>
	bool success = getSknobsString(knob_def_s,knob_def_value_s);
	if (knob_def_value_s == knob_name){
		success = false;
		PLOG_WARN ("self-referential knob:"<<knob_def_value_s<<endl);
	}
	return success;
}

bool PKnobsReader::buildKnob(string const & path
                            ,string const & knob_name
                            ,Knob * & k
                            ,int count) {

	int pos = count*8;
	count++;

	PKH_BLDKNOB(pos) << "<>BuildKnob path:"<<path<<"name: "<<knob_name<<endl;

	string knob_type_s    = path + knob_name + "/type";
	string knob_value_s   = path + knob_name + "/value";
	string knob_count_s   = path + knob_name + "/numKnobs";
	string knob_min_s     = path + knob_name + "/min";
	string knob_max_s     = path + knob_name + "/max";

	// get knobType +.*/type=<value>
	string knob_type;
	bool success = getSknobsString(knob_type_s,knob_type);
	if (success == false) return false;

	boost::algorithm::to_lower(knob_type);
	//
	// decode type value
	//
	if (knob_type.find("cknob") != string::npos){
		// get the value string: +.*/value=<value>
		pknobs::u64 knob_value;
		bool success = getSknobsValue(knob_value_s,knob_value);
		if (success){
			k = new CKnob(knob_name,knob_value);
			PKH_BLDKNOB(pos) << "<=>CKnob: "<<*k<<endl;
			return true;
		}
	}
	if (knob_type == "rknob"){
		k = new RKnob(knob_name);
		PKH_BLDKNOB(pos) << "<=>RKnob: "<<*k<<endl;
		return true;
	}
	if (knob_type == "rrknob"){
		bool success;
		pknobs::u64 knob_min, knob_max;
		success = getSknobsValue(knob_min_s,knob_min);
		success = getSknobsValue(knob_max_s,knob_max) && success;
		if (success) {
			k = new RRKnob(knob_name,knob_min,knob_max);
			PKH_BLDKNOB(pos) << "<=>RRKnob: "<<*k<<endl;
			return true;
		}
	}
    if (knob_type == "uniqknob") {
		bool success;
		pknobs::u64 knob_min, knob_max;
		success = getSknobsValue(knob_min_s,knob_min);
		success = getSknobsValue(knob_max_s,knob_max) && success;
		if (success) {
			k = new UniqKnob(knob_name,knob_min,knob_max);
			PKH_BLDKNOB(pos) << "<=>UniqKnob: "<<*k<<endl;
			return true;
		}
    }
	if (knob_type == "mrknob"){
		bool success;
		pknobs::u64 knob_min, knob_max;
		success = getSknobsValue(knob_min_s,knob_min);
		success = getSknobsValue(knob_max_s,knob_max) && success;
		// get KnobDef
		string knob_def_value_s;
		string knob_def_s = path + knob_name + "/chooserref";
		string type_def_s = knob_name + "/chooser";

		success = getKnobDefinition(knob_name,knob_def_s,knob_def_value_s);
		PKH_BLDKNOB(pos) <<"--chooserref: "<<knob_def_s<<" def:"<<knob_def_value_s<<endl;

		Knob * tmp;
		bool build_success = success && buildKnob(path,knob_def_value_s,tmp,count);
		if (build_success){
			k = new MRKnob(knob_name,knob_min,knob_max,tmp);
			tmp->setParent(k);
			PKH_BLDKNOB(pos) << "--defKnob:"<<*tmp << endl;
		} else {
			// didn't work so try "type"
			build_success = buildKnob(path,type_def_s,tmp,count);
			if (build_success == true){
				k = new MRKnob(knob_name,knob_min,knob_max,tmp);
				tmp->setParent(k);
				PKH_BLDKNOB(pos) << "--type:"<<*tmp << endl;
			}
		}
		if (build_success == false) return false;
		PKH_BLDKNOB(pos) << "<=>MRKnob: "<<*k<<endl;
		return true;
	}

	if (knob_type == "mlknob"){
            pknobs::u64 num_knobs;
            // get knob count
            bool success = getSknobsValue(knob_count_s.c_str(),num_knobs);
            if (success == false) return false;

            k = new MLKnob(knob_name);

            enum KNOB_SEARCH { KNOB_LIST, CHOOSER};
            for(int j=KNOB_LIST;j<=CHOOSER;j++){

                if (j==CHOOSER) {
                    num_knobs = 1; // only one knob for chooser
                }
                for(unsigned i=0;i<num_knobs;i++){

                    string knob_def_value_s;
                    string knob_def_s;
                    string type_def_s;

                    if (j==KNOB_LIST){
                        knob_def_s = path + knob_name + "/" + to_string(i) + "/knobref";
                        type_def_s = knob_name + "/" + to_string(i) + "/knob";
                    } else {
                        knob_def_s = path + knob_name + "/chooserref";
                        type_def_s = knob_name + "/chooser";
                    }
                    //
                    // find list of knobs
                    // extract KnobDef: hunt for +.*/knob=<value>
                    // knob_def_value_s will hold the value string
                    // if not successful, hunt for +.*/type=<value> and then recurse
                    //
                    success = getKnobDefinition(knob_name,knob_def_s,knob_def_value_s);
                    PKH_BLDKNOB(pos) <<"--knob: "<<knob_def_s<<" def:"<<knob_def_value_s<<endl;
                    Knob * tmp;
                    // try "knob"
                    bool build_success = success && buildKnob(path,knob_def_value_s,tmp,count);
                    if (build_success == true){
                        tmp->setParent(k);
                        PKH_BLDKNOB(pos) <<  "--defKnob:"<<*tmp << endl;
                        if (j==KNOB_LIST){
                            ((MLKnob*) k)->addValueKnob(tmp);
                        } else {
                            assert(num_knobs == 1);
                            assert(j==CHOOSER);
                            ((MLKnob*) k)->addChooserKnob(tmp);
                        }
                    } else {
                        // didn't work so try "type"
                        build_success = buildKnob(path,type_def_s,tmp,count);
                        if (build_success == true){
                            tmp->setParent(k);
                            PKH_BLDKNOB(pos) << "--type:"<<*tmp << endl;
                            if (j==KNOB_LIST){
                                ((MLKnob*) k)->addValueKnob(tmp);
                            } else {
                                assert(num_knobs == 1);
                                assert(j==CHOOSER);
                                ((MLKnob*) k)->addChooserKnob(tmp);
                            }
					}
                    }
                    if (build_success == false){
                        delete k;
                        return false;
                    }
                }
            }
            PKH_BLDKNOB(pos) << "<=>MLKnob: "<<*k <<endl;
            return true;
	}

	if (knob_type == "blknob"){
            pknobs::u64 num_knobs;
            // get knob count
            bool success = getSknobsValue(knob_count_s.c_str(),num_knobs);
            if (success == false) return false;

            k = new BLKnob(knob_name);

            enum KNOB_SEARCH { KNOB_LIST, CHOOSER};
            for(int j=KNOB_LIST;j<=CHOOSER;j++){

                for(unsigned i=0;i<num_knobs;i++){

                    string knob_def_value_s;
                    string knob_def_s;
                    string type_def_s;

                    if (j==KNOB_LIST){
                        knob_def_s = path+ knob_name + "/" + to_string(i) + "/knobref";
                        type_def_s = knob_name + "/" + to_string(i) + "/knob";

                    } else {
                        knob_def_s = path + knob_name + "/" + to_string(i) + "/chooserref";
                        type_def_s = knob_name + "/" + to_string(i) + "/chooser";
                    }


                    //
                    // find list of knobs
                    // extract KnobDef: hunt for +.*/knob=<value>
                    // knob_def_value_s will hold the value string
                    // if not successful, hunt for +.*/type=<value> and then recurse
                    //
                    success = getKnobDefinition(knob_name,knob_def_s,knob_def_value_s);
                    PKH_BLDKNOB(pos) <<"--knob: "<<knob_def_s<<" def:"<<knob_def_value_s<<endl;
                    Knob * tmp;
                    // try "knob"
                    bool build_success = success && buildKnob(path,knob_def_value_s,tmp,count);
                    if (build_success == true){
                        tmp->setParent(k);
                        PKH_BLDKNOB(pos) <<  "--defKnob:"<<*tmp << endl;
                        if (j==KNOB_LIST){
                            ((BLKnob*) k)->addBurstKnob(tmp);
                        } else {
                            assert(j==CHOOSER);
                            ((BLKnob*) k)->addBurstCntKnob(tmp);
                        }
                    } else {
                        // didn't work so try "type"
                        build_success = buildKnob(path,type_def_s,tmp,count);
                        if (build_success == true){
                            tmp->setParent(k);
                            PKH_BLDKNOB(pos) << "--type:"<<*tmp << endl;
                            if (j==KNOB_LIST){
                                ((BLKnob*) k)->addBurstKnob(tmp);
                            } else {
                                assert(j==CHOOSER);
                                ((BLKnob*) k)->addBurstCntKnob(tmp);
                            }
                        }
                    }
                    if (build_success == false){
                        delete k;
                        return false;
                    }
                }
            }
            pknobs::u64 val = 0;
            getSknobsValue(path+knob_name+"/stick", val);
            if (val) {
                ((BLKnob*) k)->setStick();
            }
            val = 0;
            getSknobsValue(path+knob_name+"/permute", val);
            if (val) {
                ((BLKnob*) k)->setPermute();
            }

            PKH_BLDKNOB(pos) << "<=>BLKnob: "<<*k <<endl;
            return true;
	}

	// +KnobDef=thirdKnob
	// +thirdKnob/type=WLKnob
	// +thirdKnob/numKnobs=2
	// +thirdKnob/0/knob=firstKnob
	// +thirdKnob/1/knob=secondKnob
	// +thirdKnob/[01]/weight=10
	if (knob_type == "wlknob"){

            pknobs::u64 num_knobs;
            bool success = getSknobsValue(knob_count_s.c_str(),num_knobs);
            if (success == false) return false;

            k = new WLKnob(knob_name);
            for(unsigned i=0;i<num_knobs;i++){

                string knob_def_value_s;
                string knob_def_s = path + knob_name + "/" + to_string(i) + "/knobref";
                string type_def_s = knob_name + "/" + to_string(i) + "/knob";
                //
                // find list of knobs
                // extract KnobDef: hunt for +.*/knob=<value>
                // knob_def_value_s will hold the value string
                // if not successful, hunt for +.*/type=<value> and then recurse
                //
                success = getKnobDefinition(knob_name,knob_def_s,knob_def_value_s);
                PKH_BLDKNOB(pos) <<"--knob: "<<knob_def_s<<" def:"<<knob_def_value_s<<endl;

                Knob * tmp;
                // try knob
                bool build_success = success && buildKnob(path,knob_def_value_s,tmp,count);
                if (build_success == true){
                    tmp->setParent(k);
                    PKH_BLDKNOB(pos) <<  "--defKnob:"<<*tmp << endl;
                    ((WLKnob*) k)->addValueKnob(tmp);
                } else {
                    // didn't work so try "type"
                    build_success = buildKnob(path,type_def_s,tmp,count);
                    if (build_success == true){
                        tmp->setParent(k);
                        PKH_BLDKNOB(pos) << "--type:"<<*tmp << endl;
                        ((WLKnob*) k)->addValueKnob(tmp);
                    }
                }
                if (build_success == false){
                    delete k;
                    return false;
                }
                // now get weight
                string next_weight_s= path + knob_name + "/" + to_string(i) + "/weight";
                pknobs::u64 weight_value;
                success = getSknobsValue(next_weight_s,weight_value);
                PKH_BLDKNOB(pos) << "--weight for "<<next_weight_s<<" :"<<weight_value<<endl;
                ((WLKnob*) k)->addWeight(weight_value);
                if (success == false) {
                    delete k;
                    return false;
                }
            }
            PKH_BLDKNOB(pos) << "<=> WLKnob"<<*k <<endl;
            return true;
	}

	if (knob_type == "plknob"){

            pknobs::u64 num_knobs;
            bool success = getSknobsValue(knob_count_s.c_str(),num_knobs);
            if (success == false) return false;

            k = new PLKnob(knob_name);
            for(unsigned i=0;i<num_knobs;i++){

                string knob_def_value_s;
                string knob_def_s = path + knob_name + "/" + to_string(i) + "/knobref";
                string type_def_s = knob_name + "/" + to_string(i) + "/knob";
                //
                // find list of knobs
                // extract KnobDef: hunt for +.*/knob=<value>
                // knob_def_value_s will hold the value string
                // if not successful, hunt for +.*/type=<value> and then recurse
                //
                success = getKnobDefinition(knob_name,knob_def_s,knob_def_value_s);
                PKH_BLDKNOB(pos) <<"--knob: "<<knob_def_s<<" def:"<<knob_def_value_s<<endl;

                Knob * tmp;
                // try knob
                bool build_success = success && buildKnob(path,knob_def_value_s,tmp,count);
                if (build_success == true){
                    tmp->setParent(k);
                    PKH_BLDKNOB(pos) <<  "--defKnob:"<<*tmp << endl;
                    ((PLKnob*) k)->addValueKnob(tmp);
                } else {
                    // didn't work so try "type"
                    build_success = buildKnob(path,type_def_s,tmp,count);
                    if (build_success == true){
                        tmp->setParent(k);
                        PKH_BLDKNOB(pos) << "--type:"<<*tmp << endl;
                        ((PLKnob*) k)->addValueKnob(tmp);
                    }
                }
                if (build_success == false){
                    delete k;
                    return false;
                }
            }
            PKH_BLDKNOB(pos) << "<=> PLKnob"<<*k <<endl;
            return true;
	}

	return false;
}
