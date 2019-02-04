#include "pknobs.h"


namespace pknobs {

pknobs::u64 Knob::seed = 2;
int Knob::knob_cnt = 0;


std::ostream& operator<< (std::ostream& os, const pknobs::Knob& knob){
	knob.print(os);
	return os;
}

void Knob::print (std::ostream& os) const {
	os << "name:"<<name<<" this:"<< this<<" parent:"<<parent;
}

void CKnob::print (std::ostream& os) const {
	os << "CKnob->"; Knob::print(os); os << " value:"<<value;
}

unsigned long RKnob::hash(const char *str) {
	unsigned long hash = 5381;
	int c;
	while (*str) {
		c = (int)*str++;
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	return hash;
}

boost::random::mt19937_64* RKnob::getRandGen(){
	if (parent == NULL) {
		p_mtrand = new boost::random::mt19937_64;
		my_seed  = Knob::seed+hash(getName().c_str());
		p_mtrand->seed(my_seed);
		parent   = this;
	}
	return p_mtrand;
}
void RKnob::print (std::ostream& os) const {
	os << "RKnob->"; Knob::print(os); os << " p_mtrand:"<<p_mtrand;
}


// Random Range Knob.  returns int64 number in specified range
void RRKnob::copy(const RRKnob & k){
	lower = k.lower;
	upper = k.upper;
	range = k.range;
}
void RRKnob::init(int64 _lower, int64 _upper){
	lower = _lower;
	upper = _upper;
	if (_lower > _upper) {
		lower = _upper;
		upper = _lower;
	}
	range = upper - lower + 1;
}

void RRKnob::print (std::ostream& os) const {
	os << "RRKnob->"; Knob::print(os); os << " min:"<<lower<<" max:"<<upper;
}


// returns a number from a specified range that is monotonically
// increasing/decreasing by step size amount

void MRKnob::init(int64 _lower, int64 _upper, Knob * _chooser){
	min = _lower;
	max = _upper;
	if (_lower > _upper) {
		min = _upper;
		max = _lower;
	}
	last    = min;
	range   = max - min + 1;
	chooser = _chooser;
	setParent(this);
}
void MRKnob::copy(const MRKnob & k){
	min     = k.min;
	max     = k.max;
	last    = k.last;
	range   = k.range;
	chooser = k.chooser->clone();
}
void MRKnob::print (std::ostream& os) const {
	os << "MRKnob->"; RKnob::print(os); os << endl
	                                       << string(8,' ') << " min:"<<min<<" max:"<<max<<" last:"<<last<<endl
	                                       << string(8,' ') << "chooser:"<<*chooser;
}



void MLKnob::copy(const MLKnob & k){
	for (vector<Knob*>::const_iterator it = k.knob_vec.begin(); it != k.knob_vec.end(); ++it) {
		knob_vec.push_back((*it)->clone());
	}
}
//const vector<Knob *> & getKnobVec(){return knob_vec;}
// define list of knobs in list + chooser.
// specify size based on list, not list+chooser
// example: list of 4 elements + rand chooser would have size of 4
MLKnob::MLKnob(const string & n, KnobParam knob_type, int num_knobs, ... )
	: MRKnob(n){

	va_list arg_list;
	va_start(arg_list, num_knobs);
	assert(num_knobs > 0);
	Knob* chooser;
	if (knob_type == KNOB_KNOB) {
		//elements are knobs
		//chooser is a knob
		for (int i=0; i<num_knobs; i++) {
			Knob* t = va_arg(arg_list,Knob*);
			knob_vec.push_back(t);
		}
		chooser = va_arg(arg_list,Knob*);
	}
	else if (knob_type == CONST32) {
		for (int i=0; i<num_knobs; i++) {
			int64 t =  va_arg(arg_list,int);
			knob_vec.push_back(new CKnob(t));
		}
		int64 t  =  va_arg(arg_list,int);
		chooser = new CKnob(t);
	}
	else if (knob_type == CONST64) {
		for (int i=0; i<num_knobs; i++) {
			int64 t =  va_arg(arg_list,int64);
			knob_vec.push_back(new CKnob(t));
		}
		int64 t  =  va_arg(arg_list,int);
		chooser = new CKnob(t);

	} else {
		assert(0);
	}
	MRKnob::init(0,num_knobs-1,chooser);
	setParent(this);
	va_end(arg_list);
}

MLKnob::MLKnob(KnobParam knob_type, int num_knobs, ... )
	: MRKnob(){

	va_list arg_list;
	va_start(arg_list, num_knobs);
	assert(num_knobs > 0);
	Knob* chooser;
	if (knob_type == KNOB_KNOB) {
		//elements are knobs
		//chooser is a knob
		for (int i=0; i<num_knobs; i++) {
			Knob* t = va_arg(arg_list,Knob*);
			knob_vec.push_back(t);
		}
		chooser = va_arg(arg_list,Knob*);
	}
	else if (knob_type == CONST32) {
		for (int i=0; i<num_knobs; i++) {
			int64 t =  va_arg(arg_list,int);
			knob_vec.push_back(new CKnob(t));
		}
		int64 t  =  va_arg(arg_list,int);
		chooser = new CKnob(t);
	}
	else if (knob_type == CONST64) {
		for (int i=0; i<num_knobs; i++) {
			int64 t =  va_arg(arg_list,int64);
			knob_vec.push_back(new CKnob(t));
		}
		int64 t  =  va_arg(arg_list,int);
		chooser = new CKnob(t);
	}
	else {
		assert(0);
	}
	MRKnob::init(0,num_knobs-1,chooser);
	setParent(this);
	va_end(arg_list);
}

void MLKnob::print (std::ostream& os) const {
	os << "MLKnob->"; RKnob::print(os); os << endl;
	for(unsigned int i=0; i<knob_vec.size(); i++) {
		os << string(8,' ') << "knob["<<i<<"]:"<<*knob_vec[i]<< endl;
	}
	os << string(8,' '); MRKnob::print(os);
}



void WLKnob::copy(const WLKnob & k){
	for (vector<Knob*>::const_iterator it = k.knob_vec.begin(); it != k.knob_vec.end(); ++it) {
		knob_vec.push_back((*it)->clone());
	}
	for (vector<int>::const_iterator it = k.wgt_vec.begin(); it != k.wgt_vec.end(); ++it) {
		wgt_vec.push_back(*it);
	}
	total_wgt = k.total_wgt;
}

WLKnob::WLKnob(const string & n, KnobParam knob_type, int num_knobs, ... )
	: RKnob(n){
	total_wgt = 0;
	va_list arg_list;
	va_start(arg_list, num_knobs);
	assert(num_knobs > 0);
	//elements are knobs or CONST
	//chooser is a knob
	for (int i=0; i<num_knobs; i++) {
		if (knob_type == KNOB_CONST32) {
			Knob* t = va_arg(arg_list,Knob*);
			knob_vec.push_back(t);
		} else if (knob_type == CONST32) {
			int64 t =  va_arg(arg_list,int);
			knob_vec.push_back(new CKnob(t));
		} else if (knob_type == CONST64) { // CONST64
			int64 t =  va_arg(arg_list,int64);
			knob_vec.push_back(new CKnob(t));
		} else {
			assert(0);
		}
		int u = va_arg(arg_list,int);
		if (i==0) wgt_vec.push_back(u);
		else wgt_vec.push_back(u + wgt_vec[i-1]);
		total_wgt += u;
	}
	setParent(this);
	va_end(arg_list);
}

void WLKnob::print (std::ostream& os) const {
	os << "WLKnob->"; RKnob::print(os); os << endl;
	for(unsigned int i=0; i<knob_vec.size(); i++) {
		os << string(8,' ') << "knob["<<i<<"]:"<<*knob_vec[i]<< endl;
	}
	for(unsigned int i=0; i<wgt_vec.size(); i++) {
		os << string(8,' ') << "wgt["<<i<<"]:"<<wgt_vec[i]<< endl;
	}
	os << "total_wgt:"<<total_wgt<<" ";
}


void BLKnob::copy(const BLKnob & k){
	for (vector<Knob*>::const_iterator it = k.burst_knob.begin(); it != k.burst_knob.end(); ++it) {
		burst_knob.push_back((*it)->clone());
	}
	for (vector<Knob*>::const_iterator it = k.burst_cnt_knob.begin(); it != k.burst_cnt_knob.end(); ++it) {
		burst_cnt_knob.push_back((*it)->clone());
	}
	for (vector<int>::const_iterator it = k.curr_burst_size.begin(); it != k.curr_burst_size.end(); ++it) {
		curr_burst_size.push_back((*it));
	}
	for (vector<int>::const_iterator it = k.curr_burst_cnt.begin(); it != k.curr_burst_cnt.end(); ++it) {
		curr_burst_cnt.push_back((*it));
	}
	curr_idx  = k.curr_idx;
	stick     = k.stick;
	last_value= k.last_value;
}

BLKnob::BLKnob(const string & n, KnobParam knob_type, int num_knobs, ... )
	: RKnob(n){
	curr_idx        = 0;
	stick           = false;
	last_value      = 0;
	va_list arg_list;
	va_start(arg_list, num_knobs);
	assert(num_knobs > 0);

	//elements are knobs or CONST
	//chooser is a knob
	for (int i=0; i<num_knobs; i++) {

		curr_burst_cnt.push_back(0);
		curr_burst_size.push_back(0);

		if (knob_type == KNOB_CONST32) {
			Knob* t = va_arg(arg_list,Knob*);
			burst_knob.push_back(t);
			int u = va_arg(arg_list,int);
			burst_cnt_knob.push_back(new CKnob(u));
		} else if (knob_type == KNOB_KNOB) {
			Knob* t = va_arg(arg_list,Knob*);
			burst_knob.push_back(t);
			t = va_arg(arg_list,Knob*);
			burst_cnt_knob.push_back(t);
		} else {
			assert(0);
		}
	}
	setParent(this);
	va_end(arg_list);
}

void BLKnob::print (std::ostream& os) const {
	os << "BLKnob->"; RKnob::print(os); os << endl;
	for(unsigned int i=0; i<burst_knob.size(); i++) {
		os << string(8,' ') << "burst["<<i<<"]:"<<*burst_knob[i]<< endl;
	}
	for(unsigned int i=0; i<burst_cnt_knob.size(); i++) {
		os << string(8,' ') << "burst_cnt["<<i<<"]:"<<*burst_cnt_knob[i]<< endl;
	}
	os << string(8,' ') << "stick:"<<stick<<" ";
}


// An array of knobs that is shuffled and then cycled through.
// When all knobs have been evaluated in the array, a cycle is completed the list
// is re-shuffled and a new cycle begins
//

void PLKnob::copy(const PLKnob & k){
	for (vector<Knob*>::const_iterator it = k.knob_vec.begin(); it != k.knob_vec.end(); ++it) {
		knob_vec.push_back((*it)->clone());
	}
	curr_idx = k.curr_idx;
}

PLKnob::PLKnob(const string & n, KnobParam knob_type, int num_knobs, ... )
	: RKnob(n){
	curr_idx        = 0;
	va_list arg_list;
	va_start(arg_list, num_knobs);
	assert(num_knobs > 0);
	//elements are knobs or CONST
	//chooser is a knob
	for (int i=0; i<num_knobs; i++) {
		if (knob_type == KNOB) {

			Knob* t = va_arg(arg_list,Knob*);
			knob_vec.push_back(t);

		} else if (knob_type == CONST32) {

			int u = va_arg(arg_list,int);
			knob_vec.push_back(new CKnob(u));

		} else {

			assert(knob_type == CONST64);
			int64 t =  va_arg(arg_list,int64);
			knob_vec.push_back(new CKnob(t));

		}
	}

	va_end(arg_list);

}
void PLKnob::print (std::ostream& os) const {
	os << "PLKnob->"; RKnob::print(os); os << endl;
	for(unsigned int i=0; i<knob_vec.size(); i++) {
		os << string(8,' ') << "knob["<<i<<"]:"<<*knob_vec[i]<< endl;
	}
}

}


