/* Copyright (c) 2020-2022 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "xentropywidget.h"
#include "ui_xentropywidget.h"

class XProcentWidgetItem : public QTableWidgetItem // TODO move to Controls !!!
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

    g_pDevice=nullptr;
    g_nOffset=0;
    g_nSize=0;

    g_pGrid=nullptr; // TODO remove

    g_pGrid=new QwtPlotGrid;
    g_pGrid->enableXMin(true);

    QPen penRed(Qt::red);
    g_pCurve=new QwtPlotCurve;
    g_pCurve->setPen(penRed);
    g_pCurve->attach(ui->widgetEntropy);

    QPen penBlue(Qt::blue);
    g_pHistogram=new QwtPlotHistogram;
    g_pHistogram->setPen(penBlue);

    g_pHistogram->attach(ui->widgetBytes);

    ui->widgetEntropy->setAxisScale(0,0,8); // Fix
//    ui->widgetEntropy->setAxisScale(2,0,100);
//    ui->widgetEntropy->setAutoReplot();

    ui->widgetBytes->setAxisScale(2,0,256,32);
    ui->widgetBytes->updateAxes();

    g_pPicker=new QwtPlotPicker(2,0,
            QwtPlotPicker::CrossRubberBand,QwtPicker::AlwaysOn,
            ui->widgetEntropy->canvas());
//        g_pPicker->setStateMachine(new QwtPickerDragPointMachine());
    g_pPicker->setRubberBandPen(QColor(Qt::green));
    g_pPicker->setRubberBand(QwtPicker::CrossRubberBand);
    g_pPicker->setTrackerPen(QColor(Qt::red));

    ui->tabWidget->setCurrentIndex(0);

    ui->spinBoxCount->setValue(100);
}

XEntropyWidget::~XEntropyWidget()
{
    delete ui;
}

void XEntropyWidget::setData(QIODevice *pDevice,qint64 nOffset,qint64 nSize,XBinary::FT fileType,bool bAuto)
{
    g_pDevice=pDevice;
    g_nOffset=nOffset;
    g_nSize=nSize;

    if(this->g_nSize==-1)
    {
        this->g_nSize=(pDevice->size())-(this->g_nOffset);
    }

//    g_entropyData.nOffset=0; // We are using subdevice. Offset is always 0.
//    g_entropyData.nSize=this->g_nSize;
    g_entropyData.nOffset=nOffset;
    g_entropyData.nSize=nSize;

    if(fileType!=XBinary::FT_REGION)
    {
        SubDevice subDevice(g_pDevice,g_nOffset,g_nSize);

        if(subDevice.open(QIODevice::ReadOnly))
        {
            g_entropyData.fileType=XFormats::setFileTypeComboBox(fileType,&subDevice,ui->comboBoxType);

            subDevice.close();
        }
    }
    else
    {
        ui->comboBoxType->addItem(XBinary::fileTypeIdToString(fileType),fileType);
    }

    qint64 nCount=g_nSize/0x200;

    nCount=qMin(nCount,(qint64)100);

    if(nCount)
    {
        ui->spinBoxCount->setValue(nCount);
    }
    else
    {
        ui->spinBoxCount->setValue(1);
    }

    adjust();

    if(bAuto)
    {
        reload(true,true);
    }
}

void XEntropyWidget::setSaveDirectory(QString sSaveDirectory)
{
    this->g_sSaveDirectory=sSaveDirectory;
}

void XEntropyWidget::reload(bool bGraph,bool bRegions)
{
    if(g_pDevice)
    {
        DialogEntropyProcess dep(XOptions::getMainWidget(this),g_pDevice,&g_entropyData,bGraph,bRegions,ui->spinBoxCount->value());

        dep.showDialogDelay(1000);

        if(dep.isSuccess())
        {
            if(bGraph)
            {
                ui->lineEditTotalEntropy->setText(XBinary::doubleToString(g_entropyData.dTotalEntropy,5));

                ui->progressBarTotalEntropy->setMaximum(8*100);
                ui->progressBarTotalEntropy->setValue(g_entropyData.dTotalEntropy*100);

                ui->lineEditOffset->setValue32_64(g_nOffset);
                ui->lineEditSize->setValue32_64(g_nSize);
                ui->progressBarTotalEntropy->setFormat(g_entropyData.sStatus+"(%p%)");

                qint32 nNumberOfEntropies=g_entropyData.listEntropies.count();

                double *pOffsets=new double[nNumberOfEntropies];
                double *pEntropies=new double[nNumberOfEntropies];

                for(qint32 i=0;i<nNumberOfEntropies;i++)
                {
                    pOffsets[i]=g_entropyData.listEntropies.at(i).dOffset;
                    pEntropies[i]=g_entropyData.listEntropies.at(i).dEntropy;
                }

                g_pCurve->setSamples(pOffsets,pEntropies,nNumberOfEntropies);

                delete [] pOffsets;
                delete [] pEntropies;

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

                for(qint32 i=0;i<256;i++)
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

                // TODO Size 0,2 columns !!!

                QVector<QwtIntervalSample> samples(256);

                for(quint32 i=0;i<256;i++)
                {
                    QwtInterval qwtInterval(double(i),i+1.0);
                    qwtInterval.setBorderFlags(QwtInterval::ExcludeMaximum);

                    samples[i]=QwtIntervalSample(g_entropyData.byteCounts.nCount[i],qwtInterval);
                }

                g_pHistogram->setSamples(samples);
                ui->widgetBytes->replot();
            }

            if(bRegions)
            {
                qint32 nNumberOfZones=g_listZones.count();

                for(qint32 i=0;i<nNumberOfZones;i++)
                {
                    g_listZones.at(i)->setVisible(false);
                }

                ui->widgetEntropy->replot();

                g_listZones.clear();

                ui->tableWidgetRegions->clear();

                qint32 nNumberOfMemoryRecords=g_entropyData.listMemoryRecords.count();

                ui->tableWidgetRegions->setRowCount(nNumberOfMemoryRecords);
                ui->tableWidgetRegions->setColumnCount(5);

                QStringList slHeader;
                slHeader.append(tr("Offset"));
                slHeader.append(tr("Size"));
                slHeader.append(tr("Entropy"));
                slHeader.append(tr("Status"));
                slHeader.append(tr("Name"));

                ui->tableWidgetRegions->setHorizontalHeaderLabels(slHeader);
                ui->tableWidgetRegions->horizontalHeader()->setVisible(true);

                for(qint32 i=0;i<nNumberOfMemoryRecords;i++)
                {
                    QTableWidgetItem *pItemOffset=new QTableWidgetItem;

                    pItemOffset->setData(Qt::UserRole+0,g_entropyData.listMemoryRecords.at(i).nOffset);
                    pItemOffset->setData(Qt::UserRole+1,g_entropyData.listMemoryRecords.at(i).nSize);

                    pItemOffset->setText(XLineEditHEX::getFormatString(g_entropyData.mode,g_entropyData.listMemoryRecords.at(i).nOffset+g_nOffset));
                    pItemOffset->setTextAlignment(Qt::AlignRight);
                    ui->tableWidgetRegions->setItem(i,0,pItemOffset);

                    QTableWidgetItem *pItemSize=new QTableWidgetItem;

                    pItemSize->setText(XLineEditHEX::getFormatString(g_entropyData.mode,g_entropyData.listMemoryRecords.at(i).nSize));
                    pItemSize->setTextAlignment(Qt::AlignRight);
                    ui->tableWidgetRegions->setItem(i,1,pItemSize);

                    QTableWidgetItem *pItemEntropy=new QTableWidgetItem;

                    pItemEntropy->setText(XBinary::doubleToString(g_entropyData.listMemoryRecords.at(i).dEntropy,5));
                    pItemEntropy->setTextAlignment(Qt::AlignRight);
                    ui->tableWidgetRegions->setItem(i,2,pItemEntropy);

                    QTableWidgetItem *pItemStatus=new QTableWidgetItem;

                    pItemStatus->setText(g_entropyData.listMemoryRecords.at(i).sStatus);
                    pItemStatus->setTextAlignment(Qt::AlignLeft);
                    ui->tableWidgetRegions->setItem(i,3,pItemStatus);

                    QTableWidgetItem *pItemName=new QTableWidgetItem;

                    pItemName->setText(g_entropyData.listMemoryRecords.at(i).sName);
                    pItemName->setTextAlignment(Qt::AlignLeft);

                    ui->tableWidgetRegions->setItem(i,4,pItemName);

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

                ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Interactive);
                ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Interactive);
                ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
                ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Interactive);
                ui->tableWidgetRegions->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Stretch);

                qint32 nColumnSize=XLineEditHEX::getWidthFromMode(this,g_entropyData.mode);

                ui->tableWidgetRegions->setColumnWidth(0,nColumnSize);
                ui->tableWidgetRegions->setColumnWidth(1,nColumnSize);
            }
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

    g_entropyData.fileType=(XBinary::FT)(ui->comboBoxType->currentData().toInt());

    reload(false,true);
}

void XEntropyWidget::on_tableWidgetRegions_itemSelectionChanged()
{
    qint32 nNumberOfZones=g_listZones.count();

    for(qint32 i=0;i<nNumberOfZones;i++)
    {
        g_listZones.at(i)->setVisible(false);
    }

    QList<QTableWidgetItem *> listSelectedItems=ui->tableWidgetRegions->selectedItems();

    qint32 nNumberOfItems=listSelectedItems.count();

    for(qint32 i=0;i<nNumberOfItems;i++)
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
    // TODO !!!
    //XShortcutsWidget::registerShortcuts(bState);
}

void XEntropyWidget::on_pushButtonSaveEntropyTable_clicked()
{    
    QString sResultFileName=XBinary::getResultFileName(g_pDevice,QString("%1.txt").arg(tr("Strings")));

    QAbstractItemModel *pModel=nullptr;

    if(ui->tabWidget->currentIndex()==0)
    {
        pModel=ui->tableWidgetRegions->model();
    }
    else
    {
        pModel=ui->tableWidgetBytes->model();
    }

    XShortcutsWidget::saveModel(pModel,sResultFileName);
}

void XEntropyWidget::on_pushButtonSaveEntropyDiagram_clicked()
{
    const QList<QByteArray> listImageFormats=QImageWriter::supportedImageFormats();

    QStringList listFilter;

    qint32 nNumberOfImageFormats=listImageFormats.count();

    if(nNumberOfImageFormats)
    {
        QString sImageFilter=tr("Images")+" (";

        for(qint32 i=0;i<nNumberOfImageFormats;i++)
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
//    listFilter.append(QString("Postscript %1 (*.ps)").arg(tr("Documents")));

    QString sFilter=listFilter.join(";;");

    QString sFileName=XBinary::getResultFileName(g_pDevice,QString("%1.png").arg(tr("Entropy")));

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

void XEntropyWidget::on_spinBoxCount_valueChanged(int nValue)
{
    Q_UNUSED(nValue)

    adjust();
}

void XEntropyWidget::adjust()
{
    qint32 nValue=ui->spinBoxCount->value();

    if(nValue)
    {
        ui->lineEditPartSize->setValue32_64(g_nSize/nValue);
    }
    else
    {
        ui->lineEditPartSize->setValue((quint32)0);
    }
}

void XEntropyWidget::on_checkBoxGridRegions_toggled(bool bChecked)
{
    if(bChecked)
    {
        g_pGrid->attach(ui->widgetEntropy);
    }
    else
    {
        g_pGrid->detach();
    }

    ui->widgetEntropy->replot();
}
