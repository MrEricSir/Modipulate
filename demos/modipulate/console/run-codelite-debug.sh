#!/bin/bash
echo 'Running CodeLite debug build'
echo '************************************'
echo
export LD_LIBRARY_PATH="../../../src/modipulate/Debug:$LD_LIBRARY_PATH"
./console

echo "console returned: " $?
