#ifndef CODEBLOCKVIEW_H
#define CODEBLOCKVIEW_H

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QWidget>
#include <vector>

#include "codeeditor.h"
#include "components/evidencepreview.h"
#include "models/codeblock.h"

// matches supported languages on the front end
static std::vector<std::pair<QString, QString>> SUPPORTED_LANGUAGES = {
    std::pair<QString, QString>("Plain Text", ""),
    std::pair<QString, QString>("ABAP", "abap"),
    std::pair<QString, QString>("ActionScript", "actionscript"),
    std::pair<QString, QString>("Ada", "ada"),
    std::pair<QString, QString>("C / C++", "c_cpp"),
    std::pair<QString, QString>("C#", "csharp"),
    std::pair<QString, QString>("COBOL", "cobol"),
    std::pair<QString, QString>("D", "d"),
    std::pair<QString, QString>("Dart", "dart"),
    std::pair<QString, QString>("Delphi/Object Pascal", "pascal"),
    std::pair<QString, QString>("Dockerfile", "dockerfile"),
    std::pair<QString, QString>("Elixir", "elixir"),
    std::pair<QString, QString>("Elm", "elm"),
    std::pair<QString, QString>("Erlang", "erlang"),
    std::pair<QString, QString>("F#", "fsharp"),
    std::pair<QString, QString>("Fortran", "fortran"),
    std::pair<QString, QString>("Go", "golang"),
    std::pair<QString, QString>("Groovy", "groovy"),
    std::pair<QString, QString>("Haskell", "haskell"),
    std::pair<QString, QString>("Java", "java"),
    std::pair<QString, QString>("JavaScript", "javascript"),
    std::pair<QString, QString>("Julia", "julia"),
    std::pair<QString, QString>("Kotlin", "kotlin"),
    std::pair<QString, QString>("Lisp", "lisp"),
    std::pair<QString, QString>("Lua", "lua"),
    std::pair<QString, QString>("MATLAB", "matlab"),
    std::pair<QString, QString>("Markdown", "markdown"),
    std::pair<QString, QString>("Objective-C", "objectivec"),
    std::pair<QString, QString>("PHP", "php"),
    std::pair<QString, QString>("Perl", "perl"),
    std::pair<QString, QString>("Prolog", "prolog"),
    std::pair<QString, QString>("Properties", "properties"),
    std::pair<QString, QString>("Python", "python"),
    std::pair<QString, QString>("R", "r"),
    std::pair<QString, QString>("Ruby", "ruby"),
    std::pair<QString, QString>("Rust", "rust"),
    std::pair<QString, QString>("SQL", "sql"),
    std::pair<QString, QString>("Sass", "sass"),
    std::pair<QString, QString>("Scala", "scala"),
    std::pair<QString, QString>("Scheme", "scheme"),
    std::pair<QString, QString>("Shell/Bash", "sh"),
    std::pair<QString, QString>("Swift", "swift"),
    std::pair<QString, QString>("Tcl", "tcl"),
    std::pair<QString, QString>("Terraform", "terraform"),
    std::pair<QString, QString>("Toml", "toml"),
    std::pair<QString, QString>("TypeScript", "typescript"),
    std::pair<QString, QString>("VBScript", "vbscript"),
    std::pair<QString, QString>("XML", "xml"),
};

/**
 * @brief The CodeBlockView class provides a wrapped code editor, along with editable
 * areas for source and language. Note that even though this is a "view" it's fully editable.
 * Set to readonly if you need a proper view.
 */
class CodeBlockView : public EvidencePreview {
  Q_OBJECT
 public:
  explicit CodeBlockView(QWidget* parent = nullptr);
  ~CodeBlockView();

 private:
  /// buildUi constructs the UI, without wiring any connections
  void buildUi();

  /// wireUi connects UI elements together (currently a no-op)
  void wireUi();

 public:
  /// loadFromFile attempts to load the indicated codeblock from disk.
  /// If this process fails, renders a message instead of the codeblock. Inherited from
  /// EvidencePreview
  virtual void loadFromFile(QString filepath) override;

  /// saveEvidence attempts to write the codeblock back to disk, where it was loaded from.
  /// Inherited from EvidencePreview
  /// Note: Will silently fail if an error occurs while saving. Implemented as best effort approach.
  virtual void saveEvidence() override;

  /// clearPreview removes the content, source, and sets the language to "Plain Text". Inherited
  /// from EvidencePreview
  virtual void clearPreview() override;

  /// Sets the editable areas to readonly/writable (depending on value passed).
  /// Inherited from EvidencePreview
  virtual void setReadonly(bool readonly) override;

 private:
  Codeblock loadedCodeblock;

  // UI components
  QGridLayout* gridLayout;
  QLabel* _languageLabel;
  QLabel* _sourceLabel;

  CodeEditor* codeEditor;
  QLineEdit* sourceTextBox;
  QComboBox* languageComboBox;
};

#endif  // CODEBLOCKVIEW_H
