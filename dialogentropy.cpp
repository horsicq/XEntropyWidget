// copyright (c) 2017-2021 hors<horsicq@gmail.com>
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
#include "dialogentropy.h"
#include "ui_dialogentropy.h"

DialogEntropy::DialogEntropy(QWidget *pParent) :
    QDialog(pParent),
    ui(new Ui::DialogEntropy)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);
}

DialogEntropy::~DialogEntropy()
{
    delete ui;
}

void DialogEntropy::setData(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    XBinary::FT ft=XBinary::FT_UNKNOWN;

    XBinary::OFFSETSIZE os=XBinary::convertOffsetAndSize(pDevice,nOffset,nSize);

    if(os.nSize!=pDevice->size())
    {
        ft=XBinary::FT_BINARY;
    }

    ui->widgetEntropy->setData(pDevice,nOffset,nSize,ft,true);
}

void DialogEntropy::setShortcuts(XShortcuts *pShortcuts)
{
    Q_UNUSED(pShortcuts)
    // TODO
}

void DialogEntropy::on_pushButtonClose_clicked()
{
    this->close();
}
