#!/bin/bash

set -e

echo "First build 'image2tiles' in the dir above"
echo

../image2tiles --p1=722,-19.0649,290,64.0088 --p2=1360,-19.0182,421,64.0046 --file=sample_sketch.jpg --max-zoom-level=13 --debug

# Start HTTP server
python -m http.server
