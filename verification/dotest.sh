#!/bin/bash
echo "Test $1"
set -e
ruby maketest.rb $1
zasm -u test.asm 
../emulator/aqnext test.rom go
