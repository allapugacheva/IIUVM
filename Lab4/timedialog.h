#ifndef TIMEDIALOG_H
#define TIMEDIALOG_H

#include <QDialog>
#include <QValidator>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

namespace Ui {
class TimeDialog;
}

class TimeValidator : public QRegularExpressionValidator {
public:
    TimeValidator(QObject* parent = nullptr) : QRegularExpressionValidator(QRegularExpression("^([01][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])$"), parent) {}
};

class TimeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimeDialog(QWidget *parent = nullptr);
    int getTime();
    ~TimeDialog();

private:
    Ui::TimeDialog *ui;
};

#endif // TIMEDIALOG_H
