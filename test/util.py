import inspect
import os
import subprocess
import unittest

from tempfile import NamedTemporaryFile

class DiffTestCase(unittest.TestCase):
    def assertNoDiff(self, output, expectation_file):
        with open(os.path.join(self.expectations_dir(), expectation_file), 'r') as f:
            expected = f.read()
        self.assertMultiLineEqual(output, expected)

    def base_dir(self):
        # We get the path to our child class to calculate base directory
        child_path = inspect.getfile(self.__class__)
        return os.path.dirname(child_path)

    def runner_path(self):
        return os.path.join(self.base_dir(), "run")

    def expectations_dir(self):
        return os.path.join(self.base_dir(), "expectations")

    def input_dir(self):
        return os.path.join(self.base_dir(), "input")

    def runtest(self, filename):
        file_path = os.path.join(self.input_dir(), filename)
        return subprocess.check_output([self.runner_path(), file_path]).decode('utf-8')