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

XEntropyWidget::XEntropyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::XEntropyWidget)
{
    ui->setupUi(this);

    entropyData={};

    QPen penRed(Qt::red);
    pCurve=new QwtPlotCurve;
    pCurve->setPen(penRed);
    pCurve->attach(ui->widgetEntropy);
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

    if(bAuto)
    {
        const QSignalBlocker blocker(ui->comboBoxType);

        ui->comboBoxType->clear();

        SubDevice subDevice(pDevice,nOffset,nSize);

        if(subDevice.open(QIODevice::ReadOnly))
        {
            QList<XBinary::FT> listFileTypes=XBinary::_getFileTypeListFromSet(XBinary::getFileTypes(&subDevice));

            int nCount=listFileTypes.count();

            for(int i=0;i<nCount;i++)
            {
                XBinary::FT ft=listFileTypes.at(i);
                ui->comboBoxType->addItem(XBinary::fileTypeIdToString(ft),ft);
            }

            if(nCount)
            {
                ui->comboBoxType->setCurrentIndex(nCount-1);
                updateRegions();
            }

            subDevice.close();
        }

        reload();
    }
}

void XEntropyWidget::setSaveDirectory(QString sSaveDirectory)
{
    this->sSaveDirectory=sSaveDirectory;
}

void XEntropyWidget::reload()
{
    entropyData.nOffset=nOffset;
    entropyData.nSize=nSize;

    DialogEntropyProcess dep(this,pDevice,&entropyData);

    if(dep.exec()==QDialog::Accepted)
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
            QTableWidgetItem *itemByte=new QTableWidgetItem;

            itemByte->setText(QString("0x%1").arg(i,2,16,QChar('0')));
            itemByte->setTextAlignment(Qt::AlignRight);
            ui->tableWidgetBytes->setItem(i,0,itemByte);

            QTableWidgetItem *itemCount=new QTableWidgetItem;

            itemCount->setData(Qt::DisplayRole,entropyData.byteCounts.nCount[i]);
            itemCount->setTextAlignment(Qt::AlignRight);
            ui->tableWidgetBytes->setItem(i,1,itemCount);

            QTableWidgetItem *itemProcent=new QTableWidgetItem;

            itemProcent->setText(XBinary::doubleToString(((double)entropyData.byteCounts.nCount[i]*100)/entropyData.byteCounts.nSize,4));
            itemProcent->setTextAlignment(Qt::AlignRight);
            ui->tableWidgetBytes->setItem(i,2,itemProcent);
        }

        ui->tableWidgetBytes->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Interactive);
        ui->tableWidgetBytes->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
        ui->tableWidgetBytes->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);

        // TODO Size 0,2 columns

        QVector<QwtIntervalSample> samples(256);

        for(uint i=0;i< 256; i++)
        {
            QwtInterval interval(double(i),i+1.0);
            interval.setBorderFlags(QwtInterval::ExcludeMaximum);

            samples[i]=QwtIntervalSample(entropyData.byteCounts.nCount[i],interval);
        }

        pHistogram->setSamples(samples);
        ui->widgetBytes->replot();
    }
}

void XEntropyWidget::on_pushButtonReload_clicked()
{
    reload();
}

void XEntropyWidget::updateRegions()
{
    XBinary binary(pDevice);

    XBinary::FT ft=(XBinary::FT)(ui->comboBoxType->currentData().toInt());

    SubDevice subDevice(pDevice,nOffset,nSize);

    if(subDevice.open(QIODevice::ReadOnly))
    {
        XBinary::_MEMORY_MAP memoryMap=XFormats::getMemoryMap(ft,&subDevice);

        XLineEditHEX::MODE mode;

        if(memoryMap.mode==XBinary::MODE_16)
        {
            mode=XLineEditHEX::MODE_16;
        }
        else if((memoryMap.mode==XBinary::MODE_16SEG)||(memoryMap.mode==XBinary::MODE_32))
        {
            mode=XLineEditHEX::MODE_32;
        }
        else if(memoryMap.mode==XBinary::MODE_64)
        {
            mode=XLineEditHEX::MODE_64;
        }
        else if(memoryMap.mode==XBinary::MODE_UNKNOWN)
        {
            mode=XLineEditHEX::getModeFromSize(memoryMap.nRawSize);
        }
        int nCount=0;

        nCount=listZones.count();

        for(int i=0;i<nCount;i++)
        {
            listZones.at(i)->setVisible(false);
        }

        ui->widgetEntropy->replot();

        listZones.clear();

        ui->tableWidgetRegions->clear();

        ui->tableWidgetRegions->setRowCount(XBinary::getNumberOfPhysicalRecords(&memoryMap));
        ui->tableWidgetRegions->setColumnCount(5);

        QStringList slHeader;
        slHeader.append(tr("Name"));
        slHeader.append(tr("Offset"));
        slHeader.append(tr("Size"));
        slHeader.append(tr("Entropy"));
        slHeader.append(tr("Status"));

        ui->tableWidgetRegions->setHorizontalHeaderLabels(slHeader);
        ui->tableWidgetRegions->horizontalHeader()->setVisible(true);

        nCount=memoryMap.listRecords.count();

        for(int i=0,j=0;i<nCount;i++)
        {
            bool bIsVirtual=memoryMap.listRecords.at(i).bIsVirtual;

            if(!bIsVirtual)
            {
                double dEntropy=binary.getEntropy(nOffset+memoryMap.listRecords.at(i).nOffset,memoryMap.listRecords.at(i).nSize);

                QTableWidgetItem *itemName=new QTableWidgetItem;

                itemName->setText(memoryMap.listRecords.at(i).sName);
                itemName->setData(Qt::UserRole+0,memoryMap.listRecords.at(i).nOffset);
                itemName->setData(Qt::UserRole+1,memoryMap.listRecords.at(i).nSize);

                ui->tableWidgetRegions->setItem(j,0,itemName);

                QTableWidgetItem *itemOffset=new QTableWidgetItem;

                itemOffset->setText(XLineEditHEX::getFormatString(mode,memoryMap.listRecords.at(i).nOffset));
                itemOffset->setTextAlignment(Qt::AlignRight);
                ui->tableWidgetRegions->setItem(j,1,itemOffset);

                QTableWidgetItem *itemSize=new QTableWidgetItem;

                itemSize->setText(XLineEditHEX::getFormatString(mode,memoryMap.listRecords.at(i).nSize));
                itemSize->setTextAlignment(Qt::AlignRight);
                ui->tableWidgetRegions->setItem(j,2,itemSize);

                QTableWidgetItem *itemEntropy=new QTableWidgetItem;

                itemEntropy->setText(XBinary::doubleToString(dEntropy,5));
                itemEntropy->setTextAlignment(Qt::AlignRight);
                ui->tableWidgetRegions->setItem(j,3,itemEntropy);

                QTableWidgetItem *itemStatus=new QTableWidgetItem;

                itemStatus->setText(XBinary::isPacked(dEntropy)?(tr("packed")):(tr("not packed")));
                ui->tableWidgetRegions->setItem(j,4,itemStatus);

                QwtPlotZoneItem *pZone=new QwtPlotZoneItem;
                pZone->setInterval(nOffset+memoryMap.listRecords.at(i).nOffset,nOffset+memoryMap.listRecords.at(i).nOffset+memoryMap.listRecords.at(i).nSize);
                pZone->setVisible(false);
                QColor color=Qt::darkBlue;
                color.setAlpha(100);
                pZone->setPen(color);
                color.setAlpha(20);
                pZone->setBrush(color);
                pZone->attach(ui->widgetEntropy);
                listZones.append(pZone);

                j++;
            }
        }

        ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
        ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Interactive);
        ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
        ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Interactive);
        ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Interactive);

        qint32 nColumnSize=XLineEditHEX::getWidthFromMode(this,mode);

        ui->tableWidgetRegions->setColumnWidth(1,nColumnSize);
        ui->tableWidgetRegions->setColumnWidth(2,nColumnSize);

        subDevice.close();
    }
}

void XEntropyWidget::on_comboBoxType_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    updateRegions();
}

void XEntropyWidget::on_tableWidgetRegions_itemSelectionChanged()
{
    int nCount=listZones.count();

    for(int i=0;i<nCount;i++)
    {
        listZones.at(i)->setVisible(false);
    }

    QList<QTableWidgetItem*> listItems=ui->tableWidgetRegions->selectedItems();

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
    const QList<QByteArray> imageFormats=QImageWriter::supportedImageFormats();
    // TODO
}

QString XEntropyWidget::getResultName()
{
    QString sResult;

    // TODO

    return sResult;
}
