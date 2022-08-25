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


class QgsFeedback;

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
  //! Metadata standard uri, QGIS QMD metadata format uses "http://mrcc.com/qgis.dtd"
  QString standardUri;
  //! Metadata XML
  QgsLayerMetadata metadata;
};

/**
 * \ingroup core
 * \brief List of results from a layer metadata search, it contains
 * the records of the layer metadata provider that matched the search
 * criteria and the list of the errors that occourred while searching
 * for metadata.
 *
 * \since QGIS 3.28
 */
struct CORE_EXPORT QgsLayerMetadataSearchResult
{
  //! List of metadata that matched the search criteria
  QList<QgsLayerMetadataProviderResult> metadata;
  //! List of errors occourred while searching
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

    /**
     * Returns the name of the layer metadata provider implementation, usually the name of the data provider
     * but if can be another unique identifier.
     */
    virtual QString type() const = 0;

    /**
     * Searches for metadata optionally filtering by \a searchString and \a geographicExtent.
     * \param searchString defines a filter to limit the results to the records where the search string appears in the "identifier", "title" or "abstract" metadata fields, a case-insensitive comparison is used for the match.
     * \param geographicExtent defines a filter where the spatial extent matches the given extent in EPSG:4326
     * \returns a QgsLayerMetadataSearchResult object with a list of metadata and errors
     */
    virtual QgsLayerMetadataSearchResult search( const QString &searchString = QString(), const QgsRectangle &geographicExtent = QgsRectangle(), QgsFeedback * = nullptr ) const;

};

#endif // QGSABSTRACTLAYERMETADATAPROVIDER_H
