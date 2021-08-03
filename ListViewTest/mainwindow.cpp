#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ListView/listview.h"
#include "ListView/listviewitem.h"
#include <QPainter>

class ListViewTestModel : public QObject, public ListDataModel, public ListViewDelegate
{
public:
    ~ListViewTestModel()
    {

    }
    ListViewTestModel(QObject* parent) : QObject(parent)
    {
        heights.resize(10);
        for (auto& hs : heights)
        {
            hs.resize(100);
            for (auto& h : hs)
            {
                h = 72;
            }
        }
    }

    void setHeight(const ListIndex &index, int height)
    {
        heights[index.group][index.item] = height;
        itemUpdated(index);
    }

    void insertItems(int group, int item, int count, int height)
    {
        std::vector<int> insertedItems(count, height);
        auto& groupHeights = heights[group];
        beginInsertItem(ListIndex(group, item), count);
        groupHeights.insert(groupHeights.begin() + item, insertedItems.begin(), insertedItems.end());
        endInsertItem();
    }
    void removeItems(int group, int item, int count)
    {
        auto& groupHeights = heights[group];
        beginRemoveItem(ListIndex(group, item), count);
        groupHeights.erase(groupHeights.begin() + item, groupHeights.begin() + item + count);
        endRemoveItem();
    }
    void insertGroup(int group, int count, int height)
    {
        std::vector<int> insertedItems(count, height);
        beginInsertGroup(group);
        heights.insert(heights.begin() + group, insertedItems);
        endInsertGroup();
    }
    void removeGroup(int group)
    {
        beginRemoveGroup(group);
        heights.erase(heights.begin() + group);
        endRemoveGroup();
    }

    // ListViewDelegate interface
public:
    int heightForIndex(const ListIndex &index, int ) override
    {
        return heights[index.group][index.item];
    }
    const QMetaObject *viewMetaObjectForIndex(const ListIndex &) override
    {
        return &ListViewItem::staticMetaObject;
    }
    void prepareItemView(const ListIndex &, ListViewItem *) override
    {
    }
    void cleanItemView(const ListIndex &, ListViewItem *) override
    {
    }
    bool canSelectItem(const ListIndex &) override
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
        return w;
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
        return heights.size();
    }
    int numItemsInGroup(int group) override
    {
        return heights[group].size();
    }


protected:
    void *dataForIndex(const ListIndex &) override
    {
        return nullptr;
    }
    void onRequestMoreLeadingData() override
    {
    }
    void onRequestMoreTailingData() override
    {
    }
    std::vector<std::vector<int>> heights;
};

class ListViewEmptyModel : public QObject, public ListDataModel
{
public:
    using QObject::QObject;
    ~ListViewEmptyModel()
    {

    }

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
    delete listView;
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

void MainWindow::on_btnScrollTo_clicked()
{
    auto group = ui->editGroup->text().toInt();
    auto item = ui->editItem->text().toInt();
    listView->scrollToItem(ListIndex(group, item));
}

void MainWindow::on_btnBottom_clicked()
{
    listView->scrollToBottom();
}

void MainWindow::on_btnTop_clicked()
{
    listView->scrollToTop();
}

void MainWindow::on_editItem_textChanged(const QString &)
{
    adjustButtonsText();
}

void MainWindow::on_editGroup_textChanged(const QString &)
{
    adjustButtonsText();
}

void MainWindow::on_editHeight_textChanged(const QString &)
{
    adjustButtonsText();
}

void MainWindow::on_editCount_textChanged(const QString &)
{
    adjustButtonsText();
}

void MainWindow::on_btnChangeHeight_clicked()
{
    auto group = ui->editGroup->text().toInt();
    auto item = ui->editItem->text().toInt();
    auto height = ui->editHeight->text().toInt();
    testModel->setHeight(ListIndex(group, item), height);
}

void MainWindow::on_btnInsertItem_clicked()
{
    auto group = ui->editGroup->text().toInt();
    auto item = ui->editItem->text().toInt();
    auto height = ui->editHeight->text().toInt();
    auto count = ui->editCount->text().toInt();
    testModel->insertItems(group, item, count, height);
}

void MainWindow::on_btnRemoveItem_clicked()
{
    auto group = ui->editGroup->text().toInt();
    auto item = ui->editItem->text().toInt();
    auto count = ui->editCount->text().toInt();
    testModel->removeItems(group, item, count);
}

void MainWindow::on_btnInsertGroup_clicked()
{
    auto group = ui->editGroup->text().toInt();
    auto height = ui->editHeight->text().toInt();
    auto count = ui->editCount->text().toInt();
    testModel->insertGroup(group, count, height);
}

void MainWindow::on_btnRemoveGroup_clicked()
{
    auto group = ui->editGroup->text().toInt();
    testModel->removeGroup(group);
}

void MainWindow::adjustButtonsText()
{
    auto group = ui->editGroup->text().toInt();
    auto item = ui->editItem->text().toInt();
    auto height = ui->editHeight->text().toInt();
    auto count = ui->editCount->text().toInt();
    ui->btnScrollTo->setText(QString::asprintf("Scroll To (%d,%d)", group, item));
    ui->btnChangeHeight->setText(QString::asprintf("Change Height To %d", height));
    ui->btnInsertItem->setText(QString::asprintf("Insert %d Item(s) At (%d,%d) Heights %d)", count, group, item, height));
    ui->btnRemoveItem->setText(QString::asprintf("Remove %d Item(s) At (%d,%d))", count, group, item));
    ui->btnInsertGroup->setText(QString::asprintf("Insert Group At %d With %d Items Heights %d)", group, count, height));
    ui->btnRemoveGroup->setText(QString::asprintf("Remove Group At %d", group));
}
