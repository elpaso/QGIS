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

QList<QgsAbstractLayerMetadataProvider *> QgsLayerMetadataProviderRegistry::metadataProviders() const
{
  return mMetadataProviders.values();
}
