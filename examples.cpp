

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

template <typename T, size_t N>
std::string
to_string(const boost::array<T, N>& v)
{
  if (v.size() == 0)
    return std::string("[]");
  std::ostringstream result;
  result << "[";
  for (size_t i=0; i < v.size()-1; ++i)
    result << v[i] << ", ";
  result << v[v.size()-1] << "]";

  return result.str();
}



int main()
{
  //using namespace multi_array_helper;
  namespace mah = multi_array_helper;
  
  typedef  boost::multi_array<double, 2> array;
  typedef boost::multi_array<double, 2>::extent_range range;


  // Some data from which to create multi arrays
  double values1[] = {
    0, 1, 2,
    3, 4, 5 
  };
  const int values_size1=sizeof(values1);

  double values2[2][3] = { {6,7,8},
			   {9, 10, 11}
  };
  const int values_size2=sizeof(values2);

  
  std::cout << "Testing multi array creation:\n";
  std::cout << "-----------------------------\n";

  array A(boost::extents[2][3]);
  A.assign(values1,values1+values_size1);
  std::cout << "Array A:\n";
  mah::info(A);
  mah::print(std::cout, A);
  printf("\n");

  array B(boost::extents[2][range(1,4)]);
  B.assign(values1,values1+values_size1);
  std::cout << "Array B:\n";
  mah::info(B);
  mah::print(std::cout, B);
  printf("\n");


  array C(boost::extents[2][range(1,4)]);
  double* values3 = reinterpret_cast<double*>(values2);
  C.assign(values3, values3+values_size2);
  std::cout << "Array C:\n";
  mah::info(C);
  mah::print(std::cout, C);
  printf("\n");


  std::cout << "Testing construct function:\n";
  std::cout << "---------------------------\n";
  boost::multi_array<int, 1> B1 = mah::construct({4, 4, 5});
  std::cout << "Array B1:\n";
  mah::info(B1);
  mah::print(std::cout, B1);
  printf("\n");

  boost::multi_array<int, 2> B2 = mah::construct({ {6,7,8},
  					           {9, 10, 11}});
  std::cout << "Array B2:\n";
  mah::info(B2);
  mah::print(std::cout, B2);
  printf("\n");


  boost::multi_array<double, 2> B3 = mah::construct(values2);
  B3.reindex(boost::array<int, 2>({2, -5}));   // Set the index bases
  std::cout << "Array B3:\n";
  mah::info(B3);
  mah::print(std::cout, B3);
  printf("\n");

  std::cout << "Testing printing high-dimensional array:\n";
  std::cout << "----------------------------------------\n";
  boost::multi_array<double, 4> D(boost::extents[3][3][3][3]);
  std::cout << "Array D:\n";
  mah::info(D);
  mah::print(std::cout, D);
  printf("\n");


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

  std::cout << "Testing MAH_FOREACH:\n";
  std::cout << "--------------------\n";
  boost::multi_array<double, 1> A1(boost::extents[range(-1, 2)]);
  printf("A1:\n");
  mah::info(A1);
  MAH_FOREACH1(i, A1) {
    printf("A1[%i]==%f\n", i, A1[i]);
  }
  printf("\n");
  
  printf("A:\n");
  mah::info(A);
  MAH_FOREACH2(i, j, A) {
    printf("A[%i, %i]==%f\n", i, j, A[i][j]);
  }
  printf("\n");

  std::cout << "Testing iterators:\n";
  std::cout << "------------------\n";
  for (auto it=mah::begin(A); it != mah::end(A); ++it) {
    printf("A[%s]==%f\n", to_string(it.get_indices()).c_str(), *it);
  }
  return boost::exit_success;
}

//  The output is: 
// [[0,1,2],[3,4,5]]
