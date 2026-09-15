#ifndef COLORMATH_H
#define COLORMATH_H

#include <algorithm>
#include <cmath>

struct RGBColor { double r, g, b; };
struct CMYKColor { double c, m, y, k; };
struct HSVColor { double h, s, v; };
struct HLSColor { double h, l, s; };
struct XYZColor { double x, y, z; };
struct LABColor { double l, a, b; };

struct ConversionResult {
    RGBColor rgb;
    bool clipped;
};

inline ConversionResult clampRGB(double r, double g, double b) {
    bool clipped = (r < 0.0 || r > 255.0 || g < 0.0 || g > 255.0 || b < 0.0 || b > 255.0);
    return {{std::max(0.0, std::min(255.0, r)),
             std::max(0.0, std::min(255.0, g)),
             std::max(0.0, std::min(255.0, b))}, clipped};
}

// CMYK
inline CMYKColor rgbToCmyk(RGBColor rgb) {
    double rN = rgb.r / 255.0, gN = rgb.g / 255.0, bN = rgb.b / 255.0;
    double k = 1.0 - std::max({rN, gN, bN});
    if (k >= 1.0) return {0, 0, 0, 100};
    return {(1.0 - rN - k) / (1.0 - k) * 100.0,
            (1.0 - gN - k) / (1.0 - k) * 100.0,
            (1.0 - bN - k) / (1.0 - k) * 100.0, k * 100.0};
}
inline ConversionResult cmykToRgb(CMYKColor c) {
    double cN = c.c / 100.0, mN = c.m / 100.0, yN = c.y / 100.0, kN = c.k / 100.0;
    return clampRGB(255.0 * (1.0 - cN) * (1.0 - kN),
                    255.0 * (1.0 - mN) * (1.0 - kN),
                    255.0 * (1.0 - yN) * (1.0 - kN));
}

// HSV
inline HSVColor rgbToHsv(RGBColor rgb) {
    double rN = rgb.r / 255.0, gN = rgb.g / 255.0, bN = rgb.b / 255.0;
    double maxV = std::max({rN, gN, bN}), minV = std::min({rN, gN, bN}), d = maxV - minV;
    double h = 0, s = (maxV == 0) ? 0 : d / maxV;
    if (maxV != minV) {
        if (maxV == rN) h = (gN - bN) / d + (gN < bN ? 6 : 0);
        else if (maxV == gN) h = (bN - rN) / d + 2;
        else if (maxV == bN) h = (rN - gN) / d + 4;
        h /= 6.0;
    }
    return {h * 360.0, s * 100.0, maxV * 100.0};
}
inline ConversionResult hsvToRgb(HSVColor hsv) {
    double hN = hsv.h / 360.0, sN = hsv.s / 100.0, vN = hsv.v / 100.0;
    int i = std::floor(hN * 6.0);
    double f = hN * 6.0 - i, p = vN * (1.0 - sN), q = vN * (1.0 - f * sN), t = vN * (1.0 - (1.0 - f) * sN);
    double r = 0, g = 0, b = 0;
    switch (i % 6) {
    case 0: r = vN; g = t; b = p; break;
    case 1: r = q; g = vN; b = p; break;
    case 2: r = p; g = vN; b = t; break;
    case 3: r = p; g = q; b = vN; break;
    case 4: r = t; g = p; b = vN; break;
    case 5: r = vN; g = p; b = q; break;
    }
    return clampRGB(r * 255.0, g * 255.0, b * 255.0);
}

// XYZ & LAB
inline XYZColor rgbToXyz(RGBColor rgb) {
    double r = (rgb.r / 255.0 > 0.04045) ? std::pow((rgb.r / 255.0 + 0.055) / 1.055, 2.4) : rgb.r / 255.0 / 12.92;
    double g = (rgb.g / 255.0 > 0.04045) ? std::pow((rgb.g / 255.0 + 0.055) / 1.055, 2.4) : rgb.g / 255.0 / 12.92;
    double b = (rgb.b / 255.0 > 0.04045) ? std::pow((rgb.b / 255.0 + 0.055) / 1.055, 2.4) : rgb.b / 255.0 / 12.92;
    return {(r * 0.4124564 + g * 0.3575761 + b * 0.1804375) * 100.0,
            (r * 0.2126729 + g * 0.7151522 + b * 0.0721750) * 100.0,
            (r * 0.0193339 + g * 0.1191920 + b * 0.9503041) * 100.0};
}
inline ConversionResult xyzToRgb(XYZColor xyz) {
    double x = xyz.x / 100.0, y = xyz.y / 100.0, z = xyz.z / 100.0;
    double rL = x * 3.2404542 + y * -1.5371385 + z * -0.4985314;
    double gL = x * -0.9692660 + y * 1.8760108 + z * 0.0415560;
    double bL = x * 0.0556434 + y * -0.2040259 + z * 1.0572252;
    double rN = (rL > 0.0031308) ? 1.055 * std::pow(rL, 1.0 / 2.4) - 0.055 : 12.92 * rL;
    double gN = (gL > 0.0031308) ? 1.055 * std::pow(gL, 1.0 / 2.4) - 0.055 : 12.92 * gL;
    double bN = (bL > 0.0031308) ? 1.055 * std::pow(bL, 1.0 / 2.4) - 0.055 : 12.92 * bL;
    return clampRGB(rN * 255.0, gN * 255.0, bN * 255.0);
}
inline LABColor xyzToLab(XYZColor xyz) {
    auto f = [](double t) { return (t > 0.008856) ? std::cbrt(t) : (7.787 * t) + (16.0 / 116.0); };
    double fx = f(xyz.x / 95.047), fy = f(xyz.y / 100.000), fz = f(xyz.z / 108.883);
    return {(116.0 * fy) - 16.0, 500.0 * (fx - fy), 200.0 * (fy - fz)};
}
inline XYZColor labToXyz(LABColor lab) {
    double fy = (lab.l + 16.0) / 116.0, fx = lab.a / 500.0 + fy, fz = fy - lab.b / 200.0;
    auto fInv = [](double t) { return (std::pow(t, 3) > 0.008856) ? std::pow(t, 3) : (t - 16.0 / 116.0) / 7.787; };
    return {95.047 * fInv(fx), 100.000 * fInv(fy), 108.883 * fInv(fz)};
}
inline HLSColor rgbToHls(RGBColor rgb) {
    double rN = rgb.r / 255.0, gN = rgb.g / 255.0, bN = rgb.b / 255.0;
    double maxV = std::max({rN, gN, bN}), minV = std::min({rN, gN, bN});
    double h = 0, l = (maxV + minV) / 2.0, s = 0;
    if (maxV != minV) {
        double d = maxV - minV;
        s = l > 0.5 ? d / (2.0 - maxV - minV) : d / (maxV + minV);
        if (maxV == rN) h = (gN - bN) / d + (gN < bN ? 6 : 0);
        else if (maxV == gN) h = (bN - rN) / d + 2;
        else if (maxV == bN) h = (rN - gN) / d + 4;
        h /= 6.0;
    }
    return {h * 360.0, l * 100.0, s * 100.0};
}
inline ConversionResult hlsToRgb(HLSColor hls) {
    double hN = hls.h / 360.0, lN = hls.l / 100.0, sN = hls.s / 100.0;
    if (sN == 0) return clampRGB(lN * 255.0, lN * 255.0, lN * 255.0);
    auto hue2rgb = [](double p, double q, double t) {
        if (t < 0) t += 1.0; if (t > 1) t -= 1.0;
        if (t < 1.0/6.0) return p + (q - p) * 6.0 * t;
        if (t < 1.0/2.0) return q;
        if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
        return p;
    };
    double q = lN < 0.5 ? lN * (1.0 + sN) : lN + sN - lN * sN;
    double p = 2.0 * lN - q;
    return clampRGB(hue2rgb(p, q, hN + 1.0/3.0) * 255.0, hue2rgb(p, q, hN) * 255.0, hue2rgb(p, q, hN - 1.0/3.0) * 255.0);
}

#endif