IC_CONFIG                           = mt5932
BOARD_CONFIG                        = mt5932_hdk
# debug level: none, error, warning, and info
MTK_DEBUG_LEVEL                     = none

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
MTK_WIFI_TGN_VERIFY_ENABLE          = n
MTK_WIFI_PROFILE_ENABLE             = n
MTK_WIFI_SLIM_ENABLE                = y
MTK_SMTCN_V5_ENABLE                 = n
MTK_CM4_WIFI_TASK_ENABLE            = y
MTK_WIFI_ROM_ENABLE                 = y

MTK_WIFI_STUB_CONF_ENABLE           = y
MTK_WFC_AT_SLAVE_ENABLE             = y
#MTK_WIFI_STUB_CONF_SPI_ENABLE      = n  #SPI support between Host&Device, default use SDIO
#MTK_WFC_WITH_LWIP_NO_WIFI_ENABLE   = y  #Host & Device can do lwip communication test directly, RD internal development, just for test purpose
#MTK_WFC_WITH_WIFI_NO_LWIP_ENABLE   = y  #Device no Lwip, it is the default open option,  DHCP&other 802.3 packet will be passed to Host directly


#LWIP features
MTK_IPERF_ENABLE                    = n
MTK_PING_OUT_ENABLE                 = y

#CLI , ATCMD features
MTK_MINICLI_ENABLE                  = n
MTK_CLI_TEST_MODE_ENABLE            = n
MTK_CLI_EXAMPLE_MODE_ENABLE         = n
MTK_CLI_FORK_ENABLE                 = n
MTK_OS_CLI_ENABLE                   = n

MTK_AT_CMD_DISABLE                  = y
MTK_ATCI_ENABLE                     = n

#HAL
MTK_HAL_LOWPOWER_ENABLE             = n

