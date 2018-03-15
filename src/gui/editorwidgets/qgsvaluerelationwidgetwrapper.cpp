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

  if ( mListWidget )
  {
    QStringList selection;
    for ( int i = 0; i < mListWidget->count(); ++i )
    {
      QListWidgetItem *item = mListWidget->item( i );
      if ( item->checkState() == Qt::Checked )
        selection << item->data( Qt::UserRole ).toString();
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
    connect( form, &QgsAttributeForm::formValueChanged, this, &QgsValueRelationWidgetWrapper::formValueChanged );
  if ( config( QStringLiteral( "AllowMulti" ) ).toBool() )
  {
    return new QListWidget( parent );
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
  mListWidget = qobject_cast<QListWidget *>( editor );
  mLineEdit = qobject_cast<QLineEdit *>( editor );

  // Read current initial form values from the editor context
  mFormValues = context().formValues();

  populate();

  if ( mComboBox )
  {
    connect( mComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
             this, static_cast<void ( QgsEditorWidgetWrapper::* )()>( &QgsEditorWidgetWrapper::emitValueChanged ) );
  }
  else if ( mListWidget )
  {
    connect( mListWidget, &QListWidget::itemChanged, this, static_cast<void ( QgsEditorWidgetWrapper::* )()>( &QgsEditorWidgetWrapper::emitValueChanged ) );
  }
  else if ( mLineEdit )
  {
    connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & value ) { emit valueChanged( value ); } );
  }
}

bool QgsValueRelationWidgetWrapper::valid() const
{
  return mListWidget || mLineEdit || mComboBox;
}

void QgsValueRelationWidgetWrapper::setValue( const QVariant &value )
{
  if ( mListWidget )
  {
    QStringList checkList;
    if ( value.type() == QVariant::StringList )
      checkList = value.toStringList();
    else if ( value.type() == QVariant::String )
      checkList = value.toString().remove( QChar( '{' ) ).remove( QChar( '}' ) ).split( ',' );

    for ( int i = 0; i < mListWidget->count(); ++i )
    {
      QListWidgetItem *item = mListWidget->item( i );
      item->setCheckState( checkList.contains( item->data( Qt::UserRole ).toString() ) ? Qt::Checked : Qt::Unchecked );
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

void QgsValueRelationWidgetWrapper::formValueChanged( const QString &attribute, const QVariant &newValue )
{
  // Exit if the value has not changed: no need to repopulate
  if ( mFormValues.contains( attribute ) && mFormValues[ attribute ] == newValue )
  {
    return;
  }
  // Store/update the value
  mFormValues[ attribute ] = newValue;
  // Update combos if the value used in the filter expression has changed
  if ( QgsValueRelationFieldFormatter::expressionRequiresFormScope( config(), attribute ) )
  {
    populate();
    // Restore value
    setValue( value( ) );
  }
}

void QgsValueRelationWidgetWrapper::setFeature( const QgsFeature &feature )
{
  mFeature = feature;
  mFormValues.clear();
  bool hasValidValues = false;
  const QgsFields fields( feature.fields( ) );
  for ( const auto &field : fields )
  {
    QVariant value = feature.attribute( field.name( ) );
    if ( value.isValid( ) )
    {
      mFormValues[ field.name( ) ] = feature.attribute( field.name( ) );
      hasValidValues = true;
    }
  }
  if ( hasValidValues && QgsValueRelationFieldFormatter::expressionRequiresFormScope( config( ) ) )
    populate();
  setValue( feature.attribute( mFieldIdx ) );
}

void QgsValueRelationWidgetWrapper::populate( )
{
  // Initialize
  if ( QgsValueRelationFieldFormatter::expressionRequiresFormScope( config( ) ) && ! mFormValues.isEmpty( ) )
  {
    mCache = QgsValueRelationFieldFormatter::createCache( config( ), mFormValues );
  }
  else if ( mCache.isEmpty() )
  {
    mCache = QgsValueRelationFieldFormatter::createCache( config( ) );
  }

  if ( mComboBox )
  {
    mComboBox->clear();
    if ( config( QStringLiteral( "AllowNull" ) ).toBool( ) )
    {
      mComboBox->addItem( tr( "(no selection)" ), QVariant( field().type( ) ) );
    }

    Q_FOREACH ( const QgsValueRelationFieldFormatter::ValueRelationItem &element, mCache )
    {
      mComboBox->addItem( element.value, element.key );
    }
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
  if ( mListWidget )
  {
    mListWidget->blockSignals( true );
    for ( int i = 0; i < mListWidget->count(); ++i )
    {
      mListWidget->item( i )->setCheckState( Qt::PartiallyChecked );
    }
    mListWidget->blockSignals( false );
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

  if ( mListWidget )
  {
    for ( int i = 0; i < mListWidget->count(); ++i )
    {
      QListWidgetItem *item = mListWidget->item( i );

      if ( enabled )
        item->setFlags( item->flags() | Qt::ItemIsEnabled );
      else
        item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
    }
  }
  else
    QgsEditorWidgetWrapper::setEnabled( enabled );
}
