# coding=utf-8
""""Test for postgres layer metadata provider

.. note:: This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2022-08-19'
__copyright__ = 'Copyright 2022, ItOpen'

import os

from qgis.core import (
    QgsVectorLayer,
    QgsProviderRegistry,
    QgsWkbTypes,
    QgsLayerMetadata,
    QgsBox3d,
)

from qgis.PyQt.QtCore import QCoreApplication
from utilities import compareWkt
from qgis.testing import start_app, unittest

QGIS_APP = start_app()


class TestPostgresLayerMetadataProvider(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)

    def setUp(self):

        super().setUp()
        dbconn = 'service=qgis_test'
        if 'QGIS_PGTEST_DB' in os.environ:
            dbconn = os.environ['QGIS_PGTEST_DB']

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(dbconn, {})
        conn.execSql('DROP TABLE IF EXISTS qgis_test.qgis_layer_metadata')

    def testMetadataWriteRead(self):

        # init connection string
        dbconn = 'service=qgis_test'
        if 'QGIS_PGTEST_DB' in os.environ:
            dbconn = os.environ['QGIS_PGTEST_DB']

        pg_layer = QgsVectorLayer('{} type=Point table="qgis_test"."someData" (geom) sql='.format(dbconn), "someData", "postgres")
        self.assertTrue(pg_layer.isValid())

        m = pg_layer.metadata()
        m.setAbstract('QGIS Some Data')
        m.setIdentifier('MD012345')
        ext = QgsLayerMetadata.Extent()
        spatial_ext = QgsLayerMetadata.SpatialExtent()
        spatial_ext.bounds = QgsBox3d(pg_layer.extent())
        spatial_ext.crs = pg_layer.crs()
        ext.setSpatialExtents([spatial_ext])
        m.setExtent(ext)
        pg_layer.setMetadata(m)

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        self.assertIsNotNone(md)

        layer_uri = pg_layer.dataProvider().uri().uri()
        self.assertTrue(md.saveLayerMetadata(layer_uri, m))

        conn = md.createConnection(dbconn, {})

        self.assertTrue(conn.tableExists('qgis_test', 'qgis_layer_metadata'))

        # Check the table
        data = conn.execSql('SELECT * FROM qgis_test.qgis_layer_metadata')

        conn.setConfiguration({'metadataInDatabase': True})
        conn.store('PG Metadata Enabled Connection')

        pg_layer = QgsVectorLayer('{} type=Point table="qgis_test"."someData" (geom) sql='.format(dbconn), "someData", "postgres")
        m = pg_layer.metadata()
        self.assertEqual(m.identifier(), 'MD012345')
        self.assertEqual(m.abstract(), 'QGIS Some Data')
        self.assertEqual(m.crs().authid(), 'EPSG:4326')

        reg = QGIS_APP.layerMetadataProviderRegistry()
        md_provider = reg.layerMetadataProviderFromType('postgres')
        results = md_provider.search('QGIS Some Data')
        self.assertEqual(len(results), 1)

        result = results[0]

        self.assertEqual(result.abstract, 'QGIS Some Data')
        self.assertEqual(result.identifier, 'MD012345')
        self.assertEqual(result.layerType, 'vector')
        self.assertEqual(result.crs, 'EPSG:4326')
        self.assertEqual(result.geometryType, QgsWkbTypes.PointGeometry)
        self.assertEqual(result.dataProviderName, 'postgres')
        self.assertTrue(compareWkt(result.extent.asWkt(), """MultiPolygon (((-71.12300000000000466 66.32999999999999829, -65.31999999999999318 66.32999999999999829, -65.31999999999999318 78.29999999999999716, -71.12300000000000466 78.29999999999999716, -71.12300000000000466 66.32999999999999829)))"""))

        # Check layer load
        md_pg_layer = QgsVectorLayer(result.uri, 'PG MD Layer', result.dataProviderName)
        self.assertTrue(md_pg_layer.isValid())


if __name__ == '__main__':
    unittest.main()
