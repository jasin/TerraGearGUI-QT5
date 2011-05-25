/*
    file: tggui_utils.cpp

    Written by Geoff R. Mclane, started May 2011.

    Copyright (C) 2011  Geoff R. McLane - reports@geoffair.info

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id: 0.0.1 2001-05-07 - version 0.0.1 $
   */

#include "tggui_utils.h"

/*
    Utility function that recursivily searches for files.
*/
QStringList findFiles(const QString &startDir, QStringList filters)
{
    QStringList names;
    QDir dir(startDir);

    foreach (QString file, dir.entryList(filters, QDir::Files))
        names += startDir + "/" + file;

    foreach (QString subdir, dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        names += findFiles(startDir + "/" + subdir, filters);
    return names;
}

// return
// 1 = Ok was clicked - the default
// 0 = Cancel or otherwise
int getYesNo( QString title, QString msg)
{
    QMessageBox msgBox;
    msgBox.setText(title); // set TITLE text
    msgBox.setIcon(QMessageBox::Information); // set predefined icon, icon is show on left side of text.
    msgBox.setInformativeText(msg); // set the Question
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No); // Add ok and cancel button.
    msgBox.setDefaultButton(QMessageBox::Yes);  //Set focus of ok button
    //execute message box. method exec() return the button value of cliecke button
    int ret = msgBox.exec();
    //User get input from returned value (ret). you can handle it here.
    switch (ret)
    {
    case QMessageBox::Yes:
        ret = 1;    // 1 for OK to continue
        break;
    case QMessageBox::No:
        ret = 0; // Cancel was clicked
        break;
    default:
        // should never be reached
        ret = 0; // but assume Cancel
        break;
    }
    return ret;
}

// givem millisecond, return appropriate string
QString getElapTimeStg(int ms)
{
    QString fmt = "";
    if (ms < 1000) {
        fmt.sprintf("%d ms", ms);
    } else {
        int secs = ms / 1000;
        ms = ms % 1000;
        if (secs < 60) {
            fmt.sprintf("%d.%03d secs", secs, ms);
        } else {
            int mins = secs / 60;
            secs = secs % 60;
            if (mins < 60) {
                fmt.sprintf("%d:%02d.%03d mins", mins, secs, ms);
            } else {
                int hrs = mins / 60;
                mins = mins % 60;
                fmt.sprintf("%d:%02d:%02d.%03d hrs", hrs, mins, secs, ms);
            }
        }
    }
    return fmt;
}

int util_verifySRTMfiles(QString minLat, QString maxLat, QString minLon, QString maxLon, QString workDirectory)
{
    int result = 0;
    QStringList elev_src;
    elev_src <<  "SRTM2-Africa-3" << "SRTM2-Australia-3" << "SRTM2-Eurasia-3" << "SRTM2-Islands-3" <<
            "SRTM2-North_America-3" << "SRTM2-South_America-3" << "DEM-USGS-3" << "SRTM-1" << "SRTM-3" << "SRTM-30";
    // QRegExp rx("\/\d+\.arr\.gz");
    QStringList indexList;
    QStringList indexList_needed;
    int i, j, k, l;
    QString index = "";
#ifdef _NEWBUCKET_HXX   // we have SGBucket capability
    // break the set into buckets
    double dminLon = minLon.toDouble();
    double dminLat = minLat.toDouble();
    double dmaxLon = maxLon.toDouble();
    double dmaxLat = maxLat.toDouble();

    SGBucket b_min( dminLon, dminLat );
    SGBucket b_max( dmaxLon, dmaxLat );
    if (b_min == b_max) {
        // just one bucket
        index.sprintf("%ld", b_min.gen_index());
        indexList_needed += index;
    } else {
        // we have a range of buckets
        int dx, dy;
        SGBucket b_cur;
        sgBucketDiff(b_min, b_max, &dx, &dy);
        for ( j = 0; j <= dy; j++ ) {
            for ( i = 0; i <= dx; i++ ) {
                b_cur = sgBucketOffset(dminLon, dminLat, i, j);
                index.sprintf("%ld", b_cur.gen_index());
                indexList_needed += index;
            }
        }
    }
    // ok, got a list of indexes needed
#endif // #ifdef _NEWBUCKET_HXX   // we have SGBucket capability
    QStringList files;
    for (l = 0; l < elev_src.size(); l++) {
        QDir dir(workDirectory+"/"+elev_src[l]);
        if (dir.exists()) {
            files = findFiles(dir.absolutePath(), QStringList() << "*.arr.gz" << "*.fit.gz");
            if (files.count() > 0) {
#ifdef _NEWBUCKET_HXX   // we have SGBucket capability
                foreach (QString file, files) {
                    //QFile f(file);
                    //if (file.contains(rx)) {
                    for (i = 0; i < file.size(); i++) {
                        if ( file.at(i) == QChar('/') ) {
                            // got a path separator - get index
                            index = ""; // clear the string
                            for (j = i + 1; j < file.size(); j++) {
                                if ((file.at(j) >= QChar('0')) && (file.at(j) <= QChar('9'))) {
                                    index += file.at(j);
                                } else break;
                            }
                            if ((j < file.size()) && (file.at(j) == QChar('.')) && (index.size() == 7)) {
                                for (k = 0; k < indexList.size(); k++) {
                                    if (indexList[k] == index)
                                        break;
                                }
                                if (k == indexList.size())
                                    indexList += index;
                            }
                        }
                    }
                }
#else // !#ifdef _NEWBUCKET_HXX   // NO SGBucket capability
                result++; // we at least HAVE some elevation files
#endif // #ifdef _NEWBUCKET_HXX   // we have SGBucket capability y/n
            }
        }
    }
#ifdef _NEWBUCKET_HXX   // we have SGBucket capability
    for (i = 0; i < indexList.size(); i++) {
        for (j = 0; j < indexList_needed.size(); j++) {
            if (indexList[i] == indexList_needed[j])
                result++; // even if only one result in many, it is better than none
        }
    }
    // show what was found
    //index.sprintf("Found %d arr/fit files in the 'work' directory.", files.count());
    //outputToLog(index);
    //index.sprintf("Of the desired %d bucket indexes, found elevations (arr/fit) for %d tiles.", indexList_needed.size(), result);
    //outputToLog(index);
#endif // #ifdef _NEWBUCKET_HXX   // we have SGBucket capability

    return result; // If 0, oops, NO ELEVATION directory located in work, or no matching indexes
}

// given a SGBucket path = <chunk>/<1x1>/index, and a space separated list of directories to try,
int countDataFound(QString in_path, QString selectedMaterials, QString workDirectory)
{
    QStringList materList = selectedMaterials.split(QChar(' '),QString::SkipEmptyParts); // space split list
    QStringList pathList = in_path.split(QChar('/'));
    int i, totFound;
    totFound = 0;
    if ((pathList.size() == 3) && (materList.size() > 0)) {
        // we have a chance...
        QString path = pathList[0]+"/"+pathList[1];
        QString index = pathList[2]; // the bucket INDEX
        for (i = 0; i < materList.size(); i++) {
            QString landUse = materList[i];
            // got <chunk>/<1x1> path
            QString test = workDirectory+"/"+landUse+"/"+path;
            QDir dir(test);
            if (dir.exists()) {
                // ok, this <worK>/<landuse>/<chunk>/<1x1>/<index>.* exists
                QStringList files = findFiles(dir.absolutePath(), QStringList() << index+".*");
                totFound += files.count();
            }
        }
    }
    return totFound; // ZERO if NO DATA FOUND - not good to continue ;=))
}

// eof - tggui_utils.cpp

