#define _CRT_SECURE_NO_WARNINGS // get rid of some unnecessary warnings
#include <QFileDialog>
#include <QMessageBox>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstring>
#include "pmbrowserwindow.h"
#include "ui_pmbrowserwindow.h"

QString myAppName("PM browser");

Q_DECLARE_METATYPE(hkTreeNode*)

static_assert(sizeof(BundleHeader) == 256, "unexpected size of BundleHeader");

constexpr std::time_t EPOCHDIFF_MAC_UNIX = 2082844800;
constexpr double JanFirst1990MACTime = 1580947200.0; // better value?
constexpr auto HIGH_DWORD = 4294967296.0;

std::time_t PMtime2time_t(double t)
{
    t -= JanFirst1990MACTime;
    if (t < 0.0) {
        t += HIGH_DWORD; // why is this necessary?
    }
    return std::time_t(std::floor(t)) - EPOCHDIFF_MAC_UNIX;
}

void PMbrowserWindow::populateTreeView()
{
    auto tree =ui -> treePulse;
    tree->setColumnCount(1);
    auto& pultree=datfile->GetPulTree();
    QList<QTreeWidgetItem *> grpitems;
    int i=0;
    for(auto& groupe : pultree.GetRootNode().Children) {
        QString count=QString("%1").arg(++i), label;
  //        std::cout << grplabel << std::endl;
        QLatin1String labelL1(groupe.getString(GrLabel).c_str());
        label = labelL1;
        QStringList qsl;
        qsl.append(count+" "+QString(label));
        QTreeWidgetItem* grpitem = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), qsl);
        grpitem->setExpanded(true);
        grpitems.append(grpitem);
        int j=0;
        for(auto& series : groupe.Children) {
            QLatin1String labelL1(series.getString(SeLabel).c_str());
            QString label2 = QString("%1").arg(++j)+" "+QString(labelL1);
            auto seriesitem =new QTreeWidgetItem(grpitem,QStringList(label2));
            seriesitem->setExpanded(true);
            int k = 0;
            for(auto& sweep : series.Children) {
                QLatin1String labelL1(sweep.getString(SwLabel).c_str());
                QString label3 = QString("sweep %1").arg(++k)+" "+QString(labelL1);
                auto sweepitem = new QTreeWidgetItem(seriesitem,QStringList(label3));
                sweepitem->setExpanded(true);
                int l = 0;
                for(auto& trace : sweep.Children) {
                    ++l;
                    int32_t datakind = trace.extractUInt16(TrDataKind);
                    QString tracelabel;
                    if(datakind & IsImon) {
                        tracelabel = "Imon";
                    } else if (datakind & IsVmon) {
                        tracelabel = "Vmon";
                    } else {
                        tracelabel = QString("trace %1").arg(l);
                    }
                    auto traceitem = new QTreeWidgetItem(sweepitem,QStringList(tracelabel));
                    traceitem->setData(0,Qt::UserRole, QVariant::fromValue(&trace)); // store pointer to trace for later use
                }
            }
        }
    }
    tree->addTopLevelItems(grpitems);
    tree->expandAll();
}

void PMbrowserWindow::closeFile()
{
    if(datfile) {
        // there is an open file
        delete datfile; datfile = nullptr;
        infile.close();
        ui->treePulse->clear();
        this->setWindowTitle(myAppName);
    }
}
void PMbrowserWindow::loadFile(QString filename)
{
    if(datfile) {
        // there is an open file
        ui->textEdit->append("(closing current file)");
        closeFile();
    }
    infile.open(filename.toStdString(), std::ios::in|std::ios::binary);
    if (!infile) {
        QMessageBox msgBox;
        msgBox.setText(QString("error opening file (") + QString(std::strerror(errno) + QString(")")));
        msgBox.exec();
    }
    datfile = new DatFile;
    try {
        datfile->InitFromStream(infile);
    }
    catch (const std::exception& e) {
        QString msg("error while processing dat file: ");
        msg.append(e.what());
        QMessageBox msgBox;
        msgBox.setText(msg);
        msgBox.exec();
        delete datfile;
        datfile=nullptr;
        infile.close();
    }
    if(datfile) {
        populateTreeView();
    }
}


PMbrowserWindow::PMbrowserWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PMbrowserWindow), currentFile{}, infile{}, datfile{nullptr}
{
    ui->setupUi(this);
    this->setWindowTitle(myAppName);
}

PMbrowserWindow::~PMbrowserWindow()
{
    delete ui;
}


void PMbrowserWindow::on_actionOpen_triggered()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("DAT-file (*.dat)");
    dialog.setViewMode(QFileDialog::Detail);
    if(dialog.exec()) {
        currentFile = dialog.selectedFiles().at(0);
    }
    this->setWindowTitle(myAppName + " - "+currentFile.split("/").back());
    ui->textEdit->append("loading file "+currentFile);
    loadFile(currentFile);
}

void PMbrowserWindow::traceSelected(QTreeWidgetItem* item, hkTreeNode* trace)
{
    // figure out index
    QTreeWidgetItem* sweepitem = item->parent();
    QTreeWidgetItem* seriesitem = sweepitem->parent();
    QTreeWidgetItem* groupitem = seriesitem->parent();
    int indexseries = groupitem->indexOfChild(seriesitem),
            indexsweep = seriesitem->indexOfChild(sweepitem),
            indextrace = sweepitem->indexOfChild(item);
    int indexgroup = ui->treePulse->indexOfTopLevelItem(groupitem);
    QString tracename = QString("tr_%1_%2_%3_%4").arg(indexgroup).arg(indexseries).arg(indexsweep).arg(indextrace);
    ui->textEdit->append(tracename);
    (void)trace;
}

void PMbrowserWindow::on_treePulse_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    (void)previous;
    QVariant v = current->data(0, Qt::UserRole);
    hkTreeNode* node = v.value<hkTreeNode*>();
    if(node) { // this is a trace item
        traceSelected(current, node);
    }
}
