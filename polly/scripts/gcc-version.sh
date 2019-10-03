#!/bin/bash

$1 --version | grep -Pwo "[0-9]\.[0-9]+\.[0-9]+" | head -1
