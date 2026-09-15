#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QMap>
#include "colormath.h"

struct CompUI {
    QSlider* slider;
    QDoubleSpinBox* spin;
    double minV, maxV;
};

struct ModelUI {
    QGroupBox* box;
    QVector<CompUI> comps;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onVariantChanged(int idx);
    void onValueChanged(const QString& model);
    void chooseColor();

private:
    void setupUI();
    void updateModels(RGBColor rgb, bool clipped, QString src = "");

    QComboBox* varCombo;
    QLabel* warnLabel;
    QWidget* colorPreview;
    QMap<QString, ModelUI> modelsUI;
    RGBColor currentRGB{128, 128, 255};
    bool updating{false};

    QVector<QVector<QString>> varList = {
        {"RGB", "LAB", "CMYK"}, {"RGB", "CMYK", "HLS"}, {"RGB", "XYZ", "LAB"},
        {"RGB", "HSV", "LAB"},  {"CMYK", "LAB", "HSV"}, {"CMYK", "RGB", "HLS"},
        {"CMYK", "RGB", "HSV"}, {"RGB", "XYZ", "HSV"},  {"HSV", "XYZ", "LAB"},
        {"CMYK", "LAB", "RGB"}, {"XYZ", "LAB", "HLS"},  {"RGB", "XYZ", "HLS"},
        {"RGB", "XYZ", "CMYK"}, {"CMYK", "LAB", "XYZ"}, {"RGB", "CMYK", "HSV"},
        {"CMYK", "HLS", "XYZ"}, {"RGB", "HLS", "LAB"},  {"CMYK", "XYZ", "RGB"}
    };
};

#endif