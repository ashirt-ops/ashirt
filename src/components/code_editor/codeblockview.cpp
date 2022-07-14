#include "codeblockview.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include "codeeditor.h"
#include "helpers/ui_helpers.h"

CodeBlockView::CodeBlockView(QWidget* parent)
  : EvidencePreview(parent)
  , codeEditor(new CodeEditor(this))
  , sourceTextBox(new QLineEdit(this))
  , languageComboBox(new QComboBox(this))
{
  buildUi();
}

void CodeBlockView::buildUi() {

  for (auto & lang : SUPPORTED_LANGUAGES) {
    languageComboBox->addItem(lang.first, lang.second);
  }

  // Layout
  /*        0              1           2          3
       +------------+-------------+----------+----------------+
    0  | Lang Lab   | [ textbox ] | Src Lab  | [src textbox]  |
       +------------+-------------+----------+----------------+
    1  |                                                      |
       |            Code Block                                |
       |                                                      |
       +------------+-------------+----------+----------------+
  */
  auto gridLayout = new QGridLayout(this);
  gridLayout->setContentsMargins(0, 0, 0, 0);
  // row 0
  gridLayout->addWidget(new QLabel(tr("Language"), this), 0, 0);
  gridLayout->addWidget(languageComboBox, 0, 1);
  gridLayout->addWidget(new QLabel(tr("Source"), this), 0, 2);
  gridLayout->addWidget(sourceTextBox, 0, 3);
  // row 1
  gridLayout->addWidget(codeEditor, 1, 0, 1, gridLayout->columnCount());
}

void CodeBlockView::loadFromFile(QString filepath)
{
    codeEditor->setPlainText(tr("No Codeblock Loaded"));
    loadedCodeblock = Codeblock::readCodeblock(filepath);
    codeEditor->setPlainText(loadedCodeblock.content);
    sourceTextBox->setText(loadedCodeblock.source);
    UIHelpers::setComboBoxValue(languageComboBox, loadedCodeblock.subtype);
}

bool CodeBlockView::saveEvidence() {
  loadedCodeblock.source = sourceTextBox->text();
  loadedCodeblock.subtype = languageComboBox->currentData().toString();
  loadedCodeblock.content = codeEditor->toPlainText();
  if (!loadedCodeblock.filePath().isEmpty())
      return Codeblock::saveCodeblock(loadedCodeblock);
  return false;
}

void CodeBlockView::clearPreview() {
  codeEditor->clear();
  sourceTextBox->clear();
  languageComboBox->setCurrentIndex(0);  // should be Plain Text
}

void CodeBlockView::setReadonly(bool readonly) {
  EvidencePreview::setReadonly(readonly);
  codeEditor->setReadOnly(readonly);
  sourceTextBox->setReadOnly(readonly);
  languageComboBox->setEnabled(!readonly);
}
