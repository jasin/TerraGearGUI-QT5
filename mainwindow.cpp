/* mainwindow.cpp -- top level construction routines
//
// Written by Gijs de Rooy, started March 2010.
//
// Copyright (C) 2010-2012  Gijs de Rooy
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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QtGlobal>
#include <QIcon>
#include <QIODevice>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPalette>
#include <QProcess>
#include <QSettings>
#include <QStringList>
#include <QTextStream>
#include <QUrl>
#include <QXmlStreamReader>
#include <QTime> // found it - a ms timer
// #include <QWebView> // maybe try this in place of QDesktopServices::openUrl(qu)!?!?

#include "newbucket.h"
#include "tggui_utils.h"

QString airportFile;
QString elevationDirectory;
QString selectedMaterials;
QString output = "";

QString fgRoot;
QString terragearDirectory;
QString projDirectory;

QString dataDirectory;
QString outpDirectory;
QString workDirectory;

QString minElev;
QString maxElev;
QString elevList;

QString m_north;
QString m_south;
QString m_west;
QString m_east;

bool m_break;

// save variables for future sessions
QSettings settings("TerraGear", "TerraGearGUI");

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle( tr("TerraGear GUI") );

    elevList = "";
    // restore variables from previous session
    terragearDirectory  = settings.value("paths/terragear").toString();
    projDirectory       = settings.value("paths/project").toString();
    fgRoot              = settings.value("paths/fg-root").toString();
    elevationDirectory  = settings.value("path/elevationDir").toString();
    airportFile         = settings.value("path/airportFile").toString();

    ui->lineEdit_2->setText(terragearDirectory);
    ui->lineEdit_4->setText(projDirectory);
    ui->lineEdit_22->setText(fgRoot);
    ui->lineEdit_11->setText(elevationDirectory);

    m_north = settings.value("boundaries/north").toString();
    m_south = settings.value("boundaries/south").toString();
    m_west  = settings.value("boundaries/west").toString();
    m_east  = settings.value("boundaries/east").toString();

    // TAB: Start
    ui->lineEdit_8->setText(m_south);
    ui->lineEdit_7->setText(m_north);
    ui->lineEdit_6->setText(m_west);
    ui->lineEdit_5->setText(m_east);

    // TAB: FGFS Construct
    updateCenter(); // set the center/distance

    // TAB: GenApts
    ui->label_65->setText(m_south);
    ui->label_63->setText(m_north);
    ui->label_59->setText(m_west);
    ui->label_61->setText(m_east);

    ui->tblShapesAlign->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblShapesAlign->setHorizontalHeaderLabels(QStringList() << tr("Shapefile") << tr("Material"));
    ui->tblShapesAlign->horizontalHeader()->setResizeMode( QHeaderView::Stretch);
    ui->tblShapesAlign->horizontalHeader()->setStyleSheet("font: bold;");
    ui->tblShapesAlign->verticalHeader()->hide();

    // create sub-directory variables
    dataDirectory = projDirectory+"/data";
    outpDirectory = projDirectory+"/output";
    workDirectory = projDirectory+"/work";

    // run functions on startup
    if (fgRoot != 0) {
        updateMaterials();
        if (airportFile.size() == 0) {
            // try to find apt.dat.gz file
            QString apfile = fgRoot+"/Airports/apt.dat.gz";
            QFile apf(apfile);
            if (apf.exists()) {
               airportFile = apfile;
                settings.setValue("path/airportFile", airportFile); // keep the airport file found
            }
        }
        if (airportFile.size())
            ui->lineEdit_20->setText(airportFile);
    }
    updateElevationRange();
    updateCenter();
	
	// re-apply the check boxes (for construct)
    bool no_over = settings.value("check/no_overwrite").toBool();
    ui->checkBox_noovr->setCheckState(no_over ? Qt::Checked : Qt::Unchecked);
    bool ign_lm = settings.value("check/ignore_landmass").toBool();
    ui->checkBox_4->setCheckState(ign_lm ? Qt::Checked : Qt::Unchecked);
    bool no_data = settings.value("check/no_data").toBool();
    ui->checkBox_nodata->setCheckState(no_data ? Qt::Checked : Qt::Unchecked);
    bool ign_err = settings.value("check/ign_errors").toBool();
    ui->checkBox_igerr->setCheckState(ign_err ? Qt::Checked : Qt::Unchecked);

	
    m_break = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    // add shapefiles to list, if empty
    if (index == 3 and ui->tblShapesAlign->rowCount() == 0) {
        ui->pushButton_12->click();
    }
    // add terraintypes to list, if empty
    if (index == 4 and ui->listWidget_2->count() == 0) {
        ui->pushButton_15->click();
    }
}

void MainWindow::on_tabWidget_selected(QString )
{
    show();
}

// menu //

// close window
void MainWindow::on_actionQuit_triggered()
{
    MainWindow::close();
}

// show about dialog
void MainWindow::on_about_triggered()
{
    QMessageBox::about(this, tr("TerraGUI v0.7"),tr("©2010-2012 Gijs de Rooy for FlightGear\nGNU General Public License version 2"));
}

// show wiki article in a browser
void MainWindow::on_wiki_triggered()
{
    QString url = "http://wiki.flightgear.org/TerraGear_GUI";
    QUrl qu(url);
    if ( ! QDesktopServices::openUrl(qu) ) {
        QMessageBox::critical(this,"URL cannot be opened","The following URL cannot be opened "+url+".\nCopy the URL to your browser");
    }
}

// actions //

void MainWindow::on_lineEdit_5_editingFinished()
{
    m_east = ui->lineEdit_5->text();
    ui->label_61->setText(m_east);
    updateElevationRange();
    updateCenter();
    settings.setValue("boundaries/east", m_east);
}

void MainWindow::on_lineEdit_6_editingFinished()
{
    m_west = ui->lineEdit_6->text();
    ui->label_59->setText(m_west);
    updateElevationRange();
    updateCenter();
    settings.setValue("boundaries/west", m_west);
}

void MainWindow::on_lineEdit_7_editingFinished()
{
    m_north = ui->lineEdit_7->text();
    ui->label_65->setText(m_north);
    updateElevationRange();
    updateCenter();
    settings.setValue("boundaries/north", m_north);
}

void MainWindow::on_lineEdit_8_editingFinished()
{
    m_south = ui->lineEdit_8->text();
    ui->label_63->setText(m_south);
    updateElevationRange();
    updateCenter();
    settings.setValue("boundaries/south", m_south);
}

// disable lat/lon boundaries when tile-id is entered
void MainWindow::on_lineEdit_13_textEdited(const QString &arg1)
{
    int empty = 0;
    if (ui->lineEdit_13->text() != "") {
        empty = 1;
    }
    ui->label_3->setDisabled(empty);
    ui->label_59->setDisabled(empty);
    ui->label_60->setDisabled(empty);
    ui->label_61->setDisabled(empty);
    ui->label_62->setDisabled(empty);
    ui->label_63->setDisabled(empty);
    ui->label_64->setDisabled(empty);
    ui->label_65->setDisabled(empty);
    ui->label_66->setDisabled(empty);
    ui->lineEdit_18->setDisabled(empty);
}

// disable lat/lon boundaries when airport ID is entered
void MainWindow::on_lineEdit_18_textEdited(const QString &arg1)
{
    int empty = 0;
    if (ui->lineEdit_18->text() != "") {
        empty = 1;
    }
    ui->label_20->setDisabled(empty);
    ui->label_59->setDisabled(empty);
    ui->label_60->setDisabled(empty);
    ui->label_61->setDisabled(empty);
    ui->label_62->setDisabled(empty);
    ui->label_63->setDisabled(empty);
    ui->label_64->setDisabled(empty);
    ui->label_65->setDisabled(empty);
    ui->label_66->setDisabled(empty);
    ui->lineEdit_13->setDisabled(empty);
}

// disable lat/lon boundaries when tile-id is entered
void MainWindow::on_lineEdit_35_textEdited(const QString &arg1)
{
    int empty = 0;
    if (ui->lineEdit_35->text() != "") {
        empty = 1;
    }
    ui->label_39->setDisabled(empty);
    ui->label_40->setDisabled(empty);
    ui->label_41->setDisabled(empty);
    ui->label_42->setDisabled(empty);
    ui->label_67->setDisabled(empty);
    ui->label_68->setDisabled(empty);
    ui->label_69->setDisabled(empty);
    ui->label_70->setDisabled(empty);
}

void MainWindow::on_pushButton_clicked()
{
    airportFile = QFileDialog::getOpenFileName(this,tr("Open airport file"), airportFile, tr("Airport files (*.dat *.dat.gz)"));
    ui->lineEdit_20->setText(airportFile);
    settings.setValue("path/airportFile", airportFile); // keep the last airport file used
}

// download shapefiles from mapserver
void MainWindow::on_pushButton_2_clicked()
{
    double eastInt     = m_east.toDouble();
    double northInt    = m_north.toDouble();
    double southInt    = m_south.toDouble();
    double westInt     = m_west.toDouble();

    if ((westInt < eastInt) && (northInt > southInt)) {
        QString url  = "http://mapserver.flightgear.org/";
        QString source = ui->comboBox_3->currentText();
        if (source == "Custom scenery"){
            url += "dlcs";
        }
        else if (source == "OpenStreetMap"){
            url += "dlosm";
        }
        else {
            url += "dlclc";
        }
        url += "?xmin="+m_west+"&xmax="+m_east+"&ymin="+m_south+"&ymax="+m_north;
        // save output to log
        outputToLog(url);
        QUrl qu(url);
        if ( ! QDesktopServices::openUrl(qu) ) {
            // QWebView::webview = new QWebView;
            // webview.load(url);
            QMessageBox::critical(this,"URL cannot be opened","The following URL cannot be opened "+url+".\nCopy the URL to your browser");
        }
    }
    else{
        QString msg = tr("Minimum longitude and/or latitude is not less than maximum.\nCorrect the coordinates, and try again.");
        outputToLog(msg);
        QMessageBox::critical(this,tr("Boundary error"),msg);
    }

}

// select elevation directory
void MainWindow::on_pushButton_3_clicked()
{
    elevationDirectory = QFileDialog::getExistingDirectory(this,tr("Select the elevation directory, this is the directory in which the .hgt files live."),elevationDirectory);
    ui->lineEdit_11->setText(elevationDirectory);
    settings.setValue("path/elevationDir", elevationDirectory); // keep the last directory used
}

// run genapts
// missing adding an elevation source directory - ie --terrain=<path> - This would be in addition to the
// 10 builtin elevation sources searched - SRTM-1, SRTM-3, SRTM-30, DEM-USGS-3, etc
// And although a 'Chunk' edit box has been added, it is presently disabled/not used, as just
// another way to set mina/max ranges
void MainWindow::on_pushButton_5_clicked()
{
    QString arguments;
    // construct genapts commandline
    QString airportId   = ui->lineEdit_18->text();
    QString startAptId  = ui->lineEdit_19->text();
    QString tileId  = ui->lineEdit_13->text();

    QString minLat  = m_south;
    QString maxLat  = m_north;
    QString minLon  = m_west;
    QString maxLon  = m_east;
    QString maxSlope  = ui->lineEdit_21->text();
    QTime rt;
    QString tm;
    QString msg;

    if ( !util_verifySRTMfiles(minLat, maxLat, minLon, maxLon, workDirectory) ) {
        // potentially NO elevations for AIRPORT - generally a BIG waste of time to continue
        arguments = "No elevation data was found in "+workDirectory+"\n\nThis means airports will be generated with no elevation information!";
        if ( ! getYesNo("No elevation data found",arguments) )
            return;
    }
    rt.start();
    // proceed to do airport generation
    arguments   = "\""+terragearDirectory+"/genapts\" --input=\""+airportFile+"\" --work=\""+workDirectory+"\" ";
    if ( !ui->checkBox_minmax->isChecked() ) {
        if (airportId.size() > 0){
            arguments += "--airport="+airportId+" ";
        }
        if (startAptId.size() > 0) {
            arguments += "--start-id="+startAptId+" ";
        }
        if (airportId == "" and tileId == "") {
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
    }

    if (maxSlope.size() > 0){
        arguments += "--max-slope="+maxSlope+" ";
    }
    if (tileId.size() > 0){
        arguments += "--tile="+tileId+" ";
    }

    // save output to log
    outputToLog(arguments);

    QProcess proc;
    proc.start(arguments, QIODevice::ReadWrite);

    // run genapts command
    proc.waitForReadyRead();
    proc.QProcess::waitForFinished();
    int errCode = proc.exitCode();
    tm = " in "+getElapTimeStg(rt.elapsed());
    msg = proc.readAllStandardOutput()+"\n";
    if (errCode) {
        msg += proc.readAllStandardError()+"\n";
    }
    msg += "*PROC_ENDED*"+tm+"\n";
    output += msg;
    ui->textBrowser->setText(output);
    outputToLog("PROC_ENDED"+tm);

}

// download elevation data SRTM
void MainWindow::on_pushButton_6_clicked()
{
    // QString minElev and maxElev set
    outputToLog(elevList); /* provide a 'helpful' list of SRTM files */
    QString info = "\nDownload the SRTM in the range ["+minElev+"] to ["+maxElev+"]";
    info += "\nA list of the ranges should be in the log.txt file, if enabled";
    info += "\nWhen the zips are downloaded they should be expanded into a\ndirectory of your choice before running HGT CHop.";
    QString url = "http://dds.cr.usgs.gov/srtm/version2_1/";
    QUrl qu(url);
    if ( ! QDesktopServices::openUrl(qu) ) {
        // QWebView::webview = new QWebView;
        // webview.load(url);
        QMessageBox::critical(this,"OPEN URL FAILED","Attempted to opn the URL ["+url+"] but FAILED.\nCopy the URL to your browser"+info);
    } else {
        QMessageBox::information(this,"DATA DOWNLOAD","The URL ["+url+"] should be displayed in your default browser. Download the required zips.\nIf this fails, copy the URL to your browser manually."+info);
    }
}

// select project directory
void MainWindow::on_pushButton_7_clicked()
{
    projDirectory = QFileDialog::getExistingDirectory(this,tr("Select the project's location, everything that is used and created during the scenery generating process is stored in this location."));
    ui->lineEdit_4->setText(projDirectory);
    settings.setValue("paths/project", projDirectory);

    // set project's directories
    dataDirectory = projDirectory+"/data";
    outpDirectory = projDirectory+"/output";
    workDirectory = projDirectory+"/work";
}

// select FlightGear root
void MainWindow::on_pushButton_8_clicked()
{
    fgRoot = QFileDialog::getExistingDirectory(this,tr("Select the FlightGear root (data directory). This is optional; it is only used to retrieve an up-to-date list of available materials. You can use the GUI without setting the FG root."));
    ui->lineEdit_22->setText(fgRoot);
    settings.setValue("paths/fg-root", fgRoot);

    updateMaterials();
}

// select TerraGear directory
void MainWindow::on_pushButton_9_clicked()
{
    terragearDirectory = QFileDialog::getExistingDirectory(this,tr("Select TerraGear root, this is the directory in which ogr-decode, genapts etc. live."));
    ui->lineEdit_2->setText(terragearDirectory);
    settings.setValue("paths/terragear", terragearDirectory);

    // disable tabs if terragear path is not set
    int enabled = 0;
    if (terragearDirectory != ""){
        enabled = 1;
    }
    ui->tabWidget->setTabEnabled(1,enabled);
    ui->tabWidget->setTabEnabled(2,enabled);
    ui->tabWidget->setTabEnabled(3,enabled);
    ui->tabWidget->setTabEnabled(4,enabled);
}

// run hgt-chop
void MainWindow::on_pushButton_11_clicked()
{
    QString minnode     = ui->lineEdit->text();
    QString maxnode     = ui->lineEdit_3->text();
    QString maxerror    = ui->lineEdit_23->text();

    QDir dir(elevationDirectory);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    QFileInfoList list = dir.entryInfoList();
    QString tot;
    QString cnt;
    QString tm;
    QTime rt;
    QTime pt;
    QStringList argList;
    QStringList filList;
    QString elevationFile;
    QString arguments;
    int i;

    if (list.size() == 0) {
        QMessageBox::information(this,"ERROR","There are no elevation files in "+elevationDirectory+"\nNothing to do!");
        return;
    }

    // reset progress bar
    ui->progressBar_3->setValue(0);

    rt.start(); // start total running time
    tot.sprintf("%d", list.size());

    // build set of arguments
    for (i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QString elevationRes = ui->comboBox->currentText();
        elevationFile        = QString("%1").arg(fileInfo.fileName());
        arguments            = "\""+terragearDirectory+"/hgtchop\" "+elevationRes+" \""+elevationDirectory+"/"+elevationFile+"\" \""+workDirectory+"/SRTM-30\"";
        // store runtime argument, and file name
        // could add a check that it is a HGT file...
        argList += arguments;
        filList += elevationFile;
    }

    // process set of arguments
    for (i = 0; i < argList.size(); i++)
    {
        pt.start();
        arguments = argList[i];
        elevationFile = filList[i];

        outputToLog(arguments);

        cnt.sprintf("%d", (i + 1));
        tm = " (elap "+getElapTimeStg(rt.elapsed())+")";
        ui->label_52->setText("File:"+cnt+" of "+tot+": "+elevationFile+tm);
        ui->label_52->repaint();

        // adjust progress bar
        ui->progressBar_3->setMaximum(tot.toInt()*2);
        ui->progressBar_3->setValue(cnt.toInt());

        QProcess proc;
        proc.start(arguments, QIODevice::ReadWrite);

        // run hgtchop command
        proc.waitForReadyRead();
        proc.QProcess::waitForFinished();

        output += proc.readAllStandardOutput()+"\n*PROC_ENDED*\n";
        ui->textBrowser->setText(output);
        outputToLog("PROC_ENDED");

        tm = " in "+getElapTimeStg(pt.elapsed());
        output += proc.readAllStandardOutput()+"\n*PROC_ENDED*"+tm+"\n";
        ui->textBrowser->setText(output);
        ui->textBrowser->repaint(); // get the info shown
        outputToLog("PROC_ENDED"+tm);
        // save output to log - all is now saved at application end
    }

    pt.start();
    ui->label_52->setText("Now running terrafit...");
    ui->label_52->repaint();

    // generate and run terrafit command
    QString argumentsTerrafit = "\""+terragearDirectory+"/terrafit\" ";
    if (minnode.size() > 0){
        argumentsTerrafit += "--minnodes "+minnode+" ";
    }
    if (maxnode.size() > 0){
        argumentsTerrafit += "--maxnodes "+maxnode+" ";
    }
    if (maxerror.size() > 0){
        argumentsTerrafit += "--maxerror "+maxerror+" ";
    }

    argumentsTerrafit +="\""+workDirectory+"/SRTM-30\"";

    outputToLog(argumentsTerrafit);
    QProcess procTerrafit;
    procTerrafit.start(argumentsTerrafit, QIODevice::ReadWrite);
    procTerrafit.waitForReadyRead();
    procTerrafit.QProcess::waitForFinished();

    tm = " in "+getElapTimeStg(pt.elapsed());
    output += procTerrafit.readAllStandardOutput()+"\n*PROC_ENDED*"+tm+"\n";

    ui->textBrowser->setText(output);
    ui->textBrowser->repaint(); // get the info shown
    outputToLog("PROC_ENDED"+tm);

    if (list.size() > 0) {
        tm = " "+getElapTimeStg(rt.elapsed()); // get elapsed time string
        cnt.sprintf("%d", list.size());
        ui->label_52->setText("Done "+cnt+" files in "+tm);
        outputToLog("Done "+cnt+" files in "+tm);

        // adjust progress bar
        ui->progressBar_3->setMaximum(tot.toInt()*2);
        ui->progressBar_3->setValue(tot.toInt()+cnt.toInt());
    } else {
        ui->label_52->setText("No files processed...");
    }
}


// update shapefiles list for ogr-decode
void MainWindow::on_pushButton_12_clicked()
{
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
        if (fInfo.suffix() == "dbf" or fInfo.suffix() == "prj" or fInfo.suffix() == "shp" or fInfo.suffix() == "shx"){
            fFileName1.chop(4);     // remove fileformat from name
            QFile file (fFilePath);
            QString fPath_ren = fPath+"/"+fFileName1+"/"+fFileName2;
            dir.mkpath(fPath+"/"+fFileName1);
            dir.rename(fFilePath, fPath_ren);
        }
    }

    // update list
    while (ui->tblShapesAlign->rowCount() != 0)
    {
        ui->tblShapesAlign->removeRow(0);
    }

    // list of scenery shapefiles
    QStringList csShape;
    csShape << "cs_agroforest" << "cs_airport" << "cs_asphalt" << "cs_barrencover" << "cs_bog" <<
            "cs_burnt" << "cs_canal" << "cs_cemetery" << "cs_complexcrop" << "cs_construction" <<
            "cs_cropgrass" << "cs_deciduousforest" << "cs_default" << "cs_dirt" << "cs_drycrop" <<
            "cs_dump" << "cs_estuary" << "cs_evergreenforest" << "cs_floodland" << "cs_freeway" <<
            "cs_glacier" << "cs_golfcourse" << "cs_grassland" << "cs_greenspace" << "cs_heath" <<
            "cs_hebtundra" << "cs_industrial" << "cs_intermittentlake" << "cs_intermittentstream" <<
            "cs_irrcrop" << "cs_lagoon" << "cs_lake" << "cs_lava" << "cs_littoral" << "cs_marsh" <<
            "cs_mixedcrop" << "cs_mixedforest" << "cs_naturalcrop" << "cs_olives" << "cs_openmining" <<
            "cs_orchard" << "cs_packice" << "cs_polarice" << "cs_port" << "cs_railroad1" << "cs_railroad2" <<
            "cs_rainforest" << "cs_rice" << "cs_road" << "cs_rock" << "cs_saline" << "cs_saltmarsh" <<
            "cs_sand" << "cs_sclerophyllous" << "cs_scrub" << "cs_stream" << "cs_suburban" << "cs_town" <<
            "cs_transport" << "cs_urban" << "cs_vineyard" << "cs_watercourse" << "v0_landmass" << 
            "clc_airport" << "clc_complexcrop" << "clc_construction" << "clc_cropgrass" << "clc_deciduousforest" << 
            "clc_drycrop" << "clc_evergreenforest" << "clc_golfcourse" << "clc_grassland" << "clc_greenspace" << 
            "clc_industrial" << "clc_lake" << "clc_marsh" << "clc_naturalcrop" << "clc_port" << "clc_sand" << 
            "clc_town" << "clc_transport" << "clc_watercourse" << "osm_canal" << "osm_light_rail" << "osm_motorway" <<
            "osm_primary" << "osm_rail" << "osm_residential" << "osm_river" << "osm_secondary" << "osm_service" <<
            "osm_stream" << "osm_tertiary" << "osm_trunk";
			
    // list of correpsonding materials
    QStringList csMater;
    csMater << "AgroForest" << "Airport" << "Asphalt" << "BarrenCover" << "Bog" <<
            "Burnt" << "Canal" << "Cemetery" << "ComplexCrop" << "Construction" <<
            "CropGrass" << "DeciduousForest" << "Default" << "Dirt" << "DryCrop" <<
            "Dump" << "Estuary" << "EvergreenForest" << "FloodLand" << "Freeway" <<
            "Glacier" << "GolfCourse" << "GrassLand" << "GreenSpace" << "Heath" <<
            "HerbTundra" << "Industrial" << "IntermittentLake" << "IntermittentStream" <<
            "IrrCrop" << "Lagoon" << "Lake" << "Lava" << "Littoral" << "Marsh" <<
            "MixedCrop" << "MixedForest" << "NaturalCrop" << "Olives" << "OpenMining" <<
            "Orchard" << "PackIce" << "PolarIce" << "Port" << "Railroad" << "Railroad" <<
            "RainForest" << "Rice" << "Road" << "Rock" << "Saline" << "SaltMarsh" <<
            "Sand" << "Sclerophyllous" << "ScrubCover" << "Stream" << "SubUrban" << "Town" <<
            "Transport" << "Urban" << "Vineyard" << "Watercourse" << "Landmass" << 
            "Airport" << "ComplexCrop" << "Construction" << "CropGrass" << "DeciduousForest" << 
            "DryCrop" << "EvergreenForest" << "GolfCourse" << "GrassLand" << "Greenspace" <<
            "Industrial" << "Lake" << "Marsh" << "NaturalCrop" << "Port" << "Sand" << 
            "Town" << "Transport" << "Watercourse" << "Canal" << "Railroad" << "Freeway" <<
            "Road" << "Railroad" << "Road" << "Canal" << "Road" << "Road" <<
            "Canal" << "Road" << "Road";

    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QFileInfoList dirList = dir.entryInfoList();
    for (int i = 0; i < dirList.size(); ++i) {
        QFileInfo dirInfo = dirList.at(i);
        if(dirInfo.fileName()!= "SRTM-30" and dirInfo.fileName()!= "SRTM-3" and dirInfo.fileName()!= "SRTM-1" and dirInfo.fileName()!= "SRTM"){
            ui->tblShapesAlign->insertRow(ui->tblShapesAlign->rowCount());
            QTableWidgetItem *twiCellShape = new QTableWidgetItem(0);
            twiCellShape->setText(tr(qPrintable(QString("%1").arg(dirInfo.fileName()))));
            twiCellShape->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
            ui->tblShapesAlign->setItem(ui->tblShapesAlign->rowCount()-1, 0, twiCellShape);
        }
        QTableWidgetItem *twiCellMater = new QTableWidgetItem(0);

        // suggest a material
        QString suggestedMaterial;
        for (int j = 0; j < csShape.length(); ++j) {
            if (dirInfo.fileName() == csShape[j]){
                suggestedMaterial = csMater[j];
            }
        }
        twiCellMater->setText(suggestedMaterial);
        ui->tblShapesAlign->setItem(i, 1, twiCellMater);
    }
    ui->tblShapesAlign->resizeRowsToContents();

    // check for tool existance
    QString tgTool = terragearDirectory+"/ogr-decode";
#ifdef Q_OS_WIN
    tgTool += ".exe";
#endif
    QFile f(tgTool);
    if ( ! f.exists() ) {
        tgTool = terragearDirectory+"/shape-decode";
#ifdef Q_OS_WIN
        tgTool += ".exe";
#endif
        QFile f2(tgTool);
        if (f2.exists()) {
            ui->checkBox_ogr->setCheckState(Qt::Checked);
        }
    }
}

// run fgfs-construct (fgfs_construct)
void MainWindow::on_pushButton_13_clicked()
{
    QString lat = ui->label_67->text();
    QString lon = ui->label_68->text();
    QString x = ui->label_70->text();
    QString y = ui->label_69->text();
    QString selectedMaterials;
    QString msg;
    bool brk = false;
    int folderCnt = ui->listWidget_2->count();
    if (folderCnt == 0) {
        QMessageBox::critical(this,"No terraintypes","There are no terraintypes listed. Use [Update list] to populate it, then select those desired.");
        return;
    }
    folderCnt = 0; // RESTART COUNTER
    // create string with selected terraintypes
    for (int i = 0; i < ui->listWidget_2->count(); ++i){
        if (ui->listWidget_2->item(i)->isSelected() == 1) {
            selectedMaterials += ui->listWidget_2->item(i)->text()+" ";
            folderCnt++;
        }
    }
    if (folderCnt == 0) {
        QMessageBox::critical(this,"No terraintype selected","It appears you have not selected any of the terraintypes! Select all types that you would like to include in the scenery.");
        return;
    }
    // reset progress bar
    ui->progressBar->setValue(0);

#ifdef _NEWBUCKET_HXX   // we have SGBucket capability
    // construct fgfs-construct commandline,
    // FOR EACH BUCKET SEPARATELY, like master/client do
    // We could concurrently run multiple constructions, but then like server.cxx
    // we should skip every other column, to avoid two clients working on adjacent tiles
    // but here fgfs-construct is just run with for each 'bucket'
    QStringList argList; // build a string list to run
    QStringList pathList; // and the PATH to each bucket
    QString arguments;
    QString index;
    QString path;
    QTime rt;
    QTime pt;
    QString tm;
    QString em;
    QString info;
    bool no_overwrite = ui->checkBox_noovr->isChecked();
    bool skip_error = ui->checkBox_igerr->isChecked();
    bool skip_nodata = ui->checkBox_nodata->isChecked();
    long ind;
    int dx, dy, i, j;
    int gotcnt = 0;
    bool add_it = true; // ADD all buckets, unless there is a reason not to
    SGBucket b_cur;
    rt.start();
    // build the general runtime string
    QString runtime = "\""+terragearDirectory+"/fgfs-construct\" ";
    runtime += "--priorities=\""+terragearDirectory+"/default_priorities.txt\" ";
    runtime += "--usgs-map=\""+terragearDirectory+"/usgsmap.txt\" ";
    runtime += "--work-dir=\""+workDirectory+"\" ";
    runtime += "--output-dir=\""+outpDirectory+"/Terrain\" ";
    if (ui->checkBox_3->isChecked()) {
        runtime += "--useUKgrid ";
    }
    if (ui->checkBox_4->isChecked()) {
        runtime += "--ignore-landmass ";
    }
    gotcnt = 0;
    if (ui->lineEdit_35->text() > 0) {
        // just ONE to do - DO IT
        index = ui->lineEdit_35->text();
        ind = index.toLong();
        SGBucket b(ind);
        path.sprintf("%s", b.gen_base_path().c_str());
        path += "/"+index;
        arguments = runtime; // common runtime and params
        arguments += "--tile-id="+index+" ";
        arguments += selectedMaterials;
        if (no_overwrite) {
            msg = outpDirectory+"/Terrain/"+path+".btg.gz";
            QFile f(msg);
            if ( f.exists() ) {
                add_it = false;
            }
        }
        if (add_it) {
            argList << arguments; // set the ONE argument
            pathList << path;
        }
        gotcnt++;
    } else {
        // break the set into buckets
        double dlon = lon.toDouble();
        double dlat = lat.toDouble();
        double xdist = x.toDouble();
        double ydist = y.toDouble();

        double min_x = dlon - xdist;
        double min_y = dlat - ydist;
        SGBucket b_min( min_x, min_y );
        SGBucket b_max( dlon + xdist, dlat + ydist );
        if (b_min == b_max) {
            // just ONE bucket
            index.sprintf("%ld", b_min.gen_index());
            path.sprintf("%s", b_min.gen_base_path().c_str());
            path += "/"+index;
            arguments = runtime; // common runtime and params
            arguments += "--tile-id="+index+" ";
            arguments += selectedMaterials;
            if (no_overwrite) {
                msg = outpDirectory+"/Terrain/"+path+".btg.gz";
                QFile f(msg);
                if ( f.exists() ) {
                    add_it = false;
                }
            }
            if (add_it && skip_nodata) {
                if ( !countDataFound(path,selectedMaterials,workDirectory))
                    add_it = false;
            }
            if (add_it) {
                argList << arguments; // set the ONE argument
                pathList << path;
            }
            gotcnt++;
        } else {
            // a range of buckets
            sgBucketDiff(b_min, b_max, &dx, &dy);
            for ( j = 0; j <= dy; j++ ) {
                for ( i = 0; i <= dx; i++ ) {
                    add_it = true; // initially ADD ALL buckets, for this INDEX
                    b_cur = sgBucketOffset(min_x, min_y, i, j);
                    index.sprintf("%ld", b_cur.gen_index());
                    path.sprintf("%s", b_cur.gen_base_path().c_str());
                    path += "/"+index;
                    arguments = runtime;
                    arguments += "--tile-id="+index+" ";
                    arguments += selectedMaterials;
                    if (no_overwrite) {
                        msg = outpDirectory+"/Terrain/"+path+".btg.gz";
                        QFile f(msg);
                        if ( f.exists() ) {
                            add_it = false; // KILLLED BY NO OVERWRITE RULE
                        }
                    }
                    if (add_it && skip_nodata) {
                        if ( !countDataFound(path,selectedMaterials,workDirectory))
                            add_it = false; // KILLED BY NO ARRAY FILES FOUND - build with what??
                    }
                    if (add_it) {
                        argList << arguments; // set the EACH argument
                        pathList << path;
                    }
                    gotcnt++; // count the max. possible total for range
                }
            }
        }
    }
    ind = argList.size();
    if (ind != pathList.size()) {
        // catastophic FAILURE
        QMessageBox::critical(this,"INTERNAL ERROR","Lists are NOT equal in length!");
        return;
    }
    if (ind == 0) {
        msg.sprintf("With the current min/max, and options, have %d buckets to process, but perhpas no overwrite, and/or skip no data are checked, so nothing to do!", gotcnt);
        QMessageBox::information(this,"NO BUCKETS TO PROCESS",msg);
        return;
    }

    // we are on our way. to construct <index>.btg.gz file(s),
    // plus the associated <index>.stg to load these items in FG
    int setup_ms = rt.restart(); // restart timer

    // add a complete set of arguments to a templog.txt
    dy = 0;
    tm = "Setup: "+getElapTimeStg(setup_ms);
    outTemp(tm+"\n");
    for (i = 0; i < argList.size(); i++) {
        dy++;
        arguments = argList[i]; // get command line arguments
        path = pathList[i]; // get the destination path
        // output commandline to log.txt
        outTemp(arguments+"\n");
        msg.sprintf("%d: ", dy);
        msg += outpDirectory+"/Terrain/"+path+".btg.gz\n";
        outTemp(msg);
    }

    dx = argList.size();
    dy = 0;
    uint diff_time;
    uint tot_time;

    tot_time = 0; // start accumulating the total time (in ms)
    // this is the section that should be run on a thread
    // ==================================================
    for (i = 0; i < argList.size(); i++) {
        if (brk || m_break)
            break; // all over - user requested a break
        dy++;
        pt.start();
        // about to RUN fgfs-construct, for each bucket argument
        arguments = argList[i]; // get command line arguments
        path = pathList[i]; // get the destination path
        // output commandline to log.txt
        outputToLog(arguments);

        // and show starting proc
        tm = " rt "+getElapTimeStg(rt.elapsed());
        // msg.sprintf("%d of %d: fgfs-construct with %d folders - moment...", dy, dx, folderCnt);
        msg.sprintf("%d of %d: fgfs-construct ", dy, dx);

        msg += path;
        msg += tm;
        ui->label_54->setText(msg);
        ui->label_54->repaint(); // does not seem to always appear, but sometimes???

        // start command
        QProcess proc;
        proc.setWorkingDirectory(terragearDirectory);
        proc.start(arguments, QIODevice::ReadWrite);

        // wait for process to finish, before allowing the next action
        proc.waitForReadyRead();
        proc.QProcess::waitForFinished();

        int errCode = proc.exitCode();
        info = proc.readAllStandardOutput();
        diff_time = pt.elapsed();
        tot_time += diff_time;
        tm = " in "+getElapTimeStg(diff_time);
        arguments = "\n*PROC_ENDED* "+tm+"\n";
        em = "";
        if (errCode) {
            em.sprintf("ERROR CODE %d",errCode);
            info += proc.readAllStandardError();
            arguments = "\n*PROC_ENDED* with "+em+"\n";
        }

        outTemp(info+"\n");
        outTemp(arguments+"\n");
        output += info+"\n"; // add to full output
        output += arguments+"\n";

        // adjust progress bar
        ui->progressBar->setMaximum(dx);
        ui->progressBar->setValue(dy);

        ui->textBrowser->setText(info); // only the last
        ui->textBrowser->repaint();

        msg = "PROC_ENDED "+tm+", total "+getElapTimeStg(rt.elapsed())+" "+path;
        outputToLog(msg);
        outTemp(msg+"\n");

        if (info.contains("unknown area = ")) {
            QMessageBox msgBox;
            msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Ignore);
            msgBox.setDefaultButton(QMessageBox::Abort);
            msgBox.setWindowTitle("Unknown area");
            msgBox.setText("A material is not listed in default_priorities.txt.");
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
        // if the string end is [Finished successfully], then all is well
        else if ( ! info.contains("[Finished successfully]") ) {
            if (em == "ERROR CODE -1"){
                msg = "FGFS Construct failed to ouput [Finished successfully].\n";
            }
            else {
                msg = "FGFS Construct error: ";
                msg += em+"\n";
            }
            msg += "Tile: "+path+"\n\n";
            msg += "This usually indicates some error in processing!\n";
            outTemp(msg);
            if (!skip_error) {
                msg += "Click OK to continue contruction.";
                if ( !getYesNo("Process warning",msg) ) {
                    break;
                }
            }
        }
        ui->label_54->repaint(); // does not seem to always appear, but sometimes???
    }

    // scenery has been successfully created, congratulate developer
    if ( info.contains("[Finished successfully]") ) {
        msg = "Your scenery has been successfully created!";
        QMessageBox::information(this,"Congratulations", msg);
    }

    // ==================================================================
    msg.sprintf("fgfs-construct did %d buckets (%d/%d)", i, argList.count(), gotcnt);
    msg += " in "+getElapTimeStg(rt.elapsed());
    ui->label_54->setText(msg);
    ui->textBrowser->setText(output); // add it ALL

#else // !#ifdef _NEWBUCKET_HXX

    // construct fgfs-construct commandline
    QString arguments = "\""+terragearDirectory+"/fgfs-construct\" ";
    arguments += "--work-dir=\""+workDirectory+"\" ";
    arguments += "--output-dir=\""+outpDirectory+"/Terrain\" ";

    if (ui->lineEdit_35->text() > 0) {
        arguments += "--tile-id="+ui->lineEdit_35->text();
    } else {
        // if a tile ID, no need for these...
        arguments += "--lon="+lon+" --lat="+lat+" ";
        arguments += "--xdist="+x+" --ydist="+y+" ";
    }

    if (ui->checkBox_3->isChecked()){
        arguments += "--useUKgrid ";
    }
    if (ui->checkBox_4->isChecked()){
        arguments += "--ignore-landmass ";
    }
    arguments += selectedMaterials;

    // output commandline to log.txt
    outputToLog(arguments);

    // and show starting proc
    msg.sprintf("Start fgfs-construct with %d folders - moment...", cnt);
    ui->label_54->setText(msg);
    ui->label_54->repaint(); // does not seem to appear???

    rt.start();

    // start command
    QProcess proc;
    proc.setWorkingDirectory(terragearDirectory);
    proc.start(arguments, QIODevice::ReadWrite);

    // wait for process to finish, before allowing the next action
    proc.waitForReadyRead();
    proc.QProcess::waitForFinished();
    int errCode = proc.exitCode();
    output += proc.readAllStandardOutput();

    tm = " in "+getElapTimeStg(rt.elapsed())
    arguments = "\n*PROC_ENDED*"+tm+"\n";
    if (errCode) {
        output += proc.readAllStandardError();
        arguments.sprintf("\nPROC_ENDED with ERROR CODE %d!\n",errCode);
    }
    output += arguments;
    ui->textBrowser->setText(output);
    outputToLog("PROC_ENDED"+tm);
    msg = "fgfs-construct ran for "+tm;
    ui->label_54->setText(msg);

#endif // #ifdef _NEWBUCKET_HXX y/n

}

// update terraintypes list for fgfs-construct
// *TBD* Should maybe EXCLUDE directory 'Shared'!
void MainWindow::on_pushButton_15_clicked()
{
    int j = 0;
    ui->listWidget_2->clear();
    QDir dir(workDirectory);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);

        // do not add the Shared folder
        if (fileInfo.fileName() != "Shared"){
            QString test = qPrintable(QString("%1").arg(fileInfo.fileName()));
            new QListWidgetItem(tr(qPrintable(QString("%1").arg(fileInfo.fileName()))), ui->listWidget_2);
            // select all materials per default
            ui->listWidget_2->item(j)->setSelected(1);
            j++;
        }
    }
}

// run ogr-decode (or shape-decode)
void MainWindow::on_pushButton_16_clicked()
{
    QTime rt;
    QTime pt;
    QString tm;
    int i;
    QString msg;
    QStringList argList;
    QStringList shpList;
    QString arguments;
    QString info;
    QString shapefile;

    // reset progress bar
    ui->progressBar_2->setValue(0);

    rt.start();
    // check if terragear tool exists
    QString TGfile      = terragearDirectory+"/ogr-decode";
    if (ui->checkBox_ogr->isChecked()) {
        TGfile = terragearDirectory+"/shape-decode";
    }
#ifdef Q_OS_WIN
    TGfile += ".exe"; // add EXE for windows
#endif
    QFile f(TGfile);
    if ( ! f.exists() ) {
        QString msg = "Unable to locate executable at "+TGfile;
        QMessageBox::critical(this,"File not found", msg);
        return;
    }

    // skip if no shapefiles are listed
    if (ui->tblShapesAlign->rowCount() == 0){
        QMessageBox::critical(this,"Shapefile error", "No shapefiles are listed. Please click the [Retrieve shapefiles] button.");
        return;
    }

    for (i = 0; i < ui->tblShapesAlign->rowCount(); i++) {
        // skip if material are not assigned
        if ((ui->tblShapesAlign->item(i, 1) == 0) || (ui->tblShapesAlign->item(i, 1)->text().length() == 0)){
            QMessageBox::critical(this,"Material error", "You did not assign materials for each shapefile.");
            return;
        }
    }

    // got executable, and have assigned materials, so do the work
    for (i = 0; i < ui->tblShapesAlign->rowCount(); i++) {
        QString material = ui->tblShapesAlign->item(i, 0)->text();
        QString lineWidth;
        if (ui->tblShapesAlign->item(i, 2) != 0) {
            // cell item already created
            lineWidth = ui->tblShapesAlign->item(i, 2)->text();
        }
        else {
            // cell item are not created - default width
            lineWidth = "10";
        }

        shapefile = ui->tblShapesAlign->item(i, 1)->text();

        if (ui->checkBox_ogr->isChecked()) {
            /* until I can get ogr-decode built ;=() */
            arguments   = "\""+terragearDirectory+"/shape-decode\" ";
            arguments += "--line-width "+lineWidth+" ";

            if (ui->lineEdit_24->text() > 0){
                arguments += "--point-width "+ui->lineEdit_24->text()+" ";
            }
            if (ui->checkBox_2->isChecked() == 1){
                arguments += "--continue-on-errors ";
            }
            if (ui->lineEdit_26->text() > 0){
                arguments += "--max-segment "+ui->lineEdit_26->text()+" ";
            }
            // last 3 arguments
            // shape file, with no extension
            arguments += "\""+dataDirectory+"/"+material+"/"+material+"\" ";
            // work directory - where to put the output
            arguments += "\""+workDirectory+"/"+shapefile+"\" ";
            // area string
            arguments += " "+shapefile+" ";

        } else {
            arguments   = "\""+terragearDirectory+"/ogr-decode\" ";

            arguments += "--line-width "+lineWidth+" ";

            if (ui->lineEdit_24->text() > 0){
                arguments += "--point-width "+ui->lineEdit_24->text()+" ";
            }
            if (ui->checkBox_2->isChecked() == 1){
                arguments += "--continue-on-errors ";
            }
            if (ui->lineEdit_26->text() > 0){
                arguments += "--max-segment "+ui->lineEdit_26->text()+" ";
            }
            arguments += "--area-type "+shapefile+" ";
            arguments += "\""+workDirectory+"/"+shapefile+"\" ";
            arguments += "\""+dataDirectory+"/"+material+"\"";
        }
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
        outputToLog(arguments);

        // advise file in process
        ui->label_53->setText("Doing file "+shapefile);
        ui->label_53->repaint();

        QProcess proc;
        proc.start(arguments, QIODevice::ReadWrite);

        // run command
        proc.waitForReadyRead();
        proc.QProcess::waitForFinished();
        int errCode = proc.exitCode();
        info = proc.readAllStandardOutput();
        tm = " in "+getElapTimeStg(pt.elapsed());
        arguments = "\n*PROC_ENDED*"+tm+"\n";
        if (errCode) {
            info += proc.readAllStandardError();
            arguments.sprintf("\n*PROC_ENDED* with ERROR CODE %d!\n",errCode);

            if ( errCode == -1073741515) {
                QMessageBox msgBox;
                msgBox.setStandardButtons(QMessageBox::Abort | QMessageBox::Ignore);
                msgBox.setDefaultButton(QMessageBox::Abort);
                msgBox.setWindowTitle("Process error");
                msgBox.setText("Failed to decode shapefiles.\nTry using Shape Decode, instead of OGR Decode.");
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
        }
        output += arguments+"\n"+info+"\n";
        outTemp(arguments+"\n"+info+"\n");

        ui->textBrowser->setText(info);
        ui->textBrowser->repaint();

        msg = "PROC_ENDED"+tm;
        outputToLog(msg);
        outTemp(msg+"\n");

        // adjust progress bar
        ui->progressBar_2->setMaximum(argList.size()-1);
        ui->progressBar_2->setValue(i);
    }

    msg.sprintf("Done %d files", argList.size());
    msg += " in "+getElapTimeStg(rt.elapsed());
    ui->label_53->setText(msg);
    ui->textBrowser->setText(output);
}

// add material
void MainWindow::on_pushButton_17_clicked()
{
    if (ui->tblShapesAlign->item(ui->tblShapesAlign->currentRow(), 1) == 0) {
        QTableWidgetItem *twiMaterialCell = new QTableWidgetItem(0);
        twiMaterialCell->setText(ui->comboBox_2->itemText(ui->comboBox_2->currentIndex()));
        ui->tblShapesAlign->setItem(ui->tblShapesAlign->currentRow(), 1, twiMaterialCell);
    }
    else
    {
        ui->tblShapesAlign->item(ui->tblShapesAlign->currentRow(), 1)->setText(ui->comboBox_2->itemText(ui->comboBox_2->currentIndex()));
    }
}

// functions //

// update elevation download range
void MainWindow::updateElevationRange()
{
    QString east;
    QString north;
    QString south;
    QString west;

    double eastDbl     = ui->lineEdit_5->text().toDouble();
    double northDbl    = ui->lineEdit_7->text().toDouble();
    double southDbl    = ui->lineEdit_8->text().toDouble();
    double westDbl     = ui->lineEdit_6->text().toDouble();

    // initialize text colors
    QPalette q1;
    QPalette q2;
    q1.setColor(QPalette::Text, Qt::black);
    q2.setColor(QPalette::Text, Qt::black);

    QString prevmin = minElev;
    QString prevmax = maxElev;
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
        ui->label_50->setText(minElev);
        ui->label_51->setText(maxElev);

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
            // outputToLog(elevList); /* provide a 'helpful' list of SRTM files */
        }
    }

    // if boundaries are not valid: do not display elevation range and set text color to red
    else{
        ui->label_50->setText("");
        ui->label_51->setText("");

        if (westDbl == eastDbl or westDbl > eastDbl){
            q1.setColor(QPalette::Text, Qt::red);
        }
        if (northDbl == southDbl or southDbl > northDbl){
            q2.setColor(QPalette::Text, Qt::red);
        }
    }

    // change text color in the boundary fields
    ui->lineEdit_5->setPalette(q1);
    ui->lineEdit_6->setPalette(q1);
    ui->lineEdit_7->setPalette(q2);
    ui->lineEdit_8->setPalette(q2);
}

void MainWindow::on_tblShapesAlign_cellDoubleClicked(int row, int column)
{
    if (column == 0)
        ui->tblShapesAlign->removeRow(row);
}

// populate material list with materials from FG's materials.xml
void MainWindow::updateMaterials()
{
    QFile materialfile(fgRoot+"/materials.xml");
    if (materialfile.exists() == true) {

        if (materialfile.open(QIODevice::ReadOnly)) {

            QXmlStreamReader materialreader(&materialfile);
            QXmlStreamReader::TokenType tokenType;

            QStringList materialList;
            QString material;
            while ((tokenType = materialreader.readNext()) != QXmlStreamReader::EndDocument) {
                if (materialreader.name() == "material") {
                    while ((tokenType = materialreader.readNext()) != QXmlStreamReader::EndDocument) {


                        if (materialreader.name() == "name") {
                            material = materialreader.readElementText();
                            // ignore rwy lights, textures and signs
                            if (!material.startsWith("BlackSign") and !material.startsWith("dirt_rwy") and !material.startsWith("grass_rwy") and !material.startsWith("FramedSign") and !material.startsWith("lakebed_taxiway") and !material.startsWith("lf_") and !material.startsWith("pa_") and !material.startsWith("pc_") and !material.startsWith("RedSign") and !material.startsWith("RunwaySign") and !material.startsWith("RUNWAY_") and !material.startsWith("RWY_") and !material.startsWith("Unidirectional") and !material.startsWith("YellowSign")) {

                                // ignore materials already present
                                if (materialList.indexOf(material, 0) == -1)
                                    materialList.append(material);
                            }
                        }
                        // ignore sign materials
                        if (materialreader.name() == "glyph") {
                            materialreader.skipCurrentElement();
                        }
                    }
                }
            }
            materialfile.close();
            materialList.sort();
            ui->comboBox_2->clear();
            ui->comboBox_2->addItems(materialList);
        }
    }
}

// calculate center of scenery area and radii
void MainWindow::updateCenter()
{

    double eastInt     = m_east.toDouble();
    double northInt    = m_north.toDouble();
    double southInt    = m_south.toDouble();
    double westInt     = m_west.toDouble();

    QPalette p;

    if ((westInt < eastInt) && (northInt > southInt)) {
        double latInt      = (northInt + southInt)/2;
        double lonInt      = (eastInt + westInt)/2;

        QString lat     = QString::number(latInt);
        QString lon     = QString::number(lonInt);
        QString xRad     = QString::number(eastInt-lonInt);
        QString yRad     = QString::number(northInt-latInt);

        ui->label_67->setText(lat);
        ui->label_68->setText(lon);
        ui->label_70->setText(xRad);
        ui->label_69->setText(yRad);

        p.setColor(QPalette::WindowText, Qt::black);
    }
    else{
        p.setColor(QPalette::WindowText, Qt::red);
    }

    // change label colors on error - on Start tab/page
    ui->label_7->setPalette(p);
    ui->label_10->setPalette(p);
    ui->label_11->setPalette(p);
    ui->label_9->setPalette(p);
}

void MainWindow::outputToLog(QString s)
{
    if ( ui->checkBox_log->isChecked() ) {
        QDateTime datetime  = QDateTime::currentDateTime();
        QString sDateTime   = datetime.toString("yyyy/MM/dd HH:mm:ss");

        QFile data(projDirectory+"/log.txt");
        if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
            QTextStream out(&data);
            out << endl;
            out << sDateTime;
            out << "  -  ";
            out << s;
        }
    }
}

void MainWindow::outTemp(QString s)
{
    QFile data(projDirectory+"/templog.txt");
    if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
        QTextStream out(&data);
        out << s;
    }
}

void MainWindow::on_checkBox_noovr_toggled(bool checked)
{
    settings.setValue("check/no_overwrite", checked);
}

void MainWindow::on_checkBox_4_toggled(bool checked)
{
    settings.setValue("check/ignore_landmass", checked);
}

void MainWindow::on_checkBox_minmax_clicked()
{
    // grey-out boundaries when ignored by genapts
    int checked = ui->checkBox_minmax->isChecked();
    ui->label_3->setDisabled(checked);
    ui->label_20->setDisabled(checked);
    ui->label_59->setDisabled(checked);
    ui->label_60->setDisabled(checked);
    ui->label_61->setDisabled(checked);
    ui->label_62->setDisabled(checked);
    ui->label_63->setDisabled(checked);
    ui->label_64->setDisabled(checked);
    ui->label_65->setDisabled(checked);
    ui->label_66->setDisabled(checked);
    ui->lineEdit_13->setDisabled(checked);
    ui->lineEdit_18->setDisabled(checked);
}

void MainWindow::on_checkBox_nodata_toggled(bool checked)
{
    settings.setValue("check/no_data", checked);
}

void MainWindow::on_checkBox_igerr_toggled(bool checked)
{
    settings.setValue("check/ign_errors", checked);
}

// eof - mainwindow.cpp
