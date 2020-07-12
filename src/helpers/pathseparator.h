// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef PATHSEPARATOR_H
#define PATHSEPARATOR_H

// FROM:
// https://stackoverflow.com/questions/12971499/how-to-get-the-file-separator-symbol-in-standard-c-c-or
// Note: this is only going to work for Windows and (linux/mac). Boost may have the ability to
// resolve this Note2: We probably don't need this. Qt treats "/" as the universal path separator.
// We would only need this for reflecting the path to the user. See:
// https://doc.qt.io/qt-5/qdir.html

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

#endif  // PATHSEPARATOR_H
