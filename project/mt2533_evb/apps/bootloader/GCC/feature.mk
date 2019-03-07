IC_CONFIG                   = mt2533
BOARD_CONFIG                = mt2533_evb
MTK_BL_LOAD_ENABLE          = y

#can modify
MTK_BL_FOTA_CUST_ENABLE     = n
MTK_BL_DEBUG_ENABLE         = y
MTK_FOTA_ENABLE             = y
MTK_FOTA_FS_ENABLE          = n

#use external pmic
MTK_EXTERNAL_PMIC           = y
# external pmic type
ifeq ($(MTK_EXTERNAL_PMIC), y)
#  MTK_EXTERNAL_PMIC_TYPE = MAX14745
MTK_EXTERNAL_PMIC_TYPE = MT6327
endif


#internal use
MTK_BL_FPGA_LOAD_ENABLE     = n
MTK_BL_FLASH_STT_ENABLE     = n
MTK_BL_FLASH_DIST_ENABLE    = n
MTK_BL_LOG_VIA_UART0        = y

#factory
MTK_CAL_DCXO_CAPID          = n
# DCXO calibration value is in SW
MTK_BL_DCXO_KVALUE_SW       = y

