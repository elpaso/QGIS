/***************************************************************************
  qgsrelationadddlg.cpp - QgsRelationAddDlg
  ---------------------------------

 begin                : 4.10.2013
 copyright            : (C) 2013 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelationadddlg.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerproxymodel.h"
#include <set>
#include <QPushButton>

QgsRelationAddDlg::QgsRelationAddDlg( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );

  connect( mCbxReferencingLayer, &QgsMapLayerComboBox::layerChanged, mCbxReferencingField, &QgsFieldComboBox::setLayer );
  connect( mCbxReferencedLayer, &QgsMapLayerComboBox::layerChanged, mCbxReferencedField, &QgsFieldComboBox::setLayer );

  mCbxReferencingLayer->setFilters( QgsMapLayerProxyModel::VectorLayer );
  mCbxReferencingField->setLayer( mCbxReferencingLayer->currentLayer() );
  mCbxReferencedLayer->setFilters( QgsMapLayerProxyModel::VectorLayer );
  mCbxReferencedField->setLayer( mCbxReferencedLayer->currentLayer() );

  mCbxRelationStrength->addItem( tr( "Association" ), QVariant::fromValue( QgsRelation::RelationStrength::Association ) );
  mCbxRelationStrength->addItem( tr( "Composition" ), QVariant::fromValue( QgsRelation::RelationStrength::Composition ) );
  mCbxRelationStrength->setToolTip( tr( "On composition, the child features will be duplicated too.\nDuplications are made by the feature duplication action.\nThe default actions are activated in the Action section of the layer properties." ) );

  mTxtRelationId->setPlaceholderText( tr( "[Generated automatically]" ) );
  checkDefinitionValid();

  connect( mCbxReferencingLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddDlg::checkDefinitionValid );
  connect( mCbxReferencingField, &QgsFieldComboBox::fieldChanged, this, &QgsRelationAddDlg::checkDefinitionValid );
  connect( mCbxReferencedLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddDlg::checkDefinitionValid );
  connect( mCbxReferencedField, &QgsFieldComboBox::fieldChanged, this, &QgsRelationAddDlg::checkDefinitionValid );

  mFieldsTableWidget->setHorizontalHeaderLabels( QStringList() << tr( "Referencing field" ) << tr( "Referenced field" ) );
  mFieldsTableWidget->horizontalHeader()->setStretchLastSection( true );
  mFieldsTableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

  // Add a new field pair to the relation
  connect( mAddRelationButton, &QPushButton::clicked, [ = ]
  {
    if ( !mCbxReferencingField->currentField().isEmpty() && !mCbxReferencedField->currentField().isEmpty() &&
         !this->references().contains( QPair<QString, QString>( mCbxReferencingField->currentField(), mCbxReferencedField->currentField() ) ) )
    {
      mFieldsTableWidget->insertRow( 0 );
      mFieldsTableWidget->setItem( 0, 0, new QTableWidgetItem( mCbxReferencingField->currentField() ) );
      mFieldsTableWidget->setItem( 0, 1, new QTableWidgetItem( mCbxReferencedField->currentField() ) );
    }
  } );

  // Remove selected field pair(s) from the relation
  connect( mRemoveSelectedRelationButton, &QPushButton::clicked, [ = ]
  {
    if ( mFieldsTableWidget->selectedItems().count() )
    {
      std::vector<int> toRemove;
      const auto constItems = mFieldsTableWidget->selectedItems();
      for ( const auto &item : constItems )
      {
        toRemove.push_back( item->row() );
      }
      std::sort( toRemove.rbegin(), toRemove.rend() );
      std::unique( toRemove.begin(), toRemove.end() );
      for ( const auto &idx : toRemove )
      {
        mFieldsTableWidget->removeRow( idx );
      }
    }
  } );

  connect( mFieldsTableWidget, &QTableWidget::itemChanged, [ = ]( QTableWidgetItem * )
  {
    mCbxReferencingLayer->setEnabled( mFieldsTableWidget->rowCount() == 0 );
    mCbxReferencedLayer->setEnabled( mFieldsTableWidget->rowCount() == 0 );
    mRemoveSelectedRelationButton->setEnabled( mFieldsTableWidget->rowCount() > 0 );
  } );

}

QgsRelationAddDlg::QgsRelationAddDlg( const QgsRelation &relation, QWidget *parent ):
  QgsRelationAddDlg( parent )
{
  mCbxReferencingLayer->setLayer( relation.referencingLayer() );
  mCbxReferencedLayer->setLayer( relation.referencedLayer() );
  mCbxRelationStrength->setCurrentIndex( mCbxRelationStrength->findData( relation.strength() ) );
  mTxtRelationId->setText( relation.id() );
  mTxtRelationName->setText( relation.name() );
  const auto constPairs { relation.fieldPairs() };
  for ( const auto &pair : constPairs )
  {
    mFieldsTableWidget->insertRow( 0 );
    mFieldsTableWidget->setItem( 0, 0, new QTableWidgetItem( pair.first ) );
    mFieldsTableWidget->setItem( 0, 1, new QTableWidgetItem( pair.second ) );
  }
}

QString QgsRelationAddDlg::referencingLayerId() const
{
  return mCbxReferencingLayer->currentLayer()->id();
}

QString QgsRelationAddDlg::referencedLayerId() const
{
  return mCbxReferencedLayer->currentLayer()->id();
}

QList< QPair< QString, QString > > QgsRelationAddDlg::references() const
{
  QList< QPair< QString, QString > > references;

  for ( int i = 0; i < mFieldsTableWidget->rowCount(); ++i )
  {
    references.append( QPair<QString, QString> ( mFieldsTableWidget->item( i, 0 )->text(), mFieldsTableWidget->item( i, 1 )->text() ) );
  }

  return references;
}

QString QgsRelationAddDlg::relationId() const
{
  return mTxtRelationId->text();
}

QString QgsRelationAddDlg::relationName() const
{
  return mTxtRelationName->text();
}

QgsRelation::RelationStrength QgsRelationAddDlg::relationStrength() const
{
  return mCbxRelationStrength->currentData().value<QgsRelation::RelationStrength>();
}

void QgsRelationAddDlg::checkDefinitionValid()
{
  // TODO check that reference and referencing layers are not the same?
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( mFieldsTableWidget->rowCount() > 0 );
}
