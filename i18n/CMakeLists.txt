FIND_PROGRAM(QT_LRELEASE_EXECUTABLE NAMES lrelease-qt5 lrelease)

MACRO(ADD_TRANSLATION_FILES _sources )
    FOREACH (_current_FILE ${ARGN})
      GET_FILENAME_COMPONENT(_in ${_current_FILE} ABSOLUTE)
      GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)

      SET(_out ${QGIS_OUTPUT_DIRECTORY}/i18n/${_basename}.qm)

      ADD_CUSTOM_COMMAND(
         OUTPUT ${_out}
         COMMAND ${QT_LRELEASE_EXECUTABLE}
         ARGS -silent ${_in} -qm ${_out}
         DEPENDS ${_in}
      )

      SET(${_sources} ${${_sources}} ${_out} )
   ENDFOREACH (_current_FILE)
ENDMACRO(ADD_TRANSLATION_FILES)

# make sure the output directory exists
FILE(MAKE_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/i18n)

SET(TS_FILES qgis_ar.ts qgis_bs.ts qgis_ca.ts qgis_cs.ts qgis_da.ts qgis_de.ts qgis_el.ts qgis_es.ts qgis_et.ts qgis_eu.ts qgis_fi.ts qgis_fr.ts qgis_gl.ts qgis_hi.ts qgis_hu.ts qgis_id.ts qgis_is.ts qgis_it.ts qgis_ja.ts qgis_ko.ts qgis_ky.ts qgis_lt.ts qgis_lv.ts qgis_nb.ts qgis_nl.ts qgis_pl.ts qgis_pt_BR.ts qgis_pt_PT.ts qgis_ro.ts qgis_ru.ts qgis_sl.ts qgis_sv.ts qgis_tr.ts qgis_uk.ts qgis_vi.ts qgis_zh-Hans.ts qgis_zh-Hant.ts)

ADD_TRANSLATION_FILES (QM_FILES ${TS_FILES})

IF (UNIX AND NOT APPLE AND PYQT5_FOUND)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_BINARY_DIR}/org.qgis.qgis.desktop ${CMAKE_BINARY_DIR}/org.qgis.qgis.appdata.xml
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMAND ${PYTHON_EXECUTABLE}
    ARGS ${CMAKE_SOURCE_DIR}/scripts/ts2appinfo.py "${CMAKE_BINARY_DIR}"
    COMMENT "Updating appinfo files..."
    DEPENDS ${QM_FILES} ${CMAKE_SOURCE_DIR}/linux/org.qgis.qgis.desktop.in ${CMAKE_SOURCE_DIR}/linux/org.qgis.qgis.appdata.xml.in
  )

  SET(MD_FILES ${CMAKE_BINARY_DIR}/org.qgis.qgis.desktop ${CMAKE_BINARY_DIR}/org.qgis.qgis.appdata.xml)

  INSTALL(FILES ${CMAKE_BINARY_DIR}/org.qgis.qgis.desktop DESTINATION share/applications)
  INSTALL(FILES ${CMAKE_BINARY_DIR}/org.qgis.qgis.appdata.xml DESTINATION share/metainfo)
ENDIF (UNIX AND NOT APPLE AND PYQT5_FOUND)

# creating a custom target is needed to make the files build
# "ALL" means that it will be run by default
ADD_CUSTOM_TARGET (translations ALL DEPENDS ${QM_FILES} ${MD_FILES})

# first compile sources, then compile translations
IF (WITH_DESKTOP)
  ADD_DEPENDENCIES (translations ${QGIS_APP_NAME})
ENDIF (WITH_DESKTOP)

INSTALL (FILES ${QM_FILES}
         DESTINATION ${QGIS_DATA_DIR}/i18n)
