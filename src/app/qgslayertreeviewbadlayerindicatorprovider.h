/***************************************************************************
  qgslayertreeviewbadlayerindicatorprovider.h - QgsLayerTreeViewBadLayerIndicatorProvider

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
#ifndef QGSLAYERTREEVIEWBADLAYERINDICATORPROVIDER_H
#define QGSLAYERTREEVIEWBADLAYERINDICATORPROVIDER_H

#include "qgslayertreeviewindicator.h"

#include <QObject>
#include <QSet>
#include <memory>

class QgsLayerTreeNode;
class QgsLayerTreeView;
class QgsVectorLayer;


//! Indicators for bad layers
class QgsLayerTreeViewBadLayerIndicatorProvider : public QObject
{
    Q_OBJECT

  public:
    explicit QgsLayerTreeViewBadLayerIndicatorProvider( QgsLayerTreeView *view );

  signals:

  private slots:
    //! Connects to signals of layers newly added to the tree
    void onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Disconnects from layers about to be removed from the tree
    void onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void onLayerLoaded();
    //! Adds/removes indicator of a layer
    void onDataSourceChanged();

    void onIndicatorClicked( const QModelIndex &index );

  private:

    std::unique_ptr< QgsLayerTreeViewIndicator > newIndicator();
    void addOrRemoveIndicator( QgsLayerTreeNode *node, QgsVectorLayer *vlayer );

    QgsLayerTreeView *mLayerTreeView = nullptr;
    QIcon mIcon;
    QSet<QgsLayerTreeViewIndicator *> mIndicators;

};

#endif // QGSLAYERTREEVIEWBADLAYERINDICATORPROVIDER_H
