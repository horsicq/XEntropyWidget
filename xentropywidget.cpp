// copyright (c) 2020 hors<horsicq@gmail.com>
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

XEntropyWidget::XEntropyWidget(QWidget *pParent) :
    QWidget(pParent),
    ui(new Ui::XEntropyWidget)
{
    ui->setupUi(this);

    entropyData={};

    QPen penRed(Qt::red);
    pCurve=new QwtPlotCurve;
    pCurve->setPen(penRed);
    pCurve->attach(ui->widgetEntropy);
    ui->widgetEntropy->setAxisScale(0,0,8); // Fix
//    ui->widgetEntropy->setAutoReplot();

    QPen penBlue(Qt::blue);
    pHistogram=new QwtPlotHistogram;
    pHistogram->setPen(penBlue);

    pHistogram->attach(ui->widgetBytes);

    ui->widgetBytes->setAxisScale(2,0,256,32);
    ui->widgetBytes->updateAxes();

    ui->tabWidget->setCurrentIndex(0);
}

XEntropyWidget::~XEntropyWidget()
{
    delete ui;
}

void XEntropyWidget::setData(QIODevice *pDevice,qint64 nOffset,qint64 nSize,bool bAuto)
{
    this->pDevice=pDevice;
    this->nOffset=nOffset;
    this->nSize=nSize;

    if(this->nSize==-1)
    {
        this->nSize=(pDevice->size())-(this->nOffset);
    }

    entropyData.nOffset=this->nOffset;
    entropyData.nSize=this->nSize;

    if(bAuto)
    {
        const QSignalBlocker blocker(ui->comboBoxType);

        ui->comboBoxType->clear();

        SubDevice subDevice(pDevice,nOffset,nSize);

        if(subDevice.open(QIODevice::ReadOnly))
        {
            QList<XBinary::FT> listFileTypes=XBinary::_getFileTypeListFromSet(XBinary::getFileTypes(&subDevice));

            int nNumberOfFileTypes=listFileTypes.count();

            for(int i=0;i<nNumberOfFileTypes;i++)
            {
                XBinary::FT fileType=listFileTypes.at(i);
                ui->comboBoxType->addItem(XBinary::fileTypeIdToString(fileType),fileType);
            }

            if(nNumberOfFileTypes)
            {
                ui->comboBoxType->setCurrentIndex(nNumberOfFileTypes-1);
                entropyData.fileType=(XBinary::FT)(ui->comboBoxType->currentData().toInt());
            }

            subDevice.close();
        }

        reload(true,true);
    }
}

void XEntropyWidget::setSaveDirectory(QString sSaveDirectory)
{
    this->sSaveDirectory=sSaveDirectory;
}

void XEntropyWidget::reload(bool bGraph, bool bRegions)
{
    DialogEntropyProcess dep(this,pDevice,&entropyData,bGraph,bRegions);

    if(dep.exec()==QDialog::Accepted)
    {
        if(bGraph)
        {
            ui->lineEditTotalEntropy->setText(XBinary::doubleToString(entropyData.dTotalEntropy,5));

            ui->progressBarTotalEntropy->setMaximum(8*100);
            ui->progressBarTotalEntropy->setValue(entropyData.dTotalEntropy*100);

            ui->lineEditOffset->setValue32_64(entropyData.nOffset);
            ui->lineEditSize->setValue32_64(entropyData.nSize);
            ui->labelStatus->setText(entropyData.sStatus);

            pCurve->setSamples(entropyData.dOffset,entropyData.dOffsetEntropy,entropyData.nMaxGraph+1);
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

                pItemCount->setData(Qt::DisplayRole,entropyData.byteCounts.nCount[i]);
                pItemCount->setTextAlignment(Qt::AlignRight);
                ui->tableWidgetBytes->setItem(i,1,pItemCount);

                QTableWidgetItem *pItemProcent=new QTableWidgetItem;

                // TODO setData
                pItemProcent->setText(XBinary::doubleToString(((double)entropyData.byteCounts.nCount[i]*100)/entropyData.byteCounts.nSize,4));
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

                samples[i]=QwtIntervalSample(entropyData.byteCounts.nCount[i],interval);
            }

            pHistogram->setSamples(samples);
            ui->widgetBytes->replot();
        }

        if(bRegions)
        {
            int nNumberOfZones=listZones.count();

            for(int i=0;i<nNumberOfZones;i++)
            {
                listZones.at(i)->setVisible(false);
            }

            ui->widgetEntropy->replot();

            listZones.clear();

            ui->tableWidgetRegions->clear();

            int nNumberOfMemoryRecords=entropyData.listMemoryRecords.count();

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

                pItemName->setText(entropyData.listMemoryRecords.at(i).sName);
                pItemName->setTextAlignment(Qt::AlignLeft);
                pItemName->setData(Qt::UserRole+0,entropyData.listMemoryRecords.at(i).nOffset);
                pItemName->setData(Qt::UserRole+1,entropyData.listMemoryRecords.at(i).nSize);

                ui->tableWidgetRegions->setItem(i,0,pItemName);

                QTableWidgetItem *pItemOffset=new QTableWidgetItem;

                pItemOffset->setText(XLineEditHEX::getFormatString(entropyData.mode,entropyData.listMemoryRecords.at(i).nOffset));
                pItemOffset->setTextAlignment(Qt::AlignRight);
                ui->tableWidgetRegions->setItem(i,1,pItemOffset);

                QTableWidgetItem *pItemSize=new QTableWidgetItem;

                pItemSize->setText(XLineEditHEX::getFormatString(entropyData.mode,entropyData.listMemoryRecords.at(i).nSize));
                pItemSize->setTextAlignment(Qt::AlignRight);
                ui->tableWidgetRegions->setItem(i,2,pItemSize);

                QTableWidgetItem *pItemEntropy=new QTableWidgetItem;

                pItemEntropy->setText(XBinary::doubleToString(entropyData.listMemoryRecords.at(i).dEntropy,5));
                pItemEntropy->setTextAlignment(Qt::AlignRight);
                ui->tableWidgetRegions->setItem(i,3,pItemEntropy);

                QTableWidgetItem *pItemStatus=new QTableWidgetItem;

                pItemStatus->setText(entropyData.listMemoryRecords.at(i).sStatus);
                pItemEntropy->setTextAlignment(Qt::AlignLeft);
                ui->tableWidgetRegions->setItem(i,4,pItemStatus);

                QwtPlotZoneItem *pZone=new QwtPlotZoneItem;
                pZone->setInterval(nOffset+entropyData.listMemoryRecords.at(i).nOffset,nOffset+entropyData.listMemoryRecords.at(i).nOffset+entropyData.listMemoryRecords.at(i).nSize);
                pZone->setVisible(false);
                QColor color=Qt::darkBlue;
                color.setAlpha(100);
                pZone->setPen(color);
                color.setAlpha(20);
                pZone->setBrush(color);
                pZone->attach(ui->widgetEntropy);
                listZones.append(pZone);
            }

            ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
            ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Interactive);
            ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
            ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Interactive);
            ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Interactive);

            qint32 nColumnSize=XLineEditHEX::getWidthFromMode(this,entropyData.mode);

            ui->tableWidgetRegions->setColumnWidth(1,nColumnSize);
            ui->tableWidgetRegions->setColumnWidth(2,nColumnSize);
        }
    }
}

void XEntropyWidget::on_pushButtonReload_clicked()
{
    reload(true,true);
}

void XEntropyWidget::on_comboBoxType_currentIndexChanged(int nIndex)
{
    Q_UNUSED(nIndex)

    entropyData.fileType=(XBinary::FT)(ui->comboBoxType->currentData().toInt());

    reload(false,true);
}

void XEntropyWidget::on_tableWidgetRegions_itemSelectionChanged()
{
    int nCount=listZones.count();

    for(int i=0;i<nCount;i++)
    {
        listZones.at(i)->setVisible(false);
    }

    QList<QTableWidgetItem *> listItems=ui->tableWidgetRegions->selectedItems();

    nCount=listItems.count();

    for(int i=0;i<nCount;i++)
    {
        if(listItems.at(i)->column()==0)
        {
            listZones.at(listItems.at(i)->row())->setVisible(true);
        }
    }

    ui->widgetEntropy->replot();
}

void XEntropyWidget::on_pushButtonSaveEntropy_clicked()
{
    const QList<QByteArray> listImageFormats=QImageWriter::supportedImageFormats();

    QStringList listFilter;

    int nCount=listImageFormats.count();

    if(nCount)
    {
        QString sImageFilter=tr("Images")+" (";

        for(int i=0;i<nCount;i++)
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

    QString sFileName=getResultName();

    sFileName=QFileDialog::getSaveFileName(this,tr("Save diagram"),sFileName,sFilter);

    if(!sFileName.isEmpty())
    {
        QwtPlotRenderer renderer;
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground,false);
        //        renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames,true);
        renderer.renderDocument(ui->widgetEntropy,sFileName,QSizeF(300,200),85);
    }
}

QString XEntropyWidget::getResultName()
{
    QString sResult;

    sResult="Image.png";

    return sResult;
}
