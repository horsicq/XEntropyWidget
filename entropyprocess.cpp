/* Copyright (c) 2020-2023 hors<horsicq@gmail.com>
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
#include "entropyprocess.h"

EntropyProcess::EntropyProcess(QObject *pParent) : QObject(pParent)
{
    g_pDevice = nullptr;
    g_pData = nullptr;
    g_bGraph = false;
    g_bRegions = false;
    g_nMax = 0;
    g_pPdStruct = nullptr;
    g_pdStructEmpty = XBinary::createPdStruct();
}

void EntropyProcess::setData(QIODevice *pDevice, DATA *pData, bool bGraph, bool bRegions, qint32 nMax, XBinary::PDSTRUCT *pPdStruct)
{
    this->g_pDevice = pDevice;
    this->g_pData = pData;
    this->g_bGraph = bGraph;
    this->g_bRegions = bRegions;
    this->g_nMax = nMax;
    this->g_pPdStruct = pPdStruct;

    if (!(this->g_pPdStruct)) {
        this->g_pPdStruct = &g_pdStructEmpty;
    }
}

EntropyProcess::DATA EntropyProcess::processRegionsDevice(QIODevice *pDevice)
{
    EntropyProcess::DATA result = {};

    result.nSize = -1;
    result.fileType = XBinary::getPrefFileType(pDevice);

    EntropyProcess entropyProcess;
    entropyProcess.setData(pDevice, &result, false, true, 100, nullptr);  // TODO Consts
    entropyProcess.process();

    return result;
}

EntropyProcess::DATA EntropyProcess::processRegionsFile(QString sFileName)
{
    EntropyProcess::DATA result = {};

    QFile file;

    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        result = processRegionsDevice(&file);

        file.close();
    }

    return result;
}

QString EntropyProcess::dataToPlainString(EntropyProcess::DATA *pData)
{
    QString sResult;

    sResult += QString("Total %1: %2\n").arg(QString::number(pData->dTotalEntropy), pData->sStatus);

    qint32 nNumberOfRecords = pData->listMemoryRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        sResult += QString("  %1|%2|%3|%4|%5: %6\n")
                       .arg(QString::number(i), pData->listMemoryRecords.at(i).sName, QString::number(pData->listMemoryRecords.at(i).nOffset),
                            QString::number(pData->listMemoryRecords.at(i).nSize), QString::number(pData->listMemoryRecords.at(i).dEntropy),
                            pData->listMemoryRecords.at(i).sStatus);
    }

    return sResult;
}

QString EntropyProcess::dataToJsonString(EntropyProcess::DATA *pData)
{
    QString sResult;

    QJsonObject jsonResult;

    jsonResult.insert("total", pData->dTotalEntropy);
    jsonResult.insert("status", pData->sStatus);

    QJsonArray jsArray;

    qint32 nNumberOfRecords = pData->listMemoryRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        QJsonObject jsRecord;

        jsRecord.insert("name", pData->listMemoryRecords.at(i).sName);
        jsRecord.insert("offset", pData->listMemoryRecords.at(i).nOffset);
        jsRecord.insert("size", pData->listMemoryRecords.at(i).nSize);
        jsRecord.insert("entropy", pData->listMemoryRecords.at(i).dEntropy);
        jsRecord.insert("status", pData->listMemoryRecords.at(i).sStatus);

        jsArray.append(jsRecord);
    }

    jsonResult.insert("records", jsArray);

    QJsonDocument saveFormat(jsonResult);

    sResult = saveFormat.toJson(QJsonDocument::Indented).constData();

    return sResult;
}

QString EntropyProcess::dataToXmlString(EntropyProcess::DATA *pData)
{
    QString sResult;

    QXmlStreamWriter xml(&sResult);

    xml.setAutoFormatting(true);

    xml.writeStartElement("fileentropy");
    xml.writeAttribute("total", QString::number(pData->dTotalEntropy, 'g', 16));
    xml.writeAttribute("status", pData->sStatus);

    qint32 nNumberOfRecords = pData->listMemoryRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        xml.writeStartElement("record");
        xml.writeAttribute("name", pData->listMemoryRecords.at(i).sName);
        xml.writeAttribute("offset", QString::number(pData->listMemoryRecords.at(i).nOffset));
        xml.writeAttribute("size", QString::number(pData->listMemoryRecords.at(i).nSize));
        xml.writeAttribute("entropy", QString::number(pData->listMemoryRecords.at(i).dEntropy, 'g', 16));
        xml.writeAttribute("status", pData->listMemoryRecords.at(i).sStatus);
        xml.writeEndElement();
    }

    xml.writeEndElement();

    return sResult;
}

QString EntropyProcess::dataToCsvString(DATA *pData)
{
    QString sResult;

    qint32 nNumberOfRecords = pData->listMemoryRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        sResult += QString("%1;%2;%3;%4;%5;%6\n")
                       .arg(QString::number(i), pData->listMemoryRecords.at(i).sName, QString::number(pData->listMemoryRecords.at(i).nOffset),
                            QString::number(pData->listMemoryRecords.at(i).nSize), QString::number(pData->listMemoryRecords.at(i).dEntropy),
                            pData->listMemoryRecords.at(i).sStatus);
    }

    return sResult;
}

QString EntropyProcess::dataToTsvString(DATA *pData)
{
    QString sResult;

    qint32 nNumberOfRecords = pData->listMemoryRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        sResult += QString("%1\t%2\t%3\t%4\t%5\t%6\n")
                       .arg(QString::number(i), pData->listMemoryRecords.at(i).sName, QString::number(pData->listMemoryRecords.at(i).nOffset),
                            QString::number(pData->listMemoryRecords.at(i).nSize), QString::number(pData->listMemoryRecords.at(i).dEntropy),
                            pData->listMemoryRecords.at(i).sStatus);
    }

    return sResult;
}

void EntropyProcess::process()
{
    QElapsedTimer scanTimer;
    scanTimer.start();

    qint32 _nFreeIndex = XBinary::getFreeIndex(g_pPdStruct);

    XBinary binary(this->g_pDevice);

    connect(&binary, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)));

    g_pData->dTotalEntropy = binary.getEntropy(g_pData->nOffset, g_pData->nSize, g_pPdStruct);

    if (XBinary::isPacked(g_pData->dTotalEntropy)) {
        g_pData->sStatus = tr("packed");
    } else {
        g_pData->sStatus = tr("not packed");
    }

    if (g_bGraph) {
        XBinary::setPdStructInit(g_pPdStruct, _nFreeIndex, g_nMax);

        g_pData->byteCounts = binary.getByteCounts(g_pData->nOffset, g_pData->nSize, g_pPdStruct);

        qint64 nGraph = (g_pData->nSize) / g_nMax;

        if (nGraph) {
            g_pData->listEntropies.clear();
            //        for(qint32 i=0;i<=pData->nMaxGraph;i++)
            //        {
            //            pData->dOffset[i]=pData->nOffset+i*nGraph;
            //            pData->dOffsetEntropy[i]=pData->dTotalEntropy;
            //        }
            RECORD record = {};

            for (qint32 i = 0; (i < g_nMax) && (!(g_pPdStruct->bIsStop)); i++) {
                //                g_pData->dOffset[i]=g_pData->nOffset+i*nGraph;
                //                g_pData->dOffsetEntropy[i]=g_binary.getEntropy(g_pData->nOffset+i*nGraph,qMin(nGraph*(g_nMax/10),g_pData->nSize-(i*nGraph)));
                //                g_pData->listOffsetEntropy.append(g_binary.getEntropy(g_pData->nOffset+i*nGraph,qMin(nGraph*(g_nMax/10),g_pData->nSize-(i*nGraph))));

                record.dOffset = g_pData->nOffset + i * nGraph;
                record.dEntropy = binary.getEntropy(g_pData->nOffset + i * nGraph, nGraph, g_pPdStruct);

                g_pData->listEntropies.append(record);

                XBinary::setPdStructCurrent(g_pPdStruct, _nFreeIndex, i);
            }

            record.dOffset = g_pData->nOffset + g_pData->nSize;

            g_pData->listEntropies.append(record);

            //            g_pData->listOffsets.append(g_pData->nOffset+g_pData->nSize);

            //            g_pData->listOffsetEntropy.append(g_pData->listOffsetEntropy.last());

            //            g_pData->dOffset[g_pData->nMaxGraph]=;
            //            g_pData->dOffsetEntropy[g_pData->nMaxGraph]=g_pData->dOffsetEntropy[g_pData->nMaxGraph-1];
        }
    }

    if (g_bRegions) {
        g_pData->listMemoryRecords.clear();

        XBinary::_MEMORY_MAP memoryMap = XFormats::getMemoryMap(g_pData->fileType, this->g_pDevice);

#ifdef QT_GUI_LIB
        g_pData->mode = XLineEditHEX::MODE_32;

        XBinary::MODE _mode = XBinary::getWidthModeFromMemoryMap(&memoryMap);

        // TODO move to Widget
        if (_mode == XBinary::MODE_8)
            g_pData->mode = XLineEditHEX::MODE_8;
        else if (_mode == XBinary::MODE_16)
            g_pData->mode = XLineEditHEX::MODE_16;
        else if (_mode == XBinary::MODE_32)
            g_pData->mode = XLineEditHEX::MODE_32;
        else if (_mode == XBinary::MODE_64)
            g_pData->mode = XLineEditHEX::MODE_64;
#endif

        qint32 nNumberOfRecords = memoryMap.listRecords.count();

        for (qint32 i = 0, j = 0; (i < nNumberOfRecords) && (!(g_pPdStruct->bIsStop)); i++) {
            bool bIsVirtual = memoryMap.listRecords.at(i).bIsVirtual;

            if (!bIsVirtual) {
                MEMORY_RECORD memoryRecord = {};

                if ((memoryMap.listRecords.at(i).nOffset == 0) && (memoryMap.listRecords.at(i).nSize == g_pData->nSize)) {
                    memoryRecord.dEntropy = g_pData->dTotalEntropy;
                } else {
                    memoryRecord.dEntropy = binary.getEntropy(g_pData->nOffset + memoryMap.listRecords.at(i).nOffset, memoryMap.listRecords.at(i).nSize, g_pPdStruct);
                }

                memoryRecord.sName = memoryMap.listRecords.at(i).sName;
                memoryRecord.nOffset = memoryMap.listRecords.at(i).nOffset;
                memoryRecord.nSize = memoryMap.listRecords.at(i).nSize;
                memoryRecord.sStatus = XBinary::isPacked(memoryRecord.dEntropy) ? (tr("packed")) : (tr("not packed"));

                g_pData->listMemoryRecords.append(memoryRecord);

                j++;
            }
        }
    }

    XBinary::setPdStructFinished(g_pPdStruct, _nFreeIndex);

    emit completed(scanTimer.elapsed());
}
