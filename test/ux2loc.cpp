// clang -O2 -o ux2loc ux2loc.cpp -lstdc++ -std=c++11

#include <chrono>
#include <ctime>
#include <iostream>

static inline std::string ux2loc(double dTimestamp)
{
	using namespace std::chrono;
	typedef duration<double, std::ratio<1,1>> t_dur;

	//system_clock::time_point tpNow = system_clock::now();
	//system_clock::duration durNow = tpNow.time_since_epoch();
	system_clock::time_point tpZero /*= tpNow - durNow*/;

	t_dur durTS(dTimestamp);
	system_clock::duration durTS_sys = duration_cast<system_clock::duration>(durTS);
	system_clock::time_point tp = tpZero + durTS_sys;

	//t_dur durRem = durTS - duration_cast<t_dur>(durTS_sys);
	//std::cout << double(t_dur::period::num)/double(t_dur::period::den) * double(durRem.count()) << std::endl;

	std::time_t tt = system_clock::to_time_t(tp);
	std::tm tm = *std::localtime(&tt);

	char cTime[128];
	std::strftime(cTime, sizeof cTime, "%a %Y-%b-%d %H:%M:%S %Z (UTC%z)", &tm);

	double dRem = dTimestamp - double(long(dTimestamp));
	dRem *= 1000.;

	std::string strTime(cTime);
	if(dRem != 0.)
		strTime += " " + std::to_string(dRem) + " ms";
	return strTime;
}

int main(int argc, char** argv)
{
	if(argc<=1)
	{
		std::cerr << "Usage: " << argv[0] << " <timestamp1> <timestamp2> ...";
		std::cerr << std::endl;
		return -1;
	}

	for(int i=1; i<argc; ++i)
	{
		double dTimestamp = std::stod(argv[i]);
		std::string strLoc = ux2loc(dTimestamp);

		double dRem = dTimestamp - double(long(dTimestamp));
		if(dRem != 0.)
			std::cout << std::fixed << dTimestamp;
		else
			std::cout << long(dTimestamp);

		std::cout << ": " << strLoc << "\n";
	}

	std::cout.flush();
	return 0;
}
