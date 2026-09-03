#include "PdfReportGenerator.hpp"
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QPdfWriter>
#include <QSysInfo>

namespace ro_screenshot {

PdfReportGenerator::PdfReportGenerator(QObject *parent) : QObject(parent) {}

bool PdfReportGenerator::generateReport(const QString &outputPath,
                                        const QVector<PdfReportEntry> &entries,
                                        const PdfReportOptions &options) {
  if (outputPath.isEmpty() || entries.isEmpty())
    return false;

  QPdfWriter writer(outputPath);
  writer.setPageSize(QPageSize(QPageSize::A4));
  writer.setPageOrientation(QPageLayout::Portrait);
  writer.setResolution(300); // 300 DPI high resolution

  QPainter painter(&writer);
  if (!painter.isActive())
    return false;

  const int pageWidth = writer.width();
  const int pageHeight = writer.height();
  const int margin = 200;
  const int contentWidth = pageWidth - (margin * 2);

  // 1. Cover / Title Page
  painter.save();
  // Header bar
  painter.fillRect(QRect(0, 0, pageWidth, 400), QColor("#1E293B"));

  QFont titleFont("Sans Serif", 24, QFont::Bold);
  painter.setFont(titleFont);
  painter.setPen(Qt::white);
  painter.drawText(QRect(margin, 120, contentWidth, 200),
                   Qt::AlignLeft | Qt::AlignVCenter, options.title);

  // Subtitle / metadata
  QFont metaFont("Sans Serif", 10, QFont::Normal);
  painter.setFont(metaFont);
  painter.setPen(QColor("#334155"));

  int currentY = 550;
  painter.drawText(
      margin, currentY,
      QString("Generated: %1")
          .arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
  currentY += 80;
  painter.drawText(
      margin, currentY,
      QString("Author: %1 (%2)").arg(options.author, options.organization));
  currentY += 80;
  painter.drawText(margin, currentY,
                   QString("Total Screenshots: %1").arg(entries.size()));
  currentY += 80;

  if (options.includeSystemInfo) {
    painter.drawText(margin, currentY,
                     QString("System: %1 | Kernel: %2 | Architecture: %3")
                         .arg(QSysInfo::prettyProductName(),
                              QSysInfo::kernelVersion(),
                              QSysInfo::currentCpuArchitecture()));
    currentY += 100;
  }

  if (!options.notes.isEmpty()) {
    painter.drawText(margin, currentY, "Notes:");
    currentY += 60;
    QRect notesRect(margin, currentY, contentWidth, 400);
    painter.drawText(notesRect, Qt::TextWordWrap, options.notes);
    currentY += 450;
  }

  // Summary list / Table of Contents
  if (options.includeTableOfContents) {
    QFont tocHeader("Sans Serif", 14, QFont::Bold);
    painter.setFont(tocHeader);
    painter.drawText(margin, currentY, "Table of Contents / Items");
    currentY += 80;

    painter.setFont(metaFont);
    int itemNum = 1;
    for (const auto &entry : entries) {
      if (currentY > pageHeight - 300)
        break;
      QString line = QString("%1. %2").arg(itemNum++).arg(
          entry.title.isEmpty() ? QFileInfo(entry.imagePath).fileName()
                                : entry.title);
      painter.drawText(margin, currentY, line);
      currentY += 60;
    }
  }

  painter.restore();

  // 2. Individual Screenshot Pages
  for (int i = 0; i < entries.size(); ++i) {
    writer.newPage();
    const auto &entry = entries[i];

    // Page header
    painter.fillRect(QRect(0, 0, pageWidth, 120), QColor("#0F172A"));
    painter.setFont(QFont("Sans Serif", 12, QFont::Bold));
    painter.setPen(Qt::white);
    QString headerText =
        QString("Item %1 / %2: %3")
            .arg(i + 1)
            .arg(entries.size())
            .arg(entry.title.isEmpty() ? QFileInfo(entry.imagePath).fileName()
                                       : entry.title);
    painter.drawText(QRect(margin, 20, contentWidth, 80),
                     Qt::AlignLeft | Qt::AlignVCenter, headerText);

    // Page footer
    painter.setFont(QFont("Sans Serif", 8));
    painter.setPen(QColor("#64748B"));
    painter.drawText(QRect(margin, pageHeight - 120, contentWidth, 60),
                     Qt::AlignCenter,
                     QString("ro-ScreenShot Report • Page %1").arg(i + 2));

    int itemY = 180;

    // Metadata box
    painter.setFont(QFont("Sans Serif", 9));
    painter.setPen(QColor("#334155"));
    if (entry.timestamp.isValid()) {
      painter.drawText(
          margin, itemY,
          QString("Captured: %1").arg(entry.timestamp.toString(Qt::ISODate)));
      itemY += 50;
    }
    if (!entry.tags.isEmpty()) {
      painter.drawText(margin, itemY, QString("Tags: %1").arg(entry.tags));
      itemY += 50;
    }
    if (!entry.description.isEmpty()) {
      QRect descRect(margin, itemY, contentWidth, 120);
      painter.drawText(descRect, Qt::TextWordWrap,
                       QString("Description: %1").arg(entry.description));
      itemY += 140;
    }

    // Screenshot Image
    QImage img(entry.imagePath);
    if (!img.isNull()) {
      int maxImgW = contentWidth;
      int maxImgH = pageHeight - itemY - 200;
      QImage scaled = img.scaled(maxImgW, maxImgH, Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);

      int drawX = margin + (contentWidth - scaled.width()) / 2;
      painter.drawImage(drawX, itemY, scaled);

      // Border around image
      painter.setPen(QPen(QColor("#CBD5E1"), 2));
      painter.setBrush(Qt::NoBrush);
      painter.drawRect(drawX, itemY, scaled.width(), scaled.height());
    }
  }

  painter.end();
  return true;
}

} // namespace ro_screenshot
