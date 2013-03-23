/* startTab.cpp
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

void MainWindow::on_viewOnlineManualButton_clicked()
{
    QString url = "http://wiki.flightgear.org/TerraGear_GUI";
    QUrl qu(url);
    if ( ! QDesktopServices::openUrl(qu) ) {
        QMessageBox::critical(this,"URL cannot be opened","The following URL cannot be opened "+url+".\nCopy the URL to your browser");
    }
}


//################################################################//
//################################################################//
//###############    SELECT PROJECT DIRECTORY    #################//
//################################################################//
//################################################################//

void MainWindow::on_projectDirectoryButton_clicked()
{
    projectDirectory = QFileDialog::getExistingDirectory(this,
                                                      tr("Select the project's location, everything that is used and created during the scenery generating process is stored in this location."));
    ui->projectDirectoryField->setText(projectDirectory);
    settings.setValue("paths/project", projectDirectory);

    // set project's directories
    dataDirectory = projectDirectory+"/data";
    outpDirectory = projectDirectory+"/output";
    workDirectory = projectDirectory+"/work";
}

//== Select FlightGear root
// TODO consolidate per platform settings.. eg fgx::settings
void MainWindow::on_flightgearRootButton_clicked()
{
    flightgearDirectory = QFileDialog::getExistingDirectory(this,
                                               tr("Select the FlightGear root (data directory). This is optional; it is only used to retrieve an up-to-date list of available materials. You can use the GUI without setting the FG root."
                                                  ));
    ui->flightgearRootField->setText(flightgearDirectory);
    settings.setValue("paths/flightgear", flightgearDirectory);

    updateMaterials();
}

// select TerraGear directory
void MainWindow::on_terragearRootButton_clicked()
{
    terragearDirectory = QFileDialog::getExistingDirectory(
                this,
                tr("Select TerraGear root, this is the directory in which bin/ and share/ live."
                   ));
    if (terragearDirectory.endsWith("bin")) {
        QMessageBox::information(this,"Directory error","You should select the root directory, in which bin/ and share/ live.\nDo NOT select the bin/ directory.");
        ui->terragearRootButton->click();
    }
    ui->terragearRootField->setText(terragearDirectory);
    settings.setValue("paths/terragear", terragearDirectory);

    // disable tabs if terragear path is not set
    int enabled = 0;
    if (terragearDirectory != "") {
        enabled = 1;
    }
    ui->tabWidget->setTabEnabled(1,enabled);
    ui->tabWidget->setTabEnabled(2,enabled);
    ui->tabWidget->setTabEnabled(3,enabled);
    ui->tabWidget->setTabEnabled(4,enabled);
}

// show/hide output field
void MainWindow::on_showOutputOnScreenCB_clicked()
{
    if ( ui->showOutputOnScreenCB->isChecked() ) {
        ui->textBrowser->show();
    }
    else {
        ui->textBrowser->hide();
    }
}
