// clang -o test_tok test_tok.cpp ../helper/string.cpp -lstdc++ -std=c++11
#include "../helper/string.h"

int main()
{
	std::vector<std::string> vecTok;
	get_tokens<std::string, std::string>("ABC DEF;1234;X Y Z", ";", vecTok);
	std::copy(vecTok.begin(), vecTok.end(), std::ostream_iterator<std::string>(std::cout, ", "));
	std::cout << std::endl;


	std::vector<int> vecTokInt;
	get_tokens<int, std::string>("123, 345, 5677", ",", vecTokInt);
	std::copy(vecTokInt.begin(), vecTokInt.end(), std::ostream_iterator<int>(std::cout, ", "));
	std::cout << std::endl;

	return 0;
}
