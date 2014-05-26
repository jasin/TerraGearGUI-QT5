/* constructTab.cpp
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
//##################       RUN TG-CONSTRUCT       ################//
//################################################################//
//################################################################//

void MainWindow::on_generateSceneryButton_clicked()
{
    ui->generateSceneryButton->setEnabled(false);
    QScrollBar *sb = ui->textBrowser->verticalScrollBar();
    QString lat = ui->centerLatField->text();
    QString lon = ui->centerLonField->text();
    QString x = ui->radiusXField->text();
    QString y = ui->radiusYField->text();
    QString selectedMaterials;
    QString msg;
    bool unknownTotalTiles = true;
    double tileProgress = 0;
    double writeProgress = 0;
    double totalTiles = 0;
    double tileNumber = 0;
    QRegExp exp("([0-9]*) of ([0-9]*)");

    int folderCnt = ui->terrainTypesList->count();
    if (folderCnt == 0) {
        QMessageBox::critical(  this,
                                "No terraintypes",
                                "There are no terraintypes listed. Use [Update list] to populate it, then select those desired."
                                );
        ui->generateSceneryButton->setEnabled(true);
        return;
    }

    folderCnt = 0; // RESTART COUNTER
    // create string with selected terraintypes
    for (int i = 0; i < ui->terrainTypesList->count(); ++i){
        if (ui->terrainTypesList->item(i)->isSelected() == 1) {
            selectedMaterials += ui->terrainTypesList->item(i)->text()+" ";
            folderCnt++;
        }
    }
    if (folderCnt == 0) {
        QMessageBox::critical(this,
                              "No terraintype selected",
                              "It appears you have not selected any of the terraintypes! Select all types that you would like to include in the scenery."
                              );
        ui->generateSceneryButton->setEnabled(true);
        return;
    }
    if (altDefPropsFile != "") {
        QFile prioritiesFile(altDefPropsFile);
        if ( ! prioritiesFile.exists() ) {
            QString msg = "Unable to locate default_priorities at\n" + altDefPropsFile;
            QMessageBox::critical(this,"File not found", msg);
            ui->generateSceneryButton->setEnabled(true);
            return;
        }
    } else {
        QFile prioritiesFile(terragearDirectory + "/share/TerraGear/default_priorities.txt");
        if ( ! prioritiesFile.exists() ) {
            QString msg = "Unable to locate default_priorities at\n" + terragearDirectory + "/share/TerraGear/default_priorities.txt";
            QMessageBox::critical(this,"File not found", msg);
            ui->generateSceneryButton->setEnabled(true);
            return;
        }
    }
    QFile usgmapFile(terragearDirectory + "/share/TerraGear/usgsmap.txt");
    if ( ! usgmapFile.exists() ) {
        QString msg = "Unable to locate usgsmap at\n" + terragearDirectory + "/share/TerraGear/usgsmap.txt";
        QMessageBox::critical(this,"File not found", msg);
        ui->generateSceneryButton->setEnabled(true);
        return;
    }

    // construct tg-construct commandline,
    QString arguments;
    QString index;
    QString path;
    QTime rt;
    QTime pt;
    QString tm;
    QString em;
    QString info;

    // build the general runtime string
    QString runtime = "\""+terragearDirectory;
    runtime += "/bin/tg-construct\" ";

    if (altDefPropsFile != "") {
        runtime += "--priorities=\""+altDefPropsFile;
        runtime += "\" ";
    } else {
        runtime += "--priorities=\""+terragearDirectory;
        runtime += "/share/TerraGear/default_priorities.txt\" ";
    }

    runtime += "--usgs-map=\""+terragearDirectory;
    runtime += "/share/TerraGear/usgsmap.txt\" ";
    runtime += "--work-dir=\""+workDirectory+"\" ";
    runtime += "--output-dir=\""+outpDirectory+"/Terrain\" ";
    if (ui->useUkGridCB->isChecked()) {
        runtime += "--useUKgrid ";
    }
    if (ui->ignoreLandmassCB->isChecked()) {
        runtime += "--ignore-landmass ";
    } else if (!selectedMaterials.contains("Default ")) {
        // Check if this option is valid: "Default" material must be present
        QMessageBox::critical(this,
                              "No landmass found",
                              "Landmass is generally not required to generate scenery. Please enable the 'ignore landmass' option and retry if you don't know what it does.\n\nIf you do want to use an explicit landmass layer; map it to the 'Default' material."
                              );
        ui->generateSceneryButton->setEnabled(true);
        return;
    }


    if (ui->allThreadsCB->isChecked()) {
        runtime += "--threads ";
    }

    index = ui->constructTileIdField->text();
    arguments = runtime;

    if (index > 0) {
        arguments += "--tile-id="+index+" ";
    } else {
        double dlon = lon.toDouble();
        double dlat = lat.toDouble();
        double xdist = x.toDouble();
        double ydist = y.toDouble();

        double min_x = dlon - xdist;
        double min_y = dlat - ydist;
        double max_x = dlon + xdist;
        double max_y = dlat + ydist;

        arguments += "--min-lat="+ QString::number(min_y) +" --max-lat="+QString::number(max_y)+" --min-lon="+QString::number(min_x)+" --max-lon="+QString::number(max_x)+" ";
    }
    arguments += selectedMaterials;

    GUILog( arguments + "\n", "default" ); // output commandline to log.txt
    GUILog( arguments + "\n", "tg-construct" );
    ui->textBrowser->append( arguments );
    sb->setValue(sb->maximum());

    QProcess proc;
    proc.setWorkingDirectory(terragearDirectory);
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(arguments, QIODevice::ReadWrite);

    // reset progress bar
    ui->generateSceneryProgressBar->setValue(0);

    while(proc.waitForReadyRead()){
        QCoreApplication::processEvents();

        QString info( proc.readAll() );
        GUILog( info, "tg-construct" );

        //ui->textBrowser->append( info );  // If we want see console output in the 
        //sb->setValue(sb->maximum());      // textBrowser, we just need to uncomment these 2 lines

        if ( info.contains( exp ) ) {
            if ( unknownTotalTiles ) {
                totalTiles = exp.cap(2).toDouble();
                ui->generateSceneryProgressBar->setMaximum( totalTiles*3 ); // x3 because tg-construct use 3 passes
                unknownTotalTiles = false;
            }
            tileProgress += 1;
            ui->generateSceneryProgressBar->setValue(tileProgress);
        }
    }
    proc.waitForFinished(-1);

    // scenery has been successfully created, congratulate developer
    if ( info.contains("[Finished successfully]") ) {
        QMessageBox::information(this, "Finished successfully", "Congratulations, you've successfully built some scenery!");
    }

    ui->generateSceneryButton->setEnabled(true);
    ui->generateSceneryProgressBar->setMaximum( 100 );
    ui->generateSceneryProgressBar->setValue( 100 );
}

// select Alt default_properties.txt file
void MainWindow::on_altDefaultPropertiesButton_clicked()
{
    altDefPropsFile = QFileDialog::getOpenFileName(
		this,
		tr("Select Alternate Default_Priorities file, only if you want to use an alternate Default_Priorities.txt"),
		altDefPropsFile, tr("Def_Property files (*.txt)"));
    ui->altDefaultPropertiesField->setText(altDefPropsFile);
    settings.setValue("paths/altdefpropsfile", altDefPropsFile);
}

// update terraintypes list for tg-construct
void MainWindow::on_updateTerrainTypeButton_clicked()
{
    int j = 0;
    ui->terrainTypesList->clear();
    QDir dir(workDirectory);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);

        // do not add the Shared folder
        if (fileInfo.fileName() != "Shared"){
            QString test = qPrintable(QString("%1").arg(fileInfo.fileName()));
            new QListWidgetItem(tr(qPrintable(QString("%1").arg(fileInfo.fileName()))), ui->terrainTypesList);
            // select all materials per default
            ui->terrainTypesList->item(j)->setSelected(1);
            j++;
        }
    }
}

// calculate center of scenery area and radii
void MainWindow::updateCenter()
{
    //== Grab the values from the widget and froce in double int long.. umm the bounds
    //TODO = we need a bounds Object
    //== Initialise the Lat Lon params in a square box
    double eastInt     = m_east.toDouble();
    double northInt    = m_north.toDouble();
    double southInt    = m_south.toDouble();
    double westInt     = m_west.toDouble();

    // save values for next session
    settings.setValue("boundaries/north", m_north);
    settings.setValue("boundaries/south", m_south);
    settings.setValue("boundaries/east", m_east);
    settings.setValue("boundaries/west", m_west);

    if ((westInt < eastInt) && (northInt > southInt)) {
        // Calculate center
        double latInt = (northInt + southInt) / 2;
        double lonInt = (eastInt + westInt) / 2;

        // Display center and radius
        ui->centerLatField->setText( QString::number(latInt) );
        ui->centerLonField->setText( QString::number(lonInt) );
        ui->radiusXField->setText( QString::number(eastInt - lonInt) );
        ui->radiusYField->setText( QString::number(northInt - latInt) ) ;
    }
}

void MainWindow::on_ignoreLandmassCB_toggled(bool checked)
{
    settings.setValue("check/ignore_landmass", checked);
}

// disable lat/lon boundaries when airport ID is entered
void MainWindow::on_airportIcaoField_textEdited(const QString &arg1)
{
    int empty = 0;
    if (ui->airportIcaoField->text() != "") {
        empty = 1;
    }
}

// disable lat/lon boundaries when tile-id is entered
void MainWindow::on_constructTileIdField_textEdited(const QString &arg1)
{
    int empty = 0;
    if (ui->constructTileIdField->text() != "") {
        empty = 1;
    }
    ui->centerRadiusText->setDisabled(empty);
    ui->centerLatLabel->setDisabled(empty);
    ui->centerLonLabel->setDisabled(empty);
    ui->radiusXLabel->setDisabled(empty);
    ui->radiusYLabel->setDisabled(empty);
    ui->centerLatField->setDisabled(empty);
    ui->centerLonField->setDisabled(empty);
    ui->radiusYField->setDisabled(empty);
    ui->radiusXField->setDisabled(empty);
}
