#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_btnScrollTo_clicked();

    void on_btnBottom_clicked();

    void on_btnTop_clicked();

    void on_editItem_textChanged(const QString &arg1);

    void on_editGroup_textEdited(const QString &arg1);

private:
    class ListView* listView;
    class ListViewTestModel* testModel;
    class ListViewEmptyModel* emptyModel;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
