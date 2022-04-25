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
    std::pair<QString, QString>(QStringLiteral("Plain Text"), QString()),
    std::pair<QString, QString>(QStringLiteral("ABAP"), QStringLiteral("abap")),
    std::pair<QString, QString>(QStringLiteral("ActionScript"), QStringLiteral("actionscript")),
    std::pair<QString, QString>(QStringLiteral("Ada"), QStringLiteral("ada")),
    std::pair<QString, QString>(QStringLiteral("C / C++"), QStringLiteral("c_cpp")),
    std::pair<QString, QString>(QStringLiteral("C#"), QStringLiteral("csharp")),
    std::pair<QString, QString>(QStringLiteral("COBOL"), QStringLiteral("cobol")),
    std::pair<QString, QString>(QStringLiteral("D"), QStringLiteral("d")),
    std::pair<QString, QString>(QStringLiteral("Dart"), QStringLiteral("dart")),
    std::pair<QString, QString>(QStringLiteral("Delphi/Object Pascal"), QStringLiteral("pascal")),
    std::pair<QString, QString>(QStringLiteral("Dockerfile"), QStringLiteral("dockerfile")),
    std::pair<QString, QString>(QStringLiteral("Elixir"), QStringLiteral("elixir")),
    std::pair<QString, QString>(QStringLiteral("Elm"), QStringLiteral("elm")),
    std::pair<QString, QString>(QStringLiteral("Erlang"), QStringLiteral("erlang")),
    std::pair<QString, QString>(QStringLiteral("F#"), QStringLiteral("fsharp")),
    std::pair<QString, QString>(QStringLiteral("Fortran"), QStringLiteral("fortran")),
    std::pair<QString, QString>(QStringLiteral("Go"), QStringLiteral("golang")),
    std::pair<QString, QString>(QStringLiteral("Groovy"), QStringLiteral("groovy")),
    std::pair<QString, QString>(QStringLiteral("Haskell"), QStringLiteral("haskell")),
    std::pair<QString, QString>(QStringLiteral("Java"), QStringLiteral("java")),
    std::pair<QString, QString>(QStringLiteral("JavaScript"), QStringLiteral("javascript")),
    std::pair<QString, QString>(QStringLiteral("Julia"), QStringLiteral("julia")),
    std::pair<QString, QString>(QStringLiteral("Kotlin"), QStringLiteral("kotlin")),
    std::pair<QString, QString>(QStringLiteral("Lisp"), QStringLiteral("lisp")),
    std::pair<QString, QString>(QStringLiteral("Lua"), QStringLiteral("lua")),
    std::pair<QString, QString>(QStringLiteral("MATLAB"), QStringLiteral("matlab")),
    std::pair<QString, QString>(QStringLiteral("Markdown"), QStringLiteral("markdown")),
    std::pair<QString, QString>(QStringLiteral("Objective-C"), QStringLiteral("objectivec")),
    std::pair<QString, QString>(QStringLiteral("PHP"), QStringLiteral("php")),
    std::pair<QString, QString>(QStringLiteral("Perl"), QStringLiteral("perl")),
    std::pair<QString, QString>(QStringLiteral("Prolog"), QStringLiteral("prolog")),
    std::pair<QString, QString>(QStringLiteral("Properties"), QStringLiteral("properties")),
    std::pair<QString, QString>(QStringLiteral("Python"), QStringLiteral("python")),
    std::pair<QString, QString>(QStringLiteral("R"), QStringLiteral("r")),
    std::pair<QString, QString>(QStringLiteral("Ruby"), QStringLiteral("ruby")),
    std::pair<QString, QString>(QStringLiteral("Rust"), QStringLiteral("rust")),
    std::pair<QString, QString>(QStringLiteral("SQL"), QStringLiteral("sql")),
    std::pair<QString, QString>(QStringLiteral("Sass"), QStringLiteral("sass")),
    std::pair<QString, QString>(QStringLiteral("Scala"), QStringLiteral("scala")),
    std::pair<QString, QString>(QStringLiteral("Scheme"), QStringLiteral("scheme")),
    std::pair<QString, QString>(QStringLiteral("Shell/Bash"), QStringLiteral("sh")),
    std::pair<QString, QString>(QStringLiteral("Swift"), QStringLiteral("swift")),
    std::pair<QString, QString>(QStringLiteral("Tcl"), QStringLiteral("tcl")),
    std::pair<QString, QString>(QStringLiteral("Terraform"), QStringLiteral("terraform")),
    std::pair<QString, QString>(QStringLiteral("Toml"), QStringLiteral("toml")),
    std::pair<QString, QString>(QStringLiteral("TypeScript"), QStringLiteral("typescript")),
    std::pair<QString, QString>(QStringLiteral("VBScript"), QStringLiteral("vbscript")),
    std::pair<QString, QString>(QStringLiteral("XML"), QStringLiteral("xml")),
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
