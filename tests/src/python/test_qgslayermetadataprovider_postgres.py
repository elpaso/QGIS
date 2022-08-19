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
)
from qgis.testing import start_app, unittest

QGIS_APP = start_app()

class TestPostgresLayerMetadataProvider(unittest.TestCase):

    def setUp(self):

        super().setUp()
        dbconn = 'service=qgis_test'
        if 'QGIS_PGTEST_DB' in os.environ:
            dbconn = os.environ['QGIS_PGTEST_DB']

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(dbconn, {})
        conn.execSql('DROP TABLE IF EXISTS qgis_layer_metadata')


    def testMetadataWrite(self):

        # init connection string
        dbconn = 'service=qgis_test'
        if 'QGIS_PGTEST_DB' in os.environ:
            dbconn = os.environ['QGIS_PGTEST_DB']

        pg_layer = QgsVectorLayer('{} table="qgis_test"."someData" sql='.format(dbconn), "someData", "postgres")
        self.assertTrue(pg_layer.isValid())

        m = pg_layer.metadata()
        m.setAbstract('QGIS Some Data')
        pg_layer.setMetadata(m)

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        self.assertIsNotNone(md)

        layer_uri = pg_layer.dataProvider().uri().uri()
        md.saveLayerMetadata(layer_uri, m)

        conn = md.createConnection(dbconn, {})

        self.assertTrue(conn.tableExists('public', 'qgis_layer_metadata'))

        from IPython import embed; embed(using=False)


if __name__ == '__main__':
    unittest.main()
