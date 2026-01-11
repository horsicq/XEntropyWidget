/* Copyright (c) 2020-2026 hors<horsicq@gmail.com>
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

EntropyProcess::EntropyProcess(QObject *pParent) : XThreadObject(pParent)
{
    m_pDevice = nullptr;
    m_pData = nullptr;
    m_bGraph = false;
    m_bRegions = false;
    m_nMax = 0;
    m_pPdStruct = nullptr;
    m_pdStructEmpty = XBinary::createPdStruct();
}

void EntropyProcess::setData(QIODevice *pDevice, DATA *pData, bool bGraph, bool bRegions, qint32 nMax, XBinary::PDSTRUCT *pPdStruct)
{
    this->m_pDevice = pDevice;
    this->m_pData = pData;
    this->m_bGraph = bGraph;
    this->m_bRegions = bRegions;
    this->m_nMax = nMax;
    this->m_pPdStruct = pPdStruct;

    if (!(this->m_pPdStruct)) {
        this->m_pPdStruct = &m_pdStructEmpty;
    }
}

EntropyProcess::DATA EntropyProcess::processRegionsDevice(QIODevice *pDevice)
{
    EntropyProcess::DATA result = {};

    result.nSize = -1;
    result.fileType = XBinary::getPrefFileType(pDevice);
    result.mapMode = XBinary::MAPMODE_UNKNOWN;

    EntropyProcess entropyProcess;
    entropyProcess.setData(pDevice, &result, false, true, 100, nullptr);  // TODO Consts
    entropyProcess.process();

    return result;
}

EntropyProcess::DATA EntropyProcess::processRegionsFile(const QString &sFileName)
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

    sResult += QString("Total %1: %2\n").arg(QString::number(pData->dTotalEntropy), pData->sStatus);  // mb Translate

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
    qint32 _nFreeIndex = XBinary::getFreeIndex(m_pPdStruct);

    XBinary binary(this->m_pDevice);

    connect(&binary, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)));

    m_pData->dTotalEntropy = binary.getBinaryStatus(XBinary::BSTATUS_ENTROPY, m_pData->nOffset, m_pData->nSize, m_pPdStruct);

    if (binary.isPacked(m_pData->dTotalEntropy)) {
        m_pData->sStatus = tr("packed");
    } else {
        m_pData->sStatus = tr("not packed");
    }

    if (m_bGraph) {
        XBinary::setPdStructInit(m_pPdStruct, _nFreeIndex, m_nMax);

        m_pData->byteCounts = binary.getByteCounts(m_pData->nOffset, m_pData->nSize, m_pPdStruct);

        qint64 nGraph = (m_pData->nSize) / m_nMax;

        if (nGraph) {
            m_pData->listEntropies.clear();
            RECORD record = {};

            for (qint32 i = 0; (i < m_nMax) && XBinary::isPdStructNotCanceled(m_pPdStruct); i++) {
                record.dOffset = m_pData->nOffset + i * nGraph;
                record.dEntropy = binary.getBinaryStatus(XBinary::BSTATUS_ENTROPY, m_pData->nOffset + i * nGraph, nGraph, m_pPdStruct);

                m_pData->listEntropies.append(record);

                XBinary::setPdStructCurrent(m_pPdStruct, _nFreeIndex, i);
            }

            record.dOffset = m_pData->nOffset + m_pData->nSize;

            m_pData->listEntropies.append(record);
        }
    }

    if (m_bRegions) {
        m_pData->listMemoryRecords.clear();

        XBinary::_MEMORY_MAP memoryMap = XFormats::getMemoryMap(m_pData->fileType, m_pData->mapMode, this->m_pDevice);

#ifdef QT_GUI_LIB
        m_pData->mode = XLineEditValidator::MODE_HEX_32;

        XBinary::MODE _mode = XBinary::getWidthModeFromMemoryMap(&memoryMap);

        // TODO move to Widget
        if (_mode == XBinary::MODE_8) m_pData->mode = XLineEditValidator::MODE_HEX_8;
        else if (_mode == XBinary::MODE_16) m_pData->mode = XLineEditValidator::MODE_HEX_16;
        else if (_mode == XBinary::MODE_32) m_pData->mode = XLineEditValidator::MODE_HEX_32;
        else if (_mode == XBinary::MODE_64) m_pData->mode = XLineEditValidator::MODE_HEX_64;
#endif

        qint32 nNumberOfRecords = memoryMap.listRecords.count();

        for (qint32 i = 0, j = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(m_pPdStruct); i++) {
            bool bIsVirtual = memoryMap.listRecords.at(i).bIsVirtual;

            if (!bIsVirtual) {
                MEMORY_RECORD memoryRecord = {};

                if ((memoryMap.listRecords.at(i).nOffset == 0) && (memoryMap.listRecords.at(i).nSize == m_pData->nSize)) {
                    memoryRecord.dEntropy = m_pData->dTotalEntropy;
                } else {
                    memoryRecord.dEntropy = binary.getBinaryStatus(XBinary::BSTATUS_ENTROPY, m_pData->nOffset + memoryMap.listRecords.at(i).nOffset,
                                                                   memoryMap.listRecords.at(i).nSize, m_pPdStruct);
                }

                memoryRecord.sName = memoryMap.listRecords.at(i).sName;
                memoryRecord.nOffset = memoryMap.listRecords.at(i).nOffset;
                memoryRecord.nSize = memoryMap.listRecords.at(i).nSize;
                memoryRecord.sStatus = binary.isPacked(memoryRecord.dEntropy) ? (tr("packed")) : (tr("not packed"));

                m_pData->listMemoryRecords.append(memoryRecord);

                j++;
            }
        }
    }

    XBinary::setPdStructFinished(m_pPdStruct, _nFreeIndex);
}
