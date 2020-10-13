/*
    Copyright 2020 Christian R. Halaszovich

     This file is part of PMbrowser.

    PMbrowser is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PMbrowser is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PMbrowser.  If not, see <https://www.gnu.org/licenses/>.
*/

#define _CRT_SECURE_NO_WARNINGS // get rid of some unnecessary warnings
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QDir>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstring>
#include <cmath>
#include "pmbrowserwindow.h"
#include "exportIBW.h"
#include "hkTree.h"
#include "ui_pmbrowserwindow.h"
#include "DlgChoosePathAndPrefix.h"
#include "ui_DlgChoosePathAndPrefix.h"

const QString myAppName("PM browser");
const QString appVersion("1.1");

Q_DECLARE_METATYPE(hkTreeNode*)




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
        label = groupe.getString(GrLabel).c_str();//labelL1;
        QStringList qsl;
        qsl.append(count+" "+label);
        QTreeWidgetItem* grpitem = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), qsl);
        grpitem->setExpanded(true);
        grpitems.append(grpitem);
        int j=0;
        for(auto& series : groupe.Children) {
            QString label2 = QString("%1").arg(++j)+" "+QString(series.getString(SeLabel).c_str());
            auto seriesitem =new QTreeWidgetItem(grpitem,QStringList(label2));
            seriesitem->setExpanded(true);
            int k = 0;
            for(auto& sweep : series.Children) {
                QString label3 = QString("sweep %1").arg(++k)+" "+QString(sweep.getString(SwLabel).c_str());
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
                    }else if(datakind & IsLeak){
                        tracelabel = "Leak";
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
        ui->renderArea->clearTrace();
        ui->treePulse->clear();
        this->setWindowTitle(myAppName);
        delete datfile; datfile = nullptr;
        infile.close();
    }
}
void PMbrowserWindow::loadFile(QString filename)
{
    if(datfile) {
        // there is an open file
        ui->textEdit->append("(closing current file)");
        closeFile();
    }
    ui->textEdit->append("loading file " + filename);
    infile.open(filename.toStdString(), std::ios::in|std::ios::binary);
    if (!infile) {
        QMessageBox::warning(this, QString("File Error"),
            QString("error opening file:\n") + QString(std::strerror(errno)));
    }
    currentFile = filename;
    datfile = new DatFile;
    try {
        datfile->InitFromStream(infile);
    }
    catch (const std::exception& e) {
        QMessageBox::warning(this, QString("File Error"), 
            QString("error while processing dat file:\n") + QString(e.what()));
        delete datfile;
        datfile=nullptr;
        infile.close();
    }
    if(datfile) {
        populateTreeView();
        this->setWindowTitle(myAppName + " - " + filename.split("/").back());
        QString txt = QString("PM Version ") + QString::fromStdString(datfile->getVersion());
        if (datfile->getIsSwapped()) {
            txt.append(QString::fromUtf8(" [byte order: big endian]"));
        }
        ui->textEdit->append(txt);
        ui->textEdit->append(QString::fromUtf8("file date: ")
            + QString::fromStdString(datfile->getFileDate()));
    }
}


PMbrowserWindow::PMbrowserWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PMbrowserWindow), currentFile{}, infile{}, datfile{nullptr}, lastexportpath{}
{
    ui->setupUi(this);
    setWindowIcon(QIcon(QString(":/myappico.ico")));
    setWindowTitle(myAppName);
    setAcceptDrops(true);
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
        loadFile(currentFile);
    }
}

void PMbrowserWindow::traceSelected(QTreeWidgetItem* item, hkTreeNode* trace)
{
    // figure out index
    QTreeWidgetItem* sweepitem = item->parent();
    QTreeWidgetItem* seriesitem = sweepitem->parent();
    QTreeWidgetItem* groupitem = seriesitem->parent();
    int indexseries = groupitem->indexOfChild(seriesitem)+1,
            indexsweep = seriesitem->indexOfChild(sweepitem)+1,
            indextrace = sweepitem->indexOfChild(item)+1;
    int indexgroup = ui->treePulse->indexOfTopLevelItem(groupitem)+1;
    QString tracename = QString("tr_%1_%2_%3_%4").arg(indexgroup).arg(indexseries).arg(indexsweep).arg(indextrace);
    ui->textEdit->append(tracename);
    double sealresistance = trace->extractLongReal(TrSealResistance),
            cslow = trace->extractLongReal(TrCSlow),
            Rseries = 1.0/trace->extractLongReal(TrGSeries),
            Vhold = trace->extractLongRealNoThrow(TrTrHolding);
    QString info = QString("Rmem=%1 Ohm\nCslow=%2 F\nRs=%3 Ohm\nVhold=%4 V").arg(sealresistance).arg(cslow).arg(Rseries).arg(Vhold);
    ui->textEdit->append(info);
    ui->renderArea->renderTrace(trace, infile);
}


void PMbrowserWindow::on_actionClose_triggered()
{
    closeFile();
}

void PMbrowserWindow::on_actionClear_Text_triggered()
{
    ui->textEdit->clear();
}

void PMbrowserWindow::exportSubTree(QTreeWidgetItem* item, const QString& path, const QString& prefix)
{
    int N = item->childCount();
    if (N > 0) {
        for (int i = 0; i < N; ++i) {
            exportSubTree(item->child(i), path, prefix);
        }
    } else {
        // must be at trace level already
        // figure out index
        QTreeWidgetItem* sweepitem = item->parent();
        QTreeWidgetItem* seriesitem = sweepitem->parent();
        QTreeWidgetItem* groupitem = seriesitem->parent();
        int indexseries = groupitem->indexOfChild(seriesitem) + 1,
            indexsweep = seriesitem->indexOfChild(sweepitem) + 1,
            indextrace = sweepitem->indexOfChild(item) + 1;
        int indexgroup = ui->treePulse->indexOfTopLevelItem(groupitem) + 1;
        QVariant v = item->data(0, Qt::UserRole);
        hkTreeNode* traceentry = v.value<hkTreeNode*>();
        int32_t datakind = traceentry->extractUInt16(TrDataKind);
        QString tracelabel;
        if (datakind & IsImon) {
            tracelabel = "Imon";
        }
        else if (datakind & IsVmon) {
            tracelabel = "Vmon";
        }
        else if (datakind & IsLeak) {
            tracelabel = "Leak";
        }
        else {
            tracelabel = QString("%1").arg(indextrace);
        }
        QString wavename = prefix + QString("_%1_%2_%3_%4").arg(indexgroup).arg(indexseries).arg(indexsweep).arg(tracelabel);
        QString filename = path + wavename + ".ibw";

        ui->textEdit->append("exporting " + wavename);
        ui->textEdit->update();
        ExportTrace(infile, *traceentry, filename.toStdString(), wavename.toStdString());
    }
}

bool PMbrowserWindow::choosePathAndPrefix(QString& path, QString& prefix)
{
    DlgChoosePathAndPrefix dlg(this, lastexportpath);
    if (dlg.exec())
    {
        path = dlg.path;
        if (!QDir(path).exists()) {
            QMessageBox::warning(this, QString("Error"), QString("Path does not exist!"));
            return false;
        }
        prefix = dlg.prefix;
        if (!path.endsWith('/')) {
            path.append('/');
        }
        lastexportpath = path;
    }
    return true;
}

void PMbrowserWindow::exportSubTreeAsIBW(QTreeWidgetItem* root)
{
    QString path, prefix;
    if (choosePathAndPrefix(path, prefix)) {
        try {
            exportSubTree(root, path, prefix);
        }
        catch (std::exception& e) {
            QString msg = QString("Error while exporting:\n%1").arg(QString(e.what()));
            QMessageBox::warning(this, QString("Error"), msg);
        }
    }
}

void PMbrowserWindow::on_actionExport_IBW_File_triggered()
{
    auto item = ui->treePulse->currentItem();
    if (!datfile) {
        QMessageBox msg;
        msg.setText("no file open");
        msg.exec();
    }
    else if(!item){
        QMessageBox msg;
        msg.setText("no item selected");
        msg.exec();
    }
    else {
        exportSubTreeAsIBW(item);
    }
}

void PMbrowserWindow::on_actionExport_All_as_IBW_triggered()
{
    if (!datfile) {
        QMessageBox msg;
        msg.setText("no file open");
        msg.exec();
    }
    else {
        QString path, prefix;
        if (choosePathAndPrefix(path, prefix)) {
            ui->textEdit->append("exporting...");
            try {
                ExportAllTraces(infile, *datfile, path.toStdString(), prefix.toStdString());
            }
            catch (std::exception& e) {
                QString msg = QString("Error while exporting:\n%1").arg(QString(e.what()));
                QMessageBox::warning(this, QString("Error"), msg);
                ui->textEdit->append("error -> aborting export");
            }
            ui->textEdit->append("done.");
        }
    }
}

void PMbrowserWindow::on_actionAbout_triggered()
{
    QString txt = myAppName +  ", Version " + appVersion +
                                "\n\n© Copyright 2020 Christian R. Halaszovich"
                                "\n\nAn open source tool to handle PatchMaster Files.\n"
                                "PatchMaster is a trademark of Heka GmbH\n\n"
                                "Build using Qt Library version " + QT_VERSION_STR +
                                "\n\nLicense: GNU General Public License Version 3 (GPLv3)";
    QMessageBox msg;
    msg.setWindowTitle("About");
    msg.setText(txt);
    msg.exec();
}

void PMbrowserWindow::on_treePulse_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    (void)previous;
    if(!current) {
        // strangely, this can get called with current = nullptr
        return;
    }
    QVariant v = current->data(0, Qt::UserRole);
    hkTreeNode* node = v.value<hkTreeNode*>();
    if(node != nullptr && node->getLevel() == 4) { // this is a trace item
        traceSelected(current, node);
    }
}

void PMbrowserWindow::dragEnterEvent(QDragEnterEvent* event)
{
    auto mimedata = event->mimeData();
    if (mimedata->hasUrls()) {
        auto url = mimedata->urls()[0];
        if (url.isLocalFile()) {
            auto filename = url.toLocalFile();
            if (filename.endsWith(".dat")) {
                event->acceptProposedAction();
            }
        }
    }
}

void PMbrowserWindow::dropEvent(QDropEvent* event)
{
    auto mimedata = event->mimeData();
    if (mimedata->hasUrls()) {
        auto url = mimedata->urls()[0];
        if (url.isLocalFile()) {
            auto filename = url.toLocalFile();
            if (filename.endsWith(".dat")) {
                loadFile(filename);
            }
        }
    }
}

void PMbrowserWindow::resizeEvent(QResizeEvent* event)
{
   auto s = event->size();
   ui->widget->resize(s);
   ui->splitterH->setGeometry(5, 5, s.width() - 10, s.height() - 30);
}
