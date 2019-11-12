#!/bin/sh

# Set project dependence to make Teonet project from sources at empty host

# Update OpenSuse
zypper -y update

# Autoconf dependence
zypper install -y autoconf intltool libtool glib2-devel doxygen make gcc g++ patch

# Project dependence
zypper ar -f http://repo.ksproject.org/opensuse/x86_64/ teonet
zypper in -y libteonet
ldconfig

# Update system dynamic libraries configuration
ldconfig
