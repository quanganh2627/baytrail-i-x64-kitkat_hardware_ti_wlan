#!/usr/bin/perl

##############################################################################
# Set the default parameters
##############################################################################
$pkg_temp_dir = 'WiLink';
$base_dir = '/vobs/WiLink';
$out_file = 'wilink61.zip';

$ccx = 0;
$gem = 0;
$twd = 0;
$win = 0;
$gpl = 0;


##############################################################################
# Files and directories
##############################################################################
@root_dirs = ('platforms', 'stad', 'TWD', 'utils', 'Test', 'fw', 'CUDK');
@root_files = ('dirs', 'sources.cmn', 'sources_cudk.cmn');

@external_drivers_dirs = (
                          'external_drivers',
                          'external_drivers/omap2430',
                          'external_drivers/zoom2'
                          );

@external_drivers_files = (                           
                           );

@external_apps_dirs = ('external_apps');

@external_apps_files = ();

@fw_dirs = (
            'fw/Latest'
            );

@fw_files = (
            'fw/Latest/*.bin'
            );

@platforms_dirs = ('platforms/hw', 
                   'platforms/os', 
                   'platforms/hw/host_platform_omap2430',
                   'platforms/hw/host_platform_zoom2',
                   'platforms/os/common',
                   'platforms/os/common/inc',
                   'platforms/os/common/src'
                   );

@platforms_files = (
                    'platforms/os/common/inc/*.h',
                    'platforms/os/common/src/*.c'
                    );


@stad_dirs = (
              'stad/Export_Inc',
              'stad/src',
              'stad/src/AirLink_Managment',
              'stad/src/Application',
              'stad/src/Connection_Managment',
              'stad/src/Ctrl_Interface',
              'stad/src/Data_link',
              'stad/src/Sta_Management'
              );

@stad_files = (
              'stad/Export_Inc/*.h',
              'stad/src/AirLink_Managment/*.c',
              'stad/src/AirLink_Managment/*.h',
              'stad/src/Application/*.c',
              'stad/src/Application/*.h',
              'stad/src/Connection_Managment/*.c',
              'stad/src/Connection_Managment/*.h',
              'stad/src/Ctrl_Interface/*.c',
              'stad/src/Ctrl_Interface/*.h',
              'stad/src/Data_link/*.c',
              'stad/src/Data_link/*.h',
              'stad/src/Sta_Management/*.c',
              'stad/src/Sta_Management/*.h'
              );

@test_dirs = ();
@test_files = (
               'Test/*.c',
               'Test/*.h',
               );

@test_files_linux = (
               'Test/*.c',
               'Test/*.h',
               );

@twd_dirs = (
             'TWD/Ctrl',
             'TWD/Ctrl/Export_Inc',
             'TWD/Data_Service',
             'TWD/Data_Service/Export_Inc',
             'TWD/FirmwareApi',
             'TWD/FW_Transfer',
             'TWD/FW_Transfer/Export_Inc',
             'TWD/MacServices',
             'TWD/MacServices/Export_Inc',
             'TWD/TwIf',
             'TWD/TWDriver',
             'Txn'
             );

@twd_files = (
            'TWD/Ctrl/*.c',
            'TWD/Ctrl/*.h',
            'TWD/Ctrl/Export_Inc/*.h',
            'TWD/Data_Service/*.c',
            'TWD/Data_Service/Export_Inc/*.h',
            'TWD/FirmwareApi/*.h',
            'TWD/FW_Transfer/*.c',
            'TWD/FW_Transfer/*.h',
            'TWD/FW_Transfer/Export_Inc/*.h',
            'TWD/MacServices/*.c',
            'TWD/MacServices/*.h',
            'TWD/MacServices/Export_Inc/*.h',
            'TWD/TwIf/*.c',
            'TWD/TwIf/*.h',
            'TWD/TWDriver/*.c',
            'TWD/TWDriver/*.h',
            'Txn/*.c',
            'Txn/*.h'
             );

@utils_dirs = ();
@utils_files = (
               'utils/*.c',
               'utils/*.h',
               );

@cudk_dirs = (
              'CUDK/configurationutility',
              'CUDK/configurationutility/inc',
              'CUDK/configurationutility/src',
              'CUDK/cu_common',
              'CUDK/os',
              'CUDK/os/common',
              'CUDK/os/common/inc'
              );

@cudk_dirs_linux = (
              'CUDK/configurationutility',
              'CUDK/configurationutility/inc',
              'CUDK/configurationutility/src',
              'CUDK/os',
              'CUDK/os/common',
              'CUDK/os/common/inc'
              );


@cudk_files = (
              'CUDK/configurationutility/inc/*.h',
              'CUDK/configurationutility/src/*.c',
              'CUDK/cu_common/*.c',
              'CUDK/os/common/inc/*.h',
               );
@cudk_files_linux = (
              'CUDK/configurationutility/inc/*.h',
              'CUDK/configurationutility/src/*.c',
              'CUDK/os/common/inc/*.h',
               );


@ccx_dirs = (
             'CUDK/configurationutility/ccx',
             'stad/src/CCX',
             'stad/Export_Inc/CCX',
             'utils/CCX'
             );


@ccx_files_win = (
              'CUDK/configurationutility/ccx/makefile',
              'CUDK/configurationutility/ccx/sources',
              'stad/src/CCX/makefile',
              'stad/src/CCX/sources',
              );
@ccx_files = (
              'CUDK/configurationutility/ccx/*.h',
              'CUDK/configurationutility/ccx/*.c',
              'stad/src/CCX/*.c',
              'stad/src/CCX/*.h',
              'stad/Export_Inc/CCX/*.h',
              'utils/CCX/*.h'
              );

@win_dirs = (
            'external_drivers/omap2430/WinMobile',
            'external_drivers/omap2430/WinMobile/sdio',
            'external_drivers/omap2430/WinMobile/sdio/Drv',
            'external_apps/WinMobile',
            'external_apps/WinMobile/bmtrace',
            'external_apps/WinMobile/bmtrace/bmtraceApp',
            'external_apps/WinMobile/bmtrace/bmtraceDll',
            'platforms/hw/host_platform_omap2430/winmobile',
            'platforms/os/winmobile',
            'platforms/os/winmobile/Export_Inc',
            'platforms/os/winmobile/src',
            'CUDK/os/win',
            'CUDK/os/win/inc',
            'CUDK/os/win/src',
            'CUDK/logger',
            'CUDK/logger/winmobile',
            'CUDK/logger/winmobile/inc',
            'CUDK/logger/winmobile/src',
             );

@win_files = (
              'external_drivers/omap2430/WinMobile/sdio/Drv/SdioDrvDbg.h',
              'external_drivers/omap2430/WinMobile/sdio/Drv/SdioDrv.h',
              'external_apps/dirs',
              'external_apps/WinMobile/dirs',
              'external_apps/WinMobile/bmtrace/dirs',
              'external_apps/WinMobile/bmtrace/bmtraceApp/bmtraceApp.c',
              'external_apps/WinMobile/bmtrace/bmtraceApp/makefile',
              'external_apps/WinMobile/bmtrace/bmtraceApp/sources',
              'external_apps/WinMobile/bmtrace/bmtraceApp/sources.cmn',
              'external_apps/WinMobile/bmtrace/bmtraceDll/bmtrace.c',
              'external_apps/WinMobile/bmtrace/bmtraceDll/bmtrace.h',
              'external_apps/WinMobile/bmtrace/bmtraceDll/bmtrace.def',
              'external_apps/WinMobile/bmtrace/bmtraceDll/makefile',
              'external_apps/WinMobile/bmtrace/bmtraceDll/sources',
              'external_apps/WinMobile/bmtrace/bmtraceDll/sources.cmn',
              'platforms/dirs',
              'platforms/hw/dirs',
              'platforms/hw/host_platform_omap2430/dirs',
              'platforms/hw/host_platform_omap2430/winmobile/makefile',
              'platforms/hw/host_platform_omap2430/winmobile/sources',
              'platforms/hw/host_platform_omap2430/winmobile/host_platform.c',
              'platforms/hw/host_platform_omap2430/winmobile/host_platform.h',
              'platforms/hw/host_platform_omap2430/winmobile/SdioAdapter.c',
              'platforms/hw/host_platform_omap2430/winmobile/SdioAdapter.h',
              'platforms/os/dirs',
              'platforms/os/winmobile/dirs',
              'platforms/os/winmobile/Export_Inc/*.h',
              'platforms/os/winmobile/src/makefile',
              'platforms/os/winmobile/src/sources',
              'platforms/os/winmobile/src/*.c',
              'platforms/os/winmobile/src/*.h',
              'platforms/os/winmobile/src/*.def',
              'platforms/os/common/src/makefile',
              'platforms/os/common/src/sources',
              'platforms/os/common/dirs',
              'stad/src/dirs',
              'stad/src/Data_link/makefile',
              'stad/src/Data_link/sources',
              'stad/src/AirLink_Managment/makefile',
              'stad/src/AirLink_Managment/sources',
              'stad/src/Application/makefile',
              'stad/src/Application/sources',
              'stad/src/Connection_Managment/makefile',
              'stad/src/Connection_Managment/sources',
              'stad/src/Ctrl_Interface/makefile',
              'stad/src/Ctrl_Interface/sources',
              'stad/src/Sta_Management/makefile',
              'stad/src/Sta_Management/sources',
              'stad/dirs',
              'TWD/dirs',
              'TWD/Ctrl/makefile',
              'TWD/Ctrl/sources',
              'TWD/Data_Service/makefile',
              'TWD/Data_Service/sources',
              'TWD/FW_Transfer/makefile',
              'TWD/FW_Transfer/sources',
              'TWD/MacServices/makefile',
              'TWD/MacServices/sources',
              'TWD/TwIf/makefile',
              'TWD/TwIf/sources',
              'TWD/TWDriver/makefile',
              'TWD/TWDriver/sources',
              'Txn/sources',
              'Txn/makefile',
              'utils/makefile',
              'utils/sources',
              'Test/makefile',
              'Test/sources',
              'CUDK/dirs',
              'CUDK/configurationutility/dirs',
              'CUDK/configurationutility/makefile',
              'CUDK/configurationutility/src/makefile',
              'CUDK/configurationutility/src/sources',
              'CUDK/cu_common/makefile',
              'CUDK/cu_common/sources',
              'CUDK/logger/dirs',
              'CUDK/logger/winmobile/dirs',
              'CUDK/logger/winmobile/src/sources',
              'CUDK/logger/winmobile/src/makefile',
              'CUDK/os/win/dirs',
              'CUDK/os/win/inc/*.h',
              'CUDK/os/win/src/makefile',
              'CUDK/os/win/src/sources',
              'CUDK/os/win/src/*.c',
              'CUDK/logger/winmobile/src/*.c',
              'CUDK/logger/winmobile/inc/*.h'
              );


@images_dirs = (
             'platforms/os/linux/images',
             'platforms/os/linux/images/omap2430'
             );

@lin_dirs_open_source = (
             'CUDK/wpa_suppl',
             'CUDK/wpa_suppl/openssl-0.9.8e',
             );
@lin_dirs_open_source2 = (
             'CUDK/gem_suppl',
             'CUDK/gem_suppl/openssl-0.9.8e',
             );

@lin_dirs_core = (
             'CUDK/output',
             'CUDK/configurationutility',
             'CUDK/tiwlan_loader',
             'CUDK/os/linux',
             'CUDK/os/linux/src',
             'CUDK/os/linux/inc',
             'CUDK/logger',
             'CUDK/logger/linux',
             'CUDK/logger/linux/inc',
             'CUDK/logger/linux/src'
             );

@lin_files_core = (
              'Android.mk',
              'CUDK/Makefile',
              'CUDK/Android.mk',
              'CUDK/os/Android.mk',
              'CUDK/os/linux/Android.mk',
              'CUDK/os/linux/src/*.c',
              'CUDK/os/linux/src/makefile',
              'CUDK/os/linux/src/Android.mk',
              'CUDK/os/linux/inc/*.h',
              'CUDK/configurationutility/makefile',
              'CUDK/configurationutility/Android.mk',
              'CUDK/tiwlan_loader/*.c',
              'CUDK/tiwlan_loader/Makefile',
              'CUDK/tiwlan_loader/Android.mk',
              'CUDK/logger/linux/inc/*.h',
              'CUDK/logger/linux/src/*.c',
              'CUDK/logger/linux/makefile'
             );


@lin_dirs_osa = (
             'external_drivers/omap2430/Linux',
             'external_drivers/omap2430/Linux/sdio',
             'external_drivers/zoom2/Linux',
             'external_drivers/zoom2/Linux/sdio',
             'external_apps/Linux',
             'external_apps/Linux/bmtrace',
             'external_apps/Linux/bmtrace/Drv',
             'platforms/hw/host_platform_omap2430/linux',
             'platforms/hw/host_platform_zoom2/linux',
             'platforms/os/linux',
             'platforms/os/linux/build',
             'platforms/os/linux/inc',
             'platforms/os/linux/src',
             'platforms/os/common/build',
             'platforms/os/common/build/linux',
             'stad/build',
             'stad/build/linux'
             );

@lin_files_osa = (
              'external_drivers/omap2430/Linux/sdio/SdioDrv.[ch]',
              'external_drivers/omap2430/Linux/sdio/SdioDrvDbg.h',
              'external_drivers/omap2430/Linux/sdio/testdrv.[ch]',
              'external_drivers/omap2430/Linux/sdio/Makefile',
              'external_drivers/zoom2/Linux/sdio/SdioDrv.[ch]',
              'external_drivers/zoom2/Linux/sdio/SdioDrvDbg.h',
              'external_drivers/zoom2/Linux/sdio/host_platform.h',
              'external_drivers/zoom2/Linux/sdio/testdrv.[ch]',
              'external_drivers/zoom2/Linux/sdio/Makefile',
              'external_apps/Linux/bmtrace/Drv/bmtrace.c',
              'external_apps/Linux/bmtrace/Drv/Makefile',
              'platforms/hw/host_platform_omap2430/linux/*.c',
              'platforms/hw/host_platform_omap2430/linux/*.h',
              'platforms/hw/host_platform_zoom2/linux/*.c',
              'platforms/hw/host_platform_zoom2/linux/*.h',
              'platforms/os/linux/Makefile',
              'platforms/os/linux/Android.mk',
              'platforms/os/linux/build/*.inc',
              'platforms/os/linux/build/Makefile',
              'platforms/os/linux/omap2430_env.tcsh',
              'platforms/os/linux/zoom2_env.bash',
              'platforms/os/linux/tiwlan.ini',
              'platforms/os/linux/tiwlan_dual.ini',
              'platforms/os/linux/inc/*.h',
              'platforms/os/linux/src/*.h',
              'platforms/os/linux/src/*.c',
              'platforms/os/common/build/linux/*.inc',
              'platforms/os/common/build/linux/Makefile',
              'stad/build/linux/*.inc',
              'stad/build/linux/Makefile'
             );


@images_files = (
              'platforms/os/linux/images/omap2430/dhcpcd',
              'platforms/os/linux/images/omap2430/udhcpc',
              'platforms/os/linux/images/omap2430/dotnet_personal_test.cer',
              'platforms/os/linux/images/omap2430/dotnet_personal_test.pem',
              'platforms/os/linux/images/omap2430/dotnet_root.cer',
              'platforms/os/linux/images/omap2430/wfa_dut',
              'platforms/os/linux/images/omap2430/wpa_supplicant_wpa_tls.txt'
             );

@ccx_images_files = (
              'platforms/os/linux/images/omap2430/pfx_password.txt',
              'platforms/os/linux/images/omap2430/acs_root_certificate.cer',
              'platforms/os/linux/images/omap2430/acs_ccx_consol.cer',
              'platforms/os/linux/images/omap2430/acs_ccx_consol.pem',
              'platforms/os/linux/images/omap2430/acs_ccx_consol.pfx'
             );

@lin_files_open_source = (
              'CUDK/wpa_suppl/.config',
              'CUDK/wpa_suppl/bufferObj.c',
              'CUDK/wpa_suppl/bufferObj.h',
              'CUDK/wpa_suppl/wsc_supplicant.c',
              'CUDK/wpa_suppl/wsc_supplicant.h',
              'CUDK/wpa_suppl/CipherWrapper.c',
              'CUDK/wpa_suppl/CipherWrapper.h',
              'CUDK/wpa_suppl/eap_wsc.c',
              'CUDK/wpa_suppl/WscTypes.h',
              'CUDK/wpa_suppl/wpa_suppl.diff',
              'CUDK/wpa_suppl/openssl-0.9.8e/openssl-gcc42.diff',
              'CUDK/wpa_suppl/openssl-0.9.8e/Makefile',
              'platforms/os/linux/wpa_supplicant.txt'
             );

@lin_files_open_source2 = (
              'CUDK/gem_suppl/.config',
              'CUDK/gem_suppl/bufferObj.c',
              'CUDK/gem_suppl/bufferObj.h',
              'CUDK/gem_suppl/wsc_supplicant.c',
              'CUDK/gem_suppl/wsc_supplicant.h',
              'CUDK/gem_suppl/CipherWrapper.c',
              'CUDK/gem_suppl/CipherWrapper.h',
              'CUDK/gem_suppl/eap_wsc.c',
              'CUDK/gem_suppl/WscTypes.h',
              'CUDK/gem_suppl/eap_wsc.c',
              'CUDK/gem_suppl/ec.c',
              'CUDK/gem_suppl/ec.h',
              'CUDK/gem_suppl/gem.c',
              'CUDK/gem_suppl/gem.h',
              'CUDK/gem_suppl/wapi.c',
              'CUDK/gem_suppl/wapi.h',
              'CUDK/gem_suppl/wpa_suppl.diff',
              'CUDK/gem_suppl/openssl-0.9.8e/openssl-gcc42.diff',
              'CUDK/gem_suppl/openssl-0.9.8e/Makefile',
              'platforms/os/linux/wpa_supplicant.txt'
             );

@bsp_dirs = (
             'files',
             'files/smartfon',
             'src',
             'src/inc',
             'src/boot',
             'src/boot/xldr',
             'src/csp',
             'src/csp/inc',
             'src/csp/omap2430',
             'src/csp/omap2430/bthci',
             'src/csp/omap2430/bthci/brf6300',
             'src/csp/omap2430/bus',
             'src/csp/omap2430/bus/omap2430bus',
             'src/csp/omap2430/inc',
             'src/csp/omap2430/oal',
             'src/csp/omap2430/oal/intr',
             'src/csp/omap2430/oal/power',
             'src/csp/omap2430/sdhc',
             'src/csp/omap2430/sdhc/base',
             'src/csp/omap2430/sdhc/impl',
             'src/csp/omap2430/sdio',
             'src/csp/omap2430/sdio/Drv',
             'src/csp/omap2430/sdio/DrvTest',
             'src/csp/omap2430/sdio/DrvTestApp',
             'src/csp/omap2430/uart',
             'src/csp/omap2430/usbotg',
             'src/drivers',
             'src/drivers/fmc',
             'src/drivers/fmc/common',
             'src/drivers/fmc/fm_rx_demo',
             'src/drivers/fmc/fm_tx_demo',
             'src/drivers/fmc/fmc_dll',
             'src/drivers/fmc/fmc_evtwrapper',
             'src/drivers/fmc/fmc_rx_api',
             'src/drivers/fmc/fmc_tx_api',
             'src/drivers/fmc/fmhal',
             'src/drivers/fmc/inc',
             'src/drivers/fmc/inc/common',
             'src/drivers/fmc/inc/int',
             'src/drivers/fmc/rx',
             'src/drivers/fmc/tx',
             'src/drivers/bthci',
             'src/drivers/bthci/brf6300',
             'src/drivers/gpio',
             'src/drivers/TI_WIFI'
             );

@bsp_files = (
              'SDP2430.bat',
              'files/*',
              'files/smartfon/oem.cpm.csv',
              'src/boot/xldr/platform.c',
              'src/inc/*',
              'src/csp/inc/*',
              'src/csp/omap2430/dirs',
              'src/csp/omap2430/bthci/dirs',
              'src/csp/omap2430/bthci/brf6300/*',
              'src/csp/omap2430/bus/omap2430bus/omap2430bus.cpp',
              'src/csp/omap2430/inc/*',
              'src/csp/omap2430/oal/intr/intr.c',
              'src/csp/omap2430/oal/power/power.c',
              'src/csp/omap2430/sdhc/dirs',
              'src/csp/omap2430/sdhc/base/*',
              'src/csp/omap2430/sdhc/impl/*',
              'src/csp/omap2430/sdio/*',
              'src/csp/omap2430/sdio/Drv/*',
              'src/csp/omap2430/sdio/DrvTest/*',
              'src/csp/omap2430/sdio/DrvTestApp/*',
              'src/csp/omap2430/uart/*',
              'src/csp/omap2430/usbotg/*',
              'src/drivers/dirs',
              'src/drivers/fmc/*',
              'src/drivers/fmc/common/*',
              'src/drivers/fmc/fm_rx_demo/*',
              'src/drivers/fmc/fm_tx_demo/*',
              'src/drivers/fmc/fmc_dll/*',
              'src/drivers/fmc/fmc_evtwrapper/*',
              'src/drivers/fmc/fmc_rx_api/*',
              'src/drivers/fmc/fmc_tx_api/*',
              'src/drivers/fmc/fmhal/*',
              'src/drivers/fmc/inc/*',
              'src/drivers/fmc/inc/common/*',
              'src/drivers/fmc/inc/int/*',
              'src/drivers/fmc/rx/*',
              'src/drivers/fmc/tx/*',
              'src/drivers/bthci/dirs',
              'src/drivers/bthci/brf6300/*',
              'src/drivers/gpio/*',
              'src/drivers/TI_WIFI/*'
             );

##############################################################################
# Full package (no CCX)
##############################################################################
@full_pkg_dirs_noccx = (@root_dirs, @external_drivers_dirs, @external_apps_dirs, @fw_dirs, @platforms_dirs, @stad_dirs, @test_dirs, @twd_dirs, @utils_dirs, @cudk_dirs);
@full_pkg_files_noccx = (@root_files, @external_drivers_files, @external_apps_files, @fw_files, @platforms_files, @stad_files, @test_files, @twd_files, @utils_files, @cudk_files);
@full_pkg_dirs_noccx_linux = (@root_dirs, @external_drivers_dirs, @external_apps_dirs, @fw_dirs, @platforms_dirs, @stad_dirs, @test_dirs, @twd_dirs, @utils_dirs, @cudk_dirs_linux);
@full_pkg_files_noccx_linux = (@external_drivers_files, @external_apps_files, @fw_files, @platforms_files, @stad_files, @test_files_linux, @twd_files, @utils_files, @cudk_files_linux);

@lin_dirs = (@lin_dirs_core, @lin_dirs_osa);
@lin_files = (@lin_files_core, @lin_files_osa);

##############################################################################
# Full package (CCX)
##############################################################################
@full_pkg_dirs_ccx = (@full_pkg_dirs_noccx, @ccx_dirs);
@full_pkg_files_ccx = (@full_pkg_files_noccx, @ccx_files, @ccx_files_win);

@full_pkg_dirs_ccx_linux = (@full_pkg_dirs_noccx_linux, @ccx_dirs);
@full_pkg_files_ccx_linux = (@full_pkg_files_noccx_linux, @ccx_files, @ccx_images_files);


##############################################################################
# TWD only package
##############################################################################
@twd_pkg_dirs = (
                 'TWD', 
                 'platforms',
                 'stad',
                 'stad/Export_Inc',
                 @platforms_dirs,
                 'platforms/os/winmobile',
                 'platforms/os/winmobile/Export_Inc/',
                 'platforms/hw/host_platform_omap2430/winmobile/',
                 @twd_dirs, 
                 @external_drivers_dirs,
                 'utils'
                 );
@twd_pkg_files = (
                  'dirs', 
                  'sources.cmn', 
                  @twd_files, 
                  @external_drivers_files,
                  'utils/*.h',
                  'platforms/os/common/inc/osApi.h',
                  'platforms/os/common/inc/tracebuf_api.h',
                  'platforms/os/winmobile/Export_Inc/*.h',
                  'platforms/hw/host_platform_omap2430/winmobile/*.h',
                  'stad/Export_Inc/TI_IPC_Api.h'
                  );

@twd_pkg_dirs_ccx = (@twd_pkg_dirs, 'utils/CCX');
@twd_pkg_files_ccx = (@twd_pkg_files, 'utils/CCX/*.h');


##############################################################################
# Parse command line
##############################################################################
for($i=0; $i<@ARGV; $i+=1)
{
    if($ARGV[$i] eq "-t")
    {
        $pkg_temp_dir = $ARGV[$i+1];
        $i+=1;
    }
    elsif($ARGV[$i] eq "-o")
    {
        $out_file = $ARGV[$i+1];
        $i+=1;
    }  
    elsif($ARGV[$i] eq "-ccx")
    {
        $ccx = 1;
    }   
    elsif($ARGV[$i] eq "-gem")
    {
        $gem = 1;
        $lin = 1;
        $win = 0;
        $ccx = 0;
        $gpl = 0;
    }   
    elsif($ARGV[$i] eq "-twd")
    {
        $twd = 1;
    }
    elsif($ARGV[$i] eq "-gpl")
    {
        $lin = 1;
        $win = 0;
        $ccx = 0;
        $gpl = 1;
    }
    elsif($ARGV[$i] eq "-lin")
    {
        $lin = 1;
        $win = 0;
    }
    else
    {
        print "\nUsage: perl $0 [OPTIONS]\n\n";
        print "-t <dir>        temp. directory        [default: WiLink]\n";
        print "-o [file]       package file. [default: wilink61.zip]\n";
        print "-ccx            build CCX package (default - no CCX)\n";
        print "-twd            build TWD only package\n";
        print "-lin            build linux package (default - WinMobile)\n";
        print "-gpl            build GPL package (default - no GPL)\n";
        die   "-h              this help screen\n\n";
    }        

}


##############################################################################
# Create temp. directory
##############################################################################

if($win == 0){
     if($ccx == 0){
        if($gpl == 0){
           # Core part 
           @pkg_dirs = (@full_pkg_dirs_noccx_linux, @lin_dirs_core, @images_dirs, @lin_dirs_open_source);
           @pkg_files = (@full_pkg_files_noccx_linux, @lin_files_core, @images_files, @lin_files_open_source);
           $out_file = 'WiLink61_Linux_noCCX_core.tar.bz2';

           # OSA part
           @pkg_dirs_osa  = (@lin_dirs_osa);
           @pkg_files_osa = (@lin_files_osa);
           $out_file_osa  = 'WiLink61_Linux_noCCX_osa.tar.bz2';
        }
        else {
           @pkg_dirs = (@full_pkg_dirs_noccx_linux, @lin_dirs, @lin_dirs_open_source);
           @pkg_files = (@full_pkg_files_noccx_linux, @lin_files, @lin_files_open_source);
           $out_file = 'WiLink61_Linux_OpenSource.tar.bz2';
        }
     }
        else
        {
           # Core part 
           @pkg_dirs = (@full_pkg_dirs_ccx_linux, @lin_dirs_core, @images_dirs, @lin_dirs_open_source);
           @pkg_files = (@full_pkg_files_ccx_linux, @lin_files_core, @images_files, @lin_files_open_source);
           $out_file = 'WiLink61_Linux_CCX_core.tar.bz2';

           # OSA part
           @pkg_dirs_osa  = (@lin_dirs_osa);
           @pkg_files_osa = (@lin_files_osa);
           $out_file_osa  = 'WiLink61_Linux_CCX_osa.tar.bz2';

        }
 }
if($gem == 1){

           # Core part 
           @pkg_dirs = (@full_pkg_dirs_noccx_linux, @lin_dirs_core, @images_dirs, @lin_dirs_open_source2);
           @pkg_files = (@full_pkg_files_noccx_linux, @lin_files_core, @images_files, @lin_files_open_source2);
           $out_file = 'WiLink61_Linux_Gem_core.tar.bz2';

           # OSA part
           @pkg_dirs_osa  = (@lin_dirs_osa);
           @pkg_files_osa = (@lin_files_osa);
           $out_file_osa  = 'WiLink61_Linux_Gem_osa.tar.bz2';

}

system("rm -drf $pkg_temp_dir");
system("rm -f $out_file");
system("mkdir $pkg_temp_dir");



#if($twd == 1) {
#   if($ccx == 1) {
#      @pkg_dirs = @twd_pkg_dirs_ccx;
#      @pkg_files = @twd_pkg_files_ccx;
#   }
#   else {
#      @pkg_dirs = @twd_pkg_dirs;
#      @pkg_files = @twd_pkg_files;
#   }
#}
#else {
#   if($ccx == 1) {
#      @pkg_dirs = @full_pkg_dirs_ccx;
#      @pkg_files = @full_pkg_files_ccx;
#   }
#   else {
#      if($win == 0) {
#         @pkg_dirs = @full_pkg_dirs_ccx;
#         @pkg_files = @full_pkg_files_noccx_linux;
#      }
#      @pkg_dirs = @full_pkg_dirs_noccx;
#      @pkg_files = @full_pkg_files_noccx;
#   }
#}

#if($twd == 0) {

#   if($win == 1) {
#      @pkg_dirs = (@pkg_dirs, @win_dirs);
#      @pkg_files = (@pkg_files, @win_files);
#   }
#   else {
#      @pkg_dirs = (@pkg_dirs, @lin_dirs);
#      @pkg_files = (@pkg_files, @lin_files);
#   }
#}




##############################################################################
# Create directory structure
##############################################################################
print "Creating directry structure for CORE package...\n";

for($i=0; $i<@pkg_dirs; $i+=1)
{
   print " making $pkg_temp_dir/$pkg_dirs[$i]\n";
   system("mkdir -p $pkg_temp_dir/$pkg_dirs[$i]");
}

print "\nCopying files...\n";

for($i=0; $i<@pkg_files; $i+=1)
{
   ###########################################################################
   # remove trailing filename
   ###########################################################################
   $dir = "$pkg_temp_dir/$pkg_files[$i]";
   $dir =~ s/\/[^\/|^~]+$//;

   ###########################################################################
   # copy file(s)
   ###########################################################################
   print " copying $base_dir/$pkg_files[$i]\n";
   system("cp -p $base_dir/$pkg_files[$i] $dir");
}

##############################################################################
# If GPL release - change all headers and rename CCX
##############################################################################

if($gpl == 1){
   print " Taking care of GPL headers\n";
   system("./scripts/package-gpl-noccx");
}

##############################################################################
# Compress the directory
##############################################################################
#system("zip -r -9 $out_file $pkg_temp_dir/*");
print "\nmake the tar file of $out_file $pkg_temp_dir...\n";
system("tar cjf $out_file $pkg_temp_dir/*");

##############################################################################
# Delete temp. directory
##############################################################################
print "\nDeleting temp. directory...\n";
system("rm -drf $pkg_temp_dir");
print "Done.\n";


##############################################################################
# OSA package
##############################################################################
if(($win == 0) && ($gpl == 0)) {

   print "Creating directry structure for OSA package...\n";
   
   for($i=0; $i<@pkg_dirs_osa; $i+=1)
   {
      print " making $pkg_temp_dir/$pkg_dirs_osa[$i]\n";
      system("mkdir -p $pkg_temp_dir/$pkg_dirs_osa[$i]");
   }
   
   print "\nCopying files...\n";
   
   for($i=0; $i<@pkg_files_osa; $i+=1)
   {
      ###########################################################################
      # remove trailing filename
      ###########################################################################
      $dir = "$pkg_temp_dir/$pkg_files_osa[$i]";
      $dir =~ s/\/[^\/|^~]+$//;
   
      ###########################################################################
      # copy file(s)
      ###########################################################################
      print " copying $base_dir/$pkg_files_osa[$i]\n";
      system("cp -p $base_dir/$pkg_files_osa[$i] $dir");
   }
      
   ##############################################################################
   # Compress the directory
   ##############################################################################
   system("tar cjf $out_file_osa $pkg_temp_dir/*");
   
   ##############################################################################
   # Delete temp. directory
   ##############################################################################
   print "\nDeleting temp. directory...\n";
   system("rm -drf $pkg_temp_dir");
   print "Done.\n";
}


print "\nBuilding BSP package...\n";
##############################################################################
# Package and compress the BSP
##############################################################################
if($win == 1) {
   system("rm -rf ~/packaged_SDP2430");
   system("mkdir -p ~/packaged_SDP2430/SDP2430");

   $bsp_temp_dir = '~/packaged_SDP2430/SDP2430';
   $bsp_base_dir = '/vobs/WLAN_WinCe14/PLATFORM/SDP2430';
#   system("cp -R -L /vobs/WLAN_WinCe14/PLATFORM/SDP2430/* ~/packaged_SDP2430/SDP2430");

   for($i=0; $i<@bsp_dirs; $i+=1)
   {
      print " making $bsp_temp_dir/$bsp_dirs[$i]\n";
      system("mkdir $bsp_temp_dir/$bsp_dirs[$i]");
   }

   print "\nCopying files...\n";

   for($i=0; $i<@bsp_files; $i+=1)
   {
      ###########################################################################
      # remove trailing filename
      ###########################################################################
      $dir = "$bsp_temp_dir/$bsp_files[$i]";
      $dir =~ s/\/[^\/|^~]+$//;

      ###########################################################################
      # copy file(s)
      ###########################################################################
      print " copying $bsp_base_dir/$bsp_files[$i]\n";
      system("cp $bsp_base_dir/$bsp_files[$i] $dir");
   }   

   print "\nCompressing the BSP...\n";
#   system("rm -rf ~/packaged_SDP2430/SDP2430/src/drivers/TI_WIFI/WiLink");
   system("cd ~/packaged_SDP2430 ; zip -r -9 -q SDP2430_bsp.zip *");
   system("mv ~/packaged_SDP2430/SDP2430_bsp.zip .");
   system("rm -rf ~/packaged_SDP2430");
}

##############################################################################
# Print final message
##############################################################################
print "Package has been built.\n\n";
