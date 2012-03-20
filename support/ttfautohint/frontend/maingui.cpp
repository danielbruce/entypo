// maingui.cpp

// Copyright (C) 2012 by Werner Lemberg.
//
// This file is part of the ttfautohint library, and may only be used,
// modified, and distributed under the terms given in `COPYING'.  By
// continuing to use, modify, or distribute this file you indicate that you
// have read `COPYING' and understand and accept it fully.
//
// The file `COPYING' mentioned in the previous paragraph is distributed
// with the ttfautohint library.


#include <config.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <QtGui>

#include "maingui.h"

#include <ttfautohint.h>


// XXX Qt 4.8 bug: locale->quoteString("foo")
//                 inserts wrongly encoded quote characters
//                 into rich text QString
#if HAVE_QT_QUOTESTRING
#  define QUOTE_STRING(x) locale->quoteString(x)
#  define QUOTE_STRING_LITERAL(x) locale->quoteString(x)
#else
#  define QUOTE_STRING(x) "\"" + x + "\""
#  define QUOTE_STRING_LITERAL(x) "\"" x "\""
#endif


Main_GUI::Main_GUI(int range_min,
                   int range_max,
                   bool ignore,
                   bool pre,
                   int fallback)
: hinting_range_min(range_min),
  hinting_range_max(range_max),
  ignore_permissions(ignore),
  pre_hinting(pre),
  latin_fallback(fallback)
{
  create_layout();
  create_connections();
  create_actions();
  create_menus();
  create_status_bar();

  read_settings();

  setUnifiedTitleAndToolBarOnMac(true);

  // XXX register translations somewhere and loop over them
  if (QLocale::system().name() == "en_US")
    locale = new QLocale;
  else
    locale = new QLocale(QLocale::C);
}


// overloading

void
Main_GUI::closeEvent(QCloseEvent* event)
{
  write_settings();
  event->accept();
}


void
Main_GUI::about()
{
  QMessageBox::about(this,
                     tr("About TTFautohint"),
                     tr("<p>This is <b>TTFautohint</b> version %1<br>"
                        " Copyright %2 2011-2012<br>"
                        " by Werner Lemberg <tt>&lt;wl@gnu.org&gt;</tt></p>"
                        ""
                        "<p><b>TTFautohint</b> adds new auto-generated hints"
                        " to a TrueType font or TrueType collection.</p>"
                        ""
                        "<p>License:"
                        " <a href='http://www.freetype.org/FTL.TXT'>FreeType"
                        " License (FTL)</a> or"
                        " <a href='http://www.freetype.org/GPL.TXT'>GNU"
                        " GPLv2</a></p>")
                        .arg(VERSION)
                        .arg(QChar(0xA9)));
}


void
Main_GUI::browse_input()
{
  // XXX remember last directory
  QString file = QFileDialog::getOpenFileName(
                   this,
                   tr("Open Input File"),
                   QDir::homePath(),
                   "");
  if (!file.isEmpty())
    input_line->setText(QDir::toNativeSeparators(file));
}


void
Main_GUI::browse_output()
{
  // XXX remember last directory
  QString file = QFileDialog::getSaveFileName(
                   this,
                   tr("Open Output File"),
                   QDir::homePath(),
                   "");

  if (!file.isEmpty())
    output_line->setText(QDir::toNativeSeparators(file));
}


void
Main_GUI::check_min()
{
  int min = min_box->value();
  int max = max_box->value();
  if (min > max)
    max_box->setValue(min);
}


void
Main_GUI::check_max()
{
  int min = min_box->value();
  int max = max_box->value();
  if (max < min)
    min_box->setValue(max);
}


void
Main_GUI::check_run()
{
  if (input_line->text().isEmpty() || output_line->text().isEmpty())
    run_button->setEnabled(false);
  else
    run_button->setEnabled(true);
}


void
Main_GUI::absolute_input()
{
  QString input_name = QDir::fromNativeSeparators(input_line->text());
  if (!input_name.isEmpty()
      && QDir::isRelativePath(input_name))
  {
    QDir cur_path(QDir::currentPath() + "/" + input_name);
    input_line->setText(QDir::toNativeSeparators(cur_path.absolutePath()));
  }
}


void
Main_GUI::absolute_output()
{
  QString output_name = QDir::fromNativeSeparators(output_line->text());
  if (!output_name.isEmpty()
      && QDir::isRelativePath(output_name))
  {
    QDir cur_path(QDir::currentPath() + "/" + output_name);
    output_line->setText(QDir::toNativeSeparators(cur_path.absolutePath()));
  }
}


int
Main_GUI::check_filenames(const QString& input_name,
                          const QString& output_name)
{
  if (!QFile::exists(input_name))
  {
    QMessageBox::warning(
      this,
      "TTFautohint",
      tr("The file %1 cannot be found.")
         .arg(QUOTE_STRING(QDir::toNativeSeparators(input_name))),
      QMessageBox::Ok,
      QMessageBox::Ok);
    return 0;
  }

  if (input_name == output_name)
  {
    QMessageBox::warning(
      this,
      "TTFautohint",
      tr("Input and output file names must be different."),
      QMessageBox::Ok,
      QMessageBox::Ok);
    return 0;
  }

  if (QFile::exists(output_name))
  {
    int ret = QMessageBox::warning(
                this,
                "TTFautohint",
                tr("The file %1 already exists.\n"
                   "Overwrite?")
                   .arg(QUOTE_STRING(QDir::toNativeSeparators(output_name))),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
    if (ret == QMessageBox::No)
      return 0;
  }

  return 1;
}


int
Main_GUI::open_files(const QString& input_name,
                     FILE** in,
                     const QString& output_name,
                     FILE** out)
{
  const int buf_len = 1024;
  char buf[buf_len];

  *in = fopen(qPrintable(input_name), "rb");
  if (!*in)
  {
    strerror_r(errno, buf, buf_len);
    QMessageBox::warning(
      this,
      "TTFautohint",
      tr("The following error occurred while opening input file %1:\n")
         .arg(QUOTE_STRING(QDir::toNativeSeparators(input_name)))
        + QString::fromLocal8Bit(buf),
      QMessageBox::Ok,
      QMessageBox::Ok);
    return 0;
  }

  *out = fopen(qPrintable(output_name), "wb");
  if (!*out)
  {
    strerror_r(errno, buf, buf_len);
    QMessageBox::warning(
      this,
      "TTFautohint",
      tr("The following error occurred while opening output file %1:\n")
         .arg(QUOTE_STRING(QDir::toNativeSeparators(output_name)))
        + QString::fromLocal8Bit(buf),
      QMessageBox::Ok,
      QMessageBox::Ok);
    return 0;
  }

  return 1;
}


extern "C" {

struct GUI_Progress_Data
{
  long last_sfnt;
  bool begin;
  QProgressDialog* dialog;
};


int
gui_progress(long curr_idx,
             long num_glyphs,
             long curr_sfnt,
             long num_sfnts,
             void* user)
{
  GUI_Progress_Data* data = (GUI_Progress_Data*)user;

  if (num_sfnts > 1 && curr_sfnt != data->last_sfnt)
  {
    data->dialog->setLabelText(QCoreApplication::translate(
                                 "GuiProgress",
                                 "Auto-hinting subfont %1 of %2"
                                 " with %3 glyphs...")
                               .arg(curr_sfnt + 1)
                               .arg(num_sfnts)
                               .arg(num_glyphs));

    if (curr_sfnt + 1 == num_sfnts)
    {
      data->dialog->setAutoReset(true);
      data->dialog->setAutoClose(true);
    }
    else
    {
      data->dialog->setAutoReset(false);
      data->dialog->setAutoClose(false);
    }

    data->last_sfnt = curr_sfnt;
    data->begin = true;
  }

  if (data->begin)
  {
    if (num_sfnts == 1)
      data->dialog->setLabelText(QCoreApplication::translate(
                                   "GuiProgress",
                                   "Auto-hinting %1 glyphs...")
                                 .arg(num_glyphs));
    data->dialog->setMaximum(num_glyphs - 1);

    data->begin = false;
  }

  data->dialog->setValue(curr_idx);

  if (data->dialog->wasCanceled())
    return 1;

  return 0;
}

} // extern "C"


// return value 1 indicates a retry

int
Main_GUI::handle_error(TA_Error error,
                       const unsigned char* error_string,
                       QString output_name)
{
  int ret = 0;

  if (error == TA_Err_Canceled)
     ;
  else if (error == TA_Err_Invalid_FreeType_Version)
    QMessageBox::critical(
      this,
      "TTFautohint",
      tr("FreeType version 2.4.5 or higher is needed.\n"
         "Are you perhaps using a wrong FreeType DLL?"),
      QMessageBox::Ok,
      QMessageBox::Ok);
  else if (error == TA_Err_Missing_Legal_Permission)
  {
    int yesno = QMessageBox::warning(
                  this,
                  "TTFautohint",
                  tr("Bit 1 in the %1 field of the %2 table is set:"
                     " This font must not be modified"
                     " without permission of the legal owner.\n"
                     "Do you have such a permission?")
                     .arg(QUOTE_STRING_LITERAL("fsType"))
                     .arg(QUOTE_STRING_LITERAL("OS/2")),
                  QMessageBox::Yes | QMessageBox::No,
                  QMessageBox::No);
    if (yesno == QMessageBox::Yes)
    {
      ignore_permissions = true;
      ret = 1;
    }
  }
  else if (error == TA_Err_Missing_Unicode_CMap)
    QMessageBox::warning(
      this,
      "TTFautohint",
      tr("No Unicode character map."),
      QMessageBox::Ok,
      QMessageBox::Ok);
  else if (error == TA_Err_Missing_Glyph)
    QMessageBox::warning(
      this,
      "TTFautohint",
      tr("No glyph for the key character"
         " to derive standard width and height.\n"
         "For the latin script, this key character is %1 (U+006F).")
         .arg(QUOTE_STRING_LITERAL("o")),
      QMessageBox::Ok,
      QMessageBox::Ok);
  else
    QMessageBox::warning(
      this,
      "TTFautohint",
      tr("Error code 0x%1 while autohinting font:\n")
         .arg(error, 2, 16, QLatin1Char('0'))
        + QString::fromLocal8Bit((const char*)error_string),
      QMessageBox::Ok,
      QMessageBox::Ok);

  if (QFile::exists(output_name) && remove(qPrintable(output_name)))
  {
    const int buf_len = 1024;
    char buf[buf_len];

    strerror_r(errno, buf, buf_len);
    QMessageBox::warning(
      this,
      "TTFautohint",
      tr("The following error occurred while removing output file %1:\n")
         .arg(QUOTE_STRING(QDir::toNativeSeparators(output_name)))
        + QString::fromLocal8Bit(buf),
      QMessageBox::Ok,
      QMessageBox::Ok);
  }

  return ret;
}


void
Main_GUI::run()
{
  statusBar()->clearMessage();

  QString input_name = QDir::fromNativeSeparators(input_line->text());
  QString output_name = QDir::fromNativeSeparators(output_line->text());
  if (!check_filenames(input_name, output_name))
    return;

  // we need C file descriptors for communication with TTF_autohint
  FILE* input;
  FILE* output;

again:
  if (!open_files(input_name, &input, output_name, &output))
    return;

  QProgressDialog dialog;
  dialog.setCancelButtonText(tr("Cancel"));
  dialog.setMinimumDuration(1000);
  dialog.setWindowModality(Qt::WindowModal);

  GUI_Progress_Data gui_progress_data = {-1, true, &dialog};
  const unsigned char* error_string;

  TA_Error error =
    TTF_autohint("in-file, out-file,"
                 "hinting-range-min, hinting-range-max,"
                 "error-string,"
                 "progress-callback, progress-callback-data,"
                 "ignore-permissions, pre-hinting,"
                 "fallback-script",
                 input, output,
                 min_box->value(), max_box->value(),
                 &error_string,
                 gui_progress, &gui_progress_data,
                 ignore_permissions, pre_box->isChecked(),
                 fallback_box->currentIndex());

  fclose(input);
  fclose(output);

  if (error)
  {
    if (handle_error(error, error_string, output_name))
      goto again;
  }
  else
    statusBar()->showMessage(tr("Auto-hinting finished."));
}


void
Main_GUI::create_layout()
{
  // file stuff
  QCompleter* completer = new QCompleter(this);
  QFileSystemModel* model = new QFileSystemModel(completer);
  model->setRootPath(QDir::rootPath());
  completer->setModel(model);

  QLabel* input_label = new QLabel(tr("&Input File:"));
  input_line = new QLineEdit;
  input_button = new QPushButton(tr("Browse..."));
  input_label->setBuddy(input_line);
  // enforce rich text to get nice word wrapping
  input_label->setToolTip(
    tr("<b></b>The input file, either a TrueType font (TTF),"
       " TrueType collection (TTC), or a TrueType-based OpenType font."));
  input_line->setCompleter(completer);

  QLabel* output_label = new QLabel(tr("&Output File:"));
  output_line = new QLineEdit;
  output_button = new QPushButton(tr("Browse..."));
  output_label->setBuddy(output_line);
  output_label->setToolTip(
    tr("<b></b>The output file, which will be essentially identical"
       " to the input font but contains new, generated hints."));
  output_line->setCompleter(completer);

  QGridLayout* file_layout = new QGridLayout;
  file_layout->addWidget(input_label, 0, 0);
  file_layout->addWidget(input_line, 0, 1);
  file_layout->addWidget(input_button, 0, 2);
  file_layout->addWidget(output_label, 1, 0);
  file_layout->addWidget(output_line, 1, 1);
  file_layout->addWidget(output_button, 1, 2);

  // minmax controls
  QLabel* min_label = new QLabel(tr("Mi&nimum:"));
  min_box = new QSpinBox;
  min_label->setBuddy(min_box);
  min_box->setKeyboardTracking(false);
  min_box->setRange(2, 10000);
  min_box->setValue(hinting_range_min);

  QLabel* max_label = new QLabel(tr("Ma&ximum:"));
  max_box = new QSpinBox;
  max_label->setBuddy(max_box);
  max_box->setKeyboardTracking(false);
  max_box->setRange(2, 10000);
  max_box->setValue(hinting_range_max);

  check_min();
  check_max();

  QGridLayout* minmax_layout = new QGridLayout;
  minmax_layout->addWidget(min_label, 0, 0);
  minmax_layout->addWidget(min_box, 0, 1);
  minmax_layout->addWidget(max_label, 1, 0);
  minmax_layout->addWidget(max_box, 1, 1);

  // hinting and fallback controls
  QLabel* hinting_label = new QLabel(tr("Hint Set Range") + " ");
  QLabel* fallback_label = new QLabel(tr("F&allback Script:"));
  hinting_label->setToolTip(
    tr("The PPEM range for which <b>TTFautohint</b> computes"
       " <i>hint sets</i>."
       " A hint set for a given PPEM value hints this size optimally."
       " The larger the range, the more hint sets are considered,"
       " usually increasing the size of the bytecode.\n"
       "Note that changing this range doesn't influence"
       " the <i>gasp</i> table:"
       " Hinting is enabled for all sizes."));
  fallback_box = new QComboBox;
  fallback_label->setBuddy(fallback_box);
  fallback_label->setToolTip(
    tr("This sets the fallback script module for glyphs"
       " which <b>TTFautohint</b> can't map to a script automatically."));
  fallback_box->insertItem(0, tr("None"));
  fallback_box->insertItem(1, tr("Latin"));

  QHBoxLayout* hint_fallback_layout = new QHBoxLayout;
  hint_fallback_layout->addWidget(hinting_label);
  hint_fallback_layout->addLayout(minmax_layout);
  hint_fallback_layout->addStretch(1);
  hint_fallback_layout->addWidget(fallback_label);
  hint_fallback_layout->addWidget(fallback_box);
  hint_fallback_layout->addStretch(2);

  // flags
  pre_box = new QCheckBox(tr("Pr&e-hinting"), this);
  pre_box->setToolTip(
    tr("If switched on, the original bytecode of the input font"
       " gets applied before <b>TTFautohint</b> starts processing"
       " the outlines of the glyphs."));

  QHBoxLayout* flags_layout = new QHBoxLayout;
  flags_layout->addWidget(pre_box);
  flags_layout->addStretch(1);

  // running
  run_button = new QPushButton(tr("&Run"));
  run_button->setEnabled(false);

  QHBoxLayout* running_layout = new QHBoxLayout;
  running_layout->addStretch(1);
  running_layout->addWidget(run_button);
  running_layout->addStretch(1);

  // the whole gui
  QVBoxLayout* gui_layout = new QVBoxLayout;
  gui_layout->addSpacing(10); // XXX urgh, pixels...
  gui_layout->addLayout(file_layout);
  gui_layout->addSpacing(20); // XXX urgh, pixels...
  gui_layout->addLayout(hint_fallback_layout);
  gui_layout->addSpacing(20); // XXX urgh, pixels...
  gui_layout->addLayout(flags_layout);
  gui_layout->addSpacing(20); // XXX urgh, pixels...
  gui_layout->addLayout(running_layout);
  gui_layout->addSpacing(10); // XXX urgh, pixels...

  // create dummy widget to register layout
  QWidget* main_widget = new QWidget;
  main_widget->setLayout(gui_layout);
  setCentralWidget(main_widget);
  setWindowTitle("TTFautohint");
}


void
Main_GUI::create_connections()
{
  connect(input_button, SIGNAL(clicked()), this,
          SLOT(browse_input()));
  connect(output_button, SIGNAL(clicked()), this,
          SLOT(browse_output()));

  connect(input_line, SIGNAL(textChanged(QString)), this,
          SLOT(check_run()));
  connect(output_line, SIGNAL(textChanged(QString)), this,
          SLOT(check_run()));

  connect(input_line, SIGNAL(editingFinished()), this,
          SLOT(absolute_input()));
  connect(output_line, SIGNAL(editingFinished()), this,
          SLOT(absolute_output()));

  connect(min_box, SIGNAL(valueChanged(int)), this,
          SLOT(check_min()));
  connect(max_box, SIGNAL(valueChanged(int)), this,
          SLOT(check_max()));

  connect(run_button, SIGNAL(clicked()), this,
          SLOT(run()));
}


void
Main_GUI::create_actions()
{
  exit_act = new QAction(tr("E&xit"), this);
  exit_act->setShortcuts(QKeySequence::Quit);
  connect(exit_act, SIGNAL(triggered()), this, SLOT(close()));

  about_act = new QAction(tr("&About"), this);
  connect(about_act, SIGNAL(triggered()), this, SLOT(about()));

  about_Qt_act = new QAction(tr("About &Qt"), this);
  connect(about_Qt_act, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}


void
Main_GUI::create_menus()
{
  file_menu = menuBar()->addMenu(tr("&File"));
  file_menu->addAction(exit_act);

  help_menu = menuBar()->addMenu(tr("&Help"));
  help_menu->addAction(about_act);
  help_menu->addAction(about_Qt_act);
}


void
Main_GUI::create_status_bar()
{
  statusBar()->showMessage("");
}


void
Main_GUI::read_settings()
{
  QSettings settings;
//  QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
//  QSize size = settings.value("size", QSize(400, 400)).toSize();
//  resize(size);
//  move(pos);
}


void
Main_GUI::write_settings()
{
  QSettings settings;
//  settings.setValue("pos", pos());
//  settings.setValue("size", size());
}

// end of maingui.cpp
