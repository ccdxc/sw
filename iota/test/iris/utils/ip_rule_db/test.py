#! /usr/bin/python3
import unittest
import os

def execute():
    start_dir = os.path.dirname(__file__)
    loader = unittest.TestLoader()
    testSuite = loader.discover(start_dir)

    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(testSuite)

    return result.wasSuccessful()
