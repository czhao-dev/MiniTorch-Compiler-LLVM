from __future__ import annotations

import functools
import inspect
import shutil
import subprocess
import tempfile
from pathlib import Path
from typing import Any, Callable, TypeVar

F = TypeVar("F", bound=Callable[..., Any])


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def compile(fn: F) -> F:
    """Compile a MiniTorch function once the native backend is implemented."""
    source = inspect.getsource(fn)
    mtc = shutil.which("mtc") or str(_repo_root() / "build" / "mtc")

    @functools.wraps(fn)
    def wrapper(*args: Any, **kwargs: Any) -> Any:
        if not Path(mtc).exists():
            raise RuntimeError("mtc compiler not found; run `cmake -S . -B build && cmake --build build` first")

        with tempfile.TemporaryDirectory(prefix="minitorch-") as tmp:
            source_path = Path(tmp) / f"{fn.__name__}.py"
            source_path.write_text(source)
            subprocess.run([mtc, str(source_path), "--emit-ast"], check=True, capture_output=True, text=True)

        raise NotImplementedError("native MiniTorch execution is not implemented yet")

    return wrapper  # type: ignore[return-value]

