# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsClassificationMethod implementations

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Denis Rouzaud'
__date__ = '3/09/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import os
import numpy as np

import qgis  # NOQA
from osgeo import gdal, osr

from qgis.PyQt.QtCore import QLocale, QTemporaryDir
from qgis.testing import unittest, start_app
from qgis.core import (
    QgsClassificationMethod,
    QgsClassificationLogarithmic,
    QgsClassificationQuantile,
    QgsFeature,
    QgsVectorLayer,
    QgsRasterLayer,
    QgsPointXY,
    QgsGeometry,
)

start_app()


# ===========================================================
# Utility functions


def createMemoryLayer(values):
    ml = QgsVectorLayer("Point?crs=epsg:4236&field=id:integer&field=value:double",
                        "test_data", "memory")
    # Data as list of x, y, id, value
    assert ml.isValid()
    pr = ml.dataProvider()
    fields = pr.fields()
    id = 0
    for value in values:
        id += 1
        feat = QgsFeature(fields)
        feat['id'] = id
        feat['value'] = value
        g = QgsGeometry.fromPointXY(QgsPointXY(id / 100, id / 100))
        feat.setGeometry(g)
        pr.addFeatures([feat])
    ml.updateExtents()
    return ml


class TestQgsClassificationMethods(unittest.TestCase):

    def testQgsClassificationLogarithmic(self):
        values = [2746.71,
                  66667.49,
                  77282.52,
                  986567.01,
                  1729508.41,
                  9957836.86,
                  35419826.29,
                  52584164.80,
                  296572842.00]

        vl = createMemoryLayer(values)

        m = QgsClassificationLogarithmic()
        r = m.classes(vl, 'value', 8)

        self.assertEqual(len(r), 6)
        self.assertEqual(r[0].label(), '{} - 10^4'.format(QLocale().toString(2746.71)))
        self.assertEqual(QgsClassificationMethod.rangesToBreaks(r),
                         [10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0, 1000000000.0])

        self.assertEqual(len(m.classes(vl, 'value', 4)), 4)

    def testQgsClassificationLogarithmic_FilterZeroNeg(self):
        values = [-2, 0, 1, 7, 66, 555, 4444]
        vl = createMemoryLayer(values)
        m = QgsClassificationLogarithmic()

        m.setParameterValues({'ZERO_NEG_VALUES_HANDLE': QgsClassificationLogarithmic.Discard})
        r = m.classes(vl, 'value', 4)
        self.assertEqual(len(r), 4)
        self.assertEqual(r[0].label(), '1 - 10^1')
        self.assertEqual(QgsClassificationMethod.rangesToBreaks(r), [10.0, 100.0, 1000.0, 10000.0])

        m.setParameterValues({'ZERO_NEG_VALUES_HANDLE': QgsClassificationLogarithmic.PrependBreak})
        r = m.classes(vl, 'value', 4)
        self.assertEqual(r[0].label(), '-2 - 10^0')
        self.assertEqual(QgsClassificationMethod.rangesToBreaks(r), [1.0, 10.0, 100.0, 1000.0, 10000.0])

    def testClassificationMethodOnRasterWrapper(self):
        """Test QgsClassificationMethod on raster wrapper"""

        tempdir = QTemporaryDir()
        temppath = os.path.join(tempdir.path(), 'as_vector_multiband.tif')

        driver = gdal.GetDriverByName('GTiff')
        # 32 columns and 16 rows 3 bands int32
        outRaster = driver.Create(temppath, 32, 16, 3, gdal.GDT_Int32)
        for band in range(1, 4):
            outband = outRaster.GetRasterBand(band)
            data = []
            for r in range(16):
                data.append([band + i for i in range(32)])
            npdata = np.array(data, np.int32)
            outband.WriteArray(npdata)
            outband.FlushCache()

        outRaster.FlushCache()
        del outRaster

        rl = QgsRasterLayer(temppath, 'as_vector_multiband')
        self.assertTrue(rl.isValid())
        vl = rl.asVector()
        self.assertEqual(vl.fields().count(), rl.bandCount())
        self.assertTrue(vl.isValid())

        m = QgsClassificationQuantile()
        r = m.classes(vl, 'Band 2', 4)
        self.assertEqual([_r.label() for _r in r], ['2 - 9.75', '9.75 - 17.5', '17.5 - 25.25', '25.25 - 33'])


if __name__ == "__main__":
    unittest.main()
