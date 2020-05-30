

// When creating a function that operates on MA,
// don't restrict it to multi_array, but
// allow also views, sub_arrays, and refs.

// The order of enable_ifs is important.


/*
TODO:

- For each tyyppinen looppi, ehkä yksi per dimensio
  muistaakseni käytin moderissa tällaista. TEHTY

- Onko mahdollista rakentaa multi_array C-tyyppisestä
  moniulotteisesta taulukosta? Joo makrojen avulla, mutta pitää olla
  jokaista ulottuvuuksien määrää kohti oma makro

- Iteraattorit. Tein flat iteratorin

*/

#include "multi_array_helper.hpp"
#include <iostream>
#include "boost/multi_array.hpp"
/*
#include <type_traits>
#include <typeinfo>
#include <cxxabi.h>
#include "boost/array.hpp"
#include "boost/cstdlib.hpp"
#include "boost/next_prior.hpp"
*/

template <typename T>
std::string
to_string(const std::vector<T>& v)
{
  if (v.size() == 0)
    return std::string("[]");
  std::ostringstream result;
  result << "[";
  for (int i=0; i < v.size()-1; ++i)
    result << v[i] << ", ";
  result << v[v.size()-1] << "]";

  return result.str();
}

template <typename T, long unsigned int N>
std::string
to_string(const boost::array<T, N>& v)
{
  if (v.size() == 0)
    return std::string("[]");
  std::ostringstream result;
  result << "[";
  for (long unsigned int i=0; i < v.size()-1; ++i)
    result << v[i] << ", ";
  result << v[v.size()-1] << "]";

  return result.str();
}

int main()
{
  //using namespace multi_array_helper;
  namespace mah = multi_array_helper;
  
  typedef  boost::multi_array<double, 2> array;
  double values[] = {
    0, 1, 2,
    3, 4, 5 
  };

  double values2[2][3] = { {6,7,8},
			   {9, 10, 11}
  };

  
  const int values_size=6;
  array A(boost::extents[2][3]);
  //typedef boost::multi_array_types::index_range range;
  typedef boost::multi_array<double, 2>::extent_range range;
  array B(boost::extents[2][range(1,4)]);
  A.assign(values,values+values_size);
  B.assign(values,values+values_size);
  array C(boost::extents[2][range(1,4)]);
  double* values3 = reinterpret_cast<double*>(values2);
  C.assign(values3, values3+values_size);
  boost::multi_array<double, 4> D(boost::extents[3][3][3][3]);
  boost::multi_array<double, 1> A1(boost::extents[range(-1, 2)]);
  
  std::cout << "Array A:\n";
  mah::print(std::cout, A);

  std::cout << "Array B:\n";
  mah::print(std::cout, B);

  boost::multi_array<int, 1> B1 = mah::construct({4, 4, 5});
  std::cout << "Array B1:\n";
  mah::info(B1);
  mah::print(std::cout, B1);

  boost::multi_array<int, 2> B2 = mah::construct({ {6,7,8},
					       {9, 10, 11}});
  std::cout << "Array B2:\n";
  mah::info(B2);
  mah::print(std::cout, B2);

  std::cout << "Array C:\n";
  mah::print(std::cout, C);

  std::cout << "Array D:\n";
  mah::print(std::cout, D);

  mah::shape(std::cout, B);

  std::cout << "Testing sum algorithm:\n";
  std::cout << "----------------------\n";
  printf("Sum of elements of A: %f\n", mah::sum(A));
  printf("Sum of first slice: %f\n", mah::sum(A[0]));
  std::cout << std::endl;
  
  std::cout << "Testing a view to A:\n";
  std::cout << "--------------------\n";
  typedef boost::multi_array_types::index_range irange;
  auto v = A[ boost::indices[1][irange(1,3)] ];
  mah::info(v);
  mah::print(std::cout, v);
  printf("Sum of view of A: %f\n", mah::sum(v));
  //printf("Sum of view of A: %f\n", sum_array(v));  // ERROR: no data() method
  printf("\n");
  
  std::cout << "Testing subarray:\n";
  std::cout << "-----------------\n";
  printf("B:\n");
  mah::info(B);
  printf("\n");

  printf("B[0]:\n");
  mah::info(B[0]);
  printf("Sum of subarray B[0]: %f\n", mah::sum(B[0]));
  //printf("Sum of subarray B[0]: %f\n", sum_array(B[0]));  // ERROR: no data() method
  printf("\n");

  std::cout << "Testing get_extents:\n";
  std::cout << "--------------------\n";
  mah::info(array(mah::get_extents(A)));
  std::cout << std::endl;

  std::cout << "Testing MA_FOREACH:\n";
  std::cout << "--------------------\n";
  printf("A1:\n");
  mah::info(A1);
  MA_FOREACH1(i, A1) {
    printf("A1[%i]==%f\n", i, A1[i]);
  }
  //for (auto it = A1.begin(); it != A1.end(); ++it)
  //  ;
  printf("\n");
  printf("A:\n");
  mah::info(A);
  MA_FOREACH2(i, j, A) {
    printf("A[%i, %i]==%f\n", i, j, A[i][j]);
  }
  printf("\n");
  //for (auto it = A[0].begin(); it != A[0].end(); ++it)
  //  ;
  

  std::cout << "Testing iterators:\n";
  std::cout << "------------------\n";
  for (auto it=mah::begin(A); it != mah::end(A); ++it) {
    printf("A[%s]==%f\n", to_string(it.get_indices()).c_str(), *it);
  }
  return boost::exit_success;
}

//  The output is: 
// [[0,1,2],[3,4,5]]
