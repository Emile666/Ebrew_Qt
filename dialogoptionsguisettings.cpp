#include "dialogoptionsguisettings.h"
#include "ui_dialogoptionsguisettings.h"
#include <QColorDialog>

dialogoptionsguisettings::dialogoptionsguisettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialogoptionsguisettings)
{
    pEbrew = dynamic_cast<MainEbrew *>(parent);

    ui->setupUi(this);
    ui->cbPipesNoFlow->setChecked(pEbrew->RegEbrew->value("CB_HIDEPIPES").toInt());

    updateButtonColor(ui->pbColInOff ,pEbrew->pipeInOff);
    updateButtonColor(ui->pbColInOn  ,pEbrew->pipeInOn);
    updateButtonColor(ui->pbColOutOff,pEbrew->pipeOutOff);
    updateButtonColor(ui->pbColOutOn ,pEbrew->pipeOutOn);
} // dialogoptionsguisettings::dialogoptionsguisettings()

dialogoptionsguisettings::~dialogoptionsguisettings()
{
    delete ui;
} // dialogoptionsguisettings::~dialogoptionsguisettings()

void dialogoptionsguisettings::on_buttonBox_accepted()
{
    pEbrew->RegEbrew->setValue("CB_HIDEPIPES",ui->cbPipesNoFlow->isChecked() ? 1 : 0);
} // dialogoptionsguisettings::on_buttonBox_accepted()

void dialogoptionsguisettings::on_pbColInOff_clicked()
{
    QColor selectedColor = QColorDialog::getColor(pEbrew->pipeInOff, this, "Select a Color for an input pipe with NO flow");

    // Check if OK is pressed (and not Cancel)
    if (selectedColor.isValid())
    {
        updateButtonColor(ui->pbColInOff,selectedColor); // Adjust color
        pEbrew->pipeInOff = selectedColor;
    } // if
} // dialogoptionsguisettings::on_pbColInOff_clicked

void dialogoptionsguisettings::on_pbColInOn_clicked()
{
    QColor selectedColor = QColorDialog::getColor(pEbrew->pipeInOn, this, "Select a Color for an input pipe with flow");

    // Check if OK is pressed (and not Cancel)
    if (selectedColor.isValid())
    {
        updateButtonColor(ui->pbColInOn,selectedColor); // Adjust color
        pEbrew->pipeInOn = selectedColor;
    } // if
} // dialogoptionsguisettings::on_pbColInOn_clicked()

void dialogoptionsguisettings::on_pbColOutOff_clicked()
{
    QColor selectedColor = QColorDialog::getColor(pEbrew->pipeOutOff, this, "Select a Color for an output pipe with NO flow");

    // Check if OK is pressed (and not Cancel)
    if (selectedColor.isValid())
    {
        updateButtonColor(ui->pbColOutOff,selectedColor); // Adjust color
        pEbrew->pipeOutOff = selectedColor;
    } // if
} // dialogoptionsguisettings::on_pbColOutOff_clicked()

void dialogoptionsguisettings::on_pbColOutOn_clicked()
{
    QColor selectedColor = QColorDialog::getColor(pEbrew->pipeOutOn, this, "Select a Color for an output pipe with flow");

    // Check if OK is pressed (and not Cancel)
    if (selectedColor.isValid())
    {
        updateButtonColor(ui->pbColOutOn,selectedColor); // Adjust color
        pEbrew->pipeOutOn = selectedColor;
    } // if
} // dialogoptionsguisettings::on_pbColOutOn_clicked()

void dialogoptionsguisettings::updateButtonColor(QPushButton *button, const QColor &color)
{
    // Calculate a hover-color which is a bit darker
    QColor hoverColor = color.darker(115);

    // Get text-color (white for dark buttons, black for light buttons)
    QString textColor = (color.lightness() < 128) ? "white" : "black";

    // Apply stylesheet to button
    QString style = QString(
                        "QPushButton {"
                        "  background-color: %1;"
                        "  color: %2;"
                        "  border: 1px solid #ababab;"
                        "  border-radius: 4px;"
                        "  padding: 6px;"
                        "}"
                        "QPushButton:hover {"
                        "  background-color: %3;"
                        "}"
                        )
                        .arg(color.name())
                        .arg(textColor)
                        .arg(hoverColor.name());
    button->setStyleSheet(style);
} // dialogoptionsguisettings::updateButtonColor()
