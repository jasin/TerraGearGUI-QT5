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

    QDirIterator dir( elevationDirectory );
    while( dir.hasNext() ) {
        dir.next();
        if( !dir.fileInfo().isDir() ) {
            QString filename = dir.fileName();
            if( filename.endsWith(".hgt") || filename.endsWith(".hgt.zip") )
                argList += "\""+terragearDirectory+"/bin/hgtchop\" "+elevationRes+" \""+elevationDirectory+"/"+filename+"\" \""+workDirectory+"/SRTM-"+elevationRes+"\"";
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

        ui->convertElevationProgressBar->setValue( (50/argList.size())*i );  // hgtchop make progress btw 0% -> 50%

        // run hgtchop command
        while(proc.waitForReadyRead()){
            QCoreApplication::processEvents();

            QString info( proc.readAll() );
            GUILog( info, "hgtchop" );
        }
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

    GUILog( "ENDED in " + getElapTimeStg(pt.elapsed()) + " secondes\n", "default");
}

// update elevation download range
void MainWindow::updateElevationRange()
{
    QString east;
    QString north;
    QString south;
    QString west;

    double eastDbl     = ui->maxLonField->text().toDouble();
    double northDbl    = ui->maxLatField->text().toDouble();
    double southDbl    = ui->minLatField->text().toDouble();
    double westDbl     = ui->minLonField->text().toDouble();

    // initialize text colors
    QPalette q1;
    QPalette q2;
    q1.setColor(QPalette::Text, Qt::black);
    q2.setColor(QPalette::Text, Qt::black);

    QString prevmin = minElev;
    QString prevmax = maxElev;

    // CAN we invalidate boundaries here... first..

    // sorry mate, your outa bounds..

    // check if boundaries are valid
    if (westDbl < eastDbl and northDbl > southDbl){

        // clear the strings
        minElev = "";
        maxElev = "";
        elevList = ""; // restart SRTM elevation

        // use absolute degrees for elevation ranges
        east.sprintf("%03d", abs(eastDbl));
        north.sprintf("%02d", abs(northDbl));
        south.sprintf("%02d", abs(southDbl));
        west.sprintf("%03d", abs(westDbl));

        // max north
        if (northDbl >= 0){
            maxElev += "N";
        }
        if (northDbl < 0) {
            maxElev += "S";
        }
        maxElev += north;

        // max south
        if (southDbl >= 0){
            minElev += "N";
        }
        if (southDbl < 0) {
            minElev += "S";
        }
        minElev += south;

        // max east
        if (eastDbl >= 0){
            maxElev += "E";
        }
        if (eastDbl < 0) {
            maxElev += "W";
        }
        maxElev += east;

        //max west
        if (westDbl >= 0){
            minElev += "E";
        }
        if (westDbl < 0) {
            minElev += "W";
        }
        minElev += west;

        // output elevation range chosen in left
        ui->minDownloadRangeLabel->setText(minElev);
        ui->maxDownloadRangeLabel->setText(maxElev);

        if ((prevmin != minElev) || (prevmax != maxElev)) {
            // build a HELPFUL SRTM list to add to the LOG, if enabled (or not)
            for (double ew = westDbl; ew <= eastDbl; ew += 1.0) {
                for (double ns = southDbl; ns <= northDbl; ns += 1.0) {
                    east.sprintf("%03d", abs(ew));
                    north.sprintf("%02d", abs(ns));
                    if (elevList.size()) elevList += ";"; // add separator
                    if (ns < 0) {
                        elevList += "S";
                    } else {
                        elevList += "N";
                    }
                    elevList += north;
                    if (ew < 0) {
                        elevList += "W";
                    } else {
                        elevList += "E";
                    }
                    elevList += east;
                }
            }
            // GUILog(elevList); /* provide a 'helpful' list of SRTM files */
        }

        // enable download buttons
        ui->downloadShapefilesButton->setEnabled(1);
        ui->downloadElevationButton->setEnabled(1);
    }

    // if boundaries are not valid: do not display elevation range and set text color to red
    else{
        ui->minDownloadRangeLabel->setText("");
        ui->maxDownloadRangeLabel->setText("");

        if (westDbl == eastDbl or westDbl > eastDbl){
            q1.setColor(QPalette::Text, Qt::red);
        }
        if (northDbl == southDbl or southDbl > northDbl){
            q2.setColor(QPalette::Text, Qt::red);
        }

        // disable download buttons
        ui->downloadShapefilesButton->setDisabled(1);
        ui->downloadElevationButton->setDisabled(1);
    }

    // change text color in the boundary fields
    // TODO - change into a widget set said pedro..
    ui->maxLonField->setPalette(q1);
    ui->minLonField->setPalette(q1);
    ui->maxLatField->setPalette(q2);
    ui->minLatField->setPalette(q2);
}

// select elevation directory
void MainWindow::on_elevationDirectoryButton_clicked()
{
    elevationDirectory = QFileDialog::getExistingDirectory(this,tr("Select the elevation directory, this is the directory in which the .hgt files live."),elevationDirectory);
    ui->elevationDirectoryField->setText(elevationDirectory);
    settings.setValue("paths/elevationDir", elevationDirectory); // keep the last directory used
}
