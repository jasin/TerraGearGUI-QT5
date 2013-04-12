/* mainwindow.cpp -- top level construction routines
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QtGlobal>
#include <QIcon>
#include <QIODevice>
#include <QLibrary>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMap>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPaintEvent>
#include <QPalette>
#include <QPoint>
#include <QPointF>
#include <QProcess>
#include <QRectF>
#include <QRegExp>
#include <QSettings>
#include <QStringList>
#include <QTextStream>
#include <QTime>
#include <QUrl>
#include <QWheelEvent>
#include <QXmlStreamReader>

#include "tggui_utils.h"


QString airportFile;
QString elevationDirectory;
QString selectedMaterials;
QString output = "";

QString flightgearDirectory;
QString terragearDirectory;
QString projectDirectory;

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


#include "menu.cpp"
#include "airportsTab.cpp"
#include "downloadManager.cpp"
#include "downloadTab.cpp"
#include "elevationTab.cpp"
#include "constructTab.cpp"
#include "materialsTab.cpp"
#include "startTab.cpp"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle( tr("TerraGear GUI") );

    // create MapControl
    mc = new MapControl(QSize(458, 254));
    mc->setMinimumSize(QSize(458,254));
    mc->showScale(true);
    mapadapter = new OSMMapAdapter();
    mainlayer = new MapLayer("OpenStreetMap-Layer", mapadapter);
    mc->addLayer(mainlayer);
    connect(mc, SIGNAL(boxDragged(QRectF)),
            this, SLOT(draggedRect(QRectF)));
    addZoomButtons();
    mc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->mapLayout-> addWidget(mc);

    // restore variables from previous session
    loadSettings();

    // TAB: Airports
    updateAirportRadios(); // hide the non-selected options

    // TAB: Construct
    ui->shapefilesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->shapefilesTable->setHorizontalHeaderLabels(QStringList() << tr("Shapefile") << tr("Material"));
    ui->shapefilesTable->horizontalHeader()->setResizeMode( QHeaderView::Stretch);
    ui->shapefilesTable->horizontalHeader()->setStyleSheet("font: bold;");
    ui->shapefilesTable->verticalHeader()->hide();

    // create sub-directory variables
    dataDirectory = projectDirectory+"/data";
    outpDirectory = projectDirectory+"/output";
    workDirectory = projectDirectory+"/work";

    // run functions on startup
    if (flightgearDirectory != 0) {
        updateMaterials();
        if (airportFile.size() == 0) {
            // try to find apt.dat.gz file
            QString apfile = flightgearDirectory+"/Airports/apt.dat.gz";
            QFile apf(apfile);
            if (apf.exists()) {
                airportFile = apfile;
                settings.setValue("paths/airportFile", airportFile); // keep the airport file found
            }
        }
        if (airportFile.size())
            ui->aptFileField->setText(airportFile);
    }

    // re-apply the check boxes (for construct)
    bool ign_lm = settings.value("check/ignore_landmass").toBool();
    ui->ignoreLandmassCB->setCheckState(ign_lm ? Qt::Checked : Qt::Unchecked);

    // Network manager
    _manager = new QNetworkAccessManager(this);
    connect(_manager, SIGNAL(finished(QNetworkReply*)), SLOT(downloadFinished(QNetworkReply*)));

    m_break = false;

    if (ui->tabWidget->currentIndex() == 1) {
        ui->textBrowser->hide();
        mc->resize(QSize(458,254));
    }

    // add context menu to table
    connect(ui->shapefilesTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(displayMenu(QPoint)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mc;
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    // hide log on download tab
    if (index == 1) {
        ui->textBrowser->hide();
        mc->resize(ui->frame->size());
    } else {
        if ( ui->showOutputOnScreenCB->isChecked() ) {
            ui->textBrowser->show();
        }
        else {
            ui->textBrowser->hide();
        }
    }
    // add shapefiles to list, if empty
    if (index == 4 and ui->shapefilesTable->rowCount() == 0) {
        ui->retrieveShapefilesButton->click();
    }
    // add terraintypes to list, if empty
    if (index == 5 and ui->terrainTypesList->count() == 0) {
        ui->updateTerrainTypeButton->click();
    }
}

void MainWindow::on_tabWidget_selected(QString )
{
    show();
}

void MainWindow::GUILog(QString s, QString tools = "default")
{

  if ( ui->createLogFileCB->isChecked() ) {

    if ( tools == "download" ) {
      QFile data(projectDirectory+"/download.log");
      if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
        QTextStream out(&data);
        out << s;
        out << endl;
      }
    }

    else if ( tools == "hgtchop" ) {
      QFile data(projectDirectory+"/hgtchop.log");
      if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
        QTextStream out(&data);
        out << s;
        out << endl;
      }
    }

    else if ( tools == "terrafit" ) {
      QFile data(projectDirectory+"/terrafit.log");
      if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
        QTextStream out(&data);
        out << s;
        out << endl;
      }
    }

    else if ( tools == "genapt" ) {
      QFile data(projectDirectory+"/genapt.log");
      if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
        QTextStream out(&data);
        out << s;
        out << endl;
      }
    }

    else if ( tools == "ogr-decode" ) {
      QFile data(projectDirectory+"/ogr-decode.log");
      if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
        QTextStream out(&data);
        out << s;
        out << endl;
      }
    }

    else if ( tools == "tg-construct" ) {
      QFile data(projectDirectory+"/tg-construct.log");
      if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
        QTextStream out(&data);
        out << s;
        out << endl;
      }
    }

    else if ( tools == "default" ) {
      QDateTime datetime  = QDateTime::currentDateTime();
      QString sDateTime   = datetime.toString("yyyy/MM/dd HH:mm:ss");

      QFile data(projectDirectory+"/default.log");
      if (data.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
        QTextStream out(&data);
        out << sDateTime;
        out << "  -  ";
        out << s;
        out << endl;
      }
    }

    else {
      qDebug() << "GUILog : " << s;
    }
  }
}

// resize the widget
void MainWindow::resizeEvent ( QResizeEvent * event )
{
    mc->resize(ui->frame->size());
}

void MainWindow::displayMenu(const QPoint &pos)
{
    QMenu menu(this);
    QAction *u = menu.addAction("Remove"); // there can be more than one
    QAction *a = menu.exec(ui->shapefilesTable->viewport()->mapToGlobal(pos));
    if (a == u)
    {
        // do what you want or call another function
        while(ui->shapefilesTable->selectedItems().count() > 0)
            ui->shapefilesTable->removeRow(ui->shapefilesTable->currentRow());
    }
}

// eof - mainwindow.cpp
