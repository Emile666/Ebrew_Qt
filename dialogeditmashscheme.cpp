/**************************************************************************************
** Filename    : dialogeditmashscheme.cpp
** Author      : Emile
** Purpose     : This file contains a dialog screen with which to read-from-file,
**               change and set a mash scheme.
** License     : This is free software: you can redistribute it and/or modify
**               it under the terms of the GNU General Public License as published by
**               the Free Software Foundation, either version 3 of the License, or
**               (at your option) any later version.
**
**               This file is distributed in the hope that it will be useful,
**               but WITHOUT ANY WARRANTY; without even the implied warranty of
**               MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**               GNU General Public License for more details.
**
**               You should have received a copy of the GNU General Public License
**               along with this file.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************************/
#include "dialogeditmashscheme.h"
#include "ui_dialogeditmashscheme.h"
#include <QFileDialog>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QTextBlock>
#include <QRegularExpression>

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
  Purpose  : This is called when 'Save as xml...' is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void DialogEditMashScheme::writeXmlFile(QFile *file)
{
    QTextDocument *doc = ui->plainTextEdit->document();
    QTextBlock block;
    QString    s;
    QStringList list1;
    uint8_t     i,j;

    if (file->open(QFile::WriteOnly | QFile::Text))
    {
        QXmlStreamWriter writer(file);
        writer.setAutoFormatting(true); // Takes care of EOL and indentation
        writer.writeStartDocument();    // Writes <?xml version="1.0" encoding="UTF-8"?>
        for (i = 0; i < 3; i++)
        {
            s = doc->findBlockByLineNumber(i).text();
            writer.writeComment(s);
        } // for i
        writer.writeStartElement("brew_session");
        writer.writeComment("1. Brew Session Parameters");
        writer.writeStartElement("settings");
        for (i = 3; i < 9; i++)
        {
            s = doc->findBlockByLineNumber(i).text();
            list1 = s.split(QRegularExpression("[(:)]"));
            if (list1.size() >= 4)
            {
                writer.writeStartElement("param");
                writer.writeAttribute("name",list1.at(0).trimmed());
                writer.writeAttribute("unit",list1.at(1));
                writer.writeCharacters(list1.at(3).trimmed());
                writer.writeEndElement();
            } // if
        } // for
        writer.writeEndElement(); // </brew_session>
        writer.device()->write(""); // force a write flush
        writer.device()->write("\n");

        // 2. Mash Trajectory
        for (i = 10; i < 12; i++)
        {
            s = doc->findBlockByLineNumber(i).text();
            writer.writeComment(s);
        } // for i
        writer.writeStartElement("mash_steps");
        while (!((s = doc->findBlockByLineNumber(i).text()).isEmpty()) && (i < 22))
        {
            list1 = s.split(QRegularExpression("[,]"));
            if (list1.size() >= 2)
            {
                writer.writeStartElement("step");
                writer.writeAttribute("temp", list1.at(0).trimmed());
                writer.writeAttribute("time", list1.at(1).trimmed());
                writer.writeEndElement(); // this is a self-closing tag <step ... />
            } // if
            i++;
        } // while
        writer.writeEndElement(); // </mash_steps>
        writer.device()->write(""); // force a write flush
        writer.device()->write("\n");

        // 3. Hop-additions
        for (j = i+1; j < i+3; j++)
        {
            s = doc->findBlockByLineNumber(j).text();
            writer.writeComment(s);
        } // for i
        writer.writeStartElement("hop_additions");
        while (!((s = doc->findBlockByLineNumber(j).text()).isEmpty()) && (j < i+10))
        {
            list1 = s.split(QRegularExpression("[,]"));
            if (list1.size() >= 2)
            {
                writer.writeStartElement("addition");
                writer.writeAttribute("time", list1.at(0).trimmed());
                writer.writeAttribute("hops", list1.at(1).trimmed());
                writer.writeEndElement(); // this is a self-closing tag <step ... />
            } // if
            j++;
        } // while
        writer.writeEndElement(); // </hop_additions>
        writer.device()->write(""); // force a write flush
        writer.device()->write("\n");

        // 4. Hop-stand (whirlpool)
        for (i = j+1; i < j+3; i++)
        {
            s = doc->findBlockByLineNumber(i).text();
            writer.writeComment(s);
        } // for i
        writer.writeStartElement("hop_stands");
        while (!((s = doc->findBlockByLineNumber(i).text()).isEmpty()) && (i < j+10))
        {
            list1 = s.split(QRegularExpression("[,]"));
            if (list1.size() >= 2)
            {
                writer.writeStartElement("stand");
                writer.writeAttribute("time", list1.at(0).trimmed());
                writer.writeAttribute("hops", list1.at(1).trimmed());
                writer.writeEndElement(); // this is a self-closing tag <step ... />
            } // if
            i++;
        } // // while
        writer.writeEndElement(); // </hop_additions>
        writer.writeEndElement(); // </mash_steps>
        writer.writeEndDocument();
        file->close();
    } // if
} // DialogEditMashScheme::writeXmlFile()

/*------------------------------------------------------------------
  Purpose  : This is called when 'Save as...' is clicked.
  Variables: -
  Returns  : -
  ------------------------------------------------------------------*/
void DialogEditMashScheme::on_pushButton_2_clicked()
{
    QString file1Name = QFileDialog::getSaveFileName(this,
             "Save As...", "./", "Mash Scheme Files (*.sch);;Brew Files (*.xml)");

    if (!file1Name.isEmpty())
    {
        QFile     file(file1Name);
        QFileInfo fileInfo(file1Name);
        QString   ext = fileInfo.suffix().toLower(); // get extension in lowercase

        if (ext == "xml")
        {
            writeXmlFile(&file);
        } // if
        else
        {
            file.open(QFile::WriteOnly | QFile::Text);
            QTextStream out (&file);
            out << ui->plainTextEdit->toPlainText();
            file.close();
        } // else
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
