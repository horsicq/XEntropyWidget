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
#ifndef XENTROPYWIDGET_H
#define XENTROPYWIDGET_H

#include <QStandardItemModel>
#include <QItemSelection>
#include <QImageWriter>
#include <QFileDialog>
#include "xshortcutswidget.h"
#include "dialogentropyprocess.h"
#include "qwt_plot.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_curve.h"
#include "qwt_legend.h"
#include "qwt_series_data.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_magnifier.h"
#include "qwt_text.h"
#include "qwt_math.h"
#include "qwt_plot_renderer.h"
#include "qwt_plot_histogram.h"
#include "qwt_plot_zoneitem.h"
#include "qwt_plot_renderer.h"

namespace Ui {
class XEntropyWidget;
}

class XEntropyWidget : public XShortcutsWidget
{
    Q_OBJECT

public:
    explicit XEntropyWidget(QWidget *pParent=nullptr);
    ~XEntropyWidget();
    void setData(QIODevice *pDevice,qint64 nOffset,qint64 nSize,XBinary::FT fileType,bool bAuto,QWidget *pParent);
    void setSaveDirectory(QString sSaveDirectory);
    void reload(bool bGraph,bool bRegions);

private slots:
    void on_pushButtonReload_clicked();
    void on_comboBoxType_currentIndexChanged(int nIndex);
    void on_tableWidgetRegions_itemSelectionChanged();
    void on_pushButtonSaveEntropy_clicked();
    QString getResultName();

protected:
    virtual void registerShortcuts(bool bState);

private:
    Ui::XEntropyWidget *ui;
    QIODevice *g_pDevice;
    QWidget *g_pParent;
    qint64 g_nOffset;
    qint64 g_nSize;
    EntropyProcess::DATA g_entropyData;
    QwtPlotCurve *g_pCurve;
    QwtPlotHistogram *g_pHistogram;
    QList<QwtPlotZoneItem *> g_listZones;
    QString g_sSaveDirectory;
};

#endif // XENTROPYWIDGET_H
