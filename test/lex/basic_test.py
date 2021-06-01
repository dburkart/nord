import unittest

import util

class TestBasicTokens(util.DiffTestCase):

    def test_variable_assignment(self):
        self.assertNoDiff(self.runtest('basic/variable_assignment.n'), 'basic/variable_assignment.txt')
