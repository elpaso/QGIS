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
#include "qgsdataitemguiprovider.h"
#include "qgsdataitemguiproviderregistry.h"
#include "qgsdataitem.h"
#include "qgsgui.h"

#include <QMenu>

QgsDbManagerDialog::QgsDbManagerDialog( QgsBrowserGuiModel *browserModel, QWidget *parent, Qt::WindowFlags f )
  :  QDialog( parent, f )
{
  if ( ! browserModel )
  {
    mModel = new QgsBrowserGuiModel( this );
    mModel->initialize();
  }
  else
  {
    mModel = browserModel;
    mModel->initialize();
  }

  setupUi( this );
  setObjectName( QStringLiteral( "QgsDbManagerDialog" ) );
  QgsGui::instance()->enableAutoGeometryRestore( this );

  mProxyModel.setBrowserModel( mModel );
  mProxyModel.setFilterDatabaseConnections( true );
  mBrowserTreeView->setHeaderHidden( true );
  mBrowserTreeView->setModel( &mProxyModel );
  mBrowserTreeView->setContextMenuPolicy( Qt::ContextMenuPolicy::CustomContextMenu );

  connect( mBrowserTreeView, &QgsBrowserTreeView::customContextMenuRequested, this, &QgsDbManagerDialog::showContextMenu );

}

void QgsDbManagerDialog::showContextMenu( const QPoint &pt )
{
  QModelIndex index = mProxyModel.mapToSource( mBrowserTreeView->indexAt( pt ) );
  QgsDataItem *item = mModel->dataItem( index );
  if ( !item )
    return;

  const QModelIndexList selection = mBrowserTreeView->selectionModel()->selectedIndexes();
  QList< QgsDataItem * > selectedItems;
  selectedItems.reserve( selection.size() );
  for ( const QModelIndex &selectedIndex : selection )
  {
    QgsDataItem *selectedItem = mProxyModel.dataItem( selectedIndex );
    if ( selectedItem )
      selectedItems << selectedItem;
  }

  QMenu *menu = new QMenu( this );

  const QList<QMenu *> menus = item->menus( menu );
  QList<QAction *> actions = item->actions( menu );

  if ( !menus.isEmpty() )
  {
    for ( QMenu *mn : menus )
    {
      menu->addMenu( mn );
    }
  }

  if ( !actions.isEmpty() )
  {
    if ( !menu->actions().isEmpty() )
      menu->addSeparator();
    // add action to the menu
    menu->addActions( actions );
  }

  QgsDataItemGuiContext context;
  context.setMessageBar( mMessageBar );

  const QList< QgsDataItemGuiProvider * > providers = QgsGui::instance()->dataItemGuiProviderRegistry()->providers();
  for ( QgsDataItemGuiProvider *provider : providers )
  {
    provider->populateContextMenu( item, menu, selectedItems, context );
  }

  if ( menu->actions().isEmpty() )
  {
    delete menu;
    return;
  }

  menu->popup( mBrowserTreeView->mapToGlobal( pt ) );
}
