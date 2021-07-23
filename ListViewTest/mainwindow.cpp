#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ListView/listview.h"
#include "ListView/listviewitem.h"
#include <QPainter>

class ListViewTestModel : public QObject, public ListDataModel, public ListViewDelegate
{
public:
    ListViewTestModel(QObject* parent) : QObject(parent) {}
    // ListViewDelegate interface
public:
    int heightForIndex(const ListIndex &, int availableWidth) override
    {
        return availableWidth / 4;
    }
    const QMetaObject *viewMetaObjectForIndex(const ListIndex &index) override
    {
        return &ListViewItem::staticMetaObject;
    }
    void prepareItemView(const ListIndex &index, ListViewItem *view) override
    {
    }
    void cleanItemView(const ListIndex &index, ListViewItem *view) override
    {
    }
    bool canSelectItem(const ListIndex &index) override
    {
        return true;
    }
    bool isMultipleSelection() override
    {
        return false;
    }
    bool canItemHeightAffectedByWidth() override
    {
        return true;
    }
    QWidget *headerViewForGroup(int) override
    {
        auto w = new QWidget();
        w->resize(200, 200);
        auto palette = w->palette();
        palette.setColor(QPalette::Window, Qt::red);
        w->setPalette(palette);
        w->setAutoFillBackground(true);
        return nullptr;
    }
    QWidget *emptyView() override
    {
        auto w = new QWidget();
        w->resize(200, 200);
        auto palette = w->palette();
        palette.setColor(QPalette::Window, Qt::blue);
        w->setPalette(palette);
        w->setAutoFillBackground(true);
        return w;
    }

    // ListDataModel interface
public:
    int numGroups() override
    {
        return 100;
    }
    int numItemsInGroup(int group) override
    {
        return 100;
    }

protected:
    void *dataForIndex(const ListIndex &index) override
    {
        return nullptr;
    }
    void onRequestMoreLeadingData() override
    {
    }
    void onRequestMoreTailingData() override
    {
    }
};

class ListViewEmptyModel : public QObject, public ListDataModel
{
public:
    using QObject::QObject;

    int numGroups() override
    {
        return 0;
    }
    int numItemsInGroup(int) override
    {
        return 0;
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    testModel = new ListViewTestModel(this);
    emptyModel = new ListViewEmptyModel(this);

    listView = new ListView(nullptr);
    listView->resize(300, 500);
    listView->setDataModel(testModel);
    listView->setViewDelegate(testModel);
    listView->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    if (listView->dataModel() == testModel)
    {
        listView->setDataModel(emptyModel);
    }
    else
    {
        listView->setDataModel(testModel);
    }
}
