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
#ifndef XENTROPYWIDGET_H
#define XENTROPYWIDGET_H

#include <QFileDialog>
#include <QImageWriter>
#include <QItemSelection>
#include <QStandardItemModel>

#include "dialogentropyprocess.h"
#include "qwt_legend.h"
#include "qwt_math.h"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_histogram.h"
#include "qwt_plot_magnifier.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_picker.h"
#include "qwt_plot_renderer.h"
#include "qwt_plot_zoneitem.h"
#include "qwt_series_data.h"
#include "qwt_text.h"
#include "xshortcutswidget.h"

namespace Ui {
class XEntropyWidget;
}

class XEntropyWidget : public XShortcutsWidget {
    Q_OBJECT

public:
    explicit XEntropyWidget(QWidget *pParent = nullptr);
    ~XEntropyWidget();

    void setData(QIODevice *pDevice, qint64 nOffset, qint64 nSize, XBinary::FT fileType, bool bAuto);
    void setSaveDirectory(QString sSaveDirectory);
    void reload(bool bGraph, bool bRegions);

private slots:
    void on_pushButtonReload_clicked();
    void on_comboBoxType_currentIndexChanged(int nIndex);
    void on_pushButtonSaveEntropyTable_clicked();
    void on_pushButtonSaveEntropyDiagram_clicked();
    void on_spinBoxCount_valueChanged(int nValue);
    void adjust();
    void on_checkBoxGridRegions_toggled(bool bChecked);
    void on_tableViewSelection(const QItemSelection &isSelected, const QItemSelection &isDeselected);
    void on_tableViewRegions_customContextMenuRequested(const QPoint &pos);
    void on_tableWidgetBytes_customContextMenuRequested(const QPoint &pos);

protected:
    virtual void registerShortcuts(bool bState);

private:
    Ui::XEntropyWidget *ui;
    QIODevice *g_pDevice;
    qint64 g_nOffset;
    qint64 g_nSize;
    EntropyProcess::DATA g_entropyData;
    QwtPlotCurve *g_pCurve;
    QwtPlotHistogram *g_pHistogram;
    QwtPlotGrid *g_pGrid;
    QList<QwtPlotZoneItem *> g_listZones;
    QString g_sSaveDirectory;
    QwtPlotPicker *g_pPicker;
};

#endif  // XENTROPYWIDGET_H
