/***************************************************************************
  qgspostgreslayermetadataprovider.h - QgsPostgresLayerMetadataProvider

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
#ifndef QGSPOSTGRESLAYERMETADATAPROVIDER_H
#define QGSPOSTGRESLAYERMETADATAPROVIDER_H

#include <qgsabstractlayermetadataprovider.h>

class QgsPostgresLayerMetadataProvider : public QgsAbstractLayerMetadataProvider
{
  public:
    explicit QgsPostgresLayerMetadataProvider( QObject *parent = nullptr );

    // QgsAbstractLayerMetadataProvider interface
  public:
    QString type() const override;
    QList<QgsLayerMetadataProviderResult> search( const QString &searchString ) const override;
};

#endif // QGSPOSTGRESLAYERMETADATAPROVIDER_H
