/***************************************************************************
                         qgsdbmanagerdialog.cpp
                         -------------------------
    begin                : July 2021
    copyright            : (C) 2021 by Alessandro Pasotti
    email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdbmanagerdialog.h"
#include "qgsbrowserproxymodel.h"
#include "qgsbrowserguimodel.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsgui.h"

QgsDbManagerDialog::QgsDbManagerDialog( QgsBrowserGuiModel *browserModel, QWidget *parent, Qt::WindowFlags f )
  :  QDialog( parent, f )
{
  if ( ! browserModel )
  {
    mBrowserModel = new QgsBrowserGuiModel( this );
    mBrowserModel->initialize();
  }
  else
  {
    mBrowserModel = browserModel;
    mBrowserModel->initialize();
  }

  setupUi( this );
  setObjectName( QStringLiteral( "QgsDbManagerDialog" ) );
  QgsGui::instance()->enableAutoGeometryRestore( this );

  mBrowserProxyModel.setBrowserModel( mBrowserModel );
  mBrowserProxyModel.setFilterDatabaseConnections( true );
  mBrowserTreeView->setHeaderHidden( true );
  mBrowserTreeView->setModel( &mBrowserProxyModel );

}
