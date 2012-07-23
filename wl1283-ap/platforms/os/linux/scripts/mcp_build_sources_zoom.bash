#!/bin/bash

#
# In case you modify the script make sure you run 
# dos2unix on it before checking it in.
#

#This script should be run from pltaform/os/linux/script directory
ROOT_DIR="$PWD/../../../../"
LINUX_DIR="$ROOT_DIR/platforms/os/linux"
VERSIONTREE_DIR="$LINUX_DIR/scripts/version_tree/sources"

WILINK_PACKAGE_PREFIX="mcp_linux_source"

cd "$LINUX_DIR"
make clean >& /dev/null
rm -r -f $VERSIONTREE_DIR

if [ "$1" = "clean" ]; then
    echo "*** Cleaning up..."
    exit
else
    echo "*** Preparing environment..."
fi


echo "*** Removing redundant private files..."
cd "$ROOT_DIR"

files=`find . -name *.contrib*`
for file in ${files}; do
    rm -f ${file}
done

files=`find . -name *.keep*`
for file in ${files}; do
    rm -f ${file}
done

files=`find . -name *.d`
for file in ${files}; do
    rm -f ${file}
done

files=`find . -name *.*.cmd`
for file in ${files}; do
    rm -f ${file}
done

files=`find . -name *.mod*`
for file in ${files}; do
    rm -f ${file}
done

files=`find . -name *.d`
for file in ${files}; do
    rm -f ${file}
done


if [ "$5" = "clean" ]; then
	echo "*** Done!"
    exit
fi


cd "$LINUX_DIR"


echo "*** Building directory tree..."
mkdir -p "$VERSIONTREE_DIR"


echo "*** Starting to create MCP WLAN packages $1"

echo "*** Creating Driver No-CCX Non-Open Source package..."
perl scripts/mcp_pkg_linux_zoom2.pl -noCCX $ROOT_DIR $WILINK_PACKAGE_PREFIX

#echo "*** Creating Driver No-CCX Open Source package..."
perl scripts/mcp_pkg_linux_zoom2.pl -gpl $ROOT_DIR $WILINK_PACKAGE_PREFIX


echo "*** Copy open source package to $VERSIONTREE_DIR"

cp "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_OpenSource.tar.bz2 "$VERSIONTREE_DIR"/"$WILINK_PACKAGE_PREFIX"_OpenSource.tar.bz2
cp "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_noCCX_core.tar.bz2 "$VERSIONTREE_DIR"/"$WILINK_PACKAGE_PREFIX"_noCCX_core.tar.bz2
cp "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_noCCX_osa.tar.bz2 "$VERSIONTREE_DIR"/"$WILINK_PACKAGE_PREFIX"_noCCX_osa.tar.bz2
#cp "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_CCX_core.tar.bz2 "$VERSIONTREE_DIR"/"$WILINK_PACKAGE_PREFIX"_CCX_core.tar.bz2
#cp "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_CCX_osa.tar.bz2 "$VERSIONTREE_DIR"/"$WILINK_PACKAGE_PREFIX"_CCX_osa.tar.bz2
#cp "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_Gem_osa.tar.bz2 "$VERSIONTREE_DIR"/"$WILINK_PACKAGE_PREFIX"_Gem_osa.tar.bz2
#cp "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_Gem_core.tar.bz2 "$VERSIONTREE_DIR"/"$WILINK_PACKAGE_PREFIX"_Gem_core.tar.bz2


#rm "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_CCX_core.tar.bz2
#rm "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_CCX_osa.tar.bz2
#rm "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_noCCX_core.tar.bz2
#rm "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_noCCX_osa.tar.bz2
#rm "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_OpenSource.tar.bz2
#rm "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_Gem_osa.tar.bz2
#rm "$LINUX_DIR"/"$WILINK_PACKAGE_PREFIX"_Gem_core.tar.bz2

echo ""
echo "****************************************************************************"
echo "* Build Zoom sources done! "
echo "* "
echo "* Shalom"
echo "****************************************************************************"
echo ""

