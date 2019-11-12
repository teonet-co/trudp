#!/bin/sh
# Create DEBIAN package and Upload it to repository
#
# To upload package to Bintray repository set this enviropment variables:
#
# CI_BUILD_REF_BT=<1234567>
# CI_BINTRAY_USER=<Bintray-User-Login-Name>
# CI_BINTRAY_API_KEY=<Binntray-Api-Key>
#
# To upload package to KSProject repository set this enviropment variable:
#
# CI_BUILD_REF=<1234567>
#
# Parameters:
#
# @param $1 Version
# @param $2 Library Major version
# @param $3 Library version
# @param $4 Release
# @param $5 Architecture
# @param $6 RPM subtype = "deb" (not used, reserved)
# @param $7 PACKET_NAME
# @param $8 PACKET_DESCRIPTION
# @param $9 MAINTAINER
# @param $10 DEPENDS
# @param $11 LICENSES
# @param $12 VCS_URL
#

# Include make deb functions
PWD=`pwd`
. "$PWD/tools/make_deb_inc.sh"

# Set exit at error
set -e

# Check parameters and set defaults
check_param $1 $2 $3 $4 $5 $6 $7 "$8" "$9" "${10}" "${11}" "${12}"
# Set global variables:
# VER_ONLY=$1
# LIBRARY_HI_VERSION=$2
# LIBRARY_VERSION=$3
# RELEASE=$4
# ARCH=$5
# VER=$1-$RELEASE
# PACKET_NAME=$7
# PACKET_DESCRIPTION=$8
# MAINTAINER=$9
# DEPENDS=$10

# Set Variables
# DEPENDS="libteonet-dev"
# Note: Add this to Depends if test will be added to distributive:
# libcunit1-dev (>= 2.1-2.dfsg-1)
# MAINTAINER="kirill@scherba.ru"
VER_ARCH=$VER"_"$ARCH
PACKAGE_NAME=$PACKET_NAME"_"$VER_ARCH
REPO_JUST_CREATED=0
REPO=../repo

# Main message
echo $ANSI_BROWN"Create debian packet $PACKET_NAME""_$VER_ARCH.deb"$ANSI_NONE
echo ""

# Update and upgrade build host
#update_host

# Create deb repository -------------------------------------------------------
if [ ! -z "$CI_BUILD_REF" ]; then

    echo $ANSI_BROWN"Create DEB repository:"$ANSI_NONE
    echo ""

    # Install reprepro
    # sudo apt-get install -y reprepro
    # echo ""

    # Download repository from remote host by ftp:
    tools/make_remote_download.sh
    
    # Create DEB repository
    create_deb_repo $REPO ubuntu Teonet teonet tools/gpg_key
fi

# Create deb package ----------------------------------------------------------
echo $ANSI_BROWN"Create DEB package:"$ANSI_NONE
echo ""

# Configure and make auto configure project (in current folder)
make_counfigure

# Make install
make_install $PWD/$PACKAGE_NAME

# Create DEBIAN control file
create_deb_control $PACKAGE_NAME $PACKET_NAME $VER $ARCH "${DEPENDS}" "${MAINTAINER}" "${PACKET_DESCRIPTION}"

# Build package
build_deb_package $PACKAGE_NAME

## Install and run application to check created package
#install_run_deb $PACKAGE_NAME "teovpn -?"
#
## Show version of installed depends
#show_teonet_depends
#
## Remove package
#apt_remove $PACKET_NAME

# Add packet to repository ----------------------------------------------------

# Upload repository to remote host, test install and run application ----------
if [ ! -z "$CI_BUILD_REF" ]; then

    echo $ANSI_BROWN"Upload DEB package:"$ANSI_NONE
    echo ""

    # Add DEB packages to local repository
    add_deb_package $REPO/ubuntu teonet $PACKAGE_NAME

    # Upload repository to remote host by ftp:
    tools/make_remote_upload.sh
    
    # Install packet from remote repository
    tools/make_remote_install.sh
fi

# Upload DEB packages to Bintray  ---------------------------------------------
if [ ! -z "$CI_BUILD_REF_BT" ]; then

    echo $ANSI_BROWN"Upload DEB package to Bintray repository:"$ANSI_NONE
    echo ""

    # Create packet if not exists
    create_package_bintray
    sleep 30
    # Upload file distribution wheezy, bionic
    upload_deb_bintray wheezy
    upload_deb_bintray bionic
fi

# Upload DEB packages to Launchpad PPA repository  ----------------------------
if [ -z "$CI_SKIP_PPA" ]; then
    echo $ANSI_BROWN"Build PPA DEB packages:"$ANSI_NONE
    echo ""

    build_ppa
fi

# Make and upload documentation  ----------------------------------------------
if [ -z "$CI_SKIP_DOCS" ]; then
    tools/make_remote_doc_upload.sh $PACKET_NAME 
fi

# Add DEB packages to Bintray download list -----------------------------------
if [ ! -z "$CI_BUILD_REF_BT" ] && [ -z "$CI_SKIP_DOWNLOADS"  ]; then

    echo $ANSI_BROWN"Add DEB packages to Bintray download list:"$ANSI_NONE
    echo ""
    
    # Add "wheezy" to direct download list
    allow_deb_binary_download wheezy
    
    # Add "bionic" to direct download list
    allow_deb_binary_download bionic
fi

# circleci local execute --job un-tagged-build-ubuntu -e CI_BUILD_REF_BT=1234567 -e CI_BINTRAY_USER=kirill-scherba -e CI_BINTRAY_API_KEY=fc6f1cae3022da43a10350552028763343bc7474 -e CI_SKIP_DOCS=true -e CI_SKIP_DOWNLOADS=true --skip-checkout
