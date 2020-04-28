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

    QPen pen(Qt::red);
    pCurve=new QwtPlotCurve;
    pCurve->setPen(pen);
    pCurve->attach(ui->widgetEntropy);
//    ui->widgetEntropy->setAutoReplot();
}

XEntropyWidget::~XEntropyWidget()
{
    delete ui;
}

void XEntropyWidget::setData(QIODevice *pDevice, bool bAuto)
{
    this->pDevice=pDevice;

    if(bAuto)
    {
        const QSignalBlocker blocker(ui->comboBoxType);

        ui->comboBoxType->clear();

        QList<XBinary::FT> listFileTypes=XBinary::_getFileTypeListFromSet(XBinary::getFileTypes(pDevice));

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

        reload();
    }
}

void XEntropyWidget::reload()
{
    entropyData.nOffset=0;
    entropyData.nSize=pDevice->size();

    DialogEntropyProcess dep(this,pDevice,&entropyData);

    if(dep.exec()==QDialog::Accepted)
    {
        ui->lineEditTotalEntropy->setText(XBinary::doubleToString(entropyData.dTotalEntropy,5));

        ui->progressBarTotalEntropy->setMaximum(8*100);
        ui->progressBarTotalEntropy->setValue(entropyData.dTotalEntropy*100);

        ui->lineEditOffset->setValue32_64(entropyData.nOffset);
        ui->lineEditSize->setValue32_64(entropyData.nSize);
        ui->labelStatus->setText(entropyData.sStatus);

        pCurve->setSamples(entropyData.dOffset,entropyData.dOffsetEntropy,entropyData.nMaxGraph);
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

            itemCount->setText(QString::number(entropyData.byteCounts.nCount[i]));
            itemCount->setTextAlignment(Qt::AlignRight);
            ui->tableWidgetBytes->setItem(i,1,itemCount);
        }

        ui->tableWidgetBytes->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Interactive);
        ui->tableWidgetBytes->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
        ui->tableWidgetBytes->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
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

    XBinary::_MEMORY_MAP memoryMap=XFormats::getMemoryMap(pDevice,ft);

    XLineEditHEX::MODE mode;

    if(memoryMap.mode==XBinary::MODE_16)
    {
        mode=XLineEditHEX::MODE_16;
    }
    else if(memoryMap.mode==XBinary::MODE_32)
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

    int nCount=memoryMap.listRecords.count();

    for(int i=0,j=0;i<nCount;i++)
    {
        bool bIsVirtual=memoryMap.listRecords.at(i).bIsVirtual;

        if(!bIsVirtual)
        {
            double dEntropy=binary.getEntropy(memoryMap.listRecords.at(i).nOffset,memoryMap.listRecords.at(i).nSize);

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

            j++;
        }
    }

    ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Interactive);
    ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
    ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Interactive);
    ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Interactive);

    qint32 nColumnSize=XLineEditHEX::getWidthFromMode(mode);

    ui->tableWidgetRegions->setColumnWidth(1,nColumnSize);
    ui->tableWidgetRegions->setColumnWidth(2,nColumnSize);
}

void XEntropyWidget::on_comboBoxType_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    updateRegions();
}

void XEntropyWidget::on_tableWidgetRegions_itemSelectionChanged()
{

}
