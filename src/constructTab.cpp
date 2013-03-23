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
    QByteArray data;

    int folderCnt = ui->terrainTypesList->count();
    if (folderCnt == 0) {
        QMessageBox::critical(  this,
                                "No terraintypes",
                                "There are no terraintypes listed. Use [Update list] to populate it, then select those desired."
                                );
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
        return;
    }
    // reset progress bar
    ui->generateSceneryProgressBar->setValue(10);

    QFile prioritiesFile(terragearDirectory + "/share/TerraGear/default_priorities.txt");
    if ( ! prioritiesFile.exists() ) {
        QString msg = "Unable to locate default_priorities at\n" + terragearDirectory + "/share/TerraGear/default_priorities.txt";
        QMessageBox::critical(this,"File not found", msg);
        return;
    }
    QFile usgmapFile(terragearDirectory + "/share/TerraGear/usgsmap.txt");
    if ( ! usgmapFile.exists() ) {
        QString msg = "Unable to locate usgsmap at\n" + terragearDirectory + "/share/TerraGear/usgsmap.txt";
        QMessageBox::critical(this,"File not found", msg);
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
    runtime += "--priorities=\""+terragearDirectory;
    runtime += "/share/TerraGear/default_priorities.txt\" ";
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
                              "There is no explicit landmass shapefile loaded. Landmass is generally NOT required to generate scenery. Please enable the 'ignore landmass' option and retry if you don't know what it does."
                              );
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

    outTemp(arguments+"\n"); // output commandline to log.txt
    outputToLog(arguments);

    QProcess proc;
    proc.setWorkingDirectory(terragearDirectory);
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(arguments, QIODevice::ReadWrite);

    // reset progress bar
    ui->generateSceneryProgressBar->setValue(0);

    while(proc.waitForReadyRead()){
        QCoreApplication::processEvents();
        data.append(proc.readAll());
        ui->textBrowser->append(data.data()); // Output the data
        sb->setValue(sb->maximum()); // scroll down
    }

    proc.waitForFinished(-1);

    info = proc.readAll();

    outTemp(info+"\n");
    outTemp(arguments+"\n");
    output += info+"\n"; // add to full output
    output += arguments+"\n";

    ui->textBrowser->append(info); // only the last
    sb->setValue(sb->maximum());

    if (info.contains("No area named")) {

        // find material name
        QStringList unknownArea1 = info.split("No area named ");
        QString unknownMaterial = unknownArea1[1];

        QMessageBox msgBox;
        msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Ignore);
        msgBox.setDefaultButton(QMessageBox::Abort);
        msgBox.setWindowTitle("Unknown area");
        msgBox.setText(QString("Material '%1' is not listed in default_priorities.txt.")
                        .arg(unknownMaterial));
        msgBox.setIcon(QMessageBox::Critical);
        int ret = msgBox.exec();
        switch (ret) {
        case QMessageBox::Ignore:
             break;
        case QMessageBox::Abort:
            return;
            break;
        }
    }

    // scenery has been successfully created, congratulate developer
    if ( info.contains("[Finished successfully]") ) {
        QMessageBox::information(this, "Finished successfully", "Congratulations, you've successfully built some scenery!");
    }

    ui->textBrowser->append(output); // add it ALL
    ui->generateSceneryButton->setEnabled(true);
    ui->generateSceneryProgressBar->setValue(100);
    sb->setValue(sb->maximum()); // get the info shown
}

// update terraintypes list for tg-construct
// *TBD* Should maybe EXCLUDE directory 'Shared'!
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
