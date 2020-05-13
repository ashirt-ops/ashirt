#! /usr/bin/env python

import xml.etree.ElementTree as ET
import sys


def main():
    if len(sys.argv) < 3:
        print("Project root not provided.")
        return

    migration_file = sys.argv[1]
    new_filename = sys.argv[2]
    tree = ET.parse(migration_file)
    root = tree.getroot()

    for child in root:
        if child.attrib.get('prefix') == '/':
            newFileEntry = ET.SubElement(child, "file")
            newFileEntry.text = new_filename
            # try to keep pretty -- not really necessary
            if len(child) > 1:
                child[-2].tail = "\n        "
            newFileEntry.tail ="\n    "
            break

    tree.write(migration_file)


if __name__ == "__main__":
    main()
