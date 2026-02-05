/*
    Copyright 2020 - 2025 Christian R. Halaszovich

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
#include <QGuiApplication>
#include <QClipboard>
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QProgressBar>
#include <QString>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QTableView>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstring>
#include "pmbrowserwindow.h"
#include "exportIBW.h"
#include "exportNPY.h"
#include "hkTree.h"
#include "StimTree.h"
#include "helpers.h"
#include "DlgChoosePathAndPrefix.h"
#include "DlgExportMetadata.h"
#include "DlgTreeFilter.h"
#include "PMparameters.h"
#include "DlgSelectParameters.h"
#include "DlgPreferences.h"
#include "TxtTableModel.h"
#include "qstring_helper.h"
#include "Config.h"

using namespace hkLib;

#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
#include <QLatin1StringView>
constexpr QLatin1StringView myAppName("PM browser");
constexpr QLatin1StringView appVersion(VERSION);
#else
const QString myAppName("PM browser");
const QString appVersion(VERSION);
#endif

Q_DECLARE_METATYPE(hkTreeNode*)


static QString MakeSweepLabel(const hkLib::hkTreeNode& sweep_node) {
    QString label = QString("Sweep %1").arg(sweep_node.extractInt32(SwSweepCount));
    auto sw_label = qs_from_sv(sweep_node.getString(SwLabel));
    if (sw_label.length() > 0) {
        label += ' ' + sw_label;
    }
    return label;
}

void PMbrowserWindow::populateTreeView()
{
    auto tree = ui -> treePulse;
    tree->setColumnCount(1);
    auto& pultree = datfile->GetPulTree();
    QList<QTreeWidgetItem *> grpitems;
    for(auto& group : pultree.GetRootNode().Children) {
        QString count=QString("%1").arg(group.extractInt32(GrGroupCount));
        QString label = qs_from_sv(group.getString(GrLabel));
        QStringList qsl;
        qsl.append(count+" "+label);
        QTreeWidgetItem* grpitem = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), qsl);
        grpitem->setData(0, Qt::UserRole, QVariant::fromValue(&group));
        grpitems.append(grpitem);
        for(auto& series : group.Children) {
            QString label2 = QString("%1").arg(series.extractInt32(SeSeriesCount))+
                " "+qs_from_sv(series.getString(SeLabel));
            auto seriesitem = new QTreeWidgetItem(grpitem, QStringList(label2));
            seriesitem->setData(0, Qt::UserRole, QVariant::fromValue(&series));
            for(auto& sweep : series.Children) {
                QString label3 = MakeSweepLabel(sweep);
                auto sweepitem = new QTreeWidgetItem(seriesitem, QStringList(label3));
                sweepitem->setData(0, Qt::UserRole, QVariant::fromValue(&sweep));
                for(auto& trace : sweep.Children) {
                    QString tracelabel{ QString::fromUtf8(formTraceName(trace, trace.extractInt32(TrTraceID))) };
                    auto traceitem = new QTreeWidgetItem(sweepitem, QStringList(tracelabel));
                    traceitem->setData(0,Qt::UserRole, QVariant::fromValue(&trace)); // store pointer to trace for later use
                }
            }
        }
    }
    tree->addTopLevelItems(grpitems);
    tree->expandAll();
}

void PMbrowserWindow::traceSelected(const QTreeWidgetItem* item, const hkTreeNode* trace)
{
    (void)item;
    int indextrace = trace->extractInt32(TrTraceID);
    auto trace_label = formTraceName(*trace, indextrace);
    QString tracename = QString("Trace ") + QString::fromUtf8(trace_label.data(), trace_label.size());
    ui->textEdit->append(tracename);

    // Give holding V / I special treatment, since we want to distingushe CC / VC mode
    std::string yunit{};
    double holding = datfile->getTraceHolding(*trace, yunit);
    QString prefix = "Vhold";
    if (yunit.at(0)=='A') {
        prefix = "Ihold";
    }

    // keep the following, since here we format it more nicely, with correct name and units
    // this is beyond what PMparmaters can do right now.
    QString info = QString("%1=%L2 %3").arg(prefix).arg(holding).arg(QLatin1StringView(yunit));
    std::string str;
    formatParamListPrint(*trace, parametersTrace, str);
    info.append("\n");
    info.append(str.c_str());
    ui->textEdit->append(info);
    ui->renderArea->renderTrace(trace, this->infile);
}

void PMbrowserWindow::collectChildTraces(const QTreeWidgetItem* item, int level, std::vector<hkTreeNode*>& trace_list)
{
    if (!item->isHidden()) {
        if (level < hkTreeNode::LevelTrace) {
            int N = item->childCount();
            for (int i = 0; i < N; ++i) {
                collectChildTraces(item->child(i), level + 1, trace_list);
            }
        }
        else {
            auto trace = item->data(0, Qt::UserRole).value<hkTreeNode*>();
            trace_list.push_back(trace);
        }
    }
}

void PMbrowserWindow::animateTraceList(const QString& info_text, const std::vector<hkLib::hkTreeNode*>& trace_list)
{
    auto num_traces = static_cast<int>(trace_list.size());
    QProgressDialog progress(info_text, "Abort", 0, num_traces, this);
    QProgressBar* pbar = new QProgressBar(&progress);
    pbar->setMaximum(num_traces);
    pbar->setMinimum(0);
    pbar->setFormat("%v/%m");
    progress.setBar(pbar);
    progress.setWindowModality(Qt::WindowModal);
    for (int i = 0; i < num_traces; ++i) {
        progress.setValue(i);
        if (progress.wasCanceled()) {
            break;
        }
        ui->renderArea->renderTrace(trace_list.at(i), infile);
        ui->renderArea->repaint();
#ifdef __APPLE__
        // unfortunately, on macOS Qt doesn't support QWdiget::repaint
        // This is a kind of work-around
        QCoreApplication::processEvents();
#endif
    }
    progress.setValue(num_traces);
}

static const hkLib::hkTreeNode* item2node(const QTreeWidgetItem* item)
{
    return item->data(0, Qt::UserRole).value<hkTreeNode*>();
}

hkLib::hkTreeView PMbrowserWindow::getVisibleNodes()
{
    hkLib::hkTreeView tree;
    tree.root.p_node = &(datfile->GetPulTree().GetRootNode());
    int Ngroup = ui->treePulse->topLevelItemCount();
    for (int groupcount = 0; groupcount < Ngroup; ++groupcount) {
        auto item_group = ui->treePulse->topLevelItem(groupcount);
        if (!item_group->isHidden()) {
            hkLib::hkNodeView group;
            group.p_node = item2node(item_group);
            int Nseries = item_group->childCount();
            for (int series_count = 0; series_count < Nseries; ++series_count) {
                auto item_series = item_group->child(series_count);
                if (!item_series->isHidden()) {
                    hkLib::hkNodeView series;
                    series.p_node= item2node(item_series);
                    int  Nsweeps = item_series->childCount();
                    for (int sweep_count = 0; sweep_count < Nsweeps; ++sweep_count) {
                        auto item_sweep = item_series->child(sweep_count);
                        if (!item_sweep->isHidden()) {
                            hkLib::hkNodeView sweep;
                            sweep.p_node = item2node(item_sweep);
                            int Ntraces = item_sweep->childCount();
                            for (int trace_count = 0; trace_count < Ntraces; ++trace_count) {
                                auto item_trace = item_sweep->child(trace_count);
                                if (!item_trace -> isHidden()) {
                                    hkLib::hkNodeView trace;
                                    trace.p_node = item2node(item_trace);
                                    sweep.children.emplace_back(std::move(trace));
                                }
                            }
                            series.children.emplace_back(std::move(sweep));
                        }
                    }
                    group.children.emplace_back(std::move(series));
                }
            }
            tree.root.children.emplace_back(std::move(group));
        }
    }
    return tree;
}

void PMbrowserWindow::sweepSelected(const QTreeWidgetItem* item, const hkTreeNode* sweep) {
    (void)item;
    QString txt = MakeSweepLabel(*sweep) + '\n';
    std::string str;
    formatParamListPrint(*sweep, parametersSweep, str);
    // to restore compatibility with Qt6.4:
    txt.append(QString::fromUtf8(str.data(), str.size()));
    //txt.append(QUtf8StringView(str));
    ui->textEdit->append(txt);
}

void PMbrowserWindow::seriesSelected(const QTreeWidgetItem* item, const hkTreeNode* series)
{
    (void)item;
    QString label = qs_from_sv(series->getString(SeLabel));
    int32_t count = series->extractInt32(SeSeriesCount);
    QString txt = QString("Series %1 %2").arg(label).arg(count);
    std::string str;
    formatParamListPrint(*series, parametersSeries, str);
    txt.append("\n");
    txt.append(str.c_str());
    ui->textEdit->append(txt);
}

void PMbrowserWindow::groupSelected(const QTreeWidgetItem* item, const hkTreeNode* group)
{
    (void)item;
    QString label = qs_from_sv(group->getString(GrLabel));
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
        //delete datfile;
        datfile = nullptr;
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
    datfile = std::make_unique<DatFile>();
    try {
        datfile->InitFromStream(infile);
    }
    catch (const std::exception& e) {
        QMessageBox::warning(this, QString("File Error"), 
            QString("error while processing dat file:\n") + QString(e.what()));
        datfile = nullptr;
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
            auto ampname = amprootnode.getString<32>(RoAmplifierName);
            // turns out, the following files are somewhat obscure/useless
            // auto amptype = static_cast<int>(amprootnode.getChar(RoAmplifier));
            // auto adboard = static_cast<int>(amprootnode.getChar(RoADBoard));
            txt.append(QString("\nAmplifier: ") + qs_from_sv(ampname));
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
        
    ui->treePulse->setExpandsOnDoubleClick(false);
    ui->actionSettings->setMenuRole(QAction::NoRole); // for macOS: prevent placment of entry in app menu
    
    setWindowIcon(QIcon(QString(":/myappico.ico")));
    setWindowTitle(myAppName);
    setAcceptDrops(true);

    QDir help_dir{QCoreApplication::applicationDirPath() + "/../" + DOCDIR + "/html"};
    QString help_path = help_dir.absoluteFilePath("index.html");
    if (QFile::exists(help_path)) {
        help_url = QUrl::fromLocalFile(help_path);
    }
    else {
       help_url = PROJECT_HOMEPAGE;
       actHelp.setText("Online &Help");
    }
    ui->menuHelp->insertAction(ui->actionAbout, &actHelp);
    QObject::connect(&actHelp, &QAction::triggered, this, &PMbrowserWindow::openHelp);

    QObject::connect(ui->action_Preferences, &QAction::triggered, this, &PMbrowserWindow::openPreferences);

    QObject::connect(ui->actionAuto_Scale, &QAction::triggered, ui->renderArea, &RenderArea::autoScale);
    ui->treePulse->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->actionDo_Autoscale_on_Load, &QAction::toggled, ui->renderArea, &RenderArea::toggleDoAutoscale);
    QObject::connect(ui->treePulse, &QTreeWidget::customContextMenuRequested, this, &PMbrowserWindow::prepareTreeContextMenu);
    QObject::connect(ui->actionSettings, &QAction::triggered, ui->renderArea, &RenderArea::showSettingsDialog);
    QObject::connect(ui->actionWipe, &QAction::triggered, ui->renderArea, &RenderArea::wipeAll);
    QObject::connect(ui->actionYX_mode, &QAction::triggered, ui->renderArea, &RenderArea::setXYmode);
    QObject::connect(ui->actionYT_mode, &QAction::triggered, ui->renderArea, &RenderArea::setYTmode);
    QObject::connect(ui->actionClear_Persitant_Traces, &QAction::triggered, ui->renderArea, &RenderArea::wipeBuffer);
    QObject::connect(ui->actionCopy, &QAction::triggered, ui->renderArea, &RenderArea::copyToClipboard);
    QAction* aboutQtAct = ui->menuHelp->addAction("About &Qt", qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip("Show the Qt library's About box");
    QObject::connect(ui->pushButtonTreeFilter, &QPushButton::clicked, this, &PMbrowserWindow::on_actionFilter_triggered);
    QObject::connect(ui->pushButtonShowAll, &QPushButton::clicked, this, &PMbrowserWindow::on_actionRemove_Filter_triggered);

    loadSettings();
    ui->renderArea->loadSettings();
}

PMbrowserWindow::~PMbrowserWindow()
{
    delete ui;
}

void PMbrowserWindow::on_actionOpen_triggered()
{
    QString loaddir = lastloadpath;
    if (!QDir(loaddir).exists()) {
        loaddir = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
    }
    QString filename = QFileDialog::getOpenFileName(this,
        "Open DAT File",
        loaddir,
        "DAT-file (*.dat)");
    if (!filename.isEmpty()) {
        loadFile(filename);
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

bool PMbrowserWindow::assertDatFileOpen()
{
    if (!datfile) {
        QMessageBox msg;
        msg.setText("Frist, open a file!");
        msg.exec();
        return false;
    }
    return true;
}

void PMbrowserWindow::exportSubTree(QTreeWidgetItem* item, const QString& path, const QString& prefix, ExportType export_type, std::ostream* poutfile, bool create_datafolders, int folder_level)
{
    if (item->isHidden()) { return; } // export only visible items
    int N = item->childCount();
    if (N > 0) {
        bool new_datafolder{ false };
        if (export_type == ExportType::Igor) {
            if (create_datafolders && poutfile != nullptr) {
                auto node = item->data(0, Qt::UserRole).value<hkTreeNode*>();
                new_datafolder = (poutfile != nullptr) && (node->getLevel() >= hkTreeNode::LevelGroup &&
                    node->getLevel() <= folder_level);
            }
            if (new_datafolder) {
                PackedFileRecordHeader pfrh{ kDataFolderStartRecord,0,32 };
                char buf[32]{};
                item->text(0).toStdString().copy(buf, 31);
                poutfile->write(reinterpret_cast<char*>(&pfrh), sizeof(PackedFileRecordHeader));
                poutfile->write(buf, 32);
            }
        }
        for (int i = 0; i < N; ++i) {
            exportSubTree(item->child(i), path, prefix, export_type, poutfile, create_datafolders, folder_level);
        }
        if (export_type == ExportType::Igor && new_datafolder) {
            PackedFileRecordHeader pfrh{ kDataFolderEndRecord,0,0 };
            poutfile->write(reinterpret_cast<char*>(&pfrh), sizeof(PackedFileRecordHeader));
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
        auto tracelabel = QString::fromStdString(formTraceName(*traceentry, indextrace));
        QString wavename = prefix + QString("_%1_%2_%3_%4").arg(indexgroup).arg(indexseries).arg(indexsweep).arg(tracelabel);
        ui->textEdit->append("exporting " + wavename);

        if (export_type == ExportType::Igor) {
            auto wname = wavename.toStdString();
            unsigned err{0};
            if (poutfile == nullptr) { // multi-file export
                QString filename = path + wavename + ".ibw";
                std::ofstream outfile(filename.toStdString(), std::ios::out | std::ios::binary);
                if (!outfile) {
                    std::stringstream msg;
                    msg << "error opening file '" << filename.toStdString() << "' for writing: " << strerror(errno);
                    throw std::runtime_error(msg.str());
                }
                err = ExportTrace(infile, *traceentry, outfile, wname);
            }
            else {
                PackedFileRecordHeader pfrh{};
                pfrh.recordType = kWaveRecord;
                size_t offset_record = poutfile->tellp();
                poutfile->write(reinterpret_cast<char*>(&pfrh), sizeof(PackedFileRecordHeader));
                err = ExportTrace(infile, *traceentry, *poutfile, wname);
                size_t offset_end = poutfile->tellp();
                pfrh.numDataBytes = static_cast<std::int32_t>(offset_end - offset_record - sizeof(PackedFileRecordHeader));
                poutfile->seekp(offset_record);
                poutfile->write(reinterpret_cast<char*>(&pfrh), sizeof(PackedFileRecordHeader));
                poutfile->seekp(offset_end);
            }
            if(err & WARNFLAG_WNAMETRUNCATED){
                ui->textEdit->append("Warning: wavename truncated to " + QString::fromUtf8(wname));
            }
        }
        else if (export_type == ExportType::NPY) {
            QString filename = path + wavename + ".npy";
            NPYorBINExportTrace(infile, *traceentry, filename.toStdString(), true);
        }
        else if (export_type == ExportType::BIN) {
            QString filename = path + wavename + ".bin";
            NPYorBINExportTrace(infile, *traceentry, filename.toStdString(), true);
        }
    }
}

bool PMbrowserWindow::choosePathAndPrefix(QString& path, QString& prefix, ExportType& export_type, bool& pxp_export, bool& create_datafolders, int & last_folder_level)
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
        export_type = dlg.export_type;
        pxp_export = dlg.pxp_export;
        create_datafolders = dlg.create_datafolders;
        last_folder_level = dlg.level_last_folder + hkTreeNode::LevelGroup; // combo box starts with Group
        lastexportpath = path;

        QSettings settings;
        settings.setValue("pmbrowserwindow/lastexportpath", lastexportpath);
        //settings_modified = true;

        return true;
    }
    else {
        return false;
    }
}

void PMbrowserWindow::exportAllVisibleTraces()
{
    if (!assertDatFileOpen()) {
        return;
    }
    
    QString path, prefix;
    ExportType export_type{};
    bool pxp_export{}, create_datafolders{};
    int folder_level{};
    std::ofstream outfile;
    if (choosePathAndPrefix(path, prefix, export_type, pxp_export, create_datafolders, folder_level)) {
        if (export_type==ExportType::Igor && pxp_export) {
            // we need filename for pxp file
            auto suggested_filename = path + currentFile.split(".").first() + ".pxp";
            auto filename = QFileDialog::getSaveFileName(this, "Save IgorPro PXP File", suggested_filename, "pxp File (*.pxp)");
            if (filename.length() == 0) return;
            QFileInfo export_path_info(filename);
            lastexportpath = export_path_info.absolutePath() + "/";
            QSettings settings;
            settings.setValue("pmbrowserwindow/lastexportpath", lastexportpath);
            outfile.open(filename.toStdString(), std::ios::binary | std::ios::out);
            if (!outfile) {
                QString msg = QString("Error while opening file:\n%1").arg(filename);
                QMessageBox::warning(this, QString("Error"), msg);
                return;
            }
        }
        try {
            if(export_type==ExportType::NPYarray) {
                auto tree = getVisibleNodes();
                hkLib::NPYExportTreeSweepsAsArray(infile, tree, path.toStdString(),
                    prefix.toStdString(), true);
            }
            else {
                // TODO: use getVisibleTraces and move the following to hekatoolslib
                int N = ui->treePulse->topLevelItemCount();
                for (int i = 0; i < N; ++i) {
                    if (export_type == ExportType::Igor && pxp_export) {
                        exportSubTree(ui->treePulse->topLevelItem(i), path, prefix, export_type, &outfile, create_datafolders, folder_level);
                    }
                    else {
                        exportSubTree(ui->treePulse->topLevelItem(i), path, prefix, export_type, nullptr, false, 0);
                    }
                }
                if (export_type == ExportType::Igor && pxp_export && create_datafolders) {
                    WriteIgorProcedureRecord(outfile);
                }
            }
        }
        catch (std::exception& e) {
            QString msg = QString("Error while exporting:\n%1").arg(QString(e.what()));
            QMessageBox::warning(this, QString("Error"), msg);
        }
    }
}

void PMbrowserWindow::formatStimMetadataAsTableExport(std::ostream& os, int max_level)
{
    if (max_level > hkTreeNode::LevelTrace) {
        throw std::runtime_error("max_level exceeds LevelTrace(=4)");
    }
    DatFile::metadataCreateTableHeader(os);
    try {
        int N = ui->treePulse->topLevelItemCount();
        for (int i = 0; i < N; ++i) { // level: group
            const auto tli = ui->treePulse->topLevelItem(i);
            if (tli->isHidden()) continue;
            const auto& grp = *(tli->data(0, Qt::UserRole).value<hkTreeNode*>());
            auto gpr_count = grp.extractValue<std::int32_t>(GrGroupCount);
            std::string grp_entry = formatParamListExportTable(grp, parametersGroup);
            int Nse = tli->childCount();
            for (int j = 0; j < Nse; ++j) { // level: series
                const auto se_item = tli->child(j);
                if (se_item->isHidden()) continue;
                const auto& series = *(se_item->data(0, Qt::UserRole).value<hkTreeNode*>());
                auto se_count = series.extractValue<std::int32_t>(SeSeriesCount);
                std::string se_entry = formatParamListExportTable(series, parametersSeries);
                int M = se_item->childCount();
                for (int k = 0; k < M; ++k) { // level: sweep
                    const auto sw_item = se_item->child(k);
                    if (sw_item->isHidden()) continue;
                    const auto& sweep = *(sw_item->data(0, Qt::UserRole).value<hkTreeNode*>());
                    auto sw_count = sweep.extractValue<std::int32_t>(SwSweepCount);
                    std::string sw_entry = formatParamListExportTable(sweep, parametersSweep);
                    int Nsw = sw_item->childCount();
                    for (int l = 0; l < Nsw; ++l) { // level: trace
                        const auto tr_item = sw_item->child(l);
                        if (tr_item->isHidden()) continue;
                        const auto& trace = *(tr_item->data(0, Qt::UserRole).value<hkTreeNode*>());
                        auto tr_count = trace.extractValue<std::int32_t>(TrTraceCount);
                        std::string tr_entry = formatParamListExportTable(trace, parametersTrace);
                        os << gpr_count << '\t' << se_count << '\t' << sw_count << '\t'
                            << tr_count <<
                            grp_entry << se_entry << sw_entry << tr_entry << '\n';
                        if (max_level < hkTreeNode::LevelTrace) break;
                    }
                    if (max_level < hkTreeNode::LevelSweep) break;
                }
                if(max_level < hkTreeNode::LevelSeries) break;
            }
            if (max_level < hkTreeNode::LevelGroup) break;
        }
    }
    catch (std::exception& e) {
        QString msg = QString("Error while exporting:\n%1").arg(QString(e.what()));
        QMessageBox::warning(this, QString("Error"), msg);
    }
}


void PMbrowserWindow::exportSubTreeAsIBW(QTreeWidgetItem* root)
{
    QString path, prefix;
    ExportType export_type{};
    bool pxp_export, create_datafolders;
    int folder_level{};
    std::ofstream outfile;

    if (choosePathAndPrefix(path, prefix, export_type, pxp_export, create_datafolders, folder_level)) {
        if (export_type==ExportType::Igor && pxp_export) {
            // we need filename for pxp file
            auto filename = QFileDialog::getSaveFileName(this, "Save IgorPro PXP File", path + "untitled.pxp", "pxp File (*.pxp)");
            if (filename.length() == 0) return;
            outfile.open(filename.toStdString(), std::ios::binary | std::ios::out);
            if (!outfile) {
                QString msg = QString("Error while opening file:\n%1").arg(filename);
                QMessageBox::warning(this, QString("Error"), msg);
                return;
            }
        }
        try {
            if (export_type == ExportType::Igor && pxp_export) {
                exportSubTree(root, path, prefix, export_type, &outfile, create_datafolders, folder_level);
            }
            else {
                exportSubTree(root, path, prefix, export_type, nullptr, false, 0);
            }
            if (export_type == ExportType::Igor && pxp_export && create_datafolders) {
                WriteIgorProcedureRecord(outfile);
            }
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
}

void PMbrowserWindow::on_actionExport_IBW_File_triggered()
{
    if (!assertDatFileOpen()) {
        return;
    }
    auto item = ui->treePulse->currentItem();
    if(!item){
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
        ExportType export_type{};
        bool pxp_export, create_datafolders;
        int folder_level{};
        if (choosePathAndPrefix(path, prefix, export_type, pxp_export, create_datafolders, folder_level)) {
            if (pxp_export) {
                QMessageBox::warning(this, "Error", "pxp export for\nthis option\nnot yet implimented");
                return;
            }
            ui->textEdit->append("exporting...");
            try {
                auto err = ExportAllTraces(infile, *datfile, path.toStdString(), prefix.toStdString());
                if(err & hkLib::WARNFLAG_WNAMETRUNCATED) {
                    ui->textEdit->append("wavename(s) truncated in export");
                }
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

void PMbrowserWindow::on_actionExport_Metadata_as_Table_triggered()
{
    if (!assertDatFileOpen()) {
        return;
    }
    DlgExportMetadata dlg(this);
    if (dlg.exec()) {
        try {
            hkLib::locale_manager lm;
            if (dlg.useSystemLocale()) {
                // for macOS, we need to jump to some hoops
                lm.setLocale(QLocale::system().name().toUtf8());
            }
            else {
                lm.setLocale("C");
            }
            auto selected = dlg.getSelection();
            if (selected < 0)
            {
                selected = hkTreeNode::LevelTrace;
            }
            else {
                ++selected; // first item in box is level 1
            }
            if (dlg.doCopy() || dlg.doShow()) {
                std::ostringstream s;
                this->formatStimMetadataAsTableExport(s, selected);
                QString txt = QString::fromUtf8(s.str());
                if(dlg.doCopy()) QGuiApplication::clipboard()->setText(txt);
                else if (dlg.doShow()) showCSVtxtInDialog(txt, true, false);
            }
            else {
                auto export_file_name = QFileDialog::getSaveFileName(this, "Export Metadata as TXT",
                    lastexportpath, "tab separated file (*.txt *.csv)");
                if (export_file_name.length() > 0) {
                    std::ofstream export_file(export_file_name.toStdString());
                    if (!export_file) {
                        QMessageBox::warning(this, "Error",
                            QString("Cannot open file '%1'\nfor saving").arg(export_file_name));
                        return;
                    }
                    this->formatStimMetadataAsTableExport(export_file, selected);
                }
            }
        }
        catch (const std::exception& e) {
            QMessageBox::warning(this, "Error while exporting", e.what());
        }
    }
}

void PMbrowserWindow::on_actionAbout_triggered()
{
    QString txt = "<b>" + myAppName + "</b>, Version " + appVersion +
                                "<br>" + COPYRIGHT_NOTICE +
                                "<p>An open source tool to handle PatchMaster Files.<br>"
                                "For help and further info see <a href="+ PROJECT_HOMEPAGE +">project homepage</a>.</p>"
                                "<p>PatchMaster is a trademark of Heka GmbH</p>"
                                "<p>Build using Qt Library version " + QT_VERSION_STR +
                                "</p><p>License: GNU General Public License Version 3 (GPLv3)</p>";
    QMessageBox msg(this);
    msg.setTextFormat(Qt::RichText);
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
        auto actSetAsTime0 = menu.addAction("set as time reference");
        QAction* actAmpstate = nullptr, * actDrawStim = nullptr,
            * actUseStimAsX{}, * actDrawSeriesStim = nullptr, * actStimulusProtocol{nullptr};
        const auto node = item->data(0, Qt::UserRole).value<hkTreeNode*>();
        if (node->getLevel() == hkTreeNode::LevelSeries) {
            menu.addSeparator();
            actAmpstate = menu.addAction("amplifier state");
            actDrawSeriesStim = menu.addAction("draw stimuli");
            actStimulusProtocol = menu.addAction("show stimulus / pgf protocol");
        }
        if (node->getLevel() == hkTreeNode::LevelSweep) {
            menu.addSeparator();
            actDrawStim = menu.addAction("show stimulus trace");
            actStimulusProtocol = menu.addAction("show stimulus / pgf protocol");
            actUseStimAsX = menu.addAction("use stim. as x trace");
        }
        auto response = menu.exec(ui->treePulse->mapToGlobal(pos));
        try {
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
        else if (response == actSetAsTime0) {
            node->setAsTime0();
        }
        else if (actAmpstate != nullptr && actAmpstate == response) {
            printAmplifierState(node);
        } else if (actDrawSeriesStim != nullptr && response == actDrawSeriesStim) {
            drawStimuliSeries(node);
        }
        else if (actDrawStim != nullptr && actDrawStim == response) {
            drawStimulus(node);
        }
        else if(actStimulusProtocol != nullptr && response == actStimulusProtocol){
            if(node->getLevel() == hkTreeNode::LevelSweep) {
                printStimProtocol(node); } else if(node->getLevel() == hkTreeNode::LevelSeries){
                printStimProtocol(&node->Children.at(0));
            }
        }
        else if (actUseStimAsX && actUseStimAsX == response) {
            if (ui->renderArea->noData() || ui->renderArea->YtraceHasX()) {
                QMessageBox::information(this, "Notice", "First, select a data-trace!"); 
            }
            else {
                useStimAsX(node);
            }
        }
        }
        catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what());
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

void PMbrowserWindow::on_treePulse_itemDoubleClicked(QTreeWidgetItem* item, int column)
{
    (void)column;
    if (item != nullptr) {
        auto node = item->data(0, Qt::UserRole).value<hkTreeNode*>();
        auto level = node->getLevel();
        if (/*level >= hkTreeNode::LevelSeries &&*/ level < hkTreeNode::LevelTrace) {
            QString info = QString("Rendering child traces for '%1'.").arg(item->text(0));
            // ui->renderArea->wipeAll(); // think about this, maybe as a setting?
            std::vector<hkTreeNode*> child_traces;
            collectChildTraces(item, level, child_traces);
            animateTraceList(info, child_traces);
        }
    }
}

void PMbrowserWindow::on_actionSelect_Parameters_triggered()
{
    DlgSelectParameters dlg(this);
    if (dlg.exec()) {
        settings_modified = true;
    }
}

void ::PMbrowserWindow::printAllParameters(const QTreeWidgetItem* item)
{
    printAllParameters(item->data(0, Qt::UserRole).value<hkTreeNode*>());
}

void ::PMbrowserWindow::printAllParameters(const hkTreeNode* n)
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
    amprecord.isSwapped = series->getIsSwapped();
    auto ampstateflag = series->extractInt32(SeAmplStateFlag),
        ampstateref = series->extractInt32(SeAmplStateRef);
    if (ampstateflag > 0 || ampstateref == 0) {
        // use local amp state record
        amprecord.Data = series->Data.subspan(SeOldAmpState, AmplifierStateSize);
        std::string s;
        formatParamListPrint(amprecord, parametersAmpplifierState, s);
        ui->textEdit->append(QString("Amplifier State:\n%1\n").arg(QString::fromUtf8(s)));
    }
    else {
        const auto& amproot = datfile->GetAmpTree().GetRootNode();
        const auto& ampse = amproot.Children.at(static_cast<std::size_t>(ampstateref) - 1); // Is this correct? Or seCount?
        for(const auto& ampre : ampse.Children) { // there might be multiple amplifiers
            auto ampstatecount = ampre.extractInt32(AmStateCount);
            amprecord.Data = ampre.Data.subspan(AmAmplifierState, AmplifierStateSize);
            std::string s;
            formatParamListPrint(amprecord, parametersAmpplifierState, s);
            ui->textEdit->append(QString("Amplifier State (Amp #%1):\n%2\n").arg(ampstatecount).arg(QString::fromUtf8(s)));
        }
    }

}

void PMbrowserWindow::showCSVtxtInDialog(const QString& txt, bool hasHorzHeader, bool hasVertHeader)
{
    TxtTableModel model(txt,hasHorzHeader, hasVertHeader);
    auto tv = new QTableView();
    tv->setModel(&model);
    auto btn_copy = new QPushButton("copy");
    auto btn_close = new QPushButton("close");
    QGridLayout* grid = new QGridLayout;
    grid->addWidget(tv, 0, 0, 1, 3);
    grid->addWidget(btn_close, 1, 0);
    grid->addWidget(btn_copy, 1, 1);
    grid->addItem(new QSpacerItem(1, 1), 1 ,2);
    grid->setRowStretch(0, 1);
    grid->setColumnStretch(2, 1);
    QDialog dlg(this);
    dlg.setWindowFlag(Qt::WindowMaximizeButtonHint, true);
    dlg.setLayout(grid);
    const QRect availableGeometry = this->screen()->availableGeometry();
    dlg.resize(availableGeometry.width() * 2 / 3, availableGeometry.height() * 2 / 3);
    dlg.move(availableGeometry.left() +
        (availableGeometry.width() - dlg.width()) / 2,
        availableGeometry.top() +
        (availableGeometry.height() - dlg.height()) / 2);
    QObject::connect(btn_close, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(btn_copy, &QPushButton::clicked, this, [&]{
        QGuiApplication::clipboard()->setText(txt);
    });
    dlg.exec();
}

void PMbrowserWindow::printStimProtocol(const hkLib::hkTreeNode* sweep)
{
    try{
    assert(sweep->getLevel() == hkTreeNode::LevelSweep);
    int stim_index = sweep->extractInt32(SwStimCount) - 1;
    //int sweep_index = sweep->extractInt32(SwSweepCount) - 1;
    const auto& stim_node = datfile->GetPgfTree().GetRootNode().Children.at(stim_index);
    std::stringstream s;
    //s << "Stimulation record #\t" << (stim_index + 1) << "\nsweep #\t" << (sweep_index+1)  << "\n";
    hkLib::stimRecordToCSV(stim_node, s, false, true, false);
    QString txt = QString::fromUtf8(s.str());
    showCSVtxtInDialog(txt, false, true);
    //ui->textEdit->append(QString::fromUtf8(s.str()));
    } catch(const std::exception& e){
        QMessageBox::warning(this,"Error","Error while printing list:\n" + QString::fromUtf8(e.what()));
    }
}

void PMbrowserWindow::create_stim_trace(const hkTreeNode* sweep, DisplayTrace& dt) const
{
    assert(sweep->getLevel() == hkTreeNode::LevelSweep);
        int stim_index = sweep->extractInt32(SwStimCount) - 1,
        sweep_index = sweep->extractInt32(SwSweepCount) - 1;
    StimRootRecord root(datfile->GetPgfTree().GetRootNode());
    const auto& stim = root.Stims.at(stim_index);
    auto stim_trace = stim.constructStimTrace(sweep_index);
    dt = DisplayTrace{ stim_trace, stim.getStimChannel().DacUnit };
}

void PMbrowserWindow::drawStimulus(const hkTreeNode* sweep)
{
    DisplayTrace dt{};
    create_stim_trace(sweep, dt);
    ui->renderArea->addTrace(std::move(dt));
}

void PMbrowserWindow::useStimAsX(const hkTreeNode* sweep)
{
    try {
        DisplayTrace dt{};
        create_stim_trace(sweep, dt);
        ui->renderArea->createInterpolatedXtrace(std::move(dt));
    }
    catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void PMbrowserWindow::drawStimuliSeries(const hkTreeNode* series)
{
    assert(series->getLevel() == hkTreeNode::LevelSeries);
    for (const auto& sweep : series->Children) {
        drawStimulus(&sweep);
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

void PMbrowserWindow::openHelp()
{
    if (!QDesktopServices::openUrl(help_url) && help_url.isLocalFile()) {
        auto res = QMessageBox::question(this, "Error",
            "Could not open local help url.\nDo you want to try the online help instead?");
        if (res == QMessageBox::Yes) {
            QDesktopServices::openUrl(QString(PROJECT_HOMEPAGE));
        }
    }
}

void PMbrowserWindow::openPreferences()
{
    DlgPreferences dlg(this);
    dlg.exec();
}

void PMbrowserWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->dropAction() == Qt::CopyAction) {
        auto mimedata = event->mimeData();
        if (mimedata->hasUrls()) {
            auto urls = mimedata->urls();
            if (urls.length() == 1) { // only accept 1 file at a time
                auto& url = urls[0];
                if (url.isLocalFile()) {
                    auto filename = url.toLocalFile();
                    if (filename.endsWith(".dat")) {
                        event->acceptProposedAction();
                    }
                }
            }
        }
    }
}

void PMbrowserWindow::dropEvent(QDropEvent* event)
{
    if (event->dropAction() == Qt::CopyAction) {
        auto mimedata = event->mimeData();
        if (mimedata->hasUrls()) {
            auto urls = mimedata->urls();
            if (urls.length() == 1) {
                auto& url = urls[0];
                if (url.isLocalFile()) {
                    auto filename = url.toLocalFile();
                    if (filename.endsWith(".dat")) {
                        loadFile(filename);
                        // check file has been loaded:
                        if(datfile) event->acceptProposedAction();
                    }
                }
            }
        }
    }
}

void PMbrowserWindow::closeEvent(QCloseEvent* event)
{
	saveSettings();
	ui->renderArea->saveSettings();
	event->accept();
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

    settings.beginGroup("params_root");
    for (const auto& p : parametersRoot) {
        settings.setValue(p.name, p.toInt());
    }
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
    settings.beginGroup("params_ampstate");
    for (const auto& p : parametersAmpplifierState) {
        settings.setValue(p.name, p.toInt());
    }
    settings.endGroup();
    settings.beginGroup("params_stimulation");
    for (const auto& p : parametersStimulation) {
        settings.setValue(p.name, p.toInt());
    }
    settings.endGroup();
    settings.beginGroup("params_channel");
    for (const auto& p : parametersChannel) {
        settings.setValue(p.name, p.toInt());
    }
    settings.endGroup();
    settings.beginGroup("params_stimsegment");
    for (const auto& p : parametersStimSegment) {
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

    settings.beginGroup("params_root");
    for (auto& p : parametersRoot) {
        p.fromInt(settings.value(p.name, p.toInt()).toInt());
    }
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
    settings.beginGroup("params_ampstate");
    for (auto& p : parametersAmpplifierState) {
        p.fromInt(settings.value(p.name, p.toInt()).toInt());
    }
    settings.endGroup();
    settings.beginGroup("params_stimulation");
    for (auto& p : parametersStimulation) {
               p.fromInt(settings.value(p.name, p.toInt()).toInt());
    }
    settings.endGroup();
    settings.beginGroup("params_channel");
    for (auto& p : parametersChannel) {
               p.fromInt(settings.value(p.name, p.toInt()).toInt());
    }
    settings.endGroup();
    settings.beginGroup("params_stimsegment");
    for (auto& p : parametersStimSegment) {
               p.fromInt(settings.value(p.name, p.toInt()).toInt());
    }
    settings.endGroup();

    settings.beginGroup("Preferences");
	int t = settings.value("selectionButton", 0).toInt();
	if(t == 2) {
		global_hkSettings.ext_Vmon = settings.value("Vmon", "Vmon").toString().toStdString();
		global_hkSettings.ext_Imon = settings.value("Imon", "Imon").toString().toStdString();
	} else if(t == 1) {
		global_hkSettings.ext_Vmon.clear();
		global_hkSettings.ext_Imon.clear();
	}
    settings.endGroup();
}
