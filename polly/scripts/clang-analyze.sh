#!/bin/bash

# Copyright (c) 2014, Ruslan Baratov
# All rights reserved.

set -x

file=""
for (( i=1; i <= "$#"; i++ ));
do
    arg="${!i}"
    if [ "${arg}" == "-o" ]
    then
        let y=i+1
        param=${!y}
        output=${param}.log
        bin=${param}.plist
    fi
done

for i in "$@"
do
  if [ "${i}" == "-c" ];
  then
    # analyze
    # -w : ignore regular compiler warnings so 'temp_out' only contains
    # messages from analyzer. '-w' should not be a part of toolchain flags
    # since Hunter toolchain-id calculated using '#pragma message' output which
    # is implemented as a warning message (hence will be suppressed by '-w')
    clang -Weverything -Reverything --analyze -analyzer-output=plist-multi-file "$@" -o "${bin}" 2> "${output}"
  fi
done

# compile real code
clang "$@"
