/*
 * mieze-tool
 * global settings
 * @author tweber
 * @date 04-mar-2013
 */

#ifndef __MIEZE_SETTINGS__
#define __MIEZE_SETTINGS__

#include <QtCore/QSettings>
#include <QtCore/QVariant>

class Settings
{
	protected:
		static QSettings *s_pGlobals;
		static void SetDefaults();

	public:
		static QSettings *GetGlobals();
		static void free();

		template<typename T>
		static T Get(const char* pcKey)
		{
			QSettings *pSett = GetGlobals();
			return pSett->value(pcKey).value<T>();
		}

		template<typename T>
		static void Set(const char* pcKey, const T& t)
		{
			QSettings *pSett = GetGlobals();
			pSett->setValue(pcKey, QVariant(t));
		}
};

#endif
