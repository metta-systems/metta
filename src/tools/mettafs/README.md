#### Metta Associative FS

Use Qt4 to prototype quickly a file-based tagfs implementation.

A graph-db may be an interesting approach (file contents are nodes, tags are either edges or separate nodes, in which case a semantic relation of object with tag may be specified e.g. FIDO is_a DOG).
Full semantic information at fs level - interesting.

#### Requirements

* Instant or nearly-instant mount. No fsck needed on crash recovery.
* No hierarchical folders.
* Arbitrary indexed metadata per each file.
* Copy-on-write for simple versioning/snapshotting.
* Extensible media structures to reduce compatibility issues as time progresses.
* Support for very large file-systems.
* Data integrity checks, including high-level snapshot checksums.
* Performance - lookups and arbitrary operations on tags might require a lot of processing and these lookups should be efficient.
