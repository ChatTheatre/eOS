# eOS

**After the deepest darkness before the dawn (SkotOS) comes the first glimmerings of the rosy-fingered dawn (eOS)**

## Installation

```
# Install eOS repository with recursive submodules

git clone git@github.com:ChatTheatre/eOS.git --recursive

# Move to eOS root folder

cd eOS

## Make sure all submodules are checked out to master branch

git submodule foreach --recursive git checkout master

## Move to the DGD source director

cd dgd/src

# make and install the current dgd which will be at …/eos/dgd/bin/dgd

make install

# return to repository root

cd ../..

# test dgd compiled, should get usage help message

./dgd/bin/dgd

# Add eos path to eos/eos.DGD

pwd
nano ./eos.dgd

# start dgd

./dgd/bin/dgd ./eos.dgd

# login telnet port

telnet local.host 50100

```

## eOS Directory Structure

* dgd - the LPC language interpreter for eOS's low-level code
* eOS-Doc - documentation
* include - LPC headers for compilation
* kernel - code for the LPC kernel library objects, including libraries and LWOs
* kernellib - the full kernellib code repository

## TODO/TBD

* Do we
   * copy files to repository eos root like phantasmal does?
   * use the submodules folders
