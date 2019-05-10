#include "AlumaX2.h"

#include <mutexinterface.h>
#include <basicstringinterface.h>
#include <sberrorx.h>

#include <stdexcept>
#include <algorithm>

#define DEVICE_DRIVER_INFO_STRING "DL Aluma"

AlumaX2* AlumaX2::GetInstance(SleeperInterface* pSleeper, LoggerInterface* pLoggerIn, MutexInterface* pIOMutex)
{
	static auto instance = new AlumaX2{ pSleeper, pLoggerIn, pIOMutex };
	return instance;
}

AlumaX2::AlumaX2(SleeperInterface* pSleeper, LoggerInterface* pLoggerIn, MutexInterface* pIOMutex) : m_sleeper(pSleeper), m_logger(pLoggerIn), m_mutex(pIOMutex)
{
}

AlumaX2::~AlumaX2()
{
	delete m_sleeper;
	delete m_mutex;
}

//DriverRootInterface
int AlumaX2::queryAbstraction(const char* pszName, void** ppVal)
{
	X2MutexLocker locker(GetMutex());

	if (!strcmp(pszName, FilterWheelMoveToInterface_Name))
		* ppVal = dynamic_cast<FilterWheelMoveToInterface*>(this);
	else if (!strcmp(pszName, SubframeInterface_Name))
		* ppVal = dynamic_cast<SubframeInterface*>(this);

	return SB_OK;
}

//DriverInfoInterface
void AlumaX2::driverInfoDetailedInfo(BasicStringInterface& str) const
{
	X2MutexLocker locker(GetMutex());
	str = DEVICE_DRIVER_INFO_STRING;
}

double AlumaX2::driverInfoVersion() const
{
	X2MutexLocker locker(GetMutex());
	return 1.0;
}


//HardwareInfoInterface
void AlumaX2::deviceInfoNameShort(BasicStringInterface& str) const
{
	X2MutexLocker locker(GetMutex());
	str = DEVICE_DRIVER_INFO_STRING;
}

void AlumaX2::deviceInfoNameLong(BasicStringInterface& str) const
{
	X2MutexLocker locker(GetMutex());
	str = DEVICE_DRIVER_INFO_STRING;
}

void AlumaX2::deviceInfoDetailedDescription(BasicStringInterface& str) const
{
	X2MutexLocker locker(GetMutex());
	str = DEVICE_DRIVER_INFO_STRING;
}

void AlumaX2::deviceInfoFirmwareVersion(BasicStringInterface& str)
{
	X2MutexLocker locker(GetMutex());
	str = DEVICE_DRIVER_INFO_STRING;
}

void AlumaX2::deviceInfoModel(BasicStringInterface& str)
{
	X2MutexLocker locker(GetMutex());
	str = DEVICE_DRIVER_INFO_STRING;
}

//CameraDriverInterface
int AlumaX2::CCSettings(const enumCameraIndex& Camera, const enumWhichCCD& CCD)
{
	X2MutexLocker locker(GetMutex());
	return ERR_NOT_IMPL;
}

int AlumaX2::CCEstablishLink(enumLPTPort portLPT, const enumWhichCCD& CCD, enumCameraIndex DesiredCamera,
	enumCameraIndex& CameraFound, const int nDesiredCFW, int& nFoundCFW)
{
	X2MutexLocker locker(GetMutex());

	if (m_bLinked)
		return SB_OK;

	m_gateway.reset(dl::getGateway(), [](dl::IGateway * gw) { dl::deleteGateway(gw); });

	m_gateway->queryUSBCameras();
	m_cameraPtr = m_gateway->getUSBCamera(0);
	m_cameraPtr->initialize();

	auto sensor = m_cameraPtr->getSensor(0);
	HandlePromise(sensor->setSetting(dl::ISensor::AutoFanMode, 1));
	HandlePromise(sensor->setSetting(dl::ISensor::UseOverscan, 0));

	m_filterWheelPtr = m_cameraPtr->getFW();
	if (m_filterWheelPtr != nullptr)
	{
		m_filterWheelPtr->initialize();
		nFoundCFW = 1;
	}

	setLinked(true);

	m_flipSensors = false;

	return SB_OK;
}

int AlumaX2::CCDisconnect(const bool bShutDownTemp)
{
	X2MutexLocker locker(GetMutex());

	setLinked(false);

	return SB_OK;
}

int AlumaX2::CCGetChipSize(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nXBin, const int& nYBin,
	const bool& bOffChipBinning, int& nW, int& nH, int& nReadOut)
{
	X2MutexLocker locker(GetMutex());

	const auto sensorInfo = m_cameraPtr->getSensor(ConvertCCDtoSensorId(CCD))->getInfo();
	nW = static_cast<int>(sensorInfo.pixelsX / nXBin);
	nH = static_cast<int>(sensorInfo.pixelsY / nYBin);

	if (CCD == enumWhichCCD::CCD_IMAGER)
	{
		m_imagerBinX = nXBin;
		m_imagerBinY = nYBin;
	}
	else if (CCD == enumWhichCCD::CCD_GUIDER)
	{
		m_guiderBinX = nXBin;
		m_guiderBinY = nYBin;
	}


	return SB_OK;
}

int AlumaX2::CCGetNumBins(const enumCameraIndex& Camera, const enumWhichCCD& CCD, int& nNumBins)
{
	X2MutexLocker locker(GetMutex());

	if (m_cameraPtr == nullptr)
		return ERR_NOLINK;

	const auto sensorInfo = m_cameraPtr->getSensor(ConvertCCDtoSensorId(CCD))->getInfo();
	nNumBins = static_cast<int>(sensorInfo.maxBinX);

	return SB_OK;
}

int AlumaX2::CCGetBinSizeFromIndex(const enumCameraIndex & Camera, const enumWhichCCD & CCD, const int& nIndex,
	long& nBincx, long& nBincy)
{
	X2MutexLocker locker(GetMutex());

	switch (nIndex)
	{
	case 0:		nBincx = nBincy = 1; break;
	case 1:		nBincx = nBincy = 2; break;
	case 2:		nBincx = nBincy = 3; break;
	case 3:		nBincx = nBincy = 4; break;
	case 4:		nBincx = nBincy = 5; break;
	default:	nBincx = nBincy = 1; break;
	}

	return SB_OK;
}

int AlumaX2::CCSetBinnedSubFrame(const enumCameraIndex & Camera, const enumWhichCCD & CCD, const int& nLeft,
	const int& nTop, const int& nRight, const int& nBottom)
{
	X2MutexLocker locker(GetMutex());

	//const auto sensor = m_cameraPtr->getSensor(ConvertCCDtoSensorId(CCD));
	//const auto sensorInfo = sensor->getInfo();

	//auto width = static_cast<int>(sensorInfo.pixelsX - 1) - (nLeft + nRight);
	//auto height = static_cast<int>(sensorInfo.pixelsY - 1) - (nTop + nBottom);

	//if (width == 0)
	//{
	//	width = sensorInfo.pixelsX;
	//}

	//if (height == 0)
	//{
	//	height = sensorInfo.pixelsY;
	//}

	//const dl::TSubframe subFrame{ nTop, nLeft, width, height, m_binX, m_binY };

	//return HandlePromise(sensor->setSubframe(subFrame));

	return SB_OK;
}

void AlumaX2::CCMakeExposureState(int* pnState, enumCameraIndex Cam, int nXBin, int nYBin, int abg, bool bRapidReadout)
{

}

int AlumaX2::CCStartExposure(const enumCameraIndex & Cam, const enumWhichCCD CCD, const double& dTime,
	enumPictureType Type, const int& nABGState, const bool& bLeaveShutterAlone)
{
	X2MutexLocker locker(GetMutex());

	auto isLightFrame = true;

	switch (Type)
	{
	case PT_FLAT:
	case PT_LIGHT:
		isLightFrame = true;
		break;
	case PT_DARK:
	case PT_AUTODARK:
	case PT_BIAS:
		isLightFrame = false;
		break;
	default:
		return ERR_CMDFAILED;
	}

	dl::TExposureOptions options{};
	options.duration = static_cast<float>(dTime);
	options.binX = CCD == enumWhichCCD::CCD_IMAGER ? m_imagerBinX : m_guiderBinX;
	options.binY = CCD == enumWhichCCD::CCD_IMAGER ? m_imagerBinY : m_guiderBinY;
	options.readoutMode = 0;
	options.isLightFrame = isLightFrame;
	options.useRBIPreflash = false;
	options.useExtTrigger = false;

	return HandlePromise(m_cameraPtr->getSensor(ConvertCCDtoSensorId(CCD))->startExposure(options));
}

int AlumaX2::CCIsExposureComplete(const enumCameraIndex & Cam, const enumWhichCCD CCD, bool* pbComplete,
	unsigned* pStatus)
{
	X2MutexLocker locker(GetMutex());

	dl::ICamera::Status status;
	const auto result = GetCameraStatus(status);

	if (result != SB_OK)
		return result;

	const auto sensorStatus = (ConvertCCDtoSensorId(CCD) == 0) ? status.mainSensorState : status.extSensorState;

	*pbComplete = sensorStatus == dl::ISensor::ReadyToDownload;

	return result;
}

int AlumaX2::CCEndExposure(const enumCameraIndex & Cam, const enumWhichCCD CCD, const bool& bWasAborted,
	const bool& bLeaveShutterAlone)
{
	X2MutexLocker locker(GetMutex());

	if (bWasAborted)
	{
		return HandlePromise(m_cameraPtr->getSensor(ConvertCCDtoSensorId(CCD))->abortExposure());
	}

	const auto sensor = m_cameraPtr->getSensor(ConvertCCDtoSensorId(CCD));
	const auto promise = sensor->startDownload();
	while (!IsTransferCompleted(promise))
	{
		m_sleeper->sleep(5);
	}

	return SB_OK;
}

int AlumaX2::CCReadoutLine(const enumCameraIndex & Cam, const enumWhichCCD & CCD, const int& pixelStart,
	const int& pixelLength, const int& nReadoutMode, unsigned char* pMem)
{
	X2MutexLocker locker(GetMutex());
	return ERR_NOT_IMPL;
}

int AlumaX2::CCDumpLines(const enumCameraIndex & Cam, const enumWhichCCD & CCD, const int& nReadoutMode,
	const unsigned& lines)
{
	X2MutexLocker locker(GetMutex());
	return SB_OK;
}

int AlumaX2::CCReadoutImage(const enumCameraIndex & Cam, const enumWhichCCD & CCD, const int& nWidth, const int& nHeight,
	const int& nMemWidth, unsigned char* pMem)
{
	X2MutexLocker locker(GetMutex());

	const auto sensor = m_cameraPtr->getSensor(ConvertCCDtoSensorId(CCD));

	const auto bufferLength = nWidth * nHeight;
	const auto image = sensor->getImage();
	const auto imageBuffer = image->getBufferData();
	const auto copyLength = sizeof(unsigned short) * bufferLength;

	memmove(pMem, imageBuffer, copyLength);

	return SB_OK;
}

int AlumaX2::CCRegulateTemp(const bool& bOn, const double& dTemp)
{
	X2MutexLocker locker(GetMutex());

	const auto tec = m_cameraPtr->getTEC();
	return HandlePromise(tec->setState(bOn, static_cast<float>(dTemp)));
}

int AlumaX2::CCQueryTemperature(double& dCurTemp, double& dCurPower, char* lpszPower, const int nMaxLen,
	bool& bCurEnabled, double& dCurSetPoint)
{
	X2MutexLocker locker(GetMutex());

	dl::ICamera::Status status;
	const auto result = GetCameraStatus(status);

	if (result != SB_OK)
		return result;

	dCurTemp = status.sensorTemperature;
	dCurPower = status.coolerPower;

	const auto tec = m_cameraPtr->getTEC();
	bCurEnabled = tec->getEnabled();
	dCurSetPoint = tec->getSetpoint();

	return result;
}

int AlumaX2::CCGetRecommendedSetpoint(double& dRecSP)
{
	X2MutexLocker locker(GetMutex());
	return SB_OK;
}

int AlumaX2::CCSetFan(const bool& bOn)
{
	X2MutexLocker locker(GetMutex());
	return SB_OK;
}

int AlumaX2::CCActivateRelays(const int& nXPlus, const int& nXMinus, const int& nYPlus, const int& nYMinus,
	const bool& bSynchronous, const bool& bAbort, const bool& bEndThread)
{
	X2MutexLocker locker(GetMutex());
	return ERR_NOT_IMPL;
}

int AlumaX2::CCPulseOut(unsigned nPulse, bool bAdjust, const enumCameraIndex & Cam)
{
	X2MutexLocker locker(GetMutex());
	return ERR_NOT_IMPL;

}

int AlumaX2::CCSetShutter(bool bOpen)
{
	X2MutexLocker locker(GetMutex());
	return SB_OK;
}

int AlumaX2::CCUpdateClock()
{
	X2MutexLocker locker(GetMutex());
	return SB_OK;
}

int AlumaX2::CCSetImageProps(const enumCameraIndex & Camera, const enumWhichCCD & CCD, const int& nReadOut, void* pImage)
{
	X2MutexLocker locker(GetMutex());
	return SB_OK;
}

int AlumaX2::CCGetFullDynamicRange(const enumCameraIndex & Camera, const enumWhichCCD & CCD, unsigned long& dwDynRg)
{
	X2MutexLocker locker(GetMutex());
	return SB_OK;
}

void AlumaX2::CCBeforeDownload(const enumCameraIndex & Cam, const enumWhichCCD & CCD)
{

}

void AlumaX2::CCAfterDownload(const enumCameraIndex & Cam, const enumWhichCCD & CCD)
{
	X2MutexLocker locker(GetMutex());

	//Wait for the camera driver to cleanup
	m_sleeper->sleep(100);
}

int AlumaX2::CCSetBinnedSubFrame3(const enumCameraIndex & Camera, const enumWhichCCD & CCDOrig, const int& nLeft,
	const int& nTop, const int& nWidth, const int& nHeight)
{
	X2MutexLocker locker(GetMutex());


	const auto sensor = m_cameraPtr->getSensor(ConvertCCDtoSensorId(CCDOrig));
	const auto sensorInfo = sensor->getInfo();

	const dl::TSubframe subFrame
	{ 
		nTop,
		nLeft,
		nWidth,
		nHeight,
		CCDOrig == enumWhichCCD::CCD_IMAGER ? m_imagerBinX : m_guiderBinX,
		CCDOrig == enumWhichCCD::CCD_IMAGER ? m_imagerBinY : m_guiderBinY
	};

	return HandlePromise(sensor->setSubframe(subFrame));

	//auto width = static_cast<int>(sensorInfo.pixelsX - 1) - (nLeft + nRight);
	//auto height = static_cast<int>(sensorInfo.pixelsY - 1) - (nTop + nBottom);

	//if (width == 0)
	//{
	//	width = sensorInfo.pixelsX;
	//}

	//if (height == 0)
	//{
	//	height = sensorInfo.pixelsY;
	//}

	//const dl::TSubframe subFrame{ nTop, nLeft, width, height, m_binX, m_binY };

	//return HandlePromise(sensor->setSubframe(subFrame));
}

//FilterWheelMoveToInterface
int AlumaX2::filterCount(int& nCount)
{
	X2MutexLocker locker(GetMutex());

	nCount = static_cast<int>(m_filterWheelPtr->getSlots());

	return SB_OK;
}

int AlumaX2::startFilterWheelMoveTo(const int& nTargetPosition)
{
	X2MutexLocker locker(GetMutex());

	HandlePromise(m_filterWheelPtr->setPosition(nTargetPosition + 1));

	return SB_OK;
}

int AlumaX2::isCompleteFilterWheelMoveTo(bool& bComplete) const
{
	X2MutexLocker locker(GetMutex());

	const auto result = HandlePromise(m_filterWheelPtr->queryStatus());

	if (result != SB_OK)
		return result;

	bComplete = m_filterWheelPtr->getStatus() == m_filterWheelPtr->FWIdle;

	return SB_OK;
}

int AlumaX2::endFilterWheelMoveTo()
{
	X2MutexLocker locker(GetMutex());
	return SB_OK;
}

int AlumaX2::abortFilterWheelMoveTo()
{
	X2MutexLocker locker(GetMutex());
	return SB_OK;
}

void AlumaX2::embeddedFilterWheelInit(const char* psFilterWheelSelection)
{

}


//Helpers
int AlumaX2::GetCameraStatus(dl::ICamera::Status & status) const
{
	const auto result = HandlePromise(m_cameraPtr->queryStatus());

	if (result != SB_OK)
		return result;

	status = m_cameraPtr->getStatus();
	return result;
}

int AlumaX2::HandlePromise(const dl::IPromisePtr & promise) const
{
	const auto result = promise->wait();
	if (result != dl::IPromise::Complete)
	{
		char buf[512] = { 0 };
		size_t lng = 512;
		promise->getLastError(&(buf[0]), lng);
		promise->release();

		m_logger->out(std::string(&(buf[0]), lng).c_str());
		return ERR_CMDFAILED;
	}
	promise->release();

	return SB_OK;
}

bool AlumaX2::IsTransferCompleted(const dl::IPromisePtr & promise)
{
	const auto status = promise->getStatus();
	if (status == dl::IPromise::Complete)
	{
		promise->release();
		return true;
	}

	if (status == dl::IPromise::Error)
	{
		char buf[512] = { 0 };
		size_t lng = 512;
		promise->getLastError(&(buf[0]), lng);
		promise->release();
		throw std::logic_error(&(buf[0]));
	}

	return false;
}
unsigned int AlumaX2::ConvertCCDtoSensorId(const enumWhichCCD & CCD) const
{
	return GetFlipSensors() ? (CCD == enumWhichCCD::CCD_GUIDER ? 0 : 1) : (CCD == enumWhichCCD::CCD_IMAGER ? 0 : 1);
}
