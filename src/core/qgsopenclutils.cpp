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
#include <QDebug>

QLatin1String QgsOpenClUtils::SETTINGS_KEY = QLatin1Literal( "OpenClEnabled" );
QLatin1String QgsOpenClUtils::LOGMESSAGE_TAG = QLatin1Literal( "OpenCL" );

bool QgsOpenClUtils::enabled()
{
  return QgsSettings().value( SETTINGS_KEY, true, QgsSettings::Section::Core ).toBool();
}

bool QgsOpenClUtils::available()
{
  static bool initialized;
  static bool _available;

  if ( !initialized )
  {
    std::vector<cl::Platform> platforms;
    cl::Platform::get( &platforms );
    cl::Platform plat;
    for ( auto &p : platforms )
    {
      std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
      QgsDebugMsg( QStringLiteral( "Found platform %1: %2" ).arg( QString::fromStdString( platver ), QString::fromStdString( p.getInfo<CL_PLATFORM_NAME>() ) ) );
      if ( platver.find( "OpenCL 1." ) != std::string::npos )
      {
        std::vector<cl::Device> devices;
        // Check for a GPU device
        p.getDevices( CL_DEVICE_TYPE_GPU, &devices );
        if ( devices.size() > 0 )
        {
          // Got one!
          plat = p;
          break;
        }
      }
    }
    if ( plat() == 0 )
    {
      QgsMessageLog::logMessage( QObject::tr( "No OpenCL 1.x platform with GPU found." ), LOGMESSAGE_TAG, Qgis::Warning );
      _available = false;
    }
    cl::Platform newP = cl::Platform::setDefault( plat );
    if ( newP != plat )
    {
      QgsMessageLog::logMessage( QObject::tr( "Error setting default platform." ), LOGMESSAGE_TAG, Qgis::Warning );
      _available = false;
    }
    _available = true;
  }
  return _available;
}

void QgsOpenClUtils::enable()
{
  QgsSettings().setValue( SETTINGS_KEY, true, QgsSettings::Section::Core );
}

void QgsOpenClUtils::disable()
{
  QgsSettings().setValue( SETTINGS_KEY, false, QgsSettings::Section::Core );
}

std::unique_ptr<cl::Program> QgsOpenClUtils::programFromString( const QString &programSource )
{

  if ( programSource.isEmpty() )
    return nullptr;

  // Create a program from the kernel source
  std::unique_ptr<cl::Program> program = qgis::make_unique<cl::Program>( programSource.toStdString().c_str(), false );
  try
  {
    program->build();
  }
  catch ( cl::BuildError e )
  {
    cl::BuildLogType build_logs = e.getBuildLog();
    QString build_log;
    if ( build_logs.size() > 0 )
      build_log = QString::fromStdString( build_logs[0].second );
    else
      build_log = QObject::tr( "Build logs not available!" );
    QgsMessageLog::logMessage( QObject::tr( "Error building OpenCL program:" )
                               .arg( build_log ), LOGMESSAGE_TAG, Qgis::Critical );
    return nullptr;
  }
  return program;

}

std::unique_ptr<cl::Program> QgsOpenClUtils::programFromPath( const QString &path )
{
  // Try to load the program sources
  QString source_str;

  QFile file( path );
  if ( ! file.open( QFile::ReadOnly | QFile::Text ) )
    return nullptr;

  QTextStream in( &file );
  source_str = in.readAll();
  file.close();
  auto program = programFromString( source_str );
  if ( !program )
    QgsMessageLog::logMessage( QObject::tr( "Error loading OpenCL program from: %1" ).arg( path ), LOGMESSAGE_TAG, Qgis::Critical );
  return program;
}

