IC_CONFIG                             = mt2523
BOARD_CONFIG                          = mt2523_hdk
SCREEN_CONFIG						  = mt2523_watch_v2
UI_CONFIG 							  = touchgfx

# CTP module on
MTK_CTP_ENABLE = y

# combo sensor of accelerometer and gyroscope
MT2511_E1 = y
MT2511_INTERFACE = SPI
MTK_SENSOR_BIO_USE = MT2511
MTK_SENSOR_BIO_MODULE = SOLTEAMOPTO
HR_PPG_128HZ = y
MTK_SENSOR_ACCELEROMETER_USE = ICM30630

# fusion algorithm
FUSION_PEDOMETER_USE = ICM_PEDOMETER
FUSION_HEART_RATE_MONITOR_USE = M_INHOUSE_HEART_RATE_MONITOR
FUSION_BLOOD_PRESSURE_USE = M_INHOUSE_BLOOD_PRESSURE
CFLAGS += -DFUSION_REPORT_RAW_DATA_ENABLE

# gnss
MTK_GNSS_ENABLE = y

MTK_BT_DUO_ENABLE                   = y
MTK_NVDM_ENABLE         = y
# debug level: none, error, warning, and info
MTK_DEBUG_LEVEL = info

# bt module enable
MTK_BT_ENABLE                       = y
MTK_BLE_ONLY_ENABLE                 = n
MTK_BT_HFP_ENABLE                   = n
MTK_BT_AVRCP_ENABLE                 = y
MTK_BT_AVRCP_ENH_ENABLE             = y
MTK_BT_A2DP_ENABLE                  = y
MTK_BT_PBAP_ENABLE                  = n
MTK_BT_SPP_ENABLE                   = y
MTK_BT_A2DP_SOURCE_ENABLE           = y
MTK_BT_CODEC_ENABLED                = y

# USB
MTK_USB_DEMO_ENABLED = y

#mp3 support
MTK_AUDIO_MP3_ENABLED = y

# Port service
MTK_PORT_SERVICE_ENABLE = y

# battery_management
MTK_SMART_BATTERY_ENABLE = y

# atci via port service
MTK_ATCI_VIA_PORT_SERVICE = y

# TE enable, enable this feature will avoid tearing effect
MTK_TE_ENABLE = y

