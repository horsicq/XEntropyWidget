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
    XDialogProcess(pParent),
    ui(new Ui::DialogEntropyProcess)
{
    ui->setupUi(this);
    g_pEntropyProcess=nullptr;
    g_pThread=nullptr;

    ui->progressBarMain->setMinimum(0);
    ui->progressBarMain->setMaximum(1000);
    ui->progressBarOpt->setMinimum(0);
    ui->progressBarOpt->setMaximum(1000);
}

DialogEntropyProcess::DialogEntropyProcess(QWidget *pParent,QIODevice *pDevice,EntropyProcess::DATA *pData,bool bGraph,bool bRegions,qint32 nMax) :
    DialogEntropyProcess(pParent)
{
    setData(pDevice,pData,bGraph,bRegions,nMax);
}

DialogEntropyProcess::~DialogEntropyProcess()
{
    stop();

    g_pThread->quit();
    g_pThread->wait();

    delete ui;

    delete g_pThread;
    delete g_pEntropyProcess;
}

void DialogEntropyProcess::setData(QIODevice *pDevice,EntropyProcess::DATA *pData,bool bGraph,bool bRegions,qint32 nMax)
{
    g_pEntropyProcess=new EntropyProcess;
    g_pThread=new QThread;

    g_pEntropyProcess->moveToThread(g_pThread);

    connect(g_pThread,SIGNAL(started()),g_pEntropyProcess,SLOT(process()));
    connect(g_pEntropyProcess,SIGNAL(completed(qint64)),this,SLOT(onCompleted(qint64)));
    connect(g_pEntropyProcess,SIGNAL(errorMessage(QString)),this,SLOT(errorMessage(QString)));

    g_pEntropyProcess->setData(pDevice,pData,bGraph,bRegions,nMax,getPdStruct());
    g_pThread->start();
}

void DialogEntropyProcess::_timerSlot()
{
    if(getPdStruct()->pdRecord.nTotal)
    {
        ui->progressBarMain->setValue((getPdStruct()->pdRecord.nCurrent*1000)/(getPdStruct()->pdRecord.nTotal));
    }

    if(getPdStruct()->pdRecordOpt.nTotal)
    {
        ui->progressBarOpt->setValue((getPdStruct()->pdRecordOpt.nCurrent*1000)/(getPdStruct()->pdRecordOpt.nTotal));
    }
}

void DialogEntropyProcess::on_pushButtonCancel_clicked()
{
    stop();
}
