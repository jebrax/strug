# Delivering Strug

Pushing a version tag builds the game for every platform and publishes it as a GitHub Release. Players
download from there directly, and a launcher can do the same by reading `manifest.json`.

| Platform       | Built on                    | Runs on                        |
|----------------|-----------------------------|--------------------------------|
| `macos-arm64`  | `macos-15` runner           | macOS 12 or newer, Apple Silicon |
| `linux-x86_64` | `ubuntu:22.04` in Docker    | glibc 2.34 or newer, X11       |

## Releasing

```
git tag v0.1.0
git push origin v0.1.0
```

[`release.yml`](../.github/workflows/release.yml) then builds and verifies both packages and publishes
the release. The tag is the version. A tag with a suffix, like `v0.2.0-rc.1`, is published as a
prerelease, which the "latest" links below ignore.

Pushes to `master` and pull requests run the same builds without publishing. Their packages are
attached to the workflow run as artifacts.

## Downloads

These links always point to the newest non-prerelease version:

```
https://github.com/jebrax/strug/releases/latest/download/strug-macos-arm64.tar.gz
https://github.com/jebrax/strug/releases/latest/download/strug-linux-x86_64.tar.gz
https://github.com/jebrax/strug/releases/latest/download/SHA256SUMS
```

A package unpacks into a single `strug` directory. The executable finds `assets` next to itself, so
the directory can live anywhere as long as it stays together.

```
strug/
  strug       the executable
  assets/
  licenses/   notices of the libraries linked into the executable
  VERSION     the version, on a single line
```

macOS marks files downloaded with a browser as quarantined, and refuses to open them because the
game isn't signed with an Apple Developer ID. Until it is, players need to run this once:

```
xattr -dr com.apple.quarantine strug
```

## Launcher contract

A launcher only needs this URL:

```
https://github.com/jebrax/strug/releases/latest/download/manifest.json
```

```json
{
  "schema": 1,
  "game": "strug",
  "version": "0.1.0",
  "commit": "398b11adea56bfd42d3c5edb38f7b6c7ba9833cd",
  "publishedAt": "2026-11-05T18:00:00Z",
  "builds": {
    "linux-x86_64": {
      "url": "https://github.com/jebrax/strug/releases/download/v0.1.0/strug-linux-x86_64.tar.gz",
      "sha256": "…",
      "size": 5245337,
      "executable": "strug/strug"
    },
    "macos-arm64": {
      "url": "https://github.com/jebrax/strug/releases/download/v0.1.0/strug-macos-arm64.tar.gz",
      "sha256": "…",
      "size": 5245337,
      "executable": "strug/strug",
      "minimumOs": "12.0"
    }
  }
}
```

To install or update, a launcher compares `version` with the `VERSION` file of the installed copy,
downloads `url` for its platform, checks `sha256` before unpacking, and starts `executable`, which is
relative to the directory it unpacked into. It should unpack next to the old copy and swap them only
after that succeeds. Increase `schema` when making a change that existing launchers can't handle.

A launcher should download with its own HTTP client rather than a browser, then macOS doesn't
quarantine the files and players don't need the command above.

## Building a package locally

macOS, with `brew install autoconf automake libtool cmake`:

```
scripts/build-deps.sh
make package
scripts/verify-package.sh dist/strug-macos-arm64.tar.gz
```

Linux, from any machine that has Docker and is x86-64:

```
docker build -t strug-builder -f ci/linux-builder.Dockerfile ci
docker run --rm --user "$(id -u):$(id -g)" -e HOME=/tmp -v "$PWD:/strug" -w /strug strug-builder \
  sh -c 'scripts/build-deps.sh && make package && scripts/verify-package.sh dist/strug-linux-x86_64.tar.gz'
```

## Dependency notes

- Glade stays at its pinned commit, later versions don't support Strug. See
  [GLADE_SUPPORT.md](../GLADE_SUPPORT.md).
- `build-deps.sh` builds Assimp with only its OBJ importer. A model in another format needs the
  matching `ASSIMP_BUILD_*_IMPORTER` option added there.
- `bullet3` isn't used by anything, so `.gitmodules` skips it.

## Known issues

Verification covers the package, not the game, because the game currently crashes on launch:

- `./strug` crashes on the first input poll: `CubeTest` never connects to a server, and
  `WorldController` sends input through Glade's null server peer. This code isn't platform specific.
- `./strug --server` throws `There is no uniform in the shader` on Mesa: Glade's
  `non-pbr.fragment.glsl` outputs a constant color, so its texture uniforms get optimized out. Whether
  it also happens on macOS depends on Apple's GLSL compiler and hasn't been checked.

Once these are fixed, start the game in `verify-package.sh` to catch regressions.
