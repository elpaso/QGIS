/***************************************************************************
  qgslayermetadataproviderregistry.h - QgsLayerMetadataProviderRegistry

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
#ifndef QGSLAYERMETADATAPROVIDERREGISTRY_H
#define QGSLAYERMETADATAPROVIDERREGISTRY_H

#include <QObject>

#include "qgis_core.h"
#include "qgis.h"

#include "qgslayermetadata.h"

class QgsAbstractLayerMetadataProvider;


/**
 * \ingroup core
 * \brief Result of layer metadata provider search.
 *
 * \since QGIS 3.28
 */
struct CORE_EXPORT QgsLayerMetadataProviderResult
{
  QgsLayerMetadata metadata;
  QString uri;
  QString dataProviderName;
};


/**
 * \ingroup core
 * \brief Registry of layer metadata provider backends.
 *
 * This is a singleton that should be accessed through QgsApplication::layerMetadataProviderRegistry().
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsLayerMetadataProviderRegistry : public QObject
{
    Q_OBJECT
  public:
    explicit QgsLayerMetadataProviderRegistry( QObject *parent = nullptr );

    //! Registers a layer metadata provider and takes ownership of it
    void registerLayerMetadataProvider( QgsAbstractLayerMetadataProvider *metadataProvider SIP_TRANSFER );

    //! Unregisters a layer metadata provider and destroys its instance
    void unregisterLayerMetadataProvider( QgsAbstractLayerMetadataProvider *metadataProvider );

    QList<QgsAbstractLayerMetadataProvider *> layerMetadataProviders() const;

    //! Returns metadata provider implementation if the type matches one. Returns NULLPTR otherwise.
    QgsAbstractLayerMetadataProvider *layerMetadataProviderFromType( const QString &type );

    //! Searchs for layers in the registered layer metadata providers
    QList<QgsLayerMetadataProviderResult> search( const QString &searchString );

    // TODO: search filters

  private:

    QHash<QString,  QgsAbstractLayerMetadataProvider *> mMetadataProviders;

};

#endif // QGSLAYERMETADATAPROVIDERREGISTRY_H
