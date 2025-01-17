#include "ImportDataView.h"
#include "PluginManager.h"
#include "../PluginAbstract.h"
#include "QtUtilities.h"
#include "HelpWidget.h"
#include "Button.h"
#include "Label.h"
#include "MainWindow.h"
#include "Project.h"
#include <QtWidgets>


ImportDataView::ImportDataView(QWidget* parent, Qt::WindowFlags flags):QWidget(parent, flags)
{
    mBrowseBut = new Button(tr("Load CSV file..."), this);
    mExportBut = new Button(tr("Export all project data as CSV"), this);
    mHelp = new HelpWidget(this);
    mHelp->setLink("http://www.chronomodel.fr/Chronomodel_User_Manual.pdf#page=29"); //chapter 3.4.2.1 Radiocarbon dating (14C)
    
    mHelp->setText(tr("Your CSV file must contain 1 data per row. Each row must start with the datation method to use. Allowed datation methods are : 14C, AM, Gauss, Typo, TL/OSL.\nComments are allowed in your CSV. They must start with  # or // and can be placed at the end of a data row. When placed at the begining of a row, the whole row is ignored.\n You can insert information in the table, you can use two keywords: Title and Structure before info."));
    
    mTable = new ImportDataTable(this, this);
    mTable->setAlternatingRowColors(true);
    mTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mTable->setDragEnabled(true);
    mTable->setDragDropMode(QAbstractItemView::DragOnly);
    
    connect(mBrowseBut, SIGNAL(clicked()), this, SLOT(browse()));
    connect(mExportBut, SIGNAL(clicked()), this, SLOT(exportDates()));
}

ImportDataView::~ImportDataView()
{
    
}

/**
 * @brief Import data from a CSV file in the table
 * @todo File encoding must be UTF8, Unix LF !!
 * Title; toy csv file
 *  Structure; Terrestrial
 *  // just comment
 *  14C;onshore;1200;30;intcal13.14c;
 *  Structure; Event : Oceanic
 *  14C;shell;2900;36;marine04.14c;-150;20
 *  14C;oyster;3000;30;marine13.14c;200;10
 */
void ImportDataView::browse()
{
    QString currentDir = MainWindow::getInstance()->getCurrentPath();
    QString path = QFileDialog::getOpenFileName(qApp->activeWindow(), tr("Open CSV File"), currentDir, tr("CSV File (*.csv)"));
    
    if(!path.isEmpty())
    {
        QFileInfo info(path);
        mPath = info.absolutePath();
        MainWindow::getInstance()->setCurrentPath(mPath);
        
        while(mTable->rowCount() > 0)
            mTable->removeRow(0);
        
        QFile file(path);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            QList<QStringList> data;
            
           /**
            * @todo File encoding must be UTF8, Unix LF !!
            */
            //QTextCodec* codec = stream.codec();
            
            int rows = 0;
            int cols = 0;
            
            QStringList headers;
            QStringList pluginNames = PluginManager::getPluginsNames();
            
            AppSettings settings = MainWindow::getInstance()->getAppSettings();
            QString csvSep = settings.mCSVCellSeparator;
            
            // Read every lines of the file
            while(!stream.atEnd())
            {
                QString line = stream.readLine();
                QStringList values = line.split(csvSep);
                if(values.size() > 0)
                {
                    qDebug()<<values[0]<<values[0].toUpper();
                    if(isComment(values[0]) || values[0] == "")
                    {
                        continue;
                    }
                    else if(values[0].contains("title", Qt::CaseInsensitive))
                    {
                        headers << "Title";
                        
                        QStringList titleText;
                        values.push_front("");
                        for (int i=1; i<values.size(); i++) {
                            titleText.append(values[i]);
                        }
                        data << titleText;
                        cols = (values.size() > cols) ? values.size() : cols;
                        ++rows;
                    }
                    else if(values[0].contains("structure", Qt::CaseInsensitive))
                    {
                        headers << "Structure";
                        QStringList titleText;
                        values.push_front("");
                        for (int i=1; i<values.size(); i++) {
                            titleText.append(values[i]);
                        }
                        data << titleText;
                        cols = (values.size() > cols) ? values.size() : cols;
                        ++rows;
                    }
                    else if(values.size() > 0)
                    {
                        // Display the line only if we have a plugin to import it !
                        QString pluginName = values[0].simplified().toUpper();
                        if(pluginNames.contains(pluginName, Qt::CaseInsensitive))
                        {
                            headers << pluginName;
                            data << values;
                            
                            // Adapt max columns count if necessary
                            cols = (values.size() > cols) ? values.size() : cols;
                            ++rows;
                        }
                    }
                }
            }
            file.close();
            
            mTable->setRowCount(rows);
            mTable->setColumnCount(cols);
            
            for(int i=0; i<data.size(); ++i)
            {
                QStringList d = data[i];
                //qDebug() << d;
                for(int j=0; j<d.size(); ++j)
                {
                    // Skip the first column containing the plugin name (already used in the table line header)
                    
                    if(j != 0)
                    {
                        QTableWidgetItem* item = new QTableWidgetItem(d[j].simplified());
                        //if ((d[0]=="Title") || (d[0]=="Structure")) item->setBackgroundColor(Qt::red);
                        mTable->setItem(i, j-1, item);

                    }
                }
            }
            // Each line has a header containing the plugin name to use
            mTable->setVerticalHeaderLabels(headers);
            mTable->setCurrentCell(0, 0);
        }
    }
}

void ImportDataView::exportDates()
{
    QString currentDir = MainWindow::getInstance()->getCurrentPath();
    QString path = QFileDialog::getSaveFileName(qApp->activeWindow(), tr("Save as CSV"), currentDir, tr("CSV File (*.csv)"));
    
    if(!path.isEmpty())
    {
        QFileInfo info(path);
        mPath = info.absolutePath();
        MainWindow::getInstance()->setCurrentPath(mPath);
        
        QString sep = MainWindow::getInstance()->getAppSettings().mCSVCellSeparator;
        QLocale csvLocal = MainWindow::getInstance()->getAppSettings().mCSVDecSeparator == "." ? QLocale::English : QLocale::French;
        csvLocal.setNumberOptions(QLocale::OmitGroupSeparator);



        QFile file(path);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            
            Project* project = MainWindow::getInstance()->getProject();
            QJsonArray events = project->mState[STATE_EVENTS].toArray();
            stream << "Title"<< sep << project->mProjectFileName<< "\n";
            for(int i=0; i<events.size(); ++i)
            {
                QJsonObject event = events[i].toObject();
                QJsonArray dates = event[STATE_EVENT_DATES].toArray();
                
                int type = event[STATE_EVENT_TYPE].toInt();
                stream << "Structure"<< sep << ((type == Event::eKnown) ? tr("Bound") : tr("Event")) << " : ";
                stream << event[STATE_NAME].toString() << "\n";
                
                for(int j=0; j<dates.size(); ++j)
                {
                    QJsonObject date = dates[j].toObject();
                    
                    try{
                        Date d = Date::fromJson(date);
                        if(!d.isNull())
                        {
                            QStringList dateCsv = d.toCSV(csvLocal);
                            stream << dateCsv.join(sep) << "\n";
                        }
                    }
                    catch(QString error){
                        QMessageBox message(QMessageBox::Critical,
                                            qApp->applicationName() + " " + qApp->applicationVersion(),
                                            tr("Error : ") + error,
                                            QMessageBox::Ok,
                                            qApp->activeWindow(),
                                            Qt::Sheet);
                        message.exec();
                    }
                }
                stream << "\n";
            }
            file.close();
        }
    }
}

void ImportDataView::removeCsvRows(QList<int> rows)
{
    sortIntList(rows);
    for(int i=rows.size()-1; i>=0; --i)
    {
        //qDebug() << "Removing row : " << rows[i];
        //mTable->removeRow(rows[i]);
        
        for(int c=0; c<mTable->columnCount(); ++c)
        {
            QTableWidgetItem* item = mTable->item(rows[i], c);
            if(item)
                item->setBackgroundColor(QColor(100, 200, 100));
            
        }
    }
}

void ImportDataView::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.fillRect(rect(), QColor(200, 200, 200));
}

void ImportDataView::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    
    int m = 5;
    int butH = 25;
    int helpH = mHelp->heightForWidth(width() - 2*m);
    
    mBrowseBut->setGeometry(m, m, (width() - 3*m)/2, butH);
    mExportBut->setGeometry(2*m + (width() - 3*m)/2, m, (width() - 3*m)/2, butH);
    
    mTable->setGeometry(0, 2*m + butH, width(), height() - 4*m - butH - helpH);
    mHelp->setGeometry(m, height() - helpH - m, width() - 2*m, helpH);
}

// ------------------------------------------------------------------------------------

#pragma mark Table

ImportDataTable::ImportDataTable(ImportDataView* importView, QWidget* parent):QTableWidget(parent),
mImportView(importView)
{
    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(updateTableHeaders()));
}

ImportDataTable::~ImportDataTable()
{

}

QMimeData* ImportDataTable::mimeData(const QList<QTableWidgetItem*> items) const
{
    QMimeData* mimeData = new QMimeData();
    
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    
    int row = -1;
    QStringList itemStr;
    
    AppSettings settings = MainWindow::getInstance()->getAppSettings();
    QString csvSep = settings.mCSVCellSeparator;
    
    foreach(QTableWidgetItem* item, items)
    {
        if(item)
        {
            if(item->row() != row)
            {
                if(!itemStr.empty())
                    stream << itemStr.join(csvSep);
                
                itemStr.clear();
                row = item->row();
                itemStr << QString::number(row);
                
                QString pluginName = verticalHeaderItem(row)->text();
                itemStr << pluginName;
            }
            QString text = item->text();
            itemStr << text;
        }
    }
    if(!itemStr.empty())
        stream << itemStr.join(csvSep);
    
    mimeData->setData("application/chronomodel.import.data", encodedData);
    return mimeData;
}

void ImportDataTable::updateTableHeaders()
{
    QList<QTableWidgetItem*> items = selectedItems();
    QString pluginName;
    for(int i=0; i<items.size(); ++i)
    {
        QString curPluginName = verticalHeaderItem(items[i]->row())->text();
        if(pluginName.isEmpty())
            pluginName = curPluginName;
        else if(pluginName != curPluginName)
        {
            pluginName = QString();
            break;
        }
    }
    
    QStringList headers;
    int numCols = columnCount();
    
    if(!pluginName.isEmpty() && (pluginName!="Title")  && (pluginName!="Structure"))
    {
        //qDebug() << pluginName;
        PluginAbstract* plugin = PluginManager::getPluginFromName(pluginName);
        
        headers = plugin->csvColumns();
        if(plugin->wiggleAllowed())
        {
            headers << "Wiggle Type (fixed | range | gaussian)";
            headers << "Wiggle value 1 (fixed | Lower date | Average)";
            headers << "Wiggle value 2 (Upper date | Error)";
        }
        while(headers.size() < numCols)
            headers << "comment";
    }
    else if ((pluginName=="Title")  || (pluginName=="Structure"))
    {
        QStringList cols;
        cols << "Name";
        for (int i=1; i<numCols; i++) cols<<"";
        headers = cols;

    }
    else
    {
        while(headers.size() < numCols)
            headers << "?";
    }
    setHorizontalHeaderLabels(headers);
}
