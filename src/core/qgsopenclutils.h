/***************************************************************************
  qgsopenclutils.h - QgsOpenClUtils

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
#ifndef QGSOPENCLUTILS_H
#define QGSOPENCLUTILS_H

#define SIP_NO_FILE

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#define CL_HPP_TARGET_OPENCL_VERSION 110
#include <CL/cl2.hpp>


#include "qgis_core.h"
#include "qgis.h"

/**
 * \ingroup core
 * \class QgsOpenClUtils
 * \brief The QgsOpenClUtils class is responsible for common OpenCL operations
 * \since QGIS 3.4
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsOpenClUtils
{
  public:
    static bool enabled();
    static bool available();
    static void enable();
    static void disable();

    static std::unique_ptr<cl::Program> loadProgram( const QString &path );

    static QLatin1String SETTINGS_KEY;
  private:
    QgsOpenClUtils();
};


#endif // QGSOPENCLUTILS_H
