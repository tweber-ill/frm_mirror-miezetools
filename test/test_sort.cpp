// clang -o test_sort test_sort.cpp -lstdc++ -std=c++11

#include "../helper/misc.h"
#include <iostream>

template<class... Ts>
void print(const std::vector<std::tuple<Ts...> >& vec)
{
	for(const std::tuple<Ts...>& elem : vec)
	{
		//const unsigned int iElems = std::tuple_size<std::tuple<Ts...>>::value;
		std::cout << std::get<0>(elem) << "\t";
		std::cout << std::get<1>(elem) << "\t";
		std::cout << std::endl;
	}
}


int main()
{
	std::vector<std::tuple<int, std::string> > vec
	{
		std::tuple<int, std::string>(4, "vier"),
		std::tuple<int, std::string>(3, "drei"),
		std::tuple<int, std::string>(1, "eins"),
		std::tuple<int, std::string>(2, "zwei"),
	};

	print<int, std::string>(vec);
	sorttuples<0, int, std::string>(vec);
	std::cout << std::endl;
	print<int, std::string>(vec);
}
