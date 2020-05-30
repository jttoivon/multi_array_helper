
#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <cxxabi.h>
#include "boost/multi_array.hpp"
#include "boost/array.hpp"
#include "boost/cstdlib.hpp"
#include "boost/next_prior.hpp"


namespace multi_array_helper {
  
//////////////////////////////////////////////////////////////
//
// Extract information from multi array objects
//
//////////////////////////////////////////////////////////////


namespace detail {
  template <typename T>
  typename std::enable_if<std::is_arithmetic<T>::value>::type
  print2(std::ostream& os, const T& x, const int total_dimensions)
  {
    os << x;
  }

  template <typename Array>
  typename std::enable_if<!std::is_arithmetic<Array>::value>::type
  print2(std::ostream& os, const Array& A, const int total_dimensions)
  {

    typename Array::const_iterator i;
    os << "[";
    for (i = A.begin(); i != A.end(); ++i) {
      print2(os, *i, total_dimensions);
      if (boost::next(i) != A.end()) {
	os << ',';
	os << std::string(Array::dimensionality - 1, '\n');
	os << std::string(total_dimensions - Array::dimensionality + 1, ' ');
      }
    }
    os << "]";
  }
}

template <typename Array>
void
print(std::ostream& os, const Array& A)
{
  detail::print2(os, A, Array::dimensionality);
  os << std::endl;
}



template <typename Array>
void shape(std::ostream& os, const Array& A)
{
  for (size_t dim=0; dim < Array::dimensionality; ++dim) {
    int base = A.index_bases()[dim];
    if (base == 0) 
      os << '[' << A.shape()[dim] << ']';
    else
      os << "[range(" << base << "," << base+A.shape()[dim] << ")]";
      
  }
  os << std::endl;
}



namespace detail {
  // level == 1
  template <typename Array>
  boost::detail::multi_array::extent_gen<Array::dimensionality>
  get_extents_helper(const Array& a, boost::detail::multi_array::extent_gen<Array::dimensionality-1> e)
  {
    typedef typename Array::extent_range range;
    int dim = Array::dimensionality - 1;
    //auto e = boost::extents;
    int base = a.index_bases()[dim];
    int size = a.shape()[dim];

    return e[range(base, base+size)];
  }

  // Level start from dim and goes down to 1 in the specialisation
  template <typename Array, size_t level>
  boost::detail::multi_array::extent_gen<Array::dimensionality>
  get_extents_helper(const Array& a, boost::detail::multi_array::extent_gen<Array::dimensionality-level> e)
  {
    typedef typename Array::extent_range range;
    int dim = Array::dimensionality - level;
    //auto e = boost::extents;
    int base = a.index_bases()[dim];
    int size = a.shape()[dim];

    return get_extents_helper<Array>(a, e[range(base, base+size)]);
  }
}

// Returns the extents of a multi array in a form that can be
// passed to the constructor of boost::multi_array.
template <typename Array>
boost::detail::multi_array::extent_gen<Array::dimensionality>
get_extents(const Array& a)
{
  return detail::get_extents_helper<Array, Array::dimensionality>(a, boost::detail::multi_array::extent_gen<0>());
}

	    
namespace detail {
  template <typename T>
  std::string
  get_type()
  {
    int status;
    std::string name = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    return name;
  }
}
  


template <typename Array>
void
info(const Array& a)
{
  //  std::cout << "Container type: " << array_type(a) << std::endl;
  std::cout << "Container type: " << detail::get_type<Array>() << std::endl;
  std::cout << "Element type: "   << detail::get_type<typename Array::element>() << std::endl;
  std::cout << "Dimensions: " << Array::dimensionality << std::endl;
  std::cout << "Shape: ";
  shape(std::cout, a);
  std::cout << "Number of elements: " << a.num_elements() << std::endl;
}



//////////////////////////////////////////////////////////////
//
// An example algorithm
//
//////////////////////////////////////////////////////////////


// Does not work for subarrays, array_refs, or views
template <typename Array>
typename Array::element
sum_array(const Array& a)
{
  typedef typename Array::element element;
  typedef typename Array::size_type size_type;
  element s = element();
  for (size_type i=0; i < a.num_elements(); ++i)
    s += a.data()[i];
  return s;
}

  // The sum algorithm is easy to do in C++17 using constexprs.
  // When using older compilers one has to use a more complicated
  // method based on enable_if, which is difficult to get right.

// if constexpr requires at least C++17
# if __cplusplus >= 201703
template <typename Array>
typename Array::element
sum(const Array& a)
{
  typedef typename Array::value_type value_type;
  typedef typename Array::const_iterator iterator;
  typedef typename Array::element element;
  element s = element();
  if constexpr(std::is_arithmetic<value_type>::value) {
    for (iterator it=a.begin(); it != a.end(); ++it)
      s += *it;
  }
  else {
    for (iterator it=a.begin(); it != a.end(); ++it)
      s += sum(*it);
  }
  return s;
}
#else
// See:
// https://stackoverflow.com/questions/12073689/c11-template-function-specialization-for-integer-types
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value,
			T>::type 
sum(const T& a)
{
  return a;
}

template <typename Array>
typename std::enable_if<!std::is_arithmetic<Array>::value,
			typename Array::element>::type
sum(const Array& a)
{
  typedef typename Array::element element;
  typedef typename Array::const_iterator iterator;
  //typedef typename Array::size_type size_type;
  element s = element();
  for (iterator it=a.begin(); it != a.end(); ++it)
    s += sum(*it);
  return s;
}
#endif



//////////////////////////////////////////////////////////////
//
// Construction
//
//////////////////////////////////////////////////////////////


// This version does not generalize.
template <typename T>
boost::multi_array<T, 1>
construct(std::initializer_list<T> L)
{
  for (T e : L) {
    std::cout << e << ", ";
  }
  boost::multi_array<T, 1> A(boost::extents[L.size()]);
  A.assign(L.begin(), L.end());
  info(A);
  return A;
}


template <typename T, size_t N1, size_t N2>
boost::multi_array<T, 2>
construct(const T (&L)[N1][N2])
{
  size_t size = N1*N2;
  boost::multi_array<T, 2> A(boost::extents[N1][N2]);
  const T* v = reinterpret_cast<const T*>(L);
  A.assign(v, v + size);
  return A;
}


//////////////////////////////////////////////////////////////
//
// Iteration
//
//////////////////////////////////////////////////////////////

  // This provides a flat iterator over all elements of the multi array.
  // The normal iterators of boost::multi_array iterate over all subarrays.
  
template <typename Array>
class iterator
{
public:
  iterator(Array& a_, bool end=false) : a(a_), k(Array::dimensionality)
  {
    for (unsigned int i=0; i < k; ++i)
      idx[i] = a.index_bases()[i];
    if (end)
      idx[0] = a.index_bases()[0] + a.shape()[0];   // this denotes an end iterator
  }

  typename Array::element
  operator*()
  {
    return a(idx);
  }

  void
  operator++()
  {
    int i=Array::dimensionality-1;
    while (i >= 0 and idx[i] == (int)(a.index_bases()[i] + a.shape()[i] - 1)) {
      idx[i] = a.index_bases()[i];
      i -= 1;
    }
    if (i == -1)
      idx[0] = a.index_bases()[0] + a.shape()[0];  // denotes the end iterator
    else
      idx[i] += 1;
  }

  bool
  operator!=(const iterator<Array>& other) const
  {
    return idx != other.idx;
  }
  
  boost::array<typename Array::index, Array::dimensionality>
  get_indices() const
  {
    return idx;
  }
  
private:
  Array& a;
  const unsigned int& k;   // can I use an alias here?
  boost::array<typename Array::index, Array::dimensionality> idx;  //
};

template <typename Array>
iterator<Array>
begin(Array& a)
{
  return iterator<Array>(a);
}

template <typename Array>
iterator<Array>
end(Array& a)
{
  return iterator<Array>(a, true);
}

// It is maybe easier to you the above iterator than these macros.
// Helper macros to iterate all dimensions of a 1, 2, 3, or 4 dimensional multi array
#define MA_FOREACH1(i1, container)                                                        \
  for (int i1=container.index_bases()[0] ; i1 < container.index_bases()[0] + (int)container.shape()[0]; ++i1)

#define MA_FOREACH2(i1, i2, container)					\
  for (int i1=container.index_bases()[0] ; i1 < container.index_bases()[0] + (int)container.shape()[0]; ++i1) \
  for (int i2=container.index_bases()[1] ; i2 < container.index_bases()[1] + (int)container.shape()[1]; ++i2)

#define MA_FOREACH3(i1, i2, i3, container)				\
  for (int i1=container.index_bases()[0] ; i1 < container.index_bases()[0] + (int)container.shape()[0]; ++i1) \
  for (int i2=container.index_bases()[1] ; i2 < container.index_bases()[1] + (int)container.shape()[1]; ++i2) \
  for (int i3=container.index_bases()[2] ; i3 < container.index_bases()[2] + (int)container.shape()[2]; ++i3)

#define MA_FOREACH4(i1, i2, i3, i4, container)				\
  for (int i1=container.index_bases()[0] ; i1 < container.index_bases()[0] + (int)container.shape()[0]; ++i1) \
  for (int i2=container.index_bases()[1] ; i2 < container.index_bases()[1] + (int)container.shape()[1]; ++i2) \
  for (int i3=container.index_bases()[2] ; i3 < container.index_bases()[2] + (int)container.shape()[2]; ++i3) \
  for (int i4=container.index_bases()[3] ; i4 < container.index_bases()[3] + (int)container.shape()[3]; ++i4)


} // end namespace multi_array_helper
