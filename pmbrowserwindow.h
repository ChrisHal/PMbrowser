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
    void on_treePulse_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void loadFile(QString filename);
    void loadFile() {loadFile(currentFile);};
    void closeFile();
    void populateTreeView();
    void traceSelected(QTreeWidgetItem* item, hkTreeNode* trace);
    Ui::PMbrowserWindow *ui;
    QString currentFile;
    std::ifstream infile;
    DatFile* datfile;
};
#endif // PMBROWSERWINDOW_H
