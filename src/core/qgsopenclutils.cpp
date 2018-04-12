/***************************************************************************
  qgsopenclutils.cpp - QgsOpenClUtils

 ---------------------
 begin                : 11.4.2018
 copyright            : (C) 2018 by elpaso
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsopenclutils.h"
#include "qgssettings.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"

#include <QTextStream>
#include <QFile>

QLatin1String QgsOpenClUtils::SETTINGS_KEY = QLatin1Literal( "OpenClEnabled" );

bool QgsOpenClUtils::enabled()
{
  return QgsSettings().value( SETTINGS_KEY, true, QgsSettings::Section::Core ).toBool();
}

bool QgsOpenClUtils::available()
{
  std::vector<cl::Platform> platforms;
  cl::Platform::get( &platforms );
  cl::Platform plat;
  // TODO: should we also accept > 1.1 ?
  for ( auto &p : platforms )
  {
    std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
    QgsDebugMsg( QStringLiteral( "Found platform %1: %2" ).arg( QString::fromStdString( platver ), QString::fromStdString( p.getInfo<CL_PLATFORM_NAME>() ) ) );
    if ( platver.find( "OpenCL 1.1" ) != std::string::npos )
    {
      plat = p;
    }
  }
  if ( plat() == 0 )
  {
    QgsMessageLog::logMessage( QObject::tr( "No OpenCL 1.1 platform found." ), QStringLiteral( "OpenCL" ), Qgis::Warning );
    return false;
  }
  cl::Platform newP = cl::Platform::setDefault( plat );
  if ( newP != plat )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error setting default platform." ), QStringLiteral( "OpenCL" ), Qgis::Warning );
    return false;
  }
  std::string platname = plat.getInfo<CL_PLATFORM_NAME>();
  QgsMessageLog::logMessage( QObject::tr( "OpenCL platform found: %1" ).arg( QString::fromStdString( platname ) ), QStringLiteral( "OpenCL" ), Qgis::Info );
  return true;
}

void QgsOpenClUtils::enable()
{
  QgsSettings().setValue( SETTINGS_KEY, true, QgsSettings::Section::Core );
}

void QgsOpenClUtils::disable()
{
  QgsSettings().setValue( SETTINGS_KEY, false, QgsSettings::Section::Core );
}

std::unique_ptr<cl::Program> QgsOpenClUtils::loadProgram( const QString &path )
{
  // Try to load the program sources
  QString source_str;

  QFile file( path );
  if ( ! file.open( QFile::ReadOnly | QFile::Text ) )
    return nullptr;

  QTextStream in( &file );
  source_str = in.readAll();
  file.close();

  // Create a program from the kernel source
  std::unique_ptr<cl::Program> program = qgis::make_unique<cl::Program>( source_str.toStdString().c_str(), true );
  return program;
}


