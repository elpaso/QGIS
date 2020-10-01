/***************************************************************************
  qgsrasterwrapperprovider.cpp - QgsRasterWrapperProvider

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
#include "qgsrasterwrapperprovider.h"
#include "qgsexpressioncontextutils.h"
#include "qgsproject.h"
#include "qgsgeometryengine.h"
#include "qgsrasteriterator.h"

///@cond PRIVATE


QgsRasterWrapperProvider::QgsRasterWrapperProvider( QgsRasterDataProvider *rasterDataProvider )
  : QgsVectorDataProvider( )
  , mRasterDataProvider( rasterDataProvider )
{
  if ( mRasterDataProvider )
  {
    // Band is 1-based
    for ( int bandNo = 1; bandNo <= mRasterDataProvider->bandCount(); ++bandNo )
    {
      QVariant::Type type;
      switch ( mRasterDataProvider->dataType( bandNo ) )
      {
        case Qgis::DataType::Byte:
        {
          type = QVariant::Type::Char;
          break;
        }
        case Qgis::DataType::Float32:
        case Qgis::DataType::Float64:
        {
          type = QVariant::Type::Double;
          break;
        }
        case Qgis::DataType::Int32:
        case Qgis::DataType::Int16:
        {
          type = QVariant::Type::Int;
          break;
        }
        case Qgis::DataType::UInt16:
        {
          type = QVariant::Type::UInt;
          break;
        }
        case Qgis::DataType::UInt32:
        {
          type = QVariant::Type::ULongLong;
          break;
        }
        case Qgis::DataType::ARGB32:
        case Qgis::DataType::ARGB32_Premultiplied:
        {
          // FIXME: is this even supported by fields?
          type = QVariant::Type::Color;
          break;
        }
        default:
        {
          type = QVariant::Type::String;
        }
      }
      const QgsField f { tr( "Band %1" ).arg( bandNo ), type, QString( QVariant::typeToName( type ) ) };
      mFields.append( f );
    }
  }
}


QgsCoordinateReferenceSystem QgsRasterWrapperProvider::crs() const
{
  return  mRasterDataProvider ? mRasterDataProvider->crs() : QgsCoordinateReferenceSystem();
}

QgsRectangle QgsRasterWrapperProvider::extent() const
{
  return  mRasterDataProvider ? mRasterDataProvider->extent() : QgsRectangle();
}

bool QgsRasterWrapperProvider::isValid() const
{
  return mRasterDataProvider ? mRasterDataProvider->isValid() : false;
}

QString QgsRasterWrapperProvider::name() const
{
  return mRasterDataProvider ? mRasterDataProvider->name() + QStringLiteral( " (vector)" ) : QStringLiteral( "(invalid)" );
}

QString QgsRasterWrapperProvider::description() const
{
  return mRasterDataProvider ? mRasterDataProvider->description() : QStringLiteral( "(invalid)" );
}

QgsAbstractFeatureSource *QgsRasterWrapperProvider::featureSource() const
{
  return new QgsRasterWrapperFeatureSource( this );
}

QString QgsRasterWrapperProvider::providerKey()
{
  return QStringLiteral( "rasterwrapper" );
}

QString QgsRasterWrapperProvider::providerDescription()
{
  return QStringLiteral( "Raster wrapper data provider" );
}

QgsFeatureIterator QgsRasterWrapperProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return QgsFeatureIterator( new QgsRasterWrapperFeatureIterator( new QgsRasterWrapperFeatureSource( this ), true, request ) );
}

QgsFields QgsRasterWrapperProvider::fields() const
{
  return mFields;
}

QgsWkbTypes::Type QgsRasterWrapperProvider::wkbType() const
{
  return QgsWkbTypes::Point;
}

long QgsRasterWrapperProvider::featureCount() const
{
  // TODO: subsetstring
  return mRasterDataProvider ? mRasterDataProvider->xSize() * mRasterDataProvider->ySize() : 0;
}

// ---------------------------------------

QgsRasterWrapperFeatureSource::QgsRasterWrapperFeatureSource( const QgsRasterWrapperProvider *p )
  : mFields( p->fields() )
  , mCrs( p->crs() )
  , mSubsetString( p->subsetString() )
  , mRasterDataProvider( p->mRasterDataProvider )
{
  mExpressionContext << QgsExpressionContextUtils::globalScope()
                     << QgsExpressionContextUtils::projectScope( QgsProject::instance() );
  mExpressionContext.setFields( mFields );
}

QgsFeatureIterator QgsRasterWrapperFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsRasterWrapperFeatureIterator( this, false, request ) );
}

// ---------------------------------------


QgsRasterWrapperFeatureIterator::QgsRasterWrapperFeatureIterator( QgsRasterWrapperFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsRasterWrapperFeatureSource>( source, ownSource, request )
  , mRasterDataProvider( source->mRasterDataProvider )
  , mRasterColumns( source->mRasterDataProvider ? source->mRasterDataProvider->xSize() : 0 )
  , mRasterRows( source->mRasterDataProvider ? source->mRasterDataProvider->ySize() : 0 )
  , mFields( source->mFields )
  , mXStep( source->mRasterDataProvider ? source->mRasterDataProvider->extent().width() / source->mRasterDataProvider->xSize() : 0 )
  , mYStep( source->mRasterDataProvider ? source->mRasterDataProvider->extent().height() / source->mRasterDataProvider->ySize() : 0 )
  , mRasterIterator( qgis::make_unique<QgsRasterIterator>( source->mRasterDataProvider ) )
{
  if ( ! mRasterDataProvider )
  {
    return;
  }

  mRasterCellsCount = mRasterColumns * mRasterRows;

  mRasterIterator->setMaximumTileWidth( 1 );
  mRasterIterator->setMaximumTileHeight( 1 );

  for ( int bandNumber = 1; bandNumber <= mRasterDataProvider->bandCount(); ++bandNumber )
  {
    mRasterIterator->startRasterRead( bandNumber, mRasterColumns, mRasterRows, mRasterDataProvider->extent() );
  }

  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs )
  {
    mTransform = QgsCoordinateTransform( mSource->mCrs, mRequest.destinationCrs(), mRequest.transformContext() );
  }
  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
  }
  catch ( QgsCsException & )
  {
    // can't reproject mFilterRect
    close();
    return;
  }

  if ( !mSource->mSubsetString.isEmpty() )
  {
    mSubsetExpression = qgis::make_unique< QgsExpression >( mSource->mSubsetString );
    mSubsetExpression->prepare( &mSource->mExpressionContext );
  }

  if ( !mFilterRect.isNull() && mRequest.flags() & QgsFeatureRequest::ExactIntersect )
  {
    mSelectRectGeom = QgsGeometry::fromRect( mFilterRect );
    mSelectRectEngine.reset( QgsGeometry::createGeometryEngine( mSelectRectGeom.constGet() ) );
    mSelectRectEngine->prepareGeometry();
  }

  rewind();
}

QgsRasterWrapperFeatureIterator::~QgsRasterWrapperFeatureIterator()
{
  close();
}

bool QgsRasterWrapperFeatureIterator::fetchFeature( QgsFeature &feature )
{
  feature.setValid( false );

  if ( ! mRasterDataProvider || ! mRasterDataProvider->isValid() || mClosed || mNextFeatureId > mRasterCellsCount || mXStep == 0 || mYStep == 0 )
  {
    return false;
  }

  // 0-based, from south east
  const auto colRow { featureIdToMatrixCoordinates( mNextFeatureId ) };
  const qlonglong &col { colRow.first };
  const qlonglong &row { colRow.second };

  const QgsRectangle fullExtent { mRasterDataProvider->extent() };
  const QgsRectangle extent
  {
    fullExtent.xMinimum() + mXStep * col,
    fullExtent.yMinimum() + mYStep * row,
    fullExtent.xMinimum() + mXStep *( col + 1 ),
    fullExtent.yMinimum() + mYStep *( row + 1 )
  };

  const QgsPointXY centroid { extent.center() };
  feature.setGeometry( QgsGeometry::fromPointXY( centroid ) );
  Q_ASSERT( featureIdFromPoint( centroid ) == mNextFeatureId );
  feature.setId( mNextFeatureId );
  feature.setFields( mFields, true );

  for ( int bandNumber = 1; bandNumber <= mRasterDataProvider->bandCount(); ++bandNumber )
  {
    std::unique_ptr<QgsRasterBlock> block;
    int iterLeft = 0;
    int iterTop = 0;
    int iterCols = 0;
    int iterRows = 0;
    QgsRectangle blockExtent;
    const bool readOk { mRasterIterator->readNextRasterPart( bandNumber, iterCols, iterRows, block, iterLeft, iterTop, &blockExtent ) };
    if ( ! block->isValid() || ! readOk )
    {
      return false;
    }
    // TODO: handle colors
    feature.setAttribute( bandNumber - 1, block->value( 0, 0 ) );
  }
  mNextFeatureId++;
  return true;
}

bool QgsRasterWrapperFeatureIterator::rewind()
{
  if ( mClosed )
  {
    return false;
  }
  mNextFeatureId = 1;
  if ( mRasterDataProvider )
  {
    for ( int bandNumber = 1; bandNumber < mRasterDataProvider->bandCount(); ++bandNumber )
    {
      mRasterIterator->stopRasterRead( bandNumber );
    }
  }
  return true;
}

bool QgsRasterWrapperFeatureIterator::close()
{
  if ( mClosed )
  {
    return false;
  }

  iteratorClosed();

  mClosed = true;
  mNextFeatureId = 1;
  if ( mRasterDataProvider )
  {
    for ( int bandNumber = 1; bandNumber < mRasterDataProvider->bandCount(); ++bandNumber )
    {
      mRasterIterator->stopRasterRead( bandNumber );
    }
  }
  return true;
}

QgsFeatureId QgsRasterWrapperFeatureIterator::featureIdFromPoint( const QgsPointXY &position )
{
  if ( ! mRasterDataProvider || ! mRasterDataProvider->isValid() || ! mRasterDataProvider->extent().contains( position ) || mXStep == 0 || mYStep == 0 )
  {
    return -1;
  }

  // 0-based from south east
  const qlonglong col { static_cast<qlonglong>( ( position.x() - mRasterDataProvider->extent().xMinimum() ) / mXStep ) };
  const qlonglong row { static_cast<qlonglong>( ( position.y() - mRasterDataProvider->extent().yMinimum() ) / mYStep ) };
  const QgsFeatureId fid { 1 + row *mRasterColumns + col };
  qDebug() << "RWP:" << "returning fid" << fid << "for xy" << position.x() << position.y() << col << row;
  return fid;
}

QPair<int, int> QgsRasterWrapperFeatureIterator::featureIdToMatrixCoordinates( const QgsFeatureId featureId )
{
  if ( ! mRasterDataProvider || ! mRasterDataProvider->isValid() || featureId > mRasterCellsCount || mRasterColumns == 0 || mRasterRows == 0 )
  {
    return { 0, 0 };
  }
  const qlonglong row { static_cast<qlonglong>( ( featureId - 1 ) / mRasterRows ) };
  const qlonglong col { static_cast<qlonglong>( ( featureId - 1 ) % mRasterColumns ) };
  return { col, row };
}


///@endcond PRIVATE

