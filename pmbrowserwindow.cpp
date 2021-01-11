/*
    Copyright 2020, 2021 Christian R. Halaszovich

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
#include <QApplication>
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstring>
#include <cmath>
#include "pmbrowserwindow.h"
#include "exportIBW.h"
#include "hkTree.h"
#include "helpers.h"
#include "ui_pmbrowserwindow.h"
#include "DlgChoosePathAndPrefix.h"
#include "ui_DlgChoosePathAndPrefix.h"
#include "DlgTreeFilter.h"
#include "PMparameters.h"
#include "DlgSelectParameters.h"

const QString myAppName("PM browser");
const QString appVersion("1.2 experimental");

Q_DECLARE_METATYPE(hkTreeNode*)




void PMbrowserWindow::populateTreeView()
{
    auto tree =ui -> treePulse;
    tree->setColumnCount(1);
    auto& pultree=datfile->GetPulTree();
    QList<QTreeWidgetItem *> grpitems;
    int i=0;
    for(auto& group : pultree.GetRootNode().Children) {
        QString count=QString("%1").arg(++i), label;
        label = group.getString(GrLabel).c_str();
        QStringList qsl;
        qsl.append(count+" "+label);
        QTreeWidgetItem* grpitem = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), qsl);
        grpitem->setData(0, Qt::UserRole, QVariant::fromValue(&group));
        grpitems.append(grpitem);
        int j=0;
        for(auto& series : group.Children) {
            QString label2 = QString("%1").arg(++j)+" "+QString(series.getString(SeLabel).c_str());
            auto seriesitem = new QTreeWidgetItem(grpitem, QStringList(label2));
            seriesitem->setData(0, Qt::UserRole, QVariant::fromValue(&series));
            int k = 0;
            for(auto& sweep : series.Children) {
                QString label3 = QString("sweep %1").arg(++k)+" "+QString(sweep.getString(SwLabel).c_str());
                auto sweepitem = new QTreeWidgetItem(seriesitem, QStringList(label3));
                sweepitem->setData(0, Qt::UserRole, QVariant::fromValue(&sweep));
                int l = 0;
                for(auto& trace : sweep.Children) {
                    ++l;
                    QString tracelabel = formTraceName(trace, l).c_str();
                    auto traceitem = new QTreeWidgetItem(sweepitem, QStringList(tracelabel));
                    traceitem->setData(0,Qt::UserRole, QVariant::fromValue(&trace)); // store pointer to trace for later use
                }
            }
        }
    }
    tree->addTopLevelItems(grpitems);
    tree->expandAll();
}

void PMbrowserWindow::traceSelected(QTreeWidgetItem* item, hkTreeNode* trace)
{
    // figure out index
    QTreeWidgetItem* sweepitem = item->parent();
    QTreeWidgetItem* seriesitem = sweepitem->parent();
    QTreeWidgetItem* groupitem = seriesitem->parent();
    int indexseries = groupitem->indexOfChild(seriesitem) + 1,
        indexsweep = seriesitem->indexOfChild(sweepitem) + 1,
        indextrace = sweepitem->indexOfChild(item) + 1;
    int indexgroup = ui->treePulse->indexOfTopLevelItem(groupitem) + 1;
    QString tracename = QString("tr_%1_%2_%3_%4").arg(indexgroup).arg(indexseries).arg(indexsweep).arg(indextrace);
    ui->textEdit->append(tracename);

    // Give holding V / I special treatment, since we want to distingushe CC / VC mode
    double holding = trace->extractLongRealNoThrow(TrTrHolding);
    char mode = trace->getChar(TrRecordingMode);
    QString prefix = "Vhold", yunit = "V";
    if (mode == CClamp) {
        yunit = "A";
        prefix = "Ihold";
    }
    // keep the following, since here we format it more nicely, with correct name an units
    // this is beyond what PMparmaters can do right now.
    QString info = QString("%1=%2 %3").arg(prefix).arg(holding).arg(yunit);
    std::string str;
    formatParamListPrint(*trace, parametersTrace, str);
    info.append("\n");
    info.append(str.c_str());
    ui->textEdit->append(info);
    ui->renderArea->renderTrace(trace, infile);
}

void PMbrowserWindow::sweepSelected(QTreeWidgetItem* item, hkTreeNode* sweep) {
    (void)item;
    QString label = QString::fromStdString(sweep->getString(SeLabel));
    int32_t count = sweep->extractInt32(SwSweepCount);
    QString txt = QString("Sweep %1 %2").arg(label).arg(count);
    std::string str;
    formatParamListPrint(*sweep, parametersSweep, str);
    txt.append("\n");
    txt.append(str.c_str());
    ui->textEdit->append(txt);
}

void PMbrowserWindow::seriesSelected(QTreeWidgetItem* item, hkTreeNode* series)
{
    (void)item;
    QString label = QString::fromStdString(series->getString(SeLabel));
    int32_t count = series->extractInt32(SeSeriesCount);
    QString txt = QString("Series %1 %2").arg(label).arg(count);
    std::string str;
    formatParamListPrint(*series, parametersSeries, str);
    txt.append("\n");
    txt.append(str.c_str());
    ui->textEdit->append(txt);
}

void PMbrowserWindow::groupSelected(QTreeWidgetItem* item, hkTreeNode* group)
{
    (void)item;
    QString label = QString::fromStdString(group->getString(GrLabel));
    int32_t count = group->extractInt32(GrGroupCount);
    QString txt = QString("Group %1 %2").arg(label).arg(count);
    std::string str;
    formatParamListPrint(*group, parametersGroup, str);
    txt.append("\n");
    txt.append(str.c_str());
    ui->textEdit->append(txt);
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
    lastloadpath = QFileInfo(filename).path();
    //settings_modified = true;
    QSettings settings;
    settings.setValue("pmbrowserwindow/lastloadpath", lastloadpath);
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
        try {
            const auto& amprootnode = datfile->GetAmpTree().GetRootNode();
            std::string ampname = amprootnode.getString(RoAmplifierName);
            // turns out, the following filed are somewhat obscure/useless
            // auto amptype = static_cast<int>(amprootnode.getChar(RoAmplifier));
            // auto adboard = static_cast<int>(amprootnode.getChar(RoADBoard));
            txt.append(QString("\nAmplifier: %1").arg(ampname.c_str()));
        }
        catch (std::out_of_range& e) {
            (void)e;
            //Note: we usually get here if there is no Amp-Tree
            //txt.append("\n(unknown amplifier)");
        }
        ui->textEdit->append(txt);
        ui->textEdit->append(QString::fromUtf8("file date: ")
            + QString::fromStdString(datfile->getFileDate()));

    }
}


PMbrowserWindow::PMbrowserWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PMbrowserWindow), currentFile{}, infile{}, datfile{ nullptr }, lastloadpath{}, lastexportpath{},
    filterStrGrp{ ".*" }, filterStrSer{ ".*" }, filterStrSwp{ ".*" }, filterStrTr{ ".*" },
    settings_modified{ false }
{
    ui->setupUi(this);
    setWindowIcon(QIcon(QString(":/myappico.ico")));
    setWindowTitle(myAppName);
    setAcceptDrops(true);
    QObject::connect(ui->actionAuto_Scale, &QAction::triggered, ui->renderArea, &RenderArea::autoScale);
    ui->treePulse->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->actionDo_Autoscale_on_Load, &QAction::toggled, ui->renderArea, &RenderArea::toggleDoAutoscale);
    QObject::connect(ui->treePulse, &QTreeWidget::customContextMenuRequested, this, &PMbrowserWindow::prepareTreeContextMenu);
    QObject::connect(ui->actionSettings, &QAction::triggered, ui->renderArea, &RenderArea::showSettingsDialog);
    QObject::connect(ui->actionWipe, &QAction::triggered, ui->renderArea, &RenderArea::wipeAll);
    QObject::connect(ui->actionYX_mode, &QAction::triggered, ui->renderArea, &RenderArea::setXYmode);
    QObject::connect(ui->actionYT_mode, &QAction::triggered, ui->renderArea, &RenderArea::setYTmode);
    QObject::connect(ui->actionClear_Persitant_Traces, &QAction::triggered, ui->renderArea, &RenderArea::wipeBuffer);
    QAction* aboutQtAct = ui->menuHelp->addAction("About &Qt", qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip("Show the Qt library's About box");

    loadSettings();
    ui->renderArea->loadSettings();
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
    if(QDir(lastloadpath).exists()) {
        dialog.setDirectory(lastloadpath);
    }
    else {
        dialog.setDirectory("./");
    }
    if (dialog.exec()) {
        loadFile(dialog.selectedFiles().at(0));
    }
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
    if (item->isHidden()) { return; } // export only visible items
    int N = item->childCount();
    if (N > 0) {
        for (int i = 0; i < N; ++i) {
            exportSubTree(item->child(i), path, prefix);
        }
    }
    else {
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
    if (!QDir(lastexportpath).exists()) {
        lastexportpath.clear();
    }
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
        settings_modified = true;
        return true;
    }
    else {
        return false;
    }
}

void PMbrowserWindow::exportAllVisibleTraces()
{
    QString path, prefix;
    if (choosePathAndPrefix(path, prefix)) {
        try {
            int N = ui->treePulse->topLevelItemCount();
            for (int i = 0; i < N; ++i) {
                exportSubTree(ui->treePulse->topLevelItem(i), path, prefix);
            }
        }
        catch (std::exception& e) {
            QString msg = QString("Error while exporting:\n%1").arg(QString(e.what()));
            QMessageBox::warning(this, QString("Error"), msg);
        }
    }
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

void PMbrowserWindow::filterTree()
{ 
    QRegularExpression reGrp(filterStrGrp), reSer(filterStrSer),
        reSwp(filterStrSwp), reTr(filterStrTr);
    if (!reGrp.isValid()) {
        QMessageBox::critical(this, "Invalid RegEx for Group", reGrp.errorString());
        return;
    }
    if (!reSer.isValid()) {
        QMessageBox::critical(this, "Invalid RegEx for Series", reSer.errorString());
        return;
    }
    if (!reSwp.isValid()) {
        QMessageBox::critical(this, "Invalid RegEx for Sweep", reSwp.errorString());
        return;
    }
    if (!reTr.isValid()) {
        QMessageBox::critical(this, "Invalid RegEx for Trace", reTr.errorString());
        return;
    }
    auto Ngrp = ui->treePulse->topLevelItemCount();
    for (int count_grp = 0; count_grp < Ngrp; ++count_grp) {
        auto grp = ui->treePulse->topLevelItem(count_grp);
        if (reGrp.match(grp->text(0)).hasMatch()) {
            grp->setHidden(false);
        }
        else {
            grp->setHidden(true);
        }
        auto Nser = grp->childCount();
        for (int count_ser = 0; count_ser < Nser; ++count_ser) {
            auto ser = grp->child(count_ser);
            if (reSer.match(ser->text(0)).hasMatch()) {
                ser->setHidden(false);
            }
            else {
                ser->setHidden(true);
            }
            auto Nswp = ser->childCount();
            for (int count_swp = 0; count_swp < Nswp; ++count_swp) {
                auto swp = ser->child(count_swp);
                if (reSwp.match(swp->text(0)).hasMatch()) {
                    swp->setHidden(false);
                }
                else {
                    swp->setHidden(true);
                }
                auto Ntr = swp->childCount();
                for (int count_tr = 0; count_tr < Ntr; ++count_tr) {
                    auto tr = swp->child(count_tr);
                    if (reTr.match(tr->text(0)).hasMatch()) {
                        tr->setHidden(false);
                    }
                    else {
                        tr->setHidden(true);
                    }
                }
            }
        }

    }
    //TODO
}

void PMbrowserWindow::on_actionExport_IBW_File_triggered()
{
    auto item = ui->treePulse->currentItem();
    if (!datfile) {
        QMessageBox msg;
        msg.setText("no file");
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
                                "\n\n© Copyright 2020, 2021 Christian R. Halaszovich"
                                "\n\nAn open source tool to handle PatchMaster Files.\n"
                                "PatchMaster is a trademark of Heka GmbH\n\n"
                                "Build using Qt Library version " + QT_VERSION_STR +
                                "\n\nLicense: GNU General Public License Version 3 (GPLv3)";
    QMessageBox msg;
    msg.setWindowTitle("About");
    msg.setText(txt);
    msg.exec();
}

void PMbrowserWindow::on_actionFilter_triggered()
{
    DlgTreeFilter dlg(this, filterStrGrp, filterStrSer, filterStrSwp, filterStrTr);
    if (dlg.exec()) {
        settings_modified = true;
        filterStrGrp = dlg.grp;
        filterStrSer = dlg.ser;
        filterStrSwp = dlg.swp;
        filterStrTr = dlg.trace;
        filterTree();
    }
}

void PMbrowserWindow::treeSetHidden(QTreeWidgetItem* item, bool hidden)
{
    item->setHidden(hidden);
    int N = item->childCount();
    for (int i = 0; i < N; ++i) {
        treeSetHidden(item->child(i), hidden);
    }
}

void PMbrowserWindow::unhideTreeItems(QTreeWidgetItem* item)
{
    treeSetHidden(item, false);
}

void PMbrowserWindow::on_actionRemove_Filter_triggered()
{
    int N = ui->treePulse->topLevelItemCount();
    for (int i = 0; i < N; ++i) {
        unhideTreeItems(ui->treePulse->topLevelItem(i));
    }
}

void PMbrowserWindow::on_actionExport_All_Visible_Traces_as_IBW_Files_triggered()
{
    exportAllVisibleTraces();
}

void PMbrowserWindow::prepareTreeContextMenu(const QPoint& pos)
{
    auto item = ui->treePulse->itemAt(pos);
    if (item) {
        QMenu menu(this);
        auto actExport = menu.addAction("export subtree");
        auto actHide = menu.addAction("hide subtree");
        auto actShow = menu.addAction("show all children");
        auto actPrintAllP = menu.addAction("print all parameters");
        QAction* actAmpstate = nullptr;
        const auto node = item->data(0, Qt::UserRole).value<hkTreeNode*>();
        if (node->getLevel() == hkTreeNode::LevelSeries) {
            actAmpstate = menu.addAction("amplifier state");
        }
        auto response = menu.exec(ui->treePulse->mapToGlobal(pos));
        if (response == actExport) {
            exportSubTreeAsIBW(item);
        }
        else if (response == actHide) {
            treeSetHidden(item, true);
        }
        else if (response == actShow) {
            treeSetHidden(item, false);
        }
        else if (response == actPrintAllP) {
            printAllParameters(item);
        }
        else if (actAmpstate != nullptr && actAmpstate == response) {
            printAmplifierState(node);
        }
    }
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
    if (node != nullptr) {
        switch (node->getLevel())
        {
        case hkTreeNode::LevelGroup:
            groupSelected(current, node);
            break;
        case hkTreeNode::LevelSeries:
            seriesSelected(current, node);
            break;
        case hkTreeNode::LevelSweep:
            sweepSelected(current, node);
            break;
        case hkTreeNode::LevelTrace:
            traceSelected(current, node);
            break;
        default:
            break;
        }
    }
}

void PMbrowserWindow::on_actionSelect_Parameters_triggered()
{
    DlgSelectParameters dlg;
    if (dlg.exec()) {
        dlg.storeParams();
        settings_modified = true;
    }
}

void ::PMbrowserWindow::printAllParameters(QTreeWidgetItem* item)
{
    printAllParameters(item->data(0, Qt::UserRole).value<hkTreeNode*>());
}

void ::PMbrowserWindow::printAllParameters(hkTreeNode* n)
{
    std::string s;
    QString lb;
    // experimentnal: include parents
    if (n->getParent()) {
        printAllParameters(n->getParent());
    }
    switch (n->getLevel()) {
    case hkTreeNode::LevelRoot:
        lb = "Root:\n";
        formatParamList(*n, parametersRoot, s);
        break;
    case hkTreeNode::LevelGroup:
        lb = "Group:\n";
        formatParamList(*n, parametersGroup, s);
        break;
    case hkTreeNode::LevelSeries:
        lb = "Series:\n";
        formatParamList(*n, parametersSeries, s);
        break;
    case hkTreeNode::LevelSweep:
        lb = "Sweep:\n";
        formatParamList(*n, parametersSweep, s);
        break;
    case hkTreeNode::LevelTrace:
        lb = "Trace:\n";
        formatParamList(*n, parametersTrace, s);
        break;
    }
    ui->textEdit->append(lb + s.c_str());
}

void PMbrowserWindow::printAmplifierState(const hkTreeNode* series)
{
    assert(series->getLevel() == hkTreeNode::LevelSeries);
    hkTreeNode amprecord;
    amprecord.len = AmplifierStateSize;
    amprecord.isSwapped = series->getIsSwapped();
    auto ampstateflag = series->extractInt32(SeAmplStateFlag),
        ampstateref = series->extractInt32(SeAmplStateRef);
    if (ampstateflag > 0 || ampstateref == 0) {
        // use local amp state record
        amprecord.Data = series->Data + SeOldAmpState;
        std::string s;
        formatParamList(amprecord, parametersAmpplifierState, s);
        ui->textEdit->append(QString("Amplifier State:\n") + s.c_str());
    }
    else {
        // auto secount = series->extractInt32(SeSeriesCount);
        const auto& amproot = datfile->GetAmpTree().GetRootNode();
        const auto& ampse = amproot.Children.at(ampstateref - 1); // Is this correct? Or seCount?
        for (const auto& ampre : ampse.Children) { // there might be multiple amplifiers
            auto ampstatecount = ampre.extractInt32(AmStateCount);
            amprecord.Data = ampre.Data + AmAmplifierState;
            std::string s;
            formatParamList(amprecord, parametersAmpplifierState, s);
            ui->textEdit->append(QString("Amplifier State (Amp #%1):\n").arg(ampstatecount) + s.c_str());
        }
    }

}

void PMbrowserWindow::on_actionPrint_All_Params_triggered()
{
    auto item = ui->treePulse->currentItem();
    if (item) {
        printAllParameters(item);
    }
}

void PMbrowserWindow::on_menuGraph_aboutToShow()
{
    ui->actionDo_Autoscale_on_Load->setChecked(ui->renderArea->isAutoscaleEnabled());
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

void PMbrowserWindow::closeEvent(QCloseEvent* event)
{
    if (settings_modified || ui->renderArea->isSettingsModified()) {
        auto res = QMessageBox::question(this, "Save Settings",
            "Some settings have bee modified.\nDo you want to save them?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
        if (res == QMessageBox::Cancel) {
            event->ignore();
        }
        else {
            if (res == QMessageBox::Yes) {
                saveSettings();
                ui->renderArea->saveSettings();
            }
            event->accept();
        }
    }
}


void PMbrowserWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("pmbrowserwindow");
    settings.setValue("lastloadpath", lastloadpath);
    settings.setValue("lastexportpath", lastexportpath);
    settings.setValue("filterStrGrp", filterStrGrp);
    settings.setValue("filterStrSer", filterStrSer);
    settings.setValue("filterStrSwp", filterStrSwp);
    settings.setValue("filterStrTr", filterStrTr);
    settings.endGroup();

    settings.beginGroup("params_group");
    for (const auto& p : parametersGroup) {
        settings.setValue(p.name, p.toInt());
    }
    settings.endGroup();
    settings.beginGroup("params_series");
    for (const auto& p : parametersSeries) {
        settings.setValue(p.name, p.toInt());
    }
    settings.endGroup();
    settings.beginGroup("params_sweep");
    for (const auto& p : parametersSweep) {
        settings.setValue(p.name, p.toInt());
    }
    settings.endGroup();
    settings.beginGroup("params_trace");
    for (const auto& p : parametersTrace) {
        settings.setValue(p.name, p.toInt());
    }
    settings.endGroup();
    
}

void PMbrowserWindow::loadSettings()
{
    QSettings settings;
	settings.beginGroup("pmbrowserwindow");
    lastloadpath = settings.value("lastloadpath", lastloadpath).toString();
    lastexportpath = settings.value("lastexportpath", lastexportpath).toString();
	filterStrGrp = settings.value("filterStrGrp", filterStrGrp).toString();
	filterStrSer = settings.value("filterStrSer", filterStrSer).toString();
	filterStrSwp = settings.value("filterStrSwp", filterStrSwp).toString();
	filterStrTr = settings.value("filterStrTr", filterStrTr).toString();
    settings.endGroup();

    settings.beginGroup("params_group");
    for (auto& p : parametersGroup) {
        p.fromInt(settings.value(p.name, p.toInt()).toInt());
    }
    settings.endGroup();
    settings.beginGroup("params_series");
    for (auto& p : parametersSeries) {
        p.fromInt(settings.value(p.name, p.toInt()).toInt());
    }
    settings.endGroup();
    settings.beginGroup("params_sweep");
    for (auto& p : parametersSweep) {
        p.fromInt(settings.value(p.name, p.toInt()).toInt());
    }
    settings.endGroup();
    settings.beginGroup("params_trace");
    for (auto& p : parametersTrace) {
        p.fromInt(settings.value(p.name, p.toInt()).toInt());
    }
    settings.endGroup();
}
