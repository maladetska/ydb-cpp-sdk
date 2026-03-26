## v3.16.0

* Added support for the new inverted index type: JSON, intended to speed-up queries on Json or JsonDocument columns.

## v3.15.0

* EXPERIMENTAL! Added `IProducer` interface to the SDK. This interface is used to write messages to a topic.
Each message can be associated with a partitioning key, which is used to determine the partition to which the message will be written.

* Added gRPC load balancing policy option for `TDriver`. Default policy: `round_robin`.

## v3.13.0

* Removed the `layout` field from `FulltextIndexSettings` and replaced it with separate index types in `TableIndexDescription`.
  `layout` was a preliminary version of API, actual YDB release 26-1 uses separate index types, so please note that creating
  full text indexes via gRPC won't work with previous versions of SDK.
