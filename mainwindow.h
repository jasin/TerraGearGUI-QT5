#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void on_pushButton_6_clicked();
    void on_pushButton_14_clicked();
    void on_pushButton_11_clicked();
    void on_listWidget_2_itemSelectionChanged();
    void on_pushButton_13_clicked();
    void on_pushButton_17_clicked();
    void on_pushButton_15_clicked();
    void on_pushButton_12_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_8_clicked();
    void on_wiki_triggered();
    void on_about_triggered();
    void on_pushButton_2_clicked();
    void on_lineEdit_20_editingFinished();
    void on_pushButton_5_clicked();
    void on_tabWidget_currentChanged(QWidget* );
    void on_tabWidget_selected(QString );
    void on_pushButton_3_clicked();
    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
