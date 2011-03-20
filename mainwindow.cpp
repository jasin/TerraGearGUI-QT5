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
#include <QTextStream>
#include <QUrl>

QString airportFile;
QString elevationDirectory;
QString selectedMaterials;
QString output;

QString fgfsDirectory;
QString terragearDirectory;

QString projDirectory;
QString dataDirectory;
QString outpDirectory;
QString workDirectory;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle( tr("TerraGear GUI") );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_3_clicked()
{
    elevationDirectory = QFileDialog::getExistingDirectory(this,tr("Select the elevation directory, this is the directory in which the .hgt files live."));
    ui->lineEdit_11->setText(elevationDirectory);
}

void MainWindow::on_tabWidget_selected(QString )
{
    show();
}

void MainWindow::on_pushButton_clicked()
{
    airportFile = QFileDialog::getOpenFileName(this,tr("Open airport file"), "C:/Users/AS9423-ULT/Desktop/TerraGear/Airports", tr("Airport files (*.dat)"));
    ui->lineEdit_20->setText(airportFile);
}

void MainWindow::on_pushButton_5_clicked()
{
    QString airportId = ui->lineEdit_18->text();
    QString argumentAirportId = "";
    if (airportId.isEmpty()){
        argumentAirportId = "";
    }
    else {
        argumentAirportId = " --airport="+airportId;
    }
    QString arguments = terragearDirectory+"/genapts.exe --input="+airportFile+" --work="+workDirectory+""+argumentAirportId;
    QMessageBox::about(this, tr("Command line"),
                     arguments);
    QProcess proc;
    proc.start(arguments, QIODevice::ReadWrite);

    // run command
    proc.waitForReadyRead();
    proc.QProcess::waitForFinished();
    QString output = proc.readAllStandardOutput();
    ui->textBrowser->setText(output);

}

void MainWindow::on_lineEdit_20_editingFinished()
{
    airportFile = ui->lineEdit_20->text();
}

void MainWindow::on_pushButton_2_clicked()
{
    QString east    = ui->lineEdit_5->text();
    QString north   = ui->lineEdit_7->text();
    QString south   = ui->lineEdit_8->text();
    QString west    = ui->lineEdit_6->text();

    int eastInt     = east.toInt();
    int northInt    = north.toInt();
    int southInt    = south.toInt();
    int westInt     = west.toInt();

    QPalette q;

    if (westInt < eastInt and northInt > southInt){
        q.setColor(QPalette::WindowText, Qt::black);

        QString mapserverUrl = "http://mapserver.flightgear.org/dlcs?xmin="+west+"&xmax="+east+"&ymin="+south+"&ymax="+north;
        QDesktopServices::openUrl(mapserverUrl);
        QDesktopServices::openUrl(QUrl(tr("http://dds.cr.usgs.gov/srtm/version2_1/")));
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

    // set boundarie on FGFS construct page
    ui->lineEdit_27->setText(west);
    ui->lineEdit_28->setText(east);
    ui->lineEdit_29->setText(north);
    ui->lineEdit_30->setText(south);
}

void MainWindow::on_actionQuit_triggered()
{
    MainWindow::close();
}

void MainWindow::on_about_triggered()
{
    QMessageBox::about(this, tr("TerraGUI v1.2"),
                         tr("©2010-2011 Gijs de Rooy for FlightGear\n" \
                            "GNU General Public License version 2"));
}

void MainWindow::on_wiki_triggered()
{
    QDesktopServices::openUrl(QUrl(tr("http://wiki.flightgear.org/index.php/TerraGear_GUI")));
}

void MainWindow::on_pushButton_9_clicked()
{
    terragearDirectory = QFileDialog::getExistingDirectory(this,tr("Select TerraGear root, this is the directory in which ogr-decode.exe, genapts.exe etc. live."));
    ui->lineEdit_2->setText(terragearDirectory);
}

void MainWindow::on_pushButton_7_clicked()
{
    projDirectory = QFileDialog::getExistingDirectory(this,tr("Select the project's location, everything that is used and created during the scenery generating process is stored in this location."));
    ui->lineEdit_4->setText(projDirectory);

    // set project's directories
    dataDirectory = projDirectory+"/data";
    outpDirectory = projDirectory+"/output";
    workDirectory = projDirectory+"/work";

    ui->lineEdit_4->setText(projDirectory);

}

void MainWindow::on_pushButton_12_clicked()
{

    ui->listWidget->clear();
    QDir dir(dataDirectory);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);
            QString test = qPrintable(QString("%1").arg(fileInfo.fileName()));
            new QListWidgetItem(tr(qPrintable(QString("%1").arg(fileInfo.fileName()))), ui->listWidget);
        }
}

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

void MainWindow::on_pushButton_13_clicked()
{
    QString lat = ui->lineEdit_31->text();
    QString lon = ui->lineEdit_32->text();
    QString x = ui->lineEdit_33->text();
    QString y = ui->lineEdit_34->text();
    QString selectedMaterials;

    for (int i = 0; i < ui->listWidget_2->count(); ++i){
        if (ui->listWidget_2->item(i)->isSelected() == 1){
           selectedMaterials += ui->listWidget_2->item(i)->text()+" ";
        }
    }

    QString arguments = terragearDirectory+"/fgfs-construct.exe ";
    arguments += "--work-dir="+workDirectory+" ";
    arguments += "--output-dir="+outpDirectory+" ";
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
    QMessageBox::about(this, tr("Command line"),
                     arguments);

    // output commandline to data.txt
    QString file = projDirectory+"/data.txt";
    QFile data(file);
     if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
         QTextStream out(&data);
         out << endl;
         out << endl;
         out << arguments;
     }

    // start command
    QProcess proc;
    proc.setWorkingDirectory(terragearDirectory);
    proc.start(arguments, QIODevice::ReadWrite);

    // wait for process to finish, before allowing the next action
    proc.waitForReadyRead();
    proc.QProcess::waitForFinished();
    output = proc.readAllStandardOutput();
    ui->textBrowser->setText(output);


}

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
             QMessageBox::about(this, tr("Command line"),
                              arguments);
             QProcess proc;
             proc.start(arguments, QIODevice::ReadWrite);

             // run hgtchop command
             proc.waitForReadyRead();
             proc.QProcess::waitForFinished();
             output = proc.readAllStandardOutput();
             ui->textBrowser->setText(output);

             // run terrafit command

             QString argumentsTerrafit = terragearDirectory+"/terrafit.exe "+workDirectory+"/SRTM-30";
             QMessageBox::about(this, tr("Command line"),
                              argumentsTerrafit);
             QProcess procTerrafit;
             procTerrafit.start(argumentsTerrafit, QIODevice::ReadWrite);
             procTerrafit.waitForReadyRead();
             procTerrafit.QProcess::waitForFinished();
             output = procTerrafit.readAllStandardOutput();
             ui->textBrowser->setText(output);
         }
}

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
        QMessageBox::about(this,tr("Error"),
                             tr("Wrong boundaries"));
        p.setColor(QPalette::WindowText, Qt::red);
    }

    // change label colors on error
    ui->label_35->setPalette(p);
    ui->label_36->setPalette(p);
    ui->label_37->setPalette(p);
    ui->label_38->setPalette(p);

}

void MainWindow::on_pushButton_16_clicked()
{
    int shapefilesLength    = ui->listWidget->count();
    int materialsLength     = ui->listWidget_3->count();

    // check whether both lists have equal length
    if (shapefilesLength == materialsLength)
    {
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
            QMessageBox::about(this, tr("Command line"),
                             arguments);
            QProcess proc;
            proc.start(arguments, QIODevice::ReadWrite);

            // run command
            proc.waitForReadyRead();
            proc.QProcess::waitForFinished();
            output = proc.readAllStandardOutput();
            ui->textBrowser->setText(output);

            QString file        = projDirectory+"/data.txt";
            QDateTime datetime  = QDateTime::currentDateTime();
            QString sDateTime   = datetime.toString("yyyy.MM.dd HH:mm:ss");

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
    else{
        QMessageBox::about(this, tr("Error"),
                         "You did not specify an equal number of shapefiles and materials.");
    }
}

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

void MainWindow::on_listWidget_3_doubleClicked(QModelIndex index)
{
    delete ui->listWidget_3->takeItem(ui->listWidget_3->currentRow());
}

void MainWindow::on_listWidget_doubleClicked(QModelIndex index)
{
    int shapefilesLength    = ui->listWidget->count();
    int materialsLength     = ui->listWidget_3->count();
    if (shapefilesLength <= materialsLength)
    {
        delete ui->listWidget_3->takeItem(ui->listWidget->currentRow());
    }
    delete ui->listWidget->takeItem(ui->listWidget->currentRow());
}
