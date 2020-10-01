/***************************************************************************
  qgsrasterwrapperprovider.h - QgsRasterWrapperProvider

 ---------------------
 begin                : 30.9.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERWRAPPERPROVIDER_H
#define QGSRASTERWRAPPERPROVIDER_H

#include "qgsvectordataprovider.h"
#include "qgsrasterdataprovider.h"
#include <QPointer>


class QgsRasterWrapperProvider : public QgsVectorDataProvider
{
  public:
    QgsRasterWrapperProvider( QgsRasterDataProvider *rasterDataProvider );

    // QgsDataProvider interface
    QgsCoordinateReferenceSystem crs() const override;
    QgsRectangle extent() const override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;

    // QgsVectorDataProvider interface
    QgsAbstractFeatureSource *featureSource() const override;

    // QgsFeatureSource interface
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    QgsFields fields() const override;
    QgsWkbTypes::Type wkbType() const override;
    long featureCount() const override;

    static QString providerKey();

    QString providerDescription();

  private:

    QPointer<QgsRasterDataProvider> mRasterDataProvider = nullptr;
    QgsFields mFields;

    friend class QgsRasterWrapperFeatureSource;

};


class QgsRasterWrapperFeatureSource final: public QgsAbstractFeatureSource
{
  public:
    explicit QgsRasterWrapperFeatureSource( const QgsRasterWrapperProvider *p );

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:
    QgsFields mFields;
    QgsExpressionContext mExpressionContext;
    QgsCoordinateReferenceSystem mCrs;
    QString mSubsetString;
    QPointer<QgsRasterDataProvider> mRasterDataProvider = nullptr;

    friend class QgsRasterWrapperFeatureIterator;
};

class QgsRasterWrapperFeatureIterator final: public QgsAbstractFeatureIteratorFromSource<QgsRasterWrapperFeatureSource>
{
  public:
    QgsRasterWrapperFeatureIterator( QgsRasterWrapperFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsRasterWrapperFeatureIterator() override;

    bool rewind() override;
    bool close() override;

    /**
     * Returns the feature id from \a position in layer's CRS.
     * Returns -1 on error.
     */
    QgsFeatureId featureIdFromPoint( const QgsPointXY &position );

    /**
     * Returns 0-based south-east col and row coordinates from \d featureId
     */
    QPair<int, int> featureIdToMatrixCoordinates( const QgsFeatureId featureId );


  protected:

    bool fetchFeature( QgsFeature &feature ) override;

  private:

    QPointer<QgsRasterDataProvider> mRasterDataProvider = nullptr;
    QgsFeatureId mNextFeatureId = 1;
    qlonglong mRasterColumns = 0;
    qlonglong mRasterRows = 0;
    qlonglong mRasterCellsCount = 0;
    QgsGeometry mSelectRectGeom;
    QString mSubsetString;
    std::unique_ptr< QgsGeometryEngine > mSelectRectEngine;
    QgsRectangle mFilterRect;
    std::unique_ptr< QgsExpression > mSubsetExpression;
    QgsCoordinateTransform mTransform;
    QgsFields mFields;
    double mXStep = 0;
    double mYStep = 0;
    std::unique_ptr< QgsRasterIterator > mRasterIterator;
};


#endif // QGSRASTERWRAPPERPROVIDER_H
