/* tggui_utils.h
   Some utility functions
 */

#ifndef TGGUI_UTILS_H
#define TGGUI_UTILS_H

#include <QString>
#include <QStringList>
#include <QDir>
#include <QMessageBox>
#include "newbucket.h"

extern QStringList findFiles(const QString &startDir, QStringList filters);
// return
// 1 = Ok was clicked - the default
// 0 = Cancel or otherwise
extern int getYesNo( QString title, QString msg);
// givem millisecond, return appropriate string
extern QString getElapTimeStg(int ms);
// check if ANY elevation data is available, in any 'KNOWN' directory
extern int util_verifySRTMfiles(QString minLat, QString maxLat, QString minLon, QString maxLon, QString workDirectory);
// return COUNT of array files found on path <chunk>/<tile>/<index>
extern int countDataFound(QString path, QString selectedMaterials, QString workDirectory);


#endif // TGGUI_UTILS_H
// eof - tggui_utils.h
