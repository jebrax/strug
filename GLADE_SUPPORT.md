# End of Glade support for Strug

Strug support ended on 2026-10-27. Future Glade development does not need to
maintain compatibility with Strug, and Strug is no longer a required reference
application for engine work.

The last supported Glade version is the original sibling `../glade` repository's
commit:

```text
aa1c8163a6d0c08f101e12b3f08f10aa93794620
Add Context-managed fixed updates for game states
```

This records the support boundary chosen by the owner. Strug was not rebuilt or
tested when recording it; the existing API-rename verification caveat in README.md
still applies.

Strug's pinned `deps/glade` remains at
`36abebdbc2a2ac992a0e042baa92079c16413afa`; it has not been migrated to the
checkpoint above. This note changes neither application code nor dependencies.
Keep the checkpoint hash when consulting or restoring the historical engine API;
do not assume later Glade versions support Strug.
