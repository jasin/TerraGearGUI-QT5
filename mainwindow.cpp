#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QIcon>
#include <QIODevice>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QProcess>
#include <QUrl>

QString airportFile;
QString dataDirectory;
QString elevationDirectory;
QString workDirectory;
QString fgfsDirectory;
QString terragearDirectory;
QString outputDirectory;
QString selectedMaterials;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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

void MainWindow::on_tabWidget_currentChanged(QWidget* )
{

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
    qDebug() << proc.readAllStandardOutput();

}

void MainWindow::on_lineEdit_20_editingFinished()
{
    airportFile = ui->lineEdit_20->text();
}

void MainWindow::on_pushButton_2_clicked()
{
    QString east = ui->lineEdit_5->text();
    QString north = ui->lineEdit_7->text();
    QString south = ui->lineEdit_8->text();
    QString west = ui->lineEdit_6->text();

    // check whether boundaries are valid
    if (west > east)
    {
        QMessageBox::about(this, tr("Error"),
                         tr("West cannot be greater than east."));
    }
    if (south > north)
    {
        QMessageBox::about(this, tr("Error"),
                         tr("South cannot be greater than north."));
    }
    if (west < east and north > south)
    {
        QString mapserverUrl = "http://mapserver.flightgear.org/dlcs?xmin="+west+"&xmax="+east+"&ymin="+south+"&ymax="+north;
        qDebug() << mapserverUrl;
        QDesktopServices::openUrl(mapserverUrl);
    }
}

void MainWindow::on_about_triggered()
{
    QMessageBox::about(this, tr("TerraGUI"),
                         tr("©2010 Gijs de Rooy for FlightGear\n" \
                            "GNU General Public License version 3"));
}

void MainWindow::on_wiki_triggered()
{
    QDesktopServices::openUrl(QUrl(tr("http://wiki.flightgear.org/index.php/TerraGear_GUI")));
}

void MainWindow::on_pushButton_8_clicked()
{
    fgfsDirectory = QFileDialog::getExistingDirectory(this,tr("Select FGFS root, this is the directory in which fgfs.exe lives."));
    ui->lineEdit->setText(fgfsDirectory);
}

void MainWindow::on_pushButton_9_clicked()
{
    terragearDirectory = QFileDialog::getExistingDirectory(this,tr("Select TerraGear root, this is the directory in which ogr-decode.exe, genapts.exe etc. live."));
    ui->lineEdit_2->setText(terragearDirectory);
}

void MainWindow::on_pushButton_7_clicked()
{
    workDirectory = QFileDialog::getExistingDirectory(this,tr("Select work location, everything that is created during the scenery generating process is stored in this location."));
    ui->lineEdit_4->setText(workDirectory);
}

void MainWindow::on_pushButton_10_clicked()
{
    outputDirectory = QFileDialog::getExistingDirectory(this,tr("Select output location, this is where the end-scenery will be created."));
    ui->lineEdit_3->setText(outputDirectory);
}

void MainWindow::on_pushButton_12_clicked()
{
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
    QDir dir(workDirectory);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();
         for (int i = 0; i < list.size(); ++i) {
             QFileInfo fileInfo = list.at(i);
             QString test = qPrintable(QString("%1").arg(fileInfo.fileName()));
             new QListWidgetItem(tr(qPrintable(QString("%1").arg(fileInfo.fileName()))), ui->listWidget_2);
         }
}

void MainWindow::on_pushButton_17_clicked()
{
    dataDirectory = QFileDialog::getExistingDirectory(this,tr("Select data location, this will contain all the data used to generate scenery."));
    ui->lineEdit_21->setText(dataDirectory);
}

void MainWindow::on_pushButton_13_clicked()
{
    QString arguments = terragearDirectory+"/fgfs-construct.exe --work="+workDirectory+" --output="+outputDirectory+" --lon=0 --lat=0 --xdist=1 --ydist=1"+selectedMaterials;
    QProcess proc;
    proc.start(arguments, QIODevice::ReadWrite);

    // run command
        proc.waitForReadyRead();
        proc.QProcess::waitForFinished();
        qDebug() << proc.readAllStandardOutput();
}

void MainWindow::on_listWidget_2_itemSelectionChanged()
{

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
             QString arguments = terragearDirectory+"/hgtchop.exe 3 "+elevationFile+" "+workDirectory;
             QProcess proc;
             proc.start(arguments, QIODevice::ReadWrite);

             // run command
                 proc.waitForReadyRead();
                 proc.QProcess::waitForFinished();
                 qDebug() << proc.readAllStandardOutput();
         }
}

void MainWindow::on_pushButton_14_clicked()
{
    QString east = ui->lineEdit_28->text();
    QString west = ui->lineEdit_27->text();
    QString north = ui->lineEdit_29->text();
    QString south = ui->lineEdit_30->text();
}

void MainWindow::on_pushButton_6_clicked()
{
    QProcess proc; // pointer definition
    QString arguments = "C:/Users/AS9423-ULT/Desktop/Terragear/genapts.exe --input=C:/Users/AS9423-ULT/Desktop/Terragear/Airports/TNCM.dat --work=C:/Users/AS9423-ULT/Desktop/";
    proc.start(arguments, QIODevice::ReadWrite); // start program
    proc.waitForReadyRead();
    proc.QProcess::waitForFinished();
    qDebug() << proc.readAllStandardOutput();


}
