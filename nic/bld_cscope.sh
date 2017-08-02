#!/bin/bash

echo -e "\033[36m Building Cscope ...\033[0m"
find -regex '.*/.*\.\(c\|cpp\|cc\|h\|hpp\)$' > cscope.files
cscope -b -q
echo -e "\033[36m Done !! \033[0m"
