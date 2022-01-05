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
#ifndef DIALOGENTROPYPROCESS_H
#define DIALOGENTROPYPROCESS_H

#include <QDialog>
#include <QMessageBox>
#include <QThread>
#include "entropyprocess.h"
#include "xoptions.h"

namespace Ui {
class DialogEntropyProcess;
}

class DialogEntropyProcess : public QDialog
{
    Q_OBJECT

public:
    // TODO setData
    explicit DialogEntropyProcess(QWidget *pParent,QIODevice *pDevice,EntropyProcess::DATA *pData,bool bGraph,bool bRegions,qint32 nMax);
    ~DialogEntropyProcess();

private slots:
    void on_pushButtonCancel_clicked();
    void errorMessage(QString sText);
    void onCompleted(qint64 nElapsed);
    void progressValueChangedMain(qint32 nValue);
    void progressValueMaximumMain(qint32 nValue);
    void progressValueMinimumMain(qint32 nValue);
    void progressValueChangedOpt(qint32 nValue);
    void progressValueMaximumOpt(qint32 nValue);
    void progressValueMinimumOpt(qint32 nValue);

private:
    Ui::DialogEntropyProcess *ui;
    EntropyProcess *g_pEntropyProcess;
    QThread *g_pThread;
    bool g_bIsStop;
};

#endif // DIALOGENTROPYPROCESS_H
