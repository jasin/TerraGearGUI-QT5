#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QtGui>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QScrollBar>
#include <QUrl>

#include "QMapControl/qmapcontrol.h"

using namespace qmapcontrol;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager* _manager;
    QScrollBar* scrollBar;

    MapControl* mc;
    MapAdapter* mapadapter;
    Layer* mainlayer;

    void addZoomButtons();
    void wheelEvent(QWheelEvent *event);

private slots:
    void on_ignoreLandmassCB_toggled(bool checked);
    void on_showOutputOnScreenCB_clicked();

    void on_maxLonField_editingFinished();
    void on_minLonField_editingFinished();
    void on_maxLatField_editingFinished();
    void on_minLatField_editingFinished();
    void on_tileIdField_textEdited(const QString &arg1);
    void on_airportIcaoField_textEdited(const QString &arg1);
    void on_constructTileIdField_textEdited(const QString &arg1);

    void on_openProjectAction_triggered();
    void on_quitAction_triggered();
    void on_saveProjectAction_triggered();
    void on_aboutAction_triggered();
    void on_wikiAction_triggered();

    void loadSettings();

    void on_aptFileButton_clicked();
    void on_downloadShapefilesButton_clicked();
    void on_elevationDirectoryButton_clicked();
    void on_viewOnlineManualButton_clicked();
    void on_generateAirportsButton_clicked();
    void on_downloadElevationButton_clicked();
    void on_projectDirectoryButton_clicked();
    void on_flightgearRootButton_clicked();
    void on_terragearRootButton_clicked();
    void on_convertElevationButton_clicked();
    void on_retrieveShapefilesButton_clicked();
    void on_generateSceneryButton_clicked();
    void on_updateTerrainTypeButton_clicked();
    void on_decodeShapefilesButton_clicked();
    void on_addMaterialButton_clicked();
    void on_altDefaultPropertiesButton_clicked();

    void on_allAirportsInFileRadio_clicked();
    void on_singleAirportRadio_clicked();
    void on_allAirportInAreaRadio_clicked();
    void on_allAirportsInTileRadio_clicked();

    void on_tabWidget_selected(QString );
    void on_tabWidget_currentChanged(int index);

    void displayMenu(const QPoint &pos);

    void updateAirportRadios();
    void updateArea();
    void switchPoint( double& a, double& b );
    void updateCenter();
    void updateElevationRange();
    void updateMaterials();

    // tools: download, hgtchop, terrafit, genapt, ogr-decode, tg-construct, default
    void GUILog(QString s, QString tools);

    void downloadFinished(QNetworkReply *reply);
    void downloadShapefilesProgressBar(qint64 bytesReceived, qint64 bytesTotal);

    void draggedRect(QRectF rect);

protected:
    virtual void resizeEvent ( QResizeEvent * event );
};

#endif // MAINWINDOW_H
