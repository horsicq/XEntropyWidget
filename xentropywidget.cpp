// copyright (c) 2020-2021 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "xentropywidget.h"
#include "ui_xentropywidget.h"

class XProcentWidgetItem : public QTableWidgetItem
{
public:
    bool operator <(const QTableWidgetItem &other) const
    {
        return text().toDouble()<other.text().toDouble();
    }
};

XEntropyWidget::XEntropyWidget(QWidget *pParent) :
    XShortcutsWidget(pParent),
    ui(new Ui::XEntropyWidget)
{
    ui->setupUi(this);

    g_entropyData={};

    QPen penRed(Qt::red);
    g_pCurve=new QwtPlotCurve;
    g_pCurve->setPen(penRed);
    g_pCurve->attach(ui->widgetEntropy);
    ui->widgetEntropy->setAxisScale(0,0,8); // Fix
//    ui->widgetEntropy->setAutoReplot();

    QPen penBlue(Qt::blue);
    g_pHistogram=new QwtPlotHistogram;
    g_pHistogram->setPen(penBlue);

    g_pHistogram->attach(ui->widgetBytes);

    ui->widgetBytes->setAxisScale(2,0,256,32);
    ui->widgetBytes->updateAxes();

    ui->tabWidget->setCurrentIndex(0);
}

XEntropyWidget::~XEntropyWidget()
{
    delete ui;
}

void XEntropyWidget::setData(QIODevice *pDevice, qint64 nOffset, qint64 nSize, XBinary::FT fileType, bool bAuto)
{
    g_pDevice=pDevice;
    g_nOffset=nOffset;
    g_nSize=nSize;

    if(this->g_nSize==-1)
    {
        this->g_nSize=(pDevice->size())-(this->g_nOffset);
    }

    g_entropyData.nOffset=0; // Cause we are using subdevice !!!
    g_entropyData.nSize=this->g_nSize;

    if(bAuto)
    {
        if(fileType!=XBinary::FT_BINARY)
        {
            SubDevice subDevice(g_pDevice,g_nOffset,g_nSize);

            if(subDevice.open(QIODevice::ReadOnly))
            {
                QSet<XBinary::FT> stFileType=XBinary::getFileTypes(&subDevice,true);
                stFileType.insert(XBinary::FT_COM);
                QList<XBinary::FT> listFileTypes=XBinary::_getFileTypeListFromSet(stFileType);

                XFormats::setFileTypeComboBox(ui->comboBoxType,&listFileTypes,fileType);

                g_entropyData.fileType=(XBinary::FT)(ui->comboBoxType->currentData().toInt());

                subDevice.close();
            }
        }
        else
        {
            ui->comboBoxType->addItem(XBinary::fileTypeIdToString(fileType),fileType);
        }

        reload(true,true);
    }
}

void XEntropyWidget::setSaveDirectory(QString sSaveDirectory)
{
    this->g_sSaveDirectory=sSaveDirectory;
}

void XEntropyWidget::reload(bool bGraph, bool bRegions)
{
    SubDevice subDevice(g_pDevice,g_nOffset,g_nSize);

    if(subDevice.open(QIODevice::ReadOnly))
    {
        DialogEntropyProcess dep(XOptions::getMainWidget(this),&subDevice,&g_entropyData,bGraph,bRegions);

        if(dep.exec()==QDialog::Accepted)
        {
            if(bGraph)
            {
                ui->lineEditTotalEntropy->setText(XBinary::doubleToString(g_entropyData.dTotalEntropy,5));

                ui->progressBarTotalEntropy->setMaximum(8*100);
                ui->progressBarTotalEntropy->setValue(g_entropyData.dTotalEntropy*100);

                ui->lineEditOffset->setValue32_64(g_nOffset);
                ui->lineEditSize->setValue32_64(g_nSize);
                ui->progressBarTotalEntropy->setFormat(g_entropyData.sStatus+"(%p%)");

                g_pCurve->setSamples(g_entropyData.dOffset,g_entropyData.dOffsetEntropy,g_entropyData.nMaxGraph+1);
                ui->widgetEntropy->replot();

                ui->tableWidgetBytes->clear();

                ui->tableWidgetBytes->setRowCount(256);
                ui->tableWidgetBytes->setColumnCount(3);

                QStringList slHeader;
                slHeader.append(tr("Byte"));
                slHeader.append(tr("Count"));
                slHeader.append(QString("%"));

                ui->tableWidgetBytes->setHorizontalHeaderLabels(slHeader);
                ui->tableWidgetBytes->horizontalHeader()->setVisible(true);

                for(int i=0;i<256;i++)
                {
                    QTableWidgetItem *pItemByte=new QTableWidgetItem;

                    pItemByte->setText(QString("0x%1").arg(i,2,16,QChar('0')));
                    pItemByte->setTextAlignment(Qt::AlignRight);
                    ui->tableWidgetBytes->setItem(i,0,pItemByte);

                    QTableWidgetItem *pItemCount=new QTableWidgetItem;

                    pItemCount->setData(Qt::DisplayRole,g_entropyData.byteCounts.nCount[i]);
                    pItemCount->setTextAlignment(Qt::AlignRight);
                    ui->tableWidgetBytes->setItem(i,1,pItemCount);

                    XProcentWidgetItem *pItemProcent=new XProcentWidgetItem;

                    pItemProcent->setText(XBinary::doubleToString(((double)g_entropyData.byteCounts.nCount[i]*100)/g_entropyData.byteCounts.nSize,4));

                    pItemProcent->setTextAlignment(Qt::AlignRight);
                    ui->tableWidgetBytes->setItem(i,2,pItemProcent);
                }

                ui->tableWidgetBytes->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Interactive);
                ui->tableWidgetBytes->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
                ui->tableWidgetBytes->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);

                // TODO Size 0,2 columns

                QVector<QwtIntervalSample> samples(256);

                for(uint i=0;i<256; i++)
                {
                    QwtInterval interval(double(i),i+1.0);
                    interval.setBorderFlags(QwtInterval::ExcludeMaximum);

                    samples[i]=QwtIntervalSample(g_entropyData.byteCounts.nCount[i],interval);
                }

                g_pHistogram->setSamples(samples);
                ui->widgetBytes->replot();
            }

            if(bRegions)
            {
                int nNumberOfZones=g_listZones.count();

                for(int i=0;i<nNumberOfZones;i++)
                {
                    g_listZones.at(i)->setVisible(false);
                }

                ui->widgetEntropy->replot();

                g_listZones.clear();

                ui->tableWidgetRegions->clear();

                int nNumberOfMemoryRecords=g_entropyData.listMemoryRecords.count();

                ui->tableWidgetRegions->setRowCount(nNumberOfMemoryRecords);
                ui->tableWidgetRegions->setColumnCount(5);

                QStringList slHeader;
                slHeader.append(tr("Name"));
                slHeader.append(tr("Offset"));
                slHeader.append(tr("Size"));
                slHeader.append(tr("Entropy"));
                slHeader.append(tr("Status"));

                ui->tableWidgetRegions->setHorizontalHeaderLabels(slHeader);
                ui->tableWidgetRegions->horizontalHeader()->setVisible(true);

                for(int i=0;i<nNumberOfMemoryRecords;i++)
                {
                    QTableWidgetItem *pItemName=new QTableWidgetItem;

                    pItemName->setText(g_entropyData.listMemoryRecords.at(i).sName);
                    pItemName->setTextAlignment(Qt::AlignLeft);
                    pItemName->setData(Qt::UserRole+0,g_entropyData.listMemoryRecords.at(i).nOffset);
                    pItemName->setData(Qt::UserRole+1,g_entropyData.listMemoryRecords.at(i).nSize);

                    ui->tableWidgetRegions->setItem(i,0,pItemName);

                    QTableWidgetItem *pItemOffset=new QTableWidgetItem;

                    pItemOffset->setText(XLineEditHEX::getFormatString(g_entropyData.mode,g_entropyData.listMemoryRecords.at(i).nOffset+g_nOffset));
                    pItemOffset->setTextAlignment(Qt::AlignRight);
                    ui->tableWidgetRegions->setItem(i,1,pItemOffset);

                    QTableWidgetItem *pItemSize=new QTableWidgetItem;

                    pItemSize->setText(XLineEditHEX::getFormatString(g_entropyData.mode,g_entropyData.listMemoryRecords.at(i).nSize));
                    pItemSize->setTextAlignment(Qt::AlignRight);
                    ui->tableWidgetRegions->setItem(i,2,pItemSize);

                    QTableWidgetItem *pItemEntropy=new QTableWidgetItem;

                    pItemEntropy->setText(XBinary::doubleToString(g_entropyData.listMemoryRecords.at(i).dEntropy,5));
                    pItemEntropy->setTextAlignment(Qt::AlignRight);
                    ui->tableWidgetRegions->setItem(i,3,pItemEntropy);

                    QTableWidgetItem *pItemStatus=new QTableWidgetItem;

                    pItemStatus->setText(g_entropyData.listMemoryRecords.at(i).sStatus);
                    pItemStatus->setTextAlignment(Qt::AlignLeft);
                    ui->tableWidgetRegions->setItem(i,4,pItemStatus);

                    QwtPlotZoneItem *pZone=new QwtPlotZoneItem;
                    pZone->setInterval(g_entropyData.listMemoryRecords.at(i).nOffset,g_entropyData.listMemoryRecords.at(i).nOffset+g_entropyData.listMemoryRecords.at(i).nSize);
                    pZone->setVisible(false);
                    QColor color=Qt::darkBlue;
                    color.setAlpha(100);
                    pZone->setPen(color);
                    color.setAlpha(20);
                    pZone->setBrush(color);
                    pZone->attach(ui->widgetEntropy);
                    g_listZones.append(pZone);
                }

                ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
                ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Interactive);
                ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
                ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Interactive);
                ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Interactive);

                qint32 nColumnSize=XLineEditHEX::getWidthFromMode(this,g_entropyData.mode);

                ui->tableWidgetRegions->setColumnWidth(1,nColumnSize);
                ui->tableWidgetRegions->setColumnWidth(2,nColumnSize);
            }
        }

        subDevice.close();
    }
}

void XEntropyWidget::on_pushButtonReload_clicked()
{
    reload(true,true);
}

void XEntropyWidget::on_comboBoxType_currentIndexChanged(int nIndex)
{
    Q_UNUSED(nIndex)

    g_entropyData.fileType=(XBinary::FT)(ui->comboBoxType->currentData().toInt());

    reload(false,true);
}

void XEntropyWidget::on_tableWidgetRegions_itemSelectionChanged()
{
    int nNumberOfZones=g_listZones.count();

    for(int i=0;i<nNumberOfZones;i++)
    {
        g_listZones.at(i)->setVisible(false);
    }

    QList<QTableWidgetItem *> listSelectedItems=ui->tableWidgetRegions->selectedItems();

    int nNumberOfItems=listSelectedItems.count();

    for(int i=0;i<nNumberOfItems;i++)
    {
        if(listSelectedItems.at(i)->column()==0)
        {
            g_listZones.at(listSelectedItems.at(i)->row())->setVisible(true);
        }
    }

    ui->widgetEntropy->replot();
}

void XEntropyWidget::registerShortcuts(bool bState)
{
    Q_UNUSED(bState)
    // TODO
}

void XEntropyWidget::on_pushButtonSaveEntropyTable_clicked()
{
    QString sFileName=XBinary::getResultFileName(g_pDevice,"entropy.txt"); // TODO

    sFileName=QFileDialog::getSaveFileName(this,tr("Save"),sFileName,QString("%1 (*.txt);;%2 (*)").arg(tr("Text files")).arg(tr("All files")));

    if(!sFileName.isEmpty())
    {
        QAbstractItemModel *pModel=nullptr;

        if(ui->tabWidget->currentIndex()==0)
        {
            pModel=ui->tableWidgetRegions->model();
        }
        else
        {
            pModel=ui->tableWidgetBytes->model();
        }

        XOptions::saveTable(pModel,sFileName);
    }
}

void XEntropyWidget::on_pushButtonSaveEntropyDiagram_clicked()
{
    const QList<QByteArray> listImageFormats=QImageWriter::supportedImageFormats();

    QStringList listFilter;

    int nNumberOfImageFormats=listImageFormats.count();

    if(nNumberOfImageFormats)
    {
        QString sImageFilter=tr("Images")+" (";

        for(int i=0;i<nNumberOfImageFormats;i++)
        {
            if(i>0)
            {
                sImageFilter+=" ";
            }

            sImageFilter+="*.";
            sImageFilter+=listImageFormats.at(i);
        }

        sImageFilter+=")";

        listFilter.append(sImageFilter);
    }

    listFilter.append(QString("PDF %1 (*.pdf)").arg(tr("Documents")));
    listFilter.append(QString("Postscript %1 (*.ps)").arg(tr("Documents")));

    QString sFilter=listFilter.join(";;");

    QString sFileName=XBinary::getResultFileName(g_pDevice,"entropy.png");

    sFileName=QFileDialog::getSaveFileName(this,tr("Save diagram"),sFileName,sFilter);

    if(!sFileName.isEmpty())
    {
        QwtPlot *pWidget=nullptr;

        if(ui->tabWidget->currentIndex()==0)
        {
            pWidget=ui->widgetEntropy;
        }
        else
        {
            pWidget=ui->widgetBytes;
        }

        QwtPlotRenderer renderer;
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground,false);
        //        renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames,true);
        renderer.renderDocument(pWidget,sFileName,QSizeF(300,200),85);
    }
}
