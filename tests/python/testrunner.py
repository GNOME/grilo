#!/usr/bin/env python

import unittest
import glob
import pygtk
pygtk.require('2.0')

test_loader = unittest.defaultTestLoader

names = []
for filename in glob.iglob("test_*.py"):
    names.append(filename[:-3])

test_suites = []
for name in names:
    test_suites.append(test_loader.loadTestsFromName(name))

runner = unittest.TextTestRunner(verbosity=2)

for suite in test_suites:
    runner.run(suite)
