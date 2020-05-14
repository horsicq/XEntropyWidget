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

    emit progressValueMinimum(0);
    emit progressValueMaximum(N_MAX_GRAPH);

    pData->dTotalEntropy=binary.getEntropy(pData->nOffset,pData->nSize);
    pData->byteCounts=binary.getByteCounts(pData->nOffset,pData->nSize);

    if(XBinary::isPacked(pData->dTotalEntropy))
    {
        pData->sStatus=tr("packed");
    }
    else
    {
        pData->sStatus=tr("not packed");
    }

    pData->nMaxGraph=N_MAX_GRAPH;

    qint64 nGraph=(pData->nSize)/pData->nMaxGraph;

    if(nGraph)
    {
//        for(int i=0;i<=pData->nMaxGraph;i++)
//        {
//            pData->dOffset[i]=pData->nOffset+i*nGraph;
//            pData->dOffsetEntropy[i]=pData->dTotalEntropy;
//        }
        for(int i=0;(i<pData->nMaxGraph)&&(!bIsStop);i++)
        {
            pData->dOffset[i]=pData->nOffset+i*nGraph;

            pData->dOffsetEntropy[i]=binary.getEntropy(pData->nOffset+i*nGraph,qMin(nGraph*(pData->nMaxGraph/10),pData->nSize-(i*nGraph)));

            emit progressValueChanged(i);
        }

        pData->dOffset[pData->nMaxGraph]=pData->nOffset+pData->nSize;
        pData->dOffsetEntropy[pData->nMaxGraph]=pData->dOffsetEntropy[pData->nMaxGraph-1];
    }

    bIsStop=false;

    emit completed(scanTimer.elapsed());
}


