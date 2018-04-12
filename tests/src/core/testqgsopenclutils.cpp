/***************************************************************************
 testqgsopenclutils.cpp - TestQgsOpenClUtils

 ---------------------
 begin                : 11.4.2018
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

#include "qgstest.h"
#include <QObject>
#include <QString>
//header for class being tested
#include <qgsopenclutils.h>

class TestQgsOpenClUtils: public QObject
{
    Q_OBJECT
  private slots:

    void TestEnable();
    void TestDisable();
    void TestAvailable();
    void testLoadProgram();

};

void TestQgsOpenClUtils::TestEnable()
{
  QgsOpenClUtils::enable();
  QVERIFY( QgsOpenClUtils::enabled() );
}

void TestQgsOpenClUtils::TestDisable()
{
  QgsOpenClUtils::disable();
  QVERIFY( !QgsOpenClUtils::enabled() );
}

void TestQgsOpenClUtils::TestAvailable()
{
  QVERIFY( QgsOpenClUtils::available() );
}

void TestQgsOpenClUtils::testLoadProgram()
{
  std::unique_ptr<cl::Program> program;
  program = QgsOpenClUtils::loadProgram( QStringLiteral( " " ) ) ;
  QCOMPARE( program.get(), nullptr );
  program = QgsOpenClUtils::loadProgram( QStringLiteral( "/home/ale/dev/QGIS/src/analysis/raster/slope.cl" ) );
  QVERIFY( program.get() != nullptr );
}


QGSTEST_MAIN( TestQgsOpenClUtils )
#include "testqgsopenclutils.moc"
