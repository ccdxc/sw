#!/bin/bash

echo -e "\033[36m Building Cscope ...\033[0m"
find -type f -and -regex '.*/.*\.\(c\|cpp\|cc\|h\|hpp\|p4\|s\|asm\|py\|proto\|spec\)$' > cscope.files
# find . -name '*.c' 2> /dev/null > cscope.files
# find . -name '*.cpp' 2> /dev/null >> cscope.files
# find . -name '*.cc' 2> /dev/null >> cscope.files
# find . -name '*.hpp' 2> /dev/null >> cscope.files
# find . -name '*.p4' 2> /dev/null >> cscope.files
# find . -name '*.s' 2> /dev/null >> cscope.files
# find . -name '*.asm' 2> /dev/null >> cscope.files
# find . -name '*.py' 2> /dev/null >> cscope.files
# find . -name '*.proto' 2> /dev/null >> cscope.files
# find . -name '*.spec' 2> /dev/null >> cscope.files

cscope -b -q
echo -e "\033[36m Done !! \033[0m"
