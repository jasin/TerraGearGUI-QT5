#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

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

private slots:
    void on_checkBox_4_toggled(bool checked);
    void on_checkBox_igerr_toggled(bool checked);
    void on_checkBox_minmax_clicked();
    void on_checkBox_nodata_toggled(bool checked);
    void on_checkBox_noovr_toggled(bool checked);
    void on_checkBox_showOutput_clicked();

    void on_lineEdit_5_editingFinished();
    void on_lineEdit_6_editingFinished();
    void on_lineEdit_7_editingFinished();
    void on_lineEdit_8_editingFinished();
    void on_lineEdit_13_textEdited(const QString &arg1);
    void on_lineEdit_18_textEdited(const QString &arg1);
    void on_lineEdit_35_textEdited(const QString &arg1);

    void on_actionQuit_triggered();
    void on_about_triggered();
    void on_wiki_triggered();

    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton_12_clicked();
    void on_pushButton_13_clicked();
    void on_pushButton_15_clicked();
    void on_pushButton_16_clicked();
    void on_pushButton_17_clicked();

    void on_radioButton_clicked();
    void on_radioButton_2_clicked();
    void on_radioButton_3_clicked();
    void on_radioButton_4_clicked();

    void on_tabWidget_selected(QString );
    void on_tabWidget_currentChanged(int index);

    void on_tblShapesAlign_cellDoubleClicked(int row, int column);

    void updateCenter();
    void updateElevationRange();
    void updateMaterials();

    void outputToLog(QString s);
    void outTemp(QString s); /* write to projDirectory+"/templog.txt" *TBD* should restart on app start */
};

#endif // MAINWINDOW_H
