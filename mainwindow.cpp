#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColorDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    onVariantChanged(0);
}

void MainWindow::setupUI() {
    setWindowTitle("Лабораторная работа №1 (Qt Widgets)");
    resize(900, 500);

    QWidget* w = new QWidget(this);
    setCentralWidget(w);
    QVBoxLayout* mainL = new QVBoxLayout(w);

    // Variant
    QHBoxLayout* topL = new QHBoxLayout();
    topL->addWidget(new QLabel("<b>Выберите вариант:</b>"));
    varCombo = new QComboBox();
    for (int i = 0; i < varList.size(); ++i) {
        varCombo->addItem(QString("Вариант %1: %2").arg(i + 1).arg(varList[i].join(" <-> ")));
    }
    topL->addWidget(varCombo, 1);
    mainL->addLayout(topL);

    // Warning
    warnLabel = new QLabel("⚠️ [ПРЕДУПРЕЖДЕНИЕ]: Выход за границы гаммы sRGB!");
    warnLabel->setStyleSheet("color: red; font-weight: bold; padding: 5px;");
    warnLabel->setVisible(false);
    mainL->addWidget(warnLabel);

    // Color Box
    colorPreview = new QWidget();
    colorPreview->setFixedHeight(50);
    mainL->addWidget(colorPreview);

    // Dynamic Models Layout
    QHBoxLayout* modelsL = new QHBoxLayout();

    auto addModel = [&](QString name, QVector<std::tuple<QString, double, double>> specs) {
        ModelUI m;
        m.box = new QGroupBox(name);
        QVBoxLayout* bL = new QVBoxLayout(m.box);

        for (auto& s : specs) {
            QHBoxLayout* r = new QHBoxLayout();
            r->addWidget(new QLabel(std::get<0>(s)));

            QSlider* sl = new QSlider(Qt::Horizontal);
            sl->setRange(0, 1000);
            QDoubleSpinBox* sp = new QDoubleSpinBox();
            sp->setRange(std::get<1>(s), std::get<2>(s));
            sp->setDecimals(1);

            r->addWidget(sl);
            r->addWidget(sp);
            bL->addLayout(r);

            m.comps.append({sl, sp, std::get<1>(s), std::get<2>(s)});

            connect(sl, &QSlider::valueChanged, this, [=](int v) {
                if (updating) return;
                sp->setValue(std::get<1>(s) + (v / 1000.0) * (std::get<2>(s) - std::get<1>(s)));
                onValueChanged(name);
            });
            connect(sp, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double v) {
                if (updating) return;
                sl->setValue(((v - std::get<1>(s)) / (std::get<2>(s) - std::get<1>(s))) * 1000.0);
                onValueChanged(name);
            });
        }

        QPushButton* btn = new QPushButton("Палитра (Color Picker)");
        bL->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, &MainWindow::chooseColor);

        modelsL->addWidget(m.box);
        modelsUI[name] = m;
    };

    addModel("RGB", {{"R",0,255}, {"G",0,255}, {"B",0,255}});
    addModel("CMYK", {{"C",0,100}, {"M",0,100}, {"Y",0,100}, {"K",0,100}});
    addModel("HSV", {{"H",0,360}, {"S",0,100}, {"V",0,100}});
    addModel("HLS", {{"H",0,360}, {"L",0,100}, {"S",0,100}});
    addModel("XYZ", {{"X",0,95.05}, {"Y",0,100}, {"Z",0,108.89}});
    addModel("LAB", {{"L*",0,100}, {"a*",-128,127}, {"b*",-128,127}});

    mainL->addLayout(modelsL);

    connect(varCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onVariantChanged);
}

void MainWindow::onVariantChanged(int idx) {
    QVector<QString> active = varList[idx];
    for (auto k : modelsUI.keys()) {
        modelsUI[k].box->setVisible(active.contains(k));
    }
    updateModels(currentRGB, false);
}

void MainWindow::onValueChanged(const QString& model) {
    if (updating) return;
    auto& c = modelsUI[model].comps;
    ConversionResult res;

    if (model == "RGB") res = clampRGB(c[0].spin->value(), c[1].spin->value(), c[2].spin->value());
    else if (model == "CMYK") res = cmykToRgb({c[0].spin->value(), c[1].spin->value(), c[2].spin->value(), c[3].spin->value()});
    else if (model == "HSV") res = hsvToRgb({c[0].spin->value(), c[1].spin->value(), c[2].spin->value()});
    else if (model == "HLS") res = hlsToRgb({c[0].spin->value(), c[1].spin->value(), c[2].spin->value()});
    else if (model == "XYZ") res = xyzToRgb({c[0].spin->value(), c[1].spin->value(), c[2].spin->value()});
    else if (model == "LAB") res = xyzToRgb(labToXyz({c[0].spin->value(), c[1].spin->value(), c[2].spin->value()}));

    currentRGB = res.rgb;
    updateModels(currentRGB, res.clipped, model);
}

void MainWindow::chooseColor() {
    QColor c = QColorDialog::getColor(QColor(currentRGB.r, currentRGB.g, currentRGB.b), this);
    if (c.isValid()) {
        currentRGB = { (double)c.red(), (double)c.green(), (double)c.blue() };
        updateModels(currentRGB, false);
    }
}

void MainWindow::updateModels(RGBColor rgb, bool clipped, QString src) {
    updating = true;
    colorPreview->setStyleSheet(QString("background-color: rgb(%1,%2,%3); border:1px solid black;")
                                    .arg((int)rgb.r).arg((int)rgb.g).arg((int)rgb.b));
    warnLabel->setVisible(clipped);

    CMYKColor cmyk = rgbToCmyk(rgb);
    HSVColor hsv = rgbToHsv(rgb);
    HLSColor hls = rgbToHls(rgb);
    XYZColor xyz = rgbToXyz(rgb);
    LABColor lab = xyzToLab(xyz);

    auto fill = [&](QString name, QVector<double> vals) {
        if (name == src) return;
        auto& m = modelsUI[name];
        for (int i = 0; i < m.comps.size(); ++i) {
            m.comps[i].spin->setValue(vals[i]);
            m.comps[i].slider->setValue(((vals[i] - m.comps[i].minV) / (m.comps[i].maxV - m.comps[i].minV)) * 1000.0);
        }
    };

    fill("RGB", {rgb.r, rgb.g, rgb.b});
    fill("CMYK", {cmyk.c, cmyk.m, cmyk.y, cmyk.k});
    fill("HSV", {hsv.h, hsv.s, hsv.v});
    fill("HLS", {hls.h, hls.l, hls.s});
    fill("XYZ", {xyz.x, xyz.y, xyz.z});
    fill("LAB", {lab.l, lab.a, lab.b});

    updating = false;
}