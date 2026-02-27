// Copyright (C) 2026 Sean Moon
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "CountdownWidget.h"

#include <QDateEdit>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

// ---------------------------------------------------------------------------
// Internal display widget
// ---------------------------------------------------------------------------

class CountdownDisplay : public QWidget {
    Q_OBJECT
public:
    CountdownDisplay(const QString& label, const QString& targetDate, QWidget* parent)
        : QWidget(parent), label_(label), targetDate_(QDate::fromString(targetDate, Qt::ISODate)) {
        buildUi();

        timer_ = new QTimer(this);
        timer_->setInterval(1000);
        connect(timer_, &QTimer::timeout, this, &CountdownDisplay::refresh);

        if (targetDate_.isValid()) {
            showCountdown();
        } else {
            showEditor();
        }
    }

    QString label() const { return label_; }
    QString targetDateString() const {
        return targetDate_.isValid() ? targetDate_.toString(Qt::ISODate) : QString();
    }

signals:
    void dataChanged(const QString& label, const QString& targetDate);

private slots:
    void refresh() {
        updateCountdownLabels();
    }

    void onSave() {
        label_      = labelEdit_->text().trimmed();
        targetDate_ = dateEdit_->date();
        if (label_.isEmpty()) label_ = "Event";
        eventNameLabel_->setText(label_);
        emit dataChanged(label_, targetDate_.toString(Qt::ISODate));
        showCountdown();
    }

    void onEdit() {
        timer_->stop();
        stack_->setCurrentIndex(1);
    }

    void onCancel() {
        if (targetDate_.isValid()) {
            showCountdown();
        }
        // If no valid date yet, stay in editor (nothing to cancel to)
    }

private:
    void buildUi() {
        const QString btnStyle =
            "QPushButton {"
            "  background-color: #26263a;"
            "  color: #c8cee8;"
            "  border: 1px solid #3e3e60;"
            "  border-radius: 6px;"
            "  padding: 4px 14px;"
            "  font-size: 12px;"
            "}"
            "QPushButton:hover { background-color: #303050; }"
            "QPushButton:pressed { background-color: #1e1e30; }";

        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(0);

        stack_ = new QStackedWidget(this);
        root->addWidget(stack_);

        // ---- Page 0: countdown view ----------------------------------------
        auto* viewPage = new QWidget(stack_);
        auto* vbox     = new QVBoxLayout(viewPage);
        vbox->setContentsMargins(12, 14, 12, 14);
        vbox->setSpacing(4);

        eventNameLabel_ = new QLabel(viewPage);
        eventNameLabel_->setAlignment(Qt::AlignCenter);
        eventNameLabel_->setStyleSheet(
            "color: #9090b8; font-size: 11px; font-weight: bold;"
            " letter-spacing: 2px; background: transparent;");

        daysLabel_ = new QLabel(viewPage);
        daysLabel_->setAlignment(Qt::AlignCenter);
        daysLabel_->setStyleSheet(
            "color: #e8eaf0; font-size: 52px; font-weight: bold; background: transparent;");
        QFont mono = daysLabel_->font();
        mono.setStyleHint(QFont::Monospace);
        daysLabel_->setFont(mono);

        daysWordLabel_ = new QLabel("days", viewPage);
        daysWordLabel_->setAlignment(Qt::AlignCenter);
        daysWordLabel_->setStyleSheet(
            "color: #606080; font-size: 13px; background: transparent;");

        hmsLabel_ = new QLabel(viewPage);
        hmsLabel_->setAlignment(Qt::AlignCenter);
        hmsLabel_->setStyleSheet(
            "color: #5577aa; font-size: 14px; background: transparent;");
        hmsLabel_->setFont(mono);

        dateLabel_ = new QLabel(viewPage);
        dateLabel_->setAlignment(Qt::AlignCenter);
        dateLabel_->setStyleSheet(
            "color: #505070; font-size: 10px; background: transparent;");

        auto* editBtn = new QPushButton("Edit", viewPage);
        editBtn->setStyleSheet(btnStyle);
        editBtn->setCursor(Qt::PointingHandCursor);
        editBtn->setFixedWidth(70);

        auto* btnRow    = new QWidget(viewPage);
        auto* btnLayout = new QHBoxLayout(btnRow);
        btnLayout->setContentsMargins(0, 0, 0, 0);
        btnLayout->setAlignment(Qt::AlignCenter);
        btnRow->setStyleSheet("background: transparent;");
        btnLayout->addWidget(editBtn);

        connect(editBtn, &QPushButton::clicked, this, &CountdownDisplay::onEdit);

        vbox->addStretch(1);
        vbox->addWidget(eventNameLabel_);
        vbox->addSpacing(4);
        vbox->addWidget(daysLabel_);
        vbox->addWidget(daysWordLabel_);
        vbox->addSpacing(2);
        vbox->addWidget(hmsLabel_);
        vbox->addSpacing(4);
        vbox->addWidget(dateLabel_);
        vbox->addStretch(1);
        vbox->addWidget(btnRow);

        stack_->addWidget(viewPage); // index 0

        // ---- Page 1: editor ------------------------------------------------
        auto* editPage  = new QWidget(stack_);
        auto* evbox     = new QVBoxLayout(editPage);
        evbox->setContentsMargins(16, 16, 16, 16);
        evbox->setSpacing(10);

        const QString inputStyle =
            "QLineEdit, QDateEdit {"
            "  background-color: #1e1e30;"
            "  color: #c8cee8;"
            "  border: 1px solid #3e3e60;"
            "  border-radius: 5px;"
            "  padding: 4px 8px;"
            "  font-size: 12px;"
            "}"
            "QDateEdit::drop-down { border: none; }"
            "QDateEdit::down-arrow { color: #c8cee8; }";

        auto* nameHdr = new QLabel("Event name", editPage);
        nameHdr->setStyleSheet("color: #9090b8; font-size: 11px; background: transparent;");

        labelEdit_ = new QLineEdit(editPage);
        labelEdit_->setPlaceholderText("e.g. Vacation");
        labelEdit_->setStyleSheet(inputStyle);

        auto* dateHdr = new QLabel("Target date", editPage);
        dateHdr->setStyleSheet("color: #9090b8; font-size: 11px; background: transparent;");

        dateEdit_ = new QDateEdit(editPage);
        dateEdit_->setCalendarPopup(true);
        dateEdit_->setDisplayFormat("yyyy-MM-dd");
        dateEdit_->setDate(QDate::currentDate().addDays(30));
        dateEdit_->setStyleSheet(inputStyle);

        auto* saveBtn   = new QPushButton("Save", editPage);
        auto* cancelBtn = new QPushButton("Cancel", editPage);
        saveBtn->setStyleSheet(btnStyle);
        cancelBtn->setStyleSheet(btnStyle);
        saveBtn->setCursor(Qt::PointingHandCursor);
        cancelBtn->setCursor(Qt::PointingHandCursor);

        connect(saveBtn,   &QPushButton::clicked, this, &CountdownDisplay::onSave);
        connect(cancelBtn, &QPushButton::clicked, this, &CountdownDisplay::onCancel);

        auto* saveBtnRow    = new QWidget(editPage);
        auto* saveBtnLayout = new QHBoxLayout(saveBtnRow);
        saveBtnLayout->setContentsMargins(0, 0, 0, 0);
        saveBtnLayout->setAlignment(Qt::AlignCenter);
        saveBtnLayout->setSpacing(8);
        saveBtnRow->setStyleSheet("background: transparent;");
        saveBtnLayout->addWidget(saveBtn);
        saveBtnLayout->addWidget(cancelBtn);

        evbox->addStretch(1);
        evbox->addWidget(nameHdr);
        evbox->addWidget(labelEdit_);
        evbox->addWidget(dateHdr);
        evbox->addWidget(dateEdit_);
        evbox->addSpacing(4);
        evbox->addWidget(saveBtnRow);
        evbox->addStretch(1);

        stack_->addWidget(editPage); // index 1
    }

    void showCountdown() {
        // Populate editor fields to match current state
        labelEdit_->setText(label_);
        if (targetDate_.isValid()) dateEdit_->setDate(targetDate_);

        eventNameLabel_->setText(label_.isEmpty() ? "Event" : label_.toUpper());
        updateCountdownLabels();
        stack_->setCurrentIndex(0);
        timer_->start();
    }

    void showEditor() {
        labelEdit_->setText(label_);
        if (targetDate_.isValid()) dateEdit_->setDate(targetDate_);
        stack_->setCurrentIndex(1);
    }

    void updateCountdownLabels() {
        if (!targetDate_.isValid()) return;

        const QDateTime now    = QDateTime::currentDateTime();
        const QDateTime target = QDateTime(targetDate_, QTime(0, 0, 0));
        const qint64    secs   = now.secsTo(target);

        if (secs <= 0) {
            // Past or today
            const qint64 absDays = qAbs(now.date().daysTo(targetDate_));
            if (absDays == 0) {
                daysLabel_->setText("Today!");
                daysLabel_->setStyleSheet(
                    "color: #55cc88; font-size: 38px; font-weight: bold; background: transparent;");
                daysWordLabel_->hide();
                hmsLabel_->hide();
            } else {
                daysLabel_->setText(QString::number(absDays));
                daysLabel_->setStyleSheet(
                    "color: #888899; font-size: 52px; font-weight: bold; background: transparent;");
                daysWordLabel_->setText(absDays == 1 ? "day ago" : "days ago");
                daysWordLabel_->show();
                hmsLabel_->hide();
            }
        } else {
            const qint64 days = secs / 86400;
            const qint64 rem  = secs % 86400;
            const qint64 h    = rem / 3600;
            const qint64 m    = (rem % 3600) / 60;
            const qint64 s    = rem % 60;

            daysLabel_->setText(QString::number(days));
            daysLabel_->setStyleSheet(
                "color: #e8eaf0; font-size: 52px; font-weight: bold; background: transparent;");
            daysWordLabel_->setText(days == 1 ? "day" : "days");
            daysWordLabel_->show();
            hmsLabel_->setText(QString("%1:%2:%3")
                .arg(h, 2, 10, QChar('0'))
                .arg(m, 2, 10, QChar('0'))
                .arg(s, 2, 10, QChar('0')));
            hmsLabel_->show();
        }

        dateLabel_->setText(targetDate_.toString("MMMM d, yyyy"));
    }

    QTimer*        timer_;
    QStackedWidget* stack_;

    // View widgets
    QLabel* eventNameLabel_;
    QLabel* daysLabel_;
    QLabel* daysWordLabel_;
    QLabel* hmsLabel_;
    QLabel* dateLabel_;

    // Editor widgets
    QLineEdit* labelEdit_;
    QDateEdit* dateEdit_;

    QString label_;
    QDate   targetDate_;
};

// ---------------------------------------------------------------------------
// CountdownWidget (plugin entry point)
// ---------------------------------------------------------------------------

CountdownWidget::CountdownWidget(QObject* parent) : QObject(parent) {}

void CountdownWidget::initialize(dashboard::WidgetContext* /*context*/) {}

QWidget* CountdownWidget::createWidget(QWidget* parent) {
    auto* display = new CountdownDisplay(label_, targetDate_, parent);
    QObject::connect(display, &CountdownDisplay::dataChanged,
                     display, [this](const QString& label, const QString& date) {
                         label_      = label;
                         targetDate_ = date;
                     });
    return display;
}

QJsonObject CountdownWidget::serialize() const {
    return {
        {"label",      label_},
        {"targetDate", targetDate_},
    };
}

void CountdownWidget::deserialize(const QJsonObject& data) {
    label_      = data.value("label").toString();
    targetDate_ = data.value("targetDate").toString();
}

dashboard::WidgetMetadata CountdownWidget::metadata() const {
    return {
        .name        = "Countdown",
        .version     = "1.0.0",
        .author      = "Dashboard",
        .description = "Count down the days to any event",
        .minSize     = QSize(160, 200),
        .maxSize     = QSize(400, 400),
        .defaultSize = QSize(220, 240),
    };
}

#include "CountdownWidget.moc"
