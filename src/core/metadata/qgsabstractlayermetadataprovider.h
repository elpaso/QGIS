/***************************************************************************
  qgslayermetadataprovider.h - QgsLayerMetadataProvider

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
#ifndef QGSABSTRACTLAYERMETADATAPROVIDER_H
#define QGSABSTRACTLAYERMETADATAPROVIDER_H

#include <QObject>

#include "qgis_core.h"
#include "qgis.h"

#include "qgslayermetadata.h"

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
 * \brief Layer metadata provider backends interface.
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsAbstractLayerMetadataProvider : public QObject
{
    Q_OBJECT
  public:
    explicit QgsAbstractLayerMetadataProvider( QObject *parent = nullptr );

    virtual QString &type() const = 0;

    virtual QList<QgsLayerMetadataProviderResult> search( const QString &searchString ) const = 0;


};

#endif // QGSABSTRACTLAYERMETADATAPROVIDER_H
