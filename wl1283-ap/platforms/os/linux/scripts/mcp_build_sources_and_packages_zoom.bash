#!/bin/bash

function print_highlight {
echo -e "\033[7m$1\033[0m"
} 


if [ "$KERNEL_DIR" = "" ]; then
	print_highlight "Error, KERNEL_DIR variable has not defined yet. Please define it and run the script again! "
        print_highlight "i.e. export KERNEL_DIR=/data/wlan_wcs_android/a0387670/25E.19/mydroid/kernel/android-2.6.29"
    exit
fi


if [ "$1" = "zoom3" ]; then
    print_highlight "Building sources and packages for zoom3..."
    ZOOM_PACKAGE="zoom3Binaries.tar"
else
    print_highlight "Building sources and packages for zoom2..."
    ZOOM_PACKAGE="zoom2Binaries.tar"
fi


START_TIME=$(date +%s)

rm -rf version_tree 
mkdir version_tree

print_highlight "I will now prepare the wlan sources and build packages for you :-)"

print_highlight "Phase 1 - Building Sources.."
./mcp_build_sources_zoom.bash $ZOOM_PACKAGE
print_highlight "Phase 1 - Finished."

print_highlight "Phase 2 - Building Packages..."
sleep 1
./mcp_build_packages_zoom.bash $ZOOM_PACKAGE
print_highlight "Phase 2 - Finished."

END_TIME=$(date +%s)
DIFF=$(( $END_TIME - $START_TIME ))
print_highlight "FYI - The build process took $DIFF seconds."
