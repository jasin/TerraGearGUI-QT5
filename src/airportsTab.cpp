/* airportsTab.cpp
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
//######################      RUN GENAPTS      ###################//
//################################################################//
//################################################################//

// run genapts
// missing adding an elevation source directory - ie --terrain=<path> - This would be in addition to the
// 10 builtin elevation sources searched - SRTM-1, SRTM-3, SRTM-30, DEM-USGS-3, etc
// And although a 'Chunk' edit box has been added, it is presently disabled/not used, as just
// another way to set mina/max ranges
void MainWindow::on_generateAirportsButton_clicked()
{
    QString arguments;
    // construct genapts commandline
    QString airportId   = ui->airportIcaoField->text();
    QString startAptId  = ui->startFromField->text();
    QString tileId      = ui->tileIdField->text();
    QScrollBar *sb      = ui->textBrowser->verticalScrollBar();

    QString minLat  = m_south;
    QString maxLat  = m_north;
    QString minLon  = m_west;
    QString maxLon  = m_east;
    QString maxSlope  = ui->maxRwySlopeField->text();
    QTime rt;
    QString msg;

    // Check if executable can be located
    QString genapts = terragearDirectory+"/bin/genapts850";
#ifdef Q_OS_WIN
    genapts += ".exe"; // add EXE for windows
#endif
    QFile f(genapts);
    if ( !f.exists() ) {
        QMessageBox::critical(this,"File not found", "Unable to locate executable at \n"+genapts);
        return;
    }

    QFile aptFile(airportFile);
    if ( !aptFile.exists() ) {
        QMessageBox::critical(this,"File not found", "Unable to locate airports at \n"+airportFile);
        return;
    }

    //+++ Lets Go!
    rt.start();
    // proceed to do airport generation
    arguments   =  "\"" + genapts;

    if ( !util_verifySRTMfiles( minLat, maxLat,
                                minLon, maxLon,
                                workDirectory)
         ) {
        if ( ! getYesNo("No elevation data found","No elevation data was found in "+workDirectory+"\n\nThis means airports will be generated with no elevation information!") )
            return;
    }

    arguments += "\" --input=\""+airportFile+"\" --work=\""+workDirectory+"\" ";

    // all airports within area
    if (ui->allAirportInAreaRadio->isChecked()) {
        // not excluded, so add where there is a min/max range
        if (maxLat.size() > 0) {
            arguments += "--max-lat="+maxLat+" ";
        }
        if (maxLon.size() > 0) {
            arguments += "--max-lon="+maxLon+" ";
        }
        if (minLat.size() > 0) {
            arguments += "--min-lat="+minLat+" ";
        }
        if (minLon.size() > 0) {
            arguments += "--min-lon="+minLon+" ";
        }
    }
    // single airport
    if (airportId.size() > 0 && ui->singleAirportRadio->isChecked()){
        arguments += "--airport="+airportId+" ";
    }
    // all airports on a single tile
    if (tileId.size() > 0 && ui->allAirportsInTileRadio->isChecked()){
        arguments += "--tile="+tileId+" ";
    }
    // all airports in file (optionally starting with...)
    if (startAptId.size() > 0 && ui->allAirportsInFileRadio->isChecked()) {
        arguments += "--start-id="+startAptId+" ";
    }

    if (maxSlope.size() > 0){
        arguments += "--max-slope="+maxSlope+" ";
    }

    if (ui->allThreadsCB->isChecked()) {
        arguments += "--threads ";
    }

    // save output to log
    GUILog( arguments + "\n", "genapt" );
    GUILog( arguments + "\n", "default" );
    ui->textBrowser->append( arguments );
    sb->setValue(sb->maximum());

    QByteArray data;
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(arguments, QIODevice::ReadWrite);     // run genapts command

    // reset progress bar
    ui->generateAirportsProgressBar->setValue(0);

    while(proc.waitForReadyRead()){
        QCoreApplication::processEvents();

        QString info( proc.readAll() );
        GUILog( info, "genapt" );

        if (info.contains("Finished building Linear Features")) {
            ui->generateAirportsProgressBar->setValue(50);
        }
        if (info.contains("Finished building runways")) {
            ui->generateAirportsProgressBar->setValue(60);
        }
        if (info.contains("Finished collecting nodes")) {
            ui->generateAirportsProgressBar->setValue(70);
        }
        if (info.contains("Finished adding intermediate nodes")) {
            ui->generateAirportsProgressBar->setValue(80);
        }
        if (info.contains("Finished cleaning polys")) {
            ui->generateAirportsProgressBar->setValue(90);
        }
    }
    proc.QProcess::waitForFinished(-1);

    ui->generateAirportsProgressBar->setValue(100);
    GUILog( "ENDED in " + getElapTimeStg(rt.elapsed()) + " secondes\n", "default");

}


void MainWindow::on_aptFileButton_clicked()
{
    airportFile = QFileDialog::getOpenFileName(this,tr("Open airport file"), airportFile, tr("Airport files (*.dat *.dat.gz)"));
    ui->aptFileField->setText(airportFile);
    settings.setValue("paths/airportFile", airportFile); // keep the last airport file used
}


void MainWindow::updateAirportRadios()
{
    if ( ui->allAirportsInFileRadio->isChecked() ) {
        ui->frameSingleAirport->hide();
        ui->frameSingleTile->hide();
        ui->frameAllAirports->show();
    }
    if ( ui->singleAirportRadio->isChecked() ) {
        ui->frameSingleAirport->show();
        ui->frameSingleTile->hide();
        ui->frameAllAirports->hide();
    }
    if ( ui->allAirportInAreaRadio->isChecked() ) {
        ui->frameSingleAirport->hide();
        ui->frameSingleTile->hide();
        ui->frameAllAirports->hide();
    }
    if ( ui->allAirportsInTileRadio->isChecked() ) {
        ui->frameSingleAirport->hide();
        ui->frameSingleTile->show();
        ui->frameAllAirports->hide();
    }
}

// disable lat/lon boundaries when tile-id is entered
void MainWindow::on_tileIdField_textEdited(const QString &arg1)
{
    int empty = 0;
    if (ui->tileIdField->text() != "") {
        empty = 1;
    }
}

// all airports in .dat
void MainWindow::on_allAirportsInFileRadio_clicked()
{
    updateAirportRadios();
}

// single airport
void MainWindow::on_singleAirportRadio_clicked()
{
    updateAirportRadios();
}

// all airports in area
void MainWindow::on_allAirportInAreaRadio_clicked()
{
    updateAirportRadios();
}

// all airports on single tile
void MainWindow::on_allAirportsInTileRadio_clicked()
{
    updateAirportRadios();
}

