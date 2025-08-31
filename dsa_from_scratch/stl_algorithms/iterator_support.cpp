#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>

  /*
    understand
    iterator_traits<T> is a struct, which
    contains appropriate typenames eg. iterator_category, difference etc
    which bind the iterator. just a helper struct

template <typename Iter>
struct iterator_traits {
    using difference_type   = typename Iter::difference_type;
    using value_type        = typename Iter::value_type;
    using pointer           = typename Iter::pointer;
    using reference         = typename Iter::reference;
    using iterator_category = typename Iter::iterator_category;
    };

  now, onto iterator_category:
  what are these?
  struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : input_iterator_tag {};
struct bidirectional_iterator_tag : forward_iterator_tag {};
struct random_access_iterator_tag : bidirectional_iterator_tag {};
struct contiguous_iterator_tag : random_access_iterator_tag {};
just an hierarchy of types from weaker to stronger.

now, how to implement stl algo??

define the above iterator_trais,
now, for function API

template<typename iter>
void func_interface(iter& it){
func_imple(it, typename iterator_traits<iter>::iterator_category);
}
take whatever arguments by defining typename <iter>::difference_type now
to implement algo's.
the actual implementation is template overloaded on the category.



option 2:
template<std::bidirectional iter>
void func(){}
this is a concept, bound by std::bidirectional CONCEPT

this is how it is defined
template<typename iter>
concept bidirectional_iterator =
std::forward_iterator<iter> &&
reequires(iter i){
{--i} ->std::same_as<iter&>
{i++} -> std::same_as<iter>;
}
we could also use traits and same_as
but yea compilers aren't able to parse it in
compile time very well!
   */


namespace prat {


  

  template<typename T, typename U>
  struct is_same{
    static constexpr bool value = false; //constexpr can be initialsied inside struct
  };
  template<typename T>
  struct is_same<T,T>{
    static constexpr bool value = true;
  };
  
	   template<typename T, typename U>
  bool is_same_v = is_same<T,U>::value;

  
  template<typename T>
  struct  bidirectional_iterator{
    static constexpr bool val = is_same<typename std::iterator_traits<T>::iterator_category,std::bidirectional_iterator_tag>::value; //compiler thinks its a value, not a type when parsing template! must use prefix typename
  };
  
  template<typename T>
 bool bidirectional_iterator_v = bidirectional_iterator<T>::val;

  
  template <typename iter>
  requires std::bidirectional_iterator<iter> //must be a concept, with  boolean predicate on a type! mine it can't resolve at compile time!  STL goes around this , by operation based checks! compilers are dumb and can't deduce that much
  void algo(iter mybegin, iter myend) {
    //requires clause approach!
    std::cout <<"bind to bidirectional" <<"\n";
  }
  //using concepts!
  
  template<std::forward_iterator iter>
  void algo(iter mybegin, iter myend){
    std::cout <<"bind to forward" <<"\n";
  }
}; // namespace prat, both are same!





int main() {
  std::vector<int> vec;
  std::set<int> se;
  std::unordered_set<int> unor;

  prat::algo(vec.begin(),vec.end());

  }  
