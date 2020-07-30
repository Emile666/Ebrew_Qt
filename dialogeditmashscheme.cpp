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
    QString mash_scheme(MASHFILE); // Load default maisch.sch
    pEbrew = dynamic_cast<MainEbrew *>(parent);

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
  Purpose  : This is called when 'Save' is clicked. It save the
             current text into the default MASHFILE and re-initializes
             the variables from this file. It doesn't update the
             mash timers, so that you can change the mash scheme in
             the middle of an on-going mash phase.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void DialogEditMashScheme::on_pushButton_3_clicked()
{
    QFile file(MASHFILE); // default Mash Scheme filename
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream out (&file);
    out << ui->plainTextEdit->toPlainText();
    file.close();
    pEbrew->readMashSchemeFile(NO_INIT_TIMERS); // Read mash scheme, but don't init the mash timers
    pEbrew->initBrewDaySettings();              // Update mash, sparge and boil-settings
} // DialogEditMashScheme::on_pushButton_3_clicked()
