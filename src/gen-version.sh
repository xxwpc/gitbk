#!/bin/sh -e

date=`git log "--pretty=format:%ai" -1 | awk '{printf $1}'`

#echo "#define VERSION_DATE \"$var\""

hash="$(git log --pretty=format:%h -1 --abbrev=7)"

#echo "#define VERSION_HASH \"$var\""

ver_str="// This file is auto generated.\n// Don't edit.\n\n\n#define VERSION_DATE \"$date\"\n#define VERSION_HASH \"$hash\""


if [ "$(echo $ver_str)" != "$(cat version.h 2> /dev/null)" ]; then
   echo $ver_str > version.h
   echo "New version.h is generated => '$date $hash...'"
fi

