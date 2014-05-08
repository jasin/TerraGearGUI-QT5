/* menu.cpp
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


//== Open

void MainWindow::on_openProjectAction_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Load project settings"), projectDirectory,
                                                    tr("TerraGear project (*.xml)"));
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    QXmlStreamReader xml(&file);
    QString sub = "path";

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement()) {
            xml.readNext();
        } else if (xml.isStartElement()) {
            QString variable = xml.name().toString();
            if (sub == "path" && xml.name().toString() == "boundaries")
                sub = "boundaries";
            if (sub == "boundaries" && xml.name().toString() == "check")
                sub = "check";
            xml.readNext();
            settings.setValue(sub+"/"+variable, xml.text().toString());
            qDebug() << settings.value(sub+"/"+variable);
        }
    }
    if (xml.hasError())
    {
        qDebug() << xml.error();
    }
    file.close();

    loadSettings();
}

//== Save
void MainWindow::on_saveProjectAction_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save project settings"), projectDirectory,
                                                    tr("TerraGear project (*.xml)"));
    if (fileName.isEmpty())
        return;
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     file.errorString());
            return;
        }        
        QXmlStreamWriter xmlWriter(&file);
        xmlWriter.setAutoFormatting(true);
        xmlWriter.writeStartDocument();
        xmlWriter.writeStartElement("settings");
            xmlWriter.writeStartElement("paths");
                xmlWriter.writeTextElement("project", settings.value("/paths/project").toString() );
                xmlWriter.writeTextElement("terragear", settings.value("/paths/terragear").toString() );
                xmlWriter.writeTextElement("flightgear", settings.value("/paths/flightgear").toString() );
                xmlWriter.writeTextElement("elevationdir", settings.value("/paths/elevationDir").toString() );
                xmlWriter.writeTextElement("airportfile", settings.value("/paths/airportFile").toString() );
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("boundaries");
                xmlWriter.writeTextElement("north", settings.value("/boundaries/north").toString() );
                xmlWriter.writeTextElement("south", settings.value("/boundaries/south").toString() );
                xmlWriter.writeTextElement("east", settings.value("/boundaries/east").toString() );
                xmlWriter.writeTextElement("west", settings.value("/boundaries/west").toString() );
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("check");
                xmlWriter.writeTextElement("ignore_landmass", settings.value("/check/ignore_landmass").toString() );
                xmlWriter.writeTextElement("no_data", settings.value("/check/no_data").toString() );
            xmlWriter.writeEndElement();
        xmlWriter.writeEndElement();
        xmlWriter.writeEndDocument();
        file.close();
    }
}

//== Close Window
void MainWindow::on_quitAction_triggered()
{
    MainWindow::close();
    QApplication::quit(); //= Lets get outta here...
}

//== About dialog
void MainWindow::on_aboutAction_triggered()
{
    QMessageBox::information(this, tr("TerraGUI v0.9.17"),tr("©2010-2013 Gijs de Rooy et al. for FlightGear\nGNU General Public License version 2"));
}

//= Show wiki article in a browser
void MainWindow::on_wikiAction_triggered()
{
    //= TODO make a constant
    QString url = "http://wiki.flightgear.org/TerraGear_GUI";
    QUrl qu(url);
    if ( ! QDesktopServices::openUrl(qu) ) {
        QMessageBox::critical(this,"URL cannot be opened","The following URL cannot be opened "+url+".\nCopy the URL to your browser");
    }
}

void MainWindow::loadSettings()
{
    projectDirectory = settings.value("paths/project").toString();
    terragearDirectory = settings.value("paths/terragear").toString();
    flightgearDirectory = settings.value("paths/flightgear").toString();
    elevationDirectory = settings.value("paths/elevationdir").toString();
    airportFile = settings.value("paths/airportfile").toString();

    // set project's directories
    dataDirectory = projectDirectory+"/data";
    outpDirectory = projectDirectory+"/output";
    workDirectory = projectDirectory+"/work";

    ui->projectDirectoryField->setText(projectDirectory);
    ui->terragearRootField->setText(terragearDirectory);
    ui->flightgearRootField->setText(flightgearDirectory);
    ui->elevationDirectoryField->setText(elevationDirectory);
    ui->aptFileField->setText(airportFile);

    m_north = settings.value("boundaries/north").toString();
    m_south = settings.value("boundaries/south").toString();
    m_west  = settings.value("boundaries/west").toString();
    m_east  = settings.value("boundaries/east").toString();

    ui->minLatField->setText(m_south);
    ui->maxLatField->setText(m_north);
    ui->minLonField->setText(m_west);
    ui->maxLonField->setText(m_east);

    updateElevationRange();
    updateCenter();

    if (!m_west.isEmpty()) {
        QList<QPointF> coords;
        coords.append(QPointF(m_west.toFloat(), m_north.toFloat()));
        coords.append(QPointF(m_east.toFloat(), m_south.toFloat()));
        mc->setViewAndZoomIn(coords);

        // draw saved boundary box
        QList<Point*> points;
        points.append(new Point(m_west.toFloat(), m_south.toFloat(), "1"));
        points.append(new Point(m_west.toFloat(), m_north.toFloat(), "1"));
        points.append(new Point(m_east.toFloat(), m_north.toFloat(), "1"));
        points.append(new Point(m_east.toFloat(), m_south.toFloat(), "1"));
        points.append(new Point(m_west.toFloat(), m_south.toFloat(), "1"));
        QPen* linepen = new QPen(Qt::red);
        linepen->setWidth(2);
        LineString* ls = new LineString(points, "Area Boundary", linepen);
        mainlayer->clearGeometries();
        mainlayer->addGeometry(ls);
    }
}
