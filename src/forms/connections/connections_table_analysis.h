#ifndef CONNECTIONS_TABLE_ANALYSIS_H
#define CONNECTIONS_TABLE_ANALYSIS_H

#include <vector>

class ConnectionsTableAnalysis {

 public:
  bool hasNoErrors() {
    return duplicateServerNames.empty() && noServerNameRows.empty();
  }

 public:
  std::vector<size_t> emptyRows; // typically: remove these
  std::vector<size_t> incompleteDataRows; // typically: warn about issues
  std::vector<std::pair<size_t, size_t>> duplicateServerNames; // typically: this is an error that cannot be saved
  std::vector<size_t> noServerNameRows; // typically: this is an error that cannot be saved
};

#endif // CONNECTIONS_TABLE_ANALYSIS_H
