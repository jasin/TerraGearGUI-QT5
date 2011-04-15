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

    ui->tblShapesAlign->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblShapesAlign->setHorizontalHeaderLabels(QStringList() << tr("Shapefile") << tr("Material"));
    ui->tblShapesAlign->horizontalHeader()->setResizeMode( QHeaderView::Stretch);
    ui->tblShapesAlign->horizontalHeader()->setStyleSheet("font: bold;");
    ui->tblShapesAlign->verticalHeader()->hide();

    dataDirectory = projDirectory+"/data";
    outpDirectory = projDirectory+"/output";
    workDirectory = projDirectory+"/work";

    // run functions on startup
    if (fgRoot != 0){
        updateMaterials();
    }
    updateElevationRange();
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

void MainWindow::on_lineEdit_27_editingFinished()
{
    updateCenter();
}

void MainWindow::on_lineEdit_28_editingFinished()
{
    updateCenter();
}

void MainWindow::on_lineEdit_29_editingFinished()
{
    updateCenter();
}

void MainWindow::on_lineEdit_30_editingFinished()
{
    updateCenter();
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

    if (westInt < eastInt and northInt > southInt){
        QString mapserverUrl = "http://mapserver.flightgear.org/dlcs?xmin="+west+"&xmax="+east+"&ymin="+south+"&ymax="+north;
        QDesktopServices::openUrl(mapserverUrl);
        // save output to log
        if (ui->checkBox_log->isChecked()){
            QDateTime datetime  = QDateTime::currentDateTime();
            QString sDateTime   = datetime.toString("yyyy/MM/dd HH:mm:ss");

            QFile data(projDirectory+"/log.txt");
            if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
                QTextStream out(&data);
                out << endl;
                out << sDateTime;
                out << "  -  ";
                out << mapserverUrl;
            }
        }
    }
    else{
        QMessageBox::about(this,tr("Error"),tr("Wrong boundaries"));
    }

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
    QString tileId  = ui->lineEdit_13->text();

    QString minLat  = ui->lineEdit_16->text();
    QString maxLat  = ui->lineEdit_15->text();
    QString minLon  = ui->lineEdit_12->text();
    QString maxLon  = ui->lineEdit_14->text();
    QString maxSlope  = ui->lineEdit_21->text();

    QString arguments   = terragearDirectory+"/genapts --input="+airportFile+" --work="+workDirectory+" ";
    if (airportId > 0){
        arguments += "--airport="+airportId+" ";
    }
    if (startAptId > 0){
        arguments += "--start-id="+startAptId+" ";
    }
    if (maxLat != 0 or maxLon != 0 or minLat != 0 or minLon != 0){
        arguments += "--min-lon="+minLon+" ";
        arguments += "--max-lon="+maxLon+" ";
        arguments += "--min-lat="+minLat+" ";
        arguments += "--max-lat="+maxLat+" ";
    }
    if (maxSlope != 0){
        arguments += "--max-slope="+maxSlope+" ";
    }
    if (tileId != 0){
        arguments += "--tile="+tileId+" ";
    }

    // save output to log
    if (ui->checkBox_log->isChecked()){
        QDateTime datetime  = QDateTime::currentDateTime();
        QString sDateTime   = datetime.toString("yyyy/MM/dd HH:mm:ss");

        QFile data(projDirectory+"/log.txt");
        if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
            QTextStream out(&data);
            out << endl;
            out << sDateTime;
            out << "  -  ";
            out << arguments;
        }
    }

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
        QString arguments = terragearDirectory+"/hgtchop "+elevationRes+" "+elevationDirectory+"/"+elevationFile+" "+workDirectory+"/SRTM-30";

        QProcess proc;
        proc.start(arguments, QIODevice::ReadWrite);

        // run hgtchop command
        proc.waitForReadyRead();
        proc.QProcess::waitForFinished();
        output += proc.readAllStandardOutput()+"\n\n";
        ui->textBrowser->setText(output);

        // generate and run terrafit command
        QString argumentsTerrafit = terragearDirectory+"/terrafit "+workDirectory+"/SRTM-30";

        QProcess procTerrafit;
        procTerrafit.start(argumentsTerrafit, QIODevice::ReadWrite);
        procTerrafit.waitForReadyRead();
        procTerrafit.QProcess::waitForFinished();
        output += procTerrafit.readAllStandardOutput()+"\n\n";
        ui->textBrowser->setText(output);

        // save output to log
        if (ui->checkBox_log->isChecked()){
            QDateTime datetime  = QDateTime::currentDateTime();
            QString sDateTime   = datetime.toString("yyyy/MM/dd HH:mm:ss");

            QFile data(projDirectory+"/log.txt");
            if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
                QTextStream out(&data);
                out << endl;
                out << sDateTime;
                out << "  -  ";
                out << arguments;
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
    while (ui->tblShapesAlign->rowCount() != 0)
    {
        ui->tblShapesAlign->removeRow(0);
    }

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
    }
    ui->tblShapesAlign->resizeRowsToContents();
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
    QString arguments = terragearDirectory+"/fgfs-construct ";
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

    // output commandline to log.txt
    if (ui->checkBox_log->isChecked()){
        QDateTime datetime  = QDateTime::currentDateTime();
        QString sDateTime   = datetime.toString("yyyy/MM/dd HH:mm:ss");

        QFile data(projDirectory+"/log.txt");
        if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
            QTextStream out(&data);
            out << endl;
            out << sDateTime;
            out << "  -  ";
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
    for (int i = 0; i < ui->tblShapesAlign->rowCount(); i++)
    {
        QString material    = ui->tblShapesAlign->item(i, 0)->text();
        // skip if material are not assigned
        if ((ui->tblShapesAlign->item(i, 1) == 0) || (ui->tblShapesAlign->item(i, 1)->text().length() == 0)) continue;

        QString shapefile   = ui->tblShapesAlign->item(i, 1)->text();
        QString arguments   = terragearDirectory+"/ogr-decode ";
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
            QDateTime datetime  = QDateTime::currentDateTime();
            QString sDateTime   = datetime.toString("yyyy/MM/dd HH:mm:ss");

            QFile data(projDirectory+"/log.txt");
            if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
                QTextStream out(&data);
                out << endl;
                out << sDateTime;
                out << "  -  ";
                out << arguments;
            }
        }
    }
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

    // check if boundaries are valid
    if (westDbl < eastDbl and northDbl > southDbl){

        // use absolute degrees for elevation ranges
        east.sprintf("%03d", abs(eastDbl));
        north.sprintf("%02d", abs(northDbl));
        south.sprintf("%02d", abs(southDbl));
        west.sprintf("%03d", abs(westDbl));

        QString minElev = "";
        QString maxElev = "";

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

        // output elevation range
        ui->lineEdit_9->setText(minElev);
        ui->lineEdit_10->setText(maxElev);
    }

    // if boundaries are not valid: do not display elevation range and set text color to red
    else{
        ui->lineEdit_9->setText("");
        ui->lineEdit_10->setText("");

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
                            // ignore materials already present
                            if (materialList.indexOf(material, 0) == -1)
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

// calculate center of scenery area and radii
void MainWindow::updateCenter()
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
        p.setColor(QPalette::WindowText, Qt::red);
    }

    // change label colors on error
    ui->label_28->setPalette(p);
    ui->label_33->setPalette(p);
    ui->label_34->setPalette(p);
    ui->label_47->setPalette(p);
}
