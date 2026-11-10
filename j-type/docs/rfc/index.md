# Protocol RFC

The network protocol is specified as an Internet-Draft-style RFC, so that someone can write another client for a J-Type
server from the specification alone.

- [Read the RFC (text)](j-type-protocol.txt)
- [Read the RFC (HTML)](j-type-protocol.html)
- [RFCXML source](j-type-protocol.xml)

The document covers version 1 of the protocol: the UDP transport and the 1200-byte datagram limit, the 8-byte packet
header and the session handshake, every game message with its fields and sizes, the chunking of a snapshot, the rules
of reliability and ordering, and what a receiver validates and drops.

**Choice: one specification, generated from a source.** The RFC is written in [RFCXML v3](https://authors.ietf.org/) and
built with `xml2rfc`, the tool of the IETF. The generated text is committed beside the source and published here, so the
website shows the same bytes a reviewer reads without running the generator.

**Price:** the text is generated, so a change to the wire format touches both the XML and the text. The contributing
guide gives the command, and the code stays the reference when the two disagree: the RFC describes what the codec and
the server do, not what they should do.

## Building the text and the HTML

`xml2rfc` is part of the documentation requirements:

```sh
pip install -r docs/requirements.txt
xml2rfc --text --out docs/rfc/j-type-protocol.txt docs/rfc/j-type-protocol.xml
xml2rfc --html --out docs/rfc/j-type-protocol.html docs/rfc/j-type-protocol.xml
mkdocs build --strict
```

A change to the protocol updates the RFCXML, regenerates the text, and updates the networking page of the technical
study. The message identifiers and sizes live in `common/include/jtype/common/Messages.hpp`.
