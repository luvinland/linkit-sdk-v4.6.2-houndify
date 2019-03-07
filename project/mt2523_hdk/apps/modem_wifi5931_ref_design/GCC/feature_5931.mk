IC_CONFIG                             = mt2523
BOARD_CONFIG                             = mt2523_hdk

# combo sensor of accelerometer and gyroscope
#MTK_SENSOR_ACCELEROMETER_USE = BMI160
#MTK_SENSOR_MAGNETIC_USE = YAS533
#MTK_SENSOR_BAROMETER_USE = BMP280
#MTK_SENSOR_PROXIMITY_USE = CM36672
#MTK_SENSOR_BIO_USE = MT2511
#MTK_SENSOR_ACCELEROMETER_USE = BMA255

# Build SMT Load
MTK_BUILD_SMT_LOAD = y

# battery_management
MTK_SMART_BATTERY_ENABLE = n

# gnss
MTK_GNSS_ENABLE = y

# CTP module on
MTK_CTP_ENABLE = y

# debug level: none, error, warning, and info
MTK_DEBUG_LEVEL = info

# MVG flash test support
MTK_MVG_FLASH_TEST_ENABLE = n

# USB
MTK_USB_DEMO_ENABLED = y

# System service debug feature for internal use
MTK_SUPPORT_HEAP_DEBUG              = n
MTK_HEAP_SIZE_GUARD_ENABLE          = n

# Development board type: EVB, HDK
MTK_DEVELOPMENT_BOARD_TYPE = EVB

# SDK version
MTK_FW_VERSION                      = SDK_VER_IOT_SDK_DEV

#SDK version query cmd
MTK_QUERY_SDK_VERSION = n

# port service
MTK_PORT_SERVICE_ENABLE = y

# atci via port service
MTK_ATCI_VIA_PORT_SERVICE = y

#MD5 configration
MTK_MBEDTLS_CONFIG_FILE             = config-mtk-basic.h

MTK_NVDM_ENABLE                     = y

MTK_PING_OUT_ENABLE = y

MTK_MODEM_ENABLE = n

MTK_MODEM_ON_HDK_ENABLE = n

MTK_FOTA_ENABLE = y

MTK_HTTPCLIENT_SSL_ENABLE           = y

MTK_WIFI_CHIP_USE_MT5931 = y

MTK_LCD_ENABLE = n

