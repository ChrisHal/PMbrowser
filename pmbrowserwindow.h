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

#ifndef PMBROWSERWINDOW_H
#define PMBROWSERWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <fstream>
#include "DatFile.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PMbrowserWindow; }
QT_END_NAMESPACE

class PMbrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    PMbrowserWindow(QWidget *parent = nullptr);
    ~PMbrowserWindow();

private slots:
    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void on_actionClear_Text_triggered();
    void on_actionExport_IBW_File_triggered();
    void on_actionExport_All_as_IBW_triggered();
    void on_actionAbout_triggered();
    void on_treePulse_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void loadFile(QString filename);
    void loadFile() {loadFile(currentFile);};
    void closeFile();
    void populateTreeView();
    void groupSelected(QTreeWidgetItem* item, hkTreeNode* node);
    void seriesSelected(QTreeWidgetItem* item, hkTreeNode* node);
    void sweepSelected(QTreeWidgetItem* item, hkTreeNode* node);
    void traceSelected(QTreeWidgetItem* item, hkTreeNode* trace);
    void exportSubTree(QTreeWidgetItem* item, const QString& path, const QString& prefix);
    bool choosePathAndPrefix(QString& path, QString& prefix);
    void exportSubTreeAsIBW(QTreeWidgetItem* root);
    Ui::PMbrowserWindow *ui;
    QString currentFile;
    std::ifstream infile;
    DatFile* datfile;
    QString lastexportpath;
};
#endif // PMBROWSERWINDOW_H
