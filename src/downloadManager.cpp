/* downloadManager.cpp
//
// Written by Gijs de Rooy, started March 2010.
//
// Copyright (C) 2010-2013  Gijs de Rooy
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// $Id: $
   ==================================================================== */


//################################################################//
//################################################################//
//#####################    DOWNLOAD MANAGER    ###################//
//################################################################//
//################################################################//

void MainWindow::downloadFinished(QNetworkReply *reply)
{
    QScrollBar *sb = ui->textBrowser->verticalScrollBar();
    QUrl url = reply->url();

    if (reply->error()) {
      if ( !url.toString().contains("hgt.zip") ) {
        ui->textBrowser->append("Download of " + QString(url.toEncoded().constData()) + " failed: " + QString(reply->errorString()));
        sb->setValue(sb->maximum()); // get the info shown
      }

        QString fileUrl = url.toEncoded().constData();
    } else {
        QString path = url.path();
        QString fileName = QFileInfo(path).fileName();
        if (fileName.isEmpty()) fileName = "download";

        QDir dir(dataDirectory);
        QFile file(dataDirectory+"/"+fileName);

        if (fileName.contains("dl")) {
            // obtain url
            QFile file(dataDirectory+"/"+fileName);
        } else if (fileName.contains("hgt")) {
            // obtain elevation files
            dir.mkpath(dataDirectory+"/SRTM-3/");
            file.setFileName(dataDirectory+"/SRTM-3/"+fileName);
            GUILog( url.toString() + "\n", "download" );
        }

        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
        }

        // download actual shapefile package
        if (fileName.contains("dl")) {
            QFile dlFile(dataDirectory+"/"+fileName);
            if (!dlFile.open(QIODevice::ReadOnly | QIODevice::Text))
                return;
            QTextStream textStream( &dlFile);
            QString dlUrl = textStream.readAll();
            dlUrl.remove("<p>The document has moved <a href=\"");
            dlUrl.remove("\">here</a></p>\n");
            QNetworkReply *r = _manager->get(QNetworkRequest("http://mapserver.flightgear.org"+dlUrl));
            connect(r, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadShapefilesProgressBar(qint64, qint64)));
        }

        ui->textBrowser->append("Download of "+QString(url.toEncoded().constData())+" succeded saved to: "+QString(fileName));
        sb->setValue(sb->maximum()); // get the info shown

        // unzip shapefile package
        if (fileName.contains("-")) {
            // unpack zip
            QString arguments;
#ifdef Q_OS_WIN
            arguments += "7z.exe x \""+dataDirectory+"/"+fileName+"\" -o\""+dataDirectory+"\" -aoa";
#endif
#ifdef Q_OS_UNIX
            arguments += "unzip -o "+dataDirectory+"/"+fileName+" -d "+dataDirectory;
#endif
            //ui->textBrowser->append(arguments);
            GUILog( arguments + "\n", "download" );
            QProcess proc;
            proc.start(arguments, QIODevice::ReadWrite);
            proc.waitForReadyRead();
            proc.waitForFinished(-1);

            // delete temporary files
            QFile shapeFile(dataDirectory+"/"+fileName);
            shapeFile.remove();
            QFile dlshpFile(dataDirectory+"/dlshp");
            dlshpFile.remove();

            // re-enable download button
            ui->downloadShapefilesButton->setText("Download shapefiles");
            ui->downloadShapefilesButton->setEnabled(1);
        }

        if (fileName.contains(".hgt.zip")){
            // adjust progress bar
            ui->downloadElevationProgressBar->setValue(ui->downloadElevationProgressBar->value()+1);
        }
    }
    // re-enable download button
    if (ui->downloadElevationProgressBar->value() == ui->downloadElevationProgressBar->maximum()) {
        ui->downloadElevationButton->setEnabled(1);
    }
}

// progressBar for shapefiles synchronized with downloadManager
void MainWindow::downloadShapefilesProgressBar(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal != -1)
    {
        ui->downloadShapefilesButton->setText(QString("Downloading... (%L1MB/%L2MB)").arg(bytesReceived/1048576.0, 0, 'f', 2).arg(bytesTotal/1048576.0, 0, 'f', 2));
        ui->downloadShapefilesProgressBar->setMaximum(bytesTotal);
        ui->downloadShapefilesProgressBar->setValue(bytesReceived);
    }
}

