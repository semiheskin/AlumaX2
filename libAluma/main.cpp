#include "stdio.h"
#include "main.h"
#include "AlumaX2.h"
#include "iostream"
#include "basicstringinterface.h"

//Ths file normally does not require changing

#define PLUGIN_NAME "Unofficial Aluma Plugin"

extern "C" PlugInExport int sbPlugInName2(BasicStringInterface& str)
{
	str = PLUGIN_NAME;

	return 0;
}

extern "C" PlugInExport int sbPlugInFactory2(
	const char* pszSelection,
	const int& nInstanceIndex,
	SerXInterface* pSerXIn,
	TheSkyXFacadeForDriversInterface* pTheSkyXIn,
	SleeperInterface* pSleeperIn,
	BasicIniUtilInterface* pIniUtilIn,
	LoggerInterface* pLoggerIn,
	MutexInterface* pIOMutexIn,
	TickCountInterface* pTickCountIn,
	void** ppObjectOut)
{
	//*ppObjectOut = nullptr;
	//AlumaX2* gpMyImpl = nullptr;

	//if (nullptr == gpMyImpl)
	//	gpMyImpl = new AlumaX2(pszSelection,
	//		nInstanceIndex,
	//		pSerXIn,
	//		pTheSkyXIn,
	//		pSleeperIn,
	//		pIniUtilIn,
	//		pLoggerIn,
	//		pIOMutexIn,
	//		pTickCountIn);

	//*ppObjectOut = gpMyImpl;

	* ppObjectOut = AlumaX2::GetInstance(pSleeperIn, pLoggerIn, pIOMutexIn);

	return 0;
}

