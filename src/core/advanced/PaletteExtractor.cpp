#include "PaletteExtractor.hpp"
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <cmath>

namespace ro_screenshot {

struct TailwindRef {
  const char *name;
  const char *className;
  int r, g, b;
};

static const TailwindRef kTailwindPalette[] = {
    {"slate-50", "bg-slate-50", 248, 250, 252},
    {"slate-200", "bg-slate-200", 226, 232, 240},
    {"slate-500", "bg-slate-500", 100, 116, 139},
    {"slate-700", "bg-slate-700", 51, 65, 85},
    {"slate-900", "bg-slate-900", 15, 23, 42},
    {"red-500", "bg-red-500", 239, 68, 68},
    {"red-700", "bg-red-700", 185, 28, 28},
    {"orange-500", "bg-orange-500", 249, 115, 22},
    {"amber-500", "bg-amber-500", 245, 158, 11},
    {"yellow-400", "bg-yellow-400", 250, 204, 21},
    {"emerald-500", "bg-emerald-500", 16, 185, 129},
    {"teal-500", "bg-teal-500", 20, 184, 166},
    {"cyan-500", "bg-cyan-500", 6, 182, 212},
    {"sky-500", "bg-sky-500", 14, 165, 233},
    {"blue-600", "bg-blue-600", 37, 99, 235},
    {"indigo-600", "bg-indigo-600", 79, 70, 229},
    {"violet-500", "bg-violet-500", 139, 92, 246},
    {"purple-600", "bg-purple-600", 147, 51, 234},
    {"fuchsia-500", "bg-fuchsia-500", 217, 70, 239},
    {"pink-500", "bg-pink-500", 236, 72, 153},
    {"rose-500", "bg-rose-500", 244, 63, 94},
    {"zinc-900", "bg-zinc-900", 24, 24, 27},
    {"neutral-100", "bg-neutral-100", 245, 245, 245},
    {"white", "bg-white", 255, 255, 255},
    {"black", "bg-black", 0, 0, 0}};

PaletteExtractor::PaletteExtractor(QObject *parent) : QObject(parent) {}

QString PaletteExtractor::matchTailwindColor(const QColor &color) {
  double minDistance = 1e9;
  QString bestName = QStringLiteral("slate-500");

  int cr = color.red();
  int cg = color.green();
  int cb = color.blue();

  for (const auto &ref : kTailwindPalette) {
    // Weighted Euclidean distance (perceptual approximation)
    double dr = (cr - ref.r) * 0.30;
    double dg = (cg - ref.g) * 0.59;
    double db = (cb - ref.b) * 0.11;
    double dist = (dr * dr) + (dg * dg) + (db * db);

    if (dist < minDistance) {
      minDistance = dist;
      bestName = QString::fromUtf8(ref.className);
    }
  }
  return bestName;
}

QVector<PaletteColor> PaletteExtractor::extract(const QImage &image,
                                                int maxColors) {
  if (image.isNull())
    return {};

  // Downsample image for performance
  QImage thumb =
      image.scaled(120, 120, Qt::KeepAspectRatio, Qt::FastTransformation);
  if (thumb.format() != QImage::Format_ARGB32 &&
      thumb.format() != QImage::Format_RGB32) {
    thumb = thumb.convertToFormat(QImage::Format_ARGB32);
  }

  // Quantize colors into 4-bit buckets (4096 bins)
  QHash<quint32, int> histogram;
  int totalPixels = 0;

  for (int y = 0; y < thumb.height(); ++y) {
    const QRgb *line = reinterpret_cast<const QRgb *>(thumb.constScanLine(y));
    for (int x = 0; x < thumb.width(); ++x) {
      QRgb p = line[x];
      if (qAlpha(p) < 50)
        continue; // Skip transparent
      // Quantize to 5 bits per channel
      int r = (qRed(p) >> 3) << 3;
      int g = (qGreen(p) >> 3) << 3;
      int b = (qBlue(p) >> 3) << 3;
      quint32 key = (static_cast<quint32>(r) << 16) |
                    (static_cast<quint32>(g) << 8) | static_cast<quint32>(b);
      histogram[key]++;
      totalPixels++;
    }
  }

  if (totalPixels == 0)
    return {};

  struct Bin {
    quint32 key;
    int count;
  };
  QVector<Bin> bins;
  bins.reserve(histogram.size());
  for (auto it = histogram.constBegin(); it != histogram.constEnd(); ++it) {
    bins.append({it.key(), it.value()});
  }

  std::sort(bins.begin(), bins.end(),
            [](const Bin &a, const Bin &b) { return a.count > b.count; });

  QVector<PaletteColor> result;
  for (const auto &b : bins) {
    if (result.size() >= maxColors)
      break;

    int r = (b.key >> 16) & 0xFF;
    int g = (b.key >> 8) & 0xFF;
    int blue = b.key & 0xFF;
    QColor col(r, g, blue);

    // Check minimum visual distance from existing selected colors to avoid
    // near-duplicates
    bool tooClose = false;
    for (const auto &existing : result) {
      int dr = existing.color.red() - r;
      int dg = existing.color.green() - g;
      int db = existing.color.blue() - blue;
      if ((dr * dr + dg * dg + db * db) < 1200) {
        tooClose = true;
        break;
      }
    }
    if (tooClose && result.size() > 0)
      continue;

    PaletteColor pc;
    pc.color = col;
    pc.hex = col.name().toUpper();
    pc.rgb = QString("rgb(%1, %2, %3)").arg(r).arg(g).arg(blue);
    pc.hsl = QString("hsl(%1, %2%, %3%)")
                 .arg(std::max(0, col.hslHue()))
                 .arg(static_cast<int>(col.hslSaturationF() * 100))
                 .arg(static_cast<int>(col.lightnessF() * 100));
    pc.tailwindClass = matchTailwindColor(col);
    pc.tailwindName = pc.tailwindClass.mid(3); // strip "bg-"
    pc.pixelCount = b.count;
    pc.dominancePercent = (static_cast<double>(b.count) / totalPixels) * 100.0;

    result.append(pc);
  }

  return result;
}

QString
PaletteExtractor::exportAsTailwindConfig(const QVector<PaletteColor> &palette) {
  QString out =
      "module.exports = {\n  theme: {\n    extend: {\n      colors: {\n";
  int idx = 1;
  for (const auto &p : palette) {
    out += QString("        'brand-%1': '%2', // %3\n")
               .arg(idx++)
               .arg(p.hex)
               .arg(p.tailwindName);
  }
  out += "      }\n    }\n  }\n};\n";
  return out;
}

QString
PaletteExtractor::exportAsCssVariables(const QVector<PaletteColor> &palette) {
  QString out = ":root {\n";
  int idx = 1;
  for (const auto &p : palette) {
    out += QString("  --color-palette-%1: %2; /* %3 */\n")
               .arg(idx++)
               .arg(p.hex)
               .arg(p.tailwindName);
  }
  out += "}\n";
  return out;
}

QString PaletteExtractor::exportAsJson(const QVector<PaletteColor> &palette) {
  QJsonArray arr;
  for (const auto &p : palette) {
    QJsonObject obj;
    obj["hex"] = p.hex;
    obj["rgb"] = p.rgb;
    obj["hsl"] = p.hsl;
    obj["tailwind"] = p.tailwindClass;
    obj["dominancePercent"] = p.dominancePercent;
    arr.append(obj);
  }
  return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

} // namespace ro_screenshot
