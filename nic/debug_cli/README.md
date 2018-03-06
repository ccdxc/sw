Debug   CLI:
==================
Install click, swig
0. Install click_repl if not installed from nic dir:
sudo pip install -e debug_cli/click_repl


1. nic/debug_cli/cli.sh  (Ignore the warnings thats show up)

2. commands available:
    debug hardware table read  ? or tab will show options
        --- reads fields from a table 
    debug hardware table write  ? or tab will show the options
        --- writes specific fields to a table

    show system p4table lists all the tables with their stages and limits

ex: debug hardware table read copp copp_index 10

3. Points to note:
Keyword completions are autocomplete. Arguments are not. Below is the sample error message where 
the arguments are not given:


debug hardware table read copp copp_index <enter>
Usage: debugshell.py  debug hardware table read copp copp_index
           [OPTIONS] copp_id COMMAND [ARGS]...
Error: Missing argument "ids".

Implies that we didn't give value to the argument "copp_id"


Contact:
lseshan@pensando.io
avinash@pensando.io
saratk@pensando.io

