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
#ifndef ENTROPYPROCESS_H
#define ENTROPYPROCESS_H

#include <QObject>
#include "xlineedithex.h"
#include "xformats.h"

class EntropyProcess : public QObject
{
    Q_OBJECT

public:
    static const int N_MAX_GRAPH=100;

    struct MEMORY_RECORD
    {
        QString sName;
        qint64 nOffset;
        qint64 nSize;
        double dEntropy;
        QString sStatus;
    };

    struct DATA
    {
        qint64 nOffset;
        qint64 nSize;
        double dTotalEntropy;
        QString sStatus;
        double dOffset[N_MAX_GRAPH+1];
        double dOffsetEntropy[N_MAX_GRAPH+1];
        XBinary::BYTE_COUNTS byteCounts;
        qint32 nMaxGraph;
        XBinary::FT fileType;
        XLineEditHEX::MODE mode;
        QList<MEMORY_RECORD> listMemoryRecords;
    };

    explicit EntropyProcess(QObject *pParent=nullptr);
    void setData(QIODevice *pDevice,DATA *pData,bool bGraph,bool bRegions);

signals:
    void errorMessage(QString sText);
    void completed(qint64 nElapsed);
    void progressValueChangedMain(qint32 nValue);
    void progressValueMinimumMain(qint32 nValue);
    void progressValueMaximumMain(qint32 nValue);
    void progressValueChangedOpt(qint32 nValue);
    void progressValueMinimumOpt(qint32 nValue);
    void progressValueMaximumOpt(qint32 nValue);

public slots:
    void stop();
    void process();

private:
    QIODevice *pDevice;
    DATA *pData;
    bool bIsStop;
    XBinary binary;
    bool bGraph;
    bool bRegions;
};

#endif // ENTROPYPROCESS_H
