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
#include "xformats.h"

class EntropyProcess : public QObject
{
    Q_OBJECT
public:
    static const int N_MAX_GRAPH=100;

    struct DATA
    {
        qint64 nOffset;
        qint64 nSize;
        double dTotalEntropy;
        double dGraph[N_MAX_GRAPH];
    };

    explicit EntropyProcess(QObject *parent=nullptr);
    void setData(QIODevice *pDevice,DATA *pData);

signals:
    void errorMessage(QString sText);
    void completed(qint64 nElapsed);
    void progressValueChanged(qint32 nValue);
    void progressValueMinimum(qint32 nValue);
    void progressValueMaximum(qint32 nValue);

public slots:
    void stop();
    void process();

private:
    QIODevice *pDevice;
    DATA *pData;
    bool bIsStop;
};

#endif // ENTROPYPROCESS_H
