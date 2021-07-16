#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ListView/listview.h"
#include <QPainter>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    auto listView = new ListView(nullptr);
    listView->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

