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
#include "entropyprocess.h"

EntropyProcess::EntropyProcess(QObject *parent) : QObject(parent)
{
    connect(&binary,SIGNAL(entropyProgressValueChanged(qint32)),this,SIGNAL(progressValueChanged1(qint32)));
    connect(&binary,SIGNAL(entropyProgressMinimumChanged(qint32)),this,SIGNAL(progressValueMinimum1(qint32)));
    connect(&binary,SIGNAL(entropyProgressMaximumChanged(qint32)),this,SIGNAL(progressValueMaximum1(qint32)));

    bIsStop=false;
}

void EntropyProcess::setData(QIODevice *pDevice, DATA *pData)
{
    this->pDevice=pDevice;
    this->pData=pData;
}

void EntropyProcess::stop()
{
    binary.setEntropyProcessEnable(false);
    bIsStop=true;
}

void EntropyProcess::process()
{
    QElapsedTimer scanTimer;
    scanTimer.start();

    bIsStop=false;

    binary.setData(this->pDevice);

    emit progressValueMinimum2(0);
    emit progressValueMaximum2(N_MAX_GRAPH);

    pData->dTotalEntropy=binary.getEntropy(pData->nOffset,pData->nSize);

    qint64 nGraph=(pData->nSize)/N_MAX_GRAPH;

    if(nGraph)
    {
        for(int i=0;(i<N_MAX_GRAPH)&&(!bIsStop);i++)
        {
            pData->dGraph[i]=binary.getEntropy(pData->nOffset+i*nGraph,nGraph);

            emit progressValueMaximum2(i);
        }
    }

    bIsStop=false;

    emit completed(scanTimer.elapsed());
}


