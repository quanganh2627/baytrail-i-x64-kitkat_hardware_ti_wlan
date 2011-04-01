#!/usr/bin/perl

##############################################################################
# Set the default parameters
##############################################################################
$pkg_temp_dir = 'WiLink';
$base_dir = $ARGV[1];
$out_file = "mcp_wlan_source_pkgs.zip";
$package_prefix=$ARGV[2];
$base_dir2 = $package_prefix . "___________";
$ccx = 0;
$gem = 0;
$twd = 0;
$win = 0;
$gpl = 0;

##############################################################################
# Files and directories
##############################################################################
@root_dirs = ('platforms', 'stad', 'TWD', 'utils', 'Test', 'fw', 'CUDK', 'config');
@root_files = ('dirs', 'sources.cmn', 'sources_cudk.cmn');

@external_drivers_dirs = (
                          'external_drivers',
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
              'stad/src/Sta_Management',
              'stad/src/RolesAndLinks'
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
              'stad/src/Sta_Management/*.h',
              'stad/src/RolesAndLinks/*.c',
              'stad/src/RolesAndLinks/*.h'
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


@images_dirs = (
             'platforms/os/linux/images',
             'platforms/os/linux/images/zoom2'
             );


@lin_dirs_open_source = (
             'CUDK/hostapd'
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
              'config/hostapd.conf',
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
             'external_drivers/zoom2/Linux',
             'external_drivers/zoom2/Linux/sdio',
             'external_apps/Linux',
             'external_apps/Linux/bmtrace',
             'external_apps/Linux/bmtrace/Drv',
             'platforms/hw/host_platform_zoom2/linux',
             'platforms/os/linux',
             'platforms/os/linux/scripts',
             'platforms/os/linux/build',
             'platforms/os/linux/inc',
             'platforms/os/linux/src',
             'platforms/os/common/build',
             'platforms/os/common/build/linux',
             'stad/build',
             'stad/build/linux'
             );

@lin_files_osa = (
              'external_drivers/zoom2/Linux/sdio/SdioDrv.[ch]',
              'external_drivers/zoom2/Linux/sdio/SdioDrvDbg.h',
              'external_drivers/zoom2/Linux/sdio/host_platform.h',
              'external_drivers/zoom2/Linux/sdio/testdrv.[ch]',
              'external_drivers/zoom2/Linux/sdio/Makefile',
              'external_apps/Linux/bmtrace/Drv/bmtrace.c',
              'external_apps/Linux/bmtrace/Drv/Makefile',
              'platforms/hw/host_platform_zoom2/linux/*.c',
              'platforms/hw/host_platform_zoom2/linux/*.h',
              'platforms/os/linux/Makefile',
              'platforms/os/linux/build/*.inc',
              'platforms/os/linux/build/Makefile',
              'platforms/os/linux/zoom3_env_.bash',
              'platforms/os/linux/tiwlan_1273.ini',
              'platforms/os/linux/tiwlan_1283.ini',
              'platforms/os/linux/tiwlan_dual_1273.ini',
              'platforms/os/linux/tiwlan_dual_1283.ini',
              'platforms/os/linux/scripts/ap_run',
              'platforms/os/linux/scripts/ap_run_use_android_apps',
              'platforms/os/linux/scripts/ap_rm',
              'platforms/os/linux/scripts/nat_start',
              'platforms/os/linux/inc/*.h',
              'platforms/os/linux/src/*.h',
              'platforms/os/linux/src/*.c',
              'platforms/os/common/build/linux/*.inc',
              'platforms/os/common/build/linux/Makefile',
              'stad/build/linux/*.inc',
              'stad/build/linux/Makefile'
             );


@images_files = (
              'platforms/os/linux/images/zoom2/udhcpd',
              'platforms/os/linux/images/zoom2/udhcpdWlan.conf',
              'platforms/os/linux/images/zoom2/dotnet_personal_test.cer',
              'platforms/os/linux/images/zoom2/dotnet_personal_test.pem',
              'platforms/os/linux/images/zoom2/dotnet_root.cer',
              'platforms/os/linux/images/zoom2/wpa_supplicant_wpa_tls.txt'
             );

@lin_files_open_source = (
              'CUDK/hostapd/patch.hostapd.android'
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
             'src/csp/zoom2',
             'src/csp/zoom2/bthci',
             'src/csp/zoom2/bthci/brf6300',
             'src/csp/zoom2/bus',
             'src/csp/zoom2/bus/zoom2bus',
             'src/csp/zoom2/inc',
             'src/csp/zoom2/oal',
             'src/csp/zoom2/oal/intr',
             'src/csp/zoom2/oal/power',
             'src/csp/zoom2/sdhc',
             'src/csp/zoom2/sdhc/base',
             'src/csp/zoom2/sdhc/impl',
             'src/csp/zoom2/sdio',
             'src/csp/zoom2/sdio/Drv',
             'src/csp/zoom2/sdio/DrvTest',
             'src/csp/zoom2/sdio/DrvTestApp',
             'src/csp/zoom2/uart',
             'src/csp/zoom2/usbotg',
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
              'src/csp/zoom2/dirs',
              'src/csp/zoom2/bthci/dirs',
              'src/csp/zoom2/bthci/brf6300/*',
              'src/csp/zoom2/bus/zoom2bus/zoom2bus.cpp',
              'src/csp/zoom2/inc/*',
              'src/csp/zoom2/oal/intr/intr.c',
              'src/csp/zoom2/oal/power/power.c',
              'src/csp/zoom2/sdhc/dirs',
              'src/csp/zoom2/sdhc/base/*',
              'src/csp/zoom2/sdhc/impl/*',
              'src/csp/zoom2/sdio/*',
              'src/csp/zoom2/sdio/Drv/*',
              'src/csp/zoom2/sdio/DrvTest/*',
              'src/csp/zoom2/sdio/DrvTestApp/*',
              'src/csp/zoom2/uart/*',
              'src/csp/zoom2/usbotg/*',
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
# Full package (no XCC)
##############################################################################
@full_pkg_dirs_noccx = (@root_dirs, @external_drivers_dirs, @external_apps_dirs, @fw_dirs, @platforms_dirs, @stad_dirs, @test_dirs, @twd_dirs, @utils_dirs, @cudk_dirs);
@full_pkg_files_noccx = (@root_files, @external_drivers_files, @external_apps_files, @fw_files, @platforms_files, @stad_files, @test_files, @twd_files, @utils_files, @cudk_files);
@full_pkg_dirs_noccx_linux = (@root_dirs, @external_drivers_dirs, @external_apps_dirs, @fw_dirs, @platforms_dirs, @stad_dirs, @test_dirs, @twd_dirs, @utils_dirs, @cudk_dirs_linux);
@full_pkg_files_noccx_linux = (@external_drivers_files, @external_apps_files, @fw_files, @platforms_files, @stad_files, @test_files_linux, @twd_files, @utils_files, @cudk_files_linux);

@lin_dirs = (@lin_dirs_core, @lin_dirs_osa);
@lin_files = (@lin_files_core, @lin_files_osa);


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
                 'platforms/hw/host_platform_zoom2/winmobile/',
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
                  'platforms/hw/host_platform_zoom2/winmobile/*.h',
                  'stad/Export_Inc/TI_IPC_Api.h'
                  );

##############################################################################
# Parse command line
##############################################################################
for($i=0; $i<1; $i+=1)
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
    elsif($ARGV[$i] eq "-noCCX")
    {
        $ccx = 0;
        $lin = 1;
        $gpl = 0;
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
        print "Arg 1: -t <dir>        temp. directory        [default: WiLink]\n";
        print "Arg 1:-o [file]       package file. [default: wilink71.zip]\n";
        print "Arg 1:-twd            build TWD only package\n";
        print "Arg 1:-lin            build linux package (default - WinMobile)\n";
        print "Arg 1:-gpl            build GPL package (default - no GPL)\n";
        print "Arg 2:                base directory                      \n";
        print "Arg 3:                package prefix \n";
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
           $out_file = $package_prefix . "_noCCX_core.tar.bz2";

           # OSA part
           @pkg_dirs_osa  = (@lin_dirs_osa);
           @pkg_files_osa = (@lin_files_osa);
           $out_file_osa  = $package_prefix . "_noCCX_osa.tar.bz2";
        }
        else {
           @pkg_dirs = (@full_pkg_dirs_noccx_linux, @lin_dirs, @lin_dirs_open_source);
           @pkg_files = (@full_pkg_files_noccx_linux, @lin_files, @lin_files_open_source);
           $out_file = $package_prefix . "_OpenSource.tar.bz2";
        }
     }
 }


system("rm -drf $pkg_temp_dir");
system("rm -f $out_file");
system("mkdir $pkg_temp_dir");



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
# If GPL release - change all headers and rename XCC
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
# Print final message
##############################################################################
print "Package has been built.\n\n";
