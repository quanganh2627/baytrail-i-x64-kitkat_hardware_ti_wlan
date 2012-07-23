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
$inFilePath = "./build";
$inFile = "$inFilePath/os_sources.inc";
$outDir = "tiwlan_gpl";
$switches = "-f";
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
        $switches = "-vf";
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

$exclude_flag = 1;
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
$mkdirCmd = "mkdir -p ${targetDir}";
system($mkdirCmd);

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
    if($_ =~ /OS_DK_INCS_FILES#/) 
    {
        $exclude_flag = 1 - $exclude_flag;      
    }
        
    if($_ =~ /OS_DK_INCS_FILES_CCX#/) 
    {
       if($ccx == 0)
       {
          $exclude_ccx_flag = 1 - $exclude_ccx_flag;        
       }
    }
    
    ##############################################################################
    # Case $exclude_flag is OFF - copy the file
    ##############################################################################
    if (($exclude_flag == 0) && ($exclude_ccx_flag == 0))
    {
        if(($_ =~ /COMSRC/) || ($_ =~ /COMMON/) || ($_ =~ /HALSRC/) || ($_ =~ /TESTSRC/) || ($_ =~ /TWD/) || ($_ =~ /TXN/) ) 
        {
            s/[\\ \t\n\r\s()]//g;
            s/\$\COMSRC/$COMSRC/;
            s/\$\COMMON/$COMMON/;
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
        }
    }
    
}
close INF;


##############################################################################
# Copy all other files and directories needed that not mentioned on the infile.
##############################################################################
system("mkdir -p $targetDir/build"); 
system("cp $switches build/* $targetDir/build"); 

system("mkdir -p $targetDir/inc"); 
system("cp $switches inc/*.h $targetDir/inc"); 

system("mkdir -p $targetDir/src"); 
system("cp $switches src/*.h src/*.c $targetDir/src"); 

system("mkdir -p $targetDir/../common/inc");
system("cp $switches -R ../common/inc/* $targetDir/../common/inc"); 

system("mkdir -p $targetDir/../../../utils");
system("cp $switches -R ../../../utils/* $targetDir/../../../utils"); 

system("mkdir -p $targetDir/../../../Txn");
system("cp $switches -R ../../../Txn/* $targetDir/../../../Txn"); 

system("mkdir -p $targetDir/../../../stad/build/linux");
system("cp $switches ../../../stad/build/linux/common.inc $targetDir/../../../stad/build/linux");

system("mkdir -p $targetDir/../../hw/host_platform_$ENV{'HOST_PLATFORM'}");
system("cp $switches -R ../../hw/host_platform_$ENV{'HOST_PLATFORM'}/* $targetDir/../../hw/host_platform_$ENV{'HOST_PLATFORM'}/");

system("mkdir -p $targetDir/../../../external_drivers/$ENV{'HOST_PLATFORM'}");
system("cp $switches -R ../../../external_drivers/$ENV{'HOST_PLATFORM'}/* $targetDir/../../../external_drivers/$ENV{'HOST_PLATFORM'}/");

system("cp $switches  $ENV{'HOST_PLATFORM'}_env.tcsh $targetDir/");


if ($ccx == 0)
{
	# Make the No CCX package default to compilation with CCX=n.
	system("cat $targetDir/../../../../build/common.inc | sed -e \"s/CCX\\ ?=\\ y/CCX\\ ?=\\ n/\" &> $targetDir/../../../../build/common.inc.new");
	system("mv -f $targetDir/../../../../build/common.inc.new $targetDir/../../../../build/common.inc");
	system("chmod 775 $targetDir/../../../../build/common.inc");
}

##############################################################################
# Compress the directory
##############################################################################
system("tar -cz $switches $outDir.tar.gz $outDir/*"); 

##############################################################################
# Print final message
##############################################################################
print "Package has been built.\n";
