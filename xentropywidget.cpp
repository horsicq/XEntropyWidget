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
    }
}

void XEntropyWidget::on_pushButtonReload_clicked()
{
    reload();
}

void XEntropyWidget::updateRegions()
{
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

    QAbstractItemModel *pOldModel=ui->tableViewRegions->model();

    int nCount=memoryMap.listRecords.count();

    QStandardItemModel *pModel=new QStandardItemModel(nCount,4,this);

    pModel->setHeaderData(0,Qt::Horizontal,tr("Name"));
    pModel->setHeaderData(1,Qt::Horizontal,tr("Offset"));
    pModel->setHeaderData(2,Qt::Horizontal,tr("Address"));
    pModel->setHeaderData(3,Qt::Horizontal,tr("Size"));

    QColor colDisabled=QWidget::palette().color(QPalette::Window);

    for(int i=0,j=0;i<nCount;i++)
    {
        bool bIsVirtual=memoryMap.listRecords.at(i).bIsVirtual;

        if(!bIsVirtual)
        {
            QStandardItem *itemName=new QStandardItem;

            itemName->setData(memoryMap.listRecords.at(i).nOffset,Qt::UserRole+0);
            itemName->setData(memoryMap.listRecords.at(i).nAddress,Qt::UserRole+1);

            if(bIsVirtual)
            {
                itemName->setBackground(colDisabled);
            }

            itemName->setText(memoryMap.listRecords.at(i).sName);
            pModel->setItem(j,0,itemName);

            QStandardItem *itemOffset=new QStandardItem;

            if(bIsVirtual)
            {
                itemOffset->setBackground(colDisabled);
            }

            itemOffset->setText(XLineEditHEX::getFormatString(mode,memoryMap.listRecords.at(i).nOffset));
            pModel->setItem(j,1,itemOffset);

            QStandardItem *itemAddress=new QStandardItem;

            if(bIsVirtual)
            {
                itemAddress->setBackground(colDisabled);
            }

            itemAddress->setText(XLineEditHEX::getFormatString(mode,memoryMap.listRecords.at(i).nAddress));
            pModel->setItem(j,2,itemAddress);

            QStandardItem *itemSize=new QStandardItem;

            if(bIsVirtual)
            {
                itemSize->setBackground(colDisabled);
            }

            itemSize->setText(XLineEditHEX::getFormatString(mode,memoryMap.listRecords.at(i).nSize));
            pModel->setItem(j,3,itemSize);

            j++;
        }
    }

    ui->tableViewRegions->setModel(pModel);

    delete pOldModel;

    ui->tableViewRegions->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    ui->tableViewRegions->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Interactive);
    ui->tableViewRegions->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
    ui->tableViewRegions->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Interactive);

    qint32 nColumnSize=XLineEditHEX::getWidthFromMode(mode);

    ui->tableViewRegions->setColumnWidth(1,nColumnSize);
    ui->tableViewRegions->setColumnWidth(2,nColumnSize);
    ui->tableViewRegions->setColumnWidth(3,nColumnSize);

    connect(ui->tableViewRegions->selectionModel(),SIGNAL(selectionChanged(QItemSelection, QItemSelection)),this,SLOT(on_tableViewSelection(QItemSelection, QItemSelection)));
}

void XEntropyWidget::on_tableViewSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    qDebug("on_tableViewSelection");
}

void XEntropyWidget::on_comboBoxType_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    updateRegions();
}
