#!/bin/sh

# Set project dependence to make Teonet project from sources at empty host

# Update Centos
yum -y update

# Autoconf dependence
yum install -y autoconf intltool libtool glib2-devel doxygen make gcc gcc-c++

# Project dependence
vi /etc/yum.repos.d/teonet.repo
#
[teonet]
name=Teonet library for RHEL / CentOS / Fedora
baseurl=http://repo.ksproject.org/rhel/x86_64/
enabled=1
gpgcheck=0
#
yum clean all
#
yum install libteonet
ldconfig 

# Update system dynamic libraries configuration
ldconfig
