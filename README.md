# ASHIRT

A Qt tray-type application that allows for capturing (via user-selectable area or entire window) screenshots, or codeblocks associated with a particular ASHIRT instance.

## Table of Contents

- [Background](#background)
- [Install](#install)
- [Configuration](#configuration)
- [Contribute](#contribute)
- [License](#license)

## Background

This application allows users to connect to a remote ASHIRT backend and create and submit new evidence. Screenshots are taken using a custom, user-defined key, or alternately by selecting the appropriate action in the tray menu. Codeblocks can be added via an action in the tray. Both can be managed from within the application.

## Install

Official releases for Mac and Linux will be provided via the releases tab in GitHub along with source code for users to build themselves if desired.

## Non-tray OSes

Current Status: Non-functional

Some OSes/desktops do not support a tray (e.g. ice3 window manager). Currently, in these cases, the application will not work, and simply exit instead. Eventually, a simple CLI will be set up to continue to interact with this application.

## Getting Started

On the first launch, the user must first set up an appropriate configuration. When the tray displays, open the tray and select `Settings`. From here, you will be presented with some options:

| Field                           | Meaning                                                                                                                      |
| ------------------------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| Evidence Repository             | Where evidence is stored. Note that this is a jumping off point. Files are stored in a subdirectory using the operation name |
| Access Key                      | The (shorter) base-64 key given by the AShirt front end (look for this in Account Settings)                                  |
| Secret Key                      | The (longer) base-64 key given by the AShirt front end                                                                       |
| Host Path                       | The http location to the AShirt server                                                                                       |
| Capture Area Command            | The CLI command to take a screenshot of an arbitrary area and save to a file. More on this below                             |
| [Capture Area Command] Shortcut | The key combination used (at a system level) to trigger the capture area command                                             |
| Capture Window Command          | The CLI command to take of a given window, and save to a file                                                                |
| [Capture Area Command] Shortcut | The key combination used (at a system level) to trigger the capture window command                                           |

Once the above is configured, save the settings and you can now select an operation. Open the tray, and under `Select Operation`, choose an operation to start using the application. (Note: you may need to choose `Refresh Operations` in the submenu)

## Screenshot Commands

This application requires taking screenshots from the command line. The application _must_:

1. Allow for saving the screenshot to a named file.
2. Create the file _must_ before the application exits.

Theorectically, any application that satisfies this requirement will work. For Mac, the system command to do this is pre-populated, since this is a standard feature. For Linux, there are a number of screenshot commands, and so none are provided. For Windows, a 3rd party application must be used, as there is currently no way to save a screenshot to a named file.

This tool will replace the above filename with `%file` as noted below:

| OS/DE/App   | Capture Window               | Capture Area                 | Notes                                                                                                                                               |
| ----------- | ---------------------------- | ---------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| Linux/Gnome | gnome-screenshot -w -f %file | gnome-screenshot -a -f %file | Capture window captures the focused window, rather than allowing a selection; adding the `--delay` flag can help mitigate choosing the wrong window |
| MacOS X     | screencapture -w %file       | screencapture -s %file       |                                                                                                                                                     |

Note: this application expects a _single, basic command_. While piping output to another command _may_ work, it is not guaranteed. Likewise, providing multiple commands on the same "line" _may_ work, but is also not guaranteed. Offically, both of these techniques are unsupported.

### Shortcuts

Global shortcut keys can be registered with your computer, depending on the exact operating system. These shortcuts may conflict with shortcuts for a given application, where it is unclear which shortcut will trigger. All this is to say that this feature, while supported, may not work perfectly every time. That said, here is how you configure shortcuts:

Within `Settings` next to each capture command is a small text box to provide the shortcut. Each shortcut should add in one or more modifier keys (e.g. `ctrl`) in order to provide less of a chance to interfere with other system/application commands. These modifier keys have reserved names for shortcuts, noted in the below table:

| Key                            | Name      | Alternate Names |
| ------------------------------ | --------- | --------------- |
| Shift                          | `shift`   | `shft`          |
| Control                        | `control` | `ctrl`          |
| Alt                            | `alt`     | --              |
| Windows/Meta/MacOS Command key | `meta`    | `win`           |

To specify a shortcut pattern, simply decide on what set of modifier keys you want, plus a single alphanumeric key (or F- key), and separate these by `+`.

E.g. `Ctrl+Shift+p`

## Switching Operations

To change operations, navigate to `Select Operation` and choose one of the operations exposed in the list. If the operation you are looking for is not in the list, try pressing the `Refresh Operations`, or check with the operation owner to ensure that you have write access to that operation.

### Multiple backends

Some users may end up using this application for multiple backends -- either for testing, or due to multiple communities using the same software. Currently this application does not strongly support swiching backends, though it is possible. To swtich backends, simply enter the new API Key, Secret Key, and Host Path in the settings menu. Note that when switching, it is likely your current selected operation will be removed and you will need to select a new operation.

A particular edgecase that is not supported is when multiple backends have the same name for different operations. In these cases, it is incumbent on the user to be vigilant and ensure that the right data goes to the right backend.

## Managing Evidence

Previous evidence can be reviewed by navigating to `View Accumulated Evidence`, which will present a screen showing evidence for the current operation. Selecting a row in the evidence list will show:

- A preview of the evidence (Images can be scaled by changing the window size, or my shrinking the description box -- mouse over the divider separating the description from the image)
- The description of the evidence
- Any (active) tags associated with the evidence.

From here you can submit the evidence, if not already submitted. Or, you may delete the file (even if previously submitted -- doing so will remove the file locally, but keep the website copy)

### Filtering Evidence

Filtering can be done by specifying items in `key:value` format. Multiple filters can be added by adding a space between each filter. Keys and values are case insensitive.

| Action                                    | Key         | Values                                             | Alias(es)                 | Notes                                                              |
| ----------------------------------------- | ----------- | -------------------------------------------------- | ------------------------- | ------------------------------------------------------------------ |
| Show submit errors                        | `err`       | `t`/`f`, or `y`/`n`                                | `error`, `fail`, `failed` | Also works with `true`/`false` `yes`/`no`                          |
| Show evidence for operations              | `op`        | operation slug                                     | `operation`               | Pre-populated with current operation, when reset button is pressed |
| Show evidence taken _before_ a given date | `before`    | `today`, `yesterday` or date in yyyy-MM-dd format, | `to`, `til`, `until`      | Starts at midnight of the given day                                |
| Show evidence taken _after_ a given date  | `after`     | `today`, `yesterday` or date in yyyy-MM-dd format, | `from`                    | Start just before midnight of the _next_ given day                 |
| Show evidence taken _on_ a given date     | `on`        | `today`, `yesterday` or date in yyyy-MM-dd format, | --                        |                                                                    |
| Show evidence that has not been submitted | `submitted` | `t`/`f`, or `y`/`n`                                | --                        | Also works with `true`/`false`, `yes`/`no`                         |

#### Date filtering

When trying to apply both a "before" date and "after" date filter, the system will adjust the times so that the "before" date is always _after_ the "after" date. Meaning, the timespan must be inclusive. For example, a range of "before March" and "after May" (excluding March and April) is not valid, and will be revised to "After March, Before May"

When applying only one date, the range is unbounded on the other end. That is, dates are implicily "from the start of time" to "until the end of time"

## Local Files

You should never need to access these files outside of the application, however, for clarity, the following files are generate and maintained by this application:

| File type      | Path                                                                | Notes                                                                                                                            |
| -------------- | ------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| Screenshots    | `$eviRepo/$operationSlug/ashirt_screenshot_$randomCharacters.png`   | Presently, random (english) characters tacked on to the end of a screenshot, to add uniqueness and prevent overwriting           |
| Codeblocks     | `$eviRepo/$operationSlug/ashirt_codeblock_$randomCharacters.json`   | Presently, random (english) characters tacked on to the end of the codeblock filename, to add uniqueness and prevent overwriting |
| Configuration  | `$userSettingsDirectory/ashirt/screenshot.json`                     | Manages connection info / configuration in "settings" menu                                                                       |
| Local Database | `$userSettingsDirectory/ashirt/screenshot.sqlite`                   |                                                                                                                                  |
| Settings       | `$userSettingsDirectory/Verizon Media Group/AShirt Screenshot.conf` | Manages state info -- e.g. last used operation ; Managed by Qt                                                                   |

### Variable locations

The above paths reference some variables. Some of these values change depending on what operating system is being used (or in how it is configured). The exact paths are unknown, but this may help you find these files:

| Path Variable            | Notes                                                                                                          |
| ------------------------ | -------------------------------------------------------------------------------------------------------------- |
| `$userSettingsDirectory` | Where user-specific configuration/settings files are stored.                                                   |
| [For Linux]              | On the command line, run `echo $XDG_CONFIG_HOME`  (by default, this is typically the `~/.config` directory)    |
| [For Mac OSX]            | Check `/Users/(username)/Library/Preferences`                                                                  |
| [For windows]            | This may be in the System Registry                                                                             |
| `$eviRepo`               | The Evidence Repository value in the "settings" window                                                         |
| `$operationSlug`         | The operation slug for a given operation. This is a unique representation of an operation name                 |
| `$randomCharacters`      | Six random english characters, case-insensitive (for those operating systems that support this). e.g. `fTaNpS` |

## Developer Notes

Interested in contributing? See the [developer notes](Readme_Developer.md) for style guide, organization, etc

## Configuration

All configuration options are managed through the application UI.

## Contribute

Please refer to [the contributing.md file](Contributing.md) for information about how to get involved. We welcome issues, questions, and pull requests.

## License

This project is licensed under the terms of the [MIT](LICENSE) open source license. Please refer to [LICENSE](LICENSE) for the full terms.

## Credits / Contributors / Thanks

- Joel Smith
- Alex David

## Maintainers

- Joe Rozner joe.rozer@verizonmedia.com
