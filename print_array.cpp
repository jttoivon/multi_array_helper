// Copyright 2002 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software 
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Boost.MultiArray Library
//  Authors: Ronald Garcia
//           Jeremy Siek
//           Andrew Lumsdaine
//  See http://www.boost.org/libs/multi_array for documentation.

// When creating a function that operates on MA,
// don't restrict it to multi_array, but
// allow also views, sub_arrays, and refs.

// The order of enable_ifs are important.


#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <cxxabi.h>
#include "boost/multi_array.hpp"
#include "boost/array.hpp"
#include "boost/cstdlib.hpp"
#include "boost/next_prior.hpp"

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value>::type
print(std::ostream& os, const T& x)
{
  os << x;
}

template <typename Array>
typename std::enable_if<!std::is_arithmetic<Array>::value>::type
print(std::ostream& os, const Array& A)
{
  typename Array::const_iterator i;
  os << "[";
  for (i = A.begin(); i != A.end(); ++i) {
    print(os, *i);
    if (boost::next(i) != A.end())
      os << ',';
  }
  os << "]";
}

template <typename Array>
void shape(std::ostream& os, const Array& A)
{
  for (int dim=0; dim < Array::dimensionality; ++dim) {
    int base = A.index_bases()[dim];
    if (base == 0) 
      os << '[' << A.shape()[dim] << ']';
    else
      os << "[range(" << base << "," << base+A.shape()[dim] << ")]";
      
  }
  os << std::endl;
}


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


template <typename Array>
boost::detail::multi_array::extent_gen<Array::dimensionality>
get_extents(const Array& a)
{
  return get_extents_helper<Array, Array::dimensionality>(a, boost::detail::multi_array::extent_gen<0>());
}

	    

template <typename T>
std::string
get_type()
{
  int status;
  std::string name = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
  return name;
}

/*
template <typename Array>
std::string
array_type(const Array& a)
{
  int status;
  std::string name = get_type<Array>();
  return std::string("Other: ") + name;
}

template <
  typename ValueType, 
  std::size_t NumDims, 
  typename Allocator = std::allocator<ValueType> >
std::string
array_type(const boost::multi_array<ValueType, NumDims, Allocator>& a)
{
  typedef boost::multi_array<ValueType, NumDims, Allocator> Array;
  return std::string("multi_array") + get_type<Array>();
}

template <
  typename ValueType, 
  std::size_t NumDims>
std::string
array_type(const typename boost::detail::multi_array::sub_array<ValueType, NumDims>& a)
{
  return "subarray";
}
*/


template <typename Array>
void
info(const Array& a)
{
  //  std::cout << "Container type: " << array_type(a) << std::endl;
  std::cout << "Container type: " << get_type<Array>() << std::endl;
  std::cout << "Dimensions: " << Array::dimensionality << std::endl;
  std::cout << "Shape: ";
  shape(std::cout, a);
  std::cout << "Number of elements: " << a.num_elements() << std::endl;
}

// Does not work for subarrays or array_refs
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

// See:
// https://stackoverflow.com/questions/12073689/c11-template-function-specialization-for-integer-types

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, double>::type 
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
  typedef typename Array::size_type size_type;
  element s = element();
  for (iterator it=a.begin(); it != a.end(); ++it)
    s += sum(*it);
  return s;
}

# if __cplusplus >= 201703
template <typename Array>
typename Array::element
sum_cpp17(const Array& a)
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
#endif

int main()
{
  typedef   boost::multi_array<double, 2> array;
  double values[] = {
    0, 1, 2,
    3, 4, 5 
  };
  const int values_size=6;
  array A(boost::extents[2][3]);
  //typedef boost::multi_array_types::index_range range;
  typedef boost::multi_array<double, 2>::extent_range range;
  array B(boost::extents[2][range(1,4)]);
  A.assign(values,values+values_size);
  B.assign(values,values+values_size);
  print(std::cout, A);
  std::cout << std::endl;
  print(std::cout, B);
  std::cout << std::endl;
  shape(std::cout, B);

  printf("Sum of elements of A: %f\n", sum(A));
  printf("Sum of first slice: %f\n", sum(A[0]));
  info(B);
  info(B[0]);

  info(array(get_extents(A)));
  //get_extents(A);
  return boost::exit_success;
}

//  The output is: 
// [[0,1,2],[3,4,5]]
