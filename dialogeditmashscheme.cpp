#include "dialogeditmashscheme.h"
#include "ui_dialogeditmashscheme.h"
#include <QFileDialog>
#include <QTextStream>

/*------------------------------------------------------------------
  Purpose  : This is the constructor for the Mash Scheme Editor
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
DialogEditMashScheme::DialogEditMashScheme(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditMashScheme)
{
    QString mash_scheme("maisch.sch");

    ui->setupUi(this);

    QFile file(mash_scheme);
    file.open(QFile::ReadWrite | QFile::Text);
    ui->plainTextEdit->setPlainText(file.readAll());
    file.flush();
    file.close();
} // DialogEditMashScheme::DialogEditMashScheme()

/*------------------------------------------------------------------
  Purpose  : This is the destructor for the Mash Scheme Editor
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
DialogEditMashScheme::~DialogEditMashScheme()
{
    delete ui;
} // DialogEditMashScheme::~DialogEditMashScheme()

/*------------------------------------------------------------------
  Purpose  : This SLOT is called when 'Load Mash Scheme...' is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void DialogEditMashScheme::on_pushButton_clicked()
{
    QString file1Name = QFileDialog::getOpenFileName(this,
             "Open Mash Scheme File", "./", "Mash Scheme Files (*.sch)");
    if (!file1Name.isEmpty())
    {
        QFile file(file1Name);
        file.open(QFile::ReadOnly | QFile::Text);
        ui->plainTextEdit->setPlainText(file.readAll());
        file.flush();
        file.close();
    } // if
} // DialogEditMashScheme::on_pushButton_clicked()

/*------------------------------------------------------------------
  Purpose  : This is called when 'Save as...' is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void DialogEditMashScheme::on_pushButton_2_clicked()
{
    QString file1Name = QFileDialog::getSaveFileName(this,
             "Save As...", "./", "Mash Scheme Files (*.sch)");
    if (!file1Name.isEmpty())
    {
        QFile file(file1Name);
        file.open(QFile::WriteOnly | QFile::Text);
        QTextStream out (&file);
        out << ui->plainTextEdit->toPlainText();
        file.close();
    } // if
} // DialogEditMashScheme::on_pushButton_2_clicked()

/*------------------------------------------------------------------
  Purpose  : This is called when 'Save' is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void DialogEditMashScheme::on_pushButton_3_clicked()
{
    QFile file("maisch.sch"); // default Mash Scheme filename
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream out (&file);
    out << ui->plainTextEdit->toPlainText();
    file.close();
} // DialogEditMashScheme::on_pushButton_3_clicked()
