#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QIcon>
#include <QIODevice>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPalette>
#include <QProcess>
#include <QSettings>
#include <QTextStream>
#include <QUrl>
#include <QXmlStreamReader>

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

// save variables for future sessions
QSettings settings("TerraGear", "TerraGearGUI");

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle( tr("TerraGear GUI") );

    // restore variables from previous session
    terragearDirectory  = settings.value("paths/terragear").toString();
    projDirectory       = settings.value("paths/project").toString();
    fgRoot              = settings.value("paths/fg-root").toString();

    if (fgRoot != 0){
        updateMaterials();
    }

    ui->lineEdit_2->setText(terragearDirectory);
    ui->lineEdit_4->setText(projDirectory);
    ui->lineEdit_22->setText(fgRoot);
    ui->lineEdit_8->setText(settings.value("boundaries/south").toString());
    ui->lineEdit_7->setText(settings.value("boundaries/north").toString());
    ui->lineEdit_6->setText(settings.value("boundaries/west").toString());
    ui->lineEdit_5->setText(settings.value("boundaries/east").toString());
    ui->lineEdit_30->setText(settings.value("boundaries/south").toString());
    ui->lineEdit_29->setText(settings.value("boundaries/north").toString());
    ui->lineEdit_27->setText(settings.value("boundaries/west").toString());
    ui->lineEdit_28->setText(settings.value("boundaries/east").toString());

    dataDirectory = projDirectory+"/data";
    outpDirectory = projDirectory+"/output";
    workDirectory = projDirectory+"/work";
}

MainWindow::~MainWindow()
{
    delete ui;
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
    QMessageBox::about(this, tr("TerraGUI v0.3"),tr("©2010-2011 Gijs de Rooy for FlightGear\nGNU General Public License version 2"));
}

// show wiki article in a browser
void MainWindow::on_wiki_triggered()
{
    QDesktopServices::openUrl(QUrl(tr("http://wiki.flightgear.org/index.php/TerraGear_GUI")));
}

// actions //

void MainWindow::on_lineEdit_5_editingFinished()
{
    updateElevationRange();
    settings.setValue("boundaries/east", ui->lineEdit_5->text());
}

void MainWindow::on_lineEdit_6_editingFinished()
{
    updateElevationRange();
    settings.setValue("boundaries/west", ui->lineEdit_6->text());
}

void MainWindow::on_lineEdit_7_editingFinished()
{
    updateElevationRange();
    settings.setValue("boundaries/north", ui->lineEdit_7->text());
}

void MainWindow::on_lineEdit_8_editingFinished()
{
    updateElevationRange();
    settings.setValue("boundaries/south", ui->lineEdit_8->text());
}

// delete shapefile
void MainWindow::on_listWidget_doubleClicked()
{
    int shapefilesLength    = ui->listWidget->count();
    int materialsLength     = ui->listWidget_3->count();

    // check if a material should be deleted
    if (shapefilesLength <= materialsLength)
    {
        delete ui->listWidget_3->takeItem(ui->listWidget->currentRow());
    }
    delete ui->listWidget->takeItem(ui->listWidget->currentRow());
}

// delete material
void MainWindow::on_listWidget_3_doubleClicked()
{
    delete ui->listWidget_3->takeItem(ui->listWidget_3->currentRow());
}

// edit material name
void MainWindow::on_listWidget_3_itemClicked(QListWidgetItem* item)
{
    QListWidgetItem * lwI = ui->listWidget_3->currentItem();
    lwI->setFlags(lwI->flags() | Qt::ItemIsEditable);
    ui->listWidget_3->editItem(lwI);
}

void MainWindow::on_pushButton_clicked()
{
    airportFile = QFileDialog::getOpenFileName(this,tr("Open airport file"), "C:/Users/AS9423-ULT/Desktop/TerraGear/Airports", tr("Airport files (*.dat)"));
    ui->lineEdit_20->setText(airportFile);
}

// download shapefiles
void MainWindow::on_pushButton_2_clicked()
{
    QString east    = ui->lineEdit_5->text();
    QString north   = ui->lineEdit_7->text();
    QString south   = ui->lineEdit_8->text();
    QString west    = ui->lineEdit_6->text();

    double eastInt     = east.toDouble();
    double northInt    = north.toDouble();
    double southInt    = south.toDouble();
    double westInt     = west.toDouble();

    QPalette q;

    if (westInt < eastInt and northInt > southInt){
        q.setColor(QPalette::WindowText, Qt::black);

        QString mapserverUrl = "http://mapserver.flightgear.org/dlcs?xmin="+west+"&xmax="+east+"&ymin="+south+"&ymax="+north;
        QDesktopServices::openUrl(mapserverUrl);
    }
    else{
        QMessageBox::about(this,tr("Error"),
                           tr("Wrong boundaries"));
        q.setColor(QPalette::WindowText, Qt::red);
    }

    // change label colors on error
    ui->label_7->setPalette(q);
    ui->label_9->setPalette(q);
    ui->label_10->setPalette(q);
    ui->label_11->setPalette(q);

    // set boundaries on FGFS construct and genapts pages
    ui->lineEdit_27->setText(west);
    ui->lineEdit_28->setText(east);
    ui->lineEdit_29->setText(north);
    ui->lineEdit_30->setText(south);
    ui->lineEdit_12->setText(west);
    ui->lineEdit_14->setText(east);
    ui->lineEdit_15->setText(north);
    ui->lineEdit_16->setText(south);
}

// select elevation directory
void MainWindow::on_pushButton_3_clicked()
{
    elevationDirectory = QFileDialog::getExistingDirectory(this,tr("Select the elevation directory, this is the directory in which the .hgt files live."));
    ui->lineEdit_11->setText(elevationDirectory);
}

// run genapts
void MainWindow::on_pushButton_5_clicked()
{
    // construct genapts commandline
    QString airportId   = ui->lineEdit_18->text();
    QString startAptId  = ui->lineEdit_19->text();
    QString arguments   = terragearDirectory+"/genapts.exe --input="+airportFile+" --work="+workDirectory+" ";
    if (airportId > 0){
        arguments += "--airport="+airportId+" ";
    }
    if (startAptId > 0){
        arguments += "--start-id="+startAptId+" ";
    }
    QMessageBox::about(this, tr("Command line"),arguments);
    QProcess proc;
    proc.start(arguments, QIODevice::ReadWrite);

    // run genapts command
    proc.waitForReadyRead();
    proc.QProcess::waitForFinished();
    output += proc.readAllStandardOutput()+"\n\n";
    ui->textBrowser->setText(output);
}

// download elevation data
void MainWindow::on_pushButton_6_clicked()
{
    QDesktopServices::openUrl(QUrl(tr("http://dds.cr.usgs.gov/srtm/version2_1/")));
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
    fgRoot = QFileDialog::getExistingDirectory(this,tr("Select the FlightGear root (data directory)."));
    ui->lineEdit_22->setText(fgRoot);
    settings.setValue("paths/fg-root", fgRoot);

    updateMaterials();
}

// select TerraGear directory
void MainWindow::on_pushButton_9_clicked()
{
    terragearDirectory = QFileDialog::getExistingDirectory(this,tr("Select TerraGear root, this is the directory in which ogr-decode.exe, genapts.exe etc. live."));
    ui->lineEdit_2->setText(terragearDirectory);
    settings.setValue("paths/terragear", terragearDirectory);
}

// run hgt-chop
void MainWindow::on_pushButton_11_clicked()
{
    QDir dir(elevationDirectory);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        QString elevationFile = QString("%1").arg(fileInfo.fileName());
        QString elevationRes = ui->comboBox->currentText();
        QString arguments = terragearDirectory+"/hgtchop.exe "+elevationRes+" "+elevationDirectory+"/"+elevationFile+" "+workDirectory+"/SRTM-30";

        QProcess proc;
        proc.start(arguments, QIODevice::ReadWrite);

        // run hgtchop command
        proc.waitForReadyRead();
        proc.QProcess::waitForFinished();
        output += proc.readAllStandardOutput()+"\n\n";
        ui->textBrowser->setText(output);

        // generate and run terrafit command
        QString argumentsTerrafit = terragearDirectory+"/terrafit.exe "+workDirectory+"/SRTM-30";

        QProcess procTerrafit;
        procTerrafit.start(argumentsTerrafit, QIODevice::ReadWrite);
        procTerrafit.waitForReadyRead();
        procTerrafit.QProcess::waitForFinished();
        output += procTerrafit.readAllStandardOutput()+"\n\n";
        ui->textBrowser->setText(output);

        // save output to log
        if (ui->checkBox_log->isChecked()){
            QString file        = projDirectory+"/data.txt";
            QDateTime datetime  = QDateTime::currentDateTime();
            QString sDateTime   = datetime.toString("yyyy/MM/dd HH:mm:ss");

            QFile data(file);
            if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
                QTextStream out(&data);
                out << endl;
                out << endl;
                out << sDateTime;
                out << "  -  ";
                out << arguments;
                out << endl;
                out << endl;
                out << sDateTime;
                out << "  -  ";
                out << argumentsTerrafit;
            }

            QString file2 = projDirectory+"/output.txt";
            QFile data2(file2);
            if (data2.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
                QTextStream out(&data2);
                out << endl;
                out << endl;
                out << output;
            }
        }
    }
}

// update shapefiles list for ogr-decode
void MainWindow::on_pushButton_12_clicked()
{
    // move shapefiles to "private" directories
    QDir dir(dataDirectory);
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
    ui->listWidget->clear();
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QFileInfoList dirList = dir.entryInfoList();
    for (int i = 0; i < dirList.size(); ++i) {
        QFileInfo dirInfo = dirList.at(i);
        //QString test = qPrintable(QString("%1").arg(dirInfo.fileName()));
        new QListWidgetItem(tr(qPrintable(QString("%1").arg(dirInfo.fileName()))), ui->listWidget);
    }
}

// run fgfs-construct
void MainWindow::on_pushButton_13_clicked()
{
    QString lat = ui->lineEdit_31->text();
    QString lon = ui->lineEdit_32->text();
    QString x = ui->lineEdit_33->text();
    QString y = ui->lineEdit_34->text();
    QString selectedMaterials;

    // create string with selected terraintypes
    for (int i = 0; i < ui->listWidget_2->count(); ++i){
        if (ui->listWidget_2->item(i)->isSelected() == 1){
            selectedMaterials += ui->listWidget_2->item(i)->text()+" ";
        }
    }

    // construct fgfs-construct commandline
    QString arguments = terragearDirectory+"/fgfs-construct.exe ";
    arguments += "--work-dir="+workDirectory+" ";
    arguments += "--output-dir="+outpDirectory+"/Terrain ";
    if (ui->lineEdit_35->text() > 0){
        arguments += "--tile-id="+ui->lineEdit_35->text();
    }
    arguments += "--lon="+lon+" --lat="+lat+" ";
    arguments += "--xdist="+x+" --ydist="+y+" ";
    if (ui->checkBox_3->isChecked()){
        arguments += "--useUKgrid ";
    }
    if (ui->checkBox_4->isChecked()){
        arguments += "--ignore-landmass ";
    }
    arguments += selectedMaterials;

    // display commandline
    QMessageBox::about(this, tr("Command line"),arguments);

    // output commandline to data.txt
    if (ui->checkBox_log->isChecked()){
        QString file = projDirectory+"/data.txt";
        QFile data(file);
        if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
            QTextStream out(&data);
            out << endl;
            out << endl;
            out << arguments;
        }
    }

    // start command
    QProcess proc;
    proc.setWorkingDirectory(terragearDirectory);
    proc.start(arguments, QIODevice::ReadWrite);

    // wait for process to finish, before allowing the next action
    proc.waitForReadyRead();
    proc.QProcess::waitForFinished();
    output += proc.readAllStandardOutput()+"\n\n";
    ui->textBrowser->setText(output);
}

// calculate scenery area center and radii
void MainWindow::on_pushButton_14_clicked()
{
    QString east    = ui->lineEdit_28->text();
    QString north   = ui->lineEdit_29->text();
    QString south   = ui->lineEdit_30->text();
    QString west    = ui->lineEdit_27->text();

    double eastInt     = east.toDouble();
    double northInt    = north.toDouble();
    double southInt    = south.toDouble();
    double westInt     = west.toDouble();

    QPalette p;

    if (westInt < eastInt and northInt > southInt){
        double latInt      = (northInt + southInt)/2;
        double lonInt      = (eastInt + westInt)/2;

        QString lat     = QString::number(latInt);
        QString lon     = QString::number(lonInt);
        QString xRad     = QString::number(eastInt-lonInt);
        QString yRad     = QString::number(northInt-latInt);

        ui->lineEdit_31->setText(lat);
        ui->lineEdit_32->setText(lon);
        ui->lineEdit_33->setText(xRad);
        ui->lineEdit_34->setText(yRad);

        p.setColor(QPalette::WindowText, Qt::black);
    }
    else{
        QMessageBox::about(this,tr("Error"),tr("Wrong boundaries"));
        p.setColor(QPalette::WindowText, Qt::red);
    }

    // change label colors on error
    ui->label_35->setPalette(p);
    ui->label_36->setPalette(p);
    ui->label_37->setPalette(p);
    ui->label_38->setPalette(p);
}

// update terraintypes list for fgfs-construct
void MainWindow::on_pushButton_15_clicked()
{
    ui->listWidget_2->clear();
    QDir dir(workDirectory);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        QString test = qPrintable(QString("%1").arg(fileInfo.fileName()));
        new QListWidgetItem(tr(qPrintable(QString("%1").arg(fileInfo.fileName()))), ui->listWidget_2);
    }
}

// run ogr-decode
void MainWindow::on_pushButton_16_clicked()
{
    // check whether both lists have equal length
    int shapefilesLength = ui->listWidget->count();
    int materialsLength  = ui->listWidget_3->count();

    if (shapefilesLength == materialsLength)
    {

        // construct ogr-decode command, for each single shapefile
        for (int i = 0; i < shapefilesLength; ++i)
        {
            QString material    = ui->listWidget->item(i)->text();
            QString shapefile   = ui->listWidget_3->item(i)->text();
            QString arguments   = terragearDirectory+"/ogr-decode.exe ";
            if (ui->lineEdit_25->text() == 0){
                arguments += "--line-width 10 ";
            }
            else{
                arguments += "--line-width "+ui->lineEdit_25->text()+" ";
            }
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
            arguments += workDirectory+"/"+shapefile+" ";
            arguments += dataDirectory+"/"+material;
            QProcess proc;
            proc.start(arguments, QIODevice::ReadWrite);

            // run command
            proc.waitForReadyRead();
            proc.QProcess::waitForFinished();
            output += proc.readAllStandardOutput();
            ui->textBrowser->setText(output);

            // save commandline to log
            if (ui->checkBox_log->isChecked()){
                QString file        = projDirectory+"/data.txt";
                QDateTime datetime  = QDateTime::currentDateTime();
                QString sDateTime   = datetime.toString("yyyy/MM/dd HH:mm:ss");

                QFile data(file);
                if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
                    QTextStream out(&data);
                    out << endl;
                    out << endl;
                    out << sDateTime;
                    out << "  -  ";
                    out << arguments;
                }
            }
        }
    }
    else{
        QMessageBox::about(this, tr("Error"),"You did not specify an equal number of shapefiles and materials.");
    }
}

// add material
void MainWindow::on_pushButton_17_clicked()
{
    // check to make sure there is an equal number of materials and shapefiles
    int shapefilesLength    = ui->listWidget->count();
    int materialsLength     = ui->listWidget_3->count();
    if (shapefilesLength > materialsLength)
    {
        QString addMaterial = ui->comboBox_2->currentText();
        new QListWidgetItem(tr(qPrintable(addMaterial)), ui->listWidget_3);
    }
}

// functions //

// update elevation download range
void MainWindow::updateElevationRange()
{
    int eastInt     = ui->lineEdit_5->text().toInt();
    int northInt    = ui->lineEdit_7->text().toInt();
    int southInt    = ui->lineEdit_8->text().toInt();
    int westInt     = ui->lineEdit_6->text().toInt();

    QString east;
    QString north;
    QString south;
    QString west;
    east.sprintf("%03d", eastInt);
    north.sprintf("%02d", northInt);
    south.sprintf("%02d", southInt);
    west.sprintf("%03d", westInt);

    QString minElev = "";
    QString maxElev = "";
    if (northInt >= 0){
        maxElev += "N";
    }
    if (northInt < 0) {
        maxElev += "S";
    }
    maxElev += north;

    if (southInt >= 0){
        minElev += "N";
    }
    if (southInt < 0) {
        minElev += "S";
    }
    minElev += south;

    if (eastInt > 0){
        maxElev += "W";
    }
    if (eastInt <= 0) {
        maxElev += "E";
    }
    maxElev += east;

    if (westInt > 0){
        minElev += "W";
    }
    if (westInt <= 0) {
        minElev += "E";
    }
    minElev += west;

    ui->lineEdit_9->setText(minElev);
    ui->lineEdit_10->setText(maxElev);
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
                            materialList.append(material);
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
