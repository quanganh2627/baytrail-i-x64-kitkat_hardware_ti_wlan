
CTRL_INTERFACE  := stad/src/Ctrl_Interface
DATA_LINK       := stad/src/Data_link
STA_MGMT        := stad/src/Sta_Management
AIRLINK_MGMT    := stad/src/AirLink_Managment
CONN_MGMT       := stad/src/Connection_Managment
ROLES_AND_LINKS := stad/src/RolesAndLinks
STAD_APP        := stad/src/Application
TIAP_TEST       := Test
TWD_MAC_SERV    := TWD/MacServices
TWD_DRIVER      := TWD/TWDriver
TWD_CTRL        := TWD/Ctrl
TWD_DATA_SERV   := TWD/Data_Service
TWD_FW_TRANS    := TWD/FW_Transfer
TWD_TWIF        := TWD/TwIf
TIAP_TXN        := Txn
TIAP_UTILS      := utils
PLAT_COMM_SRC   := platforms/os/common/src
PLAT_LINUX_SRC  := platforms/os/linux/src
PLAT_HOST_LINUX := platforms/hw/host_platform_linux/linux

PLAT_COMM_INC   := platforms/os/common/inc
PLAT_LINUX_INC  := platforms/os/linux/inc
TWD_FW_API      := TWD/FirmwareApi
STAD_EXPORT_INC := stad/Export_Inc
TWD_FW_EXPORT   := $(TWD_FW_TRANS)/Export_Inc
TWD_MAC_EXPORT  := $(TWD_MAC_SERV)/Export_Inc
TWD_DATA_EXPORT := $(TWD_DATA_SERV)/Export_Inc
TWD_CTRL_EXPORT := $(TWD_CTRL)/Export_Inc

EXTRA_CFLAGS    += -I$(M)/$(CTRL_INTERFACE)/ \
                   -I$(M)/$(DATA_LINK)/ \
                   -I$(M)/$(STA_MGMT)/ \
                   -I$(M)/$(AIRLINK_MGMT)/ \
                   -I$(M)/$(CONN_MGMT)/ \
                   -I$(M)/$(ROLES_AND_LINKS)/ \
                   -I$(M)/$(STAD_APP)/ \
                   -I$(M)/$(TIAP_TEST)/ \
                   -I$(M)/$(TWD_MAC_SERV)/ \
                   -I$(M)/$(TWD_DRIVER)/ \
                   -I$(M)/$(TWD_CTRL)/ \
                   -I$(M)/$(TWD_DATA_SERV)/ \
                   -I$(M)/$(TWD_FW_TRANS)/ \
                   -I$(M)/$(TWD_TWIF)/ \
                   -I$(M)/$(TIAP_TXN)/ \
                   -I$(M)/$(TIAP_UTILS)/ \
                   -I$(M)/$(PLAT_COMM_SRC)/ \
                   -I$(M)/$(PLAT_LINUX_SRC)/ \
                   -I$(M)/$(PLAT_HOST_LINUX)/ \
                   -I$(M)/$(PLAT_COMM_INC)/ \
                   -I$(M)/$(PLAT_LINUX_INC)/ \
                   -I$(M)/$(STAD_EXPORT_INC)/ \
                   -I$(M)/$(TWD_FW_API)/ \
                   -I$(M)/$(TWD_FW_EXPORT)/ \
                   -I$(M)/$(TWD_MAC_EXPORT)/ \
                   -I$(M)/$(TWD_DATA_EXPORT)/ \
                   -I$(M)/$(TWD_CTRL_EXPORT)/

######################### SDIO DRIVER #########################
obj-m += sdio.o

sdio-objs := $(PLAT_HOST_LINUX)/SdioDrv.o


######################## TIWLAN DRIVER ########################
obj-m += tiwlan_drv.o

tiwlan_drv-objs :=
# stad/src/Ctrl_Interface OK
tiwlan_drv-objs += $(CTRL_INTERFACE)/DrvMain.o \
                   $(CTRL_INTERFACE)/EvHandler.o \
                   $(CTRL_INTERFACE)/CmdHndlr.o \
                   $(CTRL_INTERFACE)/CmdDispatcher.o

# stad/src/Data_link OK
tiwlan_drv-objs += $(DATA_LINK)/txCtrl.o \
                   $(DATA_LINK)/txCtrlServ.o \
                   $(DATA_LINK)/txCtrlParams.o \
                   $(DATA_LINK)/txPort.o \
                   $(DATA_LINK)/txDataQueue.o \
                   $(DATA_LINK)/TxDataClsfr.o \
                   $(DATA_LINK)/txMgmtQueue.o \
                   $(DATA_LINK)/Ctrl.o \
                   $(DATA_LINK)/GeneralUtil.o \
                   $(DATA_LINK)/TrafficMonitor.o \
                   $(DATA_LINK)/rx.o

# stad/src/Sta_Management OK
tiwlan_drv-objs += $(STA_MGMT)/PowerMgr.o \
                   $(STA_MGMT)/PowerMgrKeepAlive.o \
                   $(STA_MGMT)/PowerMgrDbgPrint.o \
                   $(STA_MGMT)/healthMonitor.o \
                   $(STA_MGMT)/siteMgr.o \
                   $(STA_MGMT)/StaCap.o \
                   $(STA_MGMT)/siteHash.o \
                   $(STA_MGMT)/systemConfig.o \
                   $(STA_MGMT)/templates.o \
                   $(STA_MGMT)/mlmeBuilder.o \
                   $(STA_MGMT)/mlmeParser.o \
                   $(STA_MGMT)/mlmeSm.o \
                   $(STA_MGMT)/scr.o \
                   $(STA_MGMT)/ScanCncn.o \
                   $(STA_MGMT)/ScanCncnSm.o \
                   $(STA_MGMT)/ScanCncnSmSpecific.o \
                   $(STA_MGMT)/ScanCncnOsSm.o \
                   $(STA_MGMT)/ScanCncnApp.o \
                   $(STA_MGMT)/scanResultTable.o \
                   $(STA_MGMT)/currBss.o \
                   $(STA_MGMT)/qosMngr.o \
                   $(STA_MGMT)/trafficAdmControl.o \
                   $(STA_MGMT)/mlme.o \
                   $(STA_MGMT)/pwrState.o \
                   $(STA_MGMT)/pwrState_Dbg.o

# stad/src/AirLink_Managment OK
tiwlan_drv-objs += $(AIRLINK_MGMT)/SwitchChannel.o \
                   $(AIRLINK_MGMT)/measurementMgr.o \
                   $(AIRLINK_MGMT)/measurementMgrSM.o \
                   $(AIRLINK_MGMT)/spectrumMngmntMgr.o \
                   $(AIRLINK_MGMT)/requestHandler.o \
                   $(AIRLINK_MGMT)/SoftGemini.o \
                   $(AIRLINK_MGMT)/regulatoryDomain.o \
                   $(AIRLINK_MGMT)/rrmMgr.o

# stad/src/Connection_Managment OK
tiwlan_drv-objs += $(CONN_MGMT)/conn.o \
                   $(CONN_MGMT)/connIbss.o \
                   $(CONN_MGMT)/connInfra.o \
                   $(CONN_MGMT)/sme.o \
                   $(CONN_MGMT)/smeSm.o \
                   $(CONN_MGMT)/smeSelect.o \
                   $(CONN_MGMT)/rsn.o \
                   $(CONN_MGMT)/apConn.o

# stad/src/Application OK
tiwlan_drv-objs += $(STAD_APP)/scanMngr.o \
                   $(STAD_APP)/roamingMngr.o \
                   $(STAD_APP)/roamingMngr_manualSM.o \
                   $(STAD_APP)/roamingMngr_autoSM.o

# Test OK
tiwlan_drv-objs += $(TIAP_TEST)/connDebug.o \
                   $(TIAP_TEST)/siteMgrDebug.o \
                   $(TIAP_TEST)/ctrlDbg.o \
                   $(TIAP_TEST)/RxDbg.o \
                   $(TIAP_TEST)/TxDbg.o \
                   $(TIAP_TEST)/debug.o \
                   $(TIAP_TEST)/PowerMgrDebug.o \
                   $(TIAP_TEST)/rsnDbg.o \
                   $(TIAP_TEST)/TWD_Debug.o \
                   $(TIAP_TEST)/measurementDbg.o \
                   $(TIAP_TEST)/qosMngrDbg.o \
                   $(TIAP_TEST)/ScanMngrDbg.o \
                   $(TIAP_TEST)/roamingMgrDebug.o \
                   $(TIAP_TEST)/scrDbg.o \
                   $(TIAP_TEST)/SoftGeminiDbg.o \
                   $(TIAP_TEST)/HealthMonitorDbg.o \
                   $(TIAP_TEST)/smeDebug.o \
                   $(TIAP_TEST)/fwdriverdebug.o \
                   $(TIAP_TEST)/scrDbg.o \
                   $(TIAP_TEST)/SoftGeminiDbg.o \
                   $(TIAP_TEST)/HealthMonitorDbg.o \
                   $(TIAP_TEST)/smeDebug.o \
                   $(TIAP_TEST)/fwdriverdebug.o \
                   $(TIAP_TEST)/MibDbg.o \
                   $(TIAP_TEST)/TwIfDebug.o

# TwD/MacServices OK
tiwlan_drv-objs += $(TWD_MAC_SERV)/MeasurementSrv.o \
                   $(TWD_MAC_SERV)/MeasurementSrvSM.o \
                   $(TWD_MAC_SERV)/measurementSrvDbgPrint.o

# TwD/TWDriver OK
tiwlan_drv-objs += $(TWD_DRIVER)/TWDriver.o \
                   $(TWD_DRIVER)/TWDriverTx.o \
                   $(TWD_DRIVER)/TWDriverCtrl.o \
                   $(TWD_DRIVER)/TWDriverRadio.o

# TwD/Ctrl OK
tiwlan_drv-objs += $(TWD_CTRL)/CmdQueue.o \
                   $(TWD_CTRL)/CmdBld.o \
                   $(TWD_CTRL)/CmdBldCfg.o \
                   $(TWD_CTRL)/CmdBldCmd.o \
                   $(TWD_CTRL)/CmdBldItr.o \
                   $(TWD_CTRL)/CmdBldCfgIE.o \
                   $(TWD_CTRL)/CmdBldCmdIE.o \
                   $(TWD_CTRL)/CmdBldItrIE.o

# TwD/Data_Service OK
tiwlan_drv-objs += $(TWD_DATA_SERV)/txHwQueue.o \
                   $(TWD_DATA_SERV)/txCtrlBlk.o \
                   $(TWD_DATA_SERV)/RxQueue.o

# TwD/FW_Transfer OK
tiwlan_drv-objs += $(TWD_FW_TRANS)/CmdMBox.o \
                   $(TWD_FW_TRANS)/txResult.o \
                   $(TWD_FW_TRANS)/txXfer.o \
                   $(TWD_FW_TRANS)/FwEvent.o \
                   $(TWD_FW_TRANS)/RxXfer.o \
                   $(TWD_FW_TRANS)/eventMbox.o \
                   $(TWD_FW_TRANS)/HwInit.o \
                   $(TWD_FW_TRANS)/fwDebug.o

# TwD/TwIf OK
tiwlan_drv-objs += $(TWD_TWIF)/TwIf.o

# Txn OK
tiwlan_drv-objs += $(TIAP_TXN)/TxnQueue.o \
                   $(TIAP_TXN)/SdioBusDrv.o

# utils OK
tiwlan_drv-objs += $(TIAP_UTILS)/fsm.o \
                   $(TIAP_UTILS)/GenSM.o \
                   $(TIAP_UTILS)/report.o \
                   $(TIAP_UTILS)/queue.o \
                   $(TIAP_UTILS)/context.o \
                   $(TIAP_UTILS)/timer.o \
                   $(TIAP_UTILS)/rate.o \
                   $(TIAP_UTILS)/freq.o \
                   $(TIAP_UTILS)/version.o

# platforms/os/common/src OK
tiwlan_drv-objs += $(PLAT_COMM_SRC)/osRgstry.o

# platforms/os/linux/src OK
tiwlan_drv-objs += $(PLAT_LINUX_SRC)/WlanDrvIf.o \
                   $(PLAT_LINUX_SRC)/osapi.o \
                   $(PLAT_LINUX_SRC)/osmemapi.o \
                   $(PLAT_LINUX_SRC)/osRgstry_parser.o \
                   $(PLAT_LINUX_SRC)/ipc_k.o \
                   $(PLAT_LINUX_SRC)/WlanDrvWext.o \
                   $(PLAT_LINUX_SRC)/CmdInterpretWext.o \
                   $(PLAT_LINUX_SRC)/RxBuf.o

# platforms/hw/host_platform_linux/linux OK
tiwlan_drv-objs += $(PLAT_HOST_LINUX)/host_platform.o \
                   $(PLAT_HOST_LINUX)/SdioAdapter.o

EXTRA_CFLAGS += -D__LINUX__ -DTI_DBG -DREPORT_LOG -D__BYTE_ORDER_LITTLE_ENDIAN -DHOST_COMPILE -DFW_RUNNING_AS_STA -DTNETW1283 -DGEM_SUPPORTED
