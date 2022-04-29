#include "ashirtdialog.h"
#include <QKeySequence>

AShirtDialog::AShirtDialog(QWidget *parent, Qt::WindowFlags windowFlags) : QDialog(parent, windowFlags)
{
  addAction(QString(), QKeySequence::Close, this, &AShirtDialog::close);
}

void AShirtDialog::show()
{
    QDialog::show(); // display the window
    raise(); // bring to the top (mac)
    activateWindow(); // alternate bring to the top (windows)
}
