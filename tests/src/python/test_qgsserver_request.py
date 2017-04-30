# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServerRequest.

From build dir, run: ctest -R PyQgsServerRequest -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
import unittest

__author__ = 'Alessandro Pasotti'
__date__ = '29/04/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


from qgis.PyQt.QtCore import QUrl
from qgis.server import QgsServerRequest


class QgsServerRequestTest(unittest.TestCase):

    def test_requestHeaders(self):
        """Test request headers"""
        headers = {'header-key-1': 'header-value-1', 'header-key-2': 'header-value-2'}
        request = QgsServerRequest('http://somesite.com/somepath', QgsServerRequest.GetMethod, headers)
        for k, v in request.headers().items():
            self.assertEqual(headers[k], v)
        request.removeHeader('header-key-1')
        self.assertEqual(request.headers(), {'header-key-2': 'header-value-2'})
        request.setHeader('header-key-1', 'header-value-1')
        for k, v in request.headers().items():
            self.assertEqual(headers[k], v)

    def test_requestParameters(self):
        """Test request parameters"""
        request = QgsServerRequest('http://somesite.com/somepath?parm1=val1&parm2=val2', QgsServerRequest.GetMethod)
        parameters = {'PARM1': 'val1', 'PARM2': 'val2'}
        for k, v in request.parameters().items():
            self.assertEqual(parameters[k], v)
        request.removeParameter('PARM1')
        self.assertEqual(request.parameters(), {'PARM2': 'val2'})
        request.setHeader('PARM1', 'val1')
        for k, v in request.headers().items():
            self.assertEqual(parameters[k], v)

    def test_requestUrl(self):
        """Test url"""
        request = QgsServerRequest('http://somesite.com/somepath', QgsServerRequest.GetMethod)
        self.assertEqual(request.url().toString(), 'http://somesite.com/somepath')
        request.setUrl(QUrl('http://someother.com/someotherpath'))
        self.assertEqual(request.url().toString(), 'http://someother.com/someotherpath')

    def test_requestMethod(self):
        request = QgsServerRequest('http://somesite.com/somepath', QgsServerRequest.GetMethod)
        self.assertEqual(request.method(), QgsServerRequest.GetMethod)
        request.setMethod(QgsServerRequest.PostMethod)
        self.assertEqual(request.method(), QgsServerRequest.PostMethod)


if __name__ == '__main__':
    unittest.main()
