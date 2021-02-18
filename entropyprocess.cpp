// copyright (c) 2020-2021 hors<horsicq@gmail.com>
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

EntropyProcess::EntropyProcess(QObject *pParent) : QObject(pParent)
{
    g_bIsStop=false;

    connect(&g_binary,SIGNAL(entropyProgressValueChanged(qint32)),this,SIGNAL(progressValueChangedOpt(qint32)));
    connect(&g_binary,SIGNAL(entropyProgressMinimumChanged(qint32)),this,SIGNAL(progressValueMinimumOpt(qint32)));
    connect(&g_binary,SIGNAL(entropyProgressMaximumChanged(qint32)),this,SIGNAL(progressValueMaximumOpt(qint32)));
}

void EntropyProcess::setData(QIODevice *pDevice, DATA *pData, bool bGraph, bool bRegions)
{
    this->g_pDevice=pDevice;
    this->g_pData=pData;
    this->g_bGraph=bGraph;
    this->g_bRegions=bRegions;
}

EntropyProcess::DATA EntropyProcess::processRegionsDevice(QIODevice *pDevice)
{
    EntropyProcess::DATA result={};

    result.nSize=-1;
    result.fileType=XBinary::getPrefFileType(pDevice);

    EntropyProcess entropyProcess;
    entropyProcess.setData(pDevice,&result,false,true);
    entropyProcess.process();

    return result;
}

EntropyProcess::DATA EntropyProcess::processRegionsFile(QString sFileName)
{
    EntropyProcess::DATA result={};

    QFile file;

    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        result=processRegionsDevice(&file);

        file.close();
    }

    return result;
}

QString EntropyProcess::dataToPlainString(EntropyProcess::DATA *pData)
{
    QString sResult;

    sResult+=QString("Total %1: %2\n").arg(pData->dTotalEntropy).arg(pData->sStatus);

    int nNumberOfRecords=pData->listMemoryRecords.count();

    for(int i=0;i<nNumberOfRecords;i++)
    {
        sResult+=QString("  %1|%2|%3|%4|%5: %6\n")
                .arg(i)
                .arg(pData->listMemoryRecords.at(i).sName)
                .arg(pData->listMemoryRecords.at(i).nOffset)
                .arg(pData->listMemoryRecords.at(i).nSize)
                .arg(pData->listMemoryRecords.at(i).dEntropy)
                .arg(pData->listMemoryRecords.at(i).sStatus);
    }

    return sResult;
}

QString EntropyProcess::dataToJsonString(EntropyProcess::DATA *pData)
{
    QString sResult;

    QJsonObject jsonResult;

    jsonResult.insert("total",pData->dTotalEntropy);
    jsonResult.insert("status",pData->sStatus);

    QJsonArray jsArray;

    int nNumberOfRecords=pData->listMemoryRecords.count();

    for(int i=0; i<nNumberOfRecords; i++)
    {
        QJsonObject jsRecord;

        jsRecord.insert("name",pData->listMemoryRecords.at(i).sName);
        jsRecord.insert("offset",pData->listMemoryRecords.at(i).nOffset);
        jsRecord.insert("size",pData->listMemoryRecords.at(i).nSize);
        jsRecord.insert("entropy",pData->listMemoryRecords.at(i).dEntropy);
        jsRecord.insert("status",pData->listMemoryRecords.at(i).sStatus);

        jsArray.append(jsRecord);
    }

    jsonResult.insert("records",jsArray);

    QJsonDocument saveFormat(jsonResult);

    sResult=saveFormat.toJson(QJsonDocument::Indented).data();

    return sResult;
}

QString EntropyProcess::dataToXmlString(EntropyProcess::DATA *pData)
{
    QString sResult;

    QXmlStreamWriter xml(&sResult);

    xml.setAutoFormatting(true);

    xml.writeStartElement("fileentropy");
    xml.writeAttribute("total",QString::number(pData->dTotalEntropy,'g',16));
    xml.writeAttribute("status",pData->sStatus);

    int nNumberOfRecords=pData->listMemoryRecords.count();

    for(int i=0; i<nNumberOfRecords; i++)
    {
        xml.writeStartElement("record");
        xml.writeAttribute("name",pData->listMemoryRecords.at(i).sName);
        xml.writeAttribute("offset",QString::number(pData->listMemoryRecords.at(i).nOffset));
        xml.writeAttribute("size",QString::number(pData->listMemoryRecords.at(i).nSize));
        xml.writeAttribute("entropy",QString::number(pData->listMemoryRecords.at(i).dEntropy,'g',16));
        xml.writeAttribute("status",pData->listMemoryRecords.at(i).sStatus);
        xml.writeEndElement();
    }

    xml.writeEndElement();

    return sResult;
}

void EntropyProcess::stop()
{
    g_binary.setEntropyProcessEnable(false);
    g_bIsStop=true;
}

void EntropyProcess::process()
{
    QElapsedTimer scanTimer;
    scanTimer.start();

    g_bIsStop=false;

    g_binary.setDevice(this->g_pDevice);

    g_pData->dTotalEntropy=g_binary.getEntropy(g_pData->nOffset,g_pData->nSize);

    if(XBinary::isPacked(g_pData->dTotalEntropy))
    {
        g_pData->sStatus=tr("packed");
    }
    else
    {
        g_pData->sStatus=tr("not packed");
    }

    if(g_bGraph)
    {
        emit progressValueMinimumMain(0);
        emit progressValueMaximumMain(N_MAX_GRAPH);

        g_pData->byteCounts=g_binary.getByteCounts(g_pData->nOffset,g_pData->nSize);
        g_pData->nMaxGraph=N_MAX_GRAPH;

        qint64 nGraph=(g_pData->nSize)/g_pData->nMaxGraph;

        if(nGraph)
        {
    //        for(int i=0;i<=pData->nMaxGraph;i++)
    //        {
    //            pData->dOffset[i]=pData->nOffset+i*nGraph;
    //            pData->dOffsetEntropy[i]=pData->dTotalEntropy;
    //        }
            for(int i=0;(i<g_pData->nMaxGraph)&&(!g_bIsStop);i++)
            {
                g_pData->dOffset[i]=g_pData->nOffset+i*nGraph;

                g_pData->dOffsetEntropy[i]=g_binary.getEntropy(g_pData->nOffset+i*nGraph,qMin(nGraph*(g_pData->nMaxGraph/10),g_pData->nSize-(i*nGraph)));

                emit progressValueChangedMain(i);
            }

            g_pData->dOffset[g_pData->nMaxGraph]=g_pData->nOffset+g_pData->nSize;
            g_pData->dOffsetEntropy[g_pData->nMaxGraph]=g_pData->dOffsetEntropy[g_pData->nMaxGraph-1];
        }
    }

    if(g_bRegions)
    {
        g_pData->listMemoryRecords.clear();

        XBinary::_MEMORY_MAP memoryMap=XFormats::getMemoryMap(g_pData->fileType,this->g_pDevice);

     #ifdef QT_GUI_LIB
        g_pData->mode=XLineEditHEX::MODE_32;

        XBinary::MODE _mode=XBinary::getWidthModeFromMemoryMap(&memoryMap);

        if      (_mode==XBinary::MODE_8)    g_pData->mode=XLineEditHEX::MODE_8;
        else if (_mode==XBinary::MODE_16)   g_pData->mode=XLineEditHEX::MODE_16;
        else if (_mode==XBinary::MODE_32)   g_pData->mode=XLineEditHEX::MODE_32;
        else if (_mode==XBinary::MODE_64)   g_pData->mode=XLineEditHEX::MODE_64;
    #endif

        int nNumberOfRecords=memoryMap.listRecords.count();

        for(int i=0,j=0;i<nNumberOfRecords;i++)
        {
            bool bIsVirtual=memoryMap.listRecords.at(i).bIsVirtual;

            if(!bIsVirtual)
            {
                MEMORY_RECORD memoryRecord={};

                if((memoryMap.listRecords.at(i).nOffset==0)&&(memoryMap.listRecords.at(i).nSize==g_pData->nSize))
                {
                    memoryRecord.dEntropy=g_pData->dTotalEntropy;
                }
                else
                {
                    memoryRecord.dEntropy=g_binary.getEntropy(g_pData->nOffset+memoryMap.listRecords.at(i).nOffset,memoryMap.listRecords.at(i).nSize);
                }

                memoryRecord.sName=memoryMap.listRecords.at(i).sName;
                memoryRecord.nOffset=memoryMap.listRecords.at(i).nOffset;
                memoryRecord.nSize=memoryMap.listRecords.at(i).nSize;
                memoryRecord.sStatus=XBinary::isPacked(memoryRecord.dEntropy)?(tr("packed")):(tr("not packed"));

                g_pData->listMemoryRecords.append(memoryRecord);

                j++;
            }
        }
    }

    g_bIsStop=false;

    emit completed(scanTimer.elapsed());
}
