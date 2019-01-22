#!/bin/bash

set -e

echo "First build 'image2tiles' in the dir above"
echo

cp ../image2tiles ./

# TODO Add parameters for sample
./image2tiles

# Start HTTP server
python -m http.server
