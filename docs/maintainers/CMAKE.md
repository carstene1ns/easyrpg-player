# Maintainers' Notes for CMake

## General

- Minimum supported version should be what is available under the oldest still
  supported Debian LTS release (usually around 5 years old)

- Features of newer CMake versions are allowed to use, but backward compat
  must be assured. The CMake Changelog has notes about breaking behaviour
  changes and needs to be examined when integrating new features. CI will use
  an old version and shows potential breakage

- When bumping the version requirement, look out for potential old workarounds,
  policy changes and now unneeded stuff

## Structure

- Keep general line length under 120 (hard limit), better 100 (soft limit)

- Use lowercase syntax for script elements and functions, upper case for
  arguments and variables

- Indent with Tabs, after linebreaks inside a function or inside conditionals

- Add comments, when the intention is not clear from script alone

- Split in logical blocks, e.g. first handle all dependencies

- Handle the common case last, after all special cases

- Use a helper function if it makes the script more concise and readable

## Helper Functions and Modules

- In PlayerMisc ...TODO

- In PlayerBuildType ...TODO

- In PlayerConfigureWindows ...TODO

## Presets

- TODO: preset generator

