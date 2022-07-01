#pragma once

#include "components/evidencepreview.h"
#include "models/codeblock.h"

class CodeEditor;
class QComboBox;
class QLineEdit;
/**
 * @brief The CodeBlockView class provides a wrapped code editor, along with editable
 * areas for source and language. Note that even though this is a "view" it's fully editable.
 * Set to readonly if you need a proper view.
 */
class CodeBlockView : public EvidencePreview {
  Q_OBJECT
 public:
  explicit CodeBlockView(QWidget* parent = nullptr);
  ~CodeBlockView() = default;

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
  /// Returns False if failed.
  virtual bool saveEvidence() override;

  /// clearPreview removes the content, source, and sets the language to "Plain Text". Inherited
  /// from EvidencePreview
  virtual void clearPreview() override;

  /// Sets the editable areas to readonly/writable (depending on value passed).
  /// Inherited from EvidencePreview
  virtual void setReadonly(bool readonly) override;

 private:
  Codeblock loadedCodeblock;

  // UI components
  CodeEditor* codeEditor = nullptr;
  QLineEdit* sourceTextBox = nullptr;
  QComboBox* languageComboBox = nullptr;
  // matches supported languages on the front end
  inline static const QList<QPair<QString, QString>> SUPPORTED_LANGUAGES = {
      QPair<QString, QString>(QStringLiteral("Plain Text"), QString()),
      QPair<QString, QString>(QStringLiteral("ABAP"), QStringLiteral("abap")),
      QPair<QString, QString>(QStringLiteral("ActionScript"), QStringLiteral("actionscript")),
      QPair<QString, QString>(QStringLiteral("Ada"), QStringLiteral("ada")),
      QPair<QString, QString>(QStringLiteral("C / C++"), QStringLiteral("c_cpp")),
      QPair<QString, QString>(QStringLiteral("C#"), QStringLiteral("csharp")),
      QPair<QString, QString>(QStringLiteral("COBOL"), QStringLiteral("cobol")),
      QPair<QString, QString>(QStringLiteral("D"), QStringLiteral("d")),
      QPair<QString, QString>(QStringLiteral("Dart"), QStringLiteral("dart")),
      QPair<QString, QString>(QStringLiteral("Delphi/Object Pascal"), QStringLiteral("pascal")),
      QPair<QString, QString>(QStringLiteral("Dockerfile"), QStringLiteral("dockerfile")),
      QPair<QString, QString>(QStringLiteral("Elixir"), QStringLiteral("elixir")),
      QPair<QString, QString>(QStringLiteral("Elm"), QStringLiteral("elm")),
      QPair<QString, QString>(QStringLiteral("Erlang"), QStringLiteral("erlang")),
      QPair<QString, QString>(QStringLiteral("F#"), QStringLiteral("fsharp")),
      QPair<QString, QString>(QStringLiteral("Fortran"), QStringLiteral("fortran")),
      QPair<QString, QString>(QStringLiteral("Go"), QStringLiteral("golang")),
      QPair<QString, QString>(QStringLiteral("Groovy"), QStringLiteral("groovy")),
      QPair<QString, QString>(QStringLiteral("Haskell"), QStringLiteral("haskell")),
      QPair<QString, QString>(QStringLiteral("Java"), QStringLiteral("java")),
      QPair<QString, QString>(QStringLiteral("JavaScript"), QStringLiteral("javascript")),
      QPair<QString, QString>(QStringLiteral("Julia"), QStringLiteral("julia")),
      QPair<QString, QString>(QStringLiteral("Kotlin"), QStringLiteral("kotlin")),
      QPair<QString, QString>(QStringLiteral("Lisp"), QStringLiteral("lisp")),
      QPair<QString, QString>(QStringLiteral("Lua"), QStringLiteral("lua")),
      QPair<QString, QString>(QStringLiteral("MATLAB"), QStringLiteral("matlab")),
      QPair<QString, QString>(QStringLiteral("Markdown"), QStringLiteral("markdown")),
      QPair<QString, QString>(QStringLiteral("Objective-C"), QStringLiteral("objectivec")),
      QPair<QString, QString>(QStringLiteral("PHP"), QStringLiteral("php")),
      QPair<QString, QString>(QStringLiteral("Perl"), QStringLiteral("perl")),
      QPair<QString, QString>(QStringLiteral("Prolog"), QStringLiteral("prolog")),
      QPair<QString, QString>(QStringLiteral("Properties"), QStringLiteral("properties")),
      QPair<QString, QString>(QStringLiteral("Python"), QStringLiteral("python")),
      QPair<QString, QString>(QStringLiteral("R"), QStringLiteral("r")),
      QPair<QString, QString>(QStringLiteral("Ruby"), QStringLiteral("ruby")),
      QPair<QString, QString>(QStringLiteral("Rust"), QStringLiteral("rust")),
      QPair<QString, QString>(QStringLiteral("SQL"), QStringLiteral("sql")),
      QPair<QString, QString>(QStringLiteral("Sass"), QStringLiteral("sass")),
      QPair<QString, QString>(QStringLiteral("Scala"), QStringLiteral("scala")),
      QPair<QString, QString>(QStringLiteral("Scheme"), QStringLiteral("scheme")),
      QPair<QString, QString>(QStringLiteral("Shell/Bash"), QStringLiteral("sh")),
      QPair<QString, QString>(QStringLiteral("Swift"), QStringLiteral("swift")),
      QPair<QString, QString>(QStringLiteral("Tcl"), QStringLiteral("tcl")),
      QPair<QString, QString>(QStringLiteral("Terraform"), QStringLiteral("terraform")),
      QPair<QString, QString>(QStringLiteral("Toml"), QStringLiteral("toml")),
      QPair<QString, QString>(QStringLiteral("TypeScript"), QStringLiteral("typescript")),
      QPair<QString, QString>(QStringLiteral("VBScript"), QStringLiteral("vbscript")),
      QPair<QString, QString>(QStringLiteral("XML"), QStringLiteral("xml")),
  };
};
