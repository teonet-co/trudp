# TR-UDP library: Teonet Real time communications over UDP protocol

[![CircleCI](https://circleci.com/gh/teonet-co/trudp.svg?style=svg)](https://circleci.com/gh/teonet-co/trudp)

## 1. Description

The TR-UDP protocol is the UDP based protocol for real time communications that 
allows sending short messages with low latency and provides protocol reliability 
features.

[Read More](https://gitlab.ksproject.org/teonet/teonet/wikis/tr-udp)

## 2. Installation from sources

### Install project with submodules

    git clone git@gitlab.ksproject.org:teonet/trudp.git
    cd trudp
    git submodule update --init


### Install Dependences

There is not special dependences, just packets to build C code and execute tests 

### First time, after got sources from subversion repository

    ./autogen.sh


## 3. Make your application 

    make


## 4. Make and run tests

    make test


## 5. Run example
    
See example [README.md](examples/README.md) in examples folder


## 6 Using autoscan to create or update configure.ac

After make some global changes in sources code use ```autoscan``` to update projects 
configure.ac


## 7. Documentation

See libtrudp documentation at: http://repo.ksproject.org/docs/libtrudp/


## 8. Build package and and CI

To create package use command:

    ci-build/make_package deb

To check and build package we use [CirleCI](https://circleci.com):  [.circleci/config.yml](.circleci/config.yml)

    
## 9. Install from repository

### UBUNTU

    http://repo.ksproject.org/ubuntu/
    
### Add repository

    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 8CC88F3BE7D6113C
    sudo apt-get install -y software-properties-common
    sudo add-apt-repository "deb http://repo.ksproject.org/ubuntu/ teonet main"
    sudo apt-get update

### Install

    sudo apt-get install -y libtrudp
