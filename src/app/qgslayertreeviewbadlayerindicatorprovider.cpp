/***************************************************************************
  qgslayertreeviewbadlayerindicatorprovider.cpp - QgsLayerTreeViewBadLayerIndicatorProvider

 ---------------------
 begin                : 17.10.2018
 copyright            : (C) 2018 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewbadlayerindicatorprovider.h"
#include "qgslayertree.h"
#include "qgslayertreeview.h"
#include "qgslayertreeutils.h"
#include "qgslayertreemodel.h"
#include "qgsvectorlayer.h"
#include "qgisapp.h"

QgsLayerTreeViewBadLayerIndicatorProvider::QgsLayerTreeViewBadLayerIndicatorProvider( QgsLayerTreeView *view )
  : QObject( view )
  , mLayerTreeView( view )
{
  mIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mIndicatorBadLayer.svg" ) );

  QgsLayerTree *tree = mLayerTreeView->layerTreeModel()->rootGroup();
  onAddedChildren( tree, 0, tree->children().count() - 1 );

  connect( tree, &QgsLayerTree::addedChildren, this, &QgsLayerTreeViewBadLayerIndicatorProvider::onAddedChildren );
  connect( tree, &QgsLayerTree::willRemoveChildren, this, &QgsLayerTreeViewBadLayerIndicatorProvider::onWillRemoveChildren );

}

void QgsLayerTreeViewBadLayerIndicatorProvider::onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
{
  QList<QgsLayerTreeNode *> children = node->children();
  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsLayerTreeNode *childNode = children[i];

    if ( QgsLayerTree::isGroup( childNode ) )
    {
      onAddedChildren( childNode, 0, childNode->children().count() - 1 );
    }
    else if ( QgsLayerTree::isLayer( childNode ) )
    {
      if ( QgsLayerTreeLayer *layerNode = dynamic_cast< QgsLayerTreeLayer * >( childNode ) )
      {
        if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layerNode->layer() ) )
        {
          if ( QgsLayerTreeUtils::countMapLayerInTree( mLayerTreeView->layerTreeModel()->rootGroup(), vlayer ) == 1 )
            connect( vlayer, &QgsVectorLayer::dataSourceChanged, this, &QgsLayerTreeViewBadLayerIndicatorProvider::onDataSourceChanged );
          addOrRemoveIndicator( childNode, vlayer );
        }
        else if ( !layerNode->layer() )
        {
          // wait for layer to be loaded (e.g. when loading project, first the tree is loaded, afterwards the references to layers are resolved)
          connect( layerNode, &QgsLayerTreeLayer::layerLoaded, this, &QgsLayerTreeViewBadLayerIndicatorProvider::onLayerLoaded );
        }
      }
    }
  }
}

void QgsLayerTreeViewBadLayerIndicatorProvider::onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
{
  // recursively disconnect from providers' dataChanged() signal

  QList<QgsLayerTreeNode *> children = node->children();
  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsLayerTreeNode *childNode = children[i];

    if ( QgsLayerTree::isGroup( childNode ) )
    {
      onWillRemoveChildren( childNode, 0, childNode->children().count() - 1 );
    }
    else if ( QgsLayerTree::isLayer( childNode ) )
    {
      QgsLayerTreeLayer *childLayerNode = QgsLayerTree::toLayer( childNode );
      if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( childLayerNode->layer() ) )
      {
        if ( QgsLayerTreeUtils::countMapLayerInTree( mLayerTreeView->layerTreeModel()->rootGroup(), childLayerNode->layer() ) == 1 )
          disconnect( vlayer, &QgsVectorLayer::dataSourceChanged, this, &QgsLayerTreeViewBadLayerIndicatorProvider::onDataSourceChanged );
      }
    }
  }
}

void QgsLayerTreeViewBadLayerIndicatorProvider::onLayerLoaded()
{
  QgsLayerTreeLayer *layerNode = qobject_cast<QgsLayerTreeLayer *>( sender() );
  if ( !layerNode )
    return;

  if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layerNode->layer() ) )
  {
    if ( vlayer )
    {
      connect( vlayer, &QgsVectorLayer::dataSourceChanged, this, &QgsLayerTreeViewBadLayerIndicatorProvider::onDataSourceChanged );
      addOrRemoveIndicator( layerNode, vlayer );
    }
  }
}

void QgsLayerTreeViewBadLayerIndicatorProvider::onDataSourceChanged()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( sender() );
  if ( !vlayer )
    return;

  // walk the tree and find layer node that needs to be updated
  const QList<QgsLayerTreeLayer *> layerNodes = mLayerTreeView->layerTreeModel()->rootGroup()->findLayers();
  for ( QgsLayerTreeLayer *node : layerNodes )
  {
    if ( node->layer() && node->layer() == vlayer )
    {
      addOrRemoveIndicator( node, vlayer );
      break;
    }
  }
}

void QgsLayerTreeViewBadLayerIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->layerTreeModel()->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( node )->layer() );
  if ( !vlayer )
    return;

  // TODO: Open set data source dialog
  //QgisApp::instance()->makeMemoryLayerPermanent( vlayer );
}

std::unique_ptr<QgsLayerTreeViewIndicator> QgsLayerTreeViewBadLayerIndicatorProvider::newIndicator()
{
  std::unique_ptr< QgsLayerTreeViewIndicator > indicator = qgis::make_unique< QgsLayerTreeViewIndicator >( this );
  indicator->setIcon( mIcon );
  indicator->setToolTip( tr( "<b>Bad layer!</b><br>Layer data source could not be found, click here to set a new data source." ) );
  connect( indicator.get(), &QgsLayerTreeViewIndicator::clicked, this, &QgsLayerTreeViewBadLayerIndicatorProvider::onIndicatorClicked );
  mIndicators.insert( indicator.get() );
  return indicator;
}


void QgsLayerTreeViewBadLayerIndicatorProvider::addOrRemoveIndicator( QgsLayerTreeNode *node, QgsVectorLayer *vlayer )
{

  if ( ! vlayer->isValid() )
  {
    const QList<QgsLayerTreeViewIndicator *> nodeIndicators = mLayerTreeView->indicators( node );

    // maybe the indicator exists already
    for ( QgsLayerTreeViewIndicator *indicator : nodeIndicators )
    {
      if ( mIndicators.contains( indicator ) )
      {
        return;
      }
    }

    // it does not exist: need to create a new one
    mLayerTreeView->addIndicator( node, newIndicator().release() );
  }
  else
  {
    const QList<QgsLayerTreeViewIndicator *> nodeIndicators = mLayerTreeView->indicators( node );

    // there may be existing indicator we need to get rid of
    for ( QgsLayerTreeViewIndicator *indicator : nodeIndicators )
    {
      if ( mIndicators.contains( indicator ) )
      {
        mLayerTreeView->removeIndicator( node, indicator );
        indicator->deleteLater();
        return;
      }
    }
    // no indicator was there before, nothing to do
  }
}

