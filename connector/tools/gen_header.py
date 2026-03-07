#!/usr/bin/env python3
from __future__ import annotations

import re
import sys
from argparse import ArgumentParser
from pathlib import Path
from typing import Iterable, Optional, Sequence

# =========================
# Header templates/constants
# =========================

HEADER_PREAMBLE: str = "/* Auto-generated from vendor headers. Do not edit. */\n"
HEADER_GUARD_OPEN: str = (
    "#ifndef WEBRTCCONNECTOR_PUBLIC_API_H\n#define WEBRTCCONNECTOR_PUBLIC_API_H\n"
)
HEADER_STD_INCLUDES: str = (
    "#include <stdint.h>\n#include <stddef.h>\n#include <stdbool.h>\n"
)
# Windows-aware, consumer-safe export macro (inside extern \"C\" as requested).
# - If building the DLL itself: define WEBRTCCONNECTOR_BUILD, yields dllexport.
# - Else: dllimport for Windows, default visibility or empty elsewhere.
DLL_MACRO_PORTABLE: str = (
    "#ifndef DLLExport\n"
    "#  if defined(_WIN32) || defined(_WIN64)\n"
    "#    if defined(WEBRTCCONNECTOR_BUILD)\n"
    "#      define DLLExport __declspec(dllexport)\n"
    "#    else\n"
    "#      define DLLExport __declspec(dllimport)\n"
    "#    endif\n"
    "#  elif defined(__GNUC__) && __GNUC__ >= 4\n"
    '#    define DLLExport __attribute__((visibility("default")))\n'
    "#  else\n"
    "#    define DLLExport\n"
    "#  endif\n"
    "#endif\n"
)
EXTERN_C_OPEN: str = '#ifdef __cplusplus\nextern "C" {\n#endif\n'
EXTERN_C_CLOSE: str = "#ifdef __cplusplus\n}\n#endif\n"
HEADER_GUARD_CLOSE: str = "#endif /* WEBRTCCONNECTOR_PUBLIC_API_H */\n"

# ==================================================
# Tokens that must never be considered opaque types.
# ==================================================

BUILTINS: set[str] = {
    "void",
    "char",
    "signed",
    "unsigned",
    "short",
    "int",
    "long",
    "float",
    "double",
    "bool",
    "size_t",
    "ptrdiff_t",
    "uintptr_t",
    "intptr_t",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint64_t",
    "int8_t",
    "int16_t",
    "int32_t",
    "int64_t",
    "wchar_t",
    "char16_t",
    "char32_t",
    "_Bool",
}
QUALIFIERS: set[str] = {"const", "volatile", "struct", "class", "enum"}


# =========
# I/O utils
# =========


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="ignore")


def write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


# =========================
# Sanitisation / extraction
# =========================


def strip_local_includes(text: str) -> str:
    """Remove any local quoted include:  #include "xxx.h"  (future-proof)."""
    return re.sub(r'^[ \t]*#\s*include\s*"[^"\n]+"\s*\n', "", text, flags=re.MULTILINE)


def strip_pragma_once(text: str) -> str:
    """Remove standalone '#pragma once' lines (we add our own guard)."""
    return re.sub(r"^[ \t]*#\s*pragma\s+once\s*\n", "", text, flags=re.MULTILINE)


def replace_extern_c_with_payload(text: str) -> str:
    """
    Replace any braced extern "C" block with its inner payload.
    Robust to whitespace/comments/newlines between 'extern "C"' and '{'.
    """
    pattern = r'extern\s*"C"\s*(?:(?://[^\n]*|/\*.*?\*/|\s))*\{([\s\S]*?)\}'
    return re.sub(pattern, _extern_payload_replacer, text)


def _extern_payload_replacer(match: re.Match) -> str:
    return match.group(1)


def remove_vendor_dllexport_blocks(text: str) -> str:
    """
    Remove vendor-side DLLExport macro definitions and their guards, to avoid
    use-before-define and double-definition. Also drop now-empty groups.
    """
    # Drop direct defines
    step1 = re.sub(
        r"^[ \t]*#\s*define\s+DLLExport[^\n]*\n", "", text, flags=re.MULTILINE
    )

    # Drop any #if*…#endif block that mentions DLLExport
    def drop_if_mentions_dllexport(m: re.Match) -> str:
        block = m.group(0)
        return "" if re.search(r"\bDLLExport\b", block) else block

    guarded = r"(?ms)^[ \t]*#\s*if(?:n?def)?[^\n]*\n.*?^[ \t]*#\s*endif[^\n]*\n?"
    step2 = re.sub(guarded, drop_if_mentions_dllexport, step1)

    # Drop empty groups (only preprocessor/whitespace inside)
    def cleaner(m: re.Match) -> str:
        inner = m.group(1)
        cleaned = "\n".join(
            line
            for line in inner.splitlines()
            if not line.strip().startswith("#") and line.strip()
        )
        return "" if cleaned.strip() == "" else m.group(0)

    empty_group = r"(?ms)^[ \t]*#\s*if(?:n?def)?[^\n]*\n(.*?)^[ \t]*#\s*endif[^\n]*\n?"
    return re.sub(empty_group, cleaner, step2)


def extern_c_payload(text: str) -> str:
    """Concatenate payloads inside braced extern \"C\" blocks from a header."""
    pattern = r'extern\s*"C"\s*(?:(?://[^\n]*|/\*.*?\*/|\s))*\{([\s\S]*?)\}'
    return "\n".join(m.group(1) for m in re.finditer(pattern, text))


def is_function_prototype(line: str) -> bool:
    s = line.strip()
    return s.endswith(";") and "(" in s and ")" in s and "=" not in s


def is_c_abi_declaration(line: str) -> bool:
    s = line.strip()
    return (
        bool(s)
        and not s.startswith("#")
        and (
            s.startswith("typedef")
            or (not re.match(r"^(class|struct)\b", s) and is_function_prototype(s))
        )
    )


def c_abi_only(text: str) -> str:
    return "\n".join(
        line.strip() for line in text.splitlines() if is_c_abi_declaration(line)
    )


def opaque_identifiers(text: str) -> list[str]:
    tokens_in_order = tuple(
        dict.fromkeys(m.group(1) for m in re.finditer(r"\b([A-Za-z_]\w*)\s*\*+", text))
    )
    return [
        n
        for n in tokens_in_order
        if n.lower() not in BUILTINS and n.lower() not in QUALIFIERS
    ]


# ===============
# Header assembly
# ===============


def make_header(opaque: Sequence[str], c_from_log: str, plugin_payload: str) -> str:
    opaque_block = "".join(f"typedef struct {name} {name};\n" for name in opaque)
    return "".join(
        part
        for part in (
            HEADER_PREAMBLE,
            HEADER_GUARD_OPEN,
            HEADER_STD_INCLUDES,
            EXTERN_C_OPEN,  # Everything lives inside extern "C" as requested
            DLL_MACRO_PORTABLE,  # DLLExport defined BEFORE any use, and Windows-aware
            (opaque_block + "\n") if opaque_block else "",
            (c_from_log.strip() + "\n\n") if c_from_log.strip() else "",
            plugin_payload.rstrip(),
            "\n",
            EXTERN_C_CLOSE,
            HEADER_GUARD_CLOSE,
        )
    )


# =====
# main
# =====


def main(argv: Optional[Iterable[str]] = None) -> int:
    parser = ArgumentParser(
        description=(
            "Generate a self-contained C header from vendor plugin.h (+ optional log.h C ABI). "
            'If --log is provided and exists, C declarations inside braced extern "C" blocks are merged.'
        )
    )
    parser.add_argument(
        "--plugin", default="plugin.h", type=Path, help="path to vendor plugin.h"
    )
    parser.add_argument("--log", type=Path, help="path to vendor log.h (optional)")
    parser.add_argument(
        "--out", default="WebRTCConnector.h", type=Path, help="output header path"
    )
    args = parser.parse_args(list(argv or []))

    # 1) Prepare plugin payload: strip includes/pragma, flatten extern \"C\" braces,
    #    remove vendor DLLExport blocks (and any now-empty groups).
    plugin_text = read_text(args.plugin)
    plugin_text = strip_local_includes(plugin_text)
    plugin_text = strip_pragma_once(plugin_text)
    plugin_text = replace_extern_c_with_payload(plugin_text)
    plugin_text = remove_vendor_dllexport_blocks(plugin_text)

    # 2) Optionally add C ABI from log.h (only braced extern \"C\" content).
    log_c_abi = (
        ""
        if not args.log or not args.log.exists()
        else c_abi_only(extern_c_payload(read_text(args.log)))
    )

    # 3) Compute opaque types from the composite text we will expose.
    composite = "\n".join(p for p in (plugin_text, log_c_abi) if p)
    opaque = opaque_identifiers(composite)

    # 4) Assemble and write header.
    write_text(args.out, make_header(opaque, log_c_abi, plugin_text))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
