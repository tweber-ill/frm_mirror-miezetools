/*
 * mieze-tool
 * data export
 * @author tweber
 * @date 17-jun-2013
 */


#ifndef __MIEZE_DATA_EXPORT__
#define __MIEZE_DATA_EXPORT__

#include "../main/subwnd.h"
#include "data.h"
#include <vector>

extern bool export_py(const char* pcFile, const SubWindowBase *pSWB);
extern bool export_subplots_py(const char* pcFile, const std::vector<SubWindowBase*>&, int iHCnt, int iVCnt);

#endif
