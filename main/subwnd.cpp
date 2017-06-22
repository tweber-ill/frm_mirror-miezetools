/**
 * mieze-tool
 * base class for mdi subwindows
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 27-sep-2013
 * @license GPLv3
 */

#include "subwnd.h"
#include "data/data.h"
#include "plot/plot.h"


const StringMap* SubWindowBase::GetParamMapDyn() const
{
	const DataInterface *pIf = GetDataInterface();
	if(!pIf) return 0;

	const StringMap* pParams = 0;
	if(GetType() == PLOT_1D)
		pParams = &((Plot*)this)->GetParamMapDynMerged();
	if(!pParams)
		pParams = &pIf->GetParamMapDyn();

	return pParams;
}


const StringMap* SubWindowBase::GetParamMapStat() const
{
	const DataInterface *pIf = GetDataInterface();
	if(!pIf) return 0;
	return &pIf->GetParamMapStat();
}
