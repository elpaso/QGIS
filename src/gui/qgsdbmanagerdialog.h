/***************************************************************************
                         qgsdbmanagerdialog.h
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

#ifndef QGSDBMANAGERDIALOG_H
#define QGSDBMANAGERDIALOG_H

#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgsdbmanagerdialog.h"
#include "qgsguiutils.h"
#include "qgsbrowserproxymodel.h"
#include "qgshelp.h"

#include <QDialog>


class QgsBrowserModel;

/**
 * \ingroup gui
 * \brief The QgsDbManagerDialog class is the new DB Manager implementation.
 */
class GUI_EXPORT QgsDbManagerDialog : public QDialog, private Ui::QgsDbManagerDialog
{
    Q_OBJECT

  public:

    QgsDbManagerDialog( QgsBrowserGuiModel *browserModel = nullptr, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

  private:

    QgsBrowserProxyModel mBrowserProxyModel;
    QgsBrowserGuiModel *mBrowserModel = nullptr;


};

#endif // QGSDBMANAGERDIALOG_H
