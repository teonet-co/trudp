#!/bin/sh

apt-get update
apt-get upgrade -y
apt-get install -y autoconf intltool libtool libglib2.0-dev doxygen make gcc g++ checkinstall libcunit1-dev libcpputest-dev

