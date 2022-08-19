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
        QString errorMessage;
        const QList<QgsLayerMetadataProviderResult> res { md->searchLayerMetadata( conn->uri(), searchString, errorMessage ) };
        // TODO: log errors
        for ( const auto &result : std::as_const( res ) )
        {
          results.push_back( result );
        }
      }
    }
  }
  return results;
}

