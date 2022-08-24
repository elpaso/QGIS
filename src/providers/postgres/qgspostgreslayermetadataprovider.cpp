/***************************************************************************
  qgspostgreslayermetadataprovider.cpp - QgsPostgresLayerMetadataProvider

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
#include "qgspostgreslayermetadataprovider.h"

QgsPostgresLayerMetadataProvider::QgsPostgresLayerMetadataProvider( QObject *parent ) : QgsAbstractLayerMetadataProvider( parent )
{

}

QString QgsPostgresLayerMetadataProvider::type() const
{
  return QStringLiteral( "postgres" );
}


