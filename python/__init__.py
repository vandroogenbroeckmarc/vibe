"""Self-contained Python port of the libvibe++ (C++) implementation of ViBe."""

from .vibe import ViBeSequential, manhattan_match

__all__ = ["ViBeSequential", "manhattan_match"]
