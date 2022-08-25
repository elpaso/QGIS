/***************************************************************************
  qgslayermetadataproviderregistry.cpp - QgsLayerMetadataProviderRegistry

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
#include "qgslayermetadataproviderregistry.h"
#include "qgsabstractlayermetadataprovider.h"
#include "qgsfeedback.h"

QgsLayerMetadataProviderRegistry::QgsLayerMetadataProviderRegistry( QObject *parent ) : QObject( parent )
{

}

void QgsLayerMetadataProviderRegistry::registerLayerMetadataProvider( QgsAbstractLayerMetadataProvider *metadataProvider )
{
  mMetadataProviders.insert( metadataProvider->type(), metadataProvider );
}

void QgsLayerMetadataProviderRegistry::unregisterLayerMetadataProvider( QgsAbstractLayerMetadataProvider *metadataProvider )
{
  delete mMetadataProviders.take( metadataProvider->type() );
}

QList<QgsAbstractLayerMetadataProvider *> QgsLayerMetadataProviderRegistry::layerMetadataProviders() const
{
  return mMetadataProviders.values();
}

QgsAbstractLayerMetadataProvider *QgsLayerMetadataProviderRegistry::layerMetadataProviderFromType( const QString &type )
{
  return mMetadataProviders.value( type, nullptr );
}

QgsLayerMetadataSearchResult QgsLayerMetadataProviderRegistry::search( const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback )
{
  QgsLayerMetadataSearchResult results;
  for ( auto it = mMetadataProviders.cbegin(); it != mMetadataProviders.cend(); ++it )
  {

    if ( feedback && feedback->isCanceled() )
    {
      break;
    }

    const QgsLayerMetadataSearchResult providerResults { it.value()->search( searchString, geographicExtent ) };
    for ( const QgsLayerMetadataProviderResult &metadata : std::as_const( providerResults.metadata ) )
    {
      results.metadata.push_back( metadata );
    }
    for ( const QString &error : std::as_const( providerResults.errors ) )
    {
      results.errors.push_back( error );
    }
  }
  return results;
}
