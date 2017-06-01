#!/bin/bash
#
#   Copyright (C) 2015 Pelagicore AB
#   All rights reserved.
#

COMPONENT_FOLDER=$1
#echo $UNCRUSTIFY_CONFIG_FILE

if [ "$COMPONENT_FOLDER" = "" ];
then
  COMPONENT_FOLDER=`pwd`
fi

if [ "$UNCRUSTIFY_CONFIG_FILE" = "" ];
then
  UNCRUSTIFY_CONFIG_FILE=`dirname $0`/../uncrustify.cfg
fi

echo Formatting with Uncrustify configuration file : $UNCRUSTIFY_CONFIG_FILE

UNCRUSTIFY_COMMAND="uncrustify -c $UNCRUSTIFY_CONFIG_FILE --no-backup --replace 'file'"

#read -p "We are going to format the C/C++ (*.cpp, *.c, *.h) source files from the folder '$COMPONENT_FOLDER'.
#We recommend that you first commit your files to your local GIT before going further, since the formatter will modify your files.
#You will be able to amend the changes due to the formatting with the following command : $ git commit --amend
#Should we proceed with the formatting ? " -n 1 -r
echo    # (optional) move to a new line
if [[ 1=1 ]]
#if [[ $REPLY =~ ^[Yy]$ ]]
then
  find $COMPONENT_FOLDER -name "*.cpp" | xargs -I file echo "$UNCRUSTIFY_COMMAND" | bash -s
  find $COMPONENT_FOLDER -name "*.h" | xargs -I file echo "$UNCRUSTIFY_COMMAND" | bash -s
  find $COMPONENT_FOLDER -name "*.c" | xargs -I file echo "$UNCRUSTIFY_COMMAND" | bash -s
fi

exit
