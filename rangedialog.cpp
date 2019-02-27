#include "rangedialog.h"
#include "ui_rangedialog.h"

rangedialog::rangedialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::rangedialog),
    value(0)
{
    ui->setupUi(this);
    this->setWindowTitle("Range Operation");
    connect(ui->spinBox_min,SIGNAL(valueChanged(int)),this,SLOT(minchange(int)));
    connect(ui->spinBox_max,SIGNAL(valueChanged(int)),this,SLOT(maxchange(int)));
    connect(ui->lineEdit,SIGNAL(textChanged(QString)),this,SLOT(valuechange(QString)));
}

rangedialog::~rangedialog()
{
    delete ui;
}

void rangedialog::setmax(int v_max)
{
    max = v_max;
    ui->spinBox_max->setMaximum(max);
    ui->spinBox_max->setValue(max);
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

int rangedialog::getvalue()
{
    return value;
}

int rangedialog::getmax()
{
    return max;
}

void rangedialog::maxchange(int v)
{
    max = v;
}

void rangedialog::minchange(int v)
{
    min = v;
}

void rangedialog::valuechange(QString v)
{
    value = v.toInt();
    if(value < 0 || value > max)
    {
        QMessageBox::critical(this,"Error","Invalid value!");
        return;
    }
}
