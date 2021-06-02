import unittest

import util

class TestBasicTokens(util.DiffTestCase):

    def test_variable_assignment(self):
        self.assertNoDiff(self.runtest('basic/variable_assignment.n'), 'basic/variable_assignment.txt')

    def test_all_tokens(self):
        self.assertNoDiff(self.runtest('basic/all.n'), 'basic/all.txt')

    def test_strings(self):
        self.assertNoDiff(self.runtest('basic/strings.n'), 'basic/strings.txt')

    def test_paren_no_space(self):
        self.assertNoDiff(self.runtest('basic/paren_no_space.n'), 'basic/paren_no_space.txt')

    def test_operator_no_space(self):
        self.assertNoDiff(self.runtest('basic/operator_no_space.n'), 'basic/operator_no_space.txt')
