import unittest

import util

class TestBasicTokens(util.DiffTestCase):

    def test_variable_assignment(self):
        self.assertNoDiff(self.runtest('basic/variable_assignment.n'), 'basic/variable_assignment.txt')

    def test_all_tokens(self):
        self.assertNoDiff(self.runtest('basic/all.n'), 'basic/all.txt')
