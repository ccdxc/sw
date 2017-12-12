#ifndef _PKNOBS_H
#define _PKNOBS_H

#include <boost/random.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <iomanip>
#include <assert.h>
#include <stdarg.h>


using namespace boost;
using namespace std;

//TODO: MakeCopy which returns a pointer to a knob which is itself
//TODO: make sure copy ctor is implemented for each knob
//TODO: run valgrind on main.cc
//TODO: implement ostream (ask why the "1" string is showing up)

namespace pknobs{

  typedef unsigned long long u64;
  typedef long long int64;

  // KNOB_KNOB : all params are knobs:
  //    BLKnob <value,burst size> pairs
  //    MLKnob <value list,chooser>
  // KNOB_CONST32: all values are knobs, all choosers or weights are constants
  //    WLKnob <value,weight> pairs : value is KNOB, weight is CONST32
  //    BLKnob <value,burst size> pairs : value is KNOB, burst_size is CONST32
  // CONST32|CONST64: all values are constancs
  //    WLKnob <value,weight> pairs
  //    BLKnob <value,burst size> pairs
  //    MLKnob <value list,chooser>
  //    PLKnob <value list>
  // KNOB: all values are knobs
  //    PLKnob <knob list>
  enum KnobParam {
    KNOB_KNOB,KNOB,KNOB_CONST32,CONST32,CONST64
  };

  // knobs are made up of random objects and then containers of random objects
  // that then comprise a random objects
  // objects:
  //  - cknob : constant knob that always returns the same value
  //  - rknob : random knob that returns a 64-bit integer (long long)
  //  - rrknob: random knob that returns a 64-bit integer (long long) bounded
  //  by a min and max value, known as a range.
  // containers:
  //  - mrknob: knob that generates a monotonically increasing/decreasing number
  //  bounded by a the range.  The step up/down
  //  function can be any knob object.  It is a container because it contains
  //  a step function knob object.
  //  - mlknob: knob that evaluates an array of return value knobs whose chooser function is
  //  an mrknob.  It is a container because it holds an array of return value knobs
  //  - wlknob: knob that evaluates an array of return value knobs whose chooser is a
  //  weighted random number.  list<pair<knob value,weight>>
  //  - blknob: It is a container because it holds two arrays of knobs.  The
  // first array holds the list of return value knobs.  The second knob holds an array
  // of burst count value knobs.  list<pair<knob value,knob burst cnt>>
  //  -plknob: It is a container because it holds an array of return value knobs.
  // Each knob is evaluated once in the list.  After the list is exhausted, the
  // list is shuffeled and a new cycle begins.  Each cyle returns a permutation of
  // the knob order.


  // Abstract base-class Knob that holds a name string and a parent pointer
  // The parent pointer is used by all child knobs that are contained in a
  // knob.  All contained knobs (i.e. children) will re-use the same MTRAND
  // generator
  // virtual function eval() will return a signed long long number
  class Knob {
    string      name;
  protected:
    static u64  seed;
    static int  knob_cnt;
    Knob *      parent;
  private:
   void copy(const Knob & k){
     name   = k.name + "_copy";
     parent = k.parent;
   }
 public:
   static void set_seed(u64 s) { seed = s; }
   const string & getName() { return name;}
   void setName(const string & n){name = n;}
   static void setSeed(const u64 _seed){seed = _seed;}
   static u64 getSeed(){return seed;}
   Knob() : name("unknown"+boost::lexical_cast<std::string>(knob_cnt)),parent(NULL){
     knob_cnt++;
   };
   Knob(const string & n):name(n),parent(NULL){knob_cnt++;};
   virtual ~Knob(){};
   Knob(const Knob & k){ copy(k);}
   //modifiers
   virtual int64 eval() = 0;
   virtual void setParent(Knob * p) = 0;
   virtual Knob* clone() const = 0;
   virtual void print (std::ostream& os) const;
  };

  std::ostream& operator<< (std::ostream& os, const Knob& knob);

  class CKnob : public Knob {
  private:
   int64 value;
   void copy(const CKnob & k){
     parent = NULL;
     value  = k.value;
   }
  public:
    CKnob(int64 _v):Knob(),value(_v){}
    CKnob(const string & n):Knob(n),value(0){}
    CKnob(const string & n, int64 _v):Knob(n),value(_v){}
    CKnob(const CKnob & k) : Knob(k){ copy(k);}
    virtual int64 eval() { return value;}
    virtual void setParent(Knob *p){if (p!=this) parent = p;}
    virtual Knob* clone() const { return new CKnob(*this);}
    virtual void print (std::ostream& os) const;

  };


  // Random Knob.  returns int64 number.  Uses the boost::random::mt19937_64
  class RKnob : public Knob{
  private:
    u64 my_seed;
    boost::random::mt19937_64* p_mtrand;
    unsigned long hash(const char *str);
    void copy(const RKnob & k){
      parent    = NULL;
      p_mtrand  = NULL;
    }
  public:
    RKnob():Knob(),my_seed(0),p_mtrand(NULL){};
    RKnob(const string & n):Knob(n),my_seed(0),p_mtrand(NULL){}
    RKnob(const RKnob & k) : Knob(k){ copy(k);}
    virtual ~RKnob(){
      if (parent == this)
        delete p_mtrand;
    }
    virtual boost::random::mt19937_64* getRandGen();
    virtual int64 eval() {
      if (parent == NULL) return (*(getRandGen()))();
      return (int64) (*(static_cast<RKnob*>(parent))->getRandGen())();
    }
    virtual Knob* clone() const{ return new RKnob(*this);}
    virtual void setParent(Knob *p){if (p!=NULL && p!=this) parent = p;}
    virtual void print (std::ostream& os) const;
  };

  // Random Range Knob.  returns int64 number in specified range
  class RRKnob : public RKnob{
  private:
    int64 lower;
    int64 upper;
    int64 range;
    void copy(const RRKnob & k);
  protected:
    void init(int64 _lower, int64 _upper);
    RRKnob(const string & n) : RKnob(n){};
  public:
    RRKnob(int64 _lower, int64 _upper) : RKnob() {init(_lower,_upper);}
    RRKnob(const string & n, int64 _lower, int64 _upper):RKnob(n){
      init(_lower,_upper);
    }
    RRKnob(const RRKnob & k):RKnob(k){ copy(k);}
    virtual int64 eval(){
      u64 val = RKnob::eval();
      return(lower + (val % range));
    }
    virtual Knob* clone() const{ return new RRKnob(*this);}
    virtual void print (std::ostream& os) const;
    virtual int64 get_max_val() const { return upper; }
    virtual int64 get_min_val() const { return lower; }
  };


  // returns a number from a specified range that is monotonically
  // increasing/decreasing by step size amount
  class MRKnob : public RKnob {
  private:
    int64  min;
    int64  max;
    int64  last;
    int64  range;
    Knob* chooser;
  protected:
    void init(int64 _lower, int64 _upper, Knob * _chooser);
    void copy(const MRKnob & k);
  public:
    MRKnob() : RKnob(), min(0), max(0),last(0),range(0),chooser(NULL) {};
    MRKnob(const string & n) : RKnob(n), min(0), max(0),last(0),range(0),chooser(NULL) {};
    MRKnob(int64 _lower, int64 _upper, Knob * _chooser):RKnob(){
      init(_lower,_upper,_chooser);
    }
    MRKnob(int64 _lower, int64 _upper, int64 _step):RKnob(){
      init(_lower,_upper,new CKnob(_step));
    }
    MRKnob(const string & n, int64 _lower, int64 _upper, Knob * _chooser):RKnob(n){
      init(_lower,_upper,_chooser);
    }
    MRKnob(const MRKnob & k):RKnob(k){ copy(k);}
    ~MRKnob () {
      if (chooser != NULL){
        delete chooser;
      }
    }

    virtual Knob* clone() const { return new MRKnob(*this);}

    virtual void setParent(Knob * p){
        chooser->setParent(p);
    }
    virtual int64 eval() {
      int64 ret_value;
      int64 step_sz = (*chooser).eval()%range;
      if (step_sz > 0){
        ret_value    = last;
        if (last + step_sz > max){
          last = (last + step_sz) - range;
        } else {
          last = (last + step_sz);
        }
      } else {
        if ((last + step_sz) < min){
          last = last + step_sz + (range);
        } else{
          last = last + step_sz;
        }
        ret_value = last;
      }
      return ret_value;
    }
    virtual void addChooserKnob(int64 _lower, int64 _upper,Knob *k){
      init(_lower,_upper,k);
    }
    virtual void print (std::ostream& os) const;

  };

  // evaluates a knob from an array whose index is
  // monotonically increasing/decreasing
  class MLKnob : public MRKnob {
  private:
    vector<Knob *>  knob_vec;

    void copy(const MLKnob & k);
  public:
    //const vector<Knob *> & getKnobVec(){return knob_vec;}
    // define list of knobs in list + chooser.
    // specify size based on list, not list+chooser
    // example: list of 4 elements + rand chooser would have size of 4
    MLKnob(const string & n, KnobParam knob_type, int num_knobs, ... );
    MLKnob(KnobParam knob_type, int num_knobs, ... );
    MLKnob(const string & n) : MRKnob(n) {}
    MLKnob(const MLKnob & k) : MRKnob(k) {copy(k);}
    virtual ~MLKnob(){
      for (vector<Knob*>::iterator it = knob_vec.begin() ; it != knob_vec.end(); ++it){
        delete *it;
      }
    }
    virtual Knob* clone() const{ return new MLKnob(*this);}
    virtual void setParent(Knob * p){
      MRKnob::setParent(p);
      for (vector<Knob*>::iterator it = knob_vec.begin() ; it != knob_vec.end(); ++it){
        (*it)->setParent(p);
      }
    }
    virtual int64 eval() {
      int64 index = MRKnob::eval();
      assert(index >= 0);
      int64 ret_value = (*knob_vec[index]).eval();
      return ret_value;
    }
   void addValueKnob (Knob * k){ knob_vec.push_back(k); }
   using MRKnob::addChooserKnob;
   void addChooserKnob(Knob* k){MRKnob::init(0,knob_vec.size()-1,k);};
   virtual void print (std::ostream& os) const ;
  };

  // weighted list of knobs.  Knob selection is evaluated based on ratio of weights
  // provided
  class WLKnob : public RKnob {
  protected:
    vector<Knob *>  knob_vec;
    vector<int>     wgt_vec;
    int             total_wgt;

    void copy(const WLKnob & k);
  public:
    // define list of knobs in list + chooser.
    // specify size based on list, not list+chooser
    // example: list of 4 elements + rand chooser would have size of 4
    WLKnob(const string & n, KnobParam knob_type, int num_knobs, ... );
    WLKnob(const string & n) : RKnob(n) { total_wgt = 0;}
    WLKnob(const WLKnob & k) : RKnob(k) {copy(k);}
    ~WLKnob(){
      for (vector<Knob*>::iterator it = knob_vec.begin() ; it != knob_vec.end(); ++it){
        delete *it;
      }
    }
    virtual Knob* clone() const{ return new WLKnob(*this);}
    virtual void setParent(Knob * p){
      for (vector<Knob*>::iterator it = knob_vec.begin() ; it != knob_vec.end(); ++it){
        (*it)->setParent(p);
      }
    }
    virtual int64 eval() {
      int64 prob = ((u64) RKnob::eval()) % total_wgt;
      for(unsigned int i=0;i<knob_vec.size();i++){
        if (prob < wgt_vec[i]){
          int64 ret_value = (*knob_vec[i]).eval();
          return ret_value;
        }
      }
      assert(0);
    }
    void addValueKnob  (Knob * k){ knob_vec.push_back(k); }
    void addWeight     (int i){
      if (wgt_vec.size() == 0){
        wgt_vec.push_back(i);
        assert(total_wgt == 0);
      }
      else
        wgt_vec.push_back(i+wgt_vec.back());
      total_wgt += i;
    }
    virtual void print (std::ostream& os) const;
  };

  // An array of <knob,burst> pairs.
  // knob is evaluated burst value times and then move to the next pairs
  // a cycle is complete when all pairs have burst
  // setting the stick knob will cause the knob to be evaluated once and
  // it will be used for the burst count.  Once a full cycle is completed,
  // the knob will evaluate again.
  class BLKnob : public RKnob{
  private:
    vector<Knob*> burst_cnt_knob;
    vector<int>   curr_burst_cnt;
    vector<int>   curr_burst_size;
    vector<Knob*> burst_knob;
    int curr_idx;
    bool stick;
    bool permute = false;  // Permute list at the beginning and after every iteration.
    bool permuted = false;
    int64 last_value;

    void shuffle(){
      int rng = burst_cnt_knob.size();
      for(int i=0;i<rng;i++){
        unsigned long rand_val = RKnob::eval();
        int swap_idx = i + (rand_val % (rng - i));;

        Knob* tmp                = burst_knob[i];
        burst_knob[i]            = burst_knob[swap_idx];
        burst_knob[swap_idx]     = tmp;

        tmp                      = burst_cnt_knob[i];
        burst_cnt_knob[i]        = burst_cnt_knob[swap_idx];
        burst_cnt_knob[swap_idx] = tmp;

	int   tmp_cnt            = curr_burst_cnt[i];
	curr_burst_cnt[i]        = curr_burst_cnt[swap_idx];
	curr_burst_cnt[swap_idx] = tmp_cnt;

	curr_burst_size[swap_idx]= tmp_cnt;
      }
    }

    void copy(const BLKnob & k);
  public:
    void setStick()   { stick = true;}
    void unSetStick() { stick = false;}
    void setPermute()   { permute = true;}
    void unSetPermute() { permute = false;}
    BLKnob(const string & n, KnobParam knob_type, int num_knobs, ... );
    BLKnob(const string & n):RKnob(n),curr_idx(0),stick(false),last_value(0){};
    BLKnob(const BLKnob & k): RKnob(k){copy(k);}
    virtual ~BLKnob(){
      for (vector<Knob*>::iterator it = burst_knob.begin() ; it != burst_knob.end(); ++it){
        delete *it;
      }
      for (vector<Knob*>::iterator it = burst_cnt_knob.begin() ; it != burst_cnt_knob.end(); ++it){
        delete *it;
      }
    }
    virtual Knob* clone() const{ return new BLKnob(*this);}
    virtual void setParent(Knob * p){
      for (vector<Knob*>::iterator it = burst_knob.begin() ; it != burst_knob.end(); ++it){
        (*it)->setParent(p);
      }
      for (vector<Knob*>::iterator it = burst_cnt_knob.begin() ; it != burst_cnt_knob.end(); ++it){
        (*it)->setParent(p);
      }
    }
    virtual int64 eval(){
      if (!permuted && permute) {
	shuffle();
	permuted = true;
      }
      // if just starting, get the burst size
      if (curr_burst_cnt[curr_idx] == 0){
        curr_burst_size[curr_idx] = (*burst_cnt_knob[curr_idx]).eval();
      }

      int64 ret_value;
      // if stick is true, stick to first evaluation until cycle is over
      if (stick){
        if (curr_burst_cnt[curr_idx] == 0){
          ret_value = (*burst_knob[curr_idx]).eval();
          last_value = ret_value;
        } else {
          ret_value = last_value;
        }
      } else {
        ret_value = (*burst_knob[curr_idx]).eval();
      }

      curr_burst_cnt[curr_idx]++;

      //if cycle is over, reset count
      if (curr_burst_cnt[curr_idx] == curr_burst_size[curr_idx]){
        curr_burst_cnt[curr_idx] = 0;
        curr_idx = (curr_idx + 1);
	if (unsigned(curr_idx) >= curr_burst_cnt.size()) {
	  if (permute)
	    shuffle();
	  curr_idx -= curr_burst_cnt.size();
	}
      }

      return ret_value;

    }
    void addBurstKnob(Knob * k){
      burst_knob.push_back(k);
      curr_burst_cnt.push_back(0);
      curr_burst_size.push_back(0);
    }
    void addBurstCntKnob(Knob *k){burst_cnt_knob.push_back(k);}
    virtual void print (std::ostream& os) const ;
  };

  // An array of knobs that is shuffled and then cycled through.
  // When all knobs have been evaluated in the array, a cycle is completed the list
  // is re-shuffled and a new cycle begins
  //
  class PLKnob : public RKnob{
  private:

    vector<Knob*> knob_vec;
    unsigned int curr_idx;

    void shuffle(){
      int rng = knob_vec.size();
      for(int i=0;i<rng;i++){
        unsigned long rand_val = RKnob::eval();
        int swap_idx = i + (rand_val % (rng - i));;

        Knob* tmp          = knob_vec[i];
        knob_vec[i]        = knob_vec[swap_idx];
        knob_vec[swap_idx] = tmp;
      }
    }

    void copy(const PLKnob & k);
  public:
    PLKnob(const string & n, KnobParam knob_type, int num_knobs, ... );
    PLKnob(const string & n) : RKnob(n),curr_idx(0){}
    PLKnob(const PLKnob & k) : RKnob(k){copy(k);}
    ~PLKnob(){
      for (vector<Knob*>::iterator it = knob_vec.begin() ; it != knob_vec.end(); ++it){
        delete *it;
      }
    }
    virtual Knob* clone() const{ return new PLKnob(*this);}

    virtual void setParent(Knob * p){
      for (vector<Knob*>::iterator it = knob_vec.begin() ; it != knob_vec.end(); ++it){
        (*it)->setParent(p);
      }
    }
    virtual int64 eval(){
      if (curr_idx == 0) shuffle();
      int64 ret_value = (*knob_vec[curr_idx]).eval();
      curr_idx++;
      //if cycle is over, reset count
      if (curr_idx == knob_vec.size()){
        curr_idx = 0;
        //shuffle();
      }

      return ret_value;

    }
    void addValueKnob (Knob * k){ knob_vec.push_back(k); }
    virtual void print (std::ostream& os) const ;
  };


  class UniqKnob : public WLKnob {
      protected:

          int64 lower;
          int64 upper;
          int64 itrNo;
      public:
          void copy(const UniqKnob & k){
              WLKnob::copy(k);
              lower = k.lower;
              upper = k.upper;
          }

          void init(int64 _lower, int64 _upper) {
              lower = _lower;
              upper = _upper;
              itrNo =0;
              string knobStr = getName() + "_RRKnob_" + to_string(itrNo) + "_" + to_string(lower) + "_" + to_string(upper);

              knob_vec.push_back( new pknobs::RRKnob( knobStr, lower,upper));
              int u = upper - lower + 1;
              wgt_vec.push_back(u);
              total_wgt = u;
              setParent(this);
          }

          UniqKnob(const string &n, int64 _lower, int64 _upper) : WLKnob("foo"){
              init(_lower,_upper);
          }


          UniqKnob(int64 _lower, int64 _upper) : WLKnob("foo"){
              init(_lower, _upper);
          }

          void print (std::ostream& os) const {
              os << "UniqKnob->"; WLKnob::print(os); os << endl;
          }

          UniqKnob(const string & n) : WLKnob(n) {
              total_wgt = 0;
              lower = 0;
              upper = 0;
          }
          UniqKnob(const UniqKnob & k) : WLKnob(k) {copy(k);}
          virtual ~UniqKnob(){
          }
          virtual Knob* clone() const{ return new UniqKnob(*this);}
          virtual void setParent(Knob * p){
              WLKnob::setParent(p);
          }

          virtual int64 eval() {
              pknobs::int64 prob = ((pknobs::u64) WLKnob::eval()) % total_wgt;
              pknobs::RRKnob * new_min_rrknob = 0;
              pknobs::RRKnob * new_max_rrknob = 0;
              pknobs::int64 ret_value;
              int idx = -1;
              //cout << "total_wgt: " << total_wgt << " prob: " << prob << endl;
              for(unsigned int i=0;i<knob_vec.size();i++){
                  if (prob < wgt_vec[i]){

                      pknobs::RRKnob & old_rrknob = dynamic_cast<pknobs::RRKnob&>(*knob_vec[i]);
                      ret_value = old_rrknob.eval();
                      pknobs::int64 old_max = old_rrknob.get_max_val();
                      pknobs::int64 old_min = old_rrknob.get_min_val();
                      idx = i;
                      //cout << "DEBUG: " << "ret_value " << ret_value << " old_min " << old_min << " old_max " << old_max << endl;



                      if(old_min < ret_value) {
                          string knobStr = getName() + "_RRKnob_" + to_string(itrNo) + "_"+ to_string(old_min) + "_" + to_string(ret_value-1);
                          new_min_rrknob = new pknobs::RRKnob(knobStr, old_min, ret_value - 1);
                      }

                      if(ret_value < old_max) {
                          string knobStr = getName() + "_RRKnob_" + to_string(itrNo) + "_"+ to_string(ret_value+1) + "_" + to_string(old_max);
                          new_max_rrknob = new pknobs::RRKnob(knobStr, ret_value + 1, old_max);
                      }

                      //cout << "idx: " << idx << endl;
                      break;
                  }
              }

              if(idx != -1) {
                  //stringstream debug;
                  //knob_vec[idx]->print(debug);
                  //cout << "deleting : " << debug.str() << endl;
                  delete knob_vec[idx];
                  knob_vec.erase(knob_vec.begin() + idx);
                  pknobs::int64 old_wgt;

                  if(idx != 0) {
                      old_wgt = wgt_vec[idx-1];
                  } else {
                      old_wgt = 0;
                  }



                  wgt_vec.erase(wgt_vec.begin() + idx);


                  if(knob_vec.size() != wgt_vec.size()) {
                      cout <<" ERROR: knob_vec and wgt_vec are not of same size (" << knob_vec.size() << " vs " << wgt_vec.size() << endl;
                  }

                  if((knob_vec.size() == 0) && !new_max_rrknob && !new_min_rrknob) {
                      itrNo++;
                      string knobStr = getName() + "_RRKnob_" + to_string(itrNo) + "_" + to_string(lower) + "_" + to_string(upper);
                      knob_vec.push_back( new pknobs::RRKnob(knobStr, lower,upper));
                      int u = upper - lower + 1;
                      wgt_vec.push_back(u);
                      total_wgt = u;
                      //cout << "resetting" << endl;
                  } else {

                      total_wgt -= 1;

                      if(new_min_rrknob) {
                          knob_vec.insert(knob_vec.begin() + idx, new_min_rrknob);
                          old_wgt += (new_min_rrknob->get_max_val() - new_min_rrknob->get_min_val() + 1);
                          wgt_vec.insert(wgt_vec.begin() + idx, old_wgt);
                          idx++;
                      }

                      if(new_max_rrknob) {
                          knob_vec.insert(knob_vec.begin() + idx, new_max_rrknob);
                          old_wgt += (new_max_rrknob->get_max_val() - new_max_rrknob->get_min_val() + 1);
                          wgt_vec.insert(wgt_vec.begin() + idx, old_wgt);
                          idx++;
                      }

                      // update upper index weights
                      for(unsigned ii = idx; ii < wgt_vec.size(); ii++) {
                          wgt_vec[ii]--;
                      }

                  }



                  return ret_value;
              }
              assert(0);
          }

  };

}


#endif
