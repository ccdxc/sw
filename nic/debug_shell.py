#!/usr/bin/python
#
# Capri Debug Shell (debug_shell.py)
# Ajeer Pudiyapura (Pensando Systems)

from cmd2 import Cmd
import os
import sys

class debugCmd(Cmd):

    prompt = "debug> "
    intro = "This is a debug CLI. Theres no protection against bad arguments!"

    def __init__(self):
        Cmd.__init__(self)
        self.exclude_from_help.append('do_py')
        self.exclude_from_help.append('do_set')
        self.exclude_from_help.append('do_run')
        self.exclude_from_help.append('do_EOF')
        self.exclude_from_help.append('do_quit')
        self.exclude_from_help.append('do_save')
        self.exclude_from_help.append('do_edit')
        self.exclude_from_help.append('do_show')
        self.exclude_from_help.append('do_shell')
        self.exclude_from_help.append('do_pyscript')
        self.exclude_from_help.append('do_shortcuts')
        self.exclude_from_help.append('do__relative_load')
        self.exclude_from_help.append('do_cmdenvironment')
        self.nic_dir = os.path.abspath(__file__)
        self.nic_dir = os.path.split(self.nic_dir)[0]
        self.iris_inited = False
        self.do_init_modules("")

    def __del__(self):
        if self.iris_inited:
            try:
                from p4pd_cli import cleanup as iris_cleanup
                iris_cleanup()
                self.iris_inited = False
                debugCmd.do_iris = 0

            except ImportError:
                print("Could not cleanup Iris.")

    def do_quit(self, args):
        """Exits to the previous level"""
        return True

    do_EOF = do_quit

    do_exit = do_quit

    def iris(self, a, b):
        """Interact with Iris Tables"""
        from p4pd_cli import rootCmd as iris_root
        cmd = iris_root()
        cmd.cmdloop()

    def do_init_modules(self, args):
        """Scan for new CLI modules"""
        if self.iris_inited == False:
            iris_path = self.nic_dir + '/gen/iris/cli'
            if os.path.isfile(iris_path + '/p4pd_cli.py'):
                print("Module Iris found!")
                sys.path.append(iris_path)
                try:
                    from p4pd_cli import init as iris_init
                    iris_init()
                    self.iris_inited = True
                    debugCmd.do_iris = classmethod(self.iris)

                except ImportError:
                    print("Could not initialize Iris. Do 'make' in sw/nic/gen/iris/cli/ directory, and run 'init_modules' command here.")
            else:
                print("Module Iris not found. Do 'make' in sw/nic directory, and run 'init_modules' command here to load missing modules.")
        else:
            print("All modules initialized.")

if __name__ == '__main__':

    cmd = debugCmd()
    cmd.cmdloop()
    del cmd

