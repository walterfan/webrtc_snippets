# C++ SIP/WebRTC Practice Lab

30 个可编译但未解答的练习，用一个月复习现代 C++、SIP 和 WebRTC。所有输入都是仓库内的固定教学数据；不会打开网络连接、执行 SIP/WebRTC 信令，也不会实现 DTLS/SRTP 密码学。

## Build and run

Requires CMake 3.20+ and a compiler with usable C++20 support. The `basic` targets request C++20 locally and do not raise the parent `snippets` project from C++11.

GoogleTest is required when `BUILD_TESTING=ON`. On macOS with Homebrew: `brew install googletest`.

```bash
cmake -S snippets/basic -B build/basic
cmake --build build/basic --parallel
ctest --test-dir build/basic --output-on-failure
./build/basic/cpp_rtc_practice
./build/basic/cpp_rtc_practice --list
./build/basic/cpp_rtc_practice --info 01
./build/basic/cpp_rtc_practice --run 15
./build/basic/cpp_rtc_practice --check 15
./build/basic/practice_15
```

The menu accepts `list`, `info 01`, `run 01`, `check 01`, `progress`, `help`, and `quit`. Invalid input is rejected without ending the menu. `--check` returns 0 when passed, 1 when failed, and 2 while the supplied TODO remains. Therefore the default CTest suite tests the framework only; it does not expect unsolved exercises to pass.

The included `Makefile` provides equivalent shortcuts from `snippets/basic`.
Builds are incremental: CMake recompiles only stale translation units and
`make run` / `make check` only refresh `cpp_rtc_practice`, not all 30
standalone binaries.

```bash
make                # display available commands
make build          # configure if needed, then incrementally build the CLI
make run DAY=02     # incrementally build and run one exercise
make check DAY=02   # incrementally build and check one exercise
make run-02         # same as make run DAY=02
make list
make info DAY=02
make test           # incrementally build tests, then ctest
make build-all      # also build practice_01 .. practice_30
make web-build      # install Crow with Conan, then build the web app
make build-web      # alias for web-build
make run-web        # run in the foreground; press Ctrl-C to stop
make start-web      # build if needed and start on 127.0.0.1:8080
make stop-web       # stop the process recorded by the Makefile
make restart-web    # stop then start
```

Override `WEB_PORT`, `WEB_BUILD_DIR`, or `WEB_PID_FILE` when needed, for
example `make start-web WEB_PORT=8081`. The PID and log default to
`build/practice-web.pid` and `build/practice-web.log` in the repository root.

GoogleTest discovers one acceptance test per day. Before you implement a day, its test is reported as **skipped**. Implement the exercise and make the corresponding GoogleTest in `tests/acceptance_test.cpp` execute the Given/When/Then acceptance cases from its README; the corresponding GoogleTest then passes or fails from that result.

## Local learning website

The optional website follows the same C++ HTTP + static HTML/JavaScript layout
as `csms-test-service`. It only listens on `127.0.0.1`, serves the 30 exercises
read-only, stores completion marks in browser `localStorage`, and exposes two
actions for the selected day: `check` builds and runs its filtered GoogleTest
acceptance case, while `run()` builds and runs that day's standalone example.
It never accepts a browser-provided shell command or source path.

Install the pinned Crow dependency and configure a separate web build:

```bash
conan install snippets/basic --output-folder build/conan --build=missing -s build_type=Debug
cmake -S snippets/basic -B build/basic-web \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/build/conan/build/Debug/generators/conan_toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DPRACTICE_BUILD_WEB_SERVER=ON
cmake --build build/basic-web --parallel
./build/basic-web/practice_web_server --build-dir build/basic-web
```

Open `http://127.0.0.1:8080`. Choose a day, read its task and source tabs, edit
`starter.cpp` in your local editor, save, and click **运行验收测试** or
**运行 run() 示例**. The server rebuilds the selected target and returns a
bounded result panel.

## Workflow

For each day, read its `README.md`, inspect the declarations in `starter.hpp`, then implement the TODOs in `starter.cpp` and run the GoogleTest acceptance test for day NN. Keep your own solution local to that day's starter. Headers intentionally expose only types and function declarations; source files contain the explicit TODO implementation points and no hidden solution. Days 03–05 consume a shared header-only message series parser and intentionally model only the documented RFC subset.

## 30-day index

- [ ] [01 — SIP Request-Line](exercises/01_sip_request_line/README.md): `string_view`, `from_chars`; SIP request structure.
- [ ] [02 — SIP Header List](exercises/02_sip_headers/README.md): structs, vectors; repeated headers.
- [ ] [03 — SIP Transactions](exercises/03_sip_transaction/README.md): branch/sent-by/CSeq matching, ACK/CANCEL.
- [ ] [04 — SIP Dialogs](exercises/04_sip_dialog/README.md): Call-ID/tags, forking, early/confirmed state.
- [ ] [05 — SIP Route Paths](exercises/05_sip_router/README.md): Record-Route, Route, Contact, loose/strict routing.
- [ ] [06 — Via Routing](exercises/06_via_routing/README.md): Via hop parsing; branch/received/rport.
- [ ] [07 — Registration Bindings](exercises/07_registration_bindings/README.md): AOR Contact store; refresh/expiry.
- [ ] [08 — Response Capabilities](exercises/08_response_capabilities/README.md): status classes; Require/Supported tags.
- [ ] [09 — Offer/Answer](exercises/09_offer_answer/README.md): session media validation; direction/rejection.
- [ ] [10 — Reliable Provisional](exercises/10_reliable_provisional/README.md): 100rel; RSeq/RAck/PRACK matching.
- [ ] [11 — INVITE Transaction](exercises/11_invite_transaction/README.md): INVITE states; Accepted; ACK ownership.
- [ ] [12 — Session Refresh](exercises/12_session_refresh/README.md): Session-Expires; UPDATE; glare.
- [ ] [13 — Events and Transfer](exercises/13_events_transfer/README.md): SUBSCRIBE/NOTIFY; REFER; Replaces.
- [ ] [14 — Protocol Errors](exercises/14_protocol_errors/README.md): typed SIP parse/validation errors.
- [ ] [15 — RTP Header Codec](exercises/15_rtp_header_codec/README.md): bytes/bit ops; RTP header.
- [ ] [16 — RTP Packet View](exercises/16_rtp_packet_view/README.md): `span`; zero-copy boundaries.
- [ ] [17 — Sequence Unwrap](exercises/17_sequence_unwrap/README.md): integer safety; RTP rollover.
- [ ] [18 — RTCP Statistics](exercises/18_rtcp_statistics/README.md): numeric algorithms; RR metrics.
- [ ] [19 — Jitter Buffer](exercises/19_jitter_buffer/README.md): containers/move; reorder.
- [ ] [20 — Packet Queue](exercises/20_packet_queue/README.md): `jthread`/stop token; media pipeline.
- [ ] [21 — ICE Checks](exercises/21_ice_checks/README.md): futures/timeouts; static candidate pairs.
- [ ] [22 — RTP Stats](exercises/22_rtp_stats/README.md): atomics; counter snapshots.
- [ ] [23 — SDP Sections](exercises/23_sdp_media_sections/README.md): ranges/views; `m=`/`a=` lines.
- [ ] [24 — Codec Negotiation](exercises/24_codec_negotiation/README.md): projections; payload compatibility.
- [ ] [25 — ICE Ranking](exercises/25_ice_candidate_rank/README.md): `<=>`; priority ordering.
- [ ] [26 — DTLS Fingerprint](exercises/26_dtls_fingerprint/README.md): validated text; fingerprint syntax.
- [ ] [27 — SRTP Replay Window](exercises/27_srtp_replay_window/README.md): bitset/unsigned math; replay policy.
- [ ] [28 — DataChannel Reassembly](exercises/28_datachannel_reassembly/README.md): maps/ownership; fragments.
- [ ] [29 — Signaling Coroutine](exercises/29_signaling_coroutine/README.md): coroutines; offer/answer events.
- [ ] [30 — PeerConnection State](exercises/30_peer_connection_state/README.md): concepts/RAII; composed states.

## Reading references and scope

Use RFC 3261 for SIP, RFC 3550 for RTP/RTCP, RFC 8866 for SDP, RFC 8445 for ICE, RFC 5764 for DTLS-SRTP, RFC 3711 for SRTP, RFC 8831 for WebRTC DataChannels, and RFC 8825 for WebRTC architecture. The exercises model carefully stated fragments of these protocols only. In particular, Day 26 does not authenticate certificates, Day 27 does not encrypt media, and Days 20–30 use local deterministic data rather than external services.

The following RFCs are representation-only reading context and are **not**
implemented (no sockets, DNS, media, cryptography, or extra APIs): RFC 3263
DNS service discovery, RFC 4733 DTMF, RFC 6086 INFO, RFC 7044 History-Info,
RFC 7433 UUI, RFC 8224 STIR Identity, RFC 8760 Digest algorithms,
RFC 7865/7866 SIPREC, and RFC 8866 SDP text.
