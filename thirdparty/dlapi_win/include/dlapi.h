#ifndef _DL_H
#define _DL_H

/*! 
 * \file dlapi.h
 * \brief Main header file for DLAPI, included in client applications calling the library.
 * \author Adam Robichaud
 *
 * Copyright (c) Diffraction Limited, 2019
 *
 */

#include <stddef.h>

// Windows DLL Exports
#if defined(_WIN32) || defined(_WIN64)
	// Dynamic Linking
#	ifndef DL_STATICLIB
#		ifdef DL_EXPORTS
#			define DL_API extern "C" __declspec(dllexport)
#		else
#			define DL_API extern "C" __declspec(dllimport)
#		endif

#		define MYCDECL __cdecl

	// Static Linking
#	else
#		define DL_API
#		define MYCDECL
#	endif

#else 

// Define macros for Non-Windows systems

#define DL_API
#define DL_GUI_API
#define MYCDECL

#endif

/*!
 * \brief Top-level namespace containing all of DLAPI.
 *
 * The dl namespace contains all the interface and utility definitions you will need while
 * controlling a DLAPI compliant camera. Classes prefixed with a capital "I" (such as IGateway,
 * ICamera, or IImage) represent abstract interface classes that are instantiated by DLAPI. 
 * They are grouped by function:
 *
 *  - Discovery/management of devices (IGateway)
 *  - Camera/peripheral control (ICamera, ISensor, ITEC, IFW, and IAO)
 *  - Image buffer/metadata storage/transport (IImage)
 *  - Asynchronous command monitoring/error reporting (IPromise)
 *
 * Interface classes all have convenience pointer typedefs (e.g. ICameraPtr is ICamera*), 
 * to ease readability.
 *
 * In addition to the interface classes, there are a number of different parametrization and 
 * data structures both contained within interfaces (such as the ICamera::Info structure), and
 * without (such as the TExposureOptions structure). These structures are used to transport
 * large data structures to and from the API, and provides some degree of structure and modularity
 * to the API. 
 * 
 * 
 * 
 */
namespace dl
{
	/*!
	 * \enum EStatusCode
	 *
	 * \brief Status codes returned by a camera with every message response 
	 */
	enum EStatusCode
	{
		Success = 0,				//!< Device reports command executed successfully, and without error

		// client specific errors ...
		ClientError = 0x01,			//!< Start of client-side error codes
		InvalidSoh = ClientError,   //!< Invalid message start-of-header byte
		InvalidHeaderType,			//!< Invalid header typeid supplied in request
		InvalidBodyType,			//!< Invalid body typeid supplied in request
		InvalidFooterType,			//!< Invalid footer typeid supplied in request
		InvalidSensorId,			//!< Invalid sensor ID number supplied in request
		InvalidMessageLength,		//!< Invalid message length supplied in request
		InvalidParameter,			//!< Invalid parameter passed in request body

		// device specific errors ... 
		DeviceError = 0x80,					//!< Start of device-side error codes
		ExposureInProgress = DeviceError,	//!< Device has an exposure in progress and cannot comply
		ReadoutInProgress,					//!< Device has a frame readout in progress and cannot comply

		// Book keeping
		InvalidStatus = 0xff //!< Unknown status response from camera
	};

	/*!
	 * \enum EPulseGuideDirection
	 * \brief Bases for pulse guiding
	 * \see ICamera::PulseGuide
	 */
	enum EPulseGuideDirection
	{
		North = 0,	//!< North (+ declination/altitude)
		South = 1,	//!< South (- declination/altitude)
		East = 2,	//!< East (+ right ascension/azimuth)
		West = 3,	//!< West (- right ascension/azimuth)
	};
	
	/*!
	* \enum FilterType
	* \brief Sensor's Filter typeid
	*/
	enum FilterType
	{
		Monochrome,					//!< Monochrome filter on sensor.
		Color,						//!< Bayer color filter on sensor.
		SparseColor,				//!< TrueSense Sparse Color filter on sensor.
		InvalidFilterType = 0xff,	//!< Filter type is invalid or uninitialized.
	};

	/*!
	 * \enum EFrameType
	 * \brief Sensor's readout frame typeid
	 */
	enum EFrameType
	{
		Progressive,			 //!< Progressive, full-frame sensor.
		Interline,				 //!< Interline sensor.
		BayerInterline,			 //!< Bayer/SparseColor interline sensors.
		InvalidFrameType = 0xff, //!< Frame type is invalid or uninitialized.
	};

	/*! 
	 * \enum EShutterStatus
	 * \brief Status of sensor's shutter
	 */
	enum EShutterStatus
	{
		ShutterClosed = 0,  //!< Shutter is closed.
		ShutterOpen,	    //!< Shutter is open.
		ShutterClosing,	    //!< Shutter is moving to closed state.
		ShutterOpening,	    //!< Shutter is moving to opened state.
		ShutterRolling,     //!< for future use in Aluma Pro.
		ShutterExercising,  //!< Shutter is in exercising state, more details coming.
		ShutterError,       //!< for future use in Aluma Pro

		ShutterUninitialized = 0xFF //!< Shutter state is uninitialized
	};

	/*!
	* \enum ENetworkMode
	* \brief Network Connection type/security for WiFi operation
	*/
	enum ENetworkMode
	{
		AdHoc = 0,	//!< Use an ad-hoc network. More details coming.
		Open = 1,	//!< Connect to an open network specified via ICamera::setNetworkSettings(), retrieved using ICamera::getNetworkSettings().
		WEP = 2,	//!< Connect to a WEP secured network specified via ICamera::setNetworkSettings(), retrieved using ICamera::getNetworkSettings().
		WPA = 3,	//!< Connect to a WPA 1 secured network specified via ICamera::setNetworkSettings(), retrieved using ICamera::getNetworkSettings().
		WPA2 = 4,	//!< Connect to a WPA 2 secured network specified via ICamera::setNetworkSettings(), retrieved using ICamera::getNetworkSettings().
		InvalidNetworkMode = 0xff	//!< Network mode is either invalid, or uninitialized.
	};

	/*!
	* \enum EEndpointType
	* \brief Camera's connection endpoint type (networked, USB, etc)
	*/
	enum EEndpointType {
		Net = 1, //!< A UDP over Ethernet/WiFi endpoint.
		USB = 2, //!< A USB endpoint.

		LastEndpointType,		 //!< A convenience definition for determining the last endpoint type.
		FirstEndpointType = Net, //!< A convenience definition for determining the first endpoint type.
		InvalidEndpoint = 0xff,	 //!< The endpoint is either invalid, or uninitialized.
	};

	/*!
	 * \brief Contains Error flags/masks for a variety of peripherals/devices, as reported by the camera.
	 */
	namespace ErrorFlags {

		/*!
		* \brief Contains TEC error flags as reported by the camera in ICamera::Status::flag
		*/
		namespace TEC {
			static const int ThermistorErrorFlagMask = 0x00000003;		//<! Mask for thermistor error flags. \see ICamera::Status::flag
			static const int SensorThermistorErrorFlag = 0x00000001;	//<! Error flag for sensor thermistor. \see ICamera::Status::flag
			static const int HeatSinkThermistorErrorFlag = 0x00000002;	//<! Error flag for heatsink thermistor. \see ICamera::Status::flag
		}
		
		/*!
		 * \brief Contains AO error flags for internal use.
		 */
		namespace AO {
			static const unsigned char statusMask = 0x0f;		//!< Mask for the AO error flags.
			static const unsigned char busy = 0x01;				//!< AO is busy.
			static const unsigned char badCalibration = 0x02;	//!< AO is not calibrated
			static const unsigned char reserved = 0x04;			//!< Reserved for future use
			static const unsigned char badCommand = 0x08;		//!< Bad Command
		}
	}

	/*!

	* \brief Status flags for the pulse guide port
	* 
	*/
	namespace PulseGuideStatus
	{
		static const unsigned char c_xBusy = 0x01; //!< This bit is set to high when the x-axis guider port is busy. \see ICamera::Status::pulseGuideStatus
		static const unsigned char c_yBusy = 0x02; //!< This bit is set to high when the y-axis guider port is busy. \see ICamera::Status::pulseGuideStatus
	}

	/*!
	 * \brief A collection of metadata associated with an image
	 * \see IImage, ISensor::getImage()
	 *
	 * Structure containing various metadata of the image buffer. These values represent the image dimensions and sensor settings the camera used to take the 
	 * image, and may differ from the parametrization passed in to ISensor::setSubframe() and ISensor::startExposure() depending on things like hardware adjustment
	 * of eGain during binned exposures, constraints due to minimum exposure durations/precision, etc.
	 */
	struct TImageMetadata
	{
		unsigned int width;     //!< Width of the image frame in pixels.
		unsigned int height;    //!< Height of the image frame in pixels.
		unsigned int offsetX;   //!< The image's x-offset from the sensor's origin in pixels.
		unsigned int offsetY;   //!< The image's y-offset from the sensor's origin in pixels.
		unsigned int binX;	    //!< The image's binning in the x-direction (in pixels).
		unsigned int binY;	    //!< The image's binning in the y-direction (in pixels).
		float eGain;		    //!< The image's binning-dependent eGain (in e-/ADU)
		float exposureDuration; //!< The image's exposure duration in seconds. May differ from the requested duration due to camera exposure limitations \see ISensor::Info::minExposureDuration, ISensor::Info::exposurePrecision.
	};

	/*!
	* \brief A collection of exposure parameters required for starting an exposure
	* \see ICamera::startExposure()
	*
	* Structure containing all contextual information for starting an exposure.
	*
	*/
	struct TExposureOptions
	{
		float duration;				//!< Duration of the exposure in seconds.
		unsigned char binX;			//!< Binning of the exposure in the x-direction in pixels.
		unsigned char binY;			//!< Binning of the exposure in the y-direction in pixels.
		unsigned int readoutMode;	//!< The index of the readout mode to use in the exposure. This is the index of the readout mode chosen from ISensor::getReadoutModes(). \see ISensor::getReadoutModes()
		bool isLightFrame;			//!< true if the exposure is a light frame, false otherwise.
		bool useRBIPreflash;		//!< true to perform an RBI Preflash operation (if the camera supports it) before the exposure. Preflash duration and flush count are set via the ISensor::Settings structure. \see ISensor::Setting::RBIPreflashDuration, ISensor::Setting::RBIPreflashFlushCount, ISensor::Settings::RBIPreflashSettings
		bool useExtTrigger;			//!< true to wait for an external trigger before executing the exposure. Consult your camera's user manual for more details on external trigger operation.
	};

	/*!
	* \brief A collection of frame parameters, required for specifying the dimensions of an image.
	* \see ICamera::setSubframe()
	*
	* Structure containing all relevant subframe information.
	*
	*/
	struct TSubframe
	{
		int top;    //!< The frame's y-offset from the sensor's origin in pixels.
		int left;   //!< The frame's x-offset from the sensor's origin in pixels.
		int width;  //!< The frame's width in pixels.
		int height; //!< The frame's height in pixels.
		int binX;   //!< The frame's x-binning in pixels.
		int binY;   //!< The frame's y-binning in pixels.
	};


	/*!
	* \brief A collection of wireless network configuration settings
	* \see ICamera::queryNetworkSettings(), ICamera::setNetworkSettings(), ICamera::getNetworkSettings()
	*
	* A structure containing network connection details. You can use this to set up a camera's WiFi connection;
	* though we don't strictly advise this, since it's possible do this via the free DL Config utility, available
	* from https://diffractionlimited.com/, and is a part of the DLAPI SDK.
	*
	* If you need to implement this control in your software, you can obtain the network settings structure via
	* ICamera::queryNetworkSettings() and ICamera::getNetworkSettings(), and set it via ICamera::setNetworkSettings().
	*
	*/
	struct TNetworkSettings
	{
		char ssid[32];					//!< String containing the SSID of the camera's network settings.
		char passphrase[64];			//!< String containing the passphrase of the camera's network settings.
		unsigned int ssidLength;		//!< Length of the SSID string.
		unsigned int passphraseLength;	//!< Length of the passphrase string.
		ENetworkMode mode;				//!< The networking mode. Changing this value requires a reboot of the camera. \see ENetworkMode
		bool isUsaCanadaMode;			//!< true to enable USA/Canada mode. (More details to come)
		bool isPassphraseHex;			//!< true to treat the passphrase as hex pairs, false to treat it as an ASCII phrase.
	};

	/*!
	* \brief Network connection detail structure
	*
	* This structure contains the endpoint information necessary to connect to a camera
	* via the API's internal indexing system. Let's say you've selected a camera in one
	* session of your software, and you'd like to ensure it is used in subsequent sessions
	* even if the camera is unplugged/moved from one connection method to another. This
	* can be achieved using the ICamera::Info::serialNumber of the camera in question.
	*
	* An example on how to use camera serial numbers to fetch pointers to ICamera instances
	* via the IGateway object:
	*
	*  ```c++
	*  struct Settings
	*  {
	*    size_t cameraSerialNumber;
	*    void writeToFile();
	*    void loadFromFile();
	*  };
	*
	*  using namespace dl;
	*
	*  void saveCamera(ICameraPtr pCamera)
	*  {
	*    auto info = pCamera->getInfo();
	*    Settings settings;
	*    settings.cameraSerialNumber = info.serialNumber;
	*    settings.writeToFile();
	*  }
	*
	*  ICameraPtr loadCamera(IGatewayPtr pGateway)
	*  {
	*    Settings settings;
	*    settings.loadFromFile();
	*    pGateway->queryUSBCameras();
	*    pGateway->queryNetCameras();
	*    TConnectionDetails details;
	*    details.serialNumber = settings.cameraSerialNumber;
	*    return pGateway->getCamera(details);
	*  }
	*  ```
	*
	* You could also use the IGateway::getCameraConnectionDetails() function to retrieve the TConnectionDetails of a given
	* camera, extract TConnectionDetails::serialNumber and TConnectionDetails::endpointType, and load the camera with the
	* IGateway::getUSBCamera() or IGateway::getNetCamera() functions\--but that method isn't much faster than the one
	* shown above.
	*
	*/
	struct TConnectionDetails
	{
		unsigned int serialNumber;  //!< The ICamera::Info::serialNumber to query\--not the human readable serial number string.
		EEndpointType endpointType;	//!< Type of endpoint to connect to. Not needed in IGateway::GetCameraBySerial() \see EEndpointType
		size_t index;				//!< The Internal index of the camera to connect to (depends on EEndpointType). Not needed in IGateway::GetCameraBySerial()

		TConnectionDetails() : serialNumber(0), endpointType(InvalidEndpoint), index(0) {} //!< Default constructor for the struct.
	};


	/*!
	 * \class IImage
	 * \brief Interface to an DLAPI Image buffer.
	 *
	 * This interface provides the user access to an image buffer, along with all relevant metadata associated with said image. 
	 * The maximum buffer size supported by interface is platform dependent, and is therefore capped at UINT_MAX 16-bit pixels. 
	 * The buffer contains the camera's pixel data. The default arrangement of this buffer is a 16-byte x-index by y-index array 
	 * where the index of the buffer is equal to:
	 *
	 * \f$i = x + ( y \times w )\f$
	 *
	 * where \f$i\f$ is the index of buffer, \f$x\f$ is the index of a pixel on the x-axis of the image's subframe, \f$y\f$ is the index 
	 * of a pixel on the y-axis of the image's subframe, and \f$w\f$ is the width of the image's subframe in pixels. Sensors with non-standard
	 * pixel mappings (CMOS sensors), bit-depths, or multiple layers (SCMOS sensors) are not currently supported. 
	 *
	 * Image buffers are stored in DLAPI memory, and their lifecycle is managed entirely by DLAPI's internals. Images must therefore be copied to the 
	 * calling application's memory space at the earliest convenience to avoid that image buffer's deletion at the hands of another call to 
	 * ISensor::startDownload(). This can be achieved quickly using the `<cstring>` function `memmove()`. 
	 *
	 * e.g.
	 *
	 * ```c++
	 * #include <dlapi.h>
	 * #include <cstring>
	 * 
	 * void copyImageBuffer(dl::IImagePtr pImg, unsigned short *& pBuffer, size_t & bufferLength)
	 * {
	 *   bufferLength = pImg->getBufferLength();
	 *   pBuffer = new unsigned short[bufferLength];
	 *   memmove(pBuffer, pImg->getBufferData(), sizeof(unsigned short) * bufferLength);
	 * }
	 * ```
	 *
	 * Where the user passes in a pointer to an IImage object, retrieved via a call to ISensor::getImage() following a successful image transfer,
	 * a reference to a pointer of type unsigned short (which will receive the image buffer), and a reference to a size_t (which will receive the
	 * size of the image buffer). The calling function would then take ownership of the new image buffer.
	 *
	 */
	class IImage
	{
	public:
		/**
		 * \brief Returns a pointer to the raw data buffer of the image.
		 * \return unsigned short * pointer to the image buffer.
		 *
		 * Returns a pointer to the image's data buffer.
		 * 
		 */
		virtual unsigned short *	getBufferData() const = 0;

		/**
		 * \brief Returns the length in pixels of the raw data buffer of the image.
		 * \return the length of the data buffer in pixels.
		 * 
		 * Returns the length of the image buffer (width * height) in pixels, not bytes. 
		 *
		 */
		virtual unsigned int		getBufferLength() const = 0;

		/**
		 * \brief Returns the image metadata structure associated with the image.
		 * \return the image's metadata structure.
		 * \see TImageMetadata
		 * 
		 * Returns the image's metadata structure to the user.
		 *
		 */
		virtual TImageMetadata		getMetadata() const = 0;
	};

	typedef IImage* IImagePtr; //!< Convenience typedef for IImage pointers.
	
	/*!
	 *  \brief Promise interface class
	 * 
	 *  Device/Peripheral interface calls which complete asynchronously (e.g. query and set methods) will return a 
	 *  pointer to an IPromise object. Users are responsible for releasing IPromise pointers when they are done with 
	 *  them so that the API can manage their memory internally. A failure to release promises will result in memory 
	 *  leakage.
	 *
	 *  An example of how to use a promise in a blocking fashion:
	 *
	 *  ```c++
	 *  ICamera::Info getInfo(ICameraPtr pCamera)
	 *  {
	 *    IPromisePtr pPromise = pCamera->queryInfo();
	 *    IPromise::Status result = pPromise->wait();
	 *    if (result != IPromise::Complete)
	 *    {
	 *      char buf[512] = {0};
	 *      size_t lng = 512;
	 *      pPromise->getLastError(&(buf[0]), lng);
	 *      pPromise->release(); // Mandatory!
	 *      throw std::logic_error(&(buf[0]), lng);
	 *    }
	 *    pPromise->release(); // Mandatory!
	 *    return pCamera->getInfo();
	 *  }
	 *  ```
	 *
	 *  An example of how to use a promise in a non-blocking fashion, executing a callback function while we wait:
	 *  ```c++
	 *  bool isPromiseDone(IPromisePtr pPromise)
	 *  {
	 *    IPromise::Status result = pProimse->getStatus();
	 *    if ( result == IPromise::Error )
	 *    {
	 *      char buf[512] = {0};
	 *      size_t lng = 512;
	 *      pPromise->getLastError(&(buf[0]), lng);
	 *		pPromise->release();
	 *		throw std::logic_error(&(buf[0]), lng);
	 *    }
	 *    else if ( result == IPromise::Complete )
	 *    {
	 *      pPromise->release();
	 *    }
	 *    return result == IPromise::Complete;
	 *  }
	 *
	 *  ICamera::Info getInfo(ICameraPtr pCamera, std::function<void()> fCallback)
	 *  {
	 *    IPromisePtr pPromise = pCamera->queryInfo();
	 *    try
	 *    {
	 *	    while (!isPromiseDone())
	 *      {
	 *        // Execute fCallback while we wait
	 *        fCallback();
	 *      }
	 *    }
	 *    catch (std::exception &ex)
	 *    {
	 *      // Log any error messages
	 *    }
	 *
	 *    return pCamera->getInfo();
	 *  }
	 *  ```
	 *  
	 *  IPromise is also thread-safe, and thus could be monitored on a worker thread.
	 *  You should try to avoid making multiple calls before promises have the chance to 
	 *  return, as the internal queuing system can get overloaded, and bog-down IO to the
	 *  camera.
	 */
	class IPromise
	{
	public:

		/*!
		 * \enum IPromise::Status
		 * \brief list of supported status code from Promise objects
		 */
		enum Status
		{
			Idle,	   //!< Promise has not yet begun execution
			Executing, //!< Promise is currently being executed
			Complete,  //!< Promise completed successfully
			Error, 	   //!< Promise completed with an error

			InvalidFuture = 0xff	//!< Promise is uninstantiated
		};

		/*!
		 *  \brief Puts a human readable string into the supplied buffer containing the last known error for command the IPromise is tracking.
		 *  \param buffer a pointer to an array of characters that will receive the last error.
		 *  \param size_t a reference to a variable which passes in the size of the supplied buffer, and updates with the length of the string returned.
		 *
		 * This function retrieves the last error stored in this promise object. It is best to call this function only when getStatus(), or wait() returns
		 * IPromise::Error. All error strings are English text only. No localization is done within DLAPI.
		 *
		 */
		virtual void   getLastError(char * buffer, size_t& bufferSize) const = 0;
		
		/*!
		 *  \brief Retrieve the status of the command the IPromise is tracking.
		 *  \return IPromise::Status
		 *  \see IPromise::Status
		 *
		 *  A non-blocking call to obtain the status of the promise's operation.
		 *
		 */
		virtual Status getStatus() const = 0;

		/*!
		 *  \brief Wait (blocking) until the IPromise either completes, or returns an error (timing out at 10 seconds since the last known update).
		 *  \return IPromise::Status
		 *
		 *  Blocking function waits for the operation to complete with a result (Success, or Error), and returns that result.
		 *
		 */
		virtual Status wait() = 0;

		/*!
		 *  \brief Flags the promise for release within DLAPI.
		 *
		 *  Signals to the API that the user is done using the promise, and it can be handled by garbage collection. This function
		 *  *must* be called on *all* promises returned from the API, no exceptions. The IPromise object is no longer valid after calling this.
		 *
		 */
		virtual void   release() = 0;
	};

	typedef IPromise* IPromisePtr; //!< Convenience typedef for IPromise pointers.


	/*!
	 *  \brief TEC interface class
	 *
	 *  An interface for Cooler peripherals in a camera. This includes, but is not limited to, thermoelectric coolers. 
	 *  ITEC members are updated via the ICamera::queryStatus() command, and thus have no associated query calls with
	 *  them. 
	 *
	 */
	class ITEC 
	{
	public:
		/*!
		 *  \brief true if TEC has been enabled, false otherwise.
		 *  \return bool true if cooler is enabled, false otherwise.
		 *  
		 *  Check to see whether the TEC is enabled/disabled (i.e. the cooler power is on/off). This a buffered value,
		 *  updated via calls to ITEC::setState().
		 *
		 */
		virtual bool  getEnabled() const = 0;

		/*!
		 *  \brief Returns the value of the cooler's setpoint in degrees Celsius
		 *  \return float value of the cooler's setpoint in degrees Celsius.
		 *
		 *  Obtain the cooler's current setpoint in degrees Celsius. This value is a buffered value, updated via calls 
		 *  to ITEC:setState().
		 *
		 */
		virtual float getSetpoint() const = 0;

		/*!
		 *  \brief Returns the current cooler power in percent of max power draw.
		 *  \return float cooler power in percent of max power draw.
		 *
		 *  Returns a value between 0 - 100% representing the load on the TEC cooler, as a fraction of the max power draw.
		 *  This value is updated via calls to ICamera::queryStatus().
		 *
		 */
		virtual float getCoolerPower() const = 0;
	
		/*!
		 *  \brief Return the value of the sensor's thermopile in degrees Celsius.
		 *  \return float value of the sensor's thermopile in degrees Celsius.
		 *  \see ICamera::queryState()
		 *
		 *  Obtain the sensor's current temperature in degrees Celsius. Updated via calls to ICamera::queryState().
		 *
		 */
		virtual float getSensorThermopileTemperature() const = 0;
		
		/*!
		 *  \brief Return the value of the heatsink's thermopile in degrees Celsius.
		 *  \return float value of the heatsink's thermopile in degrees Celsius.
		 *  \see ICamera::queryState()
		 *
		 *  Obtain the sensor's current temperature in degrees Celsius. Updated via calls to ICamera::queryState().
		 */
		virtual float getHeatSinkThermopileTemperature() const = 0;

		/*!
		*  \brief Toggles the TEC's operation status, and updates the TEC's setpoint.
		*  \param enable true to enable the TEC, false to disable it.
		*  \param setpoint the desired setpoint of the TEC in degrees Celsius.
		*  \return IPromisePtr a promise pointer for the asynchronous operation.
		*  \see IPromisePtr
		*
		*  Check to see whether the TEC is enabled/disabled (i.e. the cooler power is on/off).
		*/
		virtual IPromisePtr setState(bool enable, float setpoint) = 0;
	};

	typedef ITEC* ITECPtr; //!< Convenience typedef for ITEC pointers.


	/*!
	 *  \brief AO Interface Class
	 *
	 *  An interface to Adaptive Optics peripherals connected to DLAPI cameras.
	 *
	 */
	class IAO {
	public:
		
		/*! 
		 * \enum IAO::Status
		 * \brief list of supported status codes from AO devices
		 */
		enum Status {
			Idle,	//!< Peripheral is idle.
			Busy,	//!< Peripheral is busy.
			Error,	//!< Peripheral is in an error state.
			InvalidAOStatus = 0xff, //!< Peripheral is uninitialized.
		};

		/*! 
		 * \enum IAO::Model
		 * \brief list of supported AO models
		 */
		enum Model
		{
			UnknownAOModel = 0,    //!< AO Model could not be ascertained.
			AOX,				   //!< AO-X model of adaptive optics
			AO8,				   //!< AO-8 model of adaptive optics
			InvalidAOModel = 0xff, //!< AO model was not initialized.
		};

		/*!
		 *  \brief Initialize the AO and retrieve basic model information.
		 *  \return IPromisePtr a promise to track asynchronous completion of the operation.
		 *  \see IPromise
		 *
		 *  Performs initialization of the AO unit, including obtaining the model of the unit (if it is attached, and available).
		 *
		 */
		virtual IPromisePtr initialize() = 0;

		/*!
		 *  \brief Query the status of the AO device.
		 *  \return IPromisePtr a promise to track asynchronous completion of the operation.
		 *  \see IPromise, IAO::getPosition(), IAO::getStatus()
		 * 
		 *  Query the camera for the status of the AO unit, and its position. Retrieve position on IPromise::Complete with IAO::getPosition(), and IAO::getStatus().
		 *
		 */
		virtual IPromisePtr queryStatus() = 0;

		/*!
		 *  \brief Set the Tip/Tilt position of the AO.
		 *  \return IPromisePtr a promise to track asynchronous completion of the operation.
		 *  \see IPromise
		 *
		 *  Tells the AO to GOTO a given tip/tilt. The limits of tip and tilt are given by IAO::getLimits(), nominally [0, 4095] inclusive 
		 *  on both axes. Attempting to set the position of the AO to any values outside that range will return a EStatusCode::InvalidParameter 
		 *  error when the associated IPromise completes. If the AO is currently moving when this call is made, it will also return a 
		 *  EStatusCode::DeviceBusy error when the IPromise completes.
		 *
		 */
		virtual IPromisePtr setPosition(short tip, short tilt) = 0;

		/*! 
		 *  \brief Return the buffered status of the AO device.
		 *  \return IAO::Status status of the AO peripheral.
		 *  \see IAO::queryStatus()
		 *
		 *  Returns the buffered status of the AO peripheral, refreshed using IAO::queryStatus() command.
		 *
		 */
		virtual IAO::Status getStatus() const = 0;

		/*!
		 *  \brief Return the buffered Tip/Tilt position of the AO device.
		 *  \param tip AO offset in x-axis.
		 *  \param tilt AO offset in y-axis.
		 *  \see IAO::queryStatus(), IAO::setPosition()
		 *
		 *  Obtains the buffered x- and y-offsets of the AO peripheral, refreshed using the IAO::queryStatus() command.  
		 *  Range allowed for AO tip/tilt operations is [0, 4095] inclusive on both axes. IPromise will return
		 *  EStatusCode::InvalidParameter if either of the tip/tilt values are out of range.
		 *
		 */
		virtual void getPosition(short &tip, short &tilt) const = 0;

		/*!
		*  \brief Return limits for Tip/Tilt position of the AO device.
		*  \param tipMin Reference to a variable that will receive the minimum AO offset on x-axis (i.e. tip).
		*  \param tipMax Reference to a variable that will receive the minimum AO offset on x-axis (i.e. tip).
		*  \param tiltMin Reference to a variable that will receive the minimum AO offset on y-axis (i.e. tilt).
		*  \param tiltMax Reference to a variable that will receive the maximum AO offset on y-axis (i.e. tilt).
		*  \see IAO::queryStatus(), IAO::setPosition()
		*
		*  The default range allowed for AO tip/tilt operations is [0, 4095] inclusive on both axes, but any 
		*  devices deviating from this can rely on IAO::getLimits() to report correct minima and maxima.
		*
		*/
		virtual void getLimits(size_t &tipMin, size_t &tipMax, size_t &tiltMin, size_t &tiltMax) const = 0;

		/*! 
		 *  \brief Get the model number of the connected AO device.
		 *  \return IAO::Model
		 *  \see IAO::Model
		 *
		 *  Obtains the buffered model of the AO peripheral, obtained by a call to IAO::initialize().
		 *
		 */
		virtual Model getModel() const = 0;
	};

	typedef IAO* IAOPtr; //!< Convenience typedef for IAO pointers.

	/*!
	*  \brief FW Interface Class
	*
	*  An interface to Filter Wheel peripherals connected to DLAPI cameras.
	*
	*/
	class IFW {
	public:
		/*! 
		 * \enum IFW::Model
		 * \brief List of supported filter wheel models
		 */
		enum Model
		{
			UnknownFWModel = 0, //!< Used when an unknown FW model is reported by the camera

			CFW2,			//!< CFW2              
			CFW5,			//!< CFW5              
			CFW8,			//!< CFW8              
			CFWL,			//!< CFWL              
			CFW402,			//!< CFW402            
			CFW6A,			//!< CFW6A             
			CFW10,			//!< CFW10             
			CFW10_SERIAL,	//!< CFW10_SERIAL      
			CFW9,			//!< CFW9              
			CFWL8,			//!< CFWL8             
			CFWL8G,			//!< CFWL8G            
			CFW1603,		//!< CFW1603           
			FW5_STX,		//!< STX FW5           
			FW5_8300,		//!< STF-8300 FW5
			FW8_8300,		//!< STF-8300 FW8
			FW7_STX,		//!< STX FW7           
			FW8_STT,		//!< STT FW8           
			FW5_STF_DETENT, //!< STF FW5 with detent  

			// ^^ new models here ^^

			InvalidFWModel = 0xFF, //!< Used for uninitialized variables
		};

		/*! 
		 * \enum IFW::Status
		 * \brief list of supported status codes from FW devices
		 */
		enum Status {
			FWIdle,	 //!< Filter Wheel is idle.
			FWBusy,  //!< Filter Wheel is busy.
			FWError, //!< Filter Wheel has encountered an error.

			InvalidFWStatus = 0xff, //!< Filter Wheel is uninitialized.
		};


		/*!
		 *  \brief Initialize the filter wheel, and obtain basic model information.
		 *  \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation.
		 *  \see IPromisePtr
		 *
		 *  Initializes the Filter Wheel, and obtains the filter wheel model.
		 *
		 */
		virtual IPromisePtr initialize() = 0;

		/*!
		*  \brief Queries the status of the filter wheel.
		*  \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation.
		*  \see IPromisePtr, IFW::getPosition(), IFW::getStatus()
		*
		*  Requests the status and position of the filter wheel from the camera, and updates the buffered values.
		*
		*/
		virtual IPromisePtr queryStatus() = 0;

		/*!
		 *  \brief Returns the buffered position of the filter wheel.
		 *  \return int buffered filter wheel position.
		 *  \see IFW::queryStatus()
		 *  
		 *  Returns the buffered position of the filter wheel, refreshed by IFW::queryStatus().
		 *
		 */
		virtual int getPosition() const = 0;
		
		/*!
		*  \brief Returns the buffered status of the filter wheel.
		*  \return IFW::Status buffered filter wheel status.
		*  \see IFW::queryStatus()
		*
		*  Returns the buffered status of the filter wheel, refreshed by IFW::queryStatus(). Index is base-zero.
		*
		*/
		virtual IFW::Status getStatus() const = 0;
		
		/*!
		*  \brief Returns the model number of the attached filter wheel.
		*  \return IFW::Model the model number of the filter wheel.
		*  \see IFW::Status, IFW::initialize()
		*
		*  Returns the buffered model of the filter wheel, refreshed by IFW::initialize().
		*
		*/
		virtual IFW::Model getModel() const = 0;
		
		/*!
		*  \brief returns the number of slots the filter wheel has available.
		*  \return unsigned int number of filter slots in the filter wheel.
		*
		*  Returns the number of slots available in the filter wheel.
		*
		*/
		virtual unsigned int getSlots() const = 0;
		
		/*!
		*  \brief Send a goto command to the filter wheel.
		*  \param position the desired slot to move to.
		*  \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation.
		*  \see IPromisePtr
		*
		*  Sets the position of the filter wheel. Will return EStatusCode::InvalidParameter if position
		*  is below 0, or above the slot count of the filter wheel when the IPromise completes. Index is 
		*  base-zero.
		*
		*/
		virtual IPromisePtr setPosition(int position) = 0;
	};

	typedef IFW* IFWPtr; //!< Convenience typedef for IFW pointers.


	/*!
	 *  \brief Sensor Interface Class
	 *
	 *  An interface to a sensing chip connected to DLAPI cameras.
	 *
	 */
	class ISensor {
	public:

		/*! 
		 * \enum ISensor::Model
		 * \brief list of supported Sensor models
		 */
		enum Model {
			KAI0340 = 0x00,        //!< Kodak KAI 0340
			SONY_RESERVED = 0x01,  //!< Reserved for future Sony sensor
			ICX694 = 0x02,         //!< Sony ICX 694
			ICX814 = 0x03,		   //!< Sony ICX814
			CCD4710 = 0x04,        //!< CCD 4710
			CCD7700 = 0x05,        //!< CCD 7700
			E2V_RESERVED1 = 0x06,  //!< Reserved for future E2V Sensors
			E2V_RESERVED2 = 0x07,  //!< Reserved for future E2V Sensors
			KAF1603 = 0x09,		   //!< Kodak KAF 1603
			KAF0402 = 0x08,		   //!< Kodak KAF 0402
			KAF3200 = 0x0a,		   //!< Kodak KAF 3200
			KAF8300 = 0x0b,		   //!< Kodak KAF 8300

			KAI08050 = 0x10,	   //!< Kodak KAI 08050
			KAI04070 = 0x11,	   //!< Kodak KAI 04070

			InvalidSensorModel = 0xFF //!< Uninitialized sensor model value
		};

		/*! 
		 * \enum ISensor::Status
		 * \brief list of supported status codes from ISensor devices
		 */
		enum Status
		{

			Idle,            //!< Sensor is in idle state, and can accept ISensor::startExposure() commands.
			Trigger,         //!< Sensor is waiting for trigger.
			PreShutter,      //!< Sensor is preparing for shutter movement.
			DoShutterOpen,   //!< Sensor is opening the shutter.
			Starting,        //!< Sensor is starting the exposure.
			Exposing,        //!< Sensor is exposing.
			DoShutterClose,  //!< Sensor is closing the shutter post-exposure.
			Reading,         //!< Sensor is reading data to buffer.
			ReadyToDownload, //!< Sensor is ready to transfer the image.
			HomingShutter,	 //!< Sensor is waiting for the shutter to return home. Sensor is busy, and is unable to start an exposure until the shutter is homed.

			InvalidSensorState = 0xFF, //!< The sensor status is uninitialized.
		};

		/*!
		 *  \brief A data structure containing sensor specifications.
		 *
		 *  Structure contains all fixed information pertinent to the sensor's characterization (dimensions, filters, limitations, etc)
		 *
		 */
		struct Info
		{
			unsigned int   id;							//!< The sensor's internal ID (0 is the main sensor, 1 is the external sensor, etc).
			ISensor::Model model;						//!< The sensor's model \see ISensor::Model.
			unsigned int   pixelsX;						//!< The sensor's pixel count in the x-axis. Changes when ISensor::Setting::UseOverscan is toggled. \see ISensor::Setting, ISensor::setSubframe()
			unsigned int   pixelsY;						//!< The sensor's pixel count in the y-axis. Changes when ISensor::Setting::UseOverscan is toggled. \see ISensor::Setting, ISensor::setSubframe()
			unsigned int   numberOfChannelsAvailable;	//!< The maximum number of readout channels the sensor has available for use. \see ISensor::Calibration::channelsInUse
			FilterType	   filterType;					//!< The sensor's filter type \see FilterType.
			EFrameType	   frameType;					//!< The sensor's frame type \see EFrameType.
			float		   pixelSizeX;					//!< The sensor's pixel width in micrometers.
			float		   pixelSizeY;					//!< The sensor's pixel height in micrometers.
			unsigned char  flag;						//!< Reserved for future use.
			bool		   hasRBIPreflash;				//!< true if the sensor has RBI preflash, false otherwise.
			int			   minCoolerSetpoint;			//!< Minimum cooler setpoint in degrees Celsius.
			int			   maxCoolerSetpoint;			//!< Maximum cooler setpoint in degrees Celsius.
			unsigned int   maxBinX;						//!< Maximum binning of the sensor on the x-axis.
			unsigned int   maxBinY;						//!< Maximum binning of the sensor on the y-axis.
			float          minExposureDuration;			//!< Minimum exposure duration in seconds.
			float          exposurePrecision;			//!< Minimum increment of exposure duration in seconds.
		};

		/*!
		 *  \brief A data structure containing all factory-calibrated sensor characterization.
		 *
		 *  Structure contains all dynamic aspects of the sensor's characterization (gains, offsets, etc)
		 *
		 */
		struct Calibration
		{
			unsigned int substrateVoltage; //!< Sensor's substrate voltage in mV.
			unsigned int channelsInUse;	   //!< Number of readout channels the API can make use of.
			unsigned int adcGains[4];	   //!< Array of Analog-to-Digital Converter gains in ADC, one per channel. Diffraction Limited strongly advises against modifying these values.
			int			 adcOffsets[4];	   //!< Array of Analog-to-Digital Converter offsets in ADC, one per channel. Diffraction Limited strongly advises against modifying these values.
			float        eGain;			   //!< Measured eGain in e-/ADU.
		};


		/*!
		 * \enum The complete list of enumerable sensor settings, as reported by the camera
		 * \brief Index codes for various user-controlled settings.
		 */
		enum Setting
		{
			UseOverscan,			//!< Use the sensor's overscan region in exposures. This is off by default. Enabling this changes ISensor::Info::pixelsX, and ISensor::Info::pixelsY. \see ISensor::Info, ISensor::setSubframe()
			RBIPreflashDuration,	//!< The duration of an RBI flash before exposures, value is stored in milliseconds.
			RBIPreflashFlushCount,	//!< The number of post-flash flushes performed before an exposure.
			UseWindowHeater,		//!< Toggles a sensor's window heater to prevent fog/frost.
			FanSpeed,				//!< Control the speed of the camera's fan.
			ToggleIRLEDs,			//!< Control the RBI Preflash IR LEDs on the camera. Used for testing purposes. 0 means off, anything else is on.
			UseOnChipBinning,		//!< Enable use of on-chip binning for main sensor.
			UseExtTrigger,			//!< Enable use of external trigger. Set to non-zero to hold exposures in the "Wait for Trigger" state until Ext Trigger is received.
			AutoFanMode,			//!< Enable use of self-regulating fan algorithm.
			SettingCount,			//!< The maximum number of settings available to the user.
			FirstSensorSetting = UseOverscan, //!< The first index of a sensor setting. (In this case, ISensor::UseOverscan)
			InvalidSensorSetting = 0xff //!< Uninitialized sensor setting, not for external use
		};

		/*!
		 *  \brief C-style settings structure with built-in type conversion
		 *  \see ISensor::Setting
		 *
		 *  Structure is reflected by the ISensor::Setting index keys, and contains all user-controlled settings.
		 *
		 */
		struct Settings
		{
			bool useOverscan;  //!< true to enable the sensor's overscan region, or false to disable it. Defaults to false. \seealso ISensor::UseOverscan, ISensor::setSubframe()

			/*!
			 *  \brief RBI Preflash parametrization data structure
			 *
			 *  Structure contains all RBI preflash settings used in the pre-exposure RBI preflash operation.
			 *
			 */
			struct RBIPreflashSettings
			{
				int duration; //!< Duration of the pre-exposure flash in milliseconds. Defaults to 4000. \see ISensor::RBIPreflashDuration
				int flushes;  //!< Number of post-flash flushes. Defaults to 3. \see ISensor::RBIPreflashFlushCount
			} rbiPreflash;    //!< RBI preflash settings.

			bool useWindowHeater;	//!< true to turn on the camera's window heater. false to turn it off. \see ISensor::UseWindowHeater
			int  fanSpeed;			//!< controls the fan speed of the camera. Fan speeds: high = 255, medium = 200, low = 150, off = 0 (not recommended). \see ISensor::FanSpeed
			bool enableIRLEDs;		//!< enable RBI Preflash IR LEDs for testing purposes \see ISensor::ToggleIRLEDs
			bool useOnChipBinning;	//!< Use on-chip binning \see ISensor::UseOnChipBinning
			bool useExtTrigger;		//!< Wait for external trigger during exposures \see ISensor::UseExtTrigger
			bool autoFanSpeed;		//!< Use automatic fan speed control algorithm \see ISensor::AutoFanMode
		};

		/*! 
		 *  \brief Returns the sensor index of the specified sensor (within ICamera)
		 *  \return returns the sensor's internal API index.
		 *  
		 *  Returns the sensor's internal API index. i.e. 0 for main imaging sensors, and 1+ for any connected external sensors.
		 *
		 */
		virtual const unsigned int		getSensorId() const = 0;

		/*!
		 *  \brief Returns the buffered ISensor::Info structure.
		 *  \return returns the buffered sensor info struct.
		 *  \see ISensor::Info, ISensor::queryInfo()
		 *
		 *  Returns the buffered sensor info structure, refreshed by ISensor::queryInfo().
		 *
		 */
		virtual ISensor::Info			getInfo() const = 0;

		/*!
		 *  \brief Returns the buffered ISensor::Calibration structure.
		 *  \return returns the buffered sensor calibration struct.
		 *  \see ISensor::Calibration, ISensor::queryCalibration()
		 *
		 *  Returns the buffered sensor calibration structure, refreshed by ISensor::queryCalibration().
		 *
		 */
		virtual ISensor::Calibration	getCalibration() const = 0;
		
		/*!
		 *  \brief Returns the buffered ISensor::Settings structure.
		 *  \return returns the buffered sensor settings struct.
		 *  \see ISensor::Settings, ISensor::querySettings()
		 *
		 *  Returns the buffered sensor settings structure, refreshed by ISensor::querySettings().
		 *
		 */
		virtual ISensor::Settings		getSettings() const = 0;
		
		/*!
		 *  \brief Returns the sensor's buffered TSubframe structure.
		 *  \return returns the buffered sensor subframe struct.
		 *  \see ISensor::Subframe, ISensor::querySubframe()
		 *
		 *  Returns the buffered sensor calibration structure, refreshed by ISensor::queryCalibration().
		 *
		 */
		virtual TSubframe				getSubframe() const = 0;
		
		/*!
		 *  \brief Returns the ISensor's buffered image.
		 *  \return returns the buffered image.
		 *  \see IImagePtr, ISensor::startExposure()
		 *
		 *  Returns the buffered image structure, only updated when ISensor::startDownload() is complete.
		 *  Image buffers stored here are destroyed when the next exposure is retrieved, so you should
		 *  get in the habit of making a local copy as soon as you retrieve an image.
		 * 
		 *  e.g.
		 *  ```c++
		 *  using namespace dl;
		 * 
		 *  struct image 
		 *  {
		 *    unsigned short * pBuffer;
		 *    unsigned int     length;
		 *    TImageMetadata  meta;
		 *
		 *    image() : pBuffer(nullptr), length(0), meta() {}
		 *    image(IImagePtr pImage) : pBuffer(_pBuffer), length(_length) 
		 *    {
		 *      // Make a deep copy of the incoming IImage immediately
		 *      pBuffer = new short[pImage->getBufferLength()];
		 *      memcpy(pBuffer, pImage->getImageBuffer(), length);
		 *      meta = pImage->getMetadata();
		 *    }
		 *    ~image() { delete [] pBuffer; }
		 *  }
		 *
		 *  image * getImage(IPromisePtr pImagePromise, ISensorPtr pSensor) 
		 *  {
		 *    // pImagePromise would be a promise retrieved from calling ISensor::startDownload()
		 *    // You would typically monitor this in a non-blocking fashion, using IPromise::GetStatus(), since image transfer can take several seconds.
		 *    IPromise::Status result = pImagePromise->wait(); 
		 *    if (result != IPromise::Complete) 
		 *    { 
		 *      // An error occurred.
		 *      pImagePromise->release(); 
		 *      return nullptr; 
		 *    } 
		 *
		 *    // The image was transferred successfully, retrieve it, make a local copy of it, and return it to the caller.
		 *    pImagePromise->release();
		 *    IImagePtr pImage = pSensor->getImage();
		 *    return new image(pImage);
		 *  }
		 *  ```
		 *
		 *  In the example above, we've called ISensor::startDownload() externally, and passed the resulting pointer into
		 *  getImage(), along with the sensor we're downloading the image from. Ownership of the generated image object is 
		 *  then handed off to the caller, and the API will clean up the internal image buffer when the next download is
		 *  started.
		 *
		 */
		virtual IImagePtr getImage() const = 0;

		/*!
		 *  \brief Query the camera's ISensor::Info structure.
		 *  \return queries the sensor's info structure.
		 *  \see IPromisePtr, ISensor::getInfo()
		 *
		 *  Returns the buffered sensor inof structure, refreshed by ISensor::queryImage().
		 *
		 */
		virtual IPromisePtr queryInfo() = 0;

		/*!
		 *  \brief Query a specific value with an ISensor::Setting key from the camera.
		 *  \param setting a setting key to retrieve.
		 *  \return queries a sensor's setting structure.
		 *  \see IPromisePtr, ISensor::Setting, ISensor::getSettings()
		 *
		 *  Returns the buffered sensor settings structure, refreshed by ISensor::querySettings().
		 *
		 */
		virtual IPromisePtr querySetting(ISensor::Setting setting) = 0;

		/*!
		 *  \brief Query the camera's ISensor::Calibration structure.
		 *  \return queries the sensor's calibration structure.
		 *  \see IPromisePtr, ISensor::queryCalibration()
		 *
		 *  Returns the buffered sensor image calibration structure, refreshed by ISensor::queryCalibration().
		 *
		 */
		virtual IPromisePtr queryCalibration() = 0;
		
		/*!
		 *  \brief Query the camera's current subframe.
		 *  \return returns the buffered sensor subframe.
		 *  \see IPromisePtr, ISensor::querySubframe()
		 *
		 *  Returns the buffered sensor subframe structure, refreshed by ISensor::querySubframe().
		 *
		 */
		virtual IPromisePtr querySubframe() = 0;

		/*! 
		 *  \brief Update an ISensor::Setting value in the camera by key.
		 *  \param key the sensor setting to update.
		 *  \param value the new value of the supplied key.
		 *  \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation.
		 *  \see ISensor::Settings, ISensor::Setting
		 *
		 *  Updates a sensor setting value in the camera, and returns a promise to the user to monitor the operation's execution.
		 *  Every supported sensor setting is accompanied with an associated ISensor::Setting key, which you can then update via this function. The
		 *  function accepts an int parameter which is stored in the camera's internal settings structure. Casting from the
		 *  c-style ISensor::Settings structure to the function's int parameter is the responsibility of the user.
		 *
		 */
		virtual IPromisePtr setSetting(ISensor::Setting key, int value) = 0;
		
		/*!
		 *  \brief Update the camera's active subframe.
		 *  \param value the new value of the subframe to send to the camera.
		 *  \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation.
		 *
		 *  Updates a sensor's subframe value in the camera, and returns a promise to the user to monitor the operation's execution.
		 *  The subframe's offset is base zero, with an origin dependent on whether or not ISensor::Setting::useOverscan is set. If
		 *  useOverscan is disabled, pixel [0,0] is situated at the origin of the sensor's active area. If useOverscan is enabled, 
		 *  pixel [0,0] is situated at the origin of the sensor's overscan region. Be sure to update the sensor subframe when toggling
		 *  the useOverscan setting.
		 *
		 *
		 *  ```c++
		 *  void toggleOverscan(ISensorPtr pSensor, bool enable = true)
		 *  {
		 *    // We're going to ignore any promise errors here, for the sake of brevity.
		 *    pSensor->SetSetting(ISensor::UseOverscan, enable ?  1 : 0)->release();
		 *
		 *    // Be sure to update your sensor's info structure, to ensure you've got the right frame properties.
		 *    // If you've got a locally buffered copy of the ISensor::Info struct, you should update that now as well.
		 *    pSensor->queryInfo()->release();
		 *
		 *    // If you're taking full frame images, you should take this opportunity to update your subframe using
		 *    // the new frame dimensions as well.
		 *  }
		 *  ```
		 *
		 */
		virtual IPromisePtr setSubframe(const TSubframe & value) = 0;

		/*!
		 *  \brief Start an exposure using this sensor.
		 *  \param value the parameters of the exposure.
		 *  \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation.
		 *  \see IPromisePtr, TExposureOptions, ISensor::startDownload(), ISensor::Status, ISensor::getImage(), ISensor::abortExposure()
		 *
		 *  The start exposure command takes its parameters from the TExposureOptions structure, and commands
		 *  the camera to take an exposure. Users are then responsible for polling the status of the exposure
		 *  using the ICamera::queryStatus() and ICamera::getStatus() commands. When the camera reports 
		 *  ISensor::Status::Reading, the user is responsible for calling ISensor::startDownload() to begin 
		 *  the image transfer from the camera.
		 * 
		 *  Users are responsible for calling ISensor::setSubframe() before calling ISensor::startExposure()
		 *  and retrieving frame size information for use in the exposure options.
		 *
		 *  e.g. Setting up a 1 second, full-frame, unbinned, light exposure
		 *  ```c++
		 *  using namespace dl;
		 *  void startExposure(ISensorPtr pSensor)
		 *  {
		 *    // Grab sensor frame information
		 *    ISensor::Info info = pSensor->getInfo();
		 *    TSubframe subf;
		 *    subf.top = 0;
		 *    subf.left = 0;
		 *    subf.width = info.pixelsX;
		 *    subf.height = info.pixelsY;
		 *    subf.binX = 1;
		 *    subf.binY = 1;
		 *
		 *    IPromisePtr pPromise = pSensor->setSubframe(subf);
		 *    IPromise::Status result = pPromise->wait();
		 *    if (result != IPromise::Success) throw std::logic_error("Failed to set subframe");
		 *    pPromise->release();
		 *
		 *    TExposureOptions options;
		 *    options.duration = 1.0f;
		 *    options.binX = 1;
		 *    options.binY = 1;
		 *    options.readoutMode = 0; // See: ISensor::getReadoutModes()
		 *    options.isLightFrame = true;
		 *    options.useRBIPreflash = false;
		 *    options.useExtTrigger = false;
		 *
		 *    pPromise = pSensor->startExposure(options);
		 *    result = pPromise->wait();
		 *    if (result != IPromise::Success) throw std::logic_error("Failed to start exposure");
		 *    pPromise->release();
		 *  }
		 *
		 *  ```
		 *
		 *  You can then monitor the status of the exposure using ICamera::GetStatus(), and start downloading the
		 *  image from the camera wiht ISensor::startDownload() when the respective sensor status is ISensor::ReadyToDownload.
		 *
		 */
		virtual IPromisePtr startExposure(const TExposureOptions & value) = 0;

		/*!
		*  \brief Start downloading an image from the camera to DLAPI's internal buffers.
		*  \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation.
		*  \see IPromisePtr, ISensor::startExposure(), ISensor::Status, ISensor::getImage(), ISensor::abortExposure()
		*
		*  The start download command should only be called once a sensor reports the ISensor::Status::Reading,
		*  ISensor::Status::Idle, or ISensor::Status::ReadyToDownload state. Once ISensor::startDownload() is
		*  called, the camera enters an image transferring state, and will respond busy to any commands not
		*  related to transferring the image at hand. All the user has to do at this point is wait for the IPromisePtr
		*  to report IPromise::Complete, and call ISensor::getImage() to retrieve the transferred image from the
		*  buffer. The API handles all the rest.
		* 
		*  Retrieving an image buffer looks something like this:
		*
		*  ```c++
		*  using namespace dl;
		*  bool handlePromise(IPromisePtr pPromise)
		*  {
		*    if (!pPromise) return false;
		*    auto result = pPromise->wait();
		*    // n.b. Fetch error messages here, if applicable
		*    pPromise->release();
		*    return result == IPromise::Complete;
		*  }
		*
		*  bool IsImageReady(ICameraPtr pCamera, ISensorPtr pSensor)
		*  {
		*    auto pPromise = pCamera->queryStatus();
		*    if (!handlePromise(pPromise)) return false;
		*    auto status = pCamera->getStatus();
		*    auto sensorId = pSensor->getSensorId();
		*    auto sensorStatus = ( sensorId != 0 ) ? status.extSensorState : status.mainSensorState;
		*    return sensorStatus == ISensor::ReadyToDownload;
		*  }
		*  
		*  bool startDownload(ICameraPtr pCamera, ISensorPtr pSensor, IPromisePtr& pImagePromise)
		*  {
		*    pImagePromise = nullptr; // Default to returning nothing, unless an image is ready.
		*    if (!IsImageReady(pCamera, pSensor)) return false;
		*    pImagePromise = pSensor->startDownload();
		*    return true;
		*  }
		*  ```
		* 
		*  In the case of the code above, you would call startDownload() after a successful call to ISensor::startExposure.
		*  You would pass in a pointer to a camera and sensor via pCamera and pSensor respectively, and also pass in a reference
		*  to an IPromisePtr variable which will receive the promise returned by ISensor::startDownload(). When pImagePromise
		*  returns complete, the image will be stored in the local buffer, and is retrievable via ISensor::getImage().
		* 
		*/
		virtual IPromisePtr startDownload() = 0;

		/*! 
		 *  \brief Abort any exposures/downloads either in progress, or waiting for user input.
		 *  \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation. 
		 *  \see IPromisePtr, ISensor::startExposure(), ISensor::startDownload
		 *
		 *  To abort an ongoing image, send this command to the camera. No image is buffered, or transferred from the camera.
		 *  All image data acquired is discarded. It is considered good practice to call abortExposure() when you connect to
		 *  a camera, to clear any pending exposures that may be in progress from improper shutdown procedures. 
		 *  
		 */
		virtual IPromisePtr abortExposure() = 0;


		/*!
		 * \brief Returns a list of human readable readout modes (the index of each entry corresponding to TExposureOptions::readoutMode)
		 * \param buf pointer to an array of characters of length lng 
		 * \param lng input the maximum length of the storage buffer, outputs the actual length of the storage buffer.
		 *
		 * Returns a null-terminated, '\\n' delimited list of readout modes. The index of this list is zero-based, when referencing readout 
		 * modes in TExposureOptions.
		 *
		 * ```c++
		 * std::string getReadoutModes(ISensorPtr pSensor)
		 * {
		 *   char buf[1028];
		 *   size_t lng = 1028;
		 *   pSensor->getReadoutModes(&(buf[0]), lng);
		 *   return std::string(&(buf[0]), lng);
		 * }
		 * ```
		 *
		 * e.g. The above function for an Aluma 3200 would return "Normal\nLow-Noise\0". If you wanted to use Low-Noise mode, you would set it up in TExposureOptions
		 * as follows:
		 * ```c++
		 * void startExposure(ISensorPtr pSensor)
		 * {
		 *   TExposureOptions expOptions;
		 *   expOptions.duration = 1.0;
		 *   expOptions.binX = 1;
		 *   expOptions.binY = 1;
		 *   expOptions.readoutMode = 1; // because Low-Noise is second in the zero-indexed readout mode list.
		 *   expOptions.isLightFrame = true;
		 *   expOptions.useRBIPreflash = false;
		 *   expOptions.useExtTrigger = false;
		 *   handlePromise(pSensor->startExposure(expOptions));
		 * }
		 * ```
		 *
		 */
		virtual void getReadoutModes(char * buf, size_t& lng) const = 0;
	};

	typedef ISensor* ISensorPtr; //!< Convenience typedef for ISensor pointers.

	/*!
	 * \class ICamera
	 * \brief Camera interface class
	 *
	 * An interface to a DLAPI compliant camera providing general access to camera-level functions/settings, and 
	 * stores references to connected peripherals/imaging sensors. Each instance of ICamera is self-contained, and 
	 * can safely be used in tandem with other instances of ICamera. Because Camera I/O is threaded at the DLAPI 
	 * level, the number of ICamera instances you can control simultaneously is limited by the number of active 
	 * threads your system can have at any given time.
	 *
	 * Further to this limitation, each ICamera instance has references to a number of ISensor instances. While
	 * these instances are self-contained, DLAPI is not capable of performing simultaneous downloads of two 
	 * sensors on the same camera at this time. Attempting to call image transport functions for one sensor while
	 * another sensor is in the ISensor::ReadyToDownload state will return an error (specifically 
	 * EStatusCode::ReadoutInProgress).
	 *
	 */
	class ICamera
	{
	public:	
		/*! 
		 * \enum ICamera::State
		 * \brief list of camera initialization states
		 */
		enum State {
			Uninitialized,	//<! Uninitialized Camera is uninitialized.
			Initializing,	//<! Initializing Camera is in process of initializing.
			Ready,			//<! Ready Camera is ready for operation.
			Error			//<! Error Camera has had an error.
		};

		/*! 
		 * \enum ICamera::Model
		 * \brief list of supported camera models
		 */
		enum Model {
			Aluma,			//!< Aluma model cameras. 
			AlumaPro,		//!< Aluma Pro model cameras.
			Reserved,		//!< Reserved for future use.
			AlumaCMOS,		//!< Aluma CMOS model cameras.
			AlumaVacuum,	//!< Aluma Vacuum model cameras.

			InvalidCameraModel = 0xff //!< Camera has not been initialized/reported invalid model
		};

		/*!
		 * \struct ICamera::Status
		 * \brief Camera status structure
		 * \see ICamera::queryStatus(), ICamera::getStatus()
		 *
		 * Structure containing the state of core camera components.
		 *
		 */
		struct Status
		{
			ISensor::Status mainSensorState;		//!< Main sensor state. \see ISensor::Status
			ISensor::Status extSensorState;			//!< External sensor state. \see ISensor::Status
			float           sensorTemperature;		//!< Sensor Temperature Reading
			float           coolerPower;			//!< Cooler Power Reading
			float           heatSinkTemperature;	//!< Heat Sink Temperature Reading
			float           inputVoltage;			//!< Input Voltage Reading from the power supply
			unsigned int    pulseGuideStatus;       //!< Status of the pulse guide relays. TODO: Define
			unsigned int    flag;					//!< Reserved for future use.
			EShutterStatus	shutterStatus;			//!< Main sensor shutter status.

			Status() 
				: mainSensorState(ISensor::InvalidSensorState), extSensorState(ISensor::InvalidSensorState), 
				  sensorTemperature(0), coolerPower(0), heatSinkTemperature(0), inputVoltage(0), 
				  pulseGuideStatus(0), flag(0), shutterStatus(ShutterUninitialized) {} //<! Default constructor.
		};

		/*! 
		 * \struct ICamera::Info
		 * \brief Camera info structure
		 * \see ICamera::queryInfo(), ICamera::getInfo(), ICamera::getSerial(), ICamera::getSensor()
		 *
		 * Structure containing camera specific information, such as the model identifier,
		 * encoded serial number, gate array and WiFi firmware versions, and the number of
		 * available sensors.
		 *
		 */
		struct Info
		{
			ICamera::Model	model;					//!< Model of the camera \see ICamera::Model
			unsigned int	serialNumber;			//!< Encoded serial number of the sensor, use ICamera::getSerial() for human readable string version.
			unsigned int	firmwareRevision;		//!< Revision number of the camera's Gate Array firmware.
			unsigned int	wifiFirmwareRevision;	//!< Revision number of the camera's WiFi firmware.
			unsigned char	numberOfSensors;		//!< Number of sensors connected to the camera.

			Info() : model(InvalidCameraModel), serialNumber(0), firmwareRevision(0), wifiFirmwareRevision(0), numberOfSensors(0) {}  //!< Default constructor.
		};
		
		/*!
		 * \brief virtual destructor ensuring the class is abstract.
		 *
		 * Used to ensure ICamera is pure virtual
		 */
		virtual ~ICamera() = 0;
		
		// Query functions ping the camera for updated values.
		
		/**
		 * \brief Initializes the camera by fetching operational status, and initializing the various peripherals
		 * 
		 * A blocking call which queries the camera for all relevant information stuctures:
		 *
		 * - ICamera::queryStatus()
		 * - ICamera::queryNetworkSettings()
		 * - ISensor::queryInfo()
		 * - ISensor::queryCalibration()
		 * - ISensor::querySetting()
		 *
		 * Throws a std::runtime_error on failure.
		 *
		 */
		virtual bool initialize() = 0;

		/*!
		 * \brief Queries the camera's unit information structure (ICamera::Info)
		 * \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation.
		 * \see IPromisePtr, ICamera::getInfo()
		 *
		 * Queries the camera for the ICamera::Info structure, retrieve the structure on promise completion with a call to ICamera::getInfo().
		 *
		 */
		virtual IPromisePtr     queryInfo() = 0;

		/*!
		 * \brief Queries the camera's ICamera::Status structure
		 * \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation.
		 * \see IPromisePtr, ICamera::getStatus()
		 *
		 * Queries the camera for the ICamera::Status structure, retrieve the structure on promise completion with a call to ICamera::getStatus().
		 *
		 */
		virtual IPromisePtr     queryStatus() = 0;

		/*!
		 * \brief Queries the camera's network settings
		 * \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation.
		 * \see IPromisePtr, ICamera::getNetworkSettings(), ICamera::setNetworkSettings()
		 * 
		 * Queries the camera for the ICamera::NetworkStettings structure, retrieve the structure on promise completion with a call to ICamera::getStatus().
		 *
		 */
		virtual IPromisePtr     queryNetworkSettings() = 0;

		// Commands

		/*!
		 * \brief Executes or aborts a pulse guide operation of given duration and direction
		 * \param direction the relay direction to fire. \see EPulseGuideDirection
		 * \param duration the duration of the pulse guide operation in milliseconds.
		 * \param abort set to true to abort a previous pulse guide operation, false to execute a new operation.
		 * \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation.
		 * \see IPromisePtr, EPulseGuideDirection, ICamera::getStatus() 
		 * 
		 * Command the guider port output to move the mount in the provided direction, for the provided duration, 
		 * or abort the current operation if abort is set to true. 
		 *
		 * The status of the current pulse guide operation is stored in the camera's ICamera::Status structure.
		 *
		 */
		virtual IPromisePtr     pulseGuide(EPulseGuideDirection direction, unsigned int duration, bool abort) = 0;

		// Set functions

		/*!
		 * \brief Updates the camera's network settings.
		 * \param cfg the new network configuration 
		 * \return IPromisePtr pointer to promise for monitoring the status of the asynchronous operation.
		 * \see IPromisePtr, TNetworkSettings, ICamera::queryNetworkSettings(), ICamera::getNetworkSettings()
		 *
		 * Updates the network settings of the camera.
		 *
		 */
		virtual IPromisePtr		setNetworkSettings(const TNetworkSettings& cfg) = 0;
		
		// Get functions return buffered values obtained using query functions.

		/*!
		 * \brief Returns the buffered ICamera::Info structure.
		 * \return ICamera::Info the buffered camera info structure.
		 * \see ICamera::Info, ICamera::queryInfo()
		 *
		 * Returns the buffered camera info structure, refreshed by ICamera::queryInfo().
		 * 
		 */
		virtual ICamera::Info	   getInfo()  const = 0;

		/*!
		 * \brief Returns a human readable version of the camera's serial number string. e.g. "AL8300M-19010101"
		 * \param buffer pointer to a character array which receives the serial number string.
		 * \param buffer_length [IN] the maximum size of the buffer supplied, [OUT] the size of the string returned.
		 * \return ICamera::Info the buffered camera info structure.
		 * \see ICamera::Info, ICamera::queryInfo()
		 *
		 * Returns the human-readable serial number string of the camera, which requires the camera be properly initialized.
		 * Normally, this is handled on discovery by the API, but if an error occurs, you can manually initialize the camera
		 * by calling the following functions:
		 * - ICamera::queryInfo() 
		 * - ISensor::queryInfo() for each sensor.
		 *
		 * or calling ICamera::initialize().
		 *
		 * n.b. This differs from the ICamera::Info::serialNumber in that serialNumber contains a subset of the information required
		 * to recreate the human-readable serial number string. 
		 *
		 * e.g.
		 *
		 * ```c++
		 * std::string getSerial(dl::ICameraPtr pCamera)
		 * {
		 *   char buf[512] = {0};
		 *   size_t lng = 512;
		 *   pCamera->getSerial(&(buf[0]), lng);
		 *   return std::string(&(buf[0]), lng);
		 * }
		 * ```
		 *
		 */
		virtual void			   getSerial(char * buffer, size_t & buffer_length) const = 0;

		/*!
		 * \brief Returns the connection type of the camera (USB, Networked, etc).
		 * \return EEndpointType Information about the camera's connection type. \see EEndpointType for more information.
		 * \see EEndpointType
		 *
		 * Returns the camera's connection type: USB, Net, etc. See EEndpointType for more details.
		 *
		 */
		virtual EEndpointType	   getConnectionType() const = 0;

		/*!
		 * \brief Returns a human readable information string about the camera's connection (IP Address, USB Handle, etc).
		 * \param buffer a buffer to a character array.
		 * \param buffer_length the maximum size of the supplied buffer, updated to be the length of the information string.
		 *
		 * Returns a human readable string containing connection details about this camera. This is mostly for debug purposes.
		 * e.g. "USB: handle=0x1234", or "Net: ip="127.0.0.1"
		 *
		 * ```c++
		 * std::string getConnectionInfo(dl::ICameraPtr pCamera)
		 * {
		 *   char buf[512] = {0};
		 *   size_t lng = 512;
		 *   pCamera->getConnectionInfo(&(buf[0]), lng);
		 *   return std::string(&(buf[0]), lng);
		 * }
		 * ```
		 *
		 */
		virtual void getConnectionInfo(char * buffer, size_t &buffer_length) const = 0;

		/*!
		 * \brief Returns the buffered status structure
		 * \return ICamera::Status the buffered camera status structure.
		 * \see ICamera::Status, ICamera::queryStatus()
		 *
		 * Returns the buffered camera status structure, refreshed by ICamera::queryStatus().
		 *
		 */
		virtual ICamera::Status getStatus() const = 0;


		/*!
		 * \brief Returns the buffered network settings.
		 * \return TNetworkSettings the buffered camera info structure.
		 * \see TNetworkSettings, ICamera::setNetworkSettings(), ICamera::queryNetworkSettings()
		 *
		 * Returns the buffered camera network settings structure, refreshed by ICamera::queryNetworkSettings().
		 *
		 */
		virtual TNetworkSettings  getNetworkSettings() const = 0;


		// Get Device Interfaces

		/*!
		 * \brief Returns a pointer to the indexed sensor (zero is always the primary imaging chip).
		 * \param id the internal index of the sensor to retrieve
		 * \return ISensorPtr a pointer to the sensor with id.
		 * \see ISensor, ICamera::getInfo()
		 *
		 * This function retrieves a pointer to the camera's sensor with internal id matching the supplied parameter.
		 * The internal sensor id is a zero based index, starting with the main sensor, and increasing with external 
		 * sensors up to the number of sensors a camera has is stored in the ICamera::Info::numberOfSensors variable. 
		 * If a user attempts to access a sensor outside that range, or the sensor is otherwise unavailable, the 
		 * function returns nullptr.
		 * 
		 */
		virtual ISensorPtr		getSensor(unsigned int id) const = 0;
		
		/*!
		 * \brief Returns a pointer to the camera's TEC (if one is available).
		 * \return ITECPtr a pointer to the camera's thermo-electric cooler.
		 * \see ITEC
		 *
		 * Returns a pointer to the camera's thermo-electric cooler, or nullptr if no TEC is present.
		 *
		 */
		virtual ITECPtr			getTEC() const = 0;

		/*!
		 * \brief Returns a pointer to the camera's AO (if one is available). 
		 * \return IAOPtr a pointer to the camera's adaptive optics unit.
		 * \see IAO
		 *
		 * Returns a pointer to the camera's adaptive optics unit, or nullptr if no AO is present.
		 *
		 */
		virtual IAOPtr			getAO()	const = 0;

		/**
		 * \brief Returns a pointer to the camera's FW (if one is available).
		 * \return a pointer to the camera's filter wheel.
		 * \see IFW
		 *
		 * Returns a pointer to the camera's filter wheel unit, or nullptr if no FW is present.
		 * 
		 */
		virtual IFWPtr			getFW()	const = 0;
	};

	typedef ICamera* ICameraPtr; //!< Convenience typedef for ICamera pointers.
	
	/*!
	 * \class IGateway
	 * \brief Gateway object for controlling device discovery, and lifetime management.
	 *   
	 * The IGateway interface provides users with a single point of access for discovering
	 * DLAPI devices, and managing their resources from the start to finish of an
	 * of an imaging lifecycle. 
	 *
	 * An example application would wrap all API calls in the dl::getGateway() and dl::deleteGateway()
	 * function as follows:
	 *
	 * ```
	 * #include <dlapi.h>
	 *
	 * int main()
	 * {
	 *   dl::IGatewayPtr pGateway = dl::getGateway();
	 *   pGateway->queryUSBCameras();
	 *   if (pGateway->getUSBCameraCount() <= 0) return 1;
	 *   
	 *   // If we find a camera in the USB device list, initialize it and retrieve its ICamera interface.
	 *   dl::ICameraPtr pCamera = pGateway->getUSBCamera(0);
	 *
	 *   // do stuff ...
	 *   
	 *   dl::deleteGateway(pGateway);
	 *   return 0;
	 * }
	 * ```
	 */
	class IGateway
	{
	public:

		/*! 
		 * \brief Virtual destructor to ensure the class is an abstract.
		 * 
		 * Used to ensure IGateway is pure-virtual. 
		 *
		 */
		virtual ~IGateway() = 0;
		
		/*!
		 * \brief Returns the connection details for a camera with the provided serial number.
		 * \param serial a buffer containing the serial number to look up connection details of.
		 * \return TConnectionDetails the structure containing internal API indexing information for connecting to the camera.
		 * \see TConnectionDetails, EEndpointType
		 * 
		 * This function takes a serial number input, and searches the list of found devices for one with a matching serial.
		 * If a camera with a matching serial is found, the function returns a TConnectionDetails structure with the appropriate
		 * EEndpointType, and device index. The user can use this information to access the appropriate USB, or UDP camera 
		 * access points. 
		 *
		 * If no camera is found, TConnectionDetails::endpointType will be set to InvalidEndpoint.
		 * 
		 */
		virtual TConnectionDetails getCameraConnectionDetails(unsigned int serial) const = 0;
		
		/*!
		 * \brief Fetches a pointer to a camera with the provided connection details (if one is available).
		 * \param details obtain a camera using the connection details supplied.
		 * \return ICameraPtr a pointer to a camera with matching connection details.
		 * \see ICameraPtr, TConnectionDetails
		 *
		 * This convenience function takes a TConnectionDetails structure, and attempts to access a camera from
		 * the appropriate USB, or UDP device list. If no camera can be found matching those connection details,
		 * nullptr is returned.
		 * 
		 */
		virtual ICameraPtr  getCamera(TConnectionDetails details) const = 0;

		/*!
		 * \brief Query the computer for any DLAPI compliant cameras connected via USB.
		 * 
		 * This function searches through all available USB devices connected to the computer which are not in use 
		 * by another API instance. This function blocks until the operation is complete. When the operation is complete
		 * it will have enumerated all DLAPI protocol compliant devices connected to the computer via USB. It will have
		 * called:
		 *
		 * - ICamera::queryInfo().
		 * - ICamera::queryNetworkSettings().
		 * - ISensor::queryInfo() - for each sensor.
		 * - ISensor::queryCalibration() - for each sensor.
		 * - ISensor::querySubframe() - for each sensor.
		 * 
		 */
		virtual void		queryUSBCameras() = 0;
		
		/**
		 * \brief Returns the number of cameras found via the last call to IGateway::queryUSBCameras().
		 * \return size_t the number of discovered USB cameras.
		 * \see IGateway::queryUSBCameras(), IGateway::getUSBCamera(), IGateway::queryNetCamera(), IGateway::getNetCamera(), IGateway::getNetCameraCount(), 
		 *
		 * Returns the number of cameras found using the IGateway::queryUSBCameras() function. Retrieve pointers 
		 * to found cameras via IGateway::getUSBCamera().
		 *
		 */
		virtual size_t	    getUSBCameraCount() const = 0;
		
		/**
		 * \brief Returns a pointer to the camera with the supplied index, from zero to IGateway::getUSBCameraCount().
		 * \param id zero-based index of the usb camera to retrieve
		 * \return ICameraPtr pointer to camera with index id.
		 * \see IGateway::queryUSBCameras(), IGateway::getUSBCameraCount(), IGateway::queryNetCamera(), IGateway::getNetCamera(), IGateway::getNetCameraCount(),
		 *
		 * Returns the usb camera associated with the provided zero-based index. Attempting to access indices below 
		 * zero, or at the value returned by IGateway::getUSBCameraCount() will return nullptr.
		 * 
		 */
		virtual ICameraPtr  getUSBCamera(unsigned int id) const = 0;
		
		/*!
		* \brief Broadcast and enumerate any DLAPI compliant cameras connected to your network.
		*
		* This function searches through all available UDP devices on the active network. This function blocks until the operation 
		* is complete. When the operation is complete it will have enumerated all DLAPI protocol compliant devices connected to 
		* the computer via a network. It will have called:
		*
		* - ICamera::queryInfo().
		* - ICamera::queryNetworkSettings().
		* - ISensor::queryInfo() - for each sensor.
		* - ISensor::queryCalibration() - for each sensor.
		* - ISensor::querySubframe() - for each sensor.
		* 
		* This process takes up to 2 seconds to complete while the cameras respond via UDP. Note that virtual network interfaces,
		* and bridged adapters can eat up the UDP broadcasts destined for DLAPI devices. If your device is not discovered,
		* check to make sure you have no network adapters that might be intercepting those broadcasts.
		*
		*/
		virtual void queryNetCameras() = 0;

		/*!
		* \brief Attempt to connect to a camera with the supplied IP Address and port.
		*
		* This function sends a GetInfo request to the device located at ip:port. This function blocks until the operation is 
		* complete. When the operation is complete it will have enumerated all DLAPI protocol compliant devices connected 
		* to the computer via a network. It will have called:
		*
		* - ICamera::queryInfo().
		* - ICamera::queryNetworkSettings().
		* - ISensor::queryInfo() - for each sensor.
		* - ISensor::queryCalibration() - for each sensor.
		* - ISensor::querySubframe() - for each sensor.
		*
		* This process takes up to 2 seconds to complete while the cameras respond via UDP over IP. Note that virtual network interfaces,
		* and bridged adapters can eat up the UDP broadcasts destined for DLAPI devices. If your device is not discovered,
		* check to make sure you have no network adapters that might be intercepting those broadcasts.
		*
		*/
		virtual void queryNetCamera(const char * ip, size_t port) = 0;
		
		/*!
		 * \brief Returns the number of cameras found after a call to IGateway::queryNetCameras().
		 * \return  size_t the number of discovered UDP cameras.
		 * \see IGateway::queryUSBCameras(), IGateway::getUSBCamera(), IGateway::getUSBCameraCount(), IGateway::getNetCamera(), IGateway::getNetCameraCount(),
		 *
		 * Returns the number of cameras found using the IGateway::queryNetCameras() function. Retrieve pointers
		 * to found cameras via IGateway::getNetCamera().
		 * 
		 */
		virtual size_t getNetCameraCount() const = 0;

		/*!
		 * \brief Get a pointer to a camera found on the network via IGateway::queryNetCameras() with the given index, from zero to IGateway::getNetCameraCount().
		 * \param id zero-based index of the UDP camera to retrieve
		 * \return ICameraPtr pointer to camera with index id.
		 * 
		 * Returns the Networked camera associated with the provided zero-based index. Attempting to access indices below
		 * zero, or at the value returned by IGateway::getNetCameraCount() will return nullptr.
		 *
		 */
		virtual ICameraPtr getNetCamera(unsigned int id) const = 0;
		
		/*!
		 * \brief Get a pointer to a queried camera with a given IP and port, if it exists.
		 * \param ip a null-terminated string containing the IP address of the camera to obtain.
		 * \param port the network port to query.
		 * \return ICameraPtr pointer to camera with index id.
		 *
		 * Returns the Networked camera at the given ip address. if no camera is found at the given IP address,
		 * the function returns nullptr.
		 *
		 */
		virtual ICameraPtr  getNetCamera(const char * ip, unsigned int port) const = 0;

		/*!
		 * \brief Returns the maximum FPGA revision supported by this version of DLAPI.
		 * \return size_t the maximum camera FPGA revision the driver is capable of running without error
		 */
		virtual size_t getCommProtocolVersion() const = 0;
	};

	typedef IGateway* IGatewayPtr; //!< Convenience typedef for IGateway pointers.

	/*!
	 * \brief Returns a pointer to an instance of a Gateway object, or (if one exists) return the existing pointer to it.
	 * \return IGatewayPtr pointer to the instance of the process' IGateway.
	 * \see IGateway, deleteGateway()
	 *
	 * This function provides the user with the singleton instance of the DLAPI IGateway for the process.
	 * Use this function to gain access to all DLAPI devices. Minimize the number of calls to this function
	 * if at all possible. It is not necessary to keep an IGateway instance active throughout the lifetime of
	 * your application, but you should avoid creating gateways at call sites. Keep the gateway instances
	 * alive throughout the scope of all communications with the API. All calls to dl::getGateway() should be 
	 * accompanied by a call to dl::deleteGateway().
	 *
	 * An easy way to ensure this occurs is to create a smart pointer with a custom deleter that handles
	 * this cleanup.
	 * 
	 */
	DL_API IGatewayPtr MYCDECL getGateway();

	/*!
	 * \brief Decrement the internal count of IGateway pointers supplied, and delete the active IGateway instance when it is no longer in use.
	 * \param IGatewayPtr a pointer to an IGateway object to delete.
	 * \see IGateway, getGateway()
	 *
	 * This function tracks usage counts of getGateway(), and deletes the singleton instance of IGateway when
	 * the last instance is released.
	 * 
	 */
	DL_API void        MYCDECL deleteGateway(IGatewayPtr);

#if defined(_WIN32) || defined(_WIN64)
	typedef IGatewayPtr(MYCDECL * f_GetGateway)();			//!< Convenience typedef for DLL binding to dl::getGateway().
	typedef void(MYCDECL * f_DeleteGateway)(IGatewayPtr);	//!< Convenience typedef for DLL binding to dl::deleteGateway().
#endif

}
#endif
