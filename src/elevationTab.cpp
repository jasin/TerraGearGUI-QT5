/* elevationTab.cpp
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
//#####################       RUN HGTCHOP      ###################//
//################################################################//
//################################################################//

void MainWindow::on_convertElevationButton_clicked()
{

    QScrollBar *sb       = ui->textBrowser->verticalScrollBar();
    QString elevationRes = ui->resolutionSelect->currentText();
    QString minnode      = ui->minNodesField->text();
    QString maxnode      = ui->maxNodesField->text();
    QString maxerror     = ui->maxErrorField->text();

    QString tot;
    QString cnt;
    QString tm;
    QTime rt;
    QTime pt;
    QStringList argList;
    QStringList filList;
    QString elevationFile;
    QString arguments;
    QByteArray data;
    int i;


    // reset progress bar
    ui->convertElevationProgressBar->setValue(0);
    ui->convertElevationButton->setDisabled(1);

    QDirIterator dir( elevationDirectory );
    while( dir.hasNext() ) {
        dir.next();
        if( !dir.fileInfo().isDir() ) {
            QFileInfo file = dir.filePath();
            if ( ( file.completeSuffix() == "hgt.zip" || file.completeSuffix() == "hgt" ) && elevList.contains( file.baseName() ) ) {
                argList += "\""+terragearDirectory+"/bin/hgtchop\" "+elevationRes+" \""+elevationDirectory+"/"+file.fileName()+"\" \""+workDirectory+"/SRTM-"+elevationRes+"\"";
            }
        }
    }

    if( argList.size() == 0 ) {
        QMessageBox::critical(this, "No elevation data",
                              "There are no elevation files in " + elevationDirectory + ". You can download elevation data on the Start tab." );
        return;
    }

    // process set of arguments
    for (i = 0; i < argList.size(); i++)
    {
        pt.start();
        arguments = argList[i];

        GUILog( arguments + "\n", "hgtchop" );
        GUILog( arguments + "\n", "default" );
        ui->textBrowser->append( arguments );
        sb->setValue(sb->maximum());

        QProcess proc;
        proc.setWorkingDirectory(terragearDirectory);
        proc.setProcessChannelMode(QProcess::MergedChannels);
        proc.start(arguments, QIODevice::ReadWrite);

        // run hgtchop command
        while(proc.waitForReadyRead()){
            QCoreApplication::processEvents();

            QString info( proc.readAll() );
            GUILog( info, "hgtchop" );
        }
        ui->convertElevationProgressBar->setValue( (50/argList.size())*i );  // hgtchop make progress btw 0% -> 50%
        proc.QProcess::waitForFinished(-1);

        GUILog( "ENDED in " + getElapTimeStg(pt.elapsed()) + " secondes\n", "default");
    }

    // count the number of .arr.gz files
    int numberOfArr = 0;

    QDirIterator it( workDirectory+"/SRTM-3", QDirIterator::Subdirectories );
    while( it.hasNext() ) {
        it.next();
        if( !it.fileInfo().isDir() ) {
            QString filename = it.fileName();
            if( filename.endsWith(".arr.gz") ) {
                numberOfArr += 1;
            }
        }
    }

    // start the timer
    pt.start();

    // generate and run terrafit command
    arguments = "\""+terragearDirectory;
    arguments += "/bin/terrafit\" ";

    if (minnode.size() > 0){
        arguments += "--minnodes "+minnode+" ";
    }
    if (maxnode.size() > 0){
        arguments += "--maxnodes "+maxnode+" ";
    }
    if (maxerror.size() > 0){
        arguments += "--maxerror "+maxerror+" ";
    }
//    NOT YET IMPLEMENTED IN TERRAFIT
//    if (ui->allThreadsCB->isChecked()) {
//        arguments += "--threads ";
//    }
    

    arguments +="\""+workDirectory+"/SRTM-"+elevationRes+"\"";

    GUILog( arguments + "\n", "terrafit" );
    GUILog( arguments + "\n", "default" );
    ui->textBrowser->append( arguments );
    sb->setValue(sb->maximum());

    QProcess proc;
    proc.setWorkingDirectory(terragearDirectory);
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(arguments, QIODevice::ReadWrite);

    int z = 0;

    while(proc.waitForReadyRead()){
        QCoreApplication::processEvents();

        QString info( proc.readAll() );
        GUILog( info, "terrafit" );
        if ( info.contains( ".arr.gz" ) ) {
            ui->convertElevationProgressBar->setValue( 50 + (( (double) 50 / numberOfArr ) * z ) ); // terrafit make progress btw 50% -> 100%
            z += 1;
        }
    }
    proc.QProcess::waitForFinished(-1);

    ui->convertElevationProgressBar->setValue( 100 );
    ui->convertElevationButton->setEnabled(1);

    GUILog( "ENDED in " + getElapTimeStg(pt.elapsed()) + " secondes\n", "default");
}

// select elevation directory
void MainWindow::on_elevationDirectoryButton_clicked()
{
    elevationDirectory = QFileDialog::getExistingDirectory(this,tr("Select the elevation directory, this is the directory in which the .hgt files live."),elevationDirectory);
    ui->elevationDirectoryField->setText(elevationDirectory);
    settings.setValue("paths/elevationDir", elevationDirectory); // keep the last directory used
}
