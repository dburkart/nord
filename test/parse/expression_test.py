import unittest

import util

class TestExpressions(util.DiffTestCase):

    def test_addition(self):
        self.assertNoDiff(self.runtest('expressions/addition.n'), 'expressions/addition.txt')

    def test_grouping(self):
        self.assertNoDiff(self.runtest('expressions/grouping.n'), 'expressions/grouping.txt')
