IC_CONFIG                           = mt5932
BOARD_CONFIG                        = mt5932_hdk
MTK_FW_VERSION                      = mt5932_fw
# debug level: none, error, warning, and info
MTK_DEBUG_LEVEL                     = info

# 3 options with psram/flash or not, only 1 option is y, the others should be n
MTK_MEMORY_WITH_PSRAM_FLASH         = n
MTK_MEMORY_WITHOUT_PSRAM            = n
MTK_MEMORY_WITHOUT_PSRAM_FLASH      = y
MTK_NO_PSRAM_ENABLE                 = y

# System service debug feature for internal use
MTK_SUPPORT_HEAP_DEBUG              = n
MTK_HEAP_SIZE_GUARD_ENABLE          = n
MTK_OS_CPU_UTILIZATION_ENABLE       = y

#NVDM
MTK_NVDM_ENABLE                     = n

#WIFI features
MTK_WIFI_ROM_ENABLE                 = n