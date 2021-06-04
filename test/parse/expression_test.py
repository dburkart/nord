import unittest

import util

class TestExpressions(util.DiffTestCase):

    def test_addition(self):
        self.assertNoDiff(self.runtest('expressions/addition.n'), 'expressions/addition.txt')

    def test_assignment(self):
        self.assertNoDiff(self.runtest('expressions/assignment.n'), 'expressions/assignment.txt')

    def test_grouping(self):
        self.assertNoDiff(self.runtest('expressions/grouping.n'), 'expressions/grouping.txt')

    def test_greater_or_equal(self):
        self.assertNoDiff(self.runtest('expressions/greater_or_equal.n'), 'expressions/greater_or_equal.txt')

    def test_declaration_01(self):
        self.assertNoDiff(self.runtest('expressions/declaration_01.n'), 'expressions/declaration_01.txt')

    def test_declaration_02(self):
        self.assertNoDiff(self.runtest('expressions/declaration_02.n'), 'expressions/declaration_02.txt')

    def test_declaration_03(self):
        self.assertNoDiff(self.runtest('expressions/declaration_03.n'), 'expressions/declaration_03.txt')
