#!/bin/bash

$1 --version | awk '/release/ { print $5 }' | sed s/,//