#pragma once

#include <cameradriverinterface.h>
#include <filterwheelmovetointerface.h>
#include <sleeperinterface.h>

#include <dlapi.h>

#include <memory>


class SerXInterface;
class TheSkyXFacadeForDriversInterface;
class SleeperInterface;
class BasicIniUtilInterface;
class LoggerInterface;
class MutexInterface;
class TickCountInterface;


class AlumaX2 : public CameraDriverInterface, public FilterWheelMoveToInterface
{

public:
	AlumaX2(AlumaX2 const&) = delete;
	void operator=(AlumaX2 const&) = delete;

	static AlumaX2* GetInstance(SleeperInterface* pSleeper, MutexInterface* pIOMutex);

private:
	AlumaX2(SleeperInterface* pSleeper, MutexInterface* pIOMutex);
	~AlumaX2();

public:

	//DriverRootInterface
	int queryAbstraction(const char* pszName, void** ppVal) override;

	//DriverInfoInterface
	void driverInfoDetailedInfo(BasicStringInterface& str) const override;
	double driverInfoVersion() const override;

	//HardwareInfoInterface
	void deviceInfoNameShort(BasicStringInterface& str) const override;
	void deviceInfoNameLong(BasicStringInterface& str) const override;
	void deviceInfoDetailedDescription(BasicStringInterface& str) const override;
	void deviceInfoFirmwareVersion(BasicStringInterface& str) override;
	void deviceInfoModel(BasicStringInterface& str) override;

	//CameraDriverInterface
	ReadOutMode readoutMode() override { return CameraDriverInterface::rm_Image; }
	int CCSettings(const enumCameraIndex& Camera, const enumWhichCCD& CCD) override;
	int CCEstablishLink(enumLPTPort portLPT, const enumWhichCCD& CCD, enumCameraIndex DesiredCamera, enumCameraIndex& CameraFound, const int nDesiredCFW, int& nFoundCFW) override;
	int CCDisconnect(const bool bShutDownTemp) override;
	int CCGetChipSize(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nXBin, const int& nYBin, const bool& bOffChipBinning, int& nW, int& nH, int& nReadOut) override;
	int CCGetNumBins(const enumCameraIndex& Camera, const enumWhichCCD& CCD, int& nNumBins) override;
	int CCGetBinSizeFromIndex(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nIndex, long& nBincx, long& nBincy) override;
	int CCSetBinnedSubFrame(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nLeft, const int& nTop, const int& nRight, const int& nBottom) override;
	void CCMakeExposureState(int* pnState, enumCameraIndex Cam, int nXBin, int nYBin, int abg, bool bRapidReadout) override;
	int CCStartExposure(const enumCameraIndex& Cam, const enumWhichCCD CCD, const double& dTime, enumPictureType Type, const int& nABGState, const bool& bLeaveShutterAlone) override;
	int CCIsExposureComplete(const enumCameraIndex& Cam, const enumWhichCCD CCD, bool* pbComplete, unsigned int* pStatus) override;
	int CCEndExposure(const enumCameraIndex& Cam, const enumWhichCCD CCD, const bool& bWasAborted, const bool& bLeaveShutterAlone) override;
	int CCReadoutLine(const enumCameraIndex& Cam, const enumWhichCCD& CCD, const int& pixelStart, const int& pixelLength, const int& nReadoutMode, unsigned char* pMem) override;
	int CCDumpLines(const enumCameraIndex& Cam, const enumWhichCCD& CCD, const int& nReadoutMode, const unsigned int& lines) override;
	int CCReadoutImage(const enumCameraIndex& Cam, const enumWhichCCD& CCD, const int& nWidth, const int& nHeight, const int& nMemWidth, unsigned char* pMem) override;
	int CCRegulateTemp(const bool& bOn, const double& dTemp) override;
	int CCQueryTemperature(double& dCurTemp, double& dCurPower, char* lpszPower, const int nMaxLen, bool& bCurEnabled, double& dCurSetPoint) override;
	int	CCGetRecommendedSetpoint(double& dRecSP) override;
	int	CCSetFan(const bool& bOn) override;
	int CCActivateRelays(const int& nXPlus, const int& nXMinus, const int& nYPlus, const int& nYMinus, const bool& bSynchronous, const bool& bAbort, const bool& bEndThread) override;
	int CCPulseOut(unsigned int nPulse, bool bAdjust, const enumCameraIndex& Cam) override;
	int CCSetShutter(bool bOpen) override;
	int CCUpdateClock(void) override;
	int CCSetImageProps(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nReadOut, void* pImage) override;
	int CCGetFullDynamicRange(const enumCameraIndex& Camera, const enumWhichCCD& CCD, unsigned long& dwDynRg) override;
	void CCBeforeDownload(const enumCameraIndex& Cam, const enumWhichCCD& CCD) override;
	void CCAfterDownload(const enumCameraIndex& Cam, const enumWhichCCD& CCD) override;

	//FilterWheelMoveToInterface
	int filterCount(int& nCount) override;
	int startFilterWheelMoveTo(const int& nTargetPosition)	override;
	int isCompleteFilterWheelMoveTo(bool& bComplete) const override;
	int endFilterWheelMoveTo() override;
	int abortFilterWheelMoveTo() override;
	void embeddedFilterWheelInit(const char* psFilterWheelSelection) override;


private:
	SleeperInterface* m_sleeper;
	MutexInterface* m_mutex;

	std::shared_ptr<dl::IGateway> m_gateway;
	dl::ICameraPtr m_cameraPtr;
	dl::IFWPtr m_filterWheelPtr;

	bool m_flipSensors{ false };

	unsigned char m_binX{ 1 };
	unsigned char m_binY{ 1 };

	MutexInterface* GetMutex() const { return m_mutex; };
	bool GetFlipSensors() const { return m_flipSensors; };

	dl::ICamera::Status GetCameraStatus() const;
	static void HandlePromise(const dl::IPromisePtr& promise);
	static bool IsTransferCompleted(const dl::IPromisePtr& promise);
	unsigned int ConvertCCDtoSensorId(const enumWhichCCD& CCD) const;
};

