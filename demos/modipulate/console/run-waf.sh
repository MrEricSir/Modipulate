#!/bin/bash
echo 'Running waf build'
echo '************************************'
echo

export LD_LIBRARY_PATH="../../../build/src/modipulate:$LD_LIBRARY_PATH"
../../../build/demos/modipulate/console/console

echo "console returned: " $?
