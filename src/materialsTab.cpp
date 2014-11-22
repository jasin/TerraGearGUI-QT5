/* materialsTab.cpp
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
//###################       RUN OGR-DECODE      ##################//
//################################################################//
//################################################################//

void MainWindow::on_decodeShapefilesButton_clicked()
{

    QScrollBar *sb = ui->textBrowser->verticalScrollBar();
    QStringList argList;
    QStringList shpList;
    QString arguments;
    QString shapefile;
    QByteArray data;
    QString info;
    QString msg;
    QTime pt;
    int i;


    // reset progress bar
    ui->decodeShapefilesProgressBar->setValue(0);

    // check if terragear tool exists
    QString TGfile = terragearDirectory;
    TGfile += "/bin/ogr-decode";
#ifdef Q_OS_WIN
    TGfile += ".exe"; // add EXE for windows
#endif
    QFile f(TGfile);
    if ( ! f.exists() ) {
        QMessageBox::critical(this,"File not found", "Unable to locate executable at "+TGfile+". Make sure you selected the right TerraGear root directory on the start page.");
        return;
    }

    // skip if no shapefiles are listed
    if (ui->shapefilesTable->rowCount() == 0){
        QMessageBox::critical(this,
                              "Shapefile error",
                              "No shapefiles are listed. Please click the [Retrieve shapefiles] button."
                              );
        return;
    }

    for (i = 0; i < ui->shapefilesTable->rowCount(); i++) {
        // skip if material are not assigned
        if ((ui->shapefilesTable->item(i, 1) == 0) ||
                (ui->shapefilesTable->item(i, 1)->text().length() == 0)){
            QMessageBox::critical(this,"Material error", "You did not assign materials for each shapefile.");
            return;
        }
    }

    // got executable, and have assigned materials, so do the work
    for (i = 0; i < ui->shapefilesTable->rowCount(); i++) {
        QString material = ui->shapefilesTable->item(i, 0)->text();
        QString lineWidth;
        if (ui->shapefilesTable->item(i, 2) != 0) {
            // cell item already created
            lineWidth = ui->shapefilesTable->item(i, 2)->text();
        }
        else {
            // cell item are not created - default width
            lineWidth = "10";
        }

        shapefile = ui->shapefilesTable->item(i, 1)->text();

        arguments   = "\""+terragearDirectory;
        arguments += "/bin/ogr-decode\" ";

        arguments += "--line-width "+lineWidth+" ";

        if (ui->shapefilesTable->item(i, 3) != 0){
            arguments += "--where \""+ui->shapefilesTable->item(i, 3)->text()+"\" ";
        }
        if (ui->pointWidthField->text() > 0){
            arguments += "--point-width "+ui->pointWidthField->text()+" ";
        }
        if (ui->ignoreErrorsCB->isChecked() == 1){
            arguments += "--continue-on-errors ";
        }
        if (ui->debugCB->isChecked() == 1){
            arguments += "--debug ";
        }
        if (ui->maxSegLengthField->text() > 0){
            arguments += "--max-segment "+ui->maxSegLengthField->text()+" ";
        }
        if (ui->texturedLinesCB->isChecked() == 1){
            arguments += "--texture-lines ";
        }
        if (ui->allThreadsCB->isChecked()) {
            arguments += "--all-threads ";
        }
        arguments += "--area-type "+shapefile+" ";
        arguments += "\""+workDirectory+"/"+shapefile+"\" ";
        arguments += "\""+dataDirectory+"/"+material+"\"";

        argList += arguments;
        shpList += shapefile;
    }

    // got all the arguments prepared,
    // now process then one by one...
    for (i = 0; i < argList.size(); i++) {

        pt.start();
        arguments = argList[i];
        shapefile = shpList[i];

        // save commandline to log
        GUILog( arguments + "\n", "ogr-decode" );
        GUILog( arguments + "\n", "default" );
        ui->textBrowser->append( arguments );
        sb->setValue(sb->maximum());

        //= Create shell process and start
        QProcess proc;
        proc.setProcessChannelMode(QProcess::MergedChannels);
        proc.start(arguments, QIODevice::ReadWrite);

        //= run command in shell ? ummm>?
        while(proc.waitForReadyRead()){
            QCoreApplication::processEvents();

            QString info( proc.readAll() );
            GUILog( info, "ogr-decode" );
        }
        proc.QProcess::waitForFinished(-1);

        GUILog( "ENDED in " + getElapTimeStg(pt.elapsed()) + " secondes \n", "default");

        // adjust progress bar
        ui->decodeShapefilesProgressBar->setMaximum( argList.size() - 1 );
        ui->decodeShapefilesProgressBar->setValue(i);
    }
}

// add material
//TODO there's a better on itemchanged event..
void MainWindow::on_addMaterialButton_clicked()
{
    if (ui->shapefilesTable->item(ui->shapefilesTable->currentRow(), 1) == 0) {
        QTableWidgetItem *twiMaterialCell = new QTableWidgetItem(0);
        twiMaterialCell->setText(ui->materialTypeSelect->itemText(ui->materialTypeSelect->currentIndex()));
        ui->shapefilesTable->setItem(ui->shapefilesTable->currentRow(), 1, twiMaterialCell);

    }else
    {
        ui->shapefilesTable->item(   ui->shapefilesTable->currentRow(), 1)->setText(
                    ui->materialTypeSelect->itemText(ui->materialTypeSelect->currentIndex()
                                             ));
    }
}


// populate material list with materials from FG's materials.xml
// broken since the split of materials.xml with v3.2
void MainWindow::updateMaterials()
{
    QFile materialfile(flightgearDirectory+"/Materials/default/materials.xml");
    if (materialfile.exists() == false) {
        // For FlightGear version before 2.8.0
        materialfile.setFileName(flightgearDirectory+"/materials.xml");
    }
    if (materialfile.exists() == true) {

        if (materialfile.open(QIODevice::ReadOnly | QIODevice::Text)) {

            QStringList materialList;
            QStringList textureList;
            QString material;
            QString texture;
            QStringList materials;
            bool textureFound = false;
            int numTexture = 0;

            QTextStream in(&materialfile);
            while (!in.atEnd()) {
                QString line = in.readLine();
                QRegExp rx("*<name>*</name>");
                rx.setPatternSyntax(QRegExp::Wildcard);
                if (rx.exactMatch(line)) {
                    material = line;
                    QRegExp r1("*<name>");
                    r1.setPatternSyntax(QRegExp::Wildcard);
                    texture.remove(r1);
                    material.remove(r1);
                    material.remove(QRegExp("</name>"));
                    if (!material.startsWith("Bidirectional") &&
                            !material.startsWith("BlackSign") &&
                            !material.startsWith("dirt_rwy") &&
                            !material.startsWith("grass_rwy") &&
                            !material.startsWith("FramedSign") &&
                            !material.startsWith("lakebed_taxiway") &&
                            !material.startsWith("lf_") &&
                            !material.startsWith("pa_") &&
                            !material.startsWith("pc_") &&
                            !material.startsWith("RedSign") &&
                            !material.startsWith("RunwaySign") &&
                            !material.startsWith("RUNWAY_") &&
                            !material.startsWith("RWY_") &&
                            !material.startsWith("SpecialSign") &&
                            !material.startsWith("Unidirectional") &&
                            !material.startsWith("YellowSign") &&
                            materialList.indexOf(material, 0) == -1) {
                        materialList.append(material);
                        numTexture++;
                    }
                }
                if (!textureFound) {
                    QRegExp rx("*<texture>*</texture>");
                    rx.setPatternSyntax(QRegExp::Wildcard);
                    if (rx.exactMatch(line)) {
                        texture = line;
                        textureFound = true;
                    }
                }
                QRegExp rx2("*</material>");
                rx2.setPatternSyntax(QRegExp::Wildcard);
                if (rx2.exactMatch(line)) {
                    for (int j = 0; j < numTexture; j++) {
                        QRegExp r2("*<texture>");
                        r2.setPatternSyntax(QRegExp::Wildcard);
                        texture.remove(r2);
                        texture.remove(QRegExp("</texture>"));
                        textureList.append(texture);
                    }
                    numTexture = 0;
                    textureFound = false;
                }
            }
            materialfile.close();

            // Fall back to default if we cannot read anything from materials.xml
            if ( materialList.size() == 0)
                return;

            QMap<QString, QString> map;
            for (int i = 0; i < materialList.size(); i++) {
                map[materialList[i]] = textureList[i];
            }
            materialList.sort();
            ui->materialTypeSelect->clear();
            for (int i = 0; i < materialList.size(); i++) {
                ui->materialTypeSelect->addItem(QIcon(flightgearDirectory+"/Textures/"+map.value(materialList[i])),materialList[i]);
            }
        }
    }
}


// update shapefiles list for ogr-decode
void MainWindow::on_retrieveShapefilesButton_clicked()
{
    // confirmation dialog
    if (ui->shapefilesTable->rowCount() != 0) {
        QMessageBox msgBox;
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setText("Are you sure you want to retrieve shapefiles?\nDoing so will reset all material and line width settings.");
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();
        if(ret == QMessageBox::Cancel){
            return;
        }
    }

    // move shapefiles to "private" directories
    QDir dir(dataDirectory); // search 'data' folder, for 'directories'
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fInfo = list.at(i);
        QString fPath = fInfo.absolutePath();
        QString fFilePath = fInfo.absoluteFilePath();
        QString fFileName1 = fInfo.fileName();
        QString fFileName2 = fInfo.fileName();

        // move only shapefiles
        //TODO have shapefile detector ..GBH
        if (fInfo.suffix() == "dbf" ||
                fInfo.suffix() == "prj" ||
                fInfo.suffix() == "shp" ||
                fInfo.suffix() == "shx"
                ){
            fFileName1.chop(4);     // remove fileformat from name
            QFile file (fFilePath);
            QString fPath_ren = fPath+"/"+fFileName1+"/"+fFileName2;
            dir.mkpath(fPath+"/"+fFileName1);
            dir.rename(fFilePath, fPath_ren);
        }
    }

    // update list
    while (ui->shapefilesTable->rowCount() != 0)
    {
        ui->shapefilesTable->removeRow(0);
    }

    // list of scenery shapefiles
    // TODO Can we create this as a new class eg QFGMaterials
    // ?? are these mapped as key value == hash set ?? thinks json
    QStringList csShape;
    csShape << "*_agroforest"
            << "*_airport"
            << "*_asphalt"
            << "*_barrencover"
            << "*_bog"
            << "*_burnt"
            << "*_canal"
            << "*_cemetery"
            << "*_complexcrop"
            << "*_construction"
            << "*_cropgrass"
            << "*_deciduousforest"
            << "*_default"
            << "*_dirt"
            << "*_drycrop"
            << "*_dump"
            << "*_estuary"
            << "*_evergreenforest"
            << "*_floodland"
            << "*_freeway"
            << "*_glacier"
            << "*_golfcourse"
            << "*_grassland"
            << "*_greenspace"
            << "*_heath"
            << "*_herbtundra"
            << "*_industrial"
            << "*_intermittentlake"
            << "*_intermittentstream"
            << "*_irrcrop"
            << "*_lagoon"
            << "*_lake"
            << "*_landmass"
            << "*_lava"
            << "*_light_rail"
            << "*_littoral"
            << "*_marsh"
            << "*_mixedcrop"
            << "*_mixedforest"
            << "*_motorway"
            << "*_naturalcrop"
            << "*_ocean"
            << "*_olives"
            << "*_openmining"
            << "*_orchard"
            << "*_packice"
            << "*_polarice"
            << "*_port"
            << "*_primary"
            << "*_rail"
            << "*_railroad1"
            << "*_railroad2"
            << "*_rainforest"
            << "*_residential"
            << "*_rice"
            << "*_river"
            << "*_road"
            << "*_rock"
            << "*_saline"
            << "*_saltmarsh"
            << "*_sand"
            << "*_sclerophyllous"
            << "*_scrub"
            << "*_secondary"
            << "*_service"
            << "*_stream"
            << "*_suburban"
            << "*_tertiary"
            << "*_town"
            << "*_transport"
            << "*_trunk"
            << "*_urban"
            << "*_vineyard"
            << "*_watercourse";

    // list of correpsonding materials
    QStringList csMater;
    csMater << "AgroForest"
            << "Airport"
            << "Asphalt"
            << "BarrenCover"
            << "Bog"
            << "Burnt"
            << "Canal"
            << "Cemetery"
            << "ComplexCrop"
            << "Construction"
            << "CropGrass"
            << "DeciduousForest"
            << "Default"
            << "Dirt"
            << "DryCrop"
            << "Dump"
            << "Estuary"
            << "EvergreenForest"
            << "FloodLand"
            << "Freeway"
            << "Glacier"
            << "GolfCourse"
            << "Grassland"
            << "Greenspace"
            << "Heath"
            << "HerbTundra"
            << "Industrial"
            << "IntermittentLake"
            << "IntermittentStream"
            << "IrrCrop"
            << "Lagoon"
            << "Lake"
            << "Default"
            << "Lava"
            << "Railroad"
            << "Littoral"
            << "Marsh"
            << "MixedCrop"
            << "MixedForest"
            << "Freeway"
            << "NaturalCrop"
            << "Ocean"
            << "Olives"
            << "OpenMining"
            << "Orchard"
            << "PackIce"
            << "PolarIce"
            << "Port"
            << "Road"
            << "Railroad"
            << "Railroad"
            << "Railroad"
            << "RainForest"
            << "Road"
            << "Rice"
            << "Canal"
            << "Road"
            << "Rock"
            << "Saline"
            << "SaltMarsh"
            << "Sand"
            << "Sclerophyllous"
            << "ScrubCover"
            << "Road"
            << "Road"
            << "Stream"
            << "SubUrban"
            << "Road"
            << "Town"
            << "Transport"
            << "Road"
            << "Urban"
            << "Vineyard"
            << "Watercourse";

    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    //= loop around the dir entries..
    QFileInfoList dirList = dir.entryInfoList();
    for (int i = 0; i < dirList.size(); ++i) {
        QFileInfo dirInfo = dirList.at(i);
        if (!dirInfo.fileName().contains("SRTM")) {
            ui->shapefilesTable->insertRow(ui->shapefilesTable->rowCount());
            QTableWidgetItem *twiCellShape = new QTableWidgetItem(0);
            twiCellShape->setText(tr(qPrintable(QString("%1").arg(dirInfo.fileName()))));
            twiCellShape->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
            ui->shapefilesTable->setItem(ui->shapefilesTable->rowCount()-1, 0, twiCellShape);
        }

        QTableWidgetItem *twiCellMater = new QTableWidgetItem(0);

        // suggest a material
        QString suggestedMaterial;
        for (int j = 0; j < csShape.length(); ++j) {
            QRegExp rx(csShape[j]);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if (rx.exactMatch(dirInfo.fileName())){
                suggestedMaterial = csMater[j];
            }
        }
        twiCellMater->setText(suggestedMaterial);
        ui->shapefilesTable->setItem(i, 1, twiCellMater);
    }
    ui->shapefilesTable->resizeRowsToContents();
}
