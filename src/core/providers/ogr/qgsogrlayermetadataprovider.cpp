/***************************************************************************
  qgsogrlayermetadataprovider.cpp - QgsOgrLayerMetadataProvider

 ---------------------
 begin                : 24.8.2022
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
#include "qgsogrlayermetadataprovider.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsfeedback.h"

QgsOgrLayerMetadataProvider::QgsOgrLayerMetadataProvider( QObject *parent ) : QgsAbstractLayerMetadataProvider( parent )
{

}

QString QgsOgrLayerMetadataProvider::type() const
{
  return QStringLiteral( "ogr" );
}

QgsLayerMetadataSearchResult QgsOgrLayerMetadataProvider::search( const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback ) const
{
  QList<QgsLayerMetadataProviderResult> results;
  QStringList errors;
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( type( ) ) };

  if ( md && ( ! feedback || ! feedback->isCanceled( ) ) )
  {
    const QMap<QString, QgsAbstractProviderConnection *> cConnections { md->connections( ) };
    for ( const auto &conn : std::as_const( cConnections ) )
    {

      if ( feedback && feedback->isCanceled() )
      {
        break;
      }

      try
      {
        const QList<QgsLayerMetadataProviderResult> res { md->searchLayerMetadata( conn->uri(), searchString, geographicExtent, feedback ) };
        for ( const auto &result : std::as_const( res ) )
        {
          results.push_back( result );
        }
      }
      catch ( const QgsProviderConnectionException &ex )
      {
        errors.push_back( tr( "An error occourred while searching for metadata in connection %1: %2" ).arg( conn->uri(), ex.what() ) );
      }
    }
  }

  return QgsLayerMetadataSearchResult{ results, errors };

}
