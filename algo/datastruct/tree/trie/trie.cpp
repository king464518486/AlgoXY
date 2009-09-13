#include <iostream>
#include <map>
#include <functional>
#include <sstream>
#include <algorithm>
#include <list>
#include <iterator>

template<class Coll>
void print(const Coll& xs, std::string prefix=""){
  using namespace std;
  cout<<prefix<<": ";
  copy(xs.begin(), xs.end(), ostream_iterator<typename Coll::value_type>(cout, ", "));
  cout<<"\n";
}

// default mapping functor, map from search input to keys in Trie
template<class T>
struct map_identity: public std::unary_function<T, std::list<T> >{
  std::list<T> operator()(T x){
    return std::list<T>(1, x);
  }
};

// T9 mapping functor
struct t9_mapping: public std::unary_function<char, std::list<char> >{
  std::map<char, std::string> keypad;
  t9_mapping(){
    keypad['2']="abc";
    keypad['3']="def";
    keypad['4']="ghi";
    keypad['5']="jkl";
    keypad['6']="mno";
    keypad['7']="pqrs";
    keypad['8']="tuv";
    keypad['9']="wxyz";
  }

  std::list<char> operator()(char x){
    using namespace std;
    list<char> res;
    back_insert_iterator<list<char> > i(res);
    if(keypad.find(x)!=keypad.end())
      copy(keypad[x].begin(), keypad[x].end(), i);
    return res;
  }
};

template<class T, class _MapFunc=map_identity<T> >
struct Trie{
  typedef _MapFunc MapFunc;
  typedef std::map<T, Trie<T, MapFunc>*> Children;

  Trie(int x=-1):value(x){}

  ~Trie(){
    for(typename Children::iterator it=children.begin();
        it!=children.end(); ++it)
      delete it->second;
  }

  int value; //augument data, frequency etc.
  Children children;
};

// recursive
template<class T, class MapFunc, class Coll>
Trie<T, MapFunc>* insert(Trie<T, MapFunc>* t, Coll value){
  if(!t)
    t=new Trie<T, MapFunc>;

  if(!value.empty()){
    typename Coll::iterator it=value.begin();
    t->children[*it]=insert(t->children[*it], 
                            Coll(++value.begin(), value.end())); 
  }
  return t;
}

// helper functions to map appending element a list
// map (lambda e, es, -> (e+es)) xs
template<class T, class Coll>
Coll operator+(T x, Coll xs){
  for(typename Coll::iterator it=xs.begin(); it!=xs.end(); ++it){
    std::insert_iterator<typename Coll::value_type> i(*it, it->begin());
    *i++=x;
  }
  return xs;
}

template<class T, class MapFunc, class Coll>
std::list<Coll> search(Trie<T, MapFunc>* t, Coll value){
  std::list<Coll> res;

  if(t->children.empty() || value.empty())
      return res;

  MapFunc f;
  typedef typename MapFunc::result_type Keys; 
  Keys keys=f(*value.begin());
  for(typename Keys::iterator k=keys.begin(); k!=keys.end(); ++k){
    if(t->children.find(*k)!=t->children.end()){
      Coll tail=Coll(++value.begin(), value.end());
      std::list<Coll> xs;
      if(tail.empty())
        xs.push_back(Coll(1, *k));
      else
        xs=(*k)+search(t->children[*k], tail);
      std::back_insert_iterator<std::list<Coll> > i(res);
      std::copy(xs.begin(), xs.end(), i);
    }
  }
  return res;
}

// test helpers
template<class T, class MapFunc>
std::string trie_to_str(Trie<T, MapFunc>* t, std::string prefix=""){
  std::ostringstream s;
  s<<"("<<prefix;
  for(typename Trie<T, MapFunc>::Children::iterator it=t->children.begin();
      it!=t->children.end(); ++it)
    s<<", "<<trie_to_str(it->second, prefix+it->first);
  s<<")";
  return s.str();
}

class Test{
private:
  Trie<char>* t;
  Trie<char, t9_mapping>* t1;

  void test_insert(){
    t=insert(t, std::string("a"));
    t=insert(t, std::string("b"));
    insert(t, std::string("good"));
    insert(t, std::string("home"));
    insert(t, std::string("gone"));
    std::cout<<"t=\n"<<trie_to_str(t)<<"\n";

    t1=insert(t1, std::string("good"));
    t1=insert(t1, std::string("home"));
    t1=insert(t1, std::string("gone"));
    t1=insert(t1, std::string("hood"));    
    t1=insert(t1, std::string("hello"));
    t1=insert(t1, std::string("a"));
    t1=insert(t1, std::string("an"));
    t1=insert(t1, std::string("the"));
    std::cout<<"t1=\n"<<trie_to_str(t1)<<"\n";
  }

  void test_search(){
    std::list<std::string> res=search(t, std::string("a"));
    print(res, "search a");
    res=search(t, std::string("go"));
    print(res, "search go");
    res=search(t, std::string("bar"));
    print(res, "search bar");
    res=search(t, std::string("gone"));
    print(res, "search gone");

    res=search(t1, std::string("46"));
    print(res, "t9 search 46");
    res=search(t1, std::string("4663"));
    print(res, "t9 search 4663");
    res=search(t1, std::string("2"));
    print(res, "t9 search 2");
  }

public:
  Test():t(0), t1(0){ }

  ~Test(){
    delete t;
    delete t1;
  }

  void run(){
    test_insert();
    test_search();
  }
};

int main(int, char**){
  Test().run();
}
