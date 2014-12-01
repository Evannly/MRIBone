#ifndef RANGEDIALOG_H
#define RANGEDIALOG_H

#include <QDialog>

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

public slots:
    void maxchange();
    void minchange();
private:
    Ui::rangedialog *ui;
    int min;
    int max;
};

#endif // RANGEDIALOG_H
