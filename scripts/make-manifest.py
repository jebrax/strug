#!/usr/bin/env python3
"""Writes manifest.json and SHA256SUMS for the archives in a directory. See docs/DELIVERY.md."""

import argparse
import hashlib
import json
import re
from datetime import datetime, timezone
from pathlib import Path

ARCHIVE_NAME = re.compile(r"^strug-(?P<platform>.+)\.tar\.gz$")
MINIMUM_OS = {"macos-arm64": "12.0"}


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("--version", required=True)
    parser.add_argument("--tag", required=True)
    parser.add_argument("--commit", required=True)
    parser.add_argument("--repository", required=True, help="GitHub owner/name")
    args = parser.parse_args()

    builds = {}
    sums = []

    for archive in sorted(args.directory.glob("strug-*.tar.gz")):
        platform = ARCHIVE_NAME.match(archive.name).group("platform")
        checksum = sha256(archive)
        sums.append(f"{checksum}  {archive.name}\n")
        builds[platform] = {
            "url": f"https://github.com/{args.repository}/releases/download/{args.tag}/{archive.name}",
            "sha256": checksum,
            "size": archive.stat().st_size,
            "executable": "strug/strug",
        }
        if platform in MINIMUM_OS:
            builds[platform]["minimumOs"] = MINIMUM_OS[platform]

    if not builds:
        parser.error(f"no strug-*.tar.gz archives in {args.directory}")

    manifest = {
        "schema": 1,
        "game": "strug",
        "version": args.version,
        "commit": args.commit,
        "publishedAt": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "builds": builds,
    }

    (args.directory / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    (args.directory / "SHA256SUMS").write_text("".join(sums))


if __name__ == "__main__":
    main()
