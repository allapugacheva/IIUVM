#include "timedialog.h"
#include "ui_timedialog.h"

TimeDialog::TimeDialog(QWidget *parent) : QDialog(parent), ui(new Ui::TimeDialog)
{
    ui->setupUi(this);

    TimeValidator* validator = new TimeValidator(this);
    ui->timeEdit->setValidator(validator);

    connect(ui->timeEdit, &QLineEdit::textChanged, [=]() {
        if (ui->timeEdit->hasAcceptableInput())
            accept();
    });
}

int TimeDialog::getTime()
{
    QStringList timeParts = ui->timeEdit->text().split(":");

    bool ok1, ok2, ok3;
    int hours = timeParts[0].toInt(&ok1);
    int minutes = timeParts[1].toInt(&ok2);
    int seconds = timeParts[2].toInt(&ok3);

    int totalSeconds = hours * 3600 + minutes * 60 + seconds;
    return totalSeconds * 1000;
}

TimeDialog::~TimeDialog()
{
    delete ui;
}
