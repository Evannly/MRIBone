#include "rangedialog.h"
#include "ui_rangedialog.h"

rangedialog::rangedialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::rangedialog)
{
    ui->setupUi(this);
    connect(ui->spinBox_min,SIGNAL(valueChanged(int)),this,SLOT(minchange()));
    connect(ui->spinBox_max,SIGNAL(valueChanged(int)),this,SLOT(maxchange()));
}

rangedialog::~rangedialog()
{
    delete ui;
}

void rangedialog::setmax(int v_max)
{
    max = v_max;
    ui->spinBox_max->setMaximum(max);
}

void rangedialog::setmin(int v_min)
{
    min = v_min;
    ui->spinBox_min->setMinimum(min);
}

int rangedialog::getmin()
{
    return min;
}

int rangedialog::getmax()
{
    return max;
}

void rangedialog::maxchange()
{
    max = ui->spinBox_max->value();
}

void rangedialog::minchange()
{
    min = ui->spinBox_min->value();
}
