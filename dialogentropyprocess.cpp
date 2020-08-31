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
#include "dialogentropyprocess.h"
#include "ui_dialogentropyprocess.h"

DialogEntropyProcess::DialogEntropyProcess(QWidget *pParent, QIODevice *pDevice, EntropyProcess::DATA *pData, bool bGraph, bool bRegions) :
    QDialog(pParent),
    ui(new Ui::DialogEntropyProcess)
{
    ui->setupUi(this);

    pEntropyProcess=new EntropyProcess;
    pThread=new QThread;

    pEntropyProcess->moveToThread(pThread);

    connect(pThread, SIGNAL(started()), pEntropyProcess, SLOT(process()));
    connect(pEntropyProcess, SIGNAL(completed(qint64)), this, SLOT(onCompleted(qint64)));
    connect(pEntropyProcess, SIGNAL(errorMessage(QString)), this, SLOT(errorMessage(QString)));
    connect(pEntropyProcess, SIGNAL(progressValueChanged(qint32)), this, SLOT(progressValueChanged(qint32)));
    connect(pEntropyProcess, SIGNAL(progressValueMinimum(qint32)), this, SLOT(progressValueMinimum(qint32)));
    connect(pEntropyProcess, SIGNAL(progressValueMaximum(qint32)), this, SLOT(progressValueMaximum(qint32)));

    pEntropyProcess->setData(pDevice,pData,bGraph,bRegions);
    pThread->start();

    bIsStop=false;
}

DialogEntropyProcess::~DialogEntropyProcess()
{
    pEntropyProcess->stop();

    pThread->quit();
    pThread->wait();

    delete ui;

    delete pThread;
    delete pEntropyProcess;
}

void DialogEntropyProcess::on_pushButtonCancel_clicked()
{
    bIsStop=true;

    pEntropyProcess->stop();
}

void DialogEntropyProcess::errorMessage(QString sText)
{
    QMessageBox::critical(this,tr("Error"),sText);
}

void DialogEntropyProcess::onCompleted(qint64 nElapsed)
{
    Q_UNUSED(nElapsed)

    if(!bIsStop)
    {
        accept();
    }
    else
    {
        reject();
    }
}

void DialogEntropyProcess::progressValueChanged(qint32 nValue)
{
    ui->progressBar->setValue(nValue);
}

void DialogEntropyProcess::progressValueMaximum(qint32 nValue)
{
    ui->progressBar->setMaximum(nValue);
}

void DialogEntropyProcess::progressValueMinimum(qint32 nValue)
{
    ui->progressBar->setMinimum(nValue);
}
