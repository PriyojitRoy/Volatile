#!/bin/bash
echo "Massimiliano Goi (c) 2017 http://chess.massimilianogoi.com"
echo ""
echo "Checking the integrity of the 3-4-5 pieces Syzygy files"
echo "(if every file is OK you're good to go)"
echo ""

# The Linux equivalent of fsum -c
md5sum -c checksum.md5

echo ""
read -p "Press [Enter] to exit..."

