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
