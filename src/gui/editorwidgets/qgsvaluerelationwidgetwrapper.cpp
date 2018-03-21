/***************************************************************************
    qgsvaluerelationwidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvaluerelationwidgetwrapper.h"

#include "qgis.h"
#include "qgsfields.h"
#include "qgsproject.h"
#include "qgsvaluerelationwidgetfactory.h"
#include "qgsvectorlayer.h"
#include "qgsfilterlineedit.h"
#include "qgsfeatureiterator.h"
#include "qgsvaluerelationfieldformatter.h"
#include "qgsattributeform.h"
#include "qgsattributes.h"

#include <QHeaderView>
#include <QComboBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QStringListModel>
#include <QCompleter>

QgsValueRelationWidgetWrapper::QgsValueRelationWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
{
}


QVariant QgsValueRelationWidgetWrapper::value() const
{
  QVariant v;

  if ( mComboBox )
  {
    int cbxIdx = mComboBox->currentIndex();
    if ( cbxIdx > -1 )
    {
      v = mComboBox->currentData();
    }
  }

  if ( mTableWidget )
  {
    QStringList selection;
    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < config( QStringLiteral( "NofColumns" ) ).toInt(); ++i )
      {
        QTableWidgetItem *item = mTableWidget->item( j, i );
        if ( item )
        {
          if ( item->checkState() == Qt::Checked )
            selection << item->data( Qt::UserRole ).toString();
        }
      }
    }
    v = selection.join( QStringLiteral( "," ) ).prepend( '{' ).append( '}' );
  }

  if ( mLineEdit )
  {
    Q_FOREACH ( const QgsValueRelationFieldFormatter::ValueRelationItem &item, mCache )
    {
      if ( item.value == mLineEdit->text() )
      {
        v = item.key;
        break;
      }
    }
  }

  return v;
}

QWidget *QgsValueRelationWidgetWrapper::createWidget( QWidget *parent )
{
  QgsAttributeForm *form = dynamic_cast<QgsAttributeForm *>( parent );
  if ( form )
    connect( form, &QgsAttributeForm::attributeChanged, this, &QgsValueRelationWidgetWrapper::attributeChanged );
  if ( config( QStringLiteral( "AllowMulti" ) ).toBool() )
  {
    return new QTableWidget( parent );
  }
  else if ( config( QStringLiteral( "UseCompleter" ) ).toBool() )
  {
    return new QgsFilterLineEdit( parent );
  }
  {
    return new QComboBox( parent );
  }
}

void QgsValueRelationWidgetWrapper::initWidget( QWidget *editor )
{

  mComboBox = qobject_cast<QComboBox *>( editor );
  mTableWidget = qobject_cast<QTableWidget *>( editor );
  mLineEdit = qobject_cast<QLineEdit *>( editor );

  // Read current initial form values from the editor context
  setFeature( context().formFeature() );

  if ( mComboBox )
  {
    connect( mComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
             this, static_cast<void ( QgsEditorWidgetWrapper::* )()>( &QgsEditorWidgetWrapper::emitValueChanged ), Qt::UniqueConnection );
  }
  else if ( mTableWidget )
  {
    mTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
    mTableWidget->horizontalHeader()->setVisible( false );
    mTableWidget->verticalHeader()->setResizeMode( QHeaderView::Stretch );
    mTableWidget->verticalHeader()->setVisible( false );
    mTableWidget->setShowGrid( false );
    mTableWidget->setEditTriggers( QAbstractItemView::NoEditTriggers );
    mTableWidget->setSelectionMode( QAbstractItemView::NoSelection );
    if ( mCache.size() > 0 )
      mTableWidget->setRowCount( ( mCache.size() + config( QStringLiteral( "NofColumns" ) ).toInt() - 1 ) / config( QStringLiteral( "NofColumns" ) ).toInt() );
    else
      mTableWidget->setRowCount( 1 );
    if ( config( QStringLiteral( "NofColumns" ) ).toInt() > 0 )
      mTableWidget->setColumnCount( config( QStringLiteral( "NofColumns" ) ).toInt() );
    else
      mTableWidget->setColumnCount( 1 );

    int row = 0, column = 0;
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &element : qgis::as_const( mCache ) )
    {
      if ( column == config( QStringLiteral( "NofColumns" ) ).toInt() )
      {
        row++;
        column = 0;
      }
      QTableWidgetItem *item = nullptr;
      item = new QTableWidgetItem( element.value );
      item->setData( Qt::UserRole, element.key );
      mTableWidget->setItem( row, column, item );
      column++;
    }
    connect( mTableWidget, &QTableWidget::itemChanged, this, static_cast<void ( QgsEditorWidgetWrapper::* )()>( &QgsEditorWidgetWrapper::emitValueChanged ), Qt::UniqueConnection );
  }
  else if ( mLineEdit )
  {
    QStringList values;
    values.reserve( mCache.size() );
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &i : qgis::as_const( mCache ) )
    {
      values << i.value;
    }
    QStringListModel *m = new QStringListModel( values, mLineEdit );
    QCompleter *completer = new QCompleter( m, mLineEdit );
    completer->setCaseSensitivity( Qt::CaseInsensitive );
    mLineEdit->setCompleter( completer );
    connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & value ) { emit valueChanged( value ); }, Qt::UniqueConnection );
  }
}

bool QgsValueRelationWidgetWrapper::valid() const
{
  return mTableWidget || mLineEdit || mComboBox;
}

void QgsValueRelationWidgetWrapper::setValue( const QVariant &value )
{
  if ( mTableWidget )
  {
    QStringList checkList( QgsValueRelationFieldFormatter::valueToStringList( value ) );

    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < config( QStringLiteral( "NofColumns" ) ).toInt() ; ++i )
      {
        QTableWidgetItem *item = mTableWidget->item( j, i );
        if ( item )
        {
          item->setCheckState( checkList.contains( item->data( Qt::UserRole ).toString() ) ? Qt::Checked : Qt::Unchecked );
        }
      }
    }
  }
  else if ( mComboBox )
  {
    mComboBox->setCurrentIndex( mComboBox->findData( value ) );
  }
  else if ( mLineEdit )
  {
    Q_FOREACH ( QgsValueRelationFieldFormatter::ValueRelationItem i, mCache )
    {
      if ( i.key == value )
      {
        mLineEdit->setText( i.value );
        break;
      }
    }
  }
}

void QgsValueRelationWidgetWrapper::attributeChanged( const QString &attribute, const QVariant &newValue )
{
  /*
   QVariantMap attrs;
   for ( const auto &f : mFeature.fields() )
   {
     if ( mFeature.attribute( f.name() ).isValid() )
       attrs[ f.name() ] = mFeature.attribute( f.name() );
   }
   qDebug( ) << "Feature attrs: " << attrs;
   qDebug( ) << "Form values: " << mFormValues;
   //Q_ASSERT( attrs == mFormValues );
   */

  // Do nothing if the value has not changed
  if ( ! mFormValues.contains( attribute ) || mFormValues[ attribute ] != newValue )
  {
    // Only keep valid values
    if ( newValue.isValid( ) )
      mFormValues[ attribute ] = newValue;
    else
      mFormValues.remove( attribute );
    mFeature.setAttribute( attribute, newValue );
    // Update combos if the value used in the filter expression has changed
    if ( QgsValueRelationFieldFormatter::expressionRequiresFormScope( config(), mFeature ) )
    {
      populate();
      // Restore value
      setValue( value( ) );
    }
    qDebug( ) << "Attribute " << attribute << " changed to " << newValue;
  }
}


void QgsValueRelationWidgetWrapper::setFeature( const QgsFeature &feature )
{
  qDebug( ) << "Set feature attributes: " << feature.attributes();
  qDebug( ) << "Set feature isValid : " << feature.isValid();
  qDebug( ) << "Set feature geometry : " << feature.geometry().asWkt();
  mFeature = feature;
  mFormValues.clear();
  const QgsFields fields( feature.fields( ) );
  for ( const auto &field : fields )
  {
    const QVariant value = feature.attribute( field.name( ) );
    if ( value.isValid( ) )
    {
      mFormValues[ field.name( ) ] = value;
    }
  }
  //blockSignals( true );
  populate();
  setValue( feature.attribute( mFieldIdx ) );
  //blockSignals( false );
}



void QgsValueRelationWidgetWrapper::populate( )
{
  // Initialize
  if ( QgsValueRelationFieldFormatter::expressionRequiresFormScope( config( ), mFeature ) )
  {
    qDebug( ) << "Creating filtered cache for " << mFieldIdx << " form values: " << mFormValues;
    mCache = QgsValueRelationFieldFormatter::createCache( config( ), mFeature );
  }
  else if ( mCache.isEmpty() )
  {
    qDebug( ) << "Creating full cache for " << mFieldIdx << " form values: " << mFormValues;
    mCache = QgsValueRelationFieldFormatter::createCache( config( ) );
  }

  if ( mComboBox )
  {
    // Block to avoid double signals on new features
    mComboBox->blockSignals( true );
    mComboBox->clear();
    if ( config( QStringLiteral( "AllowNull" ) ).toBool( ) )
    {
      mComboBox->addItem( tr( "(no selection)" ), QVariant( field().type( ) ) );
    }

    Q_FOREACH ( const QgsValueRelationFieldFormatter::ValueRelationItem &element, mCache )
    {
      mComboBox->addItem( element.value, element.key );
    }
    mComboBox->blockSignals( false );
  }
  else if ( mListWidget )
  {
    mListWidget->clear();
    Q_FOREACH ( const QgsValueRelationFieldFormatter::ValueRelationItem &element, mCache )
    {
      QListWidgetItem *item = nullptr;
      item = new QListWidgetItem( element.value );
      item->setData( Qt::UserRole, element.key );
      mListWidget->addItem( item );
    }
  }
  else if ( mLineEdit )
  {
    QStringList values;
    values.reserve( mCache.size() );
    Q_FOREACH ( const QgsValueRelationFieldFormatter::ValueRelationItem &i,  mCache )
    {
      values << i.value;
    }

    QStringListModel *m = new QStringListModel( values, mLineEdit );
    QCompleter *completer = new QCompleter( m, mLineEdit );
    completer->setCaseSensitivity( Qt::CaseInsensitive );
    mLineEdit->setCompleter( completer );
  }
}

void QgsValueRelationWidgetWrapper::showIndeterminateState()
{
  if ( mTableWidget )
  {
    mTableWidget->blockSignals( true );
    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < config( QStringLiteral( "NofColumns" ) ).toInt(); ++i )
      {
        mTableWidget->item( j, i )->setCheckState( Qt::PartiallyChecked );
      }
    }
    mTableWidget->blockSignals( false );
  }
  else if ( mComboBox )
  {
    whileBlocking( mComboBox )->setCurrentIndex( -1 );
  }
  else if ( mLineEdit )
  {
    whileBlocking( mLineEdit )->clear();
  }
}

void QgsValueRelationWidgetWrapper::setEnabled( bool enabled )
{
  if ( mEnabled == enabled )
    return;

  mEnabled = enabled;

  if ( mTableWidget )
  {
    for ( int j = 0; j < mTableWidget->rowCount(); j++ )
    {
      for ( int i = 0; i < mTableWidget->columnCount(); ++i )
      {
        QTableWidgetItem *item = mTableWidget->item( j, i );
        if ( item )
        {
          if ( enabled )
            item->setFlags( item->flags() | Qt::ItemIsEnabled );
          else
            item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
        }
      }
    }
  }
  else
    QgsEditorWidgetWrapper::setEnabled( enabled );
}
