#!/bin/bash
# A short little script to help with the annoyance of upgrading bitshares in a Linux Distro

if [ -z "$1" ]; then    #if no command line arguments inputted, ask user for version they'd like
  echo -n "What version of BitShares would you like?: "
  read version
fi

# env -i git pull 
# ^ this was in the original documentation...but I don't know why


# Checkout a certain version ... bts/VERSIONNUMBER !! modify accordingly
env -i git checkout bts/$version
env -i git submodule update

# Compile
make
make forcebuildweb || exit
make BitShares     || exit

## Install Binaries system-wide
cp programs/qt_wallet/bin/BitShares /usr/local/bin/
mkdir -p /usr/local/share/icons/
cp programs/qt_wallet/images/qtapp80.png /usr/local/share/icons/BitShares.png
cp programs/qt_wallet/BitShares.desktop /usr/local/share/applications/

exit

