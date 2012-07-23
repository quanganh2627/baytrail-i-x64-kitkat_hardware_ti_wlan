#!/usr/bin/perl
##############################################################################
# This Perl Script open the DK30SOURCES.MAK ($infile) and get the related 
# sources and headers files name.
# Creates new directory ($targetDir) and copy all the related files into the new directory.
# Copy other files and directories that not mentioned on the infile.
# Delete all the unnecessary files. 
##############################################################################

##############################################################################
# Set the default parameters
##############################################################################
chdir("/vobs/WiLink/platforms/os/linux");
$inFilePath = "../../../stad/build/linux";
$inFile = "$inFilePath/drv_sources.inc";
$outDir = "tiwlan_nongpl";
$switches = "-fL";
$ccx = 1;

for($i=0; $i< @ARGV; $i+=1)
{
    if($ARGV[$i] eq "-i")
    {
        $inFile = $ARGV[$i+1];
        $i+=1;
    }
    elsif($ARGV[$i] eq "-o")
    {
        $outDir = $ARGV[$i+1];
        $i+=1;
    }
    elsif($ARGV[$i] eq "-v")
    {
        $switches = "-vfL";
    }   
    elsif($ARGV[$i] eq "-no_ccx")
    {
        $ccx = 0;
    }   
    elsif($ARGV[$i] eq "-h")
    {
        print "\nUsage: perl $0 [OPTIONS]\n\n";
        print "-i [FILE]       input file        [default: $inFile]\n";
        print "-o [DIRECTORY]  output directory in absolute path. [default: $outDir]\n";
        print "-v              verbose\n";
        print "-no_ccx         build NO CCX package\n";
        die   "-h              display this help\n\n";
    }        
    else
    {
        print "perl $0: invalid parameter: $ARGV[$i]\n";
        die "Try `perl $0 -h' for more information.\n\n";
    }
}

##############################################################################
# Open the Input file - Contains all the related sources and headers files name
##############################################################################
open INF, "<${inFile}" or die "Cannot open input file ${inFile}\n";

$src_flag = 0;
$inc_flag = 0;
$exclude_flag = 0;
$exclude_ccx_flag = 0;
$HAL_ROOT = "../../..";
$DK_ROOT = "../../..";
$COMMON  = "$DK_ROOT/stad";
$COMSRC  = "$COMMON/src";
$TWD     = "$DK_ROOT/TWD";
$TXN     = "$DK_ROOT/Txn";
$HALSRCR = "$HAL_ROOT/common";
$HALSRC  = "$HALSRCR/src";
$TESTSRC = "$DK_ROOT/Test";

##############################################################################
# Create the target directory and the Output file on it as the Input file name
##############################################################################
$targetDir = "$outDir/WiLink/platforms/os/linux";
$outFile  = "$targetDir/$inFile";
$mkdirCmd = "mkdir -p ${targetDir}";
system($mkdirCmd);
$mkdirCmd = "mkdir -p ${targetDir}/$inFilePath";
system($mkdirCmd);
open OUTF, ">${outFile}" or die "Cannot open output file ${outFile}\n";

##############################################################################
# Run over the $infile, 
# In case $inc_flag is ON - parse the directory name and create the directory in the $targetDir.
# In case $src_flag is ON - parse the file name and copy the file into the $targetDir.
# In case $exclude_flag is OFF - copy the complete line into the $outFile.
##############################################################################
while(<INF>) 
{
    ##############################################################################
    # set the related flags according to the keywords in the $inFile
    ##############################################################################
    if(($_ =~ /TNETW_INCS#/) || ($_ =~ /CORE_INCS#/)) 
    {
        $inc_flag = 1 - $inc_flag;      
    }
    if(($_ =~ /TNETW_SRCS#/) || ($_ =~ /CORE_SRCS#/) || ($_ =~ /UTIL_SRCS#/)) 
    {
        $src_flag = 1 - $src_flag;      
    }
    if(($_ =~ /GWSI_INCS#/) || ($_ =~ /GWSI_SRCS#/)) 
    {
        $exclude_flag = 1 - $exclude_flag;      
    }
        
    if(($_ =~ /TNETW_INCS_CCX#/) || ($_ =~ /CORE_INCS_CCX#/) || ($_ =~ /TNETW_SRCS_CCX#/) || ($_ =~ /CORE_SRCS_CCX#/)) 
    {
       if($ccx == 0)
       {
          $exclude_ccx_flag = 1 - $exclude_ccx_flag;        
       }
    }
    ##############################################################################
    # Case $exclude_flag is OFF - copy the complete line into the $outFile.
    ##############################################################################
    if (($exclude_flag == 0) && ($exclude_ccx_flag == 0))
    {
        print OUTF $_;
    
	    ##############################################################################
	    # Case $inc_flag is ON - create the new directory and copy all the H files into it.
	    ##############################################################################
	    if ($inc_flag == 1)
	    {
	        if(($_ =~ /COMMON/) || ($_ =~ /COMSRC/) || ($_ =~ /HALSRC/) || ($_ =~ /TESTSRC/) || ($_ =~ /TWD/) || ($_ =~ /TXN/)) 
	        {
	            s/[\\ \t\n\r\s()]//g;
	            s/-I//i;
	            s/\$\COMMON/$COMMON/;
	            s/\$\COMSRC/$COMSRC/;
	            s/\$\HALSRC/$HALSRC/;           
	            s/\$\TESTSRC/$TESTSRC/;         
                    s/\$\TWD/$TWD/;
                    s/\$\TXN/$TXN/;
	            $path = $_; 
	            $mkdirCmd = "mkdir -p ${targetDir}/$path";
	            $cpCmd = "cp $switches $path/*.h ${targetDir}/$path/";
	            system("$mkdirCmd");
	            system("$cpCmd");
	        }
	    }
	    
	    ##############################################################################
	    # case $src_flag is ON - copy the C file and all the related H files 
	    ##############################################################################
	    if ($src_flag == 1)
	    {
	        if(($_ =~ /COMSRC/) || ($_ =~ /HALSRC/) || ($_ =~ /TESTSRC/) || ($_ =~ /TWD/) || ($_ =~ /TXN/)) 
	        {
	            s/[\\ \t\n\r\s()]//g;
	            s/\$\COMSRC/$COMSRC/;
	            s/\$\HALSRC/$HALSRC/;
	            s/\$\TESTSRC/$TESTSRC/;
                    s/\$\TWD/$TWD/;
                    s/\$\TXN/$TXN/;
	            @splited_path = split(/\//,$_);
	            $filename = $splited_path[$#splited_path];
	            s/$filename$//i;
	            $path = $_; 
	            $mkdirCmd = "mkdir -p ${targetDir}/$path";
	            system("$mkdirCmd");
	            $cpCmd = "cp $switches ${path}${filename} $targetDir/$path";
	            system("$cpCmd");
	            $cpCmd = "cp $switches ${path}*.h $targetDir/$path";
	            system("$cpCmd");
	        }
	    }
	}
}
close INF;
close OUTF;

##############################################################################
# Copy all other files and directories needed that not mentioned on the infile.
##############################################################################
system("cp $switches Makefile $targetDir");

system("cp $switches -R inc/ $targetDir/inc");

system("cp $switches -R src/*.h $targetDir/inc");

system("cp $switches -R images/ $targetDir/images");

system("cp $switches -R ../../../CUDK/ $targetDir/../../../CUDK");

system("mkdir -p $targetDir/../../../fw/Latest");
system("cp $switches ../../../fw/Latest/Fw1273_CHIP.bin $targetDir/../../../fw/Latest");
system("cp $switches ../../../fw/Latest/Fw1273_FPGA.bin $targetDir/../../../fw/Latest");


system("mkdir -p $targetDir/../common/inc");
system("cp $switches ../common/inc/*.h $targetDir/../common/inc");

system("mkdir -p $targetDir/../common/src");
system("cp $switches ../common/src/*.c $targetDir/../common/src");

system("mkdir -p $targetDir/../../../utils");
system("cp $switches -R ../../../utils/* $targetDir/../../../utils"); 

system("mkdir -p $targetDir/../../../Txn");
system("cp $switches -R ../../../Txn/* $targetDir/../../../Txn"); 

system("mkdir -p $targetDir/../common/build/linux");
system("cp $switches ../common/build/linux/* $targetDir/../common/build/linux");

system("mkdir -p $targetDir/../../../stad/build/linux");
system("cp $switches ../../../stad/build/linux/common.inc $targetDir/../../../stad/build/linux/");
system("cp $switches ../../../stad/build/linux/Makefile $targetDir/../../../stad/build/linux/");

system("mkdir -p $targetDir/../../hw/host_platform_$ENV{'HOST_PLATFORM'}");
system("cp $switches -R ../../hw/host_platform_$ENV{'HOST_PLATFORM'}/* $targetDir/../../hw/host_platform_$ENV{'HOST_PLATFORM'}/");

system("mkdir -p $targetDir/../../../external_drivers/$ENV{'HOST_PLATFORM'}");
system("cp $switches -R ../../../external_drivers/$ENV{'HOST_PLATFORM'}/* $targetDir/../../../external_drivers/$ENV{'HOST_PLATFORM'}/");

system("cp $switches  $ENV{'HOST_PLATFORM'}_env.tcsh $targetDir/");


##############################################################################
# Delete all the unnecessary files in the CUDK directory
##############################################################################
system("find $targetDir/../../../CUDK/ -name \"*.o\"\ -exec rm $switches '{}' ';'");
system("find $targetDir/../../../CUDK/ -name \"*.d\"\ -exec rm $switches '{}' ';'");
system("find $targetDir/../../../CUDK/ -name \"*.keep*\"\ -exec rm $switches '{}' ';'");
system("find $targetDir/../../../CUDK/ -name \"*.contrib*\"\ -exec rm $switches '{}' ';'");
system("find $targetDir/../../../CUDK/ -name \"*.tar\"\ -exec rm $switches '{}' ';'");
system("find $targetDir/../../../CUDK/ -name \"*.bmp\"\ -exec rm $switches '{}' ';'");
system("find $targetDir/../../../CUDK/ -name \"*.doc\"\ -exec rm $switches '{}' ';'");
system("find $targetDir/../../../CUDK/ -name \"*.ww\"\ -exec rm $switches '{}' ';'");

if($ccx == 0)
{
	# Make the No CCX package default to compilation with CCX=n.
	system("cat $targetDir/../../../stad/build/linux/common.inc | sed -e \"s/CCX\\ ?=\\ y/CCX\\ ?=\\ n/\" > $targetDir/../../../stad/build/linux/common.inc.new");
	system("mv -f $targetDir/../../../stad/build/linux/common.inc.new $targetDir/../../../stad/build/linux/common.inc");
	system("chmod 775 $targetDir/../../../stad/build/linux/common.inc");
	
	# In case of NO_CCX package, no supplicant is available
	system("rm $switches -R $targetDir/../../../CUDK/Supplicant");
	system("find $targetDir/../../../CUDK/ -name \"*ccx*\"\ -exec rm $switches '{}' ';'");
	system("find $targetDir/../../../CUDK/ -name \"*CCX*\"\ -exec rm $switches '{}' ';'");
	system("find $targetDir/../../../CUDK/ -name \"*ccx*\"\ -exec rmdir '{}' ';'");
	system("find $targetDir/../../../CUDK/ -name \"*CCX*\"\ -exec rmdir '{}' ';'");
}
 
##############################################################################
# Compress the directory
##############################################################################
system("tar -cz $switches $outDir.tar.gz $outDir/*"); 

##############################################################################
# Print final message
##############################################################################
print "Package has been built.\n";