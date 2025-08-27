/*
    Copyright 2020 - 2023 Christian R. Halaszovich

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

#ifndef PMBROWSERWINDOW_H
#define PMBROWSERWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include "ui_pmbrowserwindow.h"
#include <fstream>
#include <memory>
#include "DatFile.h"
#include "DlgChoosePathAndPrefix.h"
#include <hkTreeView.h>

QT_BEGIN_NAMESPACE
namespace Ui { class PMbrowserWindow; }
QT_END_NAMESPACE

class PMbrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    PMbrowserWindow(QWidget *parent = nullptr);
    ~PMbrowserWindow();
    void loadFile(QString filename);

private slots:
    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void on_actionClear_Text_triggered();
    void on_actionExport_IBW_File_triggered();
    void on_actionExport_All_as_IBW_triggered();
    void on_actionExport_Metadata_as_Table_triggered();
    //void on_actionCopy_triggered();
    void on_actionAbout_triggered();
    void on_actionFilter_triggered();
    void on_actionRemove_Filter_triggered();
    void on_actionExport_All_Visible_Traces_as_IBW_Files_triggered();
    void prepareTreeContextMenu(const QPoint& pos);
    void on_treePulse_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_treePulse_itemDoubleClicked(QTreeWidgetItem* item, int column);
    void on_actionPrint_All_Params_triggered();
    void on_menuGraph_aboutToShow();
    void openHelp();
    void openPreferences();

public slots:
    void on_actionSelect_Parameters_triggered();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void loadFile() {loadFile(currentFile);};
    void closeFile();
    void populateTreeView();
    void groupSelected(const QTreeWidgetItem* item, const hkLib::hkTreeNode* node);
    void seriesSelected(const QTreeWidgetItem* item, const hkLib::hkTreeNode* node);
    void sweepSelected(const QTreeWidgetItem* item, const hkLib::hkTreeNode* node);
    void traceSelected(const QTreeWidgetItem* item, const hkLib::hkTreeNode* trace);
    void collectChildTraces(const QTreeWidgetItem* item, int level, std::vector<hkLib::hkTreeNode*>& trace_list);
    void animateTraceList(const QString& info_text, const std::vector<hkLib::hkTreeNode*>& trace_list);
    hkLib::hkTreeView getVisibleNodes();
    void printAllParameters(const QTreeWidgetItem* item);
    void printAllParameters(const hkLib::hkTreeNode* node);
    void printAmplifierState(const hkLib::hkTreeNode* series);
    void showCSVtxtInDialog(const QString& txt, bool hasHorzHeader, bool hasVertHeader);
    void printStimProtocol(const hkLib::hkTreeNode* sweep);
    void drawStimulus(const hkLib::hkTreeNode* sweep);
    void useStimAsX(const hkLib::hkTreeNode* sweep);
    void drawStimuliSeries(const hkLib::hkTreeNode* sweep);
    void create_stim_trace(const hkLib::hkTreeNode* sweep, DisplayTrace& dt) const;
    bool assertDatFileOpen();
    void exportSubTree(QTreeWidgetItem* item, const QString& path, const QString& prefix, ExportType export_type, std::ostream *outfile, bool create_datafolders, int folder_level);
    bool choosePathAndPrefix(QString& path, QString& prefix, ExportType& export_type, bool& pxp_export, bool& create_datafolders, int & last_folder_level);
    void exportSubTreeAsIBW(QTreeWidgetItem* root);
    void exportAllVisibleTraces();
    void formatStimMetadataAsTableExport(std::ostream& os, int max_level);
    void treeSetHidden(QTreeWidgetItem* item, bool hidden);
    void unhideTreeItems(QTreeWidgetItem* item);
    void filterTree();
    void saveSettings();
    void loadSettings();
    Ui::PMbrowserWindow* ui;
    QString currentFile;
    QUrl help_url{};
    QAction actHelp{ "&Help" };
    std::ifstream infile;
    std::unique_ptr<hkLib::DatFile> datfile;
    QString lastloadpath, lastexportpath;
    QString filterStrGrp, filterStrSer, filterStrSwp, filterStrTr;
    bool settings_modified;
};
#endif // PMBROWSERWINDOW_H
