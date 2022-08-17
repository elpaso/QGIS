/***************************************************************************
  qgspostgreslayermetadataprovider.cpp - QgsPostgresLayerMetadataProvider

 ---------------------
 begin                : 17.8.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspostgreslayermetadataprovider.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsabstractdatabaseproviderconnection.h"

QgsPostgresLayerMetadataProvider::QgsPostgresLayerMetadataProvider( QObject *parent ) : QgsAbstractLayerMetadataProvider( parent )
{

}

QString QgsPostgresLayerMetadataProvider::type() const
{
  return QStringLiteral( "postgres" );
}

QList<QgsLayerMetadataProviderResult> QgsPostgresLayerMetadataProvider::search( const QString &searchString ) const
{
  QList<QgsLayerMetadataProviderResult> results;
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( type( ) ) };
  if ( md )
  {
    const auto cConnections { md->connections( ) };
    for ( const auto &conn : std::as_const( cConnections ) )
    {
      if ( conn->configuration().value( QStringLiteral( "metadataInDatabase" ), false ).toBool() )
      {

        const QList<QgsLayerMetadataProviderResult> res { md->searchLayerMetadata( conn->uri(), searchString ) };
        for ( const auto &result : std::as_const( res ) )
        {
          results.push_back( result );
        }
      }
    }
  }
  return results;
}

bool QgsPostgresLayerMetadataProvider::hasMetadataTable( const QString &connectionName ) const
{
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( type( ) ) };
  if ( md )
  {
    std::unique_ptr< QgsAbstractDatabaseProviderConnection > connection;
    connection.reset( static_cast< QgsAbstractDatabaseProviderConnection * >( md->createConnection( connectionName ) ) );
    try
    {
      if ( connection->tableExists( QStringLiteral( "public" ), QStringLiteral( "qgis_layer_metadata" ) ) )
      {
        return true;
      }
    }
    catch ( const QgsProviderConnectionException & )
    {
      return false;
    }

  }
  return false;
}

bool QgsPostgresLayerMetadataProvider::createMetadataTable( const QString &connectionName ) const
{

  if ( hasMetadataTable( connectionName ) )
  {
    return false;
  }

  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( type( ) ) };
  if ( md )
  {
    std::unique_ptr< QgsAbstractDatabaseProviderConnection > connection;
    connection.reset( static_cast< QgsAbstractDatabaseProviderConnection * >( md->createConnection( connectionName ) ) );
    try
    {
      connection->executeSql( QStringLiteral( R"SQL(
        CREATE TABLE qgis_layer_metadata (
          id SERIAL PRIMARY KEY
          ,f_table_catalog VARCHAR
          ,f_table_schema VARCHAR
          ,f_table_name VARCHAR
          ,f_geometry_column VARCHAR
          ,description TEXT
          ,identifier TEXT
          ,extent GEOMETRY(POLYGON, 4326)
          ,QMD XML
          ,owner VARCHAR(63) DEFAULT CURRENT_USER
          ,update_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
      )SQL" ) );
    }
    catch ( const QgsProviderConnectionException & )
    {
      return false;
    }
  }
  return false;
}
