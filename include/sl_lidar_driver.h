/*
 *  Slamtec LIDAR SDK
 *
 *  Copyright (c) 2020 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 */
 /*
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
  * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  */

#pragma once


#include <Arduino.h>
#include <vector>
#include <map>
#include <string>

#include "sl_lidar_cmd.h"
#include "rplidar_cmd.h"
#include "rplidar_config.h"

#define NORMAL_CAPSULE          0
#define DENSE_CAPSULE           1

#define LIDAR_TIMEOUT 10
#define RPLIDAR_SCAN_DELAY 0

// Event bits for controlling rplidar task
// LiDAR scan complete.
#define EVT_RPLIDAR_SCAN_COMPLETE 1<<0
// All event bits together... the full group...
#define EVT_RPLIDAR_SCAN_GROUP ( EVT_RPLIDAR_SCAN_COMPLETE )

namespace sl {

    /**
    * Lidar scan mode
    */
    struct LidarScanMode
    {
        // Mode id
        sl_u16  id;

        // Time cost for one measurement (in microseconds)
        float   us_per_sample;

        // Max distance in this scan mode (in meters)
        float   max_distance;

        // The answer command code for this scan mode
        sl_u8   ans_type;

        // The name of scan mode (padding with 0 if less than 64 characters)
        char    scan_mode[64];
    };

    template <typename T>
    struct Result
    {
        sl_result err;
        T value;
        Result(const T& value)
            : err(SL_RESULT_OK)
            , value(value)
        {
        }

        Result(sl_result err)
            : err(err)
            , value()
        {
        }

        operator sl_result() const
        {
            return err;
        }

        operator bool() const
        {
            return SL_IS_OK(err);
        }

        T& operator* ()
        {
            return value;
        }

        T* operator-> ()
        {
            return &value;
        }
    };

    // scan types for scan function
    typedef enum rplidar_scan_cache {
        CACHE_SCAN_DATA,
        CACHE_CAPSULED_SCAN_DATA,
        CACHE_HQ_SCAN_DATA,
        CACHE_ULTRA_CAPSULED_SCAN_DATA
    } rplidar_scan_cache;

    enum MotorCtrlSupport
    {
        MotorCtrlSupportNone = 0,
        MotorCtrlSupportPwm = 1,
        MotorCtrlSupportRpm = 2,
    };

    enum ChannelType{
        CHANNEL_TYPE_SERIALPORT = 0x0,
        CHANNEL_TYPE_TCP = 0x1,
        CHANNEL_TYPE_UDP = 0x2,
    };

        /**
    * Lidar motor info
    */
    struct LidarMotorInfo
    {
        MotorCtrlSupport motorCtrlSupport;

        // Desire speed
        sl_u16 desired_speed;

        // Max speed 
        sl_u16 max_speed;

        // Min speed
        sl_u16 min_speed;
    };

    class ILidarDriver
    {
    public:
        virtual ~ILidarDriver() {}

    public:
        /**
        * Connect to LIDAR via channel
        * \param channel The communication channel
        *                    Note: you should manage the lifecycle of the channel object, make sure it is alive during lidar driver's lifecycle
        */
        virtual sl_result connect(HardwareSerial* channel) = 0;

        /**
        * Disconnect from the LIDAR
        */
        virtual void disconnect() = 0;
        
        /**
        * Check if the connection is established
        */
        virtual bool isConnected() = 0;

    public:
        // enum
        // {
        //     DEFAULT_TIMEOUT = 2000
        // };

    public:
        /// Ask the LIDAR core system to reset it self
        /// The host system can use the Reset operation to help LIDAR escape the self-protection mode.
        ///
        ///  \param timeout       The operation timeout value (in millisecond)
        virtual sl_result reset(sl_u32 timeoutInMs = DEFAULT_TIMEOUT) = 0;

        /// Get all scan modes that supported by lidar
        virtual sl_result getAllSupportedScanModes(std::vector<LidarScanMode>& outModes, sl_u32 timeoutInMs = DEFAULT_TIMEOUT) = 0;

        /// Get typical scan mode of lidar
        virtual sl_result getTypicalScanMode(sl_u16& outMode, sl_u32 timeoutInMs = DEFAULT_TIMEOUT) = 0;

        /// Start scan
        ///
        /// \param force            Force the core system to output scan data regardless whether the scanning motor is rotating or not.
        /// \param useTypicalScan   Use lidar's typical scan mode or use the compatibility mode (2k sps)
        /// \param options          Scan options (please use 0)
        /// \param outUsedScanMode  The scan mode selected by lidar
        virtual sl_result startScan(bool force, bool useTypicalScan, sl_u32 options = 0, LidarScanMode* outUsedScanMode = nullptr) = 0;

        /// Start scan in specific mode
        ///
        /// \param force            Force the core system to output scan data regardless whether the scanning motor is rotating or not.
        /// \param scanMode         The scan mode id (use getAllSupportedScanModes to get supported modes)
        /// \param options          Scan options (please use 0)
        /// \param outUsedScanMode  The scan mode selected by lidar
        virtual sl_result startScanExpress(bool force, sl_u16 scanMode, sl_u32 options = 0, LidarScanMode* outUsedScanMode = nullptr, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        /// Retrieve the health status of the RPLIDAR
        /// The host system can use this operation to check whether RPLIDAR is in the self-protection mode.
        ///
        /// \param health        The health status info returned from the RPLIDAR
        ///
        /// \param timeout       The operation timeout value (in millisecond) for the serial port communication     
        virtual sl_result getHealth(sl_lidar_response_device_health_t& health, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        /// Get the device information of the RPLIDAR include the serial number, firmware version, device model etc.
        /// 
        /// \param info          The device information returned from the RPLIDAR
        /// \param timeout       The operation timeout value (in millisecond) for the serial port communication  
        virtual sl_result getDeviceInfo(sl_lidar_response_device_info_t& info, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        /// Check whether the device support motor control
        /// Note: this API will disable grab.
        /// 
        /// \param motorCtrlSupport Return the result.
        /// \param timeout          The operation timeout value (in millisecond) for the serial port communication. 
        virtual sl_result checkMotorCtrlSupport(MotorCtrlSupport& motorCtrlSupport, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        /// Calculate LIDAR's current scanning frequency from the given scan data
        /// Please refer to the application note doc for details
        /// Remark: the calcuation will be incorrect if the specified scan data doesn't contains enough data
        ///
        /// \param scanMode      Lidar's current scan mode
        /// \param nodes         Current scan's measurements
        /// \param count         The number of sample nodes inside the given buffer
        virtual sl_result getFrequency(const LidarScanMode& scanMode, const sl_lidar_response_measurement_node_hq_t* nodes, size_t count, float& frequency) = 0;

		///Set LPX series lidar's static IP address
		///
		/// \param conf             Network parameter that LPX series lidar owned
		/// \param timeout          The operation timeout value (in millisecond) for the ethernet udp communication
		virtual sl_result setLidarIpConf(const sl_lidar_ip_conf_t& conf, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

		///Get LPX series lidar's MAC address
		///
		/// \param macAddrArray         The device MAC information returned from the LPX series lidar
		virtual sl_result getDeviceMacAddr(sl_u8* macAddrArray, sl_u32 timeoutInMs = DEFAULT_TIMEOUT) = 0;

        /// Ask the LIDAR core system to stop the current scan operation and enter idle state. The background thread will be terminated
        ///
        /// \param timeout       The operation timeout value (in millisecond) for the serial port communication 
        virtual sl_result stop(sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        /// Wait and grab a complete 0-360 degree scan data previously received. 
        /// The grabbed scan data returned by this interface always has the following charactistics:
        ///
        /// 1) The first node of the grabbed data array (nodebuffer[0]) must be the first sample of a scan, i.e. the start_bit == 1
        /// 2) All data nodes are belong to exactly ONE complete 360-degrees's scan
        /// 3) Note, the angle data in one scan may not be ascending. You can use API ascendScanData to reorder the nodebuffer.
        ///
        /// \param nodebuffer     Buffer provided by the caller application to store the scan data
        ///
        /// \param count          The caller must initialize this parameter to set the max data count of the provided buffer (in unit of rplidar_response_measurement_node_t).
        ///                       Once the interface returns, this parameter will store the actual received data count.
        ///
        /// \param timeout        Max duration allowed to wait for a complete scan data, nothing will be stored to the nodebuffer if a complete 360-degrees' scan data cannot to be ready timely.
        ///
        /// The interface will return SL_RESULT_OPERATION_TIMEOUT to indicate that no complete 360-degrees' scan can be retrieved withing the given timeout duration. 
        ///
        /// \The caller application can set the timeout value to Zero(0) to make this interface always returns immediately to achieve non-block operation.
        virtual sl_result grabScanDataHq(sl_lidar_response_measurement_node_hq_t* nodebuffer, size_t& count, sl_u32 timeout = DEFAULT_TIMEOUT) = 0;

        /// Ascending the scan data according to the angle value in the scan.
        ///
        /// \param nodebuffer     Buffer provided by the caller application to do the reorder. Should be retrived from the grabScanData
        ///
        /// \param count          The caller must initialize this parameter to set the max data count of the provided buffer (in unit of rplidar_response_measurement_node_t).
        ///                       Once the interface returns, this parameter will store the actual received data count.
        /// The interface will return SL_RESULT_OPERATION_FAIL when all the scan data is invalid. 
        virtual sl_result ascendScanData(sl_lidar_response_measurement_node_hq_t* nodebuffer, size_t count) = 0;

        /// Return received scan points even if it's not complete scan
        ///
        /// \param nodebuffer     Buffer provided by the caller application to store the scan data
        ///
        /// \param count          Once the interface returns, this parameter will store the actual received data count.
        ///
        /// The interface will return SL_RESULT_OPERATION_TIMEOUT to indicate that not even a single node can be retrieved since last call. 
        virtual sl_result getScanDataWithIntervalHq(sl_lidar_response_measurement_node_hq_t* nodebuffer, size_t& count) = 0;
        /// Set lidar motor speed
        /// The host system can use this operation to set lidar motor speed.
        ///
        /// \param speed        The speed value set to lidar
        ///
        ///Note: The function will stop scan if speed is DEFAULT_MOTOR_SPEED.
        virtual sl_result setMotorSpeed(sl_u16 speed = DEFAULT_MOTOR_SPEED) = 0;
        
        /// Get the motor information of the RPLIDAR include the max speed, min speed, desired speed.
        /// 
        /// \param motorInfo          The motor information returned from the RPLIDAR
        virtual sl_result getMotorInfo(LidarMotorInfo &motorInfo, sl_u32 timeoutInMs = DEFAULT_TIMEOUT) = 0;
    

        /// Ask the LIDAR to use a new baudrate for serial communication
        /// The target LIDAR system must support such feature to work.
        /// This function does NOT check whether the target LIDAR works with the requiredBaudRate or not.
        /// In order to verifiy the result, use getDeviceInfo or other getXXXX functions instead.
        /// 
        /// \param requiredBaudRate   The new baudrate required to be used. It MUST matches with the baudrate of the binded channel.
        /// \param baudRateDetected   The actual baudrate detected by the LIDAR system
        virtual sl_result negotiateSerialBaudRate(sl_u32 requiredBaudRate, sl_u32* baudRateDetected = NULL) = 0;
};

    /**
    * Create a LIDAR driver instance
    *
    * Example
    * Result<ISerialChannel*> channel = createSerialPortChannel("/dev/ttyUSB0", 115200);
    * assert((bool)channel);
    * assert(*channel);
    *
    * auto lidar = createLidarDriver();
    * assert((bool)lidar);
    * assert(*lidar);
    *
    * auto res = (*lidar)->connect(*channel);
    * assert(SL_IS_OK(res));
    *
    * sl_lidar_response_device_info_t deviceInfo;
    * res = (*lidar)->getDeviceInfo(deviceInfo);
    * assert(SL_IS_OK(res));
    *
    * printf("Model: %d, Firmware Version: %d.%d, Hardware Version: %d\n",
    *        deviceInfo.model,
    *        deviceInfo.firmware_version >> 8, deviceInfo.firmware_version & 0xffu,
    *        deviceInfo.hardware_version);
    *
    * delete *lidar;
    * delete *channel;
    */
    Result<ILidarDriver*> createLidarDriver();
}
