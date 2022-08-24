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
#include "qgsrectangle.h"
#include "qgspolygon.h"

/**
 * \ingroup core
 * \brief Result record of layer metadata provider search.
 * The results contains metadata information and all information
 * that is required by QGIS to load the layer.
 *
 * \since QGIS 3.28
 */
struct CORE_EXPORT QgsLayerMetadataProviderResult
{
  //! Metadata identifier
  QString identifier;
  //! Metadata title
  QString title;
  //! Metadata abstract
  QString abstract;
  //! Metadata extent in EPSG:4326
  QgsPolygon geographicExtent;
  //! Layer geometry type (Point, Polygon, Linestring)
  QgsWkbTypes::GeometryType geometryType;
  //! Layer CRS authid
  QString crs;
  //! Layer QgsDataSourceUri string
  QString uri;
  //! Layer data provider name
  QString dataProviderName;
  //! Layer type (vector, raster etc.)
  QgsMapLayerType layerType;
  //! Metadata QMD (XML)
  QgsLayerMetadata metadata;
};

/**
 * \ingroup core
 * \brief Result of a layer metadata search, it contains
 * the records of the layer metadata provider that matched the search
 * criteria and the list of the errors that occourred while searching
 * for metadata.
 *
 * \since QGIS 3.28
 */
struct CORE_EXPORT QgsLayerMetadataSearchResult
{
  QList<QgsLayerMetadataProviderResult> metadata;
  QStringList errors;
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

    virtual QString type() const = 0;

    virtual QgsLayerMetadataSearchResult search( const QString &searchString = QString(), const QgsRectangle &geographicExtent = QgsRectangle() ) const;


};

#endif // QGSABSTRACTLAYERMETADATAPROVIDER_H
