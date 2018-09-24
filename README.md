Thanks for using Grilo!

# What is Grilo?

Grilo is a framework for browsing and searching media content from various
sources using a single API.

# Where can I find more?

We have a [wiki page](https://wiki.gnome.org/Projects/Grilo)

You can subscribe to our [mailing list](http://mail.gnome.org/mailman/listinfo/grilo-list)

You can join us on the IRC:
#grilo on GIMPNet

# How do I start?

Once you have Grilo installed, you may want to play around with the
examples (See section "Examples") or check its documentation
and tutorials (See section "Documentation").

## Ubuntu

If you are using Ubuntu you can install binary packages by configuring
our PPA, check the [wiki page](https://wiki.gnome.org/Projects/Grilo#Distros)

## Archlinux

```
pacman -S grilo grilo
```

## Others

Otherwise you  have to download Grilo's source code from GNOME's repository and
build it -don't worry, it takes only a few seconds-, see the section below if
you want to do that.

## Building from git

```
git clone https://gitlab.gnome.org/GNOME/grilo.git
cd grilo
meson build
ninja -C build
sudo ninja -C build install
```

# Examples

If you are looking for some example to play with you can try grilo-test-ui,
a simple GTK+ based test application for testing purposes. You can find it
under tools/grilo-test-ui. It allows users to:

 * Browse predefined content categories from browseable sources (like
"Youtube", "Jamendo" or "UPnP").
 * Search content by text on searchable sources (like "Youtube", "Jamanedo"
or "Flickr").
 * Query sources using source-specific syntax (for sources implementing
this feature).
 * Organize and define the source's content hierarchy (for sources
implementing this feature like "bookmarks" or
"podcasts").
 * Check available metadata for the media.

For this application to work you need some Grilo plugins that act as media
sources, that is, plugins that provide the actual content that you will
browse and search using the application. You can get a bunch of plugins for
Grilo from the [grilo-plugins page](https://gitlab.gnome.org/GNOME/grilo-plugins)

If you are looking for a step-by-step guide from beginning to end, here it is:

## Building Grilo-Plugins

Check the [Grilo-Plugins README file](https://gitlab.gnome.org/GNOME/grilo-plugins/blob/master/README.md) for instructions

## Running grilo-test-ui

```
grilo-test-ui-0.3
```
Enjoy!
