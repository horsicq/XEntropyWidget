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
#include "dialogentropyprocess.h"
#include "ui_dialogentropyprocess.h"

DialogEntropyProcess::DialogEntropyProcess(QWidget *pParent) :
    QDialog(pParent),
    ui(new Ui::DialogEntropyProcess)
{
    ui->setupUi(this);
    g_bIsStop=false;
}

DialogEntropyProcess::DialogEntropyProcess(QWidget *pParent,QIODevice *pDevice, EntropyProcess::DATA *pData, bool bGraph, bool bRegions,qint32 nMax) :
    DialogEntropyProcess(pParent)
{
    setData(pDevice,pData,bGraph,bRegions,nMax);
}

DialogEntropyProcess::~DialogEntropyProcess()
{
    g_pEntropyProcess->stop();

    g_pThread->quit();
    g_pThread->wait();

    delete ui;

    delete g_pThread;
    delete g_pEntropyProcess;
}

void DialogEntropyProcess::setData(QIODevice *pDevice, EntropyProcess::DATA *pData, bool bGraph, bool bRegions, qint32 nMax)
{
    g_pEntropyProcess=new EntropyProcess;
    g_pThread=new QThread;

    g_pEntropyProcess->moveToThread(g_pThread);

    connect(g_pThread,SIGNAL(started()),g_pEntropyProcess,SLOT(process()));
    connect(g_pEntropyProcess,SIGNAL(completed(qint64)),this,SLOT(onCompleted(qint64)));
    connect(g_pEntropyProcess,SIGNAL(errorMessage(QString)),this,SLOT(errorMessage(QString)));
    connect(g_pEntropyProcess,SIGNAL(progressValueChangedMain(qint32)),this,SLOT(progressValueChangedMain(qint32)));
    connect(g_pEntropyProcess,SIGNAL(progressValueMinimumMain(qint32)),this,SLOT(progressValueMinimumMain(qint32)));
    connect(g_pEntropyProcess,SIGNAL(progressValueMaximumMain(qint32)),this,SLOT(progressValueMaximumMain(qint32)));
    connect(g_pEntropyProcess,SIGNAL(progressValueChangedOpt(qint32)),this,SLOT(progressValueChangedOpt(qint32)));
    connect(g_pEntropyProcess, SIGNAL(progressValueMinimumOpt(qint32)), this, SLOT(progressValueMinimumOpt(qint32)));
    connect(g_pEntropyProcess, SIGNAL(progressValueMaximumOpt(qint32)), this, SLOT(progressValueMaximumOpt(qint32)));

    g_pEntropyProcess->setData(pDevice,pData,bGraph,bRegions,nMax);
    g_pThread->start();
}

void DialogEntropyProcess::on_pushButtonCancel_clicked()
{
    g_bIsStop=true;

    g_pEntropyProcess->stop();
}

void DialogEntropyProcess::errorMessage(QString sText)
{
    QMessageBox::critical(XOptions::getMainWidget(this),tr("Error"),sText);
}

void DialogEntropyProcess::onCompleted(qint64 nElapsed)
{
    Q_UNUSED(nElapsed)

    if(!g_bIsStop)
    {
        accept();
    }
    else
    {
        reject();
    }
}

void DialogEntropyProcess::progressValueChangedMain(qint32 nValue)
{
    ui->progressBarMain->setValue(nValue);
}

void DialogEntropyProcess::progressValueMaximumMain(qint32 nValue)
{
    ui->progressBarMain->setMaximum(nValue);
}

void DialogEntropyProcess::progressValueMinimumMain(qint32 nValue)
{
    ui->progressBarMain->setMinimum(nValue);
}

void DialogEntropyProcess::progressValueChangedOpt(qint32 nValue)
{
    ui->progressBarOpt->setValue(nValue);
}

void DialogEntropyProcess::progressValueMaximumOpt(qint32 nValue)
{
    ui->progressBarOpt->setMaximum(nValue);
}

void DialogEntropyProcess::progressValueMinimumOpt(qint32 nValue)
{
    ui->progressBarOpt->setValue(nValue);
}
