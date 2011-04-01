#!/usr/bin/perl

##############################################################################
# Set the default parameters
##############################################################################
$pkg_temp_dir = 'WiLink';
$base_dir = './../../../../WiLink';
$out_file = 'wilink62.zip';

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
              'stad/src/RolesAndLinks',
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
              'stad/src/RolesAndLinks/*.c',
              'stad/src/RolesAndLinks/*.h',
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

@images_dirs = (
             'platforms/os/linux/images/zoom2',
             );

@lin_dirs_open_source = (
             'CUDK/wpa_suppl',
             'CUDK/wpa_suppl/openssl-0.9.8e',
             'CUDK/hostapd/',
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
             'external_drivers/zoom2/Linux',
             'external_drivers/zoom2/Linux/sdio',
             'external_apps/Linux',
             'external_apps/Linux/bmtrace',
             'external_apps/Linux/bmtrace/Drv',
             'platforms/hw/host_platform_zoom2/linux',
             'platforms/os/linux',
             'platforms/os/linux/build',
             'platforms/os/linux/inc',
             'platforms/os/linux/src',
             'platforms/os/linux/scripts',
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
              'platforms/os/linux/Android.mk',
              'platforms/os/linux/build/*.inc',
              'platforms/os/linux/build/Makefile',
              'platforms/os/linux/zoom2_env.bash',
              'platforms/os/linux/tiwlan_1273.ini',
              'platforms/os/linux/tiwlan_1283.ini',
              'platforms/os/linux/tiwlan_dual_1273.ini',
			  'platforms/os/linux/tiwlan_dual_1283.ini',
              'platforms/os/linux/inc/*.h',
              'platforms/os/linux/src/*.h',
              'platforms/os/linux/src/*.c',
              'platforms/os/linux/scripts/ap_run',
              'platforms/os/linux/scripts/ap_rm',
              'platforms/os/linux/scripts/nat_start',
              'platforms/os/common/build/linux/*.inc',
              'platforms/os/common/build/linux/Makefile',
              'stad/build/linux/*.inc',
              'stad/build/linux/Makefile'
             );


@images_files = (
              'platforms/os/linux/images/zoom2/iperf',
              'platforms/os/linux/images/zoom2/wfa_dut',
              'platforms/os/linux/images/zoom2/dotnet_personal_test.cer',
              'platforms/os/linux/images/zoom2/dotnet_personal_test.pem',
              'platforms/os/linux/images/zoom2/dotnet_root.cer',
              'platforms/os/linux/images/zoom2/wpa_supplicant_wpa_tls.txt',
              'platforms/os/linux/images/zoom2/udhcpd',
              'platforms/os/linux/images/zoom2/udhcpdWlan.conf',
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
              'CUDK/hostapd/hostapd-0.6.9.diff',
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
           $out_file = 'WiLink62_Linux_noCCX_core.tar.bz2';

           # OSA part
           @pkg_dirs_osa  = (@lin_dirs_osa);
           @pkg_files_osa = (@lin_files_osa);
           $out_file_osa  = 'WiLink62_Linux_noCCX_osa.tar.bz2';
        }
        else {
           @pkg_dirs = (@full_pkg_dirs_noccx_linux, @lin_dirs, @images_dirs, @lin_dirs_open_source);
           @pkg_files = (@full_pkg_files_noccx_linux, @lin_files, @images_files, @lin_files_open_source);
           $out_file = 'WiLink62_Linux_OpenSource.tar.bz2';
        }
     }
        else
        {
           # Core part 
           @pkg_dirs = (@full_pkg_dirs_ccx_linux, @lin_dirs_core, @images_dirs, @lin_dirs_open_source);
           @pkg_files = (@full_pkg_files_ccx_linux, @lin_files_core, @images_files, @lin_files_open_source);
           $out_file = 'WiLink62_Linux_CCX_core.tar.bz2';

           # OSA part
           @pkg_dirs_osa  = (@lin_dirs_osa);
           @pkg_files_osa = (@lin_files_osa);
           $out_file_osa  = 'WiLink62_Linux_CCX_osa.tar.bz2';

        }
 }
if($gem == 1){

           # Core part 
           @pkg_dirs = (@full_pkg_dirs_noccx_linux, @lin_dirs_core, @images_dirs, @lin_dirs_open_source2);
           @pkg_files = (@full_pkg_files_noccx_linux, @lin_files_core, @images_files, @lin_files_open_source2);
           $out_file = 'WiLink62_Linux_Gem_core.tar.bz2';

           # OSA part
           @pkg_dirs_osa  = (@lin_dirs_osa);
           @pkg_files_osa = (@lin_files_osa);
           $out_file_osa  = 'WiLink62_Linux_Gem_osa.tar.bz2';

}

# system("rm -drf $pkg_temp_dir");
# system("rm -f $out_file");
# system("mkdir $pkg_temp_dir");




##############################################################################
# Create directory structure
##############################################################################
print "Creating directry structure for CORE package...\n";

print "pkg_temp_dir $pkg_temp_dir\n";
print "base_dir $base_dir\n";
system("pwd");

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
   print " ==== Taking care of GPL headers ====\n";
   system("pwd");
   system("ls -l ./scripts/package-gpl-noccx");
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
# system("rm -drf $pkg_temp_dir");
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
#   system("rm -drf $pkg_temp_dir");
   print "Done.\n";
}


##############################################################################
# Print final message
##############################################################################
print "Package has been built.\n\n";
