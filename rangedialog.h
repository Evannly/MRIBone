#ifndef RANGEDIALOG_H
#define RANGEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QMessageBox>

namespace Ui {
class rangedialog;
}

class rangedialog : public QDialog
{
    Q_OBJECT

public:

    explicit rangedialog(QWidget *parent = 0);
    ~rangedialog();
    int getmin();
    int getmax();
    void setmax(int);
    void setmin(int);
    int getvalue();

public slots:
    void maxchange(int);
    void minchange(int);
    void valuechange(QString);
private:
    int min;
    int max;
    int value;
    Ui::rangedialog *ui;
};

#endif // RANGEDIALOG_H
