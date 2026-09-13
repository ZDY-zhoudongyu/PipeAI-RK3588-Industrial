#pragma once
struct MemoryInfo { long rss_kb{0}; long peak_kb{0}; };
MemoryInfo get_memory_info();
