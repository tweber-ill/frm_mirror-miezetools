/*
 * Filer chain
 * @author tw
 */

#include "filter.h"


int main()
{
	FilterStack<float> filters;
	filters.AddFilter(new Despeckle<float>());
	filters.AddFilter(new Normalize<float>());

	BWImage<float> img(128, 128);
	BWImage<float> dark(128, 128);
	BWImage<float> open(128, 128);
	
	filters.Exec(&img, &dark, &open);
	
	return 0;
}
