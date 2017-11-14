#!/bin/bash

echo -e "\033[36m Building Cscope ...\033[0m"
find -type f -and -regex '.*/.*\.\(c\|cpp\|cc\|h\|hpp\|p4\|s\|asm\|py\|proto\|spec\)$' > cscope.files
cscope -b -q
echo -e "\033[36m Done !! \033[0m"
