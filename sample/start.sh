#!/bin/bash

#PROFILE=sample.jpg
PROFILE=sample_sketch.jpg

set -e

echo "First build 'image2tiles' in the dir above"
echo

../image2tiles --p1=722,-19.0649,290,64.0088 --p2=1360,-19.0182,421,64.0046 --file=$PROFILE --max-zoom-level=15 --debug

echo
echo "Just open your browser on the following address:"
echo

# Start HTTP server
python -m http.server
