/***************************************************************************
    qgsvaluerelationwidgetwrapper.h
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

#ifndef QGSVALUERELATIONWIDGETWRAPPER_H
#define QGSVALUERELATIONWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"
#include "qgsvaluerelationfieldformatter.h"
#include "qgis_gui.h"

class QTableWidget;
class QComboBox;
class QLineEdit;

SIP_NO_FILE

class QgsValueRelationWidgetFactory;

/**
 * \ingroup gui
 * Wraps a value relation widget. This widget will offer a combobox with values from another layer
 * referenced by a foreign key (a constraint may be set but is not required on data level).
 * This is useful for having value lists on a separate layer containing codes and their
 * translation to human readable names.
 *
 * Options:
 *
 * <ul>
 * <li><b>Layer</b> <i>The id of the referenced layer.</i></li>
 * <li><b>Key</b> <i>The key field on the referenced layer (code).</i></li>
 * <li><b>Value</b> <i>The value field on the referenced layer (human readable name).</i></li>
 * <li><b>AllowMulti</b> <i>If set to True, will allow multiple selections. This requires the data type to be a string. This does NOT work with normalized database structures.</i></li>
 * <li><b>AllowNull</b> <i>Will offer NULL as a possible value.</i></li>
 * <li><b>FilterExpression</b> <i>If not empty, will be used as expression. Only if this evaluates to True, the value will be shown.</i></li>
 * <li><b>OrderByValue</b> <i>Will order by value instead of key.</i></li>
 * </ul>
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsValueRelationWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT

  public:
    explicit QgsValueRelationWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *editor = nullptr, QWidget *parent = nullptr );

    QVariant value() const override;

    void showIndeterminateState() override;

    void setEnabled( bool enabled ) override;

  protected:
    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;
    bool valid() const override;

  public slots:

    void setValue( const QVariant &value ) override;

    /**
     * Will be called when a value in the current edited form or table row
     * changes
     *
     * Update widget cache if the value is used in the filter expression and
     * stores current field values to be used in expression form scope context
     *
     * \param attribute The attribute name
     * \param value The current value
     */
    void formValueChanged( const QString &attribute, const QVariant &value );

    /**
     * Will be called when the feature changes
     *
     * Is forwarded to the slot setValue()
     *
     * \param feature The new feature
     */
    void setFeature( const QgsFeature &feature ) override;

  private:

    void populate( );

    QComboBox *mComboBox = nullptr;
    QTableWidget *mTableWidget = nullptr;
    QLineEdit *mLineEdit = nullptr;

    QgsValueRelationFieldFormatter::ValueRelationCache mCache;
    QgsVectorLayer *mLayer = nullptr;
    QVariantMap mFormValues;

    bool mEnabled = true;

    friend class QgsValueRelationWidgetFactory;
    friend class TestQgsValueRelationWidgetWrapper;
};

#endif // QGSVALUERELATIONWIDGETWRAPPER_H
